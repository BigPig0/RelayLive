#include "stdafx.h"
#include "HttpLiveServer.h"
#include "LiveWorker.h"
//其他模块
//#include "SipInstance.h"
#include "libLive.h"
#include "uvIpc.h"

namespace HttpWsServer
{
	extern uv_loop_t *g_uv_loop;

    static map<string,CLiveWorker*>  m_workerMap;
    static CriticalSection           m_cs;

    static string m_strRtpIP;            //< RTP服务IP
    static int    m_nRtpBeginPort;       //< RTP监听的起始端口，必须是偶数
    static int    m_nRtpPortNum;         //< RTP使用的个数，从strRTPPort开始每次加2，共strRTPNum个
    static int    m_nRtpCatchPacketNum;  //< rtp缓存的包的数量

    static vector<int>     m_vecRtpPort;     //< RTP可用端口，使用时从中取出，使用结束重新放入
    static CriticalSection m_csRTP;          //< RTP端口锁
    static bool _do_port = false;

    static uv_ipc_handle_t* h = NULL;

    struct ipc_play_task
    {
        int ipc_status;
        int ret;
        string ssid;
        string error;
    };
    static ipc_play_task _ipc_task;

    static string strfind(char* src, char* begin, char* end){
        char *p1, *p2;
        p1 = strstr(src, begin);
        if(!p1) return "";
        p1 += strlen(begin);
        p2 = strstr(p1, end);
        if(p2) return string(p1, p2-p1);
        else return string(p1);
    }

    static void on_ipc_recv(uv_ipc_handle_t* h, void* user, char* name, char* msg, char* data, int len) {
        if (!strcmp(msg,"live_play_answer")) {
            // ssid=123&ret=0&error=XXXX
			data[len] = 0;
            _ipc_task.ssid = strfind(data, "ssid=", "&");
            _ipc_task.ret = stoi(strfind(data, "ret=", "&"));
            _ipc_task.error = strfind(data, "error=", "&");
            _ipc_task.ipc_status = 0;
        }
    }

    static int real_play(string dev_code, string rtp_ip, int rtp_port){
        // ssid=123&rtpip=1.1.1.1&rtpport=50000
        _ipc_task.ssid = dev_code;
        _ipc_task.ret = 0;
        _ipc_task.ipc_status = 1;
        _ipc_task.error = "";

        stringstream ss;
        ss << "ssid=" << dev_code << "&rtpip=" << rtp_ip << "&rtpport=" << rtp_port;
        int ret = uv_ipc_send(h, "liveSrc", "live_play", (char*)ss.str().c_str(), ss.str().size());
        if(ret) {
            Log::error("ipc send real play error");
            return ret;
        }

        while (_ipc_task.ipc_status) Sleep(100);
        return _ipc_task.ret;
    }

    static int stop_play(string dev_code){
        int ret = uv_ipc_send(h, "liveSrc", "stop_play", (char*)dev_code.c_str(), dev_code.size());
        if(ret) {
            Log::error("ipc send stop error");
            return ret;
        }
		return 0;
    }


    static int GetRtpPort()
    {
        MutexLock lock(&m_csRTP);
        if(!_do_port) {
            _do_port = true;
            m_strRtpIP           = Settings::getValue("RtpClient","IP");                    //< RTP服务IP
            m_nRtpBeginPort      = Settings::getValue("RtpClient","BeginPort",10000);       //< RTP监听的起始端口，必须是偶数
            m_nRtpPortNum        = Settings::getValue("RtpClient","PortNum",1000);          //< RTP使用的个数，从strRTPPort开始每次加2，共strRTPNum个
            m_nRtpCatchPacketNum = Settings::getValue("RtpClient", "CatchPacketNum", 100);  //< rtp缓存的包的数量

            Log::debug("RtpConfig IP:%s, BeginPort:%d,PortNum:%d,CatchPacketNum:%d"
                , m_strRtpIP.c_str(), m_nRtpBeginPort, m_nRtpPortNum, m_nRtpCatchPacketNum);
            m_vecRtpPort.clear();
            for (int i=0; i<m_nRtpPortNum; ++i)
            {
                m_vecRtpPort.push_back(m_nRtpBeginPort+i*2);
            }
        }

        int nRet = -1;
        auto it = m_vecRtpPort.begin();
        if (it != m_vecRtpPort.end())
        {
            nRet = *it;
            m_vecRtpPort.erase(it);
        }

        return nRet;
    }

    static void GiveBackRtpPort(int nPort)
    {
        MutexLock lock(&m_csRTP);
        m_vecRtpPort.push_back(nPort);
    }

