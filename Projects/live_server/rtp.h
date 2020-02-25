#pragma once
#include <string>

namespace RtpDecode {
    bool Init();
    void Cleanup();
    void* Play(void* user, std::string sdp, uint32_t port);
    void Stop(void* ret);
};
