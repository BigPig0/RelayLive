/**
 * 订阅对方平台
 */

#pragma once

namespace SipServer {

class CSipSubscribe
{
public:

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

};

};