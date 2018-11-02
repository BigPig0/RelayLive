#include "stdafx.h"
#include "SipMgr.h"
#include "SipCall.h"

CSipSubscribe*  CSipMgr::m_pSubscribe = nullptr;
CSipConfig*     CSipMgr::m_pConfig    = nullptr;
CSipMessage*    CSipMgr::m_pMessage   = nullptr;
CSipInvite*     CSipMgr::m_pInvite    = nullptr;

CSipMgr::CSipMgr(void)
    : m_pExContext(nullptr)
    , m_pSever(nullptr)
{
}

CSipMgr::~CSipMgr(void)
{
    SAFE_DELETE(m_pSever);
    SAFE_DELETE(m_pSubscribe);
    SAFE_DELETE(m_pConfig);
}

bool CSipMgr::Init()
{
    // 读取配置
    m_pConfig = new CSipConfig();
    if (nullptr == m_pConfig)
    {
        Log::error("CSipMgr::Init() Config new failed");
        return false;
    }
    m_pConfig->Load();

    //osip库初始化
    m_pExContext = eXosip_malloc();
    if (nullptr == m_pExContext)
    {
        Log::error("CSipMgr::Init() eXosip_malloc failed");
        return false;
    }

    int result = OSIP_SUCCESS;
    result = eXosip_init(m_pExContext);
    if (OSIP_SUCCESS != result)
    {
        Log::error("CSipMgr::Init() eXosip_init failed");
        return false;
    }

    // 订阅通知
    m_pSubscribe = new CSipSubscribe(m_pExContext);
    if (nullptr == m_pSubscribe)
    {
        Log::error("CSipMgr::Init() Subscribe new failed");
        return false;
    }

    // 消息
    m_pMessage = new CSipMessage(m_pExContext);
    if (nullptr == m_pMessage)
    {
        Log::error("CSipMgr::Init() SipMessage new failed");
        return false;
    }

    // 会话邀请
    m_pInvite = new CSipInvite(m_pExContext);
    if (nullptr == m_pInvite)
    {
        Log::error("CSipMgr::Init() SipInvite new failed");
        return false;
    }

    // 监听服务
    m_pSever = new CSipSever(m_pExContext);
    if (nullptr == m_pSever)
    {
        Log::error("CSipMgr::Init() SipSever new failed");
        return false;
    }
    m_pSever->StartSever();

    return true;
}