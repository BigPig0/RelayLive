#include "stdafx.h"
#include "SipSubscribe.h"
#include "SipMgr.h"


CSipSubscribe::CSipSubscribe(eXosip_t* pSip)
    : m_pExContext(pSip)
{
}


CSipSubscribe::~CSipSubscribe(void)
{
}


void CSipSubscribe::Subscribe(string strDevCode, string strAddrIP, string strAddrPort)
{
    CSipFromToHeader stFrom;
    stFrom.SetHeader(CSipMgr::m_pConfig->strDevCode.c_str()
                   , CSipMgr::m_pConfig->strAddrIP.c_str()
                   , CSipMgr::m_pConfig->strAddrPort.c_str());
    CSipFromToHeader stTo;
    stTo.SetHeader(strDevCode.c_str(), strAddrIP.c_str(), strAddrPort.c_str());
    SendSubscribe(stFrom, stTo, 3600);
}

int CSipSubscribe::SendSubscribe(CSipFromToHeader &from, 
                              CSipFromToHeader &to,
                              const int expires)
{
    static osip_message_t *subMsg = 0;
    //static int eventID = 1;
    //stringstream ssEvt;
    //ssEvt << "Catalog;id=" << eventID++;
    eXosip_lock(m_pExContext);
    int nSubID = eXosip_subscription_build_initial_subscribe( m_pExContext       // struct eXosip_t *excontext
                                               , &subMsg                         // osip_message_t ** subscribe
                                               , to.GetFormatHeader().c_str()    // const char *to
                                               , from.GetFormatHeader().c_str()  // const char *from
                                               , nullptr                         // const char *route
                                               , "presence"                      // const char *event
                                               //, ssEvt.str().c_str()
                                               , expires);                       // int expires
    if (nSubID != OSIP_SUCCESS)
    {
        Log::error("CSubscribe::SendSubscribe init msg failed");
        eXosip_unlock(m_pExContext);
        return -1;
    }

    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
       << "<Query>\r\n"
       << "<CmdType>Catalog</CmdType>\r\n"
       << "<SN>" << sn++ << "</SN>\r\n"
       << "<DeviceID>" << to.GetAddrCode() << "</DeviceID>\r\n"
       << "</Query>\r\n";
    string strBody = ss.str();

    osip_message_set_body (subMsg, strBody.c_str(), strBody.length());
    osip_message_set_content_type (subMsg, "Application/MANSCDP+xml");
    
    int ret = eXosip_subscription_send_initial_request(m_pExContext, subMsg);
    if (ret <= 0)
    {
        Log::error("CSubscribe::SendSubscribe send failed:%d",ret);
        eXosip_unlock(m_pExContext);
        return -1;
    }

    eXosip_unlock(m_pExContext);
    return 0;
}




void CSipSubscribe::SubscribeAlarm(string strDevCode, string strAddrIP, string strAddrPort)
{
    CSipFromToHeader stFrom;
    stFrom.SetHeader(CSipMgr::m_pConfig->strDevCode.c_str()
        , CSipMgr::m_pConfig->strAddrIP.c_str()
        , CSipMgr::m_pConfig->strAddrPort.c_str());
    CSipFromToHeader stTo;
    stTo.SetHeader(strDevCode.c_str(), strAddrIP.c_str(), strAddrPort.c_str());
    SendSubscribeAlarm(stFrom, stTo, 3600);
}

