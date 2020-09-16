#include "util.h"
#include "ipc.h"
#include "uvIpc.h"
#include <map>
#include <sstream>

using namespace util;

namespace IPC {
    uv_ipc_handle_t* h = NULL;
    map<string, string>      _mapClients;
    CriticalSection          _csClients;


    void on_ipc_recv(uv_ipc_handle_t* h, void* user, char* name, char* msg, char* data, int len) {
        if(!strcmp(msg,"close")) {
            MutexLock lock(&_csClients);
            auto it = _mapClients.find(name);
            if(it != _mapClients.end()) {
                _mapClients.erase(it);
            }
        } else if(!strcmp(msg,"clients")) {
            MutexLock lock(&_csClients);
            if(_mapClients.count(name) > 0) {
                _mapClients[name] = string(data, len);
            } else {
                _mapClients.insert(make_pair(name, string(data, len)));
            }
        }
    }

    bool Init() {
        /** 进程间通信 */
        string ipc_name = Settings::getValue("IPC","name","ipcsvr");
        int ret = uv_ipc_client(&h, (char*)ipc_name.c_str(), NULL, "relayctrlsvr", on_ipc_recv, NULL);
        if(ret < 0) {
            Log::error("ipc server err: %s", uv_ipc_strerr(ret));
            return false;
        }

        return true;
    }

    void Cleanup() {
        uv_ipc_close(h);
    }

    std::string GetClientsJson() {
        MutexLock lock(&_csClients);
        stringstream ss;
        ss << "{\"root\":[";
        bool first = true;
        for(auto c:_mapClients){  
            if(!first) {
                ss << ",";
            } else {
                first = false;
			}
            ss << c.second;
        }
        ss << "]}";
        return ss.str();
    }
}