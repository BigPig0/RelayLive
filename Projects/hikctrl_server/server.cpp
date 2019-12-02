#include "util.h"
#include "libwebsockets.h"
#include "web.h"

namespace Server
{
	uv_loop_t *g_uv_loop = NULL;

    static struct lws_context_creation_info info;  //libwebsockets配置信息
    static struct lws_context *context;            //libwebsockets句柄

    // http地址匹配
    //static struct lws_http_mount mount_other;  //其他
    static struct lws_http_mount mount_device; //查看设备信息、控制
    //static struct lws_http_mount mount_web;    //站点静态文件

    // 服务器配置
    //static std::string mount_web_origin;  //站点本地位置
    //static std::string mount_web_def;     //默认文件
    //static int http_port = 80;            //HTTP服务端口

    static struct lws_protocols protocols[] = {
        { "http",   callback_other_http,  sizeof(pss_other),   0 },
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
        memset(&mount_device, 0, sizeof(mount_device));
        mount_device.mountpoint = "/device";
        mount_device.mountpoint_len = 7;
        mount_device.origin_protocol = LWSMPRO_CALLBACK;
        mount_device.protocol = "device";
        //mount_device.mount_next = &mount_other;

        //创建libwebsockets环境
        memset(&info, 0, sizeof info);
        info.pcontext = &context;
        info.options = LWS_SERVER_OPTION_LIBUV | LWS_SERVER_OPTION_EXPLICIT_VHOSTS;
        info.foreign_loops = (void**)&g_uv_loop;
		info.timeout_secs = 0x1fffffff;
		info.timeout_secs_ah_idle = 0x1fffffff;
        context = lws_create_context(&info);

        //创建http服务器
        info.port = port;
        info.protocols = protocols;
        info.mounts = &mount_device;
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