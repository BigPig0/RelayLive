#include "stdafx.h"
#include "uv.h"
#include "liveReceiver.h"
#include "LiveWorker.h"
#include "rtp.h"
#include "ps.h"
#include "pes.h"
#include "es.h"
#include "h264.h"
#include "Recode.h"
#include "utilc.h"

namespace LiveClient
{
extern int  g_nRtpCatchPacketNum;  //< rtp缓存的包的数量

static void AVCallback(AV_BUFF buff, void* pUser){
    CLiveReceiver* pLive = (CLiveReceiver*)pUser;
    switch (buff.eType)
    {
    case AV_TYPE::PS:
        pLive->push_ps_stream(buff);
        break;
    case AV_TYPE::PES:
        pLive->push_pes_stream(buff);
        break;
    case AV_TYPE::ES:
        pLive->push_es_stream(buff);
        break;
    case AV_TYPE::H264_NALU:
        pLive->push_h264_stream(buff);
        break;
    default:
        break;
    }
}

//////////////////////////////////////////////////////////////////////////
/**
 * udp接收事件循环
 */
struct udp_recv_loop_t {
    uv_loop_t   uvLoop;           // udp接收的loop
    uv_udp_t    uvRtpSocket;      // rtp接收
    uv_timer_t  uvTimeOver;       // 接收超时定时器
    uv_async_t  uvAsync;          // 外部线程通知结束loop
    bool        running;          // loop线程是否正在执行
    int         uvHandleNum;      // 启动的uv句柄数，这些句柄通过uv_close关闭
    string      remoteIP;         // 发送方IP
    int         remotePort;       // 发送方端口
    int         port;
    void       *user;             // 用户对象
};

/** udp接收申请缓存空间 */
static void echo_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    UNUSED(handle);
    UNUSED(suggested_size);
    *buf = uv_buf_init((char*)malloc(PACK_MAX_SIZE), PACK_MAX_SIZE);
}

/** udp接收数据，读取一个包 */
static void after_read(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)
{
    udp_recv_loop_t *loop = (udp_recv_loop_t*)handle->data;
    UNUSED(flags);
    if(nread < 0){
        Log::error("read error: %s",uv_strerror(nread));
        free(buf->base);
    }
	if(nread == 0)
		return;

    //udp来源不匹配，将数据抛弃
    struct sockaddr_in* addr_in =(struct sockaddr_in*)addr;
    int port = ntohs(addr_in->sin_port);
    char ipv4addr[64]={0};
    uv_ip4_name(addr_in, ipv4addr, 64);
    string ip = ipv4addr;
    if(loop->remotePort != port || loop->remoteIP != ip) {
        //Log::error("this is not my rtp data");
        return;
    }

    //重置超时计时器
    int ret = uv_timer_again(&loop->uvTimeOver);
    if(ret < 0) {
        Log::error("timer again error: %s", uv_strerror(ret));
        return;
    }

    CLiveReceiver* pLive = (CLiveReceiver*)loop->user;
    pLive->RtpRecv(buf->base, nread);
}

/** 超时定时器到时回调 */
static void timer_cb(uv_timer_t* handle)
{
    udp_recv_loop_t *loop = (udp_recv_loop_t*)handle->data;
    CLiveReceiver* pLive = (CLiveReceiver*)loop->user;
    pLive->RtpOverTime();
}

/** 关闭uv句柄的回调 */
static void udp_uv_close_cb(uv_handle_t* handle){
    udp_recv_loop_t *loop = (udp_recv_loop_t*)handle->data;
    loop->uvHandleNum--;
    //uv句柄全部关闭后，停止loop
    if(!loop->uvHandleNum){
        loop->running = false;
        uv_stop(&loop->uvLoop);
    }
}

/** 外部线程通知回调，在loop回调中关闭用到的uv句柄 */
static void async_cb(uv_async_t* handle){
    udp_recv_loop_t *loop = (udp_recv_loop_t*)handle->data;
    int ret = uv_udp_recv_stop(&loop->uvRtpSocket);
    if(ret < 0) {
        Log::error("stop rtp recv port:%d err: %s", loop->port, uv_strerror(ret));
    }
    uv_close((uv_handle_t*)&loop->uvRtpSocket, udp_uv_close_cb);

    ret = uv_timer_stop(&loop->uvTimeOver);
    if(ret < 0) {
        Log::error("stop timer error: %s",uv_strerror(ret));
    }
    uv_close((uv_handle_t*)&loop->uvTimeOver, udp_uv_close_cb);

    uv_close((uv_handle_t*)&loop->uvAsync, udp_uv_close_cb);
}