    static void destroy_ring_node(void *_msg)
    {
        LIVE_BUFF *msg = (LIVE_BUFF*)_msg;
        free(msg->pBuff);
        msg->pBuff = NULL;
        msg->nLen = 0;
    }

    /** 延时销毁定时器从loop中移除 */
    static void stop_timer_close_cb(uv_handle_t* handle) {
        Log::debug("all client has been closed");
        CLiveWorker* live = (CLiveWorker*)handle->data;
        if (live->GetTimeStop()){
            live->Clear2Stop();
        }
    }

    /** 客户端全部断开后，延时断开源的定时器 */
	static void stop_timer_cb(uv_timer_t* handle) {
		CLiveWorker* live = (CLiveWorker*)handle->data;
		int ret = uv_timer_stop(handle);
		if(ret < 0) {
			Log::error("timer stop error:%s",uv_strerror(ret));
        }
        uv_close((uv_handle_t*)handle, stop_timer_close_cb);

        live->SetTimeStop(true);
	}

    /** 超时定时器从loop移除的回调 */
    static void over_timer_close_cb(uv_handle_t* handle) {
        Log::debug("src live stoped");
    }

    /** 源数据一段时间未收到，超时，断开所有客户端的定时器 */
    static void over_timer_cb(uv_timer_t* handle) {
        CLiveWorker* live = (CLiveWorker*)handle->data;
        int ret = uv_timer_stop(handle);
        if(ret < 0) {
            Log::error("timer stop error:%s",uv_strerror(ret));
        }
        uv_close((uv_handle_t*)handle, over_timer_close_cb);

        live->Over2Stop();
    }

    /** CLiveWorker析构中删除m_pLive比较耗时，会阻塞event loop，因此使用线程。 */
    static void live_worker_destory_thread(void* arg) {
		CLiveWorker* live = (CLiveWorker*)arg;
        SAFE_DELETE(live);
    }

    //////////////////////////////////////////////////////////////////////////

    CLiveWorker::CLiveWorker(string strCode, int rtpPort)
        : m_strCode(strCode)
        , m_nPort(rtpPort)
        , m_nType(0)
        , m_pLive(nullptr)
        , m_bStop(false)
        , m_bOver(false)
    {
        memset(&m_stFlvHead, 0, sizeof(m_stFlvHead));
        memset(&m_stMP4Head, 0, sizeof(m_stMP4Head));
        m_pFlvRing  = lws_ring_create(sizeof(LIVE_BUFF), 100, destroy_ring_node);
        m_pH264Ring = lws_ring_create(sizeof(LIVE_BUFF), 100, destroy_ring_node);
        m_pMP4Ring  = lws_ring_create(sizeof(LIVE_BUFF), 100, destroy_ring_node);

        m_pLive = IlibLive::CreateObj();
        m_pLive->SetLocalAddr(m_strRtpIP, m_nPort);
        m_pLive->SetCatchPacketNum(m_nRtpCatchPacketNum);
        m_pLive->SetCallback(this);
        m_pLive->StartListen();
    }

    CLiveWorker::~CLiveWorker()
    {
        //if(m_nType == 0) {
        //    if (!SipInstance::StopPlay(m_strCode)) {
        //        Log::error("stop play failed");
        //    }
        //} else {
        //    if (!SipInstance::StopRecordPlay(m_strCode, "")) {
        //        Log::error("stop play failed");
        //    }
        //}
        if(stop_play(m_strCode)) {
            Log::error("stop play failed");
        }
        SAFE_DELETE(m_pLive);
        lws_ring_destroy(m_pFlvRing);
        lws_ring_destroy(m_pH264Ring);
        lws_ring_destroy(m_pMP4Ring);
        GiveBackRtpPort(m_nPort);
        Log::debug("CLiveWorker release");
    }

    bool CLiveWorker::AddConnect(pss_http_ws_live* pss)
    {
        if(pss->media_type == media_flv) {
            m_bFlv = true;
            pss->pss_next = m_pFlvPssList;
            m_pFlvPssList = pss;
            pss->tail = lws_ring_get_oldest_tail(m_pFlvRing);
        } else if (pss->media_type == media_h264) {
            m_bH264 = true;
            pss->pss_next = m_pH264PssList;
            m_pH264PssList = pss;
            pss->tail = lws_ring_get_oldest_tail(m_pH264Ring);
        } else if (pss->media_type == media_mp4) {
            m_bMp4 = true;
            pss->pss_next = m_pMP4PssList;
            m_pMP4PssList = pss;
            pss->tail = lws_ring_get_oldest_tail(m_pMP4Ring);
        } else {
            return false;
        }

        //如果刚刚开启了结束定时器，需要将其关闭
        if(uv_is_active((const uv_handle_t*)&m_uvTimerStop)) {
            uv_timer_stop(&m_uvTimerStop);
            uv_close((uv_handle_t*)&m_uvTimerStop, stop_timer_close_cb);
        }
        return true;
    }

