// sever.cpp : �������̨Ӧ�ó������ڵ㡣
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
    if ( CTRL_C_EVENT == type           //�û�����Ctrl+C,�رճ���
        || CTRL_BREAK_EVENT == type     //�û�����CTRL+BREAK
        || CTRL_LOGOFF_EVENT == type    //�û��˳�ʱ(ע��)
        || CTRL_SHUTDOWN_EVENT == type  //��ϵͳ���ر�ʱ
        || CTRL_CLOSE_EVENT == type)    //����ͼ�رտ���̨����
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
    /** ���ÿ���̨��Ϣ�ص� */
#ifdef WINDOWS_IMPL
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlCHandler, TRUE);
#endif

    /** ������·�����õ���������λ�� */
    setworkpath2ex();

    /** ������־�ļ� */
    char path[MAX_PATH];
#ifdef WINDOWS_IMPL
    sprintf(path, "./log/ipc_server/log.txt");
#else
    sprintf(path, "/var/log/relaylive/ipc_server/log.txt");
#endif
    Log::open(Log::Print::both, uvLogPlus::Level::Debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    /** ���������ļ� */
#ifdef WINDOWS_IMPL
    const char* conf = "./config.txt";
#else
    const char* conf = "/etc/relaylive/config.txt";
#endif
    if (!Settings::loadFromProfile(conf))
        Log::error("Settings::loadFromProfile failed");
    else
        Log::debug("Settings::loadFromProfile ok");

    //����IPCͨѶ����
    uv_ipc_handle_t* h = NULL;
    string ipc_name = Settings::getValue("IPC","name","ipcsvr");
    int ret = uv_ipc_server(&h, (char*)ipc_name.c_str(), NULL);
    if(ret < 0) {
        printf("ipc server err: %s\n", uv_ipc_strerr(ret));
    }

    //���������ػ�
#ifdef WINDOWS_IMPL    
    pm = CProcessMgr::Creat();
    pm->AddTasks("pm.json");
    pm->Start();
#endif

    sleep(INFINITE);
    return 0;
}