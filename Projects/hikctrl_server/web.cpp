#include "util.h"
#include "libwebsockets.h"
#include "web.h"
#include "hiksdk.h"
#include <string>
#include <set>

namespace Server
{
    int callback_device_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
    {
        struct pss_device *pss = (struct pss_device *)user;

        switch (reason) {
        case LWS_CALLBACK_HTTP:
            {
                uint8_t buf[LWS_PRE + 2048], 
                    *start = &buf[LWS_PRE], 
                    *p = start,
                    *end = &buf[sizeof(buf) - LWS_PRE - 1];
                
				int hlen = lws_hdr_total_length(wsi, WSI_TOKEN_GET_URI);
				if (hlen && hlen < 128) {
					lws_hdr_copy(wsi, pss->path, MAX_PATH, WSI_TOKEN_GET_URI);
					buf[127] = '\0';
				}

				pss->wsi = wsi;
                Log::debug("new http-web request: %s", pss->path);

                pss->response_body = new string;
				if(!strcmp(pss->path, "/device/clients")) {
					clients_request.add_req(pss);
				} else if(!strcmp(pss->path, "/device/devlist")) {
                    devlist_request.add_req(pss);
				} else if(!strncmp(pss->path, "/device/control", 8)) {
					// 设备ID
					char szDev[30]={0};
					sscanf(pss->path, "/device/control/%s", szDev);

					// 参数
					int ud=0, lr=0, io=0;
					char buf[20];
					int n = 0;
					while (lws_hdr_copy_fragment(wsi, buf, sizeof(buf), WSI_TOKEN_HTTP_URI_ARGS, n) > 0) {
						char arg[10]={0};
						int argv = 0;
						sscanf(buf, "%[^=]=%d", arg, &argv);
						if(!strcmp(arg, "ud")) {
							ud = argv;
						} else if(!strcmp(arg, "lr")) {
							lr = argv;
						} else if(!strcmp(arg, "io")) {
							io = argv;
						}
						n++;
					}
					HikPlat::DeviceControl(szDev, io, ud, lr);

					*pss->response_body = "ok";
					lws_add_http_common_headers(wsi, HTTP_STATUS_OK, "text/html", pss->response_body->size(), &p, end);
                    lws_add_http_header_by_name(wsi, (const uint8_t *)"Access-Control-Allow-Origin", (const uint8_t *)"*", 1, &p, end);
					if (lws_finalize_write_http_header(wsi, start, &p, end))
						return 1;

					lws_callback_on_writable(wsi);
				}

                return 0;
            }
        case LWS_CALLBACK_HTTP_WRITEABLE:
            {
                if (!pss)
                    break;

                int len = pss->response_body->size();
                int wlen = lws_write(wsi, (uint8_t *)pss->response_body->c_str(), len, LWS_WRITE_HTTP_FINAL);
                SAFE_DELETE(pss->response_body);
                if (wlen != len)
                    return 1;

                if (lws_http_transaction_completed(wsi))
                    return -1;


                return 0;
            }
        case LWS_CALLBACK_CLOSED_HTTP:
            {
                asyncRequest::remove_all(pss);
                break;
            }
        default:
            break;
        }
        //return 1;
        return lws_callback_http_dummy(wsi, reason, user, in, len);
    }
};
