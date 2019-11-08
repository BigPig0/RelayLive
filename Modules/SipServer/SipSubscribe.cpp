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

void CSipSubscribe::SetPlatform(string strDevCode, string strAddrIP, string strAddrPort)
{
    m_strCode = strDevCode;
    m_strIP = strAddrIP;
    m_strPort = strAddrPort;
}

void CSipSubscribe::SubscribeDirectory(const int expires)
{
    CSipFromToHeader from;
    from.SetHeader(CSipMgr::m_pConfig->strDevCode.c_str()
                   , CSipMgr::m_pConfig->strAddrIP.c_str()
                   , CSipMgr::m_pConfig->strAddrPort.c_str());
    CSipFromToHeader to;
    to.SetHeader(m_strCode.c_str(), m_strIP.c_str(), m_strPort.c_str());
    
    static osip_message_t *subMsg = 0;
    static int eventID = 1;
    stringstream ssEvt;
    ssEvt << "Catalog;id=" << eventID++;
    eXosip_lock(m_pExContext);
    int nSubID = eXosip_subscription_build_initial_subscribe( m_pExContext       // struct eXosip_t *excontext
                                               , &subMsg                         // osip_message_t ** subscribe
                                               , to.GetFormatHeader().c_str()    // const char *to
                                               , from.GetFormatHeader().c_str()  // const char *from
                                               , nullptr                         // const char *route
                                               //, "presence"                      // const char *event
                                               , ssEvt.str().c_str()
                                               , expires);                       // int expires
    if (nSubID != OSIP_SUCCESS)
    {
        Log::error("CSubscribe::SendSubscribe init msg failed");
        eXosip_unlock(m_pExContext);
        return;
    }

    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
       << "<Query>\r\n"
       << "<CmdType>Catalog</CmdType>\r\n"
       << "<SN>" << sn++ << "</SN>\r\n"
       << "<DeviceID>" << m_strCode << "</DeviceID>\r\n"
       << "</Query>\r\n";
    string strBody = ss.str();

    osip_message_set_body (subMsg, strBody.c_str(), strBody.length());
    osip_message_set_content_type (subMsg, "Application/MANSCDP+xml");
    
    int ret = eXosip_subscription_send_initial_request(m_pExContext, subMsg);
    if (ret <= 0)
    {
        Log::error("CSubscribe::SendSubscribe send failed:%d",ret);
        eXosip_unlock(m_pExContext);
        return;
    }

    eXosip_unlock(m_pExContext);
    return;
}

void CSipSubscribe::SubscribeAlarm(const int expires)
{
    CSipFromToHeader from;
    from.SetHeader(CSipMgr::m_pConfig->strDevCode.c_str()
        , CSipMgr::m_pConfig->strAddrIP.c_str()
        , CSipMgr::m_pConfig->strAddrPort.c_str());
    CSipFromToHeader to;
    to.SetHeader(m_strCode.c_str(), m_strIP.c_str(), m_strPort.c_str());

    static osip_message_t *subMsg = 0;
    static int eventID = 1;
    stringstream ssEvt;
    ssEvt << "Catalog;id=" << eventID++;
    eXosip_lock(m_pExContext);
    int nSubID = eXosip_subscription_build_initial_subscribe( m_pExContext       // struct eXosip_t *excontext
        , &subMsg                         // osip_message_t ** subscribe
        , to.GetFormatHeader().c_str()    // const char *to
        , from.GetFormatHeader().c_str()  // const char *from
        , nullptr                         // const char *route
        //, "presence"                      // const char *event
        , ssEvt.str().c_str()
        , expires);                       // int expires
    if (nSubID != OSIP_SUCCESS)
    {
        Log::error("CSubscribe::SendSubscribeAlarm init msg failed");
        eXosip_unlock(m_pExContext);
        return;
    }

    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
        << "<Query>\r\n"
        << "<CmdType>Alarm</CmdType>\r\n"
        << "<SN>" << sn++ << "</SN>\r\n"
        << "<DeviceID>" << m_strCode << "</DeviceID>\r\n"
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
        return;
    }

    eXosip_unlock(m_pExContext);
    return;
}

void CSipSubscribe::SubscribeMobilepostion(const int expires)
{
    SubscribeMobilepostion(expires, m_strCode);
}

void CSipSubscribe::SubscribeMobilepostion(const int expires, vector<string> devs){
	for(auto strDevCode: devs){
		SubscribeMobilepostion(expires, strDevCode);
	}
}

void CSipSubscribe::SubscribeMobilepostion(const int expires, string strDevCode){
	CSipFromToHeader from;
    from.SetHeader(CSipMgr::m_pConfig->strDevCode.c_str()
        , CSipMgr::m_pConfig->strAddrIP.c_str()
        , CSipMgr::m_pConfig->strAddrPort.c_str());
    CSipFromToHeader to;
    to.SetHeader(m_strCode.c_str(), m_strIP.c_str(), m_strPort.c_str());

    static osip_message_t *subMsg = 0;
    static int eventID = 1;
    stringstream ssEvt;
    ssEvt << "Catalog;id=" << eventID++;
    eXosip_lock(m_pExContext);
    int nSubID = eXosip_subscription_build_initial_subscribe( m_pExContext       // struct eXosip_t *excontext
        , &subMsg                         // osip_message_t ** subscribe
        , to.GetFormatHeader().c_str()    // const char *to
        , from.GetFormatHeader().c_str()  // const char *from
        , nullptr                         // const char *route
        //, "presence"                      // const char *event
        , ssEvt.str().c_str()
        , expires);                       // int expires
    if (nSubID != OSIP_SUCCESS)
    {
        Log::error("CSubscribe::SendSubscribeMobilepostion init msg failed");
        eXosip_unlock(m_pExContext);
        return;
    }

    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
        << "<Query>\r\n"
        << "<CmdType>MobilePosition</CmdType>\r\n"
        << "<SN>" << sn++ << "</SN>\r\n"
        << "<DeviceID>" << strDevCode << "</DeviceID>\r\n"
		<< "<Interval>5</Interval>\r\n"
        << "</Query>\r\n";
    string strBody = ss.str();

    osip_message_set_body (subMsg, strBody.c_str(), strBody.length());
    osip_message_set_content_type (subMsg, "Application/MANSCDP+xml");

    int ret = eXosip_subscription_send_initial_request(m_pExContext, subMsg);
    if (ret <= 0)
    {
        Log::error("CSubscribe::SendSubscribeMobilepostion send failed:%d",ret);
        eXosip_unlock(m_pExContext);
        return;
    }

    eXosip_unlock(m_pExContext);
    return;
}