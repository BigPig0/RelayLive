#pragma once

namespace Server
{
    int Init(void* uv, int port);

    int Cleanup();

    class CLiveWorker;
    struct live_session {
        virtual void AsyncSend() = 0;

        bool                  isWs;           // 是否为websocket
        int                   error_code;     // 失败时的错误码
        bool                  send_header;    // 应答是否写入header
        CLiveWorker          *pWorker;        // worker对象
        live_session();
        virtual ~live_session();
    };
};