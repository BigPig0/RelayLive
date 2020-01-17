// sever.cpp : 定义控制台应用程序的入口点。
//
#include "util.h"
#include "DeviceMgr.h"
#include "SipServer.h"
#include "MiniDump.h"
#include "ipc.h"
#include "utilc_api.h"
#include "stdio.h"



int main()
{
    /** Dump设置 */
    CMiniDump dump("sipServer.dmp");

    /** 创建日志文件 */
    char path[MAX_PATH];
    sprintf_s(path, MAX_PATH, ".\\log\\sipServer.txt");
    Log::open(Log::Print::both, Log::Level::debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    /** 加载配置文件 */
    if (!Settings::loadFromProfile(".\\config.txt"))
    {
        Log::error("配置文件错误");
        return -1;
    }
    Log::debug("Settings::loadFromProfile ok");

    /** 进程间通信 */
    IPC::Init();

    /** 初始化设备模块 */
    if (!DeviceMgr::Init())
    {
        Log::error("DeviceManagerInstance init failed");
        return -1;
    }
    Log::debug("DeviceMgr::Init ok");

    /** 初始化SIP服务器 */
    if (!SipServer::Init())
    {
        Log::error("SipInstance init failed");
        return -1;
    }
    Log::debug("SipInstance::Init ok");
    
    sleep(INFINITE);
    return 0;
}