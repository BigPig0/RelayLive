// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include <stdio.h>
//#include <tchar.h>



// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�  
#include "util.h"
#include "utilc.h"
#include "SipServer.h"

#include <stdlib.h>   
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <thread>
//#include <windows.h>
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

namespace IPC {
    extern void on_play_init_cb(string strProName, uint32_t nID, uint32_t nPort);
    extern void on_play_cb(string strProName, bool bRet, uint32_t nID, uint32_t nPort, string strInfo);
}

extern void on_hour_event(time_t t);
extern void on_device(SipServer::DevInfo* dev);
extern void on_update_status(string strDevID, string strStatus);
extern void on_update_postion(string strDevID, string log, string lat);

extern map<string,SipServer::DevInfo*>  g_mapDevs;             //�¼�ƽ̨���͵��豸
extern util::CriticalSection            g_csDevs;   

namespace SipServer {
    struct DevInfo;

    extern string         g_strCode;             //��ƽ̨�������
    extern string         g_strSipIP;            //��ƽ̨����IP
    extern uint32_t       g_nSipPort;            //��ƽ̨Sip�����˿�
    extern bool           g_bRegAuthor;          //��ƽ̨�Ƿ���ע���Ȩ
    extern string         g_strLowCode;          //�¼�ƽ̨�������
    extern string         g_strLowIP;            //�¼�ƽ̨����IP
    extern uint32_t       g_nLowPort;            //�¼�ƽ̨Sip�����˿�
    extern bool           g_bLowStatus;          //�¼�ƽ̨����״̬
    extern uint32_t       g_nLowExpire;          //�¼�ƽ̨��ʱʱ��
    extern eXosip_t*      g_pExContext;          //exosipʵ��


    extern void AutoQuery();

    inline
    uint32_t sz2int(char* value) {
        if(NULL == value)
            return 0;
        int ret = 0;
        sscanf(value, "%d", &ret);
        return ret;
    }

    inline
    string sz2str(char* value) {
        if(NULL == value)
            return "";
        string ret(value);
        return ret;
    }

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