/*!
 * \file RtspSocket.h
 * \date 2019/06/10 15:51
 *
 * \author wlla
 * Contact: user@company.com
 *
 * \brief socket收发处理
 *
 * TODO: rtsp服务端监听socket和连接socket。以及rtp和rtcp的udp socket相关处理。
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
        // rtsp请求方法
        RTSP_REASON_OPTIONS,
        RTSP_REASON_DESCRIBE,
        RTSP_REASON_SETUP,
        RTSP_REASON_PLAY,
        RTSP_REASON_PAUSE,
        RTSP_REASON_TEARDOWN,
        // 各种事件
        RTSP_REASON_CLOSE,
        RTSP_REASON_WRITE,
        RTSP_REASON_RTP_WRITE,
        RTSP_REASON_RTCP_WRITE
    }RTSP_REASON;

    /** rtsp请求解析出的数据 */
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

    /** rtsp应答内容 */
    typedef struct _rtsp_response_
    {
        response_code   code;
        uint32_t        CSeq;
        string          body;
        unordered_map<string,string> headers;
    }rtsp_response;

    /** 异步事件 */
    typedef struct _rtsp_event_
    {
        RTSP_REASON  resaon;
    }rtsp_event;

    /** 服务器配置 */
    typedef int(*live_rtsp_cb)(CRtspSocket *client, RTSP_REASON reason, void *user);
    struct rtsp_options
    {
        string ip;         //监听IP
        int port;          //监听端口
        int rtp_port;      //rtp本定绑定的起始端口
        int rtp_port_num;  //rtp端口数
        int user_len;      //用户信息结构的大小
        live_rtsp_cb cb;   //用户自定义回调处理方法
    };

    /**
     * 客户端连接的socket
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

        rtsp           *m_pRtspParse;   //rtsp报文解析工具
        uv_tcp_t        m_rtsp;         //rtsp连接句柄
        //uv_udp_t        m_rtp;          //rtp发送句柄
        //uv_udp_t        m_rtcp;         //rtcp发送句柄
        uv_async_t      m_async;        //线程句柄
        uv_loop_t*      m_ploop;
        list<rtsp_event> m_asyncList;  //异步事件列表
        uv_mutex_t      m_asyncMutex;
        CRtspServer*    m_server;      //服务对象
        //CRtspWorker*    m_pWorker;     //业务对象
        CRtspSession   *m_pSession;
        CRtspSessionMgr *m_pSessMgr;
        string          m_strDevCode;  //设备编码
        string          m_strRtpIP;    //客户端IP
        int             m_nRtpPort;    //客户端接收rtp数据的端口
        int             m_nRtcpPort;   //客户端接收rtcp数据的端口
		string          m_strLocalIP;  //本地IP
        int             m_nLocalPort;  //本地发送rtp的端口
        void*           m_user;        //用户数据
        struct sockaddr_in m_addrRtp;

        rtsp_ruquest_t* m_Request;
        rtsp_response*  m_Response;
    };

    /**
     * 服务端监听socket
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
        vector<int>     m_vecRtpPort;     //< RTP可用端口，使用时从中取出，使用结束重新放入
        CriticalSection m_csRTP;          //< RTP端口锁
    };


    

    /** 线程安全的方法，通知rtsploop执行一次RTP_WRITE回调 */
    extern int rtp_callback_on_writable(CRtspSocket *client);

    /** 在回调方法中才能使用，发送rtp数据 */
    extern int rtp_write(CRtspSocket *client, char* buff, int len);

}