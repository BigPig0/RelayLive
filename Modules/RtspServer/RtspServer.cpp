#include "stdafx.h"
#include "RtspServer.h"
#include "RtspLiveServer.h"

namespace RtspServer
{
    uv_loop_t *g_uv_loop = NULL;
    CRtspServer *g_rtsp = NULL;

	extern vector<int>     m_vecRtpPort;     //< RTP可用端口，使用时从中取出，使用结束重新放入。rtp本地发送端口
    extern CriticalSection m_csRTP;          //< RTP端口锁

    int Init(void* uv)
    {
        g_uv_loop = (uv_loop_t *)uv;

        //读取配置文件
        rtsp_options o;
        o.ip = Settings::getValue("RtspServer","IP");
        o.port = Settings::getValue("RtspServer","Port", 524);
        o.rtp_port = Settings::getValue("RtspServer","RtpPort", 80000);
        o.rtp_port_num = Settings::getValue("RtspServer","RtpPort", 1000);
        o.user_len = sizeof(pss_rtsp_client);
        o.cb = callback_live_rtsp;

        g_rtsp = new CRtspServer(o);
        g_rtsp->Init(g_uv_loop);
        return 0;
    }

    int Cleanup()
    {
        SAFE_DELETE(g_rtsp);
        return 0;
    }
}