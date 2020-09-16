#include "stdafx.h"
#include "uv.h"
#include "HttpLiveServer.h"
#include "HttpWorker.h"
#include "LiveClient.h"

#include "h264.h"
#include "flv.h"
#include "mp4.h"

#include "netstream.h"

namespace HttpWsServer
{
    extern int g_nNodelay;            //< 视频格式打包是否立即发送 1:每个帧都立即发送  0:每个关键帧及其后面的非关键帧收到后一起发送

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
		, m_bConnect(true)
    {
        m_pRing  = lws_ring_create(sizeof(AV_BUFF), 100, destroy_ring_node);

        if (t == h264_handle) {
            CH264 *tmp = new CH264(AVCallback, this);
            tmp->SetNodelay(g_nNodelay);
            m_pFormat = tmp;
        } else if(t == flv_handle) {
            CFlv *tmp = new CFlv(AVCallback, this);
            tmp->SetNodelay(g_nNodelay);
            m_pFormat = tmp;
        } else if(t == fmp4_handle){
            CMP4 *tmp = new CMP4(AVCallback, this);
            tmp->SetNodelay(g_nNodelay);
            m_pFormat = tmp;
        }

        m_SocketBuff.eType = AV_TYPE::NONE;
        m_SocketBuff.pData = nullptr;
        m_SocketBuff.nLen = 0;

        m_pLive = LiveClient::GetWorker(strCode);
		if(m_pLive)
			m_pLive->AddHandle(this, t, nChannel);
    }

    CHttpWorker::~CHttpWorker()
    {

        SAFE_FREE(m_SocketBuff.pData);

        lws_ring_destroy(m_pRing);
        Log::debug("CHttpWorker release");
    }

    int CHttpWorker::GetVideo(char **buff)
    {
        int wnum = lws_ring_get_count_waiting_elements(m_pRing, NULL);
        //没有缓存数据
        if(wnum == 0)
            return 0;
        //只有一个缓存数据
        if(wnum == 1) {
            AV_BUFF *tmp = (AV_BUFF*)lws_ring_get_element(m_pRing, NULL);
            if(!tmp) {
                return 0;
            }
            uint32_t tlen = LWS_PRE +  tmp->nLen;         //LWS_PRE 多申请一段空间，供websocket发送使用
            if(m_SocketBuff.nLen < tlen){
                m_SocketBuff.pData = (char*)realloc(m_SocketBuff.pData, tlen);
                m_SocketBuff.nLen = tlen;
            }
            memset(m_SocketBuff.pData, 0, m_SocketBuff.nLen);
            memcpy(m_SocketBuff.pData+LWS_PRE, tmp->pData, tmp->nLen);
            *buff = m_SocketBuff.pData;
            lws_ring_consume(m_pRing, NULL, NULL, 1);
            return tlen;
        }

        //多个缓存数据
		AV_BUFF *tmp = (AV_BUFF*)lws_ring_get_element(m_pRing, NULL);
		if(!tmp) {
			return 0;
		}
		net_stream_maker_t *sm = create_net_stream_maker();
		while(tmp){
			net_stream_append_data(sm, tmp->pData, tmp->nLen);
			lws_ring_consume(m_pRing, NULL, NULL, 1);
			tmp = (AV_BUFF*)lws_ring_get_element(m_pRing, NULL);
		}

		uint32_t tlen = LWS_PRE +  get_net_stream_len(sm);          //LWS_PRE 多申请一段空间，供websocket发送使用
        if(m_SocketBuff.nLen < tlen){
            m_SocketBuff.pData = (char*)realloc(m_SocketBuff.pData, tlen);
            m_SocketBuff.nLen = tlen;
        }
        memset(m_SocketBuff.pData, 0, m_SocketBuff.nLen);
		memcpy(m_SocketBuff.pData+LWS_PRE, get_net_stream_data(sm), get_net_stream_len(sm));
		destory_net_stream_maker(sm);

		*buff = m_SocketBuff.pData;
        return tlen;
    }

    void CHttpWorker::play_answer(int ret, string error_info)
    {
		if(!m_bConnect){
			Log::debug("connect has stoped");
			return;
		}
        if(ret){
            m_pPss->error_code = ret;
            m_strError = error_info;
        }
        lws_callback_on_writable(m_pPss->wsi);
    }

    void CHttpWorker::push_video_stream(AV_BUFF buff)
    {
		if(!m_bConnect){
			Log::debug("connect has stoped");
			return;
		}
        if (m_eHandleType == h264_handle) {
            CH264 *tmp = (CH264 *)m_pFormat;
            tmp->Code(buff);
        } else if(m_eHandleType == flv_handle) {
            CFlv *tmp = (CFlv *)m_pFormat;
            tmp->Code(buff);
        } else if(m_eHandleType == fmp4_handle){
            CMP4 *tmp = (CMP4 *)m_pFormat;
            tmp->Code(buff);
        }
    }

    void CHttpWorker::stop()
    {
		if(!m_bConnect){
			Log::debug("connect has stoped");
			return;
		}

        //视频源没有数据并超时
        Log::debug("no data recived any more, stopped");

        //断开所有客户端连接
        lws_set_timeout(m_pPss->wsi, PENDING_TIMEOUT_CLOSE_SEND, LWS_TO_KILL_ASYNC);
    }

    void CHttpWorker::MediaCb(AV_BUFF buff)
    {
		if(!m_bConnect){
			Log::debug("connect has stoped");
			return;
		}
        int n = (int)lws_ring_get_count_free_elements(m_pRing);
        //Log::debug("ring free space %d", n);
        if (!n) {
            return;
        }

        // 将数据保存在ring buff
        char* pSaveBuff = (char*)malloc(buff.nLen);
        memcpy(pSaveBuff, buff.pData, buff.nLen);
        AV_BUFF newTag = {buff.eType, pSaveBuff, buff.nLen, 0, 0};

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
        if(m_eHandleType == HandleType::flv_handle)
            info.media = "flv";
        else if(m_eHandleType == HandleType::fmp4_handle)
            info.media = "mp4";
        else if(m_eHandleType == HandleType::h264_handle)
            info.media = "h264";
        else if(m_eHandleType == HandleType::ts_handle)
            info.media = "hls";
        else 
            info.media = "unknown";
		info.channel = m_nChannel;
        info.clientIP = m_strClientIP;
        return info;
    }

	void CHttpWorker::close()
	{
		m_bConnect = false;
		if(m_pLive)
			m_pLive->RemoveHandle(this);
	}
}