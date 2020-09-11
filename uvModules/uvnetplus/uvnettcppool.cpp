#include "uvnetplus.h"
#include "uvnettcppool.h"
#include "utilc.h"
#include "util.h"
#include <time.h>
#include <sstream>

namespace uvNetPlus {

static void OnClientReady(CTcpSocket* skt){
    //Log::debug("client ready");
}

static void OnClientConnect(CTcpSocket* skt, string err){
    CUNTcpPoolSocket *pskt = (CUNTcpPoolSocket*)skt;
    CTcpRequest *req = pskt->m_pReq;
    if(err.empty()) {
        if(pskt->m_pAgent->m_pTcpConnPool->OnRequest)
            pskt->m_pAgent->m_pTcpConnPool->OnRequest(req, skt);
    } else {
        if(pskt->m_pAgent->m_pTcpConnPool->OnError)
            pskt->m_pAgent->m_pTcpConnPool->OnError(req, err);
        skt->Delete();
    }
    if(req->autodel)
        delete req;
}

//socket远端关闭或异常
static void OnClientEnd(CTcpSocket* skt) {
    CUNTcpPoolSocket *pskt = (CUNTcpPoolSocket*)skt;
    Log::debug("pool client end %llu", pskt->fd);
    pskt->syncClose();
}

//////////////////////////////////////////////////////////////////////////
//////////////   连接到服务器的单个连接    ///////////////////////////////

CUNTcpPoolSocket::CUNTcpPoolSocket(CUVNetPlus* net)
    : CUNTcpSocket(net)
    , m_bBusy(false)
{
}

CUNTcpPoolSocket::~CUNTcpPoolSocket()
{
    //Log::debug("~CUNTcpPoolSocket()");
}

void CUNTcpPoolSocket::syncClose()
{
    m_pReq = nullptr;
    if(m_pAgent)
        m_pAgent->CloseAgentSkt(this);
    else
        CUNTcpSocket::syncClose();
}

void CUNTcpPoolSocket::Delete()
{
    m_pNet->AddEvent(ASYNC_EVENT_TCP_CONNCLOSE, this);
}

//////////////////////////////////////////////////////////////////////////
/** 指定地址的连接管理 */

static void on_uv_getaddrinfo(uv_getaddrinfo_t* req, int status, struct addrinfo* res) {
    CTcpPoolAgent *agent = (CTcpPoolAgent*)req->data;
    delete req;
    agent->OnParseHost(status, res);
    uv_freeaddrinfo(res);
}


void CTcpPoolAgent::Delete()
{
    delete this;
}

CTcpPoolAgent::CTcpPoolAgent(CUVNetPlus* net, CUNTcpConnPool *p)
    : maxConns(512)
    , maxIdle(100)
    , timeOut(20)
    , maxRequest(0)
    , m_pNet(net)
    , m_pTcpConnPool(p)
{
}

CTcpPoolAgent::~CTcpPoolAgent() {
    for(auto req : m_listReqs) {
        delete req;
    }
    m_listReqs.clear();
    for(auto skt : m_listIdleConns) {
        skt->m_pAgent = NULL;
        skt->Delete();
    }
    m_listIdleConns.clear();
    for(auto skt : m_listBusyConns) {
        skt->m_pAgent = NULL;
        skt->Delete();
    }
    m_listBusyConns.clear();
    m_pNet->RemoveEvent(this);
    Log::debug("~CTcpPoolAgent()");
}

void CTcpPoolAgent::syncHostDns(string host){
    //判断host是域名还是ip
    if(net_is_ip(host.c_str()) > 0) {
        m_listIP.push_back(host);
    } else {
        //dns解析host
        uv_getaddrinfo_t *req = new uv_getaddrinfo_t;
        req->data = this;
        struct addrinfo hints;
        hints.ai_family = PF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = 0;
        uv_getaddrinfo(&m_pNet->m_uvLoop, req, on_uv_getaddrinfo, host.c_str(), NULL, &hints);
    }
}

void CTcpPoolAgent::OnParseHost(int status, struct addrinfo* res) {
    if(status < 0) {
        return;
    }

    while(res) {
        if(res->ai_family == PF_INET) {
            char addr[17] = {0};
            uv_ip4_name((struct sockaddr_in*)res->ai_addr, addr, 17);
            m_listIP.push_back(addr);
        } else if(res->ai_family == PF_INET6) {
            char addr[46] = {0};
            uv_ip6_name((struct sockaddr_in6*)res->ai_addr, addr, 46);
            m_listIP.push_back(addr);
        }
        res = res->ai_next;
    }

    Request((CTcpRequest*)NULL);
}

bool CTcpPoolAgent::Request(CTcpRequest *req) {
    //外部请求先放到请求列表
    if(req) {
        if(maxRequest > 0 && m_listReqs.size() > maxRequest) {
            //Log::debug("HttpReq:%x TcpReq:%x", req->usr, req);
            if(m_pTcpConnPool->OnError)
                m_pTcpConnPool->OnError(req, "request list is max");
            if(req->autodel)
                delete req;
            return false;
        }
        m_listReqs.push_back(req);
    }

    //host解析尚未成功, 没有需要处理的请求
    if(m_listIP.empty() || m_listReqs.empty()){
        return false;
    }

    //没有空闲连接
    //Log::debug("busy %d idle %d max %d", m_listBusyConns.size(), m_listIdleConns.size(), m_nMaxConns);
    if(m_listBusyConns.size() >= maxConns){
        //连接总数达到上限
        //Log::debug("busy conn is max");
        return false;
    }

    // 取出第一个请求
    req = m_listReqs.front();
    m_listReqs.pop_front();
    //Log::debug("busyConn %d idleConn %d", m_listBusyConns.size(), m_listIdleConns.size());
    //查找空闲连接
    if(!m_listIdleConns.empty()){
        CUNTcpPoolSocket *skt = m_listIdleConns.front();
        m_listIdleConns.pop_front();

        //处理下一个请求
        m_pNet->AddEvent(ASYNC_EVENT_TCPAGENT_REQUEST, this);

        // 使用现有的连接进行发送请求
        //Log::debug("use a idle connect send");
        skt->m_nSendTime = time(NULL);
        skt->m_bBusy     = true;
        m_listBusyConns.push_back(skt);
        skt->m_pReq = req;

        //回调通知用户取到的socket
        if(m_pTcpConnPool->OnRequest)
            m_pTcpConnPool->OnRequest(req, skt);
        if(req->autodel)
            delete req;
    } else {
        //连接总数未达上限，可以新增l个连接
        //Log::debug("create a new connect and send");
        // 创建连接，并在连接成功后发送请求
        string ip = m_listIP.front();
        m_listIP.pop_front();
        m_listIP.push_back(ip);

        //处理下一个请求
        m_pNet->AddEvent(ASYNC_EVENT_TCPAGENT_REQUEST, this);

        CUNTcpPoolSocket *skt = new CUNTcpPoolSocket(m_pNet);
        skt->m_nSendTime = time(NULL);
        skt->m_pReq      = req;
        skt->m_pAgent    = this;
        skt->m_bBusy     = true;
        skt->OnReady     = OnClientReady;
        skt->OnConnect   = OnClientConnect;
        skt->OnEnd       = OnClientEnd;
        skt->copy        = req->copy;
        m_listBusyConns.push_back(skt);

        //回调通知用户取到的socket
        //if(m_pTcpConnPool->OnRequest)
        //    m_pTcpConnPool->OnRequest(req, skt, false);
        //if(req->autodel)
        //    delete req;

        skt->Connect(ip, port);
    }

    return true;
}

void CTcpPoolAgent::CloseAgentSkt(CUNTcpPoolSocket *skt) {
    if(skt->m_bBusy){
        m_listBusyConns.remove(skt);
        skt->m_bBusy = false;
        if(skt->m_bConnect && (m_listIdleConns.size() < maxIdle || !m_listReqs.empty())){
            m_listIdleConns.push_front(skt);
        } else {
            skt->CUNTcpSocket::syncClose();
        }
        m_pNet->AddEvent(ASYNC_EVENT_TCPAGENT_REQUEST, this);
    } else {
        m_listIdleConns.remove(skt);
        skt->CUNTcpSocket::syncClose();
    }
}

//////////////////////////////////////////////////////////////////////////
/** TCP客户端连接池 */

static void on_timer_cb(uv_timer_t* handle) {
    CUNTcpConnPool *pool = (CUNTcpConnPool*)handle->data;
    time_t now = time(NULL);
    //遍历所有agent
    for(auto it = pool->m_mapAgents.begin(); it != pool->m_mapAgents.end(); ){
        CTcpPoolAgent* agent = it->second;

        //timeout设置了超时，需要将空闲连接中空闲时间过长的连接断开
        while(!agent->m_listIdleConns.empty() && agent->timeOut){
            CUNTcpPoolSocket *conn = agent->m_listIdleConns.back();
            if(difftime(now, conn->m_nSendTime) < agent->timeOut)
                break;

            // 空闲连接超时
            //delete conn;
            conn->CUNTcpSocket::Delete();
            agent->m_listIdleConns.pop_back();
        }

        // 如果某个agent中没有连接、请求，则将其清除
        if(agent->m_listBusyConns.empty()
            && agent->m_listIdleConns.empty()
            && agent->m_listReqs.empty()){
                delete agent;
                it = pool->m_mapAgents.erase(it);
        } else {
            it++;
        }
    }
}

static void on_timer_close(uv_handle_t* handle) {
    uv_timer_t* t = (uv_timer_t*)handle;
    delete t;
}

CTcpConnPool::CTcpConnPool()
    : maxConns(512)
    , maxIdle(100)
    , timeOut(20)
    , maxRequest(0)
    , OnRequest(NULL)
{}

CTcpConnPool::~CTcpConnPool(){}

CTcpConnPool* CTcpConnPool::Create(CNet* net, ReqCB onReq)
{
    CUNTcpConnPool *pool = new CUNTcpConnPool((CUVNetPlus*)net);
    pool->OnRequest = onReq;
    return pool;
}

CUNTcpConnPool::CUNTcpConnPool(CUVNetPlus* net)
    : m_pNet(net)
{
    uv_mutex_init(&m_ReqMtx);
    m_pNet->AddEvent(ASYNC_EVENT_TCPCONN_INIT, this);
}

CUNTcpConnPool::~CUNTcpConnPool()
{
    uv_mutex_lock(&m_ReqMtx);
    for(auto req : m_listReqs) {
        delete req;
    }
    m_listReqs.clear();
    uv_mutex_unlock(&m_ReqMtx);
    for(auto pair : m_mapAgents) {
        pair.second->Delete();
    }
    m_mapAgents.clear();
    uv_mutex_destroy(&m_ReqMtx);
    m_pNet->RemoveEvent(this);
}

void CUNTcpConnPool::syncInit()
{
    m_uvTimer = new uv_timer_t;
    m_uvTimer->data = this;
    uv_timer_init(&m_pNet->m_uvLoop, m_uvTimer);
    uv_timer_start(m_uvTimer, on_timer_cb, 5000, 5000);
}

void CUNTcpConnPool::syncRequest()
{
    //取出第一个请求
    CTcpRequest* req=nullptr;
    uv_mutex_lock(&m_ReqMtx);
    while(!m_listReqs.empty()) {
        req = m_listReqs.front();
        m_listReqs.pop_front();

        //查找agent
        CTcpPoolAgent* agent = nullptr;
        stringstream ss;
        ss << req->host << ":" << req->port;
        auto agtfind = m_mapAgents.find(ss.str());
        if (agtfind == m_mapAgents.end()) {
            // 没有找到agent，需要新建
            agent = new CTcpPoolAgent(m_pNet, this);
            agent->host = req->host;
            agent->port = req->port;
            agent->localaddr = req->localaddr;
            agent->maxConns = maxConns;
            agent->maxIdle = maxIdle;
            agent->timeOut = timeOut;
            agent->maxRequest = maxRequest;
            agent->syncHostDns(req->host);
            m_mapAgents.insert(make_pair(ss.str(), agent));
        } else {
            // 找到已有agent
            agent = agtfind->second;
        }
        agent->Request(req);
    }
    uv_mutex_unlock(&m_ReqMtx);
}

void CUNTcpConnPool::syncClose()
{
    uv_timer_stop(m_uvTimer);
    uv_close((uv_handle_t*)m_uvTimer, on_timer_close);
    delete this;
}

void CUNTcpConnPool::Delete()
{
    m_pNet->AddEvent(ASYNC_EVENT_TCPCONN_CLOSE, this);
}

bool CUNTcpConnPool::Request(std::string host, uint32_t port, std::string localaddr
                 , void *usr/*=nullptr*/, bool copy/*=true*/, bool recv/*=true*/)
{
    CTcpRequest *req = new CTcpRequest();
    req->host = host;
    req->port = port;
    req->copy = copy;
    req->recv = recv;
    req->usr = usr;
    //Log::debug("1HttpReq:%x TcpReq:%x", req->usr, req);

    return Request(req);
}

bool CUNTcpConnPool::Request(CTcpRequest *req)
{
    //Log::debug("2HttpReq:%x TcpReq:%x", req->usr, req);
    req->pool = this;
    uv_mutex_lock(&m_ReqMtx);
    m_listReqs.push_back(req);
    uv_mutex_unlock(&m_ReqMtx);
    m_pNet->AddEvent(ASYNC_EVENT_TCPCONN_REQUEST, this);
    return true;
}


}