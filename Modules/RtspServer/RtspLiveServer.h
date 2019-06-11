/*!
 * \file RtspLiveServer.h
 * \date 2019/06/10 15:46
 *
 * \author wlla
 * Contact: user@company.com
 *
 * \brief 自定义回调处理
 *
 * TODO: 实现一个回调方法，所有业务相关的逻辑都在这个回调方法里面实现
 *
 * \note
*/

#pragma once
#include "RtspSocket.h"
#include "ring_buff.h"

namespace RtspServer
{
    class CRtspWorker;
    
    /** per session structure */
    typedef struct _pss_rtsp_client_ {
        struct _pss_rtsp_client_  *pss_next;
        char                path[128];             //播放端请求地址
        char                clientName[50];        //播放端的名称
        char                clientIP[50];          //播放端的ip
        char                code[50];              //设备编码
        uint32_t            channel;               //通道号，指示不通大小的码流
        char                strErrInfo[128];       //不能播放时的错误信息
        ring_buff_t*        ring;             //接收数据缓冲区
        uint32_t            tail;              //ringbuff中的位置
        bool                culled;
        CRtspWorker*        m_pWorker;
        CRtspSocket*        rtspClient;
        bool                playing;           //是否在播放
    } pss_rtsp_client;

    extern int callback_live_rtsp(CRtspSocket *client, RTSP_REASON reason, void *user);

}