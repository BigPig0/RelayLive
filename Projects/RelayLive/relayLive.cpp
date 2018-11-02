// sever.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h" 
#include "MiniDump.h"
#include "uv.h"
#include <thread>

int main()
{
    /** Dump设置 */
    CMiniDump dump("gb28181_video_control.dmp");

    /** 创建日志文件 */
    char path[MAX_PATH];
    sprintf_s(path, MAX_PATH, ".\\log\\gb28181_video_control.txt");
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

    /** 初始化SIP服务器 */
    if (!SipInstance::Init())
    {
        Log::error("SipInstance init failed");
        return -1;
    }
    Log::debug("SipInstance::Init ok");

    //根据cpu数量设置libuv线程池的线程数量
    static uv_loop_t *p_loop_uv = nullptr;
    uv_cpu_info_t* cpu_infos;
    int count;
    int err = uv_cpu_info(&cpu_infos, &count);
    if (err) {
        Log::warning("fail get cpu info: %s",uv_strerror(err));
    } else {
        wchar_t szCpuNum[10] = {0};
        swprintf_s(szCpuNum,L"%d", count);
        //设置环境变量的值
        ::SetEnvironmentVariableW(L"UV_THREADPOOL_SIZE",szCpuNum); 
    }

    //全局loop
    p_loop_uv = uv_default_loop();

    /** 创建一个http服务器 */
    HttpWsServer::Init((void**)&p_loop_uv);

    /** 创建一个rtsp服务器 */
    CRtspServer rtsp;
    rtsp.Init(p_loop_uv);

    Log::debug("GB28181 Sever start success\r\n");

    // 事件循环
    while(true)
    {
        uv_run(p_loop_uv, UV_RUN_DEFAULT);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    Sleep(INFINITE);
    return 0;
}