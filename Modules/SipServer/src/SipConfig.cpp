#include "stdafx.h"
#include "SipConfig.h"

void CSipConfig::Load()
{
    string strCode = Settings::getValue("SipSever","Code");
    if(!strCode.empty())
        strDevCode = strCode;

    string strIP = Settings::getValue("SipSever","IP");
    if(!strIP.empty())
        strAddrIP = strIP;

    string strPort = Settings::getValue("SipSever","Port");
    if(!strPort.empty())
        strAddrPort = strPort;

    string strRegAuth = Settings::getValue("SipSever","RegAuthor");
    if(strRegAuth != "true")
        bRegAuthor = false;

    Log::debug("CConfig::Load SipSever Code:%s IP:%s,Port:%s,RegAuthor:%d",
        strDevCode.c_str(), strAddrIP.c_str(), strAddrPort.c_str(), bRegAuthor);
}
