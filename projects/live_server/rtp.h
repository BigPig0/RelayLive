#pragma once
#include <stdint.h>
#include <string>

namespace RtpDecode {
    struct RtpData {
        uint32_t port;
        void    *playHandle;
    };
    /**
     * 创建CRtpStream实例，并开始监听udp
     * @param user CLiveWorker实例
     * @param port 本地监听的udp端口
     */
    void* Creat(void* user, uint32_t port);

    /**
     * 将播放参数传给CRtpStream实例，并开始解析rtp
     * @param h CRtpStream实例
     * @param sdp 接收到的应答信息
     */
    void Play(void* h, std::string sdp);

    /**
     * 停止CRtpStream的接收，并删除实例
     * @param h CRtpStream实例
     */
    void Stop(void* h);

    void TestRtp(void* h, string remoteIP, uint32_t remotePort);
};
