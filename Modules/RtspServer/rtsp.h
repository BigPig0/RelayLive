#pragma once
#include "uv.h"

enum devType {
    DEV_HIK = 0,
    DEV_DAHUA
};

struct RTSP_REQUEST
{
    string     ip;
    uint32_t   port;
    string     user_name;
    string     password;
    uint32_t   channel;     //通道号，1、2...等
    uint32_t   stream;      //码流 0:主码流 1:子码流
    uint32_t   rtp_port;
    devType    dev_type;    //设备类型
};

extern int set_uv(uv_loop_t* uv);

extern void stop_uv();

/**
 * 发起请求
 */
typedef void (*play_cb)(int status);
extern int rtsp_play(RTSP_REQUEST config);

/**
 * 关闭rtsp
 */
extern int rtsp_stop(string ip);