// sever.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "utilc_api.h"
#include "util.h"
#include "easylog.h"
#include "uvIpc.h"
#include "pm.h"
#include <stdio.h>
#include <windows.h>

using namespace util;

CProcessMgr* pm = NULL;

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

int main()
{
    /** ���ÿ���̨��Ϣ�ص� */
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlCHandler, TRUE);

    /** ������־�ļ� */
    char path[MAX_PATH];
    sprintf_s(path, MAX_PATH, ".\\log\\ipc_server.txt");
    Log::open(Log::Print::both, uvLogPlus::Level::Debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    /** ���������ļ� */
    if (!Settings::loadFromProfile(".\\config.txt"))
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
    pm = CProcessMgr::Creat();
    pm->AddTasks("pm.json");
    pm->Start();

    sleep(INFINITE);
    return 0;
}