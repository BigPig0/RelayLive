// sever.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "uv.h"
#include "util.h"
#include "utilc.h"
#include "easylog.h"
#include "server.h"
#include "hiksdk.h"
#include "ipc.h"

using namespace util;

int main(int argc, char* argv[])
{
    if(argc != 2)
        return -1;
    int port = atoi(argv[1]);

    /** ������·�����õ���������λ�� */
    setworkpath2ex();

    /** Dump���� */
    char dmpname[20]={0};
    sprintf(dmpname, "hik_sdk_%d.dmp", port);
    CMiniDump dump(dmpname);

    /** ������־�ļ� */
    char path[MAX_PATH]={0};
    sprintf(path, "./log/hik_sdk_%d.txt", port);
    Log::open(Log::Print::both, uvLogPlus::Level::Debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    /** ���������ļ� */
    if (!Settings::loadFromProfile("./config.txt"))
        Log::error("Settings::loadFromProfile failed");
    else
        Log::debug("Settings::loadFromProfile ok");

    /** ����sdk��ʼ�� */
    HikSdk::Init();

    /** ����ͨ�ų�ʼ�� */
    IPC::Init(port);

    /** ����һ��http������ */
    Server::Init(port);

    Log::debug("hik sdk @%d start success\r\n", port);

    Sleep(INFINITE);
    return 0;
}