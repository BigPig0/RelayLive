#pragma once
#define WIN32_LEAN_AND_MEAN         //屏蔽windows.h中不常用的API，防止和Winsock2.h冲突
#pragma warning(disable:4127 4458)


#include "commonDefine.h"
#include "LastError.h"
#include "List.h"
#include "Clock.h"
#include "TimeFormat.h"
#include "Mutex.h"
#include "Log.h"
#include "Profile.h"
#include "Settings.h"
#include "Runnable.h"
#include "ThreadPool.h"
#include "Timer.h"
#include "memfile.h"
#include "WINFile.h"
#include "StringHandle.h"
#include "EncodeConvert.h"