int CSipSubscribe::SendSubscribeAlarm(CSipFromToHeader &from, 
                                 CSipFromToHeader &to,
                                 const int expires)
{
    static osip_message_t *subMsg = 0;
    //static int eventID = 1;
    //stringstream ssEvt;
    //ssEvt << "Catalog;id=" << eventID++;
    eXosip_lock(m_pExContext);
    int nSubID = eXosip_subscription_build_initial_subscribe( m_pExContext       // struct eXosip_t *excontext
        , &subMsg                         // osip_message_t ** subscribe
        , to.GetFormatHeader().c_str()    // const char *to
        , from.GetFormatHeader().c_str()  // const char *from
        , nullptr                         // const char *route
        , "presence"                      // const char *event
        //, ssEvt.str().c_str()
        , expires);                       // int expires
    if (nSubID != OSIP_SUCCESS)
    {
        Log::error("CSubscribe::SendSubscribeAlarm init msg failed");
        eXosip_unlock(m_pExContext);
        return -1;
    }

    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
        << "<Query>\r\n"
        << "<CmdType>Alarm</CmdType>\r\n"
        << "<SN>" << sn++ << "</SN>\r\n"
        << "<DeviceID>" << to.GetAddrCode() << "</DeviceID>\r\n"
		<< "<StartAlarmPriority>1</StartAlarmPriority>\r\n"
		<< "<EndAlarmPriority>4</EndAlarmPriority>\r\n"
		<< "<AlarmMethod>0</AlarmMethod>\r\n"
        << "</Query>\r\n";
    string strBody = ss.str();

    osip_message_set_body (subMsg, strBody.c_str(), strBody.length());
    osip_message_set_content_type (subMsg, "Application/MANSCDP+xml");

    int ret = eXosip_subscription_send_initial_request(m_pExContext, subMsg);
    if (ret <= 0)
    {
        Log::error("CSubscribe::SendSubscribeAlarm send failed:%d",ret);
        eXosip_unlock(m_pExContext);
        return -1;
    }

    eXosip_unlock(m_pExContext);
    return 0;
}



void CSipSubscribe::SubscribeMobilepostion(string strDevCode, string strAddrIP, string strAddrPort)
{
    CSipFromToHeader stFrom;
    stFrom.SetHeader(CSipMgr::m_pConfig->strDevCode.c_str()
        , CSipMgr::m_pConfig->strAddrIP.c_str()
        , CSipMgr::m_pConfig->strAddrPort.c_str());
    CSipFromToHeader stTo;
    stTo.SetHeader(strDevCode.c_str(), strAddrIP.c_str(), strAddrPort.c_str());
    SendSubscribeMobilepostion(stFrom, stTo, 3600);
}

int CSipSubscribe::SendSubscribeMobilepostion(CSipFromToHeader &from, 
                                      CSipFromToHeader &to,
                                      const int expires)
{
    static osip_message_t *subMsg = 0;
    //static int eventID = 1;
    //stringstream ssEvt;
    //ssEvt << "Catalog;id=" << eventID++;
    eXosip_lock(m_pExContext);
    int nSubID = eXosip_subscription_build_initial_subscribe( m_pExContext       // struct eXosip_t *excontext
        , &subMsg                         // osip_message_t ** subscribe
        , to.GetFormatHeader().c_str()    // const char *to
        , from.GetFormatHeader().c_str()  // const char *from
        , nullptr                         // const char *route
        , "presence"                      // const char *event
        //, ssEvt.str().c_str()
        , expires);                       // int expires
    if (nSubID != OSIP_SUCCESS)
    {
        Log::error("CSubscribe::SendSubscribeMobilepostion init msg failed");
        eXosip_unlock(m_pExContext);
        return -1;
    }

    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
        << "<Query>\r\n"
        << "<CmdType>MobilePosition</CmdType>\r\n"
        << "<SN>" << sn++ << "</SN>\r\n"
        << "<DeviceID>" << to.GetAddrCode() << "</DeviceID>\r\n"
        << "</Query>\r\n";
    string strBody = ss.str();

    osip_message_set_body (subMsg, strBody.c_str(), strBody.length());
    osip_message_set_content_type (subMsg, "Application/MANSCDP+xml");

    int ret = eXosip_subscription_send_initial_request(m_pExContext, subMsg);
    if (ret <= 0)
    {
        Log::error("CSubscribe::SendSubscribeMobilepostion send failed:%d",ret);
        eXosip_unlock(m_pExContext);
        return -1;
    }

    eXosip_unlock(m_pExContext);
    return 0;
}