#pragma once
#include "libRtsp.h"
#include "ring_buff.h"

namespace RtspServer
{
    class CRtspWorker;
    
    /** per session structure */
    typedef struct _pss_rtsp_client_ {
        struct _pss_rtsp_client_  *pss_next;
        char                path[128];              //播放端请求地址
        char                clientName[50];        //播放端的名称
        char                clientIP[50];          //播放端的ip
        char                code[50];              //设备编码
        char                strErrInfo[128];        //不能播放时的错误信息
        ring_buff_t*        ring;             //接收数据缓冲区
        uint32_t              tail;              //ringbuff中的位置
        bool                  culled;
        CRtspWorker*          m_pWorker;
        CClient*              rtspClient;
        bool                  playing;           //是否在播放
    } pss_rtsp_client;

    extern int callback_live_rtsp(CClient *client, rtsp_method reason, void *user);

}