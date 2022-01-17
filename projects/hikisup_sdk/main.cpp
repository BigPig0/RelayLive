// sever.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "uv.h"
#include "util.h"
#include "utilc.h"
#include "easylog.h"
#include "database.h"
#include "hiksdk.h"
#include "Task.h"
#include "server.h"
#include "ipc.h"

using namespace util;

uv_loop_t *g_uvLoop;

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

    /** ������·�����õ���������λ�� */
    setworkpath2ex();

    /** Dump���� */
    //char dmpname[20]={0};
    //sprintf(dmpname, "hik_sdk_%d.dmp", port);
    //CMiniDump dump(dmpname);

    /** ������־�ļ� */
    char path[MAX_PATH]={0};
#ifdef WINDOWS_IMPL
    sprintf(path, "./log/hikisup_sdk_%d/log.txt", port);
#else
    sprintf(path, "/var/log/relaylive/hikisup_sdk_%d/log.txt", port);
#endif
    Log::open(Log::Print::both, uvLogPlus::Level::Debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    /** ���������ļ� */
#ifdef WINDOWS_IMPL
    const char* conf = "./hikisup.conf";
#else
    const char* conf = "/etc/relaylive/hikisup.conf";
#endif
    if (!Settings::loadFromProfile(conf))
        Log::error("Settings::loadFromProfile failed");
    else
        Log::debug("Settings::loadFromProfile ok");

    g_uvLoop = uv_default_loop();

    //GPS�ϴ�����
    Task::Init(g_uvLoop);

    //���ݿ��ʼ��
    DbTsk::Init();

    /** ����sdk��ʼ�� */
    HikSdk::Init(g_uvLoop);

    /** ����ͨ�ų�ʼ�� */
    IPC::Init("hiksdk", port);

    /** ��Ƶ�����ʼ�� */
    Worker::Init(play, stop, false);

    /** ����һ��http������ */
    Server::Init(port);

    Log::debug("hik sdk @%d start success\r\n", port);

    uv_run(g_uvLoop, UV_RUN_DEFAULT);

    Sleep(INFINITE);
    return 0;
}