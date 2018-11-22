#include "stdafx.h"
#include "uv.h"
#include "liveObj.h"
#include "rtp.h"
#include "ps.h"
#include "pes.h"
#include "es.h"
#include "ts.h"
#include "flv.h"
#include "mp4.h"
#include "h264.h"

IlibLive* IlibLive::CreateObj()
{
    return new CLiveObj;
}

static void echo_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    *buf = uv_buf_init((char*)malloc(PACK_MAX_SIZE), PACK_MAX_SIZE);
}

static void after_read(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)
{
    //Log::debug("after_read thread ID : %d", GetCurrentThreadId());
    if(nread < 0){
        Log::error("read error: %s",uv_strerror(nread));
        free(buf->base);
    }
	if(nread == 0)
		return;

    CLiveObj* pLive = (CLiveObj*)handle->data;
    pLive->RtpRecv(buf->base, nread);
}

static void timer_cb(uv_timer_t* handle)
{
    CLiveObj* pLive = (CLiveObj*)handle->data;
    pLive->RtpOverTime();
}

CLiveObj::CLiveObj(void)
    : m_pRtpParser(nullptr)
    , m_pPsParser(nullptr)
    , m_pPesParser(nullptr)
    , m_pEsParser(nullptr)
    , m_pTs(nullptr)
    , m_pFlv(nullptr)
    , m_pCallBack(nullptr)
    //, m_pRtpBuff(nullptr)
    //, m_nRtpLen(0)
    //, m_pPsBuff(nullptr)
    //, m_nPsLen(0)
    //, m_pPesBuff(nullptr)
    //, m_nPesLen(0)
    //, m_pEsBuff(nullptr)
    //, m_nEsLen(0)
    //, m_pNaluBuff(nullptr)
    //, m_nNaluLen(0)
    , m_pts(0)
    , m_dts(0)
    , m_nalu_type(unknow)
{
    m_pRtpParser     = new CRtp(this);
    m_pPsParser      = new CPs(this);
    m_pPesParser     = new CPes(this);
    m_pEsParser      = new CES(this);
    m_pH264          = new CH264(this);
    m_pTs            = new CTS(this);
    m_pFlv           = new CFlv(this);
    m_pMp4           = new CMP4(this);
}

CLiveObj::~CLiveObj(void)
{
    m_pCallBack = nullptr;
    int ret = uv_udp_recv_stop(&m_uvRtpSocket);
    if(ret < 0) {
        Log::error("stop rtp recv port:%d err: %s", m_nLocalRTPPort, uv_strerror(ret));
    }
    ret = uv_timer_stop(&m_uvTimeOver);
    if(ret < 0) {
        Log::error("stop timer error: %s",uv_strerror(ret));
    }

    SAFE_DELETE(m_pRtpParser);
    SAFE_DELETE(m_pPsParser);
    SAFE_DELETE(m_pPesParser);
    SAFE_DELETE(m_pEsParser);
    SAFE_DELETE(m_pH264);
    SAFE_DELETE(m_pTs);
    SAFE_DELETE(m_pFlv);
    SAFE_DELETE(m_pMp4);
    Sleep(2000);
}

void CLiveObj::SetCatchPacketNum(int nPacketNum)
{
    CRtp* rtpAnalyzer = (CRtp*)m_pRtpParser;
    rtpAnalyzer->SetCatchFrameNum(nPacketNum);
}

void CLiveObj::StartListen()
{
    // 开启udp接收
    int ret = uv_udp_init(uv_default_loop(), &m_uvRtpSocket);
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

    //开启udp接收超时判断
    ret = uv_timer_init(uv_default_loop(), &m_uvTimeOver);
    if(ret < 0) {
        Log::error("timer init error: %s", uv_strerror(ret));
        return;
    }

    m_uvTimeOver.data = (void*)this;
    ret = uv_timer_start(&m_uvTimeOver, timer_cb,30000, 30000);
    if(ret < 0) {
        Log::error("timer start error: %s", uv_strerror(ret));
        return;
    }
}

void CLiveObj::RtpRecv(char* pBuff, long nLen)
{
    int ret = uv_timer_again(&m_uvTimeOver);
    if(ret < 0) {
        Log::error("timer again error: %s", uv_strerror(ret));
        return;
    }
    CRtp* rtpAnalyzer = (CRtp*)m_pRtpParser;
    rtpAnalyzer->InputBuffer(pBuff, nLen);
}

void CLiveObj::RtpOverTime()
{
    Log::debug("OverTimeThread thread ID : %d", GetCurrentThreadId());
    time_t nowTime = time(NULL);
    // rtp接收超时
    if(nullptr != m_pCallBack)
    {
        m_pCallBack->stop();
    }
}

