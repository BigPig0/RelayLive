// sever.cpp : 定义控制台应用程序的入口点。
//
#include "utilc.h"
#include "util.h"
#include "easylog.h"
#include "uvipc.h"
#include "pm.h"
#include <stdio.h>

#ifdef WINDOWS_IMPL
#include <windows.h>
#endif

using namespace util;

CProcessMgr* pm = NULL;

#ifdef WINDOWS_IMPL
BOOL CtrlCHandler(DWORD type)
{
    if ( CTRL_C_EVENT == type           //用户按下Ctrl+C,关闭程序。
        || CTRL_BREAK_EVENT == type     //用户按下CTRL+BREAK
        || CTRL_LOGOFF_EVENT == type    //用户退出时(注销)
        || CTRL_SHUTDOWN_EVENT == type  //当系统被关闭时
        || CTRL_CLOSE_EVENT == type)    //当试图关闭控制台程序
    {
        Log::debug("stop all child process and exit");
        if(pm) delete pm;
        return TRUE;
    }
    return FALSE;
}
#endif

int main()
{
    /** 设置控制台消息回调 */
#ifdef WINDOWS_IMPL
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlCHandler, TRUE);
#endif

    /** 将工作路径设置到程序所在位置 */
    setworkpath2ex();

    /** 创建日志文件 */
    char path[MAX_PATH];
#ifdef WINDOWS_IMPL
    sprintf(path, "./log/ipc_server/log.txt");
#else
    sprintf(path, "/var/log/relaylive/ipc_server/log.txt");
#endif
    Log::open(Log::Print::both, uvLogPlus::Level::Debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    // char *test = "创建日志文件";
    // string ss = "{";
    // ss += util::EncodeConvert::AtoUTF8(test);
    // ss += "}";
    // Log::debug(ss.c_str());

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

    //启动IPC通讯服务
    uv_ipc_handle_t* h = NULL;
    string ipc_name = Settings::getValue("IPC","name","ipcsvr");
    int ret = uv_ipc_server(&h, (char*)ipc_name.c_str(), NULL);
    if(ret < 0) {
        printf("ipc server err: %s\n", uv_ipc_strerr(ret));
    }

    //启动进程守护
#ifdef WINDOWS_IMPL    
    pm = CProcessMgr::Creat();
    pm->AddTasks("pm.json");
    pm->Start();
#endif

    sleep(INFINITE);
    return 0;
}