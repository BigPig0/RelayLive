#include "stdafx.h"
#include "RtspServer.h"
#include "RtspLiveServer.h"

namespace RtspServer
{
    uv_loop_t *g_uv_loop = NULL;
    CRtspServer *g_rtsp = NULL;

    int Init(void* uv)
    {
        g_uv_loop = (uv_loop_t *)uv;

        //¶ÁÈ¡ÅäÖÃÎÄ¼þ
        rtsp_options o;
        o.ip = Settings::getValue("RtspServer","IP");
        o.port = Settings::getValue("RtspServer","Port", 524);
        o.rtp_port = Settings::getValue("RtspServer","RtpPort", 80000);
        o.rtp_port_num = Settings::getValue("RtspServer","RtpPort", 1000);

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