#pragma once
#include <string>

namespace IPC {
    /**
     * 初始化IPC
     * @name ipc客户端类型，用来生成ipc客户端名字
     * @port 视频程序监听端口，用来生成ipc客户端名字
     */
    bool Init(char *type, int port);

    /** 清理IPC */
    void Cleanup();

    /**
     * 向ctrl server发送当前连接的请求信息
     */
    void SendClients(std::string info);

};