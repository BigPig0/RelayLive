#include "stdafx.h"
#include "SipSever.h"
#include "SipRegister.h"
#include "SipMessage.h"
#include "SipMgr.h"
#include <winsock.h>

CSipSever::CSipSever(eXosip_t* pSip)
    : m_pExContext(pSip)
{
    m_bSubStat = Settings::getValue("PlatFormInfo","SubscribeStatus",0)>0?true:false;
    m_bSubPos  = Settings::getValue("PlatFormInfo","SubscribePos",0)>0?true:false;
}

CSipSever::~CSipSever(void)
{
}

void CSipSever::StartSever()
{
    thread th([this](){
        SeverThread();
    });
    th.detach();

    thread t2([this](){
        SubscribeThread();
    });
    t2.detach();
}

void CSipSever::SeverThread()
{
    DWORD nThreadID = GetCurrentThreadId();
    Log::debug("Sip Sever Thread ID : %d", nThreadID);

    eXosip_set_user_agent(m_pExContext, NULL);

    int nPort = 5060;
    if(!CSipMgr::m_pConfig->strAddrPort.empty())
        nPort = stoi(CSipMgr::m_pConfig->strAddrPort);
    if (OSIP_SUCCESS != eXosip_listen_addr(m_pExContext,IPPROTO_UDP, NULL, nPort, AF_INET, 0))
    {
        Log::error("CSipSever::SeverThread eXosip_listen_addr failure.");
        return;
    }

    if (OSIP_SUCCESS != eXosip_set_option(m_pExContext, EXOSIP_OPT_SET_IPV4_FOR_GATEWAY, CSipMgr::m_pConfig->strAddrIP.c_str()))
    {
        Log::error("CSipSever::SeverThread eXosip_set_option failure.");
        return;
    }

    eXosip_event_t* osipEventPtr = NULL;
    while (true)
    {
        // Wait the osip event.
        osipEventPtr = ::eXosip_event_wait(m_pExContext, 0, 200);// 0的单位是秒，200是毫秒
        eXosip_lock(m_pExContext);
        eXosip_automatic_action (m_pExContext);
        eXosip_unlock(m_pExContext);
        // If get nothing osip event,then continue the loop.
        if (NULL == osipEventPtr)
        {
            continue;
        }
        Log::warning("new sip event: %d",osipEventPtr->type);
        switch (osipEventPtr->type)
        {
        case EXOSIP_MESSAGE_NEW:
            {
                if (!strncmp(osipEventPtr->request->sip_method, "REGISTER", 
                    strlen("REGISTER")))
                {
                    Log::warning("recive REGISTER");
                    //OnRegister(osipEventPtr);
                    CSipRegister Register(m_pExContext);
                    Register.SetAuthorization(CSipMgr::m_pConfig->bRegAuthor);
                    Register.OnRegister(osipEventPtr);
                }
                else if (!strncmp(osipEventPtr->request->sip_method, "MESSAGE",
                    strlen("MESSAGE")))
                {
                    Log::warning("recive MESSAGE");
                    //OnMessage(osipEventPtr);
                    CSipMgr::m_pMessage->OnMessage(osipEventPtr);
                }
            }
            break;
        //case EXOSIP_MESSAGE_ANSWERED:
        //    {
        //        Log::warning("recive message ok 200");
        //    }
        //    break;
        //case EXOSIP_SUBSCRIPTION_ANSWERED:
        //    {
        //        Log::warning("recive subscription ok 200");
        //    }
        //    break;
        //case EXOSIP_CALL_MESSAGE_ANSWERED:
        //    {
        //        Log::warning("recive call message ok 200");
        //    }
        //    break;
        //case EXOSIP_CALL_PROCEEDING:
        //    {
        //        Log::warning("recive call-trying message 100");
        //    }
        //    break;
        case EXOSIP_CALL_ANSWERED:
            {
                Log::warning("recive call-answer message 200");
                //OnInviteOK(osipEventPtr);
                CSipMgr::m_pInvite->OnInviteOK(osipEventPtr);
            }
            break;
        case EXOSIP_CALL_NOANSWER:
        case EXOSIP_CALL_REQUESTFAILURE:
        case EXOSIP_CALL_SERVERFAILURE:
        case EXOSIP_CALL_GLOBALFAILURE:
            {
                Log::warning("recive call-answer failed %d", osipEventPtr->type);
                CSipMgr::m_pInvite->OnInviteFailed(osipEventPtr);
            }
            break;
        //case EXOSIP_CALL_MESSAGE_NEW:
        //    {
        //        Log::warning("announce new incoming request");
        //        OnCallNew(osipEventPtr);
        //    }
        //    break;
        case EXOSIP_SUBSCRIPTION_NOTIFY:
            {
				Log::warning("recive notify");
				//OnMessage(osipEventPtr);
                CSipMgr::m_pMessage->OnMessage(osipEventPtr);
            }
            break;
        //case EXOSIP_CALL_CLOSED:
        //    {
        //        Log::warning("recive EXOSIP_CALL_CLOSED");
        //        OnCallClose(osipEventPtr);
        //    }
        //    break;
        //case EXOSIP_CALL_RELEASED:
        //    {
        //        Log::warning("recive EXOSIP_CALL_RELEASED");
        //        //OnCallClear(osipEventPtr);
        //    }
        //    break;
        default:
            Log::warning("The sip event type that not be precessed.the event type is : %d\r\n",osipEventPtr->type);
            break;
        }
        eXosip_event_free(osipEventPtr);
        osipEventPtr = NULL;
    } //while(true)
}

