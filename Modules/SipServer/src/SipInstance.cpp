#include "stdafx.h"
#include "SipInstance.h"
#include "SipMgr.h"
#include "SipCall.h"

void* SipInstance::m_pHandle = nullptr;

SipInstance::SipInstance(void)
{
}


SipInstance::~SipInstance(void)
{
}

bool SipInstance::Init()
{
    CSipMgr* pSipMgr = new CSipMgr();
    if(nullptr == pSipMgr)
        return false;

    m_pHandle = pSipMgr;
    pSipMgr->Init();

    return true;
}

void SipInstance::Cleanup()
{
    SAFE_DELETE(m_pHandle);
}

bool SipInstance::rtsp_play(string devCode, string rtpIP, int rtpPort)
{
    // 创建会话邀请
    if (!CSipCall::CreatSipCall(devCode, rtpIP, rtpPort))
    {
        Log::error("creat sip call failed");
        return false;
    }

    return true;
}

bool SipInstance::RealPlay(string strDev, string rtpIP, int rtpPort)
{
    Log::debug("start SipInstance::RealPlay strDev:%s",strDev.c_str());

    // 创建会话邀请
    if (!CSipCall::CreatSipCall(strDev, rtpIP, rtpPort))
    {
        Log::error("creat sip call failed");
        return false;
    }
    return true;
}

bool SipInstance::StopPlay(string rtpPort)
{
    Log::debug("stop %s",rtpPort.c_str());
    CSipCall::StopSipCall(rtpPort);

    return true;
}

bool SipInstance::StopPlayAll()
{
    Log::debug("stop all");
    CSipCall::StopSipCallAll();

    return true;
}

bool SipInstance::RecordPlay(string strDev, string startTime, string endTime)
{
    Log::debug("start SipInstance::RealPlay strDev:%s",strDev.c_str());
    string strIP;
    int nPort;

    // 创建会话邀请
    if (!CSipCall::CreatSipCall(strDev, strIP, nPort, startTime, endTime))
    {
        return false;
    }
    return true;
}

bool SipInstance::DeviceControl(string strDev, int nInOut, int nUpDown, int nLeftRight)
{
    if (m_pHandle == nullptr)
    {
        Log::error("sip mgr is not init");
        return false;
    }

    PlatFormInfo* pPlatform = DeviceMgr::GetPlatformInfo();
    if (pPlatform == nullptr)
    {
        Log::error("pPlatform is not exit");
        return false;
    }

    CSipMgr::m_pMessage->DeviceControl(pPlatform->strAddrIP,pPlatform->strAddrPort,
        strDev, nInOut, nUpDown, nLeftRight);

    return true;
}