// sever.cpp : 定义控制台应用程序的入口点。
//
#include "uv.h"
#include "util.h"
#include "utilc.h"
#include "easylog.h"
#include "worker.h"
#include "server.h"
#include "ipc.h"

using namespace util;

int main(int argc, char* argv[])
{
    if(argc != 2)
        return -1;
    int port = atoi(argv[1]);

    /** 将工作路径设置到程序所在位置 */
    setworkpath2ex();

    /** Dump设置 */
    char dmpname[20]={0};
    sprintf(dmpname, "relay_server_%d.dmp", port);
    CMiniDump dump(dmpname);

    /** 创建日志文件 */
    char path[MAX_PATH]={0};
    sprintf(path, "./log/relay_server_%d.txt", port);
    Log::open(Log::Print::both, uvLogPlus::Level::Debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    /** 加载配置文件 */
    if (!Settings::loadFromProfile("./config.txt"))
        Log::error("Settings::loadFromProfile failed");
    else
        Log::debug("Settings::loadFromProfile ok");

    /** 进程通信初始化 */
    IPC::Init("relay", port);

    /** 视频处理初始化 */
    Worker::Init(NULL, NULL, false);

    /** 创建一个http服务器 */
    Server::Init(port);

    Log::debug("relay sever @%d start success\r\n", port);

    Sleep(INFINITE);
    return 0;
}