    bool CLiveWorker::DelConnect(pss_http_ws_live* pss)
    {
        if(pss->media_type == media_flv) {
            lws_ll_fwd_remove(pss_http_ws_live, pss_next, pss, m_pFlvPssList);
            if(nullptr == m_pFlvPssList) m_bFlv = false;
        } else if(pss->media_type == media_h264){
            lws_ll_fwd_remove(pss_http_ws_live, pss_next, pss, m_pH264PssList);
            if (nullptr == m_pH264PssList) m_bH264 = false;
        } else if(pss->media_type == media_mp4) {
            lws_ll_fwd_remove(pss_http_ws_live, pss_next, pss, m_pMP4PssList);
            if (nullptr == m_pMP4PssList) m_bMp4 = false;
        }

        if(m_pFlvPssList == NULL && m_pH264PssList == NULL && m_pMP4PssList == NULL) {
            uv_timer_init(g_uv_loop, &m_uvTimerStop);
            m_uvTimerStop.data = this;
			uv_timer_start(&m_uvTimerStop, stop_timer_cb, 20000, 0);
        }
        return true;
    }

	void CLiveWorker::Clear2Stop() {
		if(m_pFlvPssList == NULL && m_pH264PssList == NULL && m_pMP4PssList == NULL) {
			Log::debug("need close stream");
			DelLiveWorker(m_strCode);
		}
	}

    void CLiveWorker::Over2Stop() {
        //lws_start_foreach_llp(pss_http_ws_live **, ppss, m_pFlvPssList) {
        //    lws_close_reason((*ppss)->wsi, LWS_CLOSE_STATUS_NORMAL, (unsigned char*)"rtp stop", 8);
        //} lws_end_foreach_llp(ppss, pss_next);
        //lws_start_foreach_llp(pss_http_ws_live **, ppss, m_pH264PssList) {
        //    lws_close_reason((*ppss)->wsi, LWS_CLOSE_STATUS_NORMAL, (unsigned char*)"rtp stop", 8);
        //} lws_end_foreach_llp(ppss, pss_next);
        //lws_start_foreach_llp(pss_http_ws_live **, ppss, m_pMP4PssList) {
        //    lws_close_reason((*ppss)->wsi, LWS_CLOSE_STATUS_NORMAL, (unsigned char*)"rtp stop", 8);
        //} lws_end_foreach_llp(ppss, pss_next);
    }

    void CLiveWorker::push_flv_frame(FLV_FRAG_TYPE eType, char* pBuff, int nLen)
    {
        if (eType == FLV_HEAD) {
            m_stFlvHead.pBuff = pBuff;
            m_stFlvHead.nLen = nLen;
            Log::debug("flv head ok");
        } else {
            //内存数据保存至ring-buff
            int n = (int)lws_ring_get_count_free_elements(m_pFlvRing);
            if (!n) {
                cull_lagging_clients(media_flv);
                n = (int)lws_ring_get_count_free_elements(m_pFlvRing);
            }
            Log::debug("LWS_CALLBACK_RECEIVE: free space %d\n", n);
            if (!n)
                return;

            // 将数据保存在ring buff
            char* pSaveBuff = (char*)malloc(nLen + LWS_PRE);
            memcpy(pSaveBuff + LWS_PRE, pBuff, nLen);
            LIVE_BUFF newTag = {pSaveBuff, nLen};
            if (!lws_ring_insert(m_pFlvRing, &newTag, 1)) {
                destroy_ring_node(&newTag);
                Log::error("dropping!");
                return;
            }

            //向所有播放链接发送数据
            lws_start_foreach_llp(pss_http_ws_live **, ppss, m_pFlvPssList) {
                lws_callback_on_writable((*ppss)->wsi);
            } lws_end_foreach_llp(ppss, pss_next);
        }    
    }

