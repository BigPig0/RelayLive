#include "SipPrivate.h"
#include "SipInvite.h"

namespace SipServer {

    struct SipCall
    {
        // 会话信息
        int             m_nCallID;     //< invite建立时得到的ID
        int             m_nDialogID;   //< 建立成功可以从osip_event中得到
        int             m_nRtpPort;    //< rtp接收端口，rtcp端口加1
        string          m_strRtpIP;    //< rtp服务IP
        // 设备信息
        string          m_strDevCode;  //< 设备的编码
        string          m_strProName;  //< livesvr的名称
        uint32_t        m_nID;         //< livesvr发起的命令ID

        int             m_nInvite;     //< 邀请状态标记，发送邀请置为0，收到邀请应答置为1
        string          m_strBody;     //< 邀请成功收到的body，是一个sdp信息

        //历史视频点播
        bool            m_bRecord;
        string          m_strBeginTime;
        string          m_strEndTime;
    };

    static map<int,SipCall*>     m_mapGlobalCall;  //< 所有的该类实例建立索引，key是callid，以便响应时找到
    static map<int,SipCall*>     m_mapDeviceCall;  //< key是rtp端口。因为端口不重复，可以作为ID
    static CriticalSection       m_csGlobalCall;
    static int nCallID = 0;

static SipCall* FindByCallID(int nCID) {
    Log::debug(" cid is %d",nCID);
    MutexLock lock(&m_csGlobalCall);
    auto it = m_mapGlobalCall.find(nCID);
    if (it == m_mapGlobalCall.end())
    {
        Log::error("can't find cid %d",nCID);
        return nullptr;
    }

    return it->second;
}

static SipCall* FindByPort(uint32_t port) {
    Log::debug("port is %d", port);
    MutexLock lock(&m_csGlobalCall);
    auto it = m_mapDeviceCall.find(port);
    if (it == m_mapDeviceCall.end())
    {
        Log::error("can't find cid %d", port);
        return nullptr;
    }

    return it->second;
}

void CSipInvite::InviteInit(string strProName, uint32_t nID, string strCode, int nRTPPort) {
    Log::debug("InviteInit %s: %s %d", strProName.c_str(), strCode.c_str(), nRTPPort);

    SipCall *call = new SipCall();
    call->m_nCallID = -1;
    call->m_nDialogID = -1;
    call->m_nRtpPort = nRTPPort;
    call->m_strDevCode = strCode;
    call->m_strProName = strProName;
    call->m_nID = nID;
    call->m_nInvite = 0;
    call->m_bRecord = false;
    MutexLock lock(&m_csGlobalCall);
    m_mapDeviceCall.insert(make_pair(nRTPPort,call));
}

int CSipInvite::SendInvite(string strProName, uint32_t nID, int nRTPPort)
{
    SipCall* pCall = FindByPort(nRTPPort);
    if(NULL == pCall)
        return -1;

    string strFrom = GetFormatHeader(g_strCode , g_strSipIP , g_nSipPort);
    string strTo   = GetFormatHeader(pCall->m_strDevCode, g_strLowIP, g_nLowPort);
    string strSubject = GetSubjectHeader(pCall->m_strDevCode, g_strCode, false);

    Log::debug("invite %s %d", pCall->m_strDevCode.c_str(), nRTPPort);

    osip_message_t *cinvMsg = NULL;
    int nIviteID = eXosip_call_build_initial_invite(g_pExContext, &cinvMsg, strTo.c_str(), 
        strFrom.c_str(), nullptr, strSubject.c_str());
    if (nIviteID != OSIP_SUCCESS) {
        Log::error("init invite failed");
        return -1;
    }

    
    nCallID++;
    if(nCallID > 9999) nCallID = 0;
    char szSSRC[5]={0};
    sprintf_s(szSSRC,"%04d",nCallID);

    stringstream ss;
    ss << "v=0\r\n"
        << "o=" << g_strCode << " " << nCallID << " 0 IN IP4 " << g_strSipIP << "\r\n"
        << "s=Play\r\n"
        << "c=IN IP4 " << g_strSipIP << "\r\n"
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

    eXosip_lock(g_pExContext);
    int ret = eXosip_call_send_initial_invite(g_pExContext, cinvMsg);
    if (ret <= 0) {
        Log::error("send invite failed:%d",ret);
    }
    //osip_message_free(cinvMsg);
    eXosip_unlock(g_pExContext);

    pCall->m_nCallID = ret;
    MutexLock lock(&m_csGlobalCall);
    m_mapGlobalCall.insert(make_pair(ret, pCall));

    return ret;
}

int CSipInvite::SendRecordInvite(string strProName, uint32_t nID, int nRTPPort, string beginTime, string endTime)
{
    SipCall* pCall = FindByPort(nRTPPort);
    if(NULL == pCall)
        return -1;

    string strFrom = GetFormatHeader(g_strCode , g_strSipIP , g_nSipPort);
    string strTo   = GetFormatHeader(pCall->m_strDevCode, g_strLowIP, g_nLowPort);
    string strSubject = GetSubjectHeader(pCall->m_strDevCode, g_strCode, true);

    Log::debug("CSipInvite::SendInvite");

    osip_message_t *cinvMsg = NULL;
    int nIviteID = eXosip_call_build_initial_invite(g_pExContext, &cinvMsg, strTo.c_str(), 
        strFrom.c_str(), nullptr, strSubject.c_str());
    if (nIviteID != OSIP_SUCCESS)
    {
        Log::error("CSipInvite::SendInvite init invite failed");
        return -1;
    }

    static int nCallID = 0;
    char szSSRC[5]={0};
    sprintf_s(szSSRC,"%04d",nCallID);

    stringstream ss;
    ss << "v=0\r\n"
        << "o=" << g_strCode << " " << nCallID++ << " 0 IN IP4 " << g_strSipIP << "\r\n"
        << "s=Playback\r\n"
        << "c=IN IP4 " << g_strSipIP << "\r\n"
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

    eXosip_lock(g_pExContext);
    int ret = eXosip_call_send_initial_invite(g_pExContext, cinvMsg);
    if (ret <= 0) {
        Log::error("CSipInvite::SendInvite send failed:%d",ret);
    }
    //osip_message_free(cinvMsg);
    pCall->m_nCallID = ret;
    pCall->m_strBeginTime = beginTime;
    pCall->m_strEndTime = endTime;
    MutexLock lock(&m_csGlobalCall);
    m_mapGlobalCall.insert(make_pair(ret, pCall));

    eXosip_unlock(g_pExContext);
    return ret;
}

void CSipInvite::OnInviteOK(eXosip_event_t *osipEvent)
{
    Log::debug("on invite ok cid:%d,did:%d",osipEvent->cid, osipEvent->did);
    // 保存did
    osip_body_t *body;
    osip_message_get_body(osipEvent->response,0,&body);
    SipCall* pCall = FindByCallID(osipEvent->cid);
    CHECK_POINT_VOID(pCall);
    pCall->m_nDialogID = osipEvent->did;
    pCall->m_strBody = string(body->body, body->length);
    pCall->m_nInvite = 1;

    // 组织ack报文
    osip_message_t *cackMsg = NULL;
    int nID = eXosip_call_build_ack(g_pExContext,osipEvent->did,&cackMsg);
    if (nID != OSIP_SUCCESS) {
        Log::error("init invite ok ack failed:%d",nID);
        return;
    }

    // 发送ack报文
    eXosip_lock(g_pExContext);
    int ret = eXosip_call_send_ack(g_pExContext,osipEvent->did, cackMsg);
    if (ret < 0){
        Log::error("send invite ok ack failed:%d",ret);
    }
    eXosip_unlock(g_pExContext);

	IPC::on_play_cb(pCall->m_strProName, true, pCall->m_nID, pCall->m_nRtpPort, pCall->m_strBody);
}

void CSipInvite::OnInviteFailed(eXosip_event_t *osipEvent)
{
    Log::debug("on invite failed cid:%d,did:%d",osipEvent->cid,osipEvent->did);
    // 保存did
    osip_body_t *body;
    osip_message_get_body(osipEvent->response,0,&body);
    SipCall* pCall = FindByCallID(osipEvent->cid);
    CHECK_POINT_VOID(pCall);
    pCall->m_nDialogID = -1;
    pCall->m_nInvite = 1;

    // 组织ack报文
    osip_message_t *cackMsg = NULL;
    int nID = eXosip_call_build_ack(g_pExContext,osipEvent->did,&cackMsg);
    if (nID != OSIP_SUCCESS){
        Log::error("init invite failed ack failed:%d",nID);
        return;
    }

    // 发送ack报文
    eXosip_lock(g_pExContext);
    int ret = eXosip_call_send_ack(g_pExContext,osipEvent->did, cackMsg);
    if (ret < 0) {
        Log::error("send invite failed ack failed:%d",ret);
    }
    //osip_message_free(cackMsg);
    eXosip_unlock(g_pExContext);
}

bool CSipInvite::StopSipCall(uint32_t nRtpPort)
{
    MutexLock lock(&m_csGlobalCall);
    auto find = m_mapDeviceCall.find(nRtpPort);
    if (find == m_mapDeviceCall.end()) {
        Log::error("m_mapDeviceCall isn't find %d",nRtpPort);
        return false;
    }
    SipCall *pCall = find->second;
    m_mapDeviceCall.erase(find);
    if (pCall == nullptr) {
        Log::error("sipCall in m_mapDeviceCall is null");
        return false;
    }

    int nCallID = pCall->m_nCallID;
    int nDialogID = pCall->m_nDialogID;
    delete pCall;
    auto findCall = m_mapGlobalCall.find(nCallID);
    if (findCall == m_mapGlobalCall.end()) {
        Log::error("m_mapGlobalCall isn't find %d:%d",nRtpPort,nCallID);
        return false;
    }
    m_mapGlobalCall.erase(findCall);

    // 发送bye报文
    if(nCallID >= 0 && nDialogID >= 0) {
        eXosip_lock(g_pExContext);
        int ret = eXosip_call_terminate(g_pExContext, nCallID, nDialogID);
        if (ret < 0) {
            Log::error("call terminate failed:%d",ret);
        }
        eXosip_unlock(g_pExContext);
    }

    Log::debug("Stopped call %d",nRtpPort);
    return true;
}

vector<uint32_t> CSipInvite::StopSipCallAll(string strProName) {
    vector<uint32_t> ret;
    MutexLock lock(&m_csGlobalCall);
    for (auto it = m_mapDeviceCall.begin(); it != m_mapDeviceCall.end();) {
        if (it->second->m_strProName != strProName){
            it++;
            continue;
        }
        SipCall *pCall = it->second;
        it = m_mapDeviceCall.erase(it);
        if (pCall == nullptr) {
            Log::error("sipCall in m_mapDeviceCall is null");
            continue;
        }
        ret.push_back(pCall->m_nRtpPort);
        int nCallID = pCall->m_nCallID;
        int nDialogID = pCall->m_nDialogID;
		Log::debug("Stop call port:%d, code:%s", pCall->m_nRtpPort, pCall->m_strDevCode.c_str());
        delete pCall;
        auto findCall = m_mapGlobalCall.find(nCallID);
        if (findCall != m_mapGlobalCall.end()) {
            m_mapGlobalCall.erase(findCall);
        }

        // 发送bye报文
        if(nCallID >= 0 && nDialogID >= 0) {
            eXosip_lock(g_pExContext);
            int ret = eXosip_call_terminate(g_pExContext, nCallID, nDialogID);
            if (ret < 0) {
                Log::error("call terminate failed:%d",ret);
            }
            eXosip_unlock(g_pExContext);
        }
    }
    Log::debug("Stopped call all %s",strProName.c_str());
    return ret;
}

void CSipInvite::OnCallNew(eXosip_event_t *osipEvent)
{
    Log::debug("cid:%d,did:%d",osipEvent->cid,osipEvent->did);
    osip_body_t *body;
    osip_message_get_body(osipEvent->response,0,&body);
	CHECK_POINT_VOID(body);
    Log::debug(body->body);
    SipCall* pCall = FindByCallID(osipEvent->cid);
    Log::debug("find pCall:%d",pCall);

    // 组织ack报文
    osip_message_t *cackMsg = 0;
    int nID = eXosip_call_build_ack(g_pExContext,osipEvent->did,&cackMsg);
    if (nID != OSIP_SUCCESS) {
        Log::error("eXosip_call_build_ack failed:%d",nID);
        return;
    }

    // 发送ack报文
    eXosip_lock(g_pExContext);
    int ret = eXosip_call_send_ack(g_pExContext,osipEvent->did, cackMsg);
    if (ret < 0) {
        Log::error("eXosip_call_send_ack failed:%d",ret);
    }
    //osip_message_free(cackMsg);
    eXosip_unlock(g_pExContext);
}

void CSipInvite::OnCallClose(eXosip_event_t *osipEvent)
{
    Log::debug("cid:%d,did:%d",osipEvent->cid,osipEvent->did);
    osip_body_t *body;
    osip_message_get_body(osipEvent->response,0,&body);
    Log::debug(body->body);
    SipCall* pCall = FindByCallID(osipEvent->cid);
    Log::debug("find pCall:%d %d",pCall->m_nRtpPort, pCall->m_nDialogID);

    // 组织ack报文
    osip_message_t *cackMsg = 0;
    int nID = eXosip_call_build_ack(g_pExContext,osipEvent->did,&cackMsg);
    if (nID != OSIP_SUCCESS) {
        Log::error("eXosip_call_build_ack failed:%d",nID);
        return;
    }

    // 发送ack报文
    eXosip_lock(g_pExContext);
    int ret = eXosip_call_send_ack(g_pExContext,osipEvent->did, cackMsg);
    if (ret < 0) {
        Log::error("eXosip_call_send_ack failed:%d",ret);
    }
    //osip_message_free(cackMsg);
    eXosip_unlock(g_pExContext);
}

void CSipInvite::OnCallClear(eXosip_event_t *osipEvent)
{
    Log::debug("cid:%d,did:%d",osipEvent->cid,osipEvent->did);
    osip_body_t *body;
    osip_message_get_body(osipEvent->response,0,&body);
    Log::debug(body->body);
    SipCall* pCall = FindByCallID(osipEvent->cid);
    Log::debug("find pCall:%d",pCall);
}

};