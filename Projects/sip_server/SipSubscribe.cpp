#include "SipPrivate.h"
#include "SipSubscribe.h"

using namespace SipServer;
using namespace util;

static int eventID = 1;

void CSipSubscribe::SubscribeDirectory(const int expires)
{
    string strFrom = GetFormatHeader(g_strCode , g_strSipIP , g_nSipPort);
    string strTo   = GetFormatHeader(g_strLowCode, g_strLowIP, g_nLowPort);
    
    osip_message_t *subMsg = NULL;
    stringstream ssEvt;
    ssEvt << "Catalog;id=" << eventID++;
    eXosip_lock(g_pExContext);
    int nSubID = eXosip_subscription_build_initial_subscribe( g_pExContext       // struct eXosip_t *excontext
                                               , &subMsg                         // osip_message_t ** subscribe
                                               , strTo.c_str()    // const char *to
                                               , strFrom.c_str()  // const char *from
                                               , nullptr                         // const char *route
                                               //, "presence"                      // const char *event
                                               , ssEvt.str().c_str()
                                               , expires);                       // int expires
    if (nSubID != OSIP_SUCCESS)
    {
        Log::error("CSubscribe::SendSubscribe init msg failed");
        eXosip_unlock(g_pExContext);
        return;
    }

    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
       << "<Query>\r\n"
       << "<CmdType>Catalog</CmdType>\r\n"
       << "<SN>" << sn++ << "</SN>\r\n"
       << "<DeviceID>" << g_strLowCode << "</DeviceID>\r\n"
       << "</Query>\r\n";
    string strBody = ss.str();

    osip_message_set_body (subMsg, strBody.c_str(), strBody.length());
    osip_message_set_content_type (subMsg, "Application/MANSCDP+xml");
    
    int ret = eXosip_subscription_send_initial_request(g_pExContext, subMsg);
    if (ret <= 0)
    {
        Log::error("CSubscribe::SendSubscribe send failed:%d",ret);
        //osip_message_free(subMsg);
        eXosip_unlock(g_pExContext);
        return;
    }

    //osip_message_free(subMsg);
    eXosip_unlock(g_pExContext);
    return;
}

void CSipSubscribe::SubscribeAlarm(const int expires)
{
    string strFrom = GetFormatHeader(g_strCode , g_strSipIP , g_nSipPort);
    string strTo   = GetFormatHeader(g_strLowCode, g_strLowIP, g_nLowPort);

    osip_message_t *subMsg = NULL;
    stringstream ssEvt;
    ssEvt << "Catalog;id=" << eventID++;
    eXosip_lock(g_pExContext);
    int nSubID = eXosip_subscription_build_initial_subscribe( g_pExContext       // struct eXosip_t *excontext
        , &subMsg                         // osip_message_t ** subscribe
        , strTo.c_str()    // const char *to
        , strFrom.c_str()  // const char *from
        , nullptr                         // const char *route
        //, "presence"                      // const char *event
        , ssEvt.str().c_str()
        , expires);                       // int expires
    if (nSubID != OSIP_SUCCESS)
    {
        Log::error("CSubscribe::SendSubscribeAlarm init msg failed");
        eXosip_unlock(g_pExContext);
        return;
    }

    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
        << "<Query>\r\n"
        << "<CmdType>Alarm</CmdType>\r\n"
        << "<SN>" << sn++ << "</SN>\r\n"
        << "<DeviceID>" << g_strLowCode << "</DeviceID>\r\n"
		<< "<StartAlarmPriority>1</StartAlarmPriority>\r\n"
		<< "<EndAlarmPriority>4</EndAlarmPriority>\r\n"
		<< "<AlarmMethod>0</AlarmMethod>\r\n"
        << "</Query>\r\n";
    string strBody = ss.str();

    osip_message_set_body (subMsg, strBody.c_str(), strBody.length());
    osip_message_set_content_type (subMsg, "Application/MANSCDP+xml");

    int ret = eXosip_subscription_send_initial_request(g_pExContext, subMsg);
    if (ret <= 0)
    {
        Log::error("CSubscribe::SendSubscribeAlarm send failed:%d",ret);
        //osip_message_free(subMsg);
        eXosip_unlock(g_pExContext);
        return;
    }

    //osip_message_free(subMsg);
    eXosip_unlock(g_pExContext);
    return;
}

void CSipSubscribe::SubscribeMobilepostion(const int expires)
{
    SubscribeMobilepostion(expires, g_strLowCode);
}

void CSipSubscribe::SubscribeMobilepostion(const int expires, vector<string> devs){
	for(auto strDevCode: devs){
		SubscribeMobilepostion(expires, strDevCode);
	}
}

void CSipSubscribe::SubscribeMobilepostion(const int expires, string strDevCode){
    string strFrom = GetFormatHeader(g_strCode , g_strSipIP , g_nSipPort);
    string strTo   = GetFormatHeader(g_strLowCode, g_strLowIP, g_nLowPort);

    osip_message_t *subMsg = NULL;
    stringstream ssEvt;
    ssEvt << "Catalog;id=" << eventID++;
    eXosip_lock(g_pExContext);
    int nSubID = eXosip_subscription_build_initial_subscribe( g_pExContext       // struct eXosip_t *excontext
        , &subMsg                         // osip_message_t ** subscribe
        , strTo.c_str()    // const char *to
        , strFrom.c_str()  // const char *from
        , nullptr                         // const char *route
        //, "presence"                      // const char *event
        , ssEvt.str().c_str()
        , expires);                       // int expires
    if (nSubID != OSIP_SUCCESS)
    {
        Log::error("CSubscribe::SendSubscribeMobilepostion init msg failed");
        eXosip_unlock(g_pExContext);
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

    int ret = eXosip_subscription_send_initial_request(g_pExContext, subMsg);
    if (ret <= 0)
    {
        Log::error("CSubscribe::SendSubscribeMobilepostion send failed:%d",ret);
        //osip_message_free(subMsg);
        eXosip_unlock(g_pExContext);
        return;
    }

    //osip_message_free(subMsg);
    eXosip_unlock(g_pExContext);
    return;
}