#pragma once
#include <string>

namespace Server
{
    int Init(void* uv, int port);

    int Cleanup();
    
    struct pss_device {
        struct lws *wsi;              //http/ws Á¬½Ó
        char path[MAX_PATH];          //uri
        std::string* response_body;        //Ó¦´ðbody
    };
};