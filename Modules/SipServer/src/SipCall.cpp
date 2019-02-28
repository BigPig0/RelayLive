#include "stdafx.h"
#include "SipCall.h"
#include "SipMgr.h"

map<int,CSipCall*> CSipCall::m_mapGlobalCall;
map<string,CSipCall*> CSipCall::m_mapDeviceCall;
CriticalSection    CSipCall::m_csGlobalCall;

CSipCall::CSipCall()
    : m_nCallID(-1)
    , m_nDialogID(-1)
    , m_bRecord(false)
{
}

CSipCall::~CSipCall(void)
{
    if(!SendBye())
    {
        Log::error("CSipCall::~CSipCall send bye failed");
    }
}

bool CSipCall::CreatSipCall(string strDevCode, string strIP, int nPort)
{
    CSipCall* pNew          = new CSipCall;
    pNew->m_strDevCode      = strDevCode;
    pNew->m_strRtpIP        = strIP;
    pNew->m_nRtpPort        = nPort;

    // 发送通话邀请
    if (!pNew->SendInvite())
    {
        Log::error("SendInvite failed");
        return false;
    }

    // 等待通话邀请应答
    if (!pNew->WaiteInviteFinish())
    {
        Log::error("WaiteInviteFinish failed");
        return false;
    }
    return true;
}

bool CSipCall::CreatSipCall(string strDevCode, string strIP, int nPort, 
                  string startTime, string endTime)
{
    CSipCall* pNew          = new CSipCall;
    pNew->m_strDevCode      = strDevCode;
    pNew->m_strRtpIP        = strIP;
    pNew->m_nRtpPort        = nPort;
    pNew->m_bRecord         = true;
    pNew->m_strBeginTime    = startTime;
    pNew->m_strEndTime      = endTime;

    // 发送通话邀请
    if (!pNew->SendRecordInvite())
    {
        Log::error("SendRecordInvite failed");
        return false;
    }

    // 等待通话邀请应答
    if (!pNew->WaiteInviteFinish())
    {
        Log::error("WaiteInviteFinish failed");
        return false;
    }
    return true;
}

bool CSipCall::StopSipCall(string strRtpPort)
{
    CSipCall* pCall = nullptr;
    MutexLock lock(&m_csGlobalCall);
    auto find = m_mapDeviceCall.find(strRtpPort);
    if (find == m_mapDeviceCall.end())
    {
        Log::error("m_mapDeviceCall isn't find %s",strRtpPort.c_str());
        return false;
    }
    pCall = find->second;
    m_mapDeviceCall.erase(find);
    if (pCall == nullptr)
    {
        Log::error("sipCall in m_mapDeviceCall is null");
        return false;
    }

    int nCallID = pCall->m_nCallID;
    delete pCall;
    auto findCall = m_mapGlobalCall.find(nCallID);
    if (findCall == m_mapGlobalCall.end())
    {
        Log::error("m_mapDeviceCall isn't find %s:%d",strRtpPort.c_str(),nCallID);
        return false;
    }
    m_mapGlobalCall.erase(findCall);

    Log::debug("Stopped call %s",strRtpPort.c_str());
    return true;
}

bool CSipCall::StopSipCallAll()
{
    MutexLock lock(&m_csGlobalCall);
    for(auto devCall : m_mapDeviceCall)
    {
        delete devCall.second;
    }
    m_mapDeviceCall.clear();
    m_mapGlobalCall.clear();
    Log::debug("Stopped call all");
    return true;
}

bool CSipCall::SendInvite()
{
    PlatFormInfo* pPlatform = DeviceMgr::GetPlatformInfo();
    if (pPlatform == nullptr)
    {
        Log::error("platform is nullptr");
        return false;
    }
    DevInfo* pDevInfo = DeviceMgr::GetDeviceInfo(m_strDevCode);
    if (pDevInfo == nullptr)
    {
        Log::error("device is nullptr");
        return false;
    }

    m_nInvite = 0;
    m_nCallID = CSipMgr::m_pInvite->SendInvite(pPlatform,pDevInfo,m_nRtpPort);
    Log::debug("CSipCall::SendInvite m_nCallID:%d",m_nCallID);

    MutexLock lock(&m_csGlobalCall);
    m_mapGlobalCall.insert(make_pair(m_nCallID, this));
    m_mapDeviceCall.insert(make_pair(StringHandle::toStr<int>(m_nRtpPort),this));
    return true;
}

bool CSipCall::SendRecordInvite()
{
    PlatFormInfo* pPlatform = DeviceMgr::GetPlatformInfo();
    if (pPlatform == nullptr)
    {
        Log::error("platform is nullptr");
        return false;
    }
    DevInfo* pDevInfo = DeviceMgr::GetDeviceInfo(m_strDevCode);
    if (pDevInfo == nullptr)
    {
        Log::error("device is nullptr");
        return false;
    }

    m_nInvite = 0;
    m_nCallID = CSipMgr::m_pInvite->SendInvite(pPlatform,pDevInfo,m_nRtpPort);
    Log::debug("CSipCall::SendInvite m_nCallID:%d",m_nCallID);

    MutexLock lock(&m_csGlobalCall);
    m_mapGlobalCall.insert(make_pair(m_nCallID, this));
    m_mapDeviceCall.insert(make_pair(StringHandle::toStr<int>(m_nRtpPort),this));
    return true;
}

bool CSipCall::OnInviteOk(int nDID, char* szBody, int nLength)
{
    Log::debug("CSipCall::OnInviteOk nDID:%d,szBody:%s",nDID,szBody);
    m_nDialogID = nDID;
    m_nInvite   = 1;

    return true;
}

bool CSipCall::OnInviteFailed()
{
    Log::debug("CSipCall::OnInvite Failed");
    m_nDialogID = -1;
    m_nInvite   = 1;

    return true;
}

bool CSipCall::WaiteInviteFinish()
{
    time_t inviteTime = time(NULL);
    while (!m_nInvite)
    {
        Sleep(10);
        time_t nowTime = time(NULL);
        //Log::debug("inviteTime:%lld,nowTime:%lld",inviteTime,nowTime);
        if (nowTime - inviteTime > 20)
        {
            Log::error("WaiteInviteFinish Over time");
            return false;
        }
    }
    return m_nDialogID==-1?false:true;
}

bool CSipCall::SendBye()
{
    if (m_nCallID < 0 || m_nDialogID < 0)
    {
        Log::error("CSipCall::SendBye m_nCallID:%d,m_nDialogID:%d",m_nCallID,m_nDialogID);
        return false;
    }
    CSipMgr::m_pInvite->SendBye(m_nCallID, m_nDialogID);
    return true;
}

CSipCall* CSipCall::FindByCallID(int nCID)
{
    Log::debug("CSipCall::FindByCallID cid is %d",nCID);
    MutexLock lock(&m_csGlobalCall);
    auto it = m_mapGlobalCall.find(nCID);
    if (it == m_mapGlobalCall.end())
    {
        Log::error("CSipCall::FindByCallID not find cid %d",nCID);
        return nullptr;
    }

    return it->second;
}
