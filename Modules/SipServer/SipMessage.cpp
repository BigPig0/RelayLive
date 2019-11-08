#include "stdafx.h"
#include "SipMessage.h"
#include "SipMgr.h"


CSipMessage::CSipMessage(eXosip_t* pSip)
    : m_pExContext(pSip)
{
}


CSipMessage::~CSipMessage(void)
{
}

void CSipMessage::OnMessage(eXosip_event_t *osipEvent)
{
    sipMessageInfo msgInfo;
    parserMessageInfo(osipEvent->request, osipEvent->tid, msgInfo);
    //打印报文
    printMeaassgePkt(msgInfo);
    //发送应答报文
    sendMessageAnswer(msgInfo);
}

void CSipMessage::QueryDirtionary(string strDevCode, string strAddrIP, string strAddrPort)
{
    CSipFromToHeader stFrom;
    stFrom.SetHeader(CSipMgr::m_pConfig->strDevCode.c_str()
                   , CSipMgr::m_pConfig->strAddrIP.c_str()
                   , CSipMgr::m_pConfig->strAddrPort.c_str());
    CSipFromToHeader stTo;
    stTo.SetHeader(strDevCode.c_str(), strAddrIP.c_str(), strAddrPort.c_str());

    LogDebug("CSipMessage::QueryDirtionary");

    static osip_message_t *qdmMsg = 0;

    int nID = eXosip_message_build_request(m_pExContext, &qdmMsg, "MESSAGE", 
        stTo.GetFormatHeader().c_str(), stFrom.GetFormatHeader().c_str(), nullptr);
    if (nID != OSIP_SUCCESS)
    {
        Log::error("CSipMessage::QueryDirtionary init msg failed");
        return;
    }

    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
        << "<Query>\r\n"
        << "<CmdType>Catalog</CmdType>\r\n"
        << "<SN>" << sn++ << "</SN>\r\n"
        << "<DeviceID>" << strDevCode << "</DeviceID>\r\n"
        << "</Query>\r\n";
    string strBody = ss.str();

    osip_message_set_contact(qdmMsg, stFrom.GetFormatHeader().c_str());
    osip_message_set_content_type (qdmMsg, "Application/MANSCDP+xml");
    osip_message_set_body (qdmMsg, strBody.c_str(), strBody.length());

    eXosip_lock(m_pExContext);
    int ret = eXosip_message_send_request(m_pExContext, qdmMsg);
    eXosip_unlock(m_pExContext);
    if (ret <= 0)
    {
        Log::error("CSipMessage::QueryDirtionary send failed:%d\r\n",ret);
    }
    else
    {
        Log::warning("send Query message QueryDirtionary\r\n");
    }
}

void CSipMessage::QueryDeviceStatus(string strDevCode, string strAddrIP, string strAddrPort, string devID)
{
    CSipFromToHeader stFrom;
    stFrom.SetHeader(CSipMgr::m_pConfig->strDevCode.c_str()
        , CSipMgr::m_pConfig->strAddrIP.c_str()
        , CSipMgr::m_pConfig->strAddrPort.c_str());
    CSipFromToHeader stTo;
    stTo.SetHeader(strDevCode.c_str(), strAddrIP.c_str(), strAddrPort.c_str());

    LogDebug("CSipMessage::QueryDeviceStatus");

    static osip_message_t *qdmMsg = 0;

    int nID = eXosip_message_build_request(m_pExContext, &qdmMsg, "MESSAGE", 
        stTo.GetFormatHeader().c_str(), stFrom.GetFormatHeader().c_str(), nullptr);
    if (nID != OSIP_SUCCESS)
    {
        Log::error("CSipMessage::QueryDeviceStatus init msg failed");
        return;
    }

    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
        << "<Query>\r\n"
        << "<CmdType>DeviceStatus</CmdType>\r\n"
        << "<SN>" << sn++ << "</SN>\r\n"
        << "<DeviceID>" << devID << "</DeviceID>\r\n"
        << "</Query>\r\n";
    string strBody = ss.str();

    osip_message_set_contact(qdmMsg, stFrom.GetFormatHeader().c_str());
    osip_message_set_content_type (qdmMsg, "Application/MANSCDP+xml");
    osip_message_set_body (qdmMsg, strBody.c_str(), strBody.length());

    eXosip_lock(m_pExContext);
    int ret = eXosip_message_send_request(m_pExContext, qdmMsg);
    eXosip_unlock(m_pExContext);
    if (ret <= 0)
    {
        Log::error("CSipMessage::QueryDeviceStatus send failed:%d\r\n",ret);
    }
    else
    {
        Log::warning("send Query message QueryDeviceStatus\r\n");
    }
}

