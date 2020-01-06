#pragma once
#include <string>

namespace IPC {
    bool Init(int port);
    void Cleanup();
    void SendClients(std::string info);
};