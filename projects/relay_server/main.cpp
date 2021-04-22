// sever.cpp : �������̨Ӧ�ó������ڵ㡣
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

    /** ������·�����õ���������λ�� */
    setworkpath2ex();

    /** Dump���� */
    char dmpname[20]={0};
    sprintf(dmpname, "relay_server_%d.dmp", port);
    CMiniDump dump(dmpname);

    /** ������־�ļ� */
    char path[MAX_PATH]={0};
    sprintf(path, "./log/relay_server_%d.txt", port);
    Log::open(Log::Print::both, uvLogPlus::Level::Debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    /** ���������ļ� */
    if (!Settings::loadFromProfile("./config.txt"))
        Log::error("Settings::loadFromProfile failed");
    else
        Log::debug("Settings::loadFromProfile ok");

    /** ����ͨ�ų�ʼ�� */
    IPC::Init("relay", port);

    /** ��Ƶ�����ʼ�� */
    Worker::Init(NULL, NULL, false);

    /** ����һ��http������ */
    Server::Init(port);

    Log::debug("relay sever @%d start success\r\n", port);

    Sleep(INFINITE);
    return 0;
}