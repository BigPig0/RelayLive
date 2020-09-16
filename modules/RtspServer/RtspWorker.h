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

        /** �ͻ������� */
        bool AddConnect(pss_rtsp_client* pss);
        bool DelConnect(pss_rtsp_client* pss);

        /** ����˻�ȡ��Ƶ���� */
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
        string                m_strCode;     // ����ý����

        /**
        * lws_ring�������λ�������ֻ��һ���߳�д�룬һ���̶߳�ȡ
        * m_pRing��liveworker�е�uv_loop�߳�д�룬http�������ڵ�uv_loop�̶߳�ȡ
        */
        AV_BUFF               m_stHead;
        ring_buff_t           *m_pRing;
        pss_rtsp_client       *m_pPssList;

        int                   m_nType;          //< 0:liveֱ����1:record��ʷ��Ƶ
        LiveClient::ILiveWorker *m_pLive;         //< ֱ�����ݽ��պͽ��װ��
        HandleType            m_type;           //< ��������һ������
    };

    /** ֱ�� */
    CRtspWorker* CreatRtspWorker(string strCode);
    CRtspWorker* GetRtspWorker(string strCode);
    bool DelRtspWorker(string strCode);
};