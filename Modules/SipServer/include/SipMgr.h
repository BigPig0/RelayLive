#pragma once
#include "SipSever.h"
#include "SipSubscribe.h"
#include "SipMessage.h"
#include "SipInvite.h"
#include "SipConfig.h"

/**
 * Sip模块管理类，需要实现单实例
 */
class CSipMgr
{
public:
    CSipMgr(void);
    ~CSipMgr(void);

    bool Init();

public:
    static CSipSubscribe* m_pSubscribe;
    static CSipConfig*    m_pConfig;
    static CSipMessage*   m_pMessage;
    static CSipInvite*    m_pInvite;

private:
    eXosip_t*  m_pExContext;
    CSipSever* m_pSever;
};

