#include "stdafx.h"
#include "HttpLiveServer.h"
#include "LiveWorker.h"
//其他模块
#include "SipInstance.h"
#include "libLive.h"
#include "H264.h"
#include "Flv.h"

namespace HttpWsServer
{
    static map<string,CLiveWorker*>  m_workerMap;
    static CriticalSection           m_cs;
    static FLV_TAG_BUFF              m_flvHeader = {nullptr,0,callback_flv_header}; // flv头，对所有视频都是一样的
    
    static string m_strRtpIP           = Settings::getValue("RtpClient","IP");            //< RTP服务IP
    static int    m_nRtpBeginPort      = Settings::getValue("RtpClient","BeginPort",10000);  //< RTP监听的起始端口，必须是偶数
    static int    m_nRtpPortNum        = Settings::getValue("RtpClient","PortNum",1000);    //< RTP使用的个数，从strRTPPort开始每次加2，共strRTPNum个
    static int    m_nRtpCatchPacketNum = Settings::getValue("RtpClient", "CatchPacketNum", 100);  //< rtp缓存的包的数量
    
    static vector<int>     m_vecRtpPort;     //< RTP可用端口，使用时从中取出，使用结束重新放入
    static CriticalSection m_csRTP;          //< RTP端口锁
    static bool _do_port = false;


