#include "common.h"
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
extern int  g_nRtpCatchPacketNum;  //< rtp����İ�������

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
 * udp�����¼�ѭ��
 */
struct udp_recv_loop_t {
    uv_loop_t   uvLoop;           // udp���յ�loop
    uv_udp_t    uvRtpSocket;      // rtp����
    uv_timer_t  uvTimeOver;       // ���ճ�ʱ��ʱ��
    uv_async_t  uvAsync;          // �ⲿ�߳�֪ͨ����loop
    bool        running;          // loop�߳��Ƿ�����ִ��
    int         uvHandleNum;      // ������uv���������Щ���ͨ��uv_close�ر�
    string      remoteIP;         // ���ͷ�IP
    int         remotePort;       // ���ͷ��˿�
    int         port;
    void       *user;             // �û�����
};

/** udp�������뻺��ռ� */
static void echo_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    UNUSED(handle);
    UNUSED(suggested_size);
    *buf = uv_buf_init((char*)malloc(PACK_MAX_SIZE), PACK_MAX_SIZE);
}

/** udp�������ݣ���ȡһ���� */
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

    //udp��Դ��ƥ�䣬����������
    struct sockaddr_in* addr_in =(struct sockaddr_in*)addr;
    int port = ntohs(addr_in->sin_port);
    char ipv4addr[64]={0};
    uv_ip4_name(addr_in, ipv4addr, 64);
    string ip = ipv4addr;
    if(loop->remotePort != port || loop->remoteIP != ip) {
        //Log::error("this is not my rtp data");
        return;
    }

    //���ó�ʱ��ʱ��
    int ret = uv_timer_again(&loop->uvTimeOver);
    if(ret < 0) {
        Log::error("timer again error: %s", uv_strerror(ret));
        return;
    }

    CLiveReceiver* pLive = (CLiveReceiver*)loop->user;
    pLive->RtpRecv(buf->base, nread);
}

/** ��ʱ��ʱ����ʱ�ص� */
static void timer_cb(uv_timer_t* handle)
{
    udp_recv_loop_t *loop = (udp_recv_loop_t*)handle->data;
    CLiveReceiver* pLive = (CLiveReceiver*)loop->user;
    pLive->RtpOverTime();
}

/** �ر�uv����Ļص� */
static void udp_uv_close_cb(uv_handle_t* handle){
    udp_recv_loop_t *loop = (udp_recv_loop_t*)handle->data;
    loop->uvHandleNum--;
    //uv���ȫ���رպ�ֹͣloop
    if(!loop->uvHandleNum){
        loop->running = false;
        uv_stop(&loop->uvLoop);
    }
}

/** �ⲿ�߳�֪ͨ�ص�����loop�ص��йر��õ���uv��� */
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
 * ����һ������udp���ݵ��¼�ѭ��
 * @param port �����˿�
 * @param time_over ��ʱʱ�䣬����ô����ʱ��û���յ����ݣ���Ϊ�ղ�������
 * @note ���е�uv�쳣�жϣ�����Ӧ�ý����쳣���̣�û�п����쳣ʱ���ڴ��ͷ�
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

    // ����udp����
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

    int nRecvBuf = 10 * 1024 * 1024;       // ���������ó�10M��Ĭ��ֵ̫С�ᶪ��
    setsockopt(loop->uvRtpSocket.socket, SOL_SOCKET, SO_RCVBUF, (char*)&nRecvBuf, sizeof(nRecvBuf));
    int nOverTime = 30*1000;  //
    setsockopt(loop->uvRtpSocket.socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&nOverTime, sizeof(nOverTime));
    setsockopt(loop->uvRtpSocket.socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&nOverTime, sizeof(nOverTime));

    loop->uvRtpSocket.data = (void*)loop;
    uv_udp_recv_start(&loop->uvRtpSocket, echo_alloc, after_read);
    loop->uvHandleNum++;

    //����udp���ճ�ʱ�ж�
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

    //�첽����
    loop->uvAsync.data = (void*)loop;
    uv_async_init(&loop->uvLoop, &loop->uvAsync, async_cb);
    loop->uvHandleNum++;

    //udp ����loop�߳�
    uv_thread_t tid;
    uv_thread_create(&tid, run_loop_thread, loop);

	return loop;
}

void close_udp_recv_loop(udp_recv_loop_t *loop){
    uv_async_send(&loop->uvAsync);
}

//////////////////////////////////////////////////////////////////////////
/**
 * rtp�����¼�ѭ�������̸߳���rtp�����������������
 */
struct rtp_parse_loop_t {
    uv_loop_t   uvLoop;           // rtp���Ľ�����loop
    uv_async_t  uvAsyncRtp;       // �ⲿ�߳�֪ͨ�յ�rtp����
    uv_async_t  uvAsyncClose;     // �ⲿ�߳�֪ͨ����m_uvLoop
    bool        running;          // loop�Ƿ�����
    int         uvHandleNum;
    void       *user;             // �û�����
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

/** �ⲿ�߳�֪ͨ�ص�����loop�̻߳ص�����RTP���� */
static void async_rtp_parse_cb(uv_async_t* handle){
    rtp_parse_loop_t *loop = (rtp_parse_loop_t*)handle->data;
    CLiveReceiver* h = (CLiveReceiver*)loop->user;
    h->RtpParse();
}

/** �ر�uv����Ļص� */
static void rtp_uv_close_cb(uv_handle_t* handle){
    rtp_parse_loop_t *loop = (rtp_parse_loop_t*)handle->data;
    loop->uvHandleNum--;
    //uv���ȫ���رպ�ֹͣloop
    if(!loop->uvHandleNum){
        loop->running = false;
        uv_stop(&loop->uvLoop);
    }
}

/** �ⲿ�߳�֪ͨ�ص�����loop�ص��йر�����uv��� */
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

    //�첽����
    loop->uvAsyncRtp.data = (void*)loop;
    uv_async_init(&loop->uvLoop, &loop->uvAsyncRtp, async_rtp_parse_cb);
    loop->uvHandleNum++;

    loop->uvAsyncClose.data = (void*)loop;
    uv_async_init(&loop->uvLoop, &loop->uvAsyncClose, async_close_cb);
    loop->uvHandleNum++;

    //rtp ���� loop�߳�
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
    // �����ݱ�����ring buff
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
    // rtp���ճ�ʱ
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
    //��Ҫ�ص�TS
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