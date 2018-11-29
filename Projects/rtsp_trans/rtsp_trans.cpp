// sever.cpp : 定义控制台应用程序的入口点。
//
#include "common.h"
#include "DeviceMgr.h"
#include "MiniDump.h"
#include "uvIpc.h"
#include "util_api.h"
#include "stdio.h"

void on_ipc_recv(uv_ipc_handle_t* h, void* user, char* name, char* msg, char* data, int len)
{

}

int main()
{
    /** Dump设置 */
    CMiniDump dump("rtsp_trans.dmp");

    /** 进程间通信 */
    uv_ipc_handle_t* h = NULL;
    int ret = uv_ipc_client(&h, "relay_live", NULL, "liveSrc", on_ipc_recv, NULL);
    if(ret < 0) {
        printf("ipc server err: %s\n", uv_ipc_strerr(ret));
    }

    /** 创建日志文件 */
    char path[MAX_PATH];
    sprintf_s(path, MAX_PATH, ".\\log\\rtspTrans.txt");
    Log::open(Log::Print::both, Log::Level::debug, path);

    /** 加载配置文件 */
    if (!Settings::loadFromProfile(".\\config.txt"))
    {
        Log::error("配置文件错误");
        return -1;
    }
    Log::debug("Settings::loadFromProfile ok");


    /** 初始化设备模块 */
    if (!DeviceMgr::Init())
    {
        Log::error("DeviceManagerInstance init failed");
        return -1;
    }
    
    sleep(INFINITE);
    return 0;
}