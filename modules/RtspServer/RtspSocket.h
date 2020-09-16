/*!
 * \file RtspSocket.h
 * \date 2019/06/10 15:51
 *
 * \author wlla
 * Contact: user@company.com
 *
 * \brief socket�շ�����
 *
 * TODO: rtsp����˼���socket������socket���Լ�rtp��rtcp��udp socket��ش���
 *
 * \note
*/
#pragma once

#include "uv.h"
#include "rtsp.h"
#include "RtspSession.h"

namespace RtspServer
{
    class CRtspServer;
    class CRtspSocket;

    typedef enum _RTSP_REASON_
    {
        RTSP_REASON_ERROR = 0,
        // rtsp���󷽷�
        RTSP_REASON_OPTIONS,
        RTSP_REASON_DESCRIBE,
        RTSP_REASON_SETUP,
        RTSP_REASON_PLAY,
        RTSP_REASON_PAUSE,
        RTSP_REASON_TEARDOWN,
        // �����¼�
        RTSP_REASON_CLOSE,
        RTSP_REASON_WRITE,
        RTSP_REASON_RTP_WRITE,
        RTSP_REASON_RTCP_WRITE
    }RTSP_REASON;

    /** rtsp��������������� */
    //typedef struct _rtsp_ruquest_
    //{
    //    rtsp_method     method;
    //    string          uri;
    //    bool            parse_status;
    //    response_code   code;
    //    uint64_t        CSeq;
    //    uint32_t        rtp_port;
    //    uint32_t        rtcp_port;
    //    map<string,string> headers;
    //}rtsp_ruquest;

    /** rtspӦ������ */
    typedef struct _rtsp_response_
    {
        response_code   code;
        uint32_t        CSeq;
        string          body;
        unordered_map<string,string> headers;
    }rtsp_response;

    /** �첽�¼� */
    typedef struct _rtsp_event_
    {
        RTSP_REASON  resaon;
    }rtsp_event;

    /** ���������� */
    typedef int(*live_rtsp_cb)(CRtspSocket *client, RTSP_REASON reason, void *user);
    struct rtsp_options
    {
        string ip;         //����IP
        int port;          //�����˿�
        int rtp_port;      //rtp�����󶨵���ʼ�˿�
        int rtp_port_num;  //rtp�˿���
        int user_len;      //�û���Ϣ�ṹ�Ĵ�С
        live_rtsp_cb cb;   //�û��Զ���ص�������
    };

    /**
     * �ͻ������ӵ�socket
     */
    class CRtspSocket
    {
    public:
        CRtspSocket();
        ~CRtspSocket();

        int Init(uv_loop_t* uv);
        int Recv();
        void parse(char* buff, int len);
        int answer(rtsp_ruquest_t *req);
        void SetRemotePort(int rtp, int rtcp);

        rtsp           *m_pRtspParse;   //rtsp���Ľ�������
        uv_tcp_t        m_rtsp;         //rtsp���Ӿ��
        //uv_udp_t        m_rtp;          //rtp���;��
        //uv_udp_t        m_rtcp;         //rtcp���;��
        uv_async_t      m_async;        //�߳̾��
        uv_loop_t*      m_ploop;
        list<rtsp_event> m_asyncList;  //�첽�¼��б�
        uv_mutex_t      m_asyncMutex;
        CRtspServer*    m_server;      //�������
        //CRtspWorker*    m_pWorker;     //ҵ�����
        CRtspSession   *m_pSession;
        CRtspSessionMgr *m_pSessMgr;
        string          m_strDevCode;  //�豸����
        string          m_strRtpIP;    //�ͻ���IP
        int             m_nRtpPort;    //�ͻ��˽���rtp���ݵĶ˿�
        int             m_nRtcpPort;   //�ͻ��˽���rtcp���ݵĶ˿�
		string          m_strLocalIP;  //����IP
        int             m_nLocalPort;  //���ط���rtp�Ķ˿�
        void*           m_user;        //�û�����
        struct sockaddr_in m_addrRtp;

        rtsp_ruquest_t* m_Request;
        rtsp_response*  m_Response;
    };

    /**
     * ����˼���socket
     */
    class CRtspServer
    {
    public:
        CRtspServer(rtsp_options options);
        ~CRtspServer(void);

        int Init(uv_loop_t* uv);

        uv_tcp_t        m_tcp;
        uv_loop_t*      m_ploop;

        rtsp_options    m_options;

        int GetRtpPort();
        void GiveBackRtpPort(int nPort);
        vector<int>     m_vecRtpPort;     //< RTP���ö˿ڣ�ʹ��ʱ����ȡ����ʹ�ý������·���
        CriticalSection m_csRTP;          //< RTP�˿���
    };


    

    /** �̰߳�ȫ�ķ�����֪ͨrtsploopִ��һ��RTP_WRITE�ص� */
    extern int rtp_callback_on_writable(CRtspSocket *client);

    /** �ڻص������в���ʹ�ã�����rtp���� */
    extern int rtp_write(CRtspSocket *client, char* buff, int len);

}