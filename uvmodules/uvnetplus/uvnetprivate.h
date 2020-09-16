#pragma once
#include "util_log.h"
#include "uv.h"
#include <list>

using namespace std;

namespace uvNetPlus {

enum UV_ASYNC_EVENT
{
    ASYNC_EVENT_LOOP_CLOSE = 0, //事件循环关闭
    ASYNC_EVENT_TCP_CLIENT,     //新建一个tcp客户端
    ASYNC_EVENT_TCP_CONNECT,    //tcp客户端连接
    ASYNC_EVENT_TCP_SEND,       //tcp发送数据
    ASYNC_EVENT_TCP_LISTEN,     //tcp服务端监听
    ASYNC_EVENT_TCP_CLTCLOSE,   //tcp客户端关闭
    ASYNC_EVENT_TCP_SVRCLOSE,   //tcp服务端关闭
    ASYNC_EVENT_TCP_AGENT,      //新建一个tcp agent
    ASYNC_EVENT_TCP_AGTCLOSE,   //tcp agent关闭
    ASYNC_EVENT_TCPCONN_INIT,   //tcp连接池初始化定时器
    ASYNC_EVENT_TCPCONN_REQUEST,//tcp连接池中获取socket
    ASYNC_EVENT_TCPCONN_CLOSE,  //tcp连接池关闭
    ASYNC_EVENT_TCPAGENT_REQUEST, //tcp agent中获取socket
    ASYNC_EVENT_TCP_CONNCLOSE,  //连接池中的socket返还
};

struct UV_EVET {
    UV_ASYNC_EVENT event;
    void* param;
};

class CUVNetPlus : public CNet
{
public:
    CUVNetPlus();
    ~CUVNetPlus();

    virtual void* Loop();

    /**
     * 添加loop事件
     * @param e 事件定义
     * @param param 事件发送者
     */
    void AddEvent(UV_ASYNC_EVENT e, void* param);

    /**
     * 移除某个发送者的事件，发送者析构必须调用
     */
    void RemoveEvent(void* param);

    /**
     * 事件循环线程
     */
    void LoopThread();

    /**
     * 异步事件处理
     */
    void AsyncEvent();

    /**
     * 事件句柄关闭
     */
    void CloseHandle();

    uv_loop_t       m_uvLoop;
private:
    bool            m_bRun;     //默认为true，析构时设置为false
    bool            m_bStop;    //默认为false，loop结束后设为true
    uv_async_t      m_uvAsync;
    list<UV_EVET>   m_listAsyncEvents;
    uv_mutex_t      m_uvMtxAsEvts;

    //set<CTcpSocket*> m_pTcpClients;
    //set<C
};

}