// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: 在此处引用程序需要的其他头文件  
#include "util.h"

#include <stdlib.h>   
#include <iostream>
#include <string>
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

#include "DeviceMgr.h"

using namespace std;

#if 0
#define LogDebug(fmt, ...) Log::debug(fmt, __VA_ARGS__ )
#else
#define LogDebug(fmt, ...)
#endif