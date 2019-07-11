#pragma once
#include "LiveClient.h"

namespace HttpWsServer
{
    struct pss_http_ws_live;
    //enum MediaType;

    typedef struct _TS_BUFF_ {
        uint64_t      nID;            // 从0开始递增的视频tag的ID
        AV_BUFF       buff;
    }TS_BUFF;

    class CHlsWorker : public LiveClient::ILiveHandle
    {
    public:
        CHlsWorker(string strCode);
        ~CHlsWorker();

        /** 客户端连接 */
        bool AddConnect(pss_http_ws_live* pss);
        bool DelConnect(pss_http_ws_live* pss);

        /** 请求端获取视频数据 */
        AV_BUFF GetHeader();
        AV_BUFF GetVideo(uint64_t id);

        virtual void play_answer(int ret, string error_info);
        virtual void push_video_stream(AV_BUFF buff);
        virtual void stop();
        virtual LiveClient::ClientInfo get_clients_info();

    private:
        string                m_strCode;     // 播放媒体编号
        list<TS_BUFF>         m_listTs;         // 视频数据缓存
        //SlimRWLock            m_rwLock;         // list的同步锁
        uint64_t              m_nID;            // 记录ID

        uint64_t              m_nLastVistTime;  // 上次访问的时间
        bool                  m_bRun;
        
        pss_http_ws_live      *m_pPssList;      //< 没有缓存时，将请求保存

        int                   m_nType;          //< 0:live直播；1:record历史视频
        LiveClient::ILiveWorker *m_pLive;         //< 直播数据接收和解包装包
    };

    /** 直播 */
    CHlsWorker* CreatHlsWorker(string strCode);
    CHlsWorker* GetHlsWorker(string strCode);
    bool DelHlsWorker(string strCode);

    /** 点播 */
};