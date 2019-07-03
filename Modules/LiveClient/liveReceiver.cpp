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

static void echo_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    UNUSED(handle);
    UNUSED(suggested_size);
    *buf = uv_buf_init((char*)malloc(PACK_MAX_SIZE), PACK_MAX_SIZE);
}

static void after_read(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)
{
    //Log::debug("after_read thread ID : %d", GetCurrentThreadId());
    UNUSED(flags);
    if(nread < 0){
        Log::error("read error: %s",uv_strerror(nread));
        free(buf->base);
    }
	if(nread == 0)
		return;

    CLiveReceiver* pLive = (CLiveReceiver*)handle->data;
    pLive->RtpRecv(buf->base, nread, (struct sockaddr_in*)addr);
}

static void timer_cb(uv_timer_t* handle)
{
    CLiveReceiver* pLive = (CLiveReceiver*)handle->data;
    pLive->RtpOverTime();
}

static void rtp_udp_close_cb(uv_handle_t* handle){
    CLiveReceiver* pLive = (CLiveReceiver*)handle->data;
    pLive->m_bRtpRun = false;

	if(!pLive->m_bTimeOverRun){
		pLive->m_bRun = false;
		uv_stop(pLive->m_uvLoop);
	}
}

static void timer_over_close_cb(uv_handle_t* handle){
    CLiveReceiver* pLive = (CLiveReceiver*)handle->data;
    pLive->m_bTimeOverRun = false;

	if(!pLive->m_bRtpRun){
		pLive->m_bRun = false;
		uv_stop(pLive->m_uvLoop);
	}
}

static void async_cb(uv_async_t* handle){
    CLiveReceiver* obj = (CLiveReceiver*)handle->data;
    obj->AsyncClose();
}

static void run_loop_thread(void* arg)
{
    CLiveReceiver* h = (CLiveReceiver*)arg;
    while (h->m_bRun) {
        uv_run(h->m_uvLoop, UV_RUN_DEFAULT);
        Sleep(200);
    }
    uv_loop_close(h->m_uvLoop);
    h->m_uvLoop = NULL;
}

static void rtp_parse_thread(void* arg)
{
    CLiveReceiver* h = (CLiveReceiver*)arg;
    h->RtpParse();
}

static void destroy_ring_node(void *_msg)
{
    AV_BUFF *msg = (AV_BUFF*)_msg;
    free(msg->pData);
    msg->pData = NULL;
    msg->nLen = 0;
}


CLiveReceiver::CLiveReceiver(int nPort, CLiveWorker *worker)
	: m_nLocalRTPPort(nPort)
    , m_nLocalRTCPPort(nPort+1)
    , m_uvLoop(nullptr)
    , m_bRun(false)
    , m_bRtpRun(false)
    , m_bTimeOverRun(false)
	, m_pRtpParser(nullptr)
    , m_pPsParser(nullptr)
    , m_pPesParser(nullptr)
    , m_pEsParser(nullptr)
    , m_pWorker(worker)
    , m_nalu_type(unknow)
{
    CRtp* rtp        = new CRtp(AVCallback, this);
    rtp->SetCatchFrameNum(g_nRtpCatchPacketNum);
    m_pRtpParser     = rtp;
    m_pPsParser      = new CPs(AVCallback, this);
    m_pPesParser     = new CPes(AVCallback, this);
    m_pEsParser      = new CES(AVCallback, this);

    m_pRingRtp       = create_ring_buff(sizeof(AV_BUFF), 1000, NULL);
}

