#include "stdafx.h"
#include "RtspWorker.h"

namespace RtspServer
{
    static map<string,CRtspWorker*>  m_workerMapRtsp;
    static CriticalSection           m_csRtsp;

    static void destroy_ring_node(void *_msg)
    {
        AV_BUFF *msg = (AV_BUFF*)_msg;
        free(msg->pData);
        msg->pData = NULL;
        msg->nLen = 0;
    }

    CRtspWorker::CRtspWorker(string strCode)
        : m_strCode(strCode)
        , m_nType(0)
        , m_pLive(nullptr)
        , m_type(HandleType::rtp_handle)
    {
        m_pRing  = create_ring_buff(sizeof(AV_BUFF), 5000, destroy_ring_node);

        m_pLive = LiveClient::GetWorker(strCode);
        if(m_pLive)
            m_pLive->AddHandle(this, HandleType::rtp_handle, 0);
    }


    CRtspWorker::~CRtspWorker(void)
    {
        if(m_pLive)
            m_pLive->RemoveHandle(this);

        destroy_ring_buff(m_pRing);
        Log::debug("CRtspWorker release");
    }

    bool CRtspWorker::AddConnect(pss_rtsp_client* pss)
    {
        pss->pss_next = m_pPssList;
        m_pPssList = pss;
        pss->tail = ring_get_oldest_tail(m_pRing);

        return true;
    }

    bool CRtspWorker::DelConnect(pss_rtsp_client* pss)
    {
        ll_fwd_remove(pss_rtsp_client, pss_next, pss, m_pPssList);

        if(m_pPssList == NULL) {
            DelRtspWorker(m_strCode);
        }
        return true;
    }

    AV_BUFF CRtspWorker::GetVideo(uint32_t *tail)
    {
        AV_BUFF ret = {AV_TYPE::NONE, nullptr,0};
        AV_BUFF* tag = (AV_BUFF*)ring_get_element(m_pRing, tail);
        if(tag) ret = *tag;

        return ret;
    }

    void CRtspWorker::NextWork(pss_rtsp_client* pss)
    {
        //Log::debug("this work tail:%d\r\n", pss->tail);
        ring_consume_and_update_oldest_tail(
            m_pRing,	          /* lws_ring object */
            pss_rtsp_client,  /* type of objects with tails */
            &pss->tail,	      /* tail of guy doing the consuming */
            1,		          /* number of payload objects being consumed */
            m_pPssList,	      /* head of list of objects with tails */
            tail,		      /* member name of tail in objects with tails */
            pss_next	      /* member name of next object in objects with tails */
            );
        //Log::debug("next work tail:%d\r\n", pss->tail);

        /* more to do for us? */
        if (ring_get_element(m_pRing, &pss->tail))
            /* come back as soon as we can write more */
                rtp_callback_on_writable(pss->rtspClient);
                    ;
    }

    string CRtspWorker::GetSDP(){
        if(!m_pLive)
			return "";
		
		string origin = m_pLive->GetSDP(); //下级平台发出的sdp信息
		return origin;
    }

    void CRtspWorker::play_answer(int ret, string error_info)
    {
    }

    void CRtspWorker::push_video_stream(AV_BUFF buff)
    {
        int n = (int)ring_get_count_free_elements(m_pRing);
        if (!n) {
            cull_lagging_clients();
            n = (int)ring_get_count_free_elements(m_pRing);
        }
        Log::debug("ring free space %d\n", n);
        if (!n)
            return;

        // 将数据保存在ring buff
        char* pSaveBuff = (char*)malloc(buff.nLen);
        memcpy(pSaveBuff, buff.pData, buff.nLen);
        AV_BUFF newTag = {buff.eType, pSaveBuff, buff.nLen};
        if (!ring_insert(m_pRing, &newTag, 1)) {
            destroy_ring_node(&newTag);
            Log::error("dropping!");
            return;
        }

        //向所有播放链接发送数据
        start_foreach_llp(pss_rtsp_client **, ppss, m_pPssList) {
            rtp_callback_on_writable((*ppss)->rtspClient);
        } end_foreach_llp(ppss, pss_next);
    }

