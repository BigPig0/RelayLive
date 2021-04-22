#include "util.h"
#include "utilc.h"
#include "easylog.h"
#include "ipcex.h"
#include "uvipc.h"
#include "ipc.h"
#include <map>
#include <string>

using namespace std;
using namespace util;

namespace IPCEX {
    uv_ipc_handle_t* h = NULL;

    
    static std::map<uint32_t, PlayRequest*> _PlayRequests;
    static CriticalSection _csPlayReqs;
    static uint32_t _ID = 0;

    void on_ipc_recv(uv_ipc_handle_t* h, void* user, char* name, char* msg, char* data, int len) {
        if (!strcmp(msg,"live_init_answer")) {
            //播放初始化回调 id=123&port=80&ret=0
            data[len] = 0;
            uint32_t id = 0;
            uint32_t port = 0;
            sscanf(data, "id=%d&port=%d",&id, &port);

            MutexLock lock(&_csPlayReqs);
            auto it = _PlayRequests.find(id);
            if(it != _PlayRequests.end()) {
                it->second->port = port;
                it->second->finish = true;
            }
        } else if (!strcmp(msg,"live_play_answer")) {
            //播放请求回调 id=123&port=80&ret=0&info=XXXX
            data[len] = 0;
            uint32_t id = 0;
            uint32_t port = 0;
            int  ret = 0;
            //char szInfo[250] = {0}; // 成功时sdp信息，失败时错误描述
            sscanf(data, "id=%d&port=%d&ret=%d&error=",&id, &port, &ret);

			string infoMsg(data, len);
			size_t pos = infoMsg.find("error=");
			if(pos != string::npos){
				infoMsg = infoMsg.substr(pos+6, infoMsg.size()-pos-6);
			}

            MutexLock lock(&_csPlayReqs);
            auto it = _PlayRequests.find(id);
            if(it != _PlayRequests.end()) {
                it->second->port = port;
                it->second->ret = ret;
                it->second->info = infoMsg;
                it->second->finish = true;
            }
        } 
    }

    bool Init(int port) {
        h = IPC::Init("livesvr", port, on_ipc_recv);
        return h!=NULL;
    }

    void Cleanup() {
        IPC::Cleanup();
    }

    void SendClients(string info) {
        IPC::SendClients(info);
    }

    PlayRequest* CreateReal(std::string code) {
        PlayRequest *req = new PlayRequest();
        req->code   = code;
        req->id     = _ID++;
        req->ret    = 0;
        req->finish = false;

        _csPlayReqs.lock();
        _PlayRequests.insert(make_pair(req->id, req));
		_csPlayReqs.unlock();

        std::string msg = "devcode=" + code + "&id=" + to_string(req->id);
        uv_ipc_send(h, "sipsvr", "live_init", msg.c_str(), msg.size());

        // 等待返回结果
        time_t send_time = time(NULL);
        while (!req->finish) {
            time_t now = time(NULL);
            if(difftime(now, send_time) > 30) {
                req->finish = true;
                req->ret = 0;
                req->info = "time out";
            }
        }

        //返回播放结果
        return req;
    }

    void RealPlay(PlayRequest* req) {
        req->ret    = 0;
        req->finish = false;

        time_t send_time = time(NULL);

        std::string msg = "port=" + to_string(req->port) + "&id=" + to_string(req->id);
        uv_ipc_send(h, "sipsvr", "live_play", msg.c_str(), msg.size());

        // 等待返回结果
        while (!req->finish) {
            time_t now = time(NULL);
            if(difftime(now, send_time) > 30) {
                req->finish = true;
                req->ret = 0;
                req->info = "time out";
            }
        }
    }

    void DestoryRequest(PlayRequest *req) {
        MutexLock lock(&_csPlayReqs);
        auto fit = _PlayRequests.find(req->id);
        if(fit != _PlayRequests.end()) {
            _PlayRequests.erase(fit);
        }
        delete req;
    }

	void Stop(uint32_t port) {
		std::string msg = to_string(port);
		uv_ipc_send(h, "sipsvr", "stop_play", msg.c_str(), msg.size());
	}
}