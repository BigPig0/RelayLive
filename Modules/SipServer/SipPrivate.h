// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include <stdio.h>
#include <tchar.h>



// TODO: 在此处引用程序需要的其他头文件  
#include "util.h"
#include "utilc.h"

#include <stdlib.h>   
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <thread>
#include <windows.h>
//#include <Winsock2.h> 

#include <osipparser2/osip_message.h>
#include <osipparser2/osip_parser.h>
#include <osipparser2/osip_port.h>

#include <eXosip2/eXosip.h>
#include <eXosip2/eX_setup.h>
#include <eXosip2/eX_register.h>
#include <eXosip2/eX_options.h>
#include <eXosip2/eX_message.h>

using namespace std;

#if 0
#define LogDebug(fmt, ...) Log::debug(fmt, __VA_ARGS__ )
#else
#define LogDebug(fmt, ...)
#endif

namespace SipServer {
    struct DevInfo;

    extern string         g_strCode;             //本平台国标编码
    extern string         g_strSipIP;            //本平台本地IP
    extern uint32_t       g_nSipPort;            //本平台Sip监听端口
    extern bool           g_bRegAuthor;          //本平台是否开启注册鉴权
    extern string         g_strLowCode;          //下级平台国标编码
    extern string         g_strLowIP;            //下级平台本地IP
    extern uint32_t       g_nLowPort;            //下级平台Sip监听端口
    extern bool           g_bLowStatus;          //下级平台在线状态
    extern uint32_t       g_nLowExpire;          //下级平台超时时间
    extern map<string,DevInfo*> g_mapDevice;     //下级平台推送的设备
    extern eXosip_t*      g_pExContext;          //exosip实例

    extern void (*g_updateStatus)(string strDevID, int nOnline);
    extern void (*g_updatePostion)(string strDevID, double log, double lat);
    extern void (*g_addDevice)(DevInfo* dev);
    extern void (*g_playResult)(string strProName, bool bRet, uint32_t nID, uint32_t nPort, string strInfo);

    extern void AutoQuery();

    inline
    string GetFormatHeader(string code, string ip, uint32_t port = 0) {
            std::stringstream stream;
            stream << "<sip:" << code << "@" << ip;
            if(port)
                stream << ":" << port;
            stream << ">";
            return stream.str();
    }

    inline
    string GetContractFormatHeader(string code, string ip, uint32_t port, int expire) {
            std::stringstream stream;
            stream << "<sip:" << code << "@" << ip;
            if(port)
                stream << ":" << port;
            stream << ">;expires=" << expire;
            return stream.str();
    }

    inline
    string GetSubjectHeader(string senderCode, string reciverCode, bool isHistory = false) {
            static unsigned int sid=0,rid=0;
            std::stringstream stream;
            stream << senderCode << ":";
            if(isHistory)
                stream << "1";
            else
                stream << "0";
            stream << sid++ << "," << reciverCode << ":" << rid++;

            return stream.str();
    }
}