CLiveReceiver::~CLiveReceiver(void)
{
    uv_async_send(&m_uvAsync);
    while (m_uvLoop)
        Sleep(500);

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
    m_uvLoop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
    uv_loop_init(m_uvLoop);
    m_bRun = true;

    // 开启udp接收
    int ret = uv_udp_init(m_uvLoop, &m_uvRtpSocket);
    if(ret < 0) {
        Log::error("udp init error: %s", uv_strerror(ret));
        return;
    }

    struct sockaddr_in addr;
    ret = uv_ip4_addr("0.0.0.0", m_nLocalRTPPort, &addr);
    if(ret < 0) {
        Log::error("make address err: %s",  uv_strerror(ret));
        return ;
    }

    ret = uv_udp_bind(&m_uvRtpSocket, (struct sockaddr*)&addr, 0);
    if(ret < 0) {
        Log::error("tcp bind err: %s",  uv_strerror(ret));
        return;
    }

	int nRecvBuf = 10 * 1024 * 1024;       // 缓存区设置成10M，默认值太小会丢包
	setsockopt(m_uvRtpSocket.socket, SOL_SOCKET, SO_RCVBUF, (char*)&nRecvBuf, sizeof(nRecvBuf));
	int nOverTime = 30*1000;  //
	setsockopt(m_uvRtpSocket.socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&nOverTime, sizeof(nOverTime));
	setsockopt(m_uvRtpSocket.socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&nOverTime, sizeof(nOverTime));

    m_uvRtpSocket.data = (void*)this;
    uv_udp_recv_start(&m_uvRtpSocket, echo_alloc, after_read);
    m_bRtpRun = true;

    //开启udp接收超时判断
    ret = uv_timer_init(m_uvLoop, &m_uvTimeOver);
    if(ret < 0) {
        Log::error("timer init error: %s", uv_strerror(ret));
        return;
    }

    m_uvTimeOver.data = (void*)this;
    ret = uv_timer_start(&m_uvTimeOver, timer_cb, 30000, 30000);
    if(ret < 0) {
        Log::error("timer start error: %s", uv_strerror(ret));
        return;
    }
    m_bTimeOverRun = true;

    //异步操作
    m_uvAsync.data = (void*)this;
    uv_async_init(m_uvLoop, &m_uvAsync, async_cb);

    //udp 接收loop线程
    uv_thread_t tid;
    uv_thread_create(&tid, run_loop_thread, this);

    //rtp数据解析线程
    uv_thread_create(&tid, rtp_parse_thread, this);
}

bool CLiveReceiver::RtpRecv(char* pBuff, long nLen, struct sockaddr_in* addr_in)
{
    //udp来源不匹配，将数据抛弃
    char ipv4addr[64]={0};
    uv_ip4_name(addr_in, ipv4addr, 64);
    int port = ntohs(addr_in->sin_port);
    string ip = ipv4addr;
    if(m_nRemoteRTPPort != port || m_strRemoteIP != ip) {
        //Log::error("this is not my rtp data");
        return false;
    }

    //重置超时计时器
    int ret = uv_timer_again(&m_uvTimeOver);
    if(ret < 0) {
        Log::error("timer again error: %s", uv_strerror(ret));
        return false;
    }

    // 将数据保存在ring buff
    int n = (int)ring_get_count_free_elements(m_pRingRtp);
    if (!n) {
        Log::error("rtp ring buff is full");
        return false;
    }

    AV_BUFF newTag = {AV_TYPE::RTP, pBuff, nLen};
    if (!ring_insert(m_pRingRtp, &newTag, 1)) {
        Log::error("add data to rtp ring buff failed");
        return false;
    }

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
    while(m_bRtpRun){
        int ret = ring_consume(m_pRingRtp, NULL, &rtp, 1);
        if(!ret) {
            Sleep(10);
            continue;
        }
        CRtp* rtpAnalyzer = (CRtp*)m_pRtpParser;
        rtpAnalyzer->DeCode(rtp.pData, rtp.nLen);
    }
}

void CLiveReceiver::push_ps_stream(AV_BUFF buff)
{
    //Log::debug("RTPParseCb nlen:%ld", nLen);
    CHECK_POINT_VOID(buff.pData);
	if(m_pWorker->m_bRtp){
		m_pWorker->ReceiveStream(buff);
	} 
	{
		if(g_stream_type == STREAM_PS) {
			CPs* pPsParser = (CPs*)m_pPsParser;
			CHECK_POINT_VOID(pPsParser)
			pPsParser->DeCode(buff);
		} else if(g_stream_type == STREAM_H264) {
			push_h264_stream(buff);
		}
	}
}

void CLiveReceiver::push_pes_stream(AV_BUFF buff)
{
    //Log::debug("PSParseCb nlen:%ld", nLen);
    CHECK_POINT_VOID(buff.pData)
	/*if(m_pWorker->m_bFlv || m_pWorker->m_bMp4)*/ {
		CPes* pPesParser = (CPes*)m_pPesParser;
		CHECK_POINT_VOID(pPesParser)
		pPesParser->Decode(buff);
	}
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

void CLiveReceiver::AsyncClose()
{
    int ret = uv_udp_recv_stop(&m_uvRtpSocket);
    if(ret < 0) {
        Log::error("stop rtp recv port:%d err: %s", m_nLocalRTPPort, uv_strerror(ret));
    }
    uv_close((uv_handle_t*)&m_uvRtpSocket, rtp_udp_close_cb);

    ret = uv_timer_stop(&m_uvTimeOver);
    if(ret < 0) {
        Log::error("stop timer error: %s",uv_strerror(ret));
    }
    uv_close((uv_handle_t*)&m_uvTimeOver, timer_over_close_cb);

}
}