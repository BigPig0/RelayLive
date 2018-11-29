#include "stdafx.h"
#include "rtsp.h"
#include "rtsp_hik.h"

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

int set_uv(uv_loop_t* uv) {
    if(uv){
        _uv_loop_ = uv;
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
}

void stop_uv() {
    _inner_uv_ = false;
}

int rtsp_play(RTSP_REQUEST config, play_cb cb) {
    if (config.dev_type == DEV_HIK)
    {
        CRtspClient* c = new CRtspHik(config);
        c->play(cb);
        _map_rtsp_.insert(make_pair(config.ip, c));
    }
}

int rtsp_stop(string ip) {
    auto find = _map_rtsp_.find(ip);
    if (find != _map_rtsp_.end())
    {
        _map_rtsp_.erase(find);
    }
    return 0;
}