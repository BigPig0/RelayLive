// sever.cpp : 定义控制台应用程序的入口点。
//
#include "util.h"
#include "utilc.h"
#include "server.h"
#include "ipc.h"
#include "uv.h"
#include "easylog.h"

using namespace util;

int main(int argc, char* argv[])
{
    if(argc != 2)
        return -1;
    int port = atoi(argv[1]);

    /** Dump设置 */
    //CMiniDump dump("control_server.dmp");

    /** 创建日志文件 */
    char path[MAX_PATH];
#ifdef WINDOWS_IMPL
    sprintf(path, "./log/control_server_%d/log.txt", port);
#else
    sprintf(path, "/var/log/relaylive/control_server_%d/log.txt", port);
#endif
    Log::open(Log::Print::both, uvLogPlus::Level::Debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    /** 加载配置文件 */
#ifdef WINDOWS_IMPL
    const char* conf = "./config.txt";
#else
    const char* conf = "/etc/relaylive/config.txt";
#endif
    if (!Settings::loadFromProfile(conf))
        Log::error("Settings::loadFromProfile failed");
    else
        Log::debug("Settings::loadFromProfile ok");

    //根据cpu数量设置libuv线程池的线程数量
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
        uv_os_setenv("UV_THREADPOOL_SIZE", szThreadNum);
    }
    uv_free_cpu_info(cpu_infos, count);

    IPC::Init();

    /** 创建一个http服务器 */
    Server::Init(port);

    Log::debug("control sever @%d start success\r\n", port);

    Sleep(INFINITE);
    return 0;
}