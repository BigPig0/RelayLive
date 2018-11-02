// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: 在此处引用程序需要的其他头文件  
#include "common.h"
#include "uv.h"
#include "libwebsockets.h"

#include <stdlib.h>   
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <windows.h>

using namespace std;

#define g_strError_no_platform       "1 没有平台注册"
#define g_strError_no_device         "2 没有设备注册"
#define g_strError_play_faild        "3 播放失败"
#define g_strError_keepalive_failed  "4 保活请求失败"
#define g_strError_control_failed    "5 设备控制请求失败"
