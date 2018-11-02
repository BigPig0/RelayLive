#include "stdafx.h"
#include "SipInvite.h"
#include "SipHeaders.h"
#include "SipMgr.h"
#include "SipCall.h"


CSipInvite::CSipInvite(eXosip_t* pSip)
    : m_pExContext(pSip)
{
}


CSipInvite::~CSipInvite(void)
{
}

int CSipInvite::SendInvite(PlatFormInfo* pPlatform, DevInfo* pDevInfo, int nRTPPort)
{
    CSipFromToHeader stFrom;
    stFrom.SetHeader(CSipMgr::m_pConfig->strDevCode.c_str()
        , CSipMgr::m_pConfig->strAddrIP.c_str()
        , CSipMgr::m_pConfig->strAddrPort.c_str());
    CSipFromToHeader stTo;
    stTo.SetHeader(pDevInfo->strDevID.c_str(), pPlatform->strAddrIP.c_str(), pPlatform->strAddrPort.c_str());
    CSubjectHeader stSubject;
    stSubject.SetHeader(pDevInfo->strDevID.c_str(),CSipMgr::m_pConfig->strDevCode.c_str(),false);

    Log::debug("CSipInvite::SendInvite");

    static osip_message_t *cinvMsg = 0;
    int nID = eXosip_call_build_initial_invite(m_pExContext, &cinvMsg, stTo.GetFormatHeader().c_str(), 
        stFrom.GetFormatHeader().c_str(), nullptr, stSubject.GetFormatHeader().c_str());
    if (nID != OSIP_SUCCESS)
    {
        Log::error("CSipInvite::SendInvite init invite failed");
        return -1;
    }

    static int nCallID = 0;
    char szSSRC[5]={0};
    sprintf_s(szSSRC,"%04d",nCallID);

    stringstream ss;
    ss << "v=0\r\n"
        << "o=" << CSipMgr::m_pConfig->strDevCode << " " << nCallID++ << " 0 IN IP4 " << CSipMgr::m_pConfig->strAddrIP << "\r\n"
        << "s=Play\r\n"
        << "c=IN IP4 " << CSipMgr::m_pConfig->strAddrIP << "\r\n"
        << "t=0 0\r\n"
        << "m=video " << nRTPPort << " RTP/AVP 96 98 97\r\n"
        << "a=recvonly\r\n"
        << "a=rtpmap:96 PS/90000\r\n"
        << "a=rtpmap:98 H264/90000\r\n"
        << "a=rtpmap:97 MPEG/90000\r\n"
        //<< "y=0" << strDevCode[3] << strDevCode[4] << strDevCode[5] << strDevCode[6] << strDevCode[7] << szSSRC << "\r\n"
        //<< "f=v/2/1/10/2/10000 a=///\r\n"
        ;
    string strBody = ss.str();

    //osip_message_set_contact(cinvMsg, stFrom.GetFormatHeader().c_str());
    osip_message_set_content_type(cinvMsg, "APPLICATION/SDP");
    osip_message_set_body (cinvMsg, strBody.c_str(), strBody.length());

    eXosip_lock(m_pExContext);
    int ret = eXosip_call_send_initial_invite(m_pExContext, cinvMsg);
    if (ret <= 0)
    {
        Log::error("CSipInvite::SendInvite send failed:%d",ret);
    }
    eXosip_unlock(m_pExContext);
    return ret;
}

int CSipInvite::SendRecordInvite(PlatFormInfo* pPlatform, DevInfo* pDevInfo, int nRTPPort, string beginTime, string endTime)
{
    CSipFromToHeader stFrom;
    stFrom.SetHeader(CSipMgr::m_pConfig->strDevCode.c_str()
        , CSipMgr::m_pConfig->strAddrIP.c_str()
        , CSipMgr::m_pConfig->strAddrPort.c_str());
    CSipFromToHeader stTo;
    stTo.SetHeader(pDevInfo->strDevID.c_str(), pPlatform->strAddrIP.c_str(), pPlatform->strAddrPort.c_str());
    CSubjectHeader stSubject;
    stSubject.SetHeader(pDevInfo->strDevID.c_str(),CSipMgr::m_pConfig->strDevCode.c_str(),false);

    Log::debug("CSipInvite::SendInvite");

    static osip_message_t *cinvMsg = 0;
    int nID = eXosip_call_build_initial_invite(m_pExContext, &cinvMsg, stTo.GetFormatHeader().c_str(), 
        stFrom.GetFormatHeader().c_str(), nullptr, stSubject.GetFormatHeader().c_str());
    if (nID != OSIP_SUCCESS)
    {
        Log::error("CSipInvite::SendInvite init invite failed");
        return -1;
    }

    static int nCallID = 0;
    char szSSRC[5]={0};
    sprintf_s(szSSRC,"%04d",nCallID);

    stringstream ss;
    ss << "v=0\r\n"
        << "o=" << CSipMgr::m_pConfig->strDevCode << " " << nCallID++ << " 0 IN IP4 " << CSipMgr::m_pConfig->strAddrIP << "\r\n"
        << "s=Playback\r\n"
        << "c=IN IP4 " << CSipMgr::m_pConfig->strAddrIP << "\r\n"
        << "u=" << "\r\n"
        << "t=" << CTimeFormat::scanTime(beginTime.c_str()) << " " << CTimeFormat::scanTime(endTime.c_str()) << "\r\n"
        << "m=video " << nRTPPort << " RTP/AVP 96 98 97\r\n"
        << "a=recvonly\r\n"
        << "a=rtpmap:96 PS/90000\r\n"
        << "a=rtpmap:98 H264/90000\r\n"
        << "a=rtpmap:97 MPEG/90000\r\n"
        //<< "y=0" << strDevCode[3] << strDevCode[4] << strDevCode[5] << strDevCode[6] << strDevCode[7] << szSSRC << "\r\n"
        //<< "f=v/2/1/10/2/10000 a=///\r\n"
        ;
    string strBody = ss.str();

    //osip_message_set_contact(cinvMsg, stFrom.GetFormatHeader().c_str());
    osip_message_set_content_type(cinvMsg, "APPLICATION/SDP");
    osip_message_set_body (cinvMsg, strBody.c_str(), strBody.length());

    eXosip_lock(m_pExContext);
    int ret = eXosip_call_send_initial_invite(m_pExContext, cinvMsg);
    if (ret <= 0)
    {
        Log::error("CSipInvite::SendInvite send failed:%d",ret);
    }
    eXosip_unlock(m_pExContext);
    return ret;
}

