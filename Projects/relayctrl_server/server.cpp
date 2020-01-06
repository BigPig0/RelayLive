
#include "libwebsockets.h"
#include "ipc.h"
#include "server.h"
#include "util.h"

namespace Server
{
	uv_loop_t *g_uv_loop = NULL;

    static struct lws_context_creation_info info;  //libwebsockets配置信息
    static struct lws_context *context;            //libwebsockets句柄

    static int callback_device_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
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
                    *pss->response_body = IPC::GetClientsJson();
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
                if (lws_http_transaction_completed(wsi))
                    return -1;


                return 0;
            }
        case LWS_CALLBACK_CLOSED_HTTP:
            {
                if(pss)
                    SAFE_DELETE(pss->response_body);
                break;
            }
        default:
            break;
        }
        //return 1;
        return lws_callback_http_dummy(wsi, reason, user, in, len);
    }

    static struct lws_protocols protocols[] = {
        { "device", callback_device_http, sizeof(pss_device),  0 },
        { NULL, NULL, 0, 0 } 
    };

    //覆盖libwebsockets里的日志
    void userLog(int level, const char* line)
    {
        if(level & LLL_ERR)
            Log::error(line);
        else if(level & LLL_WARN)
            Log::warning(line);
        else if(level & LLL_NOTICE)
            Log::debug(line);
        else
            Log::debug(line);
    }

    int Init(void* uv, int port)
    {
		g_uv_loop = (uv_loop_t *)uv;

        //设置日志
        int level = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;
        lws_set_log_level(level, userLog);

        // 设备信息查看、控制请求
        //memset(&mount_device, 0, sizeof(mount_device));
        //mount_device.mountpoint = "/device";
        //mount_device.mountpoint_len = 7;
        //mount_device.origin_protocol = LWSMPRO_CALLBACK;
        //mount_device.protocol = "device";
        //mount_device.mount_next = &mount_other;

        //创建libwebsockets环境
        memset(&info, 0, sizeof info);
        info.pcontext = &context;
        info.options = LWS_SERVER_OPTION_LIBUV | LWS_SERVER_OPTION_EXPLICIT_VHOSTS;
        info.foreign_loops = (void**)&uv;
		info.timeout_secs = 0x1fffffff;
		info.timeout_secs_ah_idle = 0x1fffffff;
        context = lws_create_context(&info);

        //创建http服务器
        info.port = port;
        info.protocols = protocols;
        info.mounts = NULL;//&mount_device;
        info.vhost_name = "hik ctrl server";
        if (!lws_create_vhost(context, &info)) {
            Log::error("Failed to create http vhost\n");
            return -1;
        }

        return 0;
    }

    int Cleanup()
    {
        // 销毁libwebsockets
        lws_context_destroy(context);
        return 0;
    }

    
};