/** event loop thread */
static void run_loop_thread(void* arg)
{
    udp_recv_loop_t *loop = (udp_recv_loop_t*)arg;
    CLiveReceiver *recver = (CLiveReceiver*)loop->user;
    recver->m_nRunNum++;
    while (loop->running) {
        uv_run(&loop->uvLoop, UV_RUN_DEFAULT);
        Sleep(200);
    }
    uv_loop_close(&loop->uvLoop);
    delete loop;
    recver->m_nRunNum--;
}

/**
 * 建立一个接收udp数据的事件循环
 * @param port 监听端口
 * @param time_over 超时时间，即这么长的时间没有收到数据，认为收不到数据
 * @note 所有的uv异常判断，都不应该进入异常流程，没有考虑异常时的内存释放
 */
udp_recv_loop_t* create_udp_recv_loop(int port, int time_over, void *usr){
    SAFE_MALLOC(udp_recv_loop_t, loop);
    loop->port = port;
    loop->user = usr;

    int ret =uv_loop_init(&loop->uvLoop);
    if(ret < 0) {
        Log::error("uv_loop init error: %s", uv_strerror(ret));
        return NULL;
    }
    loop->running = true;

    // 开启udp接收
    ret = uv_udp_init(&loop->uvLoop, &loop->uvRtpSocket);
    if(ret < 0) {
        Log::error("udp init error: %s", uv_strerror(ret));
        return NULL;
    }

    struct sockaddr_in addr;
    ret = uv_ip4_addr("0.0.0.0", port, &addr);
    if(ret < 0) {
        Log::error("make address err: %s",  uv_strerror(ret));
        return NULL;
    }

    ret = uv_udp_bind(&loop->uvRtpSocket, (struct sockaddr*)&addr, 0);
    if(ret < 0) {
        Log::error("tcp bind err: %s",  uv_strerror(ret));
        return NULL;
    }

    int nRecvBuf = 10 * 1024 * 1024;       // 缓存区设置成10M，默认值太小会丢包
    setsockopt(loop->uvRtpSocket.socket, SOL_SOCKET, SO_RCVBUF, (char*)&nRecvBuf, sizeof(nRecvBuf));
    int nOverTime = 30*1000;  //
    setsockopt(loop->uvRtpSocket.socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&nOverTime, sizeof(nOverTime));
    setsockopt(loop->uvRtpSocket.socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&nOverTime, sizeof(nOverTime));

    loop->uvRtpSocket.data = (void*)loop;
    uv_udp_recv_start(&loop->uvRtpSocket, echo_alloc, after_read);
    loop->uvHandleNum++;

    //开启udp接收超时判断
    ret = uv_timer_init(&loop->uvLoop, &loop->uvTimeOver);
    if(ret < 0) {
        Log::error("timer init error: %s", uv_strerror(ret));
        return NULL;
    }

    loop->uvTimeOver.data = (void*)loop;
    ret = uv_timer_start(&loop->uvTimeOver, timer_cb, 30000, 30000);
    if(ret < 0) {
        Log::error("timer start error: %s", uv_strerror(ret));
        return NULL;
    }
    loop->uvHandleNum++;

    //异步操作
    loop->uvAsync.data = (void*)loop;
    uv_async_init(&loop->uvLoop, &loop->uvAsync, async_cb);
    loop->uvHandleNum++;

    //udp 接收loop线程
    uv_thread_t tid;
    uv_thread_create(&tid, run_loop_thread, loop);

	return loop;
}

void close_udp_recv_loop(udp_recv_loop_t *loop){
    uv_async_send(&loop->uvAsync);
}

//////////////////////////////////////////////////////////////////////////
/**
 * rtp解析事件循环，该线程负责rtp组包，解析出裸码流
 */
