#pragma once

namespace SipServer {



/**
 * 处理接收的注册请求
 */
class CSipRegister
{
public:

    /**
     * 处理注册事件
     */
    void OnRegister(eXosip_event_t *osipEvent);

};

};