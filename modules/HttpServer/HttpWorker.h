#pragma once
#include "LiveClient.h"

namespace HttpWsServer
{
    struct pss_http_ws_live;
    enum MediaType;

    class CHttpWorker : public LiveClient::ILiveHandle
    {
    public:
        CHttpWorker(string strCode, HandleType t, int nChannel, bool isWs);
        ~CHttpWorker();

        /** ����˻�ȡ��Ƶ���� */
        int GetVideo(char **buff);

        virtual void play_answer(int ret, string error_info);

        /**
         * �ײ�����H264����
         */
        virtual void push_video_stream(AV_BUFF buff);

        /**
         * �ײ�֪ͨ���Źر�(����rtp��ʱ���Է��رյ�)
         */
        virtual void stop();

        /**
         * ��װý������
         */
        void MediaCb(AV_BUFF buff);

        virtual LiveClient::ClientInfo get_clients_info();

		void close();
    private:
        void cull_lagging_clients();

    public:
        pss_http_ws_live     *m_pPss;           //< ���ӻỰ
        string                m_strCode;        //< ����ý����
        HandleType            m_eHandleType;    //< ��������һ������
        string                m_strMIME;        //< mime type
        //MediaType             m_eMediaType;
        int                   m_nChannel;       //< ͨ�� 0:ԭʼ����  1:С����
        bool                  m_bWebSocket;     //< false:http����true:websocket

        string                m_strPath;        //< ���Ŷ������ַ
        string                m_strClientName;  //< ���Ŷ˵�����
        string                m_strClientIP;    //< ���Ŷ˵�ip
        string                m_strError;       //< sip���������صĲ�������ʧ��ԭ��

    private:
        LiveClient::ILiveWorker *m_pLive;       //< ����RTP���ݲ����264����
        void                    *m_pFormat;     //< ��Ƶ��ʽ���
        struct lws_ring         *m_pRing;       //< ����ý�����ݵĻ�����
        AV_BUFF                  m_SocketBuff;  //< socket���͵����ݻ���
		bool                     m_bConnect;       //<

        int                   m_nType;          //< 0:liveֱ����1:record��ʷ��Ƶ
    };

};