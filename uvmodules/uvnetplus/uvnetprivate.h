#pragma once
#include "util_log.h"
#include "uv.h"
#include <list>

using namespace std;

namespace uvNetPlus {

enum UV_ASYNC_EVENT
{
    ASYNC_EVENT_LOOP_CLOSE = 0, //�¼�ѭ���ر�
    ASYNC_EVENT_TCP_CLIENT,     //�½�һ��tcp�ͻ���
    ASYNC_EVENT_TCP_CONNECT,    //tcp�ͻ�������
    ASYNC_EVENT_TCP_SEND,       //tcp��������
    ASYNC_EVENT_TCP_LISTEN,     //tcp����˼���
    ASYNC_EVENT_TCP_CLTCLOSE,   //tcp�ͻ��˹ر�
    ASYNC_EVENT_TCP_SVRCLOSE,   //tcp����˹ر�
    ASYNC_EVENT_TCP_AGENT,      //�½�һ��tcp agent
    ASYNC_EVENT_TCP_AGTCLOSE,   //tcp agent�ر�
    ASYNC_EVENT_TCPCONN_INIT,   //tcp���ӳس�ʼ����ʱ��
    ASYNC_EVENT_TCPCONN_REQUEST,//tcp���ӳ��л�ȡsocket
    ASYNC_EVENT_TCPCONN_CLOSE,  //tcp���ӳعر�
    ASYNC_EVENT_TCPAGENT_REQUEST, //tcp agent�л�ȡsocket
    ASYNC_EVENT_TCP_CONNCLOSE,  //���ӳ��е�socket����
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
     * ���loop�¼�
     * @param e �¼�����
     * @param param �¼�������
     */
    void AddEvent(UV_ASYNC_EVENT e, void* param);

    /**
     * �Ƴ�ĳ�������ߵ��¼��������������������
     */
    void RemoveEvent(void* param);

    /**
     * �¼�ѭ���߳�
     */
    void LoopThread();

    /**
     * �첽�¼�����
     */
    void AsyncEvent();

    /**
     * �¼�����ر�
     */
    void CloseHandle();

    uv_loop_t       m_uvLoop;
private:
    bool            m_bRun;     //Ĭ��Ϊtrue������ʱ����Ϊfalse
    bool            m_bStop;    //Ĭ��Ϊfalse��loop��������Ϊtrue
    uv_async_t      m_uvAsync;
    list<UV_EVET>   m_listAsyncEvents;
    uv_mutex_t      m_uvMtxAsEvts;

    //set<CTcpSocket*> m_pTcpClients;
    //set<C
};

}