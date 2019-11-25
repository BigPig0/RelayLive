// sever.cpp : 定义控制台应用程序的入口点。
//
#include "uvIpc.h"
#include "utilc_api.h"
#include "stdio.h"
#include "pm.h"
#include "Log.h"

CProcessMgr* pm = NULL;

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

int main()
{
    /** 设置控制台消息回调 */
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlCHandler, TRUE);

    /** 创建日志文件 */
    char path[MAX_PATH];
    sprintf_s(path, MAX_PATH, ".\\log\\ipc_server.txt");
    Log::open(Log::Print::both, Log::Level::debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    //启动IPC通讯服务
    uv_ipc_handle_t* h = NULL;
    int ret = uv_ipc_server(&h, "relay_live", NULL);
    if(ret < 0) {
        printf("ipc server err: %s\n", uv_ipc_strerr(ret));
    }

    //启动进程守护
    pm = CProcessMgr::Creat();
    pm->AddTasks("pm.json");
    pm->Start();

    sleep(INFINITE);
    return 0;
}