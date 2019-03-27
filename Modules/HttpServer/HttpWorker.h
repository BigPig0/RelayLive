#pragma once
#include "LiveClient.h"

namespace HttpWsServer
{
    struct pss_http_ws_live;
    enum MediaType;

    struct LIVE_BUFF {
        char *pBuff;
        int   nLen;
    };

    class CHttpWorker : public LiveClient::ILiveHandle
    {
    public:
        CHttpWorker(string strCode, HandleType t);
        ~CHttpWorker();

        /** 客户端连接 */
        bool AddConnect(pss_http_ws_live* pss);
        bool DelConnect(pss_http_ws_live* pss);

        /** 请求端获取视频数据 */
        LIVE_BUFF GetHeader();
        LIVE_BUFF GetVideo(uint32_t *tail);
        void NextWork(pss_http_ws_live* pss);

        virtual void push_video_stream(char* pBuff, int nLen);
        virtual void stop();
        virtual string get_clients_info();
    private:
        void cull_lagging_clients();

    private:
        string                m_strCode;     // 播放媒体编号

        /**
         * lws_ring无锁环形缓冲区，只能一个线程写入，一个线程读取
         * m_pRing由liveworker中的uv_loop线程写入，http服务所在的uv_loop线程读取
         */
        LIVE_BUFF             m_stHead;
        struct lws_ring       *m_pRing;
        pss_http_ws_live      *m_pPssList;

        int                   m_nType;          //< 0:live直播；1:record历史视频
        LiveClient::ILiveWorker *m_pLive;         //< 直播数据接收和解包装包
        HandleType            m_type;           //< 表明是哪一种类型
    };

    /** 直播 */
    CHttpWorker* CreatHttpWorker(string strCode, HandleType t);
    CHttpWorker* GetHttpWorker(string strCode, HandleType t);
    bool DelHttpWorker(string strCode, HandleType t);

    /** 点播 */

    /** 获取播放信息，返回json */
    string GetClientsInfo();
};