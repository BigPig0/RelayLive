#include "stdafx.h"
#include "uvIpc.h"
#include "LiveIpc.h"
#include "LiveClient.h"

namespace LiveClient
{
    LIVECLIENT_CB ipc_cb = NULL;

namespace LiveIpc
{
    static uv_ipc_handle_t* h = NULL;

    struct ipc_play_task
    {
        int ipc_status;
        int ret;
        string ssid;
        string error;
    };
    static ipc_play_task _ipc_task;

    static string strfind(char* src, char* begin, char* end){
        char *p1, *p2;
        p1 = strstr(src, begin);
        if(!p1) return "";
        p1 += strlen(begin);
        p2 = strstr(p1, end);
        if(p2) return string(p1, p2-p1);
        else return string(p1);
    }

    static void on_ipc_recv(uv_ipc_handle_t* h, void* user, char* name, char* msg, char* data, int len) {
        if (!strcmp(msg,"live_play_answer")) {
            // ssid=123&ret=0&error=XXXX
            data[len] = 0;
            _ipc_task.ssid = strfind(data, "ssid=", "&");
            _ipc_task.ret = stoi(strfind(data, "ret=", "&"));
            _ipc_task.error = strfind(data, "error=", "&");
            _ipc_task.ipc_status = 0;
        } else if (!strcmp(msg,"dev_list_answer")) {
            string devjson(data, len);
            if(ipc_cb)
                ipc_cb("devlist", devjson);
        }
    }

    void Init(){
        int ret = uv_ipc_client(&h, "relay_live", NULL, "liveDest", on_ipc_recv, NULL);
        if(ret < 0) {
            printf("ipc server err: %s\n", uv_ipc_strerr(ret));
        }
    }

    int RealPlay(string dev_code, string rtp_ip, int rtp_port, string &sdp){
        // ssid=123&rtpip=1.1.1.1&rtpport=50000
        _ipc_task.ssid = dev_code;
        _ipc_task.ret = 0;
        _ipc_task.ipc_status = 1;
        _ipc_task.error = "";

        stringstream ss;
        ss << "ssid=" << dev_code << "&rtpip=" << rtp_ip << "&rtpport=" << rtp_port;
        int ret = uv_ipc_send(h, "liveSrc", "live_play", (char*)ss.str().c_str(), ss.str().size());
        if(ret) {
            Log::error("ipc send real play error");
            return ret;
        }

        time_t start_time = time(NULL);
        while (_ipc_task.ipc_status) {
            time_t now = time(NULL);
            if(difftime(now, start_time) > 2.0){
                //≥¨ ±2√Î
                _ipc_task.ret = -1;
                _ipc_task.error = "time out";
                break;
            }
            Sleep(100);
        }
        sdp = _ipc_task.error;
        return _ipc_task.ret;
    }

    int StopPlay(string dev_code){
        int ret = uv_ipc_send(h, "liveSrc", "stop_play", (char*)dev_code.c_str(), dev_code.size());
        if(ret) {
            Log::error("ipc send stop error");
            return ret;
        }
        return 0;
    }

    int GetDevList()
    {
        int ret = uv_ipc_send(h, "liveSrc", "devices_list", NULL, 0);
        if(ret) {
            Log::error("ipc send devices_list error");
            return ret;
        }
        return 0;
    }

    int QueryDirtionary()
    {
        int ret = uv_ipc_send(h, "liveSrc", "QueryDirtionary", NULL, 0);
        if(ret) {
            Log::error("ipc send devices_list error");
            return ret;
        }
        return 0;
    }

	int DeviceControl(string strDev, int nInOut, int nUpDown, int nLeftRight) {
		stringstream ss;
        ss << "dev=" << strDev << "&io=" << nInOut << "&ud=" << nUpDown << "&lr=" << nLeftRight;
		int ret = uv_ipc_send(h, "liveSrc", "DeviceControl", (char*)ss.str().c_str(), ss.str().size());
        if(ret) {
            Log::error("ipc send devices_list error");
            return ret;
        }
        return 0;
	}
}
}