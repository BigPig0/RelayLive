#pragma once

struct lws;
struct lws_ring;

namespace Server
{
    class CLiveWorker;

    /** per session structure */
    struct pss_live {
        struct lws           *wsi;            // http/ws ����
        bool                  isWs;           // �Ƿ�Ϊwebsocket
        int                   error_code;     // ʧ��ʱ�Ĵ�����
        bool                  send_header;    // Ӧ���Ƿ�д��header
        CLiveWorker          *pWorker;        // worker����
    };

    extern int callback_live(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
    
};