void CLiveObj::RTPParseCb(char* pBuff, long nLen)
{
    //Log::debug("CRTSPInterface::RTPParseCb nlen:%ld", nLen);
    CHECK_POINT_VOID(pBuff)
    CPs* pPsParser = (CPs*)m_pPsParser;
    CHECK_POINT_VOID(pPsParser)
    //m_pPsBuff = pBuff;
    //m_nPsLen  = nLen;
    pPsParser->InputBuffer(pBuff, nLen);
}

void CLiveObj::PSParseCb(char* pBuff, long nLen)
{
    //Log::debug("CRTSPInterface::PSParseCb nlen:%ld", nLen);
    CHECK_POINT_VOID(pBuff)
    CPes* pPesParser = (CPes*)m_pPesParser;
    CHECK_POINT_VOID(pPesParser)
    //m_pPesBuff = pBuff;
    //m_nPesLen  = nLen;
    pPesParser->InputBuffer(pBuff, nLen);
    //需要回调TS
    if(m_pCallBack->m_bTs && nullptr != m_pTs)
    {
        CTS* ts = (CTS*)m_pTs;
        ts->SetParam(m_nalu_type, m_pts);
        ts->InputBuffer(pBuff, nLen);
    }
}

void CLiveObj::PESParseCb(char* pBuff, long nLen, uint64_t pts, uint64_t dts)
{
    //Log::debug("CRTSPInterface::PESParseCb nlen:%ld,pts:%lld,dts:%lld", nLen,pts,dts);
    CHECK_POINT_VOID(pBuff)
    CES* pEsParser = (CES*)m_pEsParser;
    CHECK_POINT_VOID(pEsParser)
    //m_pEsBuff = pBuff;
    //m_nEsLen  = nLen;
    m_pts = pts;
    m_dts = dts;
	pEsParser->InputBuffer(pBuff, nLen);
}

void CLiveObj::ESParseCb(char* pBuff, long nLen/*, uint8_t nNalType*/)
{
    //nal_unit_header4* nalu = (nal_unit_header4*)pBuff;
    //Log::debug("CLiveInstance::ESParseCb nlen:%ld, buff:%02X %02X %02X %02X %02X", nLen,pBuff[0],pBuff[1],pBuff[2],pBuff[3],pBuff[4]);
    CHECK_POINT_VOID(pBuff);
    CH264* pH264 = (CH264*)m_pH264;
    //m_pNaluBuff = pBuff;
    //m_nNaluLen  = nLen;
    pH264->InputBuffer(pBuff, nLen);
    m_nalu_type = pH264->NaluType();
    uint32_t nDataLen = 0;
    char* pData = pH264->DataBuff(nDataLen);

    CHECK_POINT_VOID(m_pCallBack);

    //需要回调Flv
    if(m_pCallBack->m_bFlv && nullptr != m_pFlv)
    {
        CFlv* flv = (CFlv*)m_pFlv;
        flv->InputBuffer(m_nalu_type, pData, nDataLen);
    }

    //需要回调mp4
    if (m_pCallBack->m_bMp4 && nullptr != m_pMp4)
    {
        CMP4* mp4 = (CMP4*)m_pMp4;
        mp4->InputBuffer(m_nalu_type, pData, nDataLen);
    }
}

void CLiveObj::H264SpsCb(uint32_t nWidth, uint32_t nHeight, double fFps)
{
    if(nullptr != m_pFlv)
    {
        CFlv* flv = (CFlv*)m_pFlv;
        flv->SetSps(nWidth,nHeight,fFps);
    }
    if (nullptr != m_pMp4)
    {
        CMP4* mp4 = (CMP4*)m_pMp4;
        mp4->SetSps(nWidth,nHeight,fFps);
    }
}

void CLiveObj::FlvCb(FLV_FRAG_TYPE eType, char* pBuff, int nBuffSize)
{
    CHECK_POINT_VOID(m_pCallBack);
    m_pCallBack->push_flv_frame(eType, pBuff, nBuffSize);
}

void CLiveObj::Mp4Cb(MP4_FRAG_TYPE eType, char* pBuff, int nBuffSize)
{
    CHECK_POINT_VOID(m_pCallBack);
    m_pCallBack->push_mp4_stream(eType, pBuff, nBuffSize);
}

void CLiveObj::TsCb(char* pBuff, int nBuffSize)
{
    CHECK_POINT_VOID(m_pCallBack);
    m_pCallBack->push_ts_stream(pBuff, nBuffSize);
}

void CLiveObj::H264Cb(char* pBuff, int nBuffSize)
{
    CHECK_POINT_VOID(m_pCallBack);
    m_pCallBack->push_h264_stream(pBuff, nBuffSize);
}