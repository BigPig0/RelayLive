#include "stdafx.h"
#include "LiveChannel.h"

namespace LiveClient
{
    extern int  g_nNodelay;

static void H264SpsCbfun(uint32_t nWidth, uint32_t nHeight, double fFps, void* pUser){
    CLiveChannel* pLive = (CLiveChannel*)pUser;
    pLive->set_h264_param(nWidth, nHeight, fFps);
}

static void AVCallback(AV_BUFF buff, void* pUser){
    CLiveChannel* pLive = (CLiveChannel*)pUser;
    switch (buff.eType)
    {
    case AV_TYPE::H264_IDR:
    case AV_TYPE::H264_NDR:
        pLive->H264Cb(buff);
        break;
    case AV_TYPE::FLV_HEAD:
    case AV_TYPE::FLV_FRAG_KEY:
    case AV_TYPE::FLV_FRAG:
        pLive->FlvCb(buff);
        break;
    case AV_TYPE::MP4_HEAD:
    case AV_TYPE::MP4_FRAG_KEY:
    case AV_TYPE::MP4_FRAG:
        pLive->Mp4Cb(buff);
        break;
    case AV_TYPE::TS:
        pLive->TsCb(buff);
        break;
    default:
        break;
    }
}

CLiveChannel::CLiveChannel()
    : m_nChannel(0)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_bFlv(false)
    , m_bMp4(false)
    , m_bH264(false)
    , m_bTs(false)
    , m_bRtp(false)
{
    Init();
}

CLiveChannel::CLiveChannel(int channel, uint32_t w, uint32_t h)
    : m_nChannel(channel)
    , m_nWidth(w)
    , m_nHeight(h)
    , m_bFlv(false)
    , m_bMp4(false)
    , m_bH264(false)
    , m_bTs(false)
    , m_bRtp(false)
{
    Init();
}

void CLiveChannel::Init()
{
    memset(&m_stFlvHead, 0, sizeof(m_stFlvHead));
    memset(&m_stMp4Head, 0, sizeof(m_stMp4Head));

    m_pH264          = new CH264(H264SpsCbfun, AVCallback, this);
    m_pTs            = new CTS(AVCallback, this);
    m_pFlv           = new CFlv(AVCallback, this);
    m_pMp4           = new CMP4(AVCallback, this);

    m_pFlv->SetNodelay(g_nNodelay);
    m_pMp4->SetNodelay(g_nNodelay);
}

CLiveChannel::~CLiveChannel()
{

}

bool CLiveChannel::AddHandle(ILiveHandle* h, HandleType t)
{
    if(t == HandleType::flv_handle) {
        m_bFlv = true;
        MutexLock lock(&m_csFlv);
        m_vecLiveFlv.push_back(h);
    } else if(t == HandleType::fmp4_handle) {
        m_bMp4 = true;
        MutexLock lock(&m_csMp4);
        m_vecLiveMp4.push_back(h);
    } else if(t == HandleType::h264_handle) {
        m_bH264 = true;
        MutexLock lock(&m_csH264);
        m_vecLiveH264.push_back(h);
    } else if(t == HandleType::ts_handle) {
        m_bTs = true;
        MutexLock lock(&m_csTs);
        m_vecLiveTs.push_back(h);
    } else if(t == HandleType::rtp_handle) {
        m_bRtp = true;
        MutexLock lock(&m_csRtp);
        m_vecLiveRtp.push_back((ILiveHandleRtp*)h);
    }

    return true;
}

bool CLiveChannel::RemoveHandle(ILiveHandle* h)
{
    bool bFind = false;
    do {
        //移除的是否是flv
        for(auto it = m_vecLiveFlv.begin(); it != m_vecLiveFlv.end(); it++) {
            if(*it == h) {
                m_vecLiveFlv.erase(it);
                bFind = true;
                break;
            }
        }
        if(bFind) {
            if(m_vecLiveFlv.empty())
                m_bFlv = false;
            break;
        }

        //移除的是否是Mp4
        for(auto it = m_vecLiveMp4.begin(); it != m_vecLiveMp4.end(); it++) {
            if(*it == h) {
                m_vecLiveMp4.erase(it);
                bFind = true;
                break;
            }
        }
        if(bFind) {
            if(m_vecLiveMp4.empty())
                m_bMp4 = false;
            break;
        }

        //移除的是否是h264
        for(auto it = m_vecLiveH264.begin(); it != m_vecLiveH264.end(); it++) {
            if(*it == h) {
                m_vecLiveH264.erase(it);
                bFind = true;
                break;
            }
        }
        if(bFind) {
            if(m_vecLiveH264.empty())
                m_bH264 = false;
            break;
        }

        //移除的是否是HLS
        for(auto it = m_vecLiveTs.begin(); it != m_vecLiveTs.end(); it++) {
            if(*it == h) {
                m_vecLiveTs.erase(it);
                bFind = true;
                break;
            }
        }
        if(bFind) {
            if(m_vecLiveTs.empty())
                m_bTs = false;
            break;
        }

        //移除的是否是RTSP
        for(auto it = m_vecLiveRtp.begin(); it != m_vecLiveRtp.end(); it++) {
            if(*it == h) {
                m_vecLiveRtp.erase(it);
                bFind = true;
                break;
            }
        }
        if(bFind) {
            if(m_vecLiveRtp.empty())
                m_bRtp = false;
            break;
        }
    }while(0);

    return m_vecLiveFlv.empty() && m_vecLiveMp4.empty() && m_vecLiveH264.empty()
        && m_vecLiveTs.empty() && m_vecLiveRtp.empty();
}

void CLiveChannel::ReceiveStream(AV_BUFF buff)
{
    if(m_nChannel == 0 && buff.eType == AV_TYPE::H264_NALU) {

    } else if (buff.eType == AV_TYPE::YUV) {

    }
}

void CLiveChannel::push_h264_stream(AV_BUFF buff)
{
    CHECK_POINT_VOID(buff.pData);
    //Log::debug("ESParseCb nlen:%ld, buff:%02X %02X %02X %02X %02X", buff.nLen,buff.pData[0],buff.pData[1],buff.pData[2],buff.pData[3],buff.pData[4]);
    CH264* pH264 = (CH264*)m_pH264;
    pH264->InputBuffer(buff.pData, buff.nLen);
    NalType m_nalu_type = pH264->NaluType();
    uint32_t nDataLen = 0;
    char* pData = pH264->DataBuff(nDataLen);


    //需要回调Flv
    if(m_bFlv && nullptr != m_pFlv)
    {
        CFlv* flv = (CFlv*)m_pFlv;
        flv->Code(m_nalu_type, pData, nDataLen);
    }
    
    //需要回调mp4
    if (m_bMp4 && nullptr != m_pMp4)
    {
        CMP4* mp4 = (CMP4*)m_pMp4;
        mp4->Code(m_nalu_type, pData, nDataLen);
    }

}

bool CLiveChannel::Empty()
{
    return m_vecLiveFlv.empty() && m_vecLiveMp4.empty() && m_vecLiveH264.empty()
        && m_vecLiveTs.empty() && m_vecLiveRtp.empty();
}

AV_BUFF CLiveChannel::GetHeader(HandleType t)
{
    if(t == HandleType::flv_handle) {
        return m_stFlvHead;
    }
    else if(t == HandleType::fmp4_handle)
        return m_stMp4Head;

    AV_BUFF ret = {AV_TYPE::NONE, NULL, 0};
    return ret;
}

/** 获取客户端信息 */
string CLiveChannel::GetClientInfo()
{
    string strResJson;
    {
        MutexLock lock(&m_csFlv);
        for(auto h : m_vecLiveFlv){
            strResJson += h->get_clients_info();
            strResJson += ",";
        }
    }
    {
        MutexLock lock(&m_csMp4);
        for(auto h : m_vecLiveMp4){
            strResJson += h->get_clients_info();
            strResJson += ",";
        }
    }
    {
        MutexLock lock(&m_csH264);
        for(auto h : m_vecLiveH264){
            strResJson += h->get_clients_info();
            strResJson += ",";
        }
    }
    {
        MutexLock lock(&m_csTs);
        for(auto h : m_vecLiveTs){
            strResJson += h->get_clients_info();
            strResJson += ",";
        }
    }
    {
        MutexLock lock(&m_csRtp);
        for(auto h : m_vecLiveRtp){
            strResJson += h->get_clients_info();
            strResJson += ",";
        }
    }
    return strResJson;
}



void CLiveChannel::FlvCb(AV_BUFF buff)
{
    if (buff.eType == FLV_HEAD) {
        m_stFlvHead.pData = buff.pData;
        m_stFlvHead.nLen = buff.nLen;
        Log::debug("flv head ok");
    } else {
        MutexLock lock(&m_csFlv);
        for (auto h : m_vecLiveFlv)
        {
            //Log::debug("flv frag ok");
            h->push_video_stream(buff);
        }   
    }
}

void CLiveChannel::H264Cb(AV_BUFF buff)
{
    MutexLock lock(&m_csH264);
    for (auto h : m_vecLiveH264)
    {
        h->push_video_stream(buff);
    } 
}

void CLiveChannel::TsCb(AV_BUFF buff)
{
    MutexLock lock(&m_csTs);
    for (auto h : m_vecLiveTs)
    {
        h->push_video_stream(buff);
    } 
}

void CLiveChannel::Mp4Cb(AV_BUFF buff)
{
    if(buff.eType == MP4_HEAD) {
        m_stMp4Head.pData = buff.pData;
        m_stMp4Head.nLen = buff.nLen;
        Log::debug("MP4 Head ok");
    } else {
        MutexLock lock(&m_csMp4);
        for (auto h : m_vecLiveMp4)
        {
            h->push_video_stream(buff);
        }
    }
}

void CLiveChannel::RtpCb(AV_BUFF buff)
{
    MutexLock lock(&m_csRtp);
    for (auto h : m_vecLiveRtp)
    {
        h->push_video_stream(buff);
    } 
}

void CLiveChannel::RtcpCb(AV_BUFF buff)
{
    MutexLock lock(&m_csRtp);
    for (auto h : m_vecLiveRtp)
    {
        h->push_rtcp_stream(buff.pData, buff.nLen);
    } 
}

void CLiveChannel::stop()
{
    for (auto h : m_vecLiveFlv) {
        h->stop();
    }
    for (auto h : m_vecLiveMp4) {
        h->stop();
    }
    for (auto h : m_vecLiveH264) {
        h->stop();
    }
    for (auto h : m_vecLiveTs) {
        h->stop();
    }
    for (auto h : m_vecLiveRtp) {
        h->stop();
    }
}

void CLiveChannel::set_h264_param(uint32_t nWidth, uint32_t nHeight, double fFps)
{
    Log::debug("H264SpsCb width:%d, height:%d, fps:%lf", nWidth, nHeight, fFps);
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
}