    void CLiveWorker::push_h264_stream(char* pBuff, int nLen)
    {
        int n = (int)lws_ring_get_count_free_elements(m_pH264Ring);
        if (!n) {
            cull_lagging_clients(media_h264);
            n = (int)lws_ring_get_count_free_elements(m_pH264Ring);
        }
        Log::debug("h264 ring free space %d\n", n);
        if (!n)
            return;

        // 将数据保存在ring buff
        char* pSaveBuff = (char*)malloc(nLen + LWS_PRE);
        memcpy(pSaveBuff + LWS_PRE, pBuff, nLen);
        LIVE_BUFF newTag = {pSaveBuff, nLen};
        if (!lws_ring_insert(m_pH264Ring, &newTag, 1)) {
            destroy_ring_node(&newTag);
            Log::error("dropping!");
            return;
        }

        //向所有播放链接发送数据
        lws_start_foreach_llp(pss_http_ws_live **, ppss, m_pH264PssList) {
            lws_callback_on_writable((*ppss)->wsi);
        } lws_end_foreach_llp(ppss, pss_next);
    }

    void CLiveWorker::push_ts_stream(char* pBuff, int nLen)
    {
    }

    void CLiveWorker::push_mp4_stream(MP4_FRAG_TYPE eType, char* pBuff, int nLen)
    {
        if(eType == MP4_HEAD) {
            m_stMP4Head.pBuff = pBuff;
            m_stMP4Head.nLen = nLen;
            Log::debug("MP4 Head ok");
        } else {
            int n = (int)lws_ring_get_count_free_elements(m_pMP4Ring);
            if (!n) {
                cull_lagging_clients(media_mp4);
                n = (int)lws_ring_get_count_free_elements(m_pMP4Ring);
            }
            Log::debug("mp4 ring free space %d\n", n);
            if (!n)
                return;

            // 将数据保存在ring buff
            char* pSaveBuff = (char*)malloc(nLen + LWS_PRE);
            memcpy(pSaveBuff + LWS_PRE, pBuff, nLen);
            LIVE_BUFF newTag = {pSaveBuff, nLen};
            if (!lws_ring_insert(m_pMP4Ring, &newTag, 1)) {
                destroy_ring_node(&newTag);
                Log::error("dropping!");
                return;
            }

            //向所有播放链接发送数据
            lws_start_foreach_llp(pss_http_ws_live **, ppss, m_pMP4PssList) {
                lws_callback_on_writable((*ppss)->wsi);
            } lws_end_foreach_llp(ppss, pss_next);
        }
    }

    void CLiveWorker::stop()
    {
        // stop是由rtp接收线程调用的，通过timer转为由当前模块的loop线程调用
        // 主动关闭客户端连接
        m_bOver = true;
        uv_timer_init(g_uv_loop, &m_uvTimerOver);
        m_uvTimerOver.data = this;
        uv_timer_start(&m_uvTimerOver, over_timer_cb, 0, 0);
    }

    LIVE_BUFF CLiveWorker::GetFlvHeader()
    {
        return m_stFlvHead;
    }

    LIVE_BUFF CLiveWorker::GetFlvVideo(uint32_t *tail)
    {
        LIVE_BUFF ret = {nullptr,0};
        LIVE_BUFF* tag = (LIVE_BUFF*)lws_ring_get_element(m_pFlvRing, tail);
        if(tag) ret = *tag;

        return ret;
    }

    LIVE_BUFF CLiveWorker::GetH264Video(uint32_t *tail)
    {
        LIVE_BUFF ret = {nullptr,0};
        LIVE_BUFF* tag = (LIVE_BUFF*)lws_ring_get_element(m_pH264Ring, tail);
        if(tag) ret = *tag;

        return ret;
    }

    LIVE_BUFF CLiveWorker::GetMp4Header()
    {
        return m_stMP4Head;
    }

    LIVE_BUFF CLiveWorker::GetMp4Video(uint32_t *tail)
    {
        LIVE_BUFF ret = {nullptr,0};
        LIVE_BUFF* tag = (LIVE_BUFF*)lws_ring_get_element(m_pMP4Ring, tail);
        if(tag) ret = *tag;

        return ret;
    }

