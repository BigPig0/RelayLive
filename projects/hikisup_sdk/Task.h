#pragma once
#include "util.h"
#include "uv.h"
#include <string>

using namespace std;
using namespace util;

struct Gps {
    uint64_t taskId;     //����ID
    string   realTime;   //ʵʱʱ��
    double   lat;        //γ��
    double   lon;        //����
    double   speed;      //�ٶ�
    double   angle;      //�Ƕ�
};

namespace Task {
    bool Init(uv_loop_t *loop);
    void addTask(Gps *gps);
}