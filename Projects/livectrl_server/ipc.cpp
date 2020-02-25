#include "util.h"
#include "ipc.h"
#include "uvIpc.h"
#include "SipServer.h"
#include <map>
#include <sstream>

namespace IPC {
    uv_ipc_handle_t* h = NULL;
    map<string, string>      _mapClients;
    CriticalSection          _csClients;
    map<string, SipServer::DevInfo*> _mapDevs;
    CriticalSection          _csDevs;

    void on_ipc_recv(uv_ipc_handle_t* h, void* user, char* name, char* msg, char* data, int len) {
        if(!strcmp(msg,"close")) {
            // livesvr关闭
            MutexLock lock(&_csClients);
            auto it = _mapClients.find(name);
            if(it != _mapClients.end()) {
                _mapClients.erase(it);
            }
        } else if(!strcmp(msg,"clients")) {
            // livesvr发送的客户端信息
            MutexLock lock(&_csClients);
            if(_mapClients.count(name) > 0) {
                _mapClients[name] = string(data, len);
            } else {
                _mapClients.insert(make_pair(name, string(data, len)));
            }
        } else if(!strcmp(msg,"add_device")) {
            // sipsvr发送的设备信息
            SipServer::DevInfo *dev = new SipServer::DevInfo;
            SipServer::TransDevInfo(string(data, len), dev);
            MutexLock lock(&_csDevs);
            if(_mapDevs.count(dev->strDevID) > 0) {
                delete _mapDevs[dev->strDevID];
                _mapDevs[dev->strDevID] = dev;
            } else {
                _mapDevs.insert(make_pair(dev->strDevID, dev));
            }
        } else if(!strcmp(msg,"update_status")) {
            // sipsvr发送的设备状态更新信息 devid=XXX&status=XXX
            data[len] = 0;
            char devid[50] = {0};
            char status[10] = {0};
            scanf(data, "devid=[^&]&status=%s", devid, status);
            MutexLock lock(&_csDevs);
            if(_mapDevs.count(devid) > 0) {
                _mapDevs[devid]->strStatus = status;
            }
        } else if(!strcmp(msg,"update_pos")) {
            // sipsvr发送的设备gps更新信息
            data[len] = 0;
            char devid[50] = {0};
            char lat[20] = {0};
            char lon[20] = {0};
            scanf(data, "devid=[^&]&log=[^&]&lat=%s", devid, lon, lat);
            MutexLock lock(&_csDevs);
            if(_mapDevs.count(devid) > 0) {
                _mapDevs[devid]->strLatitude = lat;
                _mapDevs[devid]->strLongitude = lon;
            }
        }
    }

    bool Init() {
        /** 进程间通信 */
        int ret = uv_ipc_client(&h, "ipcsvr", NULL, "livectrlsvr", on_ipc_recv, NULL);
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

    std::string GetDevsJson() {
        MutexLock lock(&_csDevs);
        stringstream ss;
        ss << "{\"root\":[";
        bool first = true;
        for(auto c:_mapDevs){  
            if(!first) {
                ss << ",";
            } else {
				first = false;
			}
            ss << FormatDevInfo(c.second, true);
        }
        ss << "]}";
        return ss.str();
    }

    void AddDev(SipServer::DevInfo *dev){

    }

    void DevsFresh() {

    }

    void DevControl(string strDev, int nInOut, int nUpDown, int nLeftRight) {

    }
}