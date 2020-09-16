#pragma once
#include <string>

namespace IPC {
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
};