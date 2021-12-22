// sever.cpp : 定义控制台应用程序的入口点。
//
#include "utilc.h"
#include "util.h"
#include "easylog.h"
#include "server.h"
#include "uv.h"

using namespace util;

extern void CleanStart();

int main(int argc, char* argv[])
{
    if(argc != 2)
        return -1;

    if(!strncasecmp(argv[1], "clean", 5)) {
        /** 创建日志文件 */
        char path[MAX_PATH];
#ifdef WINDOWS_IMPL
        sprintf(path, "./log/capture_server_clean/log.txt");
#else
        sprintf(path, "/var/log/relaylive/capture_server_clean/log.txt");
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

        /** 清理历史图片 */
        CleanStart();
        Sleep(INFINITE);
        return 0;
    }

    int port = atoi(argv[1]);

    /** 将工作路径设置到程序所在位置 */
    setworkpath2ex();

    /** Dump设置 */
    //char dmpname[20]={0};
    //sprintf(dmpname, "capture_server_%d.dmp", port);
    //CMiniDump dump(dmpname);

    /** 创建日志文件 */
    char path[MAX_PATH];
#ifdef WINDOWS_IMPL
    sprintf(path, "./log/capture_server_%d/log.txt", port);
#else
    sprintf(path, "/var/log/relaylive/capture_server_%d/log.txt", port);
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

    /** 创建一个http服务器 */
    Server::Init(port);
    Log::debug("capture sever @%d start success\r\n", port);

    Sleep(INFINITE);
    return 0;
}