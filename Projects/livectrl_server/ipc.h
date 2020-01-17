#pragma once
#include <string>

namespace IPC {
    /**
     * 初始化IPC
     */
    bool Init();

    /**
     * 清理IPC
     */
    void Cleanup();

    /**
     * 获取客户端信息的json
     */
    std::string GetClientsJson();

    /**
     * 获取设备列表的json
     */
    std::string GetDevsJson();
};