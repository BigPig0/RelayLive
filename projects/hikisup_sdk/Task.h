#pragma once
#include "util.h"
#include "uv.h"
#include <string>

using namespace std;
using namespace util;

struct Gps {
    uint64_t taskId;     //任务ID
    string   realTime;   //实时时间
    double   lat;        //纬度
    double   lon;        //经度
    double   speed;      //速度
    double   angle;      //角度
};

namespace Task {
    bool Init(uv_loop_t *loop);
    void addTask(Gps *gps);
}