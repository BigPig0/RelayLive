// sever.cpp : 定义控制台应用程序的入口点。
//
#include "uv.h"
#include "util.h"
#include "utilc.h"
#include "easylog.h"
#include "hiksdk.h"
#include "server.h"
#include "ipc.h"

using namespace util;

static bool play(CLiveWorker *worker) {
    return HikSdk::Play(worker) >= 0;
}

static bool stop(CLiveWorker *worker) {
    HikSdk::Stop(worker);
    return true;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
        return -1;
    int port = atoi(argv[1]);

    /** 将工作路径设置到程序所在位置 */
    setworkpath2ex();

    /** Dump设置 */
    //char dmpname[20]={0};
    //sprintf(dmpname, "hik_sdk_%d.dmp", port);
    //CMiniDump dump(dmpname);

    /** 创建日志文件 */
    char path[MAX_PATH]={0};
#ifdef WINDOWS_IMPL
    sprintf(path, "./log/hik_sdk_%d/log.txt", port);
#else
    sprintf(path, "/var/log/relaylive/hik_sdk_%d/log.txt", port);
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

    /** 海康sdk初始化 */
    HikSdk::Init();

    /** 进程通信初始化 */
    IPC::Init("hiksdk", port);

    /** 视频处理初始化 */
    Worker::Init(play, stop, false);

    /** 创建一个http服务器 */
    Server::Init(port);

    Log::debug("hik sdk @%d start success\r\n", port);

    Sleep(INFINITE);
    return 0;
}