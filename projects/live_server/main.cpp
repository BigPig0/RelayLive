// sever.cpp : 定义控制台应用程序的入口点。
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

    // 通知sip server创建播放请求实例，并获取本地udp端口
    IPCEX::PlayRequest *req = IPCEX::CreateReal(worker->m_pParam->strCode);
    rtp->port = req->port;

    // 创建RTP解码实例，并开始监听本地udp端口
    rtp->playHandle = RtpDecode::Creat(worker, req->port);

    //通知sip server发送请求，并获取应答信息
    IPCEX::RealPlay(req);
    if(req->ret != 0) {
        Log::error("play %s failed: %s", worker->m_pParam->strCode.c_str(), req->info.c_str());
        IPCEX:DestoryRequest(req);
        return false;
    }

    //根据应答信息开始解析rtp
    RtpDecode::Play(rtp->playHandle, req->info);

    IPCEX::DestoryRequest(req);
    return true;
}

static bool stop(CLiveWorker *worker) {
    RtpDecode::RtpData *rtp = (RtpDecode::RtpData*)worker->m_pPlay;

    /* 关闭sip播放 */
    IPCEX::Stop(rtp->port);
    //关闭rtp
    RtpDecode::Stop(rtp->playHandle);

    delete rtp;
    return true;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
        return -1;
    int port = atoi(argv[1]);

    /** 将工作路径设置到程序所在位置 */
    setworkpath2ex();

    /** Dump设置 */
    //char dmpname[20]={0};
    //sprintf(dmpname, "live_server_%d.dmp", port);
    //CMiniDump dump(dmpname);

    /** 创建日志文件 */
    char path[MAX_PATH];
    sprintf(path, "./log/live_server_%d.txt", port);
    Log::open(Log::Print::both, uvLogPlus::Level::Debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    /** 加载配置文件 */
    if (!Settings::loadFromProfile("./config.txt"))
        Log::error("Settings::loadFromProfile failed");
    else
        Log::debug("Settings::loadFromProfile ok");

    /** 进程通信初始化 */
    IPCEX::Init(port);

    /** 视频处理初始化 */
    Worker::Init(play, stop, false);

    /** 创建一个http服务器 */
    Server::Init(port);

    Log::debug("live sever @%d start success\r\n", port);

    Sleep(INFINITE);
    return 0;
}