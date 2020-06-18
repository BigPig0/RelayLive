#include "util.h"
#include "ipc.h"
#include "uvIpc.h"
#include <map>
#include <sstream>

namespace IPC {
	struct DevRequest {
		uint32_t    id;    //����ID
		int         ret;   //��������0ʧ�ܣ� 1�ɹ�
		std::string info;  //Ӧ�����Ϣ
		bool        finish;//�Ƿ��յ�Ӧ��Ĭ��false���յ�Ӧ����Ϊtrue
	};

    uv_ipc_handle_t* h = NULL;
    map<string, string>      _mapClients;
    CriticalSection          _csClients;
	map<uint32_t, DevRequest*> _mapDevs;
    CriticalSection          _csDevs;
	static uint32_t          _ID = 0;

    void on_ipc_recv(uv_ipc_handle_t* h, void* user, char* name, char* msg, char* data, int len) {
        if(!strcmp(msg,"close")) {
            // livesvr�ر�
            MutexLock lock(&_csClients);
            auto it = _mapClients.find(name);
            if(it != _mapClients.end()) {
                _mapClients.erase(it);
            }
        } else if(!strcmp(msg,"clients")) {
            // livesvr���͵Ŀͻ�����Ϣ
            MutexLock lock(&_csClients);
            if(_mapClients.count(name) > 0) {
                _mapClients[name] = string(data, len);
            } else {
                _mapClients.insert(make_pair(name, string(data, len)));
            }
        }  else if(!strcmp(msg, "dev_get_answer")) {
			data[len] = 0;
			uint32_t id = 0;
			sscanf(data, "id=%d&",&id);
			char *pos = strstr(data, "json=");
			string devs;
			if(pos)
				devs = pos + 5;

			_csDevs.lock();
			auto fit = _mapDevs.find(id);
			if(fit != _mapDevs.end()) {
				fit->second->info = devs;
				fit->second->finish = true;
			}
			_csDevs.unlock();
		}
    }

    bool Init() {
        /** ���̼�ͨ�� */
        string ipc_name = Settings::getValue("IPC","name","ipcsvr");
        int ret = uv_ipc_client(&h, (char*)ipc_name.c_str(), NULL, "livectrlsvr", on_ipc_recv, NULL);
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
			if(c.second.empty())
				continue;
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

    std::string GetDevsJson() {
		DevRequest *req = new DevRequest;
		req->id = _ID++;
		req->ret = 0;
		req->finish = false;
		_csDevs.lock();
		_mapDevs.insert(make_pair(req->id, req));
		_csDevs.unlock();
		std::string str = "id=" + to_string(req->id);
		uv_ipc_send(h, "sipsvr", "dev_get", (char*)str.c_str(), str.size());
		
		// �ȴ����ؽ��
		time_t send_time = time(NULL);
		while (!req->finish) {
			time_t now = time(NULL);
			if(difftime(now, send_time) > 30) {
				req->finish = true;
				req->ret = 0;
				req->info = "time out";
			}
		}

		string ret = req->info;
		_csDevs.lock();
		auto fit = _mapDevs.find(req->id);
		if(fit != _mapDevs.end())
			_mapDevs.erase(fit);
		_csDevs.unlock();
		delete req;

        return ret;
    }

    void DevsFresh() {
		std::string str = "fresh";
		uv_ipc_send(h, "sipsvr", "dev_fresh", (char*)str.c_str(), str.size());
    }

    void DevControl(std::string strDev, int nInOut, int nUpDown, int nLeftRight) {
		std::string msg = "dev=" + strDev + "&io=" + to_string(nInOut) + "&ud=" + to_string(nUpDown) + "&lr=" + to_string(nLeftRight);
		uv_ipc_send(h, "sipsvr", "dev_ctrl", (char*)msg.c_str(), msg.size());
    }
}