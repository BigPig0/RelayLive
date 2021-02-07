// sever.cpp : �������̨Ӧ�ó������ڵ㡣
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
    int port = atoi(argv[1]);

    /** Dump���� */
    char dmpname[20]={0};
    sprintf(dmpname, "capture_server_%d.dmp", port);
    CMiniDump dump(dmpname);

    /** ������־�ļ� */
    char path[MAX_PATH];
    sprintf(path, ".\\log\\capture_server_%d.txt", port);
    Log::open(Log::Print::both, uvLogPlus::Level::Debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    /** ���������ļ� */
    if (!Settings::loadFromProfile(".\\config.txt"))
        Log::error("Settings::loadFromProfile failed");
    else
        Log::debug("Settings::loadFromProfile ok");

    //����cpu��������libuv�̳߳ص��߳�����
    uv_cpu_info_t* cpu_infos;
    int count;
    int err = uv_cpu_info(&cpu_infos, &count);
    if (err) {
        Log::warning("fail get cpu info: %s",uv_strerror(err));
    } else {
        char szThreadNum[10] = {0};
        sprintf(szThreadNum, "%d", count*2+1);
        Log::debug("thread pool size is %s", szThreadNum);
        //���û���������ֵ
        uv_os_setenv("UV_THREADPOOL_SIZE", szThreadNum);
    }
    uv_free_cpu_info(cpu_infos, count);

    /** ����һ��http������ */
    Server::Init(port);
    Log::debug("capture sever @%d start success\r\n", port);

    /** ������ʷͼƬ */
    CleanStart();

    Sleep(INFINITE);
    return 0;
}