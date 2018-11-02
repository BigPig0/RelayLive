#pragma once

namespace HttpWsServer
{
    struct pss_other {
        char path[128];
        string* html;
    };
    extern int callback_other_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

    struct pss_device {
        char path[128];
		string* json;
    };
    extern int callback_device_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
};

