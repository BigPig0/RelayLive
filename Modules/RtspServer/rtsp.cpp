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

enum event_type_t {
    event_play = 0,
    event_stop
};

typedef struct async_event_t{
    event_type_t type;
    async_event_t *next;
    void         *param1;
    void         *param2;
    async_event_t()
        : type(event_play)
        , next(NULL)
        , param1(NULL)
        , param2(NULL)
    {}
};

uv_async_t    _uv_async_;
async_event_t *_event_list_ = NULL;
async_event_t *_event_end_ = NULL;
uv_mutex_t    _event_mutex_;

static void run_loop_thread(void* arg)
{
    while (_inner_uv_) {
        uv_run(_uv_loop_, UV_RUN_DEFAULT);
        Sleep(200);
    }
    uv_loop_close(_uv_loop_);
}

static void on_uv_async(uv_async_t* handle){
    uv_mutex_lock(&_event_mutex_);
    for (async_event_t *it = _event_list_; it != NULL; it = it->next){
        if(it->type == event_play) {
            RTSP_REQUEST option = *(RTSP_REQUEST*)it->param1;
            play_cb cb = (play_cb)it->param2;
            if (option.dev_type == DEV_HIK) {
                CRtspClient* c = new CRtspHik(option);
                c->play(cb);
                _map_rtsp_.insert(make_pair(option.ssid, c));
            }
            delete it->param1;
        } else if(it->type == event_stop){
            string ssid = *(string*)it->param1;
            auto find = _map_rtsp_.find(ssid);
            if (find != _map_rtsp_.end()) {
                find->second->stop();
                _map_rtsp_.erase(find);
            }
            delete it->param1;
        }
    }
    for (async_event_t *it = _event_list_; it != NULL; ){
        async_event_t *tmp = it;
        it = it->next;
        delete tmp;
    }
    _event_list_ = NULL;
    _event_end_  = NULL;
    uv_mutex_unlock(&_event_mutex_);
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
            free(_uv_loop_);
            return ret;
        }
    }
    uv_async_init(_uv_loop_, &_uv_async_, on_uv_async);
    uv_mutex_init(&_event_mutex_);
    return 0;
}

void stop_uv() {
    _inner_uv_ = false;
    uv_stop(_uv_loop_);
}

int rtsp_play(RTSP_REQUEST option, play_cb cb) {
    uv_mutex_lock(&_event_mutex_);
    async_event_t *e = new async_event_t;
    e->type = event_play;
    e->param1 = new RTSP_REQUEST;
    *((RTSP_REQUEST*)e->param1) = option;
    e->param2 = cb;
    if(_event_list_ == NULL) {
        _event_list_ = e;
        _event_end_ = e;
    } else {
        _event_end_->next = e;
        _event_end_ = e;
    }
    uv_mutex_unlock(&_event_mutex_);
    uv_async_send(&_uv_async_);
    return 0;
}

int rtsp_stop(string ssid) {
    uv_mutex_lock(&_event_mutex_);
    async_event_t *e = new async_event_t;
    e->type = event_stop;
    e->param1 = new string;
    *((string*)e->param1) = ssid;
    if(_event_list_ == NULL) {
        _event_list_ = e;
        _event_end_ = e;
    } else {
        _event_end_->next = e;
        _event_end_ = e;
    }
    uv_mutex_unlock(&_event_mutex_);
    uv_async_send(&_uv_async_);
    return 0;
}

//rtsp析构前确认从map移除
void sure_move(string ssid) {
    auto find = _map_rtsp_.find(ssid);
    if (find != _map_rtsp_.end()) {
        _map_rtsp_.erase(find);
    }
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