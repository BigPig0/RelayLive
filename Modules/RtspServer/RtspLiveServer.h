#pragma once
#include "libRtsp.h"

namespace RtspServer
{
    class CRtspWorker;
    
    /** per session structure */
    struct pss_rtsp_client {
        pss_rtsp_client       *pss_next;
        string                path;              //播放端请求地址
        string                clientName;        //播放端的名称
        string                clientIP;          //播放端的ip
        string                code;              //设备编码
        string                strErrInfo;        //不能播放时的错误信息
        struct lws_ring       *ring;             //接收数据缓冲区
        uint32_t              tail;              //ringbuff中的位置
        bool                  culled;
        CRtspWorker*          m_pWorker;
        CClient*              rtspClient;
        bool                  playing;           //是否在播放
    };

    extern int callback_live_rtsp(CClient *client, rtsp_method reason, void *user, void *in, size_t len);

}