void CSipSever::SubscribeThread()
{
    Sleep(10000); //延时用来保证先收到注册
    bool firstQuery = true; //启动后第一次查询目录
    time_t lastQueryTime = 0; //上次查询目录的时间
    time_t lastSubTime = 0; //订阅时间
    while(true)
    {
        time_t now = time(nullptr);
        struct tm * timeinfo = localtime(&now);
        if(firstQuery) {
            lastQueryTime = now;
            PlatFormInfo* platform = DeviceMgr::GetPlatformInfo();
            CSipMgr::m_pMessage->QueryDirtionary(platform->strDevCode, platform->strAddrIP, platform->strAddrPort);
            firstQuery = false;
        } else if(difftime(now,lastQueryTime) > 3600 //距离上一次查询查过一小时
            && timeinfo->tm_hour == 5  //夜里5点重新查询
            ) {
            lastQueryTime = now;
            DeviceMgr::CleanPlatform(); //清空缓存中的数据和数据库设备表中的记录
            PlatFormInfo* platform = DeviceMgr::GetPlatformInfo();
            CSipMgr::m_pMessage->QueryDirtionary(platform->strDevCode, platform->strAddrIP, platform->strAddrPort);
        }
        if(difftime(now,lastSubTime) > 1800){ //30分重新订阅一次
            lastSubTime = now;
            PlatFormInfo* platform = DeviceMgr::GetPlatformInfo();
            if(m_bSubStat) {
				Sleep(10000);
                CSipMgr::m_pSubscribe->Subscribe(platform->strDevCode, platform->strAddrIP, platform->strAddrPort);
			}
            if(m_bSubPos) {
				Sleep(10000);
                CSipMgr::m_pSubscribe->SubscribeMobilepostion(platform->strDevCode, platform->strAddrIP, platform->strAddrPort);
			}
            Log::debug(" Subscribe %s",platform->strDevCode.c_str());
        }
        Sleep(1000);
    }
}

void CSipSever::OnRegister(eXosip_event_t *osipEvent)
{
    auto run = [&](eXosip_t* pSip, eXosip_event_t *osipEvent, bool bRegAuthor){
        Log::debug("OnRegister thread ID : %d", GetCurrentThreadId());
        CSipRegister Register(pSip);
        Register.SetAuthorization(bRegAuthor);
        Register.OnRegister(osipEvent);
        eXosip_event_free(osipEvent);
        //Log::debug("OnRegister thread finish\r\n");
    };
    std::thread t1(run,m_pExContext, osipEvent, CSipMgr::m_pConfig->bRegAuthor);
    t1.detach();
}

void CSipSever::OnMessage(eXosip_event_t *osipEvent)
{
    auto run = [](eXosip_t* pSip, eXosip_event_t *osipEvent){
        Log::debug("OnMessage thread ID : %d", GetCurrentThreadId());
        CSipMgr::m_pMessage->OnMessage(osipEvent);
        eXosip_event_free(osipEvent);
        //Log::debug("OnMessage thread finish\r\n");
    };
    std::thread t1(run,m_pExContext, osipEvent);
    t1.detach();
}

void CSipSever::OnMessageOK(eXosip_event_t *osipEvent)
{
    //Log::debug("OnMessageOK thread finish");
}

void CSipSever::OnInviteOK(eXosip_event_t *osipEvent)
{
    auto run = [](eXosip_event_t *osipEvent){
        Log::debug("OnInviteOK thread ID : %d", GetCurrentThreadId());
        CSipMgr::m_pInvite->OnInviteOK(osipEvent);
        eXosip_event_free(osipEvent);
        //Log::debug("OnInviteOK thread finish\r\n");
    };
    std::thread t1(run, osipEvent);
    t1.detach();
}

void CSipSever::OnCallNew(eXosip_event_t *osipEvent)
{
    std::thread t([&](){
        Log::debug("OnCallNew thread ID : %d", GetCurrentThreadId());
        CSipMgr::m_pInvite->OnCallNew(osipEvent);
        eXosip_event_free(osipEvent);
    });
    t.detach();
}

void CSipSever::OnCallClose(eXosip_event_t *osipEvent)
{
    std::thread t([&](){
        Log::debug("OnCallClose thread ID : %d exosip:%d,cid:%d,did:%d", GetCurrentThreadId(),osipEvent,osipEvent->cid,osipEvent->did);
        CSipMgr::m_pInvite->OnCallClose(osipEvent);
        eXosip_event_free(osipEvent);
    });
    t.detach();
}

void CSipSever::OnCallClear(eXosip_event_t *osipEvent)
{
    std::thread t([&](){
        Log::debug("OnCallClear thread ID : %d", GetCurrentThreadId());
        CSipMgr::m_pInvite->OnCallClear(osipEvent);
        eXosip_event_free(osipEvent);
    });
    t.detach();
}