#include "stdafx.h"
#include "uv.h"
#include "HttpLiveServer.h"
#include "HlsWorker.h"
#include "LiveClient.h"

namespace HttpWsServer
{
    static map<string,CHlsWorker*>  m_workerMapHls;
    static CriticalSection          m_csHls;
    static map<string, uint64_t>    m_lives;
	static FILE *m_fall = NULL;

    //////////////////////////////////////////////////////////////////////////

    CHlsWorker::CHlsWorker(string strCode)
        : m_strCode(strCode)
        , m_nType(0)
		, m_nID(0)
        , m_pLive(nullptr){
        m_nLastVistTime = time(NULL);

        m_pLive = LiveClient::GetWorker(strCode);
        if(m_pLive)
            m_pLive->AddHandle(this, HandleType::ts_handle, 1);

		m_fall = fopen("all.ts","wb");
    }

    CHlsWorker::~CHlsWorker()
    {
        if(m_pLive)
            m_pLive->RemoveHandle(this);

        Log::debug("CHttpWorker release");
    }

    bool CHlsWorker::AddConnect(pss_http_ws_live* pss)
    {
        //pss->pss_next = m_pPssList;
        m_pPssList = pss;

        return true;
    }

    bool CHlsWorker::DelConnect(pss_http_ws_live* pss)
    {
        //lws_ll_fwd_remove(pss_http_ws_live, pss_next, pss, m_pPssList);
        return true;
    }

    AV_BUFF CHlsWorker::GetHeader()
    {
        m_nLastVistTime = time(nullptr);
        AV_BUFF ret = {AV_TYPE::TS, NULL, 0};

        if(m_listTs.empty())
            return ret;

        stringstream ss;
        ss << "#EXTM3U\n#EXT-X-TARGETDURATION:1\n";
        int nNum = 0;
        //MutexShareLock lock(&m_rwLock);
        list<TS_BUFF>::reverse_iterator rIt = m_listTs.rbegin();
        while(rIt != m_listTs.rend() && nNum < 10)
        {
            if (nNum == 0)
            {
                ss << "#EXT-X-MEDIA-SEQUENCE:" << rIt->nID << "\n\n";
            }
            ss << "#EXTINF:1\n" << m_strCode << "/" << rIt->nID << ".ts\n";
            rIt++;
            nNum++;
        }
        string strM3U8 = ss.str();
        ret.nLen = strM3U8.size();
        ret.pData = (char*)malloc(ret.nLen);
        memcpy(ret.pData, strM3U8.c_str(), strM3U8.size());

        return ret;
    }

    AV_BUFF CHlsWorker::GetVideo(uint64_t id)
    {
        m_nLastVistTime = time(nullptr);
        //MutexShareLock lock(&m_rwLock);

        for (auto rIt = m_listTs.rbegin(); rIt != m_listTs.rend(); ++rIt)
        {
            if (rIt->nID == id)
            {
                return rIt->buff;
            }
        }

        AV_BUFF ret = {AV_TYPE::NONE,nullptr,0};
        return ret;
    }

    void CHlsWorker::push_video_stream(AV_BUFF buff)
    {
        TS_BUFF ts;
        ts.nID = m_nID++;
        ts.buff = buff;
        m_listTs.push_back(ts);

		char tmp[100]={0};
		sprintf(tmp, "T%04d.ts", ts.nID);
		FILE* f = fopen(tmp, "wb");
		fwrite(ts.buff.pData, 1, ts.buff.nLen, f);
		fclose(f);
		fwrite(ts.buff.pData, 1, ts.buff.nLen, m_fall);
		fflush(m_fall);

        while (m_listTs.size() > 100)
        {
            TS_BUFF& firstTs = m_listTs.front();
            if (firstTs.buff.pData!=nullptr){
                free(firstTs.buff.pData);
            }
            m_listTs.pop_front();
        }

		//lws_start_foreach_llp_safe(pss_http_ws_live **, ppss, m_pPssList, pss_next) {
		//	lws_callback_on_writable((*ppss)->wsi);
        //} lws_end_foreach_llp_safe(ppss);
		m_pPssList = NULL;
    }

    void CHlsWorker::stop()
    {
        //视频源没有数据并超时
        Log::debug("no data recived any more, stopped");

        //断开所有客户端连接
        //lws_start_foreach_llp_safe(pss_http_ws_live **, ppss, m_pPssList, pss_next) {
        //    lws_set_timeout((*ppss)->wsi, PENDING_TIMEOUT_CLOSE_SEND, LWS_TO_KILL_ASYNC);
        //} lws_end_foreach_llp_safe(ppss);
    }

    LiveClient::ClientInfo CHlsWorker::get_clients_info()
    {
        //vector<LiveClient::ClientInfo> ret;
        //lws_start_foreach_llp(pss_http_ws_live **, ppss, m_pPssList) {
        //    LiveClient::ClientInfo info;
        //    info.devCode = m_strCode;
        //    info.connect = "HLS";
        //    info.media   = "TS";
        //    info.clientIP= (*ppss)->clientIP;
        //    ret.push_back(info);
        //} lws_end_foreach_llp(ppss, pss_next);
        //return ret;
        LiveClient::ClientInfo info;
        return info;
    }

    //////////////////////////////////////////////////////////////////////////

    CHlsWorker* CreatHlsWorker(string strCode)
    {
        Log::debug("CreatHlsWorker begin");
        CHlsWorker* pNew = new CHlsWorker(strCode);
        MutexLock lock(&m_csHls);
        m_workerMapHls.insert(make_pair(strCode, pNew));
        return pNew;
    }

    CHlsWorker* GetHlsWorker(string strCode)
    {
        //Log::debug("GetWorker begin");
        MutexLock lock(&m_csHls);
        auto itFind = m_workerMapHls.find(strCode);
        if (itFind != m_workerMapHls.end()) {
            return itFind->second;
        }
        //Log::error("GetWorker failed: %s",strCode.c_str());
        return nullptr;
    }

    bool DelHlsWorker(string strCode)
    {
        MutexLock lock(&m_csHls);
        auto itFind = m_workerMapHls.find(strCode);
        if (itFind != m_workerMapHls.end()) {
            SAFE_DELETE(itFind->second);
            m_workerMapHls.erase(itFind);
            return true;
        }
        Log::error("dosn't find worker object");
        return false;
    }

}