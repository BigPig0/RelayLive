#include "util.h"
#include "libwebsockets.h"
#include "HttpLiveServer.h"

namespace Server
{
	uv_loop_t *g_uv_loop = NULL;

    static struct lws_context_creation_info info;  //libwebsockets配置信息
    static struct lws_context *context;            //libwebsockets句柄

    // http地址匹配
    static struct lws_http_mount mount_live;   //直播

    // 服务器配置
    static std::string mount_web_origin;  //站点本地位置
    static std::string mount_web_def; //默认文件
    static int http_port = 80; //HTTP服务端口
    static int ws_port = 8000; //web socket服务端口

    static struct lws_protocols protocols[] = {
        { "live",   callback_live_http,   sizeof(pss_http_ws_live), 0 },
        { "wslive", callback_live_ws,     sizeof(pss_http_ws_live), 0 },
        { NULL, NULL, 0, 0 } 
    };

    static struct lws_protocols wsprotocols[] = {
        { "websocket", callback_live_ws,  sizeof(pss_http_ws_live), 0 },
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

    int Init(void* uv)
    {
		g_uv_loop = (uv_loop_t *)uv;

        //设置日志
        int level = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;
        lws_set_log_level(level, userLog);

        mount_web_origin = Settings::getValue("HttpServer","RootPath", "./home");
        mount_web_def    = Settings::getValue("HttpServer","DefaultFile","index.html");
        bool bDirVisible = Settings::getValue("HttpServer","DirVisible")=="yes"?true:false;
        if(bDirVisible)
            mount_web_def = "This is a not exist file.html";
        http_port        = Settings::getValue("HttpServer","Port", 80);
        ws_port          = Settings::getValue("HttpServer","wsPort", 8000);

        // 直播请求
        memset(&mount_live, 0, sizeof(mount_live));
        mount_live.mountpoint = "/live";
        mount_live.mountpoint_len = 5;
        mount_live.origin_protocol = LWSMPRO_CALLBACK;
        mount_live.protocol = "live";

        //创建libwebsockets环境
        memset(&info, 0, sizeof info);
        info.pcontext = &context;
        info.options = LWS_SERVER_OPTION_LIBUV | LWS_SERVER_OPTION_EXPLICIT_VHOSTS;
        info.foreign_loops = (void**)&g_uv_loop;
		info.timeout_secs = 0x1fffffff;
		info.timeout_secs_ah_idle = 0x1fffffff;
        context = lws_create_context(&info);

        //创建http服务器
        info.port = http_port;
        info.protocols = protocols;
        info.mounts = &mount_live;
        info.vhost_name = "gb28181 relay server";
        if (!lws_create_vhost(context, &info)) {
            Log::error("Failed to create http vhost\n");
            return -1;
        }

        //创建webSocket服务器
        info.port = ws_port;
        info.protocols = wsprotocols;
        info.mounts = NULL;
        info.vhost_name = "gb28181 relay server websocket";
        if (!lws_create_vhost(context, &info)) {
            Log::error("Failed to create websocket vhost\n");
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