#pragma once

struct lws;
struct lws_ring;

namespace HttpWsServer
{
    class CLiveWorker;

    enum MediaType
    {
        media_error = 0,
        media_flv,
        media_hls,
        media_h264,
        media_mp4
    };

    /** per session structure */
    struct pss_http_ws_live {
        pss_http_ws_live      *pss_next;
        struct lws            *wsi;              //http/ws 连接
        MediaType             media_type;        //当前连接请求的媒体格式类型
        bool                  isWs;              //false是http请求，true是websocket
        bool                  m_bSendHead;       //当前连接已经发送的步骤
        char                  path[128];         //播放端请求地址
        char                  clientName[50];    //播放端的名称
        char                  clientIP[50];      //播放端的ip
        char                  strErrInfo[128];   //不能播放时的错误信息
        CLiveWorker           *m_pWorker;        //CFlvWorker对象
        struct lws_ring       *ring;             //接收数据缓冲区
        uint32_t              tail;              //ringbuff中的位置
        bool                  culled;
    };
    extern int callback_live_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
    extern int callback_live_ws(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
};