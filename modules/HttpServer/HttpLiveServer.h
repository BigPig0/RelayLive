#pragma once

struct lws;
struct lws_ring;

namespace HttpWsServer
{
    class CHttpWorker;

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
        struct lws           *wsi;            // http/ws ����
        MediaType             media_type;     // ��ǰ���������ý���ʽ����
        CHttpWorker          *pWorker;        // CFlvWorker����
        int                   error_code;     // ʧ��ʱ�Ĵ�����
        SessState             session_state;  // �Ự״̬
        bool                  send_header;    // Ӧ���Ƿ�д��header
    };

    extern int callback_live_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
    
    extern int callback_live_ws(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
};