struct rtp_parse_loop_t {
    uv_loop_t   uvLoop;           // rtp报文解析的loop
    uv_async_t  uvAsyncRtp;       // 外部线程通知收到rtp报文
    uv_async_t  uvAsyncClose;     // 外部线程通知结束m_uvLoop
    bool        running;          // loop是否运行
    int         uvHandleNum;
    void       *user;             // 用户对象
};

static void run_rtp_parse_thread(void* arg)
{
    rtp_parse_loop_t *loop = (rtp_parse_loop_t*)arg;
    CLiveReceiver *recver = (CLiveReceiver*)loop->user;
    recver->m_nRunNum++;
    while (loop->running) {
        uv_run(&loop->uvLoop, UV_RUN_DEFAULT);
        Sleep(40);
    }
    uv_loop_close(&loop->uvLoop);
    delete loop;
    recver->m_nRunNum--;
}

/** 外部线程通知回调，在loop线程回调解析RTP数据 */
static void async_rtp_parse_cb(uv_async_t* handle){
    rtp_parse_loop_t *loop = (rtp_parse_loop_t*)handle->data;
    CLiveReceiver* h = (CLiveReceiver*)loop->user;
    h->RtpParse();
}

/** 关闭uv句柄的回调 */
static void rtp_uv_close_cb(uv_handle_t* handle){
    rtp_parse_loop_t *loop = (rtp_parse_loop_t*)handle->data;
    loop->uvHandleNum--;
    //uv句柄全部关闭后，停止loop
    if(!loop->uvHandleNum){
        loop->running = false;
        uv_stop(&loop->uvLoop);
    }
}

/** 外部线程通知回调，在loop回调中关闭所有uv句柄 */
static void async_close_cb(uv_async_t* handle){
    rtp_parse_loop_t *loop = (rtp_parse_loop_t*)handle->data;
    uv_close((uv_handle_t*)&loop->uvAsyncRtp, rtp_uv_close_cb);
    uv_close((uv_handle_t*)&loop->uvAsyncClose, rtp_uv_close_cb);
}

rtp_parse_loop_t* create_rtp_parse_loop(void *usr){
    SAFE_MALLOC(rtp_parse_loop_t, loop);
    loop->user = usr;

    int ret =uv_loop_init(&loop->uvLoop);
    if(ret < 0) {
        Log::error("uv_loop init error: %s", uv_strerror(ret));
        return NULL;
    }

    //异步操作
    loop->uvAsyncRtp.data = (void*)loop;
    uv_async_init(&loop->uvLoop, &loop->uvAsyncRtp, async_rtp_parse_cb);
    loop->uvHandleNum++;

    loop->uvAsyncClose.data = (void*)loop;
    uv_async_init(&loop->uvLoop, &loop->uvAsyncClose, async_close_cb);
    loop->uvHandleNum++;

    //rtp 解析 loop线程
    uv_thread_t tid;
	loop->running = true;
    uv_thread_create(&tid, run_rtp_parse_thread, loop);

	return loop;
}

void close_rtp_parse_loop(rtp_parse_loop_t *loop) {
    uv_async_send(&loop->uvAsyncClose);
}

void rtp_parse_event(rtp_parse_loop_t *loop) {
    uv_async_send(&loop->uvAsyncRtp);
}

//////////////////////////////////////////////////////////////////////////

CLiveReceiver::CLiveReceiver(int nPort, CLiveWorker *worker, RTP_STREAM_TYPE rst)
	: m_nLocalRTPPort(nPort)
    , m_nLocalRTCPPort(nPort+1)
	, m_pRtpParser(nullptr)
    , m_pPsParser(nullptr)
    , m_pPesParser(nullptr)
    , m_pEsParser(nullptr)
    , m_pWorker(worker)
    , m_nalu_type(unknow)
    , m_stream_type(rst)
    , m_nRunNum(0)
{
    CRtp* rtp        = new CRtp(AVCallback, this);
    rtp->SetCatchFrameNum(g_nRtpCatchPacketNum);
    rtp->SetRtpStreamType(rst);
    m_pRtpParser     = rtp;
    m_pPsParser      = new CPs(AVCallback, this);
    m_pPesParser     = new CPes(AVCallback, this);
    m_pEsParser      = new CES(AVCallback, this);

    m_pRingRtp       = create_ring_buff(sizeof(AV_BUFF), 1000, NULL);
}