void CSipInvite::OnInviteOK(eXosip_event_t *osipEvent)
{
    Log::debug("CSipInvite::OnInviteOK cid:%d,did:%d",osipEvent->cid,osipEvent->did);
    // 保存did
    osip_body_t *body;
    osip_message_get_body(osipEvent->response,0,&body);
    CSipCall* pCall = CSipCall::FindByCallID(osipEvent->cid);
    pCall->OnInviteOk(osipEvent->did,body->body,body->length);

    // 组织ack报文
    osip_message_t *cackMsg = 0;
    int nID = eXosip_call_build_ack(m_pExContext,osipEvent->did,&cackMsg);
    if (nID != OSIP_SUCCESS)
    {
        Log::error("CSipInvite::OnInviteOK init invite failed:%d",nID);
        return;
    }

    // 发送ack报文
    eXosip_lock(m_pExContext);
    int ret = eXosip_call_send_ack(m_pExContext,osipEvent->did, cackMsg);
    if (ret < 0)
    {
        Log::error("CSipInvite::OnInviteOK send ack failed:%d",ret);
    }
    eXosip_unlock(m_pExContext);

    //Sleep(30000);
    //eXosip_call_terminate(m_pExContext,osipEvent->cid,osipEvent->did);
}

void CSipInvite::SendBye(int cid, int did)
{
    // 发送bye报文
    eXosip_lock(m_pExContext);
    int ret = eXosip_call_terminate(m_pExContext,cid,did);
    if (ret < 0)
    {
        Log::error("CSipInvite::SendBye send failed:%d",ret);
    }
    eXosip_unlock(m_pExContext);
}

void CSipInvite::OnCallNew(eXosip_event_t *osipEvent)
{
    Log::debug("cid:%d,did:%d",osipEvent->cid,osipEvent->did);
    osip_body_t *body;
    osip_message_get_body(osipEvent->response,0,&body);
	CHECK_POINT_VOID(body);
    Log::debug(body->body);
    CSipCall* pCall = CSipCall::FindByCallID(osipEvent->cid);
    Log::debug("find pCall:%d",pCall);

    // 组织ack报文
    osip_message_t *cackMsg = 0;
    int nID = eXosip_call_build_ack(m_pExContext,osipEvent->did,&cackMsg);
    if (nID != OSIP_SUCCESS)
    {
        Log::error("eXosip_call_build_ack failed:%d",nID);
        return;
    }

    // 发送ack报文
    eXosip_lock(m_pExContext);
    int ret = eXosip_call_send_ack(m_pExContext,osipEvent->did, cackMsg);
    if (ret < 0)
    {
        Log::error("eXosip_call_send_ack failed:%d",ret);
    }
    eXosip_unlock(m_pExContext);
}

void CSipInvite::OnCallClose(eXosip_event_t *osipEvent)
{
    Log::debug("cid:%d,did:%d",osipEvent->cid,osipEvent->did);
    osip_body_t *body;
    osip_message_get_body(osipEvent->response,0,&body);
    Log::debug(body->body);
    CSipCall* pCall = CSipCall::FindByCallID(osipEvent->cid);
    Log::debug("find pCall:%d",pCall);

    // 组织ack报文
    osip_message_t *cackMsg = 0;
    int nID = eXosip_call_build_ack(m_pExContext,osipEvent->did,&cackMsg);
    if (nID != OSIP_SUCCESS)
    {
        Log::error("eXosip_call_build_ack failed:%d",nID);
        return;
    }

    // 发送ack报文
    eXosip_lock(m_pExContext);
    int ret = eXosip_call_send_ack(m_pExContext,osipEvent->did, cackMsg);
    if (ret < 0)
    {
        Log::error("eXosip_call_send_ack failed:%d",ret);
    }
    eXosip_unlock(m_pExContext);
}

void CSipInvite::OnCallClear(eXosip_event_t *osipEvent)
{
    Log::debug("cid:%d,did:%d",osipEvent->cid,osipEvent->did);
    osip_body_t *body;
    osip_message_get_body(osipEvent->response,0,&body);
    Log::debug(body->body);
    CSipCall* pCall = CSipCall::FindByCallID(osipEvent->cid);
    Log::debug("find pCall:%d",pCall);
}