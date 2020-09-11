#include "uvnetplus.h"
#include "uvnetprivate.h"
#include "uvnettcp.h"
#include "uvnettcppool.h"
#include "utilc.h"
#include "util_log.h"

namespace uvNetPlus {

static void on_uv_async(uv_async_t* handle) {
    CUVNetPlus* h = (CUVNetPlus*)handle->data;
    h->AsyncEvent();
}

static void on_uv_close(uv_handle_t* handle) {
    CUVNetPlus* h = (CUVNetPlus*)handle->data;
    h->CloseHandle();
}

static void run_loop_thread(void* arg)
{
    CUVNetPlus* h = (CUVNetPlus*)arg;
    h->LoopThread();
}

CUVNetPlus::CUVNetPlus()
    : m_bRun(true)
    , m_bStop(false)
{
    uv_loop_init(&m_uvLoop);
    m_uvAsync.data = this;
    uv_async_init(&m_uvLoop, &m_uvAsync, on_uv_async);
    uv_mutex_init(&m_uvMtxAsEvts);
    uv_thread_t tid;
    uv_thread_create(&tid, run_loop_thread, this);
    
}

CUVNetPlus::~CUVNetPlus()
{
    uv_mutex_lock(&m_uvMtxAsEvts);
    m_listAsyncEvents.clear();
    UV_EVET ue = {ASYNC_EVENT_LOOP_CLOSE, this};
    m_listAsyncEvents.push_back(ue);
    uv_async_send(&m_uvAsync);
    uv_mutex_unlock(&m_uvMtxAsEvts);
    while(!m_bStop) { //等待loop线程结束
        sleep(10);
    }
    uv_mutex_destroy(&m_uvMtxAsEvts);
    uv_loop_close(&m_uvLoop);
}

void* CUVNetPlus::Loop() {
    return &m_uvLoop;
}

void CUVNetPlus::AddEvent(UV_ASYNC_EVENT e, void* param) {
    if(!m_bRun)
        return;
    UV_EVET ue = {e, param};
    uv_mutex_lock(&m_uvMtxAsEvts);
    m_listAsyncEvents.push_back(ue);
    uv_mutex_unlock(&m_uvMtxAsEvts);
    uv_async_send(&m_uvAsync);
}

void CUVNetPlus::RemoveEvent(void* param) {
    if(!m_bRun)
        return;
    uv_mutex_lock(&m_uvMtxAsEvts);
    auto it = m_listAsyncEvents.begin();
    auto end = m_listAsyncEvents.end();
    for(; it != end; ) {
        if(it->param == param) {
            it = m_listAsyncEvents.erase(it);
        } else {
            it++;
        }
    }
    uv_mutex_unlock(&m_uvMtxAsEvts);
}

void CUVNetPlus::LoopThread() {
    while (m_bRun) {
        uv_run(&m_uvLoop, UV_RUN_DEFAULT);
        sleep(10);
    }

    m_bStop = true;
}

void CUVNetPlus::AsyncEvent() {
    uv_mutex_lock(&m_uvMtxAsEvts);
    list<UV_EVET> tmp = m_listAsyncEvents;
    m_listAsyncEvents.clear();
    uv_mutex_unlock(&m_uvMtxAsEvts);

    for(auto e : tmp) {
        if(e.event == ASYNC_EVENT_LOOP_CLOSE) {
            uv_close((uv_handle_t*)&m_uvAsync, on_uv_close);
        } else if(e.event == ASYNC_EVENT_TCP_CONNECT) {
            CUNTcpSocket *tcp = (CUNTcpSocket*)e.param;
            tcp->syncConnect();
        } else if(e.event == ASYNC_EVENT_TCP_SEND) {
            CUNTcpSocket *tcp = (CUNTcpSocket*)e.param;
            tcp->syncSend();
        } else if(e.event == ASYNC_EVENT_TCP_LISTEN) {
            CUNTcpServer *tcp = (CUNTcpServer*)e.param;
            tcp->syncListen();
        } else if(e.event == ASYNC_EVENT_TCP_CLTCLOSE) {
            CUNTcpSocket *tcp = (CUNTcpSocket*)e.param;
            tcp->syncClose();
        } else if(e.event == ASYNC_EVENT_TCP_SVRCLOSE) {
            CUNTcpServer *tcp = (CUNTcpServer*)e.param;
            tcp->syncClose();
        } else if(e.event == ASYNC_EVENT_TCP_AGENT) {
            CUNTcpAgent *agent = (CUNTcpAgent*)e.param;
            agent->syncInit();
        } else if(e.event == ASYNC_EVENT_TCP_AGTCLOSE) {
            CUNTcpAgent *agent = (CUNTcpAgent*)e.param;
            agent->syncClose();
        } else if(e.event == ASYNC_EVENT_TCPCONN_INIT) {
            CUNTcpConnPool *pool = (CUNTcpConnPool*)e.param;
            pool->syncInit();
        } else if(e.event == ASYNC_EVENT_TCPCONN_REQUEST) {
            CUNTcpConnPool *pool = (CUNTcpConnPool*)e.param;
            pool->syncRequest();
        } else if(e.event == ASYNC_EVENT_TCP_CONNCLOSE) {
            CUNTcpPoolSocket *pool = (CUNTcpPoolSocket*)e.param;
            pool->syncClose();
        }else if(e.event == ASYNC_EVENT_TCPAGENT_REQUEST) {
            CTcpPoolAgent *pool = (CTcpPoolAgent*)e.param;
            pool->Request(NULL);
        } else if(e.event == ASYNC_EVENT_TCPCONN_CLOSE) {
            CUNTcpConnPool *pool = (CUNTcpConnPool*)e.param;
            pool->syncClose();
        }
    }
}

void CUVNetPlus::CloseHandle() {
    m_bRun = false;
    uv_mutex_lock(&m_uvMtxAsEvts);
    m_listAsyncEvents.clear();
    uv_mutex_unlock(&m_uvMtxAsEvts);
}

CNet* CNet::Create() {
    return new CUVNetPlus();
}
}