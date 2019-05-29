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
	m_bSubPosDev  = Settings::getValue("PlatFormInfo","SubscribePosDev",0)>0?true:false;
	m_strMobile = Settings::getValue("PlatFormInfo","SubscribePosDepart", "111");
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
			Log::warning("The sip event type that not be precessed.the event type is : %d, %s",osipEventPtr->type, osipEventPtr->textinfo);
            break;
        }
        eXosip_event_free(osipEventPtr);
        osipEventPtr = NULL;
    } //while(true)
}

void CSipSever::SubscribeThread()
{
    PlatFormInfo* platform = DeviceMgr::GetPlatformInfo();
    CSipMgr::m_pSubscribe->SetPlatform(platform->strDevCode, platform->strAddrIP, platform->strAddrPort);
    time_t lastQueryTime, lastSubscribeStat, lastSubscribePos; //上次查询目录的时间

    if(1){
        Sleep(10000); //延时用来保证先收到注册
        CSipMgr::m_pMessage->QueryDirtionary(platform->strDevCode, platform->strAddrIP, platform->strAddrPort);
		lastQueryTime = time(NULL);
        Log::debug(" Query dir %s",platform->strDevCode.c_str());
        Sleep(60000); //延时保证查询接受结束再进行订阅
    }
    if(m_bSubStat) {
        CSipMgr::m_pSubscribe->SubscribeDirectory(600);
		lastSubscribeStat = time(NULL);
        Log::debug(" Subscribe dir %s",platform->strDevCode.c_str());
        Sleep(10000);
    }
    if(m_bSubPos) {
        CSipMgr::m_pSubscribe->SubscribeMobilepostion(600);
		lastSubscribePos = time(NULL);
        Log::debug(" Subscribe mobile pos %s",platform->strDevCode.c_str());
    }
	if(m_bSubPosDev){
		vector<DevInfo*> devInfo = DeviceMgr::GetDeviceInfo();
		vector<string> devs;
		for(auto info:devInfo){
			if(info->strParentID == m_strMobile)
				devs.push_back(info->strDevID);
		}
		 CSipMgr::m_pSubscribe->SubscribeMobilepostion(600, devs);
		lastSubscribePos = time(NULL);
        Log::debug(" Subscribe all mobile pos %s",platform->strDevCode.c_str());
	}

    while(true)
    {
        time_t now = time(nullptr);
        struct tm * timeinfo = localtime(&now);
        if(difftime(now,lastQueryTime) > 3600 //距离上一次查询查过一小时
            && timeinfo->tm_hour == 1  //夜里1点重新查询
          ) {
            lastQueryTime = now;
            DeviceMgr::CleanPlatform(); //清空缓存中的数据和数据库设备表中的记录
            CSipMgr::m_pMessage->QueryDirtionary(platform->strDevCode, platform->strAddrIP, platform->strAddrPort);
        }
		if(m_bSubStat && difftime(now,lastSubscribeStat) > 600){
			 lastSubscribeStat = now;
			 CSipMgr::m_pSubscribe->SubscribeDirectory(600);
			 Log::debug(" Subscribe dir %s",platform->strDevCode.c_str());
		}
		if(m_bSubPos  && difftime(now,lastSubscribePos) > 600) {
			CSipMgr::m_pSubscribe->SubscribeMobilepostion(600);
			lastSubscribePos = time(NULL);
			Log::debug(" Subscribe mobile pos %s",platform->strDevCode.c_str());
		}
		if(m_bSubPosDev && difftime(now,lastSubscribePos) > 600){
			vector<DevInfo*> devInfo = DeviceMgr::GetDeviceInfo();
			vector<string> devs;
			for(auto info:devInfo){
				if(info->strParentID == m_strMobile)
					devs.push_back(info->strDevID);
			}
			 CSipMgr::m_pSubscribe->SubscribeMobilepostion(600, devs);
			lastSubscribePos = time(NULL);
			Log::debug(" Subscribe all mobile pos %s",platform->strDevCode.c_str());
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