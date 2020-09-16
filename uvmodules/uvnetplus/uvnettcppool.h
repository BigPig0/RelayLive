/*!
 * \file uvnettcppool.h
 * \date 2019/09/05 16:11
 *
 * \author wlla
 * Contact: user@company.com
 *
 * \brief 
 *
 * TODO: 实现CTcpConnect接口。
 * 功能是向指定地址发送、接收数据。内部维护连接池，外部只可见请求和应答。
 * 可以设置只发送不应答、发送应答。有应答时外部判断应答完成
 *
 * \note
*/

#pragma once
#include "uvnetplus.h"
#include "uvnettcp.h"
#include <list>
#include <unordered_map>
using namespace std;

namespace uvNetPlus {

class CUNTcpPoolSocket;
class CTcpPoolAgent;
class CUNTcpConnPool;

enum ConnState {
    ConnState_Init = 0, //新创建的连接
    ConnState_Idle,     //空闲，在空闲列表中
    ConnState_snd,      //发送请求
    ConnState_sndok,    //发送完成
    ConnState_rcv,      //收到应答
};

/**
 * tcp客户端连接类
 */
class CUNTcpPoolSocket : public CUNTcpSocket {
public:
    CUNTcpPoolSocket(CUVNetPlus* net);
    ~CUNTcpPoolSocket();

    void syncClose();

    virtual void Delete();

    CTcpPoolAgent  *m_pAgent;     //连接所在的agent
    CUNTcpConnPool *connPool;     //连接所在的连接池
    CTcpRequest    *m_pReq;       //当前执行的请求
    bool            m_bBusy;      //true:使用中 false:空闲中
};

/**
 * tcp客户端agent类，管理一个同一个地址的连接列表
 * 所有的连接都是同样的host和端口，一个host可以是不同的ip
 */
class CTcpPoolAgent {
public:
    CTcpPoolAgent(CUVNetPlus* net, CUNTcpConnPool *p);
    ~CTcpPoolAgent();

    /** 通过dns解析域名对应的ip */
    void syncHostDns(string host);

    /** dns解析域名成功 */
    void OnParseHost(int status, struct addrinfo* res);

    void Delete();

    /** 根据请求参数，获取连接 */
    bool Request(CTcpRequest *req);

    /** 返还使用完毕的socket */
    void CloseAgentSkt(CUNTcpPoolSocket *skt);

public:
    string     host;
    uint32_t   port;
    string     localaddr;
    uint32_t   maxConns;    //最大连接数 默认512(busy+idle)
    uint32_t   maxIdle;     //最大空闲连接数 默认100
    uint32_t   timeOut;     //空闲连接超时时间 秒 默认20s 0为永不超时
    uint32_t   maxRequest;  //连接达到最大时能存放的请求数 默认0 不限制

    CUVNetPlus         *m_pNet;                //事件线程句柄
    CUNTcpConnPool     *m_pTcpConnPool;        //所在的连接池

    list<string>              m_listIP;           //解析host得到的ip地址，轮流使用，如果不通会移除
    list<CUNTcpPoolSocket*>   m_listBusyConns;    //正在使用中的连接
    list<CUNTcpPoolSocket*>   m_listIdleConns;    //空闲连接 front时间较近 back时间较久
    list<CTcpRequest*>        m_listReqs;         //请求列表
};

/**
 * TCP客户端连接池
 */
class CUNTcpConnPool : public CTcpConnPool
{
public:
    CUNTcpConnPool(CUVNetPlus* net);
    ~CUNTcpConnPool();

    void syncInit();
    void syncRequest();
    void syncClose();

    virtual void Delete();

    virtual bool Request(std::string host, uint32_t port, std::string localaddr
        , void *usr=nullptr, bool copy=true, bool recv=true);

    virtual bool Request(CTcpRequest *req);

public:
    CUVNetPlus         *m_pNet;         //事件线程句柄

    list<CTcpRequest*>  m_listReqs;      //请求列表,暂存外部的请求
    uv_mutex_t          m_ReqMtx;        //请求列表锁

    uv_timer_t         *m_uvTimer;     //定时器用来判断空闲连接是否超时

    unordered_map<string, CTcpPoolAgent*>   m_mapAgents;
};
}