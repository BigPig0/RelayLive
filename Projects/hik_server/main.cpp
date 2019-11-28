// sever.cpp : 定义控制台应用程序的入口点。
//
#include "util.h"
#include "server.h"
#include "hiksdk.h"
#include <windows.h>
#include "MiniDump.h"
#include "uv.h"

int main()
{
    /** Dump设置 */
    CMiniDump dump("hik_server.dmp");

    /** 创建日志文件 */
    char path[MAX_PATH];
    sprintf_s(path, MAX_PATH, ".\\log\\hik_server.txt");
    Log::open(Log::Print::both, Log::Level::debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    /** 加载配置文件 */
    if (!Settings::loadFromProfile(".\\config.txt"))
    {
        Log::error("配置文件错误");
        return -1;
    }
    Log::debug("Settings::loadFromProfile ok");

    //根据cpu数量设置libuv线程池的线程数量
    static uv_loop_t *p_loop_uv = nullptr;
    uv_cpu_info_t* cpu_infos;
    int count;
    int err = uv_cpu_info(&cpu_infos, &count);
    if (err) {
        Log::warning("fail get cpu info: %s",uv_strerror(err));
    } else {
        char szThreadNum[10] = {0};
        sprintf(szThreadNum, "%d", count*2+1);
        Log::debug("thread pool size is %s", szThreadNum);
        //设置环境变量的值
        //::SetEnvironmentVariableW(L"UV_THREADPOOL_SIZE",szCpuNum); 
        uv_os_setenv("UV_THREADPOOL_SIZE", szThreadNum);
    }
    uv_free_cpu_info(cpu_infos, count);

    //全局loop
    p_loop_uv = uv_default_loop();

    HikPlat::Init();

    /** 创建一个http服务器 */
    Server::Init((void*)p_loop_uv);

    Log::debug("hik tmc sever start success\r\n");

    // 事件循环
    while(true)
    {
        uv_run(p_loop_uv, UV_RUN_DEFAULT);
        Sleep(1000);
    }
    Sleep(INFINITE);
    return 0;
}