CLiveReceiver::~CLiveReceiver(void)
{
    close_udp_recv_loop(m_pUdpRecv);
    close_rtp_parse_loop(m_pRtpParse);
    while (m_nRunNum)
        sleep(50);

    m_pWorker = nullptr;
    SAFE_DELETE(m_pRtpParser);
    SAFE_DELETE(m_pPsParser);
    SAFE_DELETE(m_pPesParser);
    SAFE_DELETE(m_pEsParser);

    destroy_ring_buff(m_pRingRtp);
    //Sleep(2000);
}

void CLiveReceiver::StartListen()
{
    m_pUdpRecv = create_udp_recv_loop(m_nLocalRTPPort, 30000, this);
    m_pUdpRecv->remoteIP = m_strRemoteIP;
    m_pUdpRecv->remotePort = m_nRemoteRTPPort;
   
    m_pRtpParse = create_rtp_parse_loop(this);
}

bool CLiveReceiver::RtpRecv(char* pBuff, long nLen)
{
    // 将数据保存在ring buff
    int n = (int)ring_get_count_free_elements(m_pRingRtp);
    if (!n) {
        Log::error("rtp ring buff is full");
        rtp_parse_event(m_pRtpParse);
        return false;
    }

    AV_BUFF newTag = {AV_TYPE::RTP, pBuff, nLen};
    if (!ring_insert(m_pRingRtp, &newTag, 1)) {
        Log::error("add data to rtp ring buff failed");
        return false;
    }

    rtp_parse_event(m_pRtpParse);
    return true;
}

void CLiveReceiver::RtpOverTime()
{
    Log::debug("OverTimeThread thread ID : %d", GetCurrentThreadId());
    // rtp接收超时
    if(nullptr != m_pWorker)
    {
        m_pWorker->stop();
    }
}

void CLiveReceiver::RtpParse()
{
    AV_BUFF rtp;
    while(true){
        int ret = ring_consume(m_pRingRtp, NULL, &rtp, 1);
        if(!ret) {
            return;
        }
        CRtp* rtpAnalyzer = (CRtp*)m_pRtpParser;
        rtpAnalyzer->DeCode(rtp.pData, rtp.nLen);
    }
}

void CLiveReceiver::push_ps_stream(AV_BUFF buff)
{
    //Log::debug("RTPParseCb nlen:%ld", nLen);
    CHECK_POINT_VOID(buff.pData);
	//if(m_pWorker->m_bRtp){
	//	m_pWorker->ReceiveStream(buff);
	//} 

    if(m_stream_type == RTP_STREAM_PS) {
        CPs* pPsParser = (CPs*)m_pPsParser;
        CHECK_POINT_VOID(pPsParser);
        pPsParser->DeCode(buff);
    } else if(m_stream_type == RTP_STREAM_H264) {
        push_h264_stream(buff);
    }
}

void CLiveReceiver::push_pes_stream(AV_BUFF buff)
{
    //Log::debug("PSParseCb nlen:%ld", nLen);
    CHECK_POINT_VOID(buff.pData);
    CPes* pPesParser = (CPes*)m_pPesParser;
    CHECK_POINT_VOID(pPesParser);
    pPesParser->Decode(buff);
    //需要回调TS
    //if(m_pWorker->m_bTs && nullptr != m_pTs)
    //{
    //    CTS* ts = (CTS*)m_pTs;
    //    ts->SetParam((uint8_t)m_nalu_type, m_pts);
    //    ts->Code(buff.pData, buff.nLen);
    //}
}

void CLiveReceiver::push_es_stream(AV_BUFF buff)
{
	//Log::debug("PESParseCb nlen:%ld,pts:%lld,dts:%lld", buff.nLen,pts,dts);
    CHECK_POINT_VOID(buff.pData);
    CES* pEsParser = (CES*)m_pEsParser;
    CHECK_POINT_VOID(pEsParser);
	if(buff.m_pts>0)
		m_pts = buff.m_pts;
	if(buff.m_dts>0)
		m_dts = buff.m_dts;
	pEsParser->DeCode(buff);
}

void CLiveReceiver::push_h264_stream(AV_BUFF buff)
{
    CHECK_POINT_VOID(buff.pData);
    CHECK_POINT_VOID(m_pWorker);
    m_pWorker->ReceiveStream(buff);
}
}