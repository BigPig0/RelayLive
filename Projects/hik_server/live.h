#pragma once

struct lws;
struct lws_ring;

namespace Server
{
    class CLiveWorker;

    enum MediaType {
        media_error = 0,
        media_flv,
        media_hls,
        media_h264,
        media_mp4,
        media_m3u8,
        media_ts
    };

    enum SessState {
        session_init = 0,
        session_play_request,
        session_playing,
        session_stoped
    };

    /** per session structure */
    struct pss_http_ws_live {
        struct lws           *wsi;            // http/ws 连接
        MediaType             media_type;     // 当前连接请求的媒体格式类型
        CLiveWorker          *pWorker;        // CFlvWorker对象
        int                   error_code;     // 失败时的错误码
        SessState             session_state;  // 会话状态
        bool                  send_header;    // 应答是否写入header
    };

    extern int callback_live_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
    
    extern int callback_live_ws(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
};