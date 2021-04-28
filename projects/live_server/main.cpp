// sever.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "uv.h"
#include "util.h"
#include "utilc.h"
#include "easylog.h"
#include "rtp.h"
#include "worker.h"
#include "server.h"
#include "ipcex.h"

using namespace util;

static bool play(CLiveWorker *worker) {
    RtpDecode::RtpData *rtp = new RtpDecode::RtpData;
    worker->m_pPlay = rtp;

    // ֪ͨsip server������������ʵ��������ȡ����udp�˿�
    IPCEX::PlayRequest *req = IPCEX::CreateReal(worker->m_pParam->strCode);
    rtp->port = req->port;

    // ����RTP����ʵ��������ʼ��������udp�˿�
    rtp->playHandle = RtpDecode::Creat(worker, req->port);

    //֪ͨsip server�������󣬲���ȡӦ����Ϣ
    IPCEX::RealPlay(req);
    if(req->ret != 0) {
        Log::error("play %s failed: %s", worker->m_pParam->strCode.c_str(), req->info.c_str());
        IPCEX:DestoryRequest(req);
        return false;
    }

    //����Ӧ����Ϣ��ʼ����rtp
    RtpDecode::Play(rtp->playHandle, req->info);

    IPCEX::DestoryRequest(req);
    return true;
}

static bool stop(CLiveWorker *worker) {
    RtpDecode::RtpData *rtp = (RtpDecode::RtpData*)worker->m_pPlay;

    /* �ر�sip���� */
    IPCEX::Stop(rtp->port);
    //�ر�rtp
    RtpDecode::Stop(rtp->playHandle);

    delete rtp;
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
    //sprintf(dmpname, "live_server_%d.dmp", port);
    //CMiniDump dump(dmpname);

    /** ������־�ļ� */
    char path[MAX_PATH];
    sprintf(path, "./log/live_server_%d.txt", port);
    Log::open(Log::Print::both, uvLogPlus::Level::Debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    /** ���������ļ� */
    if (!Settings::loadFromProfile("./config.txt"))
        Log::error("Settings::loadFromProfile failed");
    else
        Log::debug("Settings::loadFromProfile ok");

    /** ����ͨ�ų�ʼ�� */
    IPCEX::Init(port);

    /** ��Ƶ�����ʼ�� */
    Worker::Init(play, stop, false);

    /** ����һ��http������ */
    Server::Init(port);

    Log::debug("live sever @%d start success\r\n", port);

    Sleep(INFINITE);
    return 0;
}