#include "stdafx.h"
#include "rtsp.h"
#include "rtsp_hik.h"
#include "uv.h"

/**
 * 保存建立的rtsp连接实力
 * key是ip，value是连接
 */
map<string, CRtspClient*> _map_rtsp_;

uv_loop_t* _uv_loop_;
bool _inner_uv_;

static void run_loop_thread(void* arg)
{
    while (_inner_uv_) {
        uv_run(_uv_loop_, UV_RUN_DEFAULT);
        Sleep(200);
    }
    uv_loop_close(_uv_loop_);
}

int set_uv(void* uv) {
    if(uv){
        _uv_loop_ = (uv_loop_t*)uv;
    } else {
        uv_thread_t tid;
        _uv_loop_ = uv_loop_new();
        _inner_uv_ = true;
        int ret = uv_thread_create(&tid, run_loop_thread, 0);
        if(ret < 0) {
            printf("uv thread creat failed: %s\n", uv_strerror(ret));
            uv_loop_close(_uv_loop_);
            return ret;
        }
    }
    return 0;
}

void stop_uv() {
    _inner_uv_ = false;
}

int rtsp_play(RTSP_REQUEST option, play_cb cb) {
    if (option.dev_type == DEV_HIK) {
        CRtspClient* c = new CRtspHik(option);
        c->play(cb);
		_map_rtsp_.insert(make_pair(option.ssid, c));
    }
    return 0;
}

int rtsp_stop(string ssid) {
    auto find = _map_rtsp_.find(ssid);
    if (find != _map_rtsp_.end()) {
        delete find->second;
        _map_rtsp_.erase(find);
    }
    return 0;
}

char* rtsp_errors[] = {
    "connect to server failed",
    "tcp receiving failed",
    "make ip4 address failed",
    "tcp sending failed",
    "rtsp option method failed",
    "rtsp describe method failed",
    "rtsp setup method failed",
    "rtsp play method failed",
    "rtsp teardown method failed",
};

char* rtsp_strerr(int status) {
    if(status == 0) {
        return "success";
    }
    if(status >= RTSP_ERR_MAX) {
        return "unknown error";
    }
    return rtsp_errors[status + 1000];
}