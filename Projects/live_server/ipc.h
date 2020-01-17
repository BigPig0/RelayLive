#pragma once
#include <string>

namespace IPC {
    struct PlayRequest {
        uint32_t    id;
        std::string code;
        int         ret;
        std::string info;
        bool        finish;
        uint32_t    port;
    };

    bool Init(int port);
    void Cleanup();
    void SendClients(std::string info);
    PlayRequest RealPlay(std::string code);
};