void CSipMessage::QueryDeviceInfo(string strDevCode, string strAddrIP, string strAddrPort, string devID)
{
    CSipFromToHeader stFrom;
    stFrom.SetHeader(CSipMgr::m_pConfig->strDevCode.c_str()
        , CSipMgr::m_pConfig->strAddrIP.c_str()
        , CSipMgr::m_pConfig->strAddrPort.c_str());
    CSipFromToHeader stTo;
    stTo.SetHeader(strDevCode.c_str(), strAddrIP.c_str(), strAddrPort.c_str());

    LogDebug("CSipMessage::QueryDeviceInfo");

    static osip_message_t *qdmMsg = 0;

    int nID = eXosip_message_build_request(m_pExContext, &qdmMsg, "MESSAGE", 
        stTo.GetFormatHeader().c_str(), stFrom.GetFormatHeader().c_str(), nullptr);
    if (nID != OSIP_SUCCESS)
    {
        Log::error("CSipMessage::QueryDeviceInfo init msg failed");
        return;
    }

    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
        << "<Query>\r\n"
        << "<CmdType>DeviceInfo</CmdType>\r\n"
        << "<SN>" << sn++ << "</SN>\r\n"
        << "<DeviceID>" << devID << "</DeviceID>\r\n"
        << "</Query>\r\n";
    string strBody = ss.str();

    osip_message_set_contact(qdmMsg, stFrom.GetFormatHeader().c_str());
    osip_message_set_content_type (qdmMsg, "Application/MANSCDP+xml");
    osip_message_set_body (qdmMsg, strBody.c_str(), strBody.length());

    eXosip_lock(m_pExContext);
    int ret = eXosip_message_send_request(m_pExContext, qdmMsg);
    eXosip_unlock(m_pExContext);
    if (ret <= 0)
    {
        Log::error("CSipMessage::QueryDeviceInfo send failed:%d\r\n",ret);
    }
    else
    {
        Log::warning("send Query message QueryDeviceInfo\r\n");
    }
}

void CSipMessage::QueryRecordInfo(string strDevCode, string strAddrIP, string strAddrPort, string devID, string strStartTime, string strEndTime)
{
    CSipFromToHeader stFrom;
    stFrom.SetHeader(CSipMgr::m_pConfig->strDevCode.c_str()
        , CSipMgr::m_pConfig->strAddrIP.c_str()
        , CSipMgr::m_pConfig->strAddrPort.c_str());
    CSipFromToHeader stTo;
    stTo.SetHeader(strDevCode.c_str(), strAddrIP.c_str(), strAddrPort.c_str());

    LogDebug("CSipMessage::QueryRecordInfo");

    static osip_message_t *qdmMsg = 0;

    int nID = eXosip_message_build_request(m_pExContext, &qdmMsg, "MESSAGE", 
        stTo.GetFormatHeader().c_str(), stFrom.GetFormatHeader().c_str(), nullptr);
    if (nID != OSIP_SUCCESS)
    {
        Log::error("CSipMessage::QueryRecordInfo init msg failed");
        return;
    }

    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
        << "<Query>\r\n"
        << "<CmdType>RecordInfo</CmdType>\r\n"
        << "<SN>" << sn++ << "</SN>\r\n"
        << "<DeviceID>" << devID << "</DeviceID>\r\n"
        << "<StartTime>" << strStartTime << "</StartTime>\r\n"
        << "<EndTime>" << strEndTime << "</EndTime>\r\n"
        << "</Query>\r\n";
    string strBody = ss.str();

    osip_message_set_contact(qdmMsg, stFrom.GetFormatHeader().c_str());
    osip_message_set_content_type (qdmMsg, "Application/MANSCDP+xml");
    osip_message_set_body (qdmMsg, strBody.c_str(), strBody.length());

    eXosip_lock(m_pExContext);
    int ret = eXosip_message_send_request(m_pExContext, qdmMsg);
    eXosip_unlock(m_pExContext);
    if (ret <= 0)
    {
        Log::error("CSipMessage::QueryRecordInfo send failed:%d\r\n",ret);
    }
    else
    {
        Log::warning("send Query message QueryRecordInfo\r\n");
    }
}