    void CLiveWorker::NextWork(pss_http_ws_live* pss)
    {
        struct lws_ring *ring;
        pss_http_ws_live* pssList;
        if(pss->media_type == media_flv) {
            ring = m_pFlvRing;
            pssList = m_pFlvPssList;
        } else if(pss->media_type == media_h264){
            ring = m_pH264Ring;
            pssList = m_pH264PssList;
        } else if (pss->media_type == media_mp4) {
            ring = m_pMP4Ring;
            pssList = m_pMP4PssList;
        } else {
            return;
        }

        //Log::debug("this work tail:%d\r\n", pss->tail);
        lws_ring_consume_and_update_oldest_tail(
            ring,	          /* lws_ring object */
            pss_http_ws_live, /* type of objects with tails */
            &pss->tail,	      /* tail of guy doing the consuming */
            1,		          /* number of payload objects being consumed */
            pssList,	      /* head of list of objects with tails */
            tail,		      /* member name of tail in objects with tails */
            pss_next	      /* member name of next object in objects with tails */
            );
        //Log::debug("next work tail:%d\r\n", pss->tail);

        /* more to do for us? */
        if (lws_ring_get_element(ring, &pss->tail))
            /* come back as soon as we can write more */
                lws_callback_on_writable(pss->wsi);
    }

    string CLiveWorker::GetClientInfo()
    {
        return "{}";
    }

    void CLiveWorker::cull_lagging_clients(MediaType type)
    {
        struct lws_ring *ring;
        pss_http_ws_live* pssList;
        if(type == media_flv) {
            ring = m_pFlvRing;
            pssList = m_pFlvPssList;
        } else {
            ring = m_pH264Ring;
            pssList = m_pH264PssList;
        }

        uint32_t oldest_tail = lws_ring_get_oldest_tail(ring);
        pss_http_ws_live *old_pss = NULL;
        int most = 0, before = lws_ring_get_count_waiting_elements(ring, &oldest_tail), m;

        lws_start_foreach_llp_safe(pss_http_ws_live **, ppss, pssList, pss_next) {
            if ((*ppss)->tail == oldest_tail) {
                //连接超时
                old_pss = *ppss;
                Log::debug("Killing lagging client %p", (*ppss)->wsi);
                lws_set_timeout((*ppss)->wsi, PENDING_TIMEOUT_LAGGING, LWS_TO_KILL_ASYNC);
                (*ppss)->culled = 1;
                lws_ll_fwd_remove(pss_http_ws_live, pss_next, (*ppss), pssList);
                continue;
            } else {
                m = lws_ring_get_count_waiting_elements(ring, &((*ppss)->tail));
                if (m > most)
                    most = m;
            }
        } lws_end_foreach_llp_safe(ppss);

        if (!old_pss)
            return;

        lws_ring_consume_and_update_oldest_tail(ring,
            pss_http_ws_live, &old_pss->tail, before - most,
            pssList, tail, pss_next);

        Log::debug("shrunk ring from %d to %d", before, most);
    }

    //////////////////////////////////////////////////////////////////////////

    void ipc_init(){
        /** 进程间通信 */
        int ret = uv_ipc_client(&h, "relay_live", NULL, "liveDest", on_ipc_recv, NULL);
        if(ret < 0) {
            printf("ipc server err: %s\n", uv_ipc_strerr(ret));
        }
    }

    CLiveWorker* CreatLiveWorker(string strCode)
    {
        Log::debug("CreatFlvBuffer begin");
        int rtpPort = GetRtpPort();
        if(rtpPort < 0) {
            Log::error("play failed %s, no rtp port",strCode.c_str());
            return nullptr;
        }

        CLiveWorker* pNew = new CLiveWorker(strCode, rtpPort);

        //if(!SipInstance::RealPlay(strCode, m_strRtpIP,  rtpPort))
        if(real_play(strCode, m_strRtpIP,  rtpPort))
        {
            delete pNew;
            Log::error("play failed %s",strCode.c_str());
            return nullptr;
        }
        Log::debug("RealPlay ok: %s",strCode.c_str());

        MutexLock lock(&m_cs);
        m_workerMap.insert(make_pair(strCode, pNew));

        return pNew;
    }

    CLiveWorker* GetLiveWorker(string strCode)
    {
        Log::debug("GetWorker begin");

        MutexLock lock(&m_cs);
        auto itFind = m_workerMap.find(strCode);
        if (itFind != m_workerMap.end())
        {
            // 已经存在
            return itFind->second;
        }

        Log::error("GetWorker failed: %s",strCode.c_str());
        return nullptr;
    }

    bool DelLiveWorker(string strCode)
    {
        MutexLock lock(&m_cs);
        auto itFind = m_workerMap.find(strCode);
        if (itFind != m_workerMap.end())
        {
            // CLiveWorker析构中删除m_pLive比较耗时，会阻塞event loop，因此使用线程。
            uv_thread_t tid;
            uv_thread_create(&tid, live_worker_destory_thread, itFind->second);
            
            m_workerMap.erase(itFind);
            return true;
        }

        Log::error("dosn't find worker object");
        return false;
    }
}