    void CRtspWorker::stop()
    {
        //视频源没有数据并超时
        Log::debug("no data recived any more, stopped");

        //断开所有客户端连接
        start_foreach_llp_safe(pss_rtsp_client **, ppss, m_pPssList, pss_next) {
            //lws_set_timeout((*ppss)->wsi, PENDING_TIMEOUT_CLOSE_SEND, LWS_TO_KILL_ASYNC);
        } end_foreach_llp_safe(ppss);
    }

    LiveClient::ClientInfo CRtspWorker::get_clients_info()
    {
        //vector<ClientInfo> ret;
        //stringstream ss;
        //start_foreach_llp(pss_rtsp_client **, ppss, m_pPssList) {
        //    ClientInfo info;
        //    info.devCode = m_strCode;
        //    info.connect = "RTSP";
        //    info.media = "RTSP";
        //    info.clientIP = (*ppss)->clientIP;
        //    ret.push_back(info);
        //} end_foreach_llp(ppss, pss_next);
        //return ret;
        LiveClient::ClientInfo info;
        return info;
    }

    void CRtspWorker::cull_lagging_clients()
    {
        uint32_t oldest_tail = ring_get_oldest_tail(m_pRing);
        pss_rtsp_client *old_pss = NULL;
        int most = 0, before = ring_get_count_waiting_elements(m_pRing, &oldest_tail), m;

        start_foreach_llp_safe(pss_rtsp_client **, ppss, m_pPssList, pss_next) {
            if ((*ppss)->tail == oldest_tail) {
                //连接超时
                old_pss = *ppss;
                Log::debug("Killing lagging client %p", (*ppss)->clientIP);
                //lws_set_timeout((*ppss)->wsi, PENDING_TIMEOUT_LAGGING, LWS_TO_KILL_ASYNC);
                (*ppss)->culled = 1;
                ll_fwd_remove(pss_rtsp_client, pss_next, (*ppss), m_pPssList);
                continue;
            } else {
                m = ring_get_count_waiting_elements(m_pRing, &((*ppss)->tail));
                if (m > most)
                    most = m;
            }
        } end_foreach_llp_safe(ppss);

        if (!old_pss)
            return;

        ring_consume_and_update_oldest_tail(m_pRing,
            pss_rtsp_client, &old_pss->tail, before - most,
            m_pPssList, tail, pss_next);

        Log::debug("shrunk ring from %d to %d", before, most);
    }

    //////////////////////////////////////////////////////////////////////////

    CRtspWorker* CreatRtspWorker(string strCode)
    {
        Log::debug("CreatRtspWorker begin");
        CRtspWorker* pNew = new CRtspWorker(strCode);
        MutexLock lock(&m_csRtsp);
        m_workerMapRtsp.insert(make_pair(strCode, pNew));
        return pNew;
    }

    CRtspWorker* GetRtspWorker(string strCode)
    {
        //Log::debug("GetWorker begin");
        MutexLock lock(&m_csRtsp);
        auto itFind = m_workerMapRtsp.find(strCode);
        if (itFind != m_workerMapRtsp.end()) {
            return itFind->second;
        }
        //Log::error("GetWorker failed: %s",strCode.c_str());
        return nullptr;
    }

    bool DelRtspWorker(string strCode)
    {
        MutexLock lock(&m_csRtsp);
        auto itFind = m_workerMapRtsp.find(strCode);
        if (itFind != m_workerMapRtsp.end()) {
            SAFE_DELETE(itFind->second);
            m_workerMapRtsp.erase(itFind);
            return true;
        }

        Log::error("dosn't find worker object");
        return false;
    }
}