#pragma once

struct lws;
struct lws_ring;

namespace Server
{
    class CLiveWorker;

    /** per session structure */
    struct pss_live {
        struct lws           *wsi;            // http/ws 连接
        bool                  isWs;           // 是否为websocket
        int                   error_code;     // 失败时的错误码
        bool                  send_header;    // 应答是否写入header
        CLiveWorker          *pWorker;        // worker对象
    };

    extern int callback_live(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
    
};