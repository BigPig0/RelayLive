#pragma once
#include <string>
#include <stdint.h>

class ILiveSession {
public:
    virtual void AsyncSend() = 0;
};

/**
 * uri中解析出参数
 */
struct RequestParam {
    std::string           strHost;        //< 海康设备域名或ip
    uint32_t              nPort;          //< 海康设备端口
    std::string           strUsr;         //< 海康设备用户名
    std::string           strPwd;         //< 海康设备密码
    uint32_t              nChannel;       //< 播放通道号

    std::string           strType;              // 媒体类型，默认为flv
    uint32_t              nWidth;               // 视频宽度，默认为0，不缩放视频
    uint32_t              nHeight;              // 视频高度，默认为0，不缩放视频
    uint32_t              nProbSize;            // 探测PS流的大小，默认为25600
    uint32_t              nProbTime;            // 探测PS流的时间，默认为1秒
    uint32_t              nInCatch;             // 输入缓存大小 默认1024*16
    uint32_t              nOutCatch;            // 输出缓存大小 默认1024*16
    RequestParam();
};

namespace Server {

int Init(int port);

int Cleanup();
};