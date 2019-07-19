#pragma once
#include "LiveClient.h"
#include "RtspLiveServer.h"
#include "ring_buff.h"
#include "linked_list.h"

namespace RtspServer
{
    class CRtspWorker : public LiveClient::ILiveHandle
    {
    public:
        CRtspWorker(string strCode);
        ~CRtspWorker(void);

        /** 客户端连接 */
        bool AddConnect(pss_rtsp_client* pss);
        bool DelConnect(pss_rtsp_client* pss);

        /** 请求端获取视频数据 */
        AV_BUFF GetVideo(uint32_t *tail);
        void NextWork(pss_rtsp_client* pss);

        string GetSDP();

        virtual void play_answer(int ret, string error_info);
        virtual void push_video_stream(AV_BUFF buff);
        virtual void stop();
        virtual LiveClient::ClientInfo get_clients_info();
    private:
        void cull_lagging_clients();

    private:
        string                m_strCode;     // 播放媒体编号

        /**
        * lws_ring无锁环形缓冲区，只能一个线程写入，一个线程读取
        * m_pRing由liveworker中的uv_loop线程写入，http服务所在的uv_loop线程读取
        */
        AV_BUFF               m_stHead;
        ring_buff_t           *m_pRing;
        pss_rtsp_client       *m_pPssList;

        int                   m_nType;          //< 0:live直播；1:record历史视频
        LiveClient::ILiveWorker *m_pLive;         //< 直播数据接收和解包装包
        HandleType            m_type;           //< 表明是哪一种类型
    };

    /** 直播 */
    CRtspWorker* CreatRtspWorker(string strCode);
    CRtspWorker* GetRtspWorker(string strCode);
    bool DelRtspWorker(string strCode);
};