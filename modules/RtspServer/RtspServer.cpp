#include "stdafx.h"
#include "RtspServer.h"
#include "RtspLiveServer.h"

namespace RtspServer
{
    uv_loop_t *g_uv_loop = NULL;
    CRtspServer *g_rtsp = NULL;

	extern vector<int>     m_vecRtpPort;     //< RTP���ö˿ڣ�ʹ��ʱ����ȡ����ʹ�ý������·��롣rtp���ط��Ͷ˿�
    extern CriticalSection m_csRTP;          //< RTP�˿���

    int Init(void* uv)
    {
        g_uv_loop = (uv_loop_t *)uv;

        //��ȡ�����ļ�
        rtsp_options o;
        o.ip           = Settings::getValue("RtspServer","IP", "0.0.0.0");
        o.port         = Settings::getValue("RtspServer","Port", 524);
        o.rtp_port     = Settings::getValue("RtspServer","RtpPort", 10000);
        o.rtp_port_num = Settings::getValue("RtspServer","RtpPort", 1000);
        o.user_len     = sizeof(pss_rtsp_client);
        o.cb           = callback_live_rtsp;

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