#include "stdafx.h"
#include "LiveChannel.h"

namespace LiveClient
{
static void AVCallback(AV_BUFF buff, void* pUser){
    CLiveChannel* pLive = (CLiveChannel*)pUser;
    switch (buff.eType)
    {
    case AV_TYPE::H264_NALU:
        pLive->ReceiveStream(buff);
        break;
    default:
        break;
    }
}

CLiveChannel::CLiveChannel()
    : m_nChannel(0)
    , m_nWidth(0)
    , m_nHeight(0)
#ifdef EXTEND_CHANNELS
	, m_pEncoder(nullptr)
#endif
{
}

CLiveChannel::CLiveChannel(int channel, uint32_t w, uint32_t h)
    : m_nChannel(channel)
    , m_nWidth(w)
    , m_nHeight(h)
#ifdef EXTEND_CHANNELS
	, m_pEncoder(nullptr)
#endif
{
#ifdef EXTEND_CHANNELS
    m_pEncoder       = IEncoder::Create(AVCallback, this);
	m_pEncoder->m_width = m_nWidth;
	m_pEncoder->m_height = m_nHeight;
#endif
}

CLiveChannel::~CLiveChannel()
{
#ifdef EXTEND_CHANNELS
    SAFE_DELETE(m_pEncoder);
#endif
	Log::debug("CLiveChannel %d release", m_nChannel);
}

#ifdef EXTEND_CHANNELS
void CLiveChannel::SetDecoder(IDecoder *decoder)
{
    m_pEncoder->SetDecoder(decoder);
}
#endif

bool CLiveChannel::AddHandle(ILiveHandle* h, HandleType t)
{
    MutexLock lock(&m_csHandle);
    m_vecHandle.push_back(h);

    return true;
}

bool CLiveChannel::RemoveHandle(ILiveHandle* h)
{
    for(auto it = m_vecHandle.begin(); it != m_vecHandle.end(); it++) {
        if(*it == h) {
            m_vecHandle.erase(it);
			delete h;
            break;
        }
    }
    return m_vecHandle.empty();
}

void CLiveChannel::ReceiveStream(AV_BUFF buff)
{
    if(buff.eType == AV_TYPE::H264_NALU) {
        MutexLock lock(&m_csHandle);
        for (auto h : m_vecHandle) {
            //Log::debug("flv frag ok");
            h->push_video_stream(buff);
        }
    } else if (buff.eType == AV_TYPE::YUV) {
#ifdef EXTEND_CHANNELS
        m_pEncoder->Code(buff);
#endif
    }
}

bool CLiveChannel::Empty()
{
    return m_vecHandle.empty();
}

/** 获取客户端信息 */
vector<ClientInfo> CLiveChannel::GetClientInfo()
{
    vector<ClientInfo> ret;
    MutexLock lock(&m_csHandle);
    for(auto h : m_vecHandle){
        ClientInfo tmp = h->get_clients_info();
        ret.push_back(tmp);
    }
    
    return ret;
}

void CLiveChannel::stop()
{
    MutexLock lock(&m_csHandle);
    for (auto h : m_vecHandle) {
        h->stop();
    }
}

}