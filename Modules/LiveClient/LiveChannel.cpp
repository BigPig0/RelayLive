#include "stdafx.h"
#include "LiveChannel.h"

namespace LiveClient
{
    extern int  g_nNodelay;

static void AVCallback(AV_BUFF buff, void* pUser){
    CLiveChannel* pLive = (CLiveChannel*)pUser;
    switch (buff.eType)
    {
    case AV_TYPE::H264_NALU:
        pLive->ReceiveStream(buff);
        break;
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

    m_pH264          = new CH264(AVCallback, this);
    m_pTs            = new CTS(AVCallback, this);
    m_pFlv           = new CFlv(AVCallback, this);
    m_pMp4           = new CMP4(AVCallback, this);

    m_pFlv->SetNodelay(g_nNodelay);
    m_pMp4->SetNodelay(g_nNodelay);

#ifdef USE_FFMPEG
    m_pEncoder       = IEncoder::Create(AVCallback, this);
	m_pEncoder->m_width = m_nWidth;
	m_pEncoder->m_height = m_nHeight;
#endif
}

CLiveChannel::~CLiveChannel()
{
    SAFE_DELETE(m_pH264);
    SAFE_DELETE(m_pTs);
    SAFE_DELETE(m_pFlv);
    SAFE_DELETE(m_pMp4);
#ifdef USE_FFMPEG
    SAFE_DELETE(m_pEncoder);
#endif
}

#ifdef USE_FFMPEG
void CLiveChannel::SetDecoder(IDecoder *decoder)
{
    m_pEncoder->SetDecoder(decoder);
}
#endif

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
    if(buff.eType == AV_TYPE::H264_NALU) {
        push_h264_stream(buff);
    } else if (buff.eType == AV_TYPE::YUV) {
#ifdef USE_FFMPEG
        m_pEncoder->Code(buff);
#endif
    }
}

void CLiveChannel::push_h264_stream(AV_BUFF buff)
{
    CHECK_POINT_VOID(buff.pData);
    //Log::debug("ESParseCb nlen:%ld, buff:%02X %02X %02X %02X %02X", buff.nLen,buff.pData[0],buff.pData[1],buff.pData[2],buff.pData[3],buff.pData[4]);

    char* pData;
    uint32_t nDataLen = 0;
    h264_nalu_data2(buff.pData, buff.nLen, &pData, &nDataLen);
    NalType nalu_type = h264_naltype(pData);

    if(nalu_type == sps_Nal && m_nWidth==0){
        double fps;
        h264_sps_info(pData, nDataLen, &m_nWidth, &m_nHeight, &fps);
        set_h264_param(m_nWidth, m_nHeight, fps);
    }

    //需要回调Flv
    if(m_bFlv && nullptr != m_pFlv)
    {
        m_pFlv->Code(nalu_type, pData, nDataLen);
    }
    
    //需要回调mp4
    if(m_bMp4 && nullptr != m_pMp4)
    {
        Mp4Cb(buff);
    }

    //需要回调h264
    if (m_bH264 && nullptr != m_pH264)
    {
        m_pH264->Code(buff.pData, buff.nLen);
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
vector<ClientInfo> CLiveChannel::GetClientInfo()
{
    vector<ClientInfo> ret;
    {
        MutexLock lock(&m_csFlv);
        for(auto h : m_vecLiveFlv){
            vector<ClientInfo> tmp = h->get_clients_info();
            for(auto &c:tmp){
                c.channel = m_nChannel;
                ret.push_back(c);
            }
        }
    }
    {
        MutexLock lock(&m_csMp4);
        for(auto h : m_vecLiveMp4){
            vector<ClientInfo> tmp = h->get_clients_info();
            for(auto &c:tmp){
                c.channel = m_nChannel;
                ret.push_back(c);
            }
        }
    }
    {
        MutexLock lock(&m_csH264);
        for(auto h : m_vecLiveH264){
            vector<ClientInfo> tmp = h->get_clients_info();
            for(auto &c:tmp){
                c.channel = m_nChannel;
                ret.push_back(c);
            }
        }
    }
    {
        MutexLock lock(&m_csTs);
        for(auto h : m_vecLiveTs){
            vector<ClientInfo> tmp = h->get_clients_info();
            for(auto &c:tmp){
                c.channel = m_nChannel;
                ret.push_back(c);
            }
        }
    }
    {
        MutexLock lock(&m_csRtp);
        for(auto h : m_vecLiveRtp){
            vector<ClientInfo> tmp = h->get_clients_info();
            for(auto &c:tmp){
                c.channel = m_nChannel;
                ret.push_back(c);
            }
        }
    }
    return ret;
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
    if(nullptr != m_pFlv) {
        CFlv* flv = (CFlv*)m_pFlv;
        flv->SetSps(nWidth,nHeight,fFps);
    }
    if (nullptr != m_pMp4) {
        CMP4* mp4 = (CMP4*)m_pMp4;
        mp4->SetSps(nWidth,nHeight,fFps);
    }
}
}