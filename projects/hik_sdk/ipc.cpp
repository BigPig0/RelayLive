#include "util.h"
#include "utilc.h"
#include "easylog.h"
#include "ipc.h"
#include "uvipc.h"
#include <map>
#include <string>

using namespace std;
using namespace util;

namespace IPC {
    uv_ipc_handle_t* h = NULL;

    void on_ipc_recv(uv_ipc_handle_t* h, void* user, char* name, char* msg, char* data, int len) {
    }

    bool Init(int port) {
        /** 进程间通信 */
        char name[20]={0};
        sprintf(name, "hiksdk%d", port);
        string ipc_name = Settings::getValue("IPC","name","ipcsvr");
        int ret = uv_ipc_client(&h, (char*)ipc_name.c_str(), NULL, name, on_ipc_recv, NULL);
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
        uv_ipc_send(h, "hiksdkctrlsvr", "clients", (char*)info.c_str(), info.size());
    }
}