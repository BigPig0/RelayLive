#include "stdafx.h"
#include "libwebsockets.h"
#include "pugixml.hpp"
#include "HttpWebServer.h"
#include "HttpLiveServer.h"
#include "HttpWorker.h"
#include "LiveClient.h"

namespace HttpWsServer
{
	uv_loop_t *g_uv_loop = NULL;
    static struct lws_context_creation_info info;  //libwebsockets配置信息
    static struct lws_context *context;            //libwebsockets句柄
    // 服务器配置
    static struct lws_http_mount mount_other;  //其他
    static struct lws_http_mount mount_device; //查看设备信息
    static struct lws_http_mount mount_live;   //直播
    static struct lws_http_mount mount_web;    //站点静态文件
    static std::string mount_web_origin("./home");  //站点本地位置
    static std::string mount_web_def("index.html"); //默认文件

    static struct lws_protocols protocols[] = {
        { "http",  callback_other_http,   sizeof(pss_other),   0 },
        { "live",   callback_live_http,   sizeof(pss_http_ws_live),    0 },
        { "device", callback_device_http, sizeof(pss_device),  0 },
        { "wslive", callback_live_ws,     sizeof(pss_http_ws_live), 0 },
        { NULL, NULL, 0, 0 } 
    };

    static struct lws_protocols wsprotocols[] = {
        { "websocket", callback_live_ws,  sizeof(pss_http_ws_live), 0 },
        { NULL, NULL, 0, 0 } 
    };

    static void serverInit()
    {
        memset(&mount_other, 0, sizeof(mount_other));
        mount_other.mountpoint = "/";
        mount_other.mountpoint_len = 1;
        mount_other.origin_protocol = LWSMPRO_CALLBACK;
        mount_other.protocol = "other";

        memset(&mount_device, 0, sizeof(mount_device));
        mount_device.mountpoint = "/device";
        mount_device.mountpoint_len = 7;
        mount_device.origin_protocol = LWSMPRO_CALLBACK;
        mount_device.protocol = "device";
        //mount_device.mount_next = &mount_other;

        memset(&mount_live, 0, sizeof(mount_live));
        mount_live.mountpoint = "/live";
        mount_live.mountpoint_len = 5;
        mount_live.origin_protocol = LWSMPRO_CALLBACK;
        mount_live.protocol = "live";
        mount_live.mount_next = &mount_device;

        string home_value = Settings::getValue("HttpServer","RootPath");
        if( !home_value.empty() ) mount_web_origin = home_value;
        string default_value = Settings::getValue("HttpServer","DefaultFile","index.html");
        if( !default_value.empty()) mount_web_def = default_value;
        bool bDirVisible = Settings::getValue("HttpServer","DirVisible")=="yes"?true:false;
        if(bDirVisible) mount_web_def = "hls.html";
        static struct lws_protocol_vhost_options mime_txt = {NULL, NULL, "txt", "text/plain"};
        static struct lws_protocol_vhost_options mime_swf = {&mime_txt, NULL, "swf", "application/x-shockwave-flash"};
        static struct lws_protocol_vhost_options mime_flv = {&mime_swf, NULL, "flv", "video/x-flv"};
        memset(&mount_web, 0, sizeof(mount_web));
        mount_web.mountpoint = "/";
        mount_web.mountpoint_len = 1;
        mount_web.origin_protocol = LWSMPRO_FILE;
        mount_web.origin = mount_web_origin.c_str();
        mount_web.def = mount_web_def.c_str();
        mount_web.extra_mimetypes = &mime_flv;
        mount_web.mount_next = &mount_live;
    }

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

        serverInit();

        LiveClient::SetCallBack(live_client_cb);

        //创建libwebsockets环境
        memset(&info, 0, sizeof info);
        info.pcontext = &context;
        info.options = LWS_SERVER_OPTION_LIBUV | LWS_SERVER_OPTION_EXPLICIT_VHOSTS;
        info.foreign_loops = (void**)&g_uv_loop;
		info.timeout_secs = 0x1fffffff;
		info.timeout_secs_ah_idle = 0x1fffffff;
        context = lws_create_context(&info);

        //创建http服务器
        info.port = 80;
        info.protocols = protocols;
        info.mounts = &mount_web;
        info.vhost_name = "gb28181 relay server";

        //读取配置文件
        string port_value = Settings::getValue("HttpServer","Port");
        if( !port_value.empty() ) info.port = stoi(port_value);

        if (!lws_create_vhost(context, &info)) {
            Log::error("Failed to create http vhost\n");
            return -1;
        }

        //创建webSocket服务器
        info.port = 8000;
        info.protocols = wsprotocols;
        info.mounts = NULL;
        info.vhost_name = "gb28181 relay server websocket";

        //读取配置文件
        port_value = Settings::getValue("HttpServer","wsPort");
        if( !port_value.empty() ) info.port = stoi(port_value);
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