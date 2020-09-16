#pragma once
#include "LiveClient.h"

namespace HttpWsServer
{
    struct pss_http_ws_live;
    //enum MediaType;

    typedef struct _TS_BUFF_ {
        uint64_t      nID;            // ��0��ʼ��������Ƶtag��ID
        AV_BUFF       buff;
    }TS_BUFF;

    class CHlsWorker : public LiveClient::ILiveHandle
    {
    public:
        CHlsWorker(string strCode);
        ~CHlsWorker();

        /** �ͻ������� */
        bool AddConnect(pss_http_ws_live* pss);
        bool DelConnect(pss_http_ws_live* pss);

        /** ����˻�ȡ��Ƶ���� */
        AV_BUFF GetHeader();
        AV_BUFF GetVideo(uint64_t id);

        virtual void play_answer(int ret, string error_info);
        virtual void push_video_stream(AV_BUFF buff);
        virtual void stop();
        virtual LiveClient::ClientInfo get_clients_info();

    private:
        string                m_strCode;     // ����ý����
        list<TS_BUFF>         m_listTs;         // ��Ƶ���ݻ���
        //SlimRWLock            m_rwLock;         // list��ͬ����
        uint64_t              m_nID;            // ��¼ID

        uint64_t              m_nLastVistTime;  // �ϴη��ʵ�ʱ��
        bool                  m_bRun;
        
        pss_http_ws_live      *m_pPssList;      //< û�л���ʱ�������󱣴�

        int                   m_nType;          //< 0:liveֱ����1:record��ʷ��Ƶ
        LiveClient::ILiveWorker *m_pLive;         //< ֱ�����ݽ��պͽ��װ��
    };

    /** ֱ�� */
    CHlsWorker* CreatHlsWorker(string strCode);
    CHlsWorker* GetHlsWorker(string strCode);
    bool DelHlsWorker(string strCode);

    /** �㲥 */
};