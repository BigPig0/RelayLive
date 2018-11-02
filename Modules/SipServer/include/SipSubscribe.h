/**
 * 订阅对方平台
 */

#pragma once
#include "SipHeaders.h"

class CSipSubscribe
{
public:
    CSipSubscribe(eXosip_t* pSip);
    virtual ~CSipSubscribe(void);

    // 生存订阅信息
    void Subscribe(string strDevCode, string strAddrIP, string strAddrPort);

    // 事件订阅
    void SubscribeAlarm(string strDevCode, string strAddrIP, string strAddrPort);

    // 位置信息订阅
    void SubscribeMobilepostion(string strDevCode, string strAddrIP, string strAddrPort);

private:
    //发送订阅信息
    int SendSubscribe(CSipFromToHeader &from, 
                      CSipFromToHeader &to,
                      const int expires);

    //发送事件订阅信息
    int SendSubscribeAlarm(CSipFromToHeader &from, 
                      CSipFromToHeader &to,
                      const int expires);

    //发送位置信息订阅
    int SendSubscribeMobilepostion(CSipFromToHeader &from, 
        CSipFromToHeader &to,
        const int expires);

private:
    eXosip_t* m_pExContext;
};

