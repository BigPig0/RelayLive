#include "util.h"
#include "ipc.h"
#include "uvIpc.h"

namespace IPC {
    uv_ipc_handle_t* h = NULL;

    void on_ipc_recv(uv_ipc_handle_t* h, void* user, char* name, char* msg, char* data, int len) {
    }

    bool Init(int port) {
        /** ���̼�ͨ�� */
        char name[20]={0};
        sprintf(name, "hiksvr%d", port);
        int ret = uv_ipc_client(&h, "ipcsvr", NULL, name, on_ipc_recv, NULL);
        if(ret < 0) {
            Log::error("ipc server err: %s", uv_ipc_strerr(ret));
            return false;
        }

        return true;
    }

    void Cleanup() {
        uv_ipc_close(h);
    }

    void SendClients(string info) {
        uv_ipc_send(h, "hikctrlsvr", "clients", (char*)info.c_str(), info.size());
    }
}