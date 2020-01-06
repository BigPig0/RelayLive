#pragma once
#include <string>

namespace IPC {
    bool Init();
    void Cleanup();
    std::string GetClientsJson();
};