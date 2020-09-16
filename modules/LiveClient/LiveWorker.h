#pragma once
#include "LiveClient.h"
#include "avtypes.h"
#include "Recode.h"
#include "uv.h"
#include "rtp.h"

namespace LiveClient
{
    class CLiveReceiver;
    class CLiveChannel;
    struct live_event_loop_t;

    struct PlayAnswerList {
        struct PlayAnswerList *pNext;
        string strDevCode;  //�豸����
        int    nRet;        //�������󷵻�ֵ 0�ɹ� ��0ʧ��
        string strMark;     //ʧ��ʱ�������ԭ�򣬳ɹ�ʱ����sdp��Ϣ
    };

    class CLiveWorker : public ILiveWorker
    {
    public:
        CLiveWorker(string strCode, int rtpPort);
        ~CLiveWorker();

        /** �ͻ������� */
        virtual bool AddHandle(ILiveHandle* h, HandleType t, int c);
        bool AddHandleAsync(ILiveHandle* h, HandleType t, int c);

        /** �ͻ��˶Ͽ� */
        virtual bool RemoveHandle(ILiveHandle* h);
        bool RemoveHandleAsync(ILiveHandle* h);
        //virtual string GetSDP();

        /** ��ȡ�ͻ�����Ϣ */
        void GetClientInfo();
        void GetClientInfoAsync();

        /** ���յ�����Ƶ������ */
        void ReceiveStream(AV_BUFF buff);
        void ReceiveStreamAsync(AV_BUFF buff);


        /** �ͻ���ȫ���Ͽ�����ʱ������ʵ�� */
        void Clear2Stop();
        bool m_bStop;          //< ���붨ʱ���ص�����Ϊtrue��close��ʱ���ص������ٶ���
        bool m_bOver;          //< ��ʱ����Ϊtrue���ͻ���ȫ���Ͽ�����ʱ����������

#ifdef EXTEND_CHANNELS
        /** yuv��Ƶ���� */
        void ReceiveYUV(AV_BUFF buff);
#endif

        /** �������ݳ�ʱ����Ľ���������֪ͨ�������ӶϿ� */
        void stop();

        /** ���Ž���ص� */
        void PlayAnswer(PlayAnswerList *pal);

        /** ����SDP */
        void ParseSdp();

    public:
        int         m_nRunNum;  //������event loop����
        bool        m_bRtp;

    private:
        string                   m_strServerIP; // rtp���Ͷ�IP
        uint32_t                 m_nServerPort; // rtp���Ͷ˿�
        uint32_t                 m_nPort;       // rtp���ն˿�
        string                   m_strCode;     // ����ý����
        bool                     m_bPlayed;     // true:���������յ�Ӧ�� false:û���յ�Ӧ��
        int                      m_nPlayRes;    // ���ŷ��صĽ�� 0�ɹ�����0ʧ��
        string                   m_strPlayInfo; // sip���������ص�sdp��ʧ��ԭ������
        RTP_STREAM_TYPE          m_stream_type; // ��Ƶ������
        
        CLiveReceiver           *m_pReceiver;   // ֱ�����ݽ��պͽ��
        CLiveChannel            *m_pOrigin;     // ԭʼ��ͨ��

#ifdef EXTEND_CHANNELS
        map<int, CLiveChannel*>  m_mapChlEx;    // ��չͨ��
        IDecoder                *m_pDecoder;    // h264����
#endif

        vector<ILiveHandleRtp*>  m_vecLiveRtp;  // RTPԭʼ��ת��

        int                      m_nType;          //< 0:liveֱ����1:record��ʷ��Ƶ

        live_event_loop_t*       m_pEventLoop;
    };

    /** ����һ���豸�Ĳ���ʵ�� */
    extern CLiveWorker* CreatLiveWorker(string strCode);
    /** ��ȡָ���豸����ʵ�� */
    extern CLiveWorker* GetLiveWorker(string strCode);
    /** ����ָ���豸����ʵ�� */
    extern bool DelLiveWorker(string strCode);
    /** ��ȡ��ǰ���в���ʵ���Ŀͻ���������� */
	extern void GetAllWorkerClientsInfo();
    /** ��Ӧsip���������صĲ���Ӧ�� */
    extern bool AsyncPlayCB(PlayAnswerList *pal);
}