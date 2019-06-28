#include "stdafx.h"
#include "uv.h"
#include "HttpLiveServer.h"
#include "HttpWorker.h"
#include "LiveClient.h"

#include "h264.h"
#include "flv.h"
#include "mp4.h"

namespace HttpWsServer
{
    static void destroy_ring_node(void *_msg)
    {
        AV_BUFF *msg = (AV_BUFF*)_msg;
        free(msg->pData);
        msg->pData = NULL;
        msg->nLen = 0;
    }

    static void AVCallback(AV_BUFF buff, void* pUser){
        CHttpWorker* pLive = (CHttpWorker*)pUser;
        pLive->MediaCb(buff);
    }

    //////////////////////////////////////////////////////////////////////////

    CHttpWorker::CHttpWorker(string strCode, HandleType t, int nChannel, bool isWs)
        : m_strCode(strCode)
        , m_nType(0)
        , m_pLive(nullptr)
        , m_eHandleType(t)
        , m_nChannel(nChannel)
        , m_bWebSocket(isWs)
    {
        m_pRing  = lws_ring_create(sizeof(AV_BUFF), 50, destroy_ring_node);

        if (t == h264_handle) {
            CH264 *tmp = new CH264(AVCallback, this);
            m_pFormat = tmp;
        } else if(t == flv_handle) {
            CFlv *tmp = new CFlv(AVCallback, this);
            m_pFormat = tmp;
        } else if(t == fmp4_handle){
            CMP4 *tmp = new CMP4(AVCallback, this);
            m_pFormat = tmp;
        }

        m_pLive = LiveClient::GetWorker(strCode);
		if(m_pLive)
			m_pLive->AddHandle(this, t, nChannel);
    }

    CHttpWorker::~CHttpWorker()
    {
		if(m_pLive)
			m_pLive->RemoveHandle(this);

        lws_ring_destroy(m_pRing);
        Log::debug("CHttpWorker release");
    }

    AV_BUFF CHttpWorker::GetHeader()
    {
		return m_pLive->GetHeader(m_eMediaType, m_nChannel);
    }

    AV_BUFF CHttpWorker::GetVideo(uint32_t *tail)
    {
        AV_BUFF ret = {AV_TYPE::NONE,nullptr,0};
        AV_BUFF* tag = (AV_BUFF*)lws_ring_get_element(m_pRing, 0);
        if(tag) ret = *tag;
        lws_ring_consume(m_pRing, 0,0,0);
        return ret;
    }

    void CHttpWorker::NextWork(pss_http_ws_live* pss)
    {
        lws_callback_on_writable(pss->wsi);
    }

    void CHttpWorker::push_video_stream(AV_BUFF buff)
    {
        if (m_eMediaType == h264_handle) {
            CH264 *tmp = (CH264 *)m_pFormat;
            tmp->Code(buff);
        } else if(m_eMediaType == flv_handle) {
            CFlv *tmp = (CFlv *)m_pFormat;
            tmp->Code(buff);
        } else if(m_eMediaType == fmp4_handle){
            CMP4 *tmp = (CMP4 *)m_pFormat;
            tmp->Code(buff);
        }
    }

    void CHttpWorker::stop()
    {
        //视频源没有数据并超时
        Log::debug("no data recived any more, stopped");

        //断开所有客户端连接
        lws_set_timeout(m_pPss->wsi, PENDING_TIMEOUT_CLOSE_SEND, LWS_TO_KILL_ASYNC);
    }

    void CHttpWorker::MediaCb(AV_BUFF buff)
    {
        int n = (int)lws_ring_get_count_free_elements(m_pRing);
        if (!n) {
            return;
        }
        if(n <80)
            Log::debug("ring free space %d\n", n);

        // 将数据保存在ring buff
        char* pSaveBuff = (char*)malloc(buff.nLen + LWS_PRE);
        memcpy(pSaveBuff + LWS_PRE, buff.pData, buff.nLen);
        AV_BUFF newTag = {buff.eType, pSaveBuff, buff.nLen};
        if (!lws_ring_insert(m_pRing, &newTag, 1)) {
            destroy_ring_node(&newTag);
            Log::error("dropping!");
            return;
        }

        //向所有播放链接发送数据
        lws_callback_on_writable(m_pPss->wsi);
    }

    LiveClient::ClientInfo CHttpWorker::get_clients_info()
    {
        LiveClient::ClientInfo info;
        info.devCode = m_strCode;
        if(m_bWebSocket){
            info.connect = "Web Socket";
        } else {
            info.connect = "Http";
        }
        if(m_eMediaType == HandleType::flv_handle)
            info.media = "flv";
        else if(m_eMediaType == HandleType::fmp4_handle)
            info.media = "mp4";
        else if(m_eMediaType == HandleType::h264_handle)
            info.media = "h264";
        else if(m_eMediaType == HandleType::ts_handle)
            info.media = "hls";
        else 
            info.media = "unknown";
        info.clientIP = m_strClientIP;
        return info;
    }
}