void CSipMessage::QueryMobilePosition(string strDevCode, string strAddrIP, string strAddrPort, string devID)
{
    CSipFromToHeader stFrom;
    stFrom.SetHeader(CSipMgr::m_pConfig->strDevCode.c_str()
        , CSipMgr::m_pConfig->strAddrIP.c_str()
        , CSipMgr::m_pConfig->strAddrPort.c_str());
    CSipFromToHeader stTo;
    stTo.SetHeader(strDevCode.c_str(), strAddrIP.c_str(), strAddrPort.c_str());

    LogDebug("CSipMessage::QueryMobilePosition");

    static osip_message_t *qdmMsg = 0;

    int nID = eXosip_message_build_request(m_pExContext, &qdmMsg, "MESSAGE", 
        stTo.GetFormatHeader().c_str(), stFrom.GetFormatHeader().c_str(), nullptr);
    if (nID != OSIP_SUCCESS)
    {
        Log::error("CSipMessage::QueryMobilePosition init msg failed");
        return;
    }

    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
        << "<Query>\r\n"
        << "<CmdType>MobilePosition</CmdType>\r\n"
        << "<SN>" << sn++ << "</SN>\r\n"
        << "<DeviceID>" << devID << "</DeviceID>\r\n"
        << "</Query>\r\n";
    string strBody = ss.str();

    osip_message_set_contact(qdmMsg, stFrom.GetFormatHeader().c_str());
    osip_message_set_content_type (qdmMsg, "Application/MANSCDP+xml");
    osip_message_set_body (qdmMsg, strBody.c_str(), strBody.length());

    eXosip_lock(m_pExContext);
    int ret = eXosip_message_send_request(m_pExContext, qdmMsg);
    eXosip_unlock(m_pExContext);
    if (ret <= 0)
    {
        Log::error("CSipMessage::QueryMobilePosition send failed:%d\r\n",ret);
    }
    else
    {
        Log::warning("send Query message QueryMobilePosition\r\n");
    }
}

void CSipMessage::DeviceControl(string strAddrIP, string strAddrPort, string strDevCode,
                   int nInOut, int nUpDown, int nLeftRight, uchar cInOutSpeed, uchar cMoveSpeed)
{
    CSipFromToHeader stFrom;
    stFrom.SetHeader(CSipMgr::m_pConfig->strDevCode.c_str()
        , CSipMgr::m_pConfig->strAddrIP.c_str()
        , CSipMgr::m_pConfig->strAddrPort.c_str());
    CSipFromToHeader stTo;
    PlatFormInfo* pPlatform = DeviceMgr::GetPlatformInfo();
    stTo.SetHeader(pPlatform->strDevCode.c_str(), strAddrIP.c_str(), strAddrPort.c_str());

    // 创建消息结构
    osip_message_t *dcMsg = 0;
    int nID = eXosip_message_build_request(m_pExContext, &dcMsg, "MESSAGE", 
        stTo.GetFormatHeader().c_str(), stFrom.GetFormatHeader().c_str(), nullptr);
    if (nID != OSIP_SUCCESS)
    {
        Log::error("eXosip_message_build_request failed");
        return;
    }

    // 控制命令
    uchar cControlCode = 0;
    if(nLeftRight == 2) cControlCode|=0x01;       // 右移
    else if(nLeftRight == 1) cControlCode|=0x02;  // 左移
    if (nUpDown == 2) cControlCode|=0x04;         // 下移
    else if(nUpDown == 1) cControlCode|=0x08;     // 上移
    if (nInOut == 2) cControlCode |= 0x10;        // 放大
    else if(nInOut == 1) cControlCode |= 0x20;    // 缩小
    char szCmd[20]={0};
    char szTmp[10]={0};
    szCmd[0] = 'A'; //字节1 A5
    szCmd[1] = '5';
    szCmd[2] = '0'; //字节2 0F
    szCmd[3] = 'F'; 
    szCmd[4] = '0'; //字节3 地址的低8位
    szCmd[5] = '1'; 
    sprintf_s(szTmp, 10,"%02X", cControlCode); 
    //Log::debug("cControlCode is %s", szTmp);
    szCmd[6]  = szTmp[0];  //字节4 控制码
    szCmd[7]  = szTmp[1];
    sprintf_s(szTmp, 10,"%02X", cMoveSpeed); 
    //Log::debug("cMoveSpeed is %s", szTmp);
    szCmd[8]  = szTmp[0];  //字节5 水平控制速度
    szCmd[9]  = szTmp[1];
    szCmd[10] = szTmp[0];  //字节6 垂直控制速度
    szCmd[11] = szTmp[1];
    sprintf_s(szTmp, 10,"%X", cInOutSpeed); 
    //Log::debug("cInOutSpeed is %s", szTmp);
    szCmd[12] = szTmp[0];  //字节7高4位 缩放控制速度
    szCmd[13] = '0';       //字节7低4位 地址的高4位
    //计算校验码
    int nCheck = (0XA5 + 0X0F + 0X01 + cControlCode + cMoveSpeed + cMoveSpeed + cInOutSpeed<<4&0XF0)%0X100;
    sprintf_s(szTmp,10,"%02X", nCheck);
    //Log::debug("nCheck is %s", szTmp);
    szCmd[14] = szTmp[0]; //字节8 校验码
    szCmd[15] = szTmp[1];
    Log::debug("PTZCmd is %s", szCmd);

    // 组成报文体
    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
        << "<Control>\r\n"
        << "<CmdType>DeviceControl</CmdType>\r\n"
        << "<SN>" << sn++ << "</SN>\r\n"
        << "<DeviceID>" << strDevCode << "</DeviceID>\r\n"
        << "<PTZCmd>" << szCmd << "</PTZCmd>\r\n"
        << "</Control>\r\n";
    string strBody = ss.str();

    osip_message_set_contact(dcMsg, stFrom.GetFormatHeader().c_str());
    osip_message_set_content_type (dcMsg, "Application/MANSCDP+xml");
    osip_message_set_body (dcMsg, strBody.c_str(), strBody.length());

    eXosip_lock(m_pExContext);
    int ret = eXosip_message_send_request(m_pExContext, dcMsg);
    eXosip_unlock(m_pExContext);
    if (ret <= 0)
    {
        Log::error("eXosip_message_send_request failed:%d\r\n",ret);
    }
    else
    {
        Log::warning("send DeviceControl message\r\n");
    }
}

void CSipMessage::parserMessageInfo(osip_message_t*request, int iReqId, sipMessageInfo &msgInfo)
{
    std::stringstream stream;

    if (nullptr == request)
    {
        Log::error("CSipMessage::parserMessageInfo nullptr == request");
        return;
    }
	/*
    Log::debug("CSipMessage::parserMessageInfo from user:%s,host:%s,port:%s; to user:%s,host:%s,port:%s"
        ,request->from->url->username,request->from->url->host,request->from->url->port
        ,request->to->url->username,request->to->url->host,request->to->url->port);
		*/

    msgInfo.method = request->sip_method;
    msgInfo.from.SetHeader(request->from->url->username,
        request->from->url->host, 
        request->from->url->port);
    msgInfo.to.SetHeader(request->to->url->username,
        request->to->url->host, 
        request->to->url->port);
    msgInfo.sipRequestId = iReqId;
    msgInfo.callId = request->call_id->number;

    //解析content消息
    osip_body_t * body = NULL;
    osip_message_get_body(request, 0, &body);
    if (body != nullptr && nullptr != body->body)
    {
        msgInfo.content = body->body;
        msgInfo.strCmdType = m_msgParser.ParseMsgBody(&msgInfo.pMsgBody, body->body);
    }
}

void CSipMessage::printMeaassgePkt(sipMessageInfo& info)
{
    LogDebug("接收到报文：");
    LogDebug("================================================================");
    LogDebug("method:      %s",info.method.c_str());
    LogDebug("from:        %s",info.from.GetFormatHeader().c_str());
    LogDebug("to:          %s",info.to.GetFormatHeader().c_str());

    //注册返回 由发送方维护的请求ID 接收方接收后原样返回即可
    LogDebug("RequestId:   %d",info.sipRequestId);
    //CALL_ID
    LogDebug("Call-Id:     %s",info.callId.c_str());
    //解析content消息
    LogDebug("CmdType:     %s",info.strCmdType.c_str());
    LogDebug("\r\n%s"         ,info.content.c_str());
    LogDebug("================================================================");
}

void CSipMessage::sendMessageAnswer(sipMessageInfo& info)
{
    //if (info.strCmdType == "Keepalive" || 
    //    info.strCmdType == "Catalog")
    {
        //Log::debug("CSipMessage::sendMessageAnswer answer Keepalive");
        osip_message_t* answer = NULL;
        int iStatus;
        eXosip_lock(m_pExContext);
        if (DeviceMgr::IsPlatformLive())
        {
            iStatus = 200;
            int result = ::eXosip_message_build_answer(m_pExContext,info.sipRequestId,iStatus, &answer);
            if (OSIP_SUCCESS != result)
            {
                ::eXosip_message_send_answer(m_pExContext,info.sipRequestId, 400, NULL);
                //Log::warning("保活应答 发送400报文");
            }
            else
            {
                //发送消息体
                ::eXosip_message_send_answer(m_pExContext,info.sipRequestId, iStatus, answer);
                //Log::warning("保活应答 发送200报文");
            }
        }
        eXosip_unlock(m_pExContext);
    }
}