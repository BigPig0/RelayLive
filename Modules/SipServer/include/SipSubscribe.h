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

    // 设置对方平台的信息
    void SetPlatform(string strDevCode, string strAddrIP, string strAddrPort);

    // 目录状态订阅
    void SubscribeDirectory(const int expires);

    // 事件订阅
    void SubscribeAlarm(const int expires);

    // 移动设备位置信息订阅
    void SubscribeMobilepostion(const int expires);

	// 移动设备位置信息订阅[订阅多个设备]
    void SubscribeMobilepostion(const int expires, vector<string> devs);

	// 移动设备位置信息订阅[订阅单个设备]
    void SubscribeMobilepostion(const int expires, string strDevCode);

private:
    eXosip_t* m_pExContext;
    string    m_strCode;    //对方平台的编码
    string    m_strIP;      //对方平台的IP
    string    m_strPort;    //对方平台的端口
};