    static int GetRtpPort()
    {
        MutexLock lock(&m_csRTP);
        if(!_do_port) {
            _do_port = true;
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

    static void destroy_flvtag(void *_msg)
    {
        FLV_TAG_BUFF *msg = (FLV_TAG_BUFF*)_msg;
        free(msg->pBuff);
        msg->pBuff = NULL;
        msg->nLen = 0;
    }

    static void destroy_h264nalu(void *_msg)
    {
        H264_NALU_BUFF *msg = (H264_NALU_BUFF*)_msg;
        free(msg->pBuff);
        msg->pBuff = NULL;
        msg->nLen = 0;
    }

    //////////////////////////////////////////////////////////////////////////

    CLiveWorker::CLiveWorker(string strCode)
        : m_strCode(strCode)
        , m_type(0)
        , m_pLive(nullptr)
    {
        memset(&m_pScriptTag, 0, sizeof(m_pScriptTag));
        memset(&m_pDecodeConfig, 0, sizeof(m_pDecodeConfig));
        m_flvRing = lws_ring_create(sizeof(FLV_TAG_BUFF), 100, destroy_flvtag);
        m_h264Ring = lws_ring_create(sizeof(H264_NALU_BUFF), 100, destroy_h264nalu);

        m_nPort = GetRtpPort();
        m_pLive = IlibLive::CreateObj();
        m_pLive->SetLocalAddr(m_strRtpIP, m_nPort);
        m_pLive->SetCatchPacketNum(m_nRtpCatchPacketNum);
        m_pLive->SetCallback(this);
        m_pLive->StartListen();
    }

    CLiveWorker::~CLiveWorker()
    {
        if(m_type == 0) {
            if (!SipInstance::StopPlay(m_strCode)) {
                Log::error("stop play failed");
            }
        } else {
            if (!SipInstance::StopRecordPlay(m_strCode, "")) {
                Log::error("stop play failed");
            }
        }
        lws_ring_destroy(m_flvRing);
        lws_ring_destroy(m_h264Ring);
        if(m_pScriptTag.pBuff) free(m_pScriptTag.pBuff);
        if(m_pDecodeConfig.pBuff) free(m_pDecodeConfig.pBuff);
        SAFE_DELETE(m_pLive);
        GiveBackRtpPort(m_nPort);
    }

    bool CLiveWorker::AddConnect(pss_http_ws_live* pss)
    {
        if(pss->media_type == media_flv) {
            m_bFlv = true;
            pss->pss_next = m_flvPssList;
            m_flvPssList = pss;
            pss->tail = lws_ring_get_oldest_tail(m_flvRing);
        } else if (pss->media_type == media_h264) {
            m_bH264 = true;
            pss->pss_next = m_h264PssList;
            m_h264PssList = pss;
            pss->tail = lws_ring_get_oldest_tail(m_h264Ring);
        } else {
            return false;
        }
        return true;
    }

    bool CLiveWorker::DelConnect(pss_http_ws_live* pss)
    {
        if(pss->media_type == media_flv) {
            lws_ll_fwd_remove(pss_http_ws_live, pss_next, pss, m_flvPssList);
            if(nullptr == m_flvPssList) m_bFlv = false;
        } else if(pss->media_type == media_h264){
            lws_ll_fwd_remove(pss_http_ws_live, pss_next, pss, m_h264PssList);
            if (nullptr == m_h264PssList) m_bH264 = false;
        }

        if(m_flvPssList == NULL && m_h264PssList == NULL) {
            std::thread t([&](){
                Sleep(20000);
                if (m_flvPssList == NULL && m_h264PssList == NULL) {
                    DelLiveWorker(m_strCode);
                }
            });
            t.detach();
        }
        return true;
    }

    void CLiveWorker::push_flv_frame(int tag_type, char* pBuff, int nLen)
    {
        flv_tag_type eType = (flv_tag_type)tag_type;
        if (eType == callback_script_tag) {
            m_pScriptTag.pBuff = pBuff;
            m_pScriptTag.nLen = nLen;
            m_pScriptTag.eType = callback_script_tag;
            Log::debug("send script tag ok");
        } else if (eType == callback_video_spspps_tag) {
            m_pDecodeConfig.pBuff = pBuff;
            m_pDecodeConfig.nLen = nLen;
            m_pDecodeConfig.eType = callback_video_spspps_tag;
            Log::debug("AVCDecoderConfigurationRecord ok");
        } else {
            //内存数据保存至ring-buff
            int n = (int)lws_ring_get_count_free_elements(m_flvRing);
            if (!n) {
                cull_lagging_clients(media_flv);
                n = (int)lws_ring_get_count_free_elements(m_flvRing);
            }
            Log::debug("LWS_CALLBACK_RECEIVE: free space %d\n", n);
            if (!n)
                return;

            // 将数据保存在ring buff
            FLV_TAG_BUFF newTag = {pBuff, nLen, eType};
            if (!lws_ring_insert(m_flvRing, &newTag, 1)) {
                destroy_flvtag(&newTag);
                Log::error("dropping!");
                return;
            }

            //向所有播放链接发送数据
            lws_start_foreach_llp(pss_http_ws_live **, ppss, m_flvPssList) {
                lws_callback_on_writable((*ppss)->wsi);
            } lws_end_foreach_llp(ppss, pss_next);
        }    
    }

    void CLiveWorker::push_h264_stream(NalType eType, char* pBuff, int nLen)
    {
        int n = (int)lws_ring_get_count_free_elements(m_h264Ring);
        if (!n) {
            cull_lagging_clients(media_h264);
            n = (int)lws_ring_get_count_free_elements(m_h264Ring);
        }
        Log::debug("h264 ring free space %d\n", n);
        if (!n)
            return;

        // 将数据保存在ring buff
        H264_NALU_BUFF newTag = {pBuff, nLen, eType};
        if (!lws_ring_insert(m_h264Ring, &newTag, 1)) {
            destroy_h264nalu(&newTag);
            Log::error("dropping!");
            return;
        }

        //向所有播放链接发送数据
        lws_start_foreach_llp(pss_http_ws_live **, ppss, m_h264PssList) {
            lws_callback_on_writable((*ppss)->wsi);
        } lws_end_foreach_llp(ppss, pss_next);
    }

    void CLiveWorker::push_ts_stream(char* pBuff, int nBuffSize)
    {

    }

    void CLiveWorker::stop()
    {

    }

    FLV_TAG_BUFF CLiveWorker::GetFlvHeader()
    {
        FLV_TAG_BUFF ret;
        memset(&ret,0,sizeof(ret));

        if (m_pScriptTag.pBuff == nullptr || m_pDecodeConfig.pBuff == nullptr)
            return ret;
        if (m_flvHeader.pBuff == nullptr) {
            IlibLive::MakeFlvHeader(&m_flvHeader.pBuff,&m_flvHeader.nLen);
        }

        ret.nLen = m_flvHeader.nLen + m_pScriptTag.nLen + m_pDecodeConfig.nLen;
        ret.pBuff = (char *)malloc(ret.nLen);

        int nPos = 0;
        memcpy(ret.pBuff + nPos, m_flvHeader.pBuff, m_flvHeader.nLen);         //flv header
        nPos += m_flvHeader.nLen;
        memcpy(ret.pBuff + nPos, m_pScriptTag.pBuff, m_pScriptTag.nLen);       //flv script
        nPos += m_pScriptTag.nLen;
        memcpy(ret.pBuff + nPos, m_pDecodeConfig.pBuff, m_pDecodeConfig.nLen); //flv decode config

        return ret;
    }

    FLV_TAG_BUFF CLiveWorker::GetFlvVideo(uint32_t *tail)
    {
        FLV_TAG_BUFF ret = {nullptr,0,callback_flv_header};
        FLV_TAG_BUFF* tag = (FLV_TAG_BUFF*)lws_ring_get_element(m_flvRing, tail);
        if(tag) ret = *tag;

        return ret;
    }

    H264_NALU_BUFF CLiveWorker::GetH264Video(uint32_t *tail)
    {
        H264_NALU_BUFF ret = {nullptr,0,unknow};
        H264_NALU_BUFF* tag = (H264_NALU_BUFF*)lws_ring_get_element(m_h264Ring, tail);
        if(tag) ret = *tag;

        return ret;
    }

    void CLiveWorker::NextWork(pss_http_ws_live* pss)
    {
        struct lws_ring *ring;
        pss_http_ws_live* pssList;
        if(pss->media_type == media_flv) {
            ring = m_flvRing;
            pssList = m_flvPssList;
        } else if(pss->media_type == media_h264){
            ring = m_h264Ring;
            pssList = m_h264PssList;
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
            ring = m_flvRing;
            pssList = m_flvPssList;
        } else {
            ring = m_h264Ring;
            pssList = m_h264PssList;
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

    CLiveWorker* CreatLiveWorker(string strCode)
    {
        Log::debug("CreatFlvBuffer begin");

        CLiveWorker* pNew = new CLiveWorker(strCode);
        {
            MutexLock lock(&m_cs);
            m_workerMap.insert(make_pair(strCode, pNew));
            Log::debug("new flv buffer ok");
        }

        if(!SipInstance::RealPlay(strCode))
        {
            DelLiveWorker(strCode);
            Log::error("play failed %s",strCode.c_str());
            return nullptr;
        }
        Log::debug("RealPlay ok: %s",strCode.c_str());

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
            SAFE_DELETE(itFind->second);
            m_workerMap.erase(itFind);
            return true;
        }

        Log::error("dosn't find worker object");
        return false;
    }
}