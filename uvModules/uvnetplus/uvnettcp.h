#pragma once
#include "uvnetplus.h"
#include "uvnetprivate.h"
#include <stdint.h>

namespace uvNetPlus {

extern bool net_is_ipv4(const char* input);
extern bool net_is_ipv6(const char* input);
extern int  net_is_ip(const char* input);

class CUNTcpSocket;
class CUNTcpServer;
class CUNTcpAgent;

//////////////////////////////////////////////////////////////////////////

class CUNTcpSocket : public CTcpSocket
{
public:
    CUNTcpSocket(CUVNetPlus* net);
    ~CUNTcpSocket();
    virtual void Delete();
    virtual void Connect(std::string strIP, uint32_t nPort);
    virtual void SetLocal(std::string strIP, uint32_t nPort);
    virtual void GetLocal(std::string &strIP, uint32_t &nPort);
    virtual void Send(const char *pData, uint32_t nLen);

    void syncInit();
    void syncConnect();
    void syncSend();
    void syncClose();

public:
    CUVNetPlus       *m_pNet;      //事件线程句柄
    CUNTcpServer     *m_pSvr;      //客户端实例为null，服务端实例指向监听服务句柄
    uv_tcp_t          uvTcp;

    string            m_strRemoteIP; //远端ip
    uint32_t          m_nRemotePort; //远端端口
    string            m_strLocalIP;  //本地ip
    uint32_t          m_nLocalPort;  //本地端口
    bool              m_bSetLocal;   //作为客户端时，是否设置本地绑定信息
    bool              m_bInit;       //是否初始化uv_tcp_t对象
    bool              m_bConnect;    //是否已经成功连接服务器

    char             *readBuff;         // 接收缓存
    uint32_t          bytesRead;        // 统计累计接收大小
    list<uv_buf_t>    sendList;         // 发送缓存
    list<uv_buf_t>    sendingList;      // 正在发送
    uv_mutex_t        sendMtx;          // 发送锁
    bool              m_bUserClose;     // 进入关闭流程
    time_t            m_nSendTime;      // 最后发送时间
    time_t            m_nRecvTime;      // 最后接收时间
};

//////////////////////////////////////////////////////////////////////////

class CUNTcpServer : public uvNetPlus::CTcpServer
{
public:
    CUNTcpServer(CUVNetPlus* net);
    ~CUNTcpServer();
    virtual void Delete();
    virtual bool Listen(std::string strIP, uint32_t nPort);
    virtual bool Listening();
    void syncListen();
    void syncConnection(uv_stream_t* server, int status);
    void syncClose();
    void removeClient(CUNTcpSocket* c);

public:
    CUVNetPlus       *m_pNet;
    uv_tcp_t          uvTcp;

    string            m_strLocalIP;
    uint32_t          m_nLocalPort;
    int               m_nBacklog;       //syns queue的大小，默认为512
    bool              m_bListening;     //开始监听时true，不在监听时为false
    int               m_nFamily;        //绑定本地IP的族 4 或 6


    list<CUNTcpSocket*> m_listClients;
};

//////////////////////////////////////////////////////////////////////////

class CUNTcpAgent : public CTcpAgent
{
public:
    CUNTcpAgent(CUVNetPlus* net);
    ~CUNTcpAgent();

    virtual bool Put(CTcpSocket *skt);
    virtual bool Remove(CTcpSocket *skt);
    virtual void Delete();

    void syncInit();
    void syncClose();

public:
    CUVNetPlus           *m_pNet;         //事件线程句柄
    list<CUNTcpSocket*>   m_listIdleConns;    //连接 front时间较久 back时间较新

    uv_timer_t           *m_uvTimer;     //定时器用来判断空闲连接是否超时
};

}