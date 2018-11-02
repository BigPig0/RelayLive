/**
 * Sip服务器
 */
#pragma once

class CSipSever
{
public:
    CSipSever(eXosip_t* pSip);
    ~CSipSever(void);

    /**
     * 启动sip服务器
     */
    void StartSever();

private:
    /**
     * sip服务器接收线程
     */
    void SeverThread();

    /**
     * 定时订阅信息
     */
    void SubscribeThread();

    /**
     * 处理接收到注册事件
     */
    void OnRegister(eXosip_event_t *osipEvent);

    /**
     * 处理接收到的消息事件
     */
    void OnMessage(eXosip_event_t *osipEvent);

    /**
     * 处理接收到的200OK事件
     */
    void OnMessageOK(eXosip_event_t *osipEvent);

    /**
     * 处理接收到的Invite 200OK事件
     */
    void OnInviteOK(eXosip_event_t *osipEvent);

    /**
     * 处理播放中的
     */
    void OnCallNew(eXosip_event_t *osipEvent);

    /**
     * 
     */
    void OnCallClose(eXosip_event_t *osipEvent);

    void OnCallClear(eXosip_event_t *osipEvent);

private:
    eXosip_t*   m_pExContext;
    time_t      m_nSubTime; //订阅时间
};

