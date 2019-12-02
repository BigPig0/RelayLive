#pragma once

namespace Server
{
    int Init(void* uv, int port);

    int Cleanup();
};