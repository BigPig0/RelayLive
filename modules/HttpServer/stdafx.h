// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�  
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

#define g_strError_no_platform       "1 û��ƽ̨ע��"
#define g_strError_no_device         "2 û���豸ע��"
#define g_strError_play_faild        "3 ����ʧ��"
#define g_strError_keepalive_failed  "4 ��������ʧ��"
#define g_strError_control_failed    "5 �豸��������ʧ��"
