#pragma once
#include "utilc_api.h"
#include <string>

namespace IPC {
    struct PlayRequest {
        uint32_t    id;    //请求ID
        std::string code;  //需要播放的设备编码
        int         ret;   //请求结果，0失败， 1成功
        std::string info;  //应答的信息
        bool        finish;//是否收到应答，默认false，收到应答置为true
        uint32_t    port;  //应答指定的本地接收端口
    };

    /**
     * 初始化IPC
     * @port liveserver监听服务端口，用来生成ipc客户端名字
     */
    bool Init(int port);

    /** 清理IPC */
    void Cleanup();

    /**
     * 向livectrl server发送当前连接的请求信息
     */
    void SendClients(std::string info);

    /**
     * 通知sip server准备发送请求，得到本次会话使用的本地端口
     * @param code 请求播放的设备编码
     */
    PlayRequest* CreateReal(std::string code);

    /**
     * 通知sip server发送请求
     */
    void RealPlay(PlayRequest* req);

    /**
     * 删除请求实例
     */
    void DestoryRequest(PlayRequest *req);

	void Stop(uint32_t port);
};