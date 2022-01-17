#pragma once
#include "util.h"
#include <map>
#include <string>
#include <vector>

using namespace std;
using namespace util;

struct VipTask {
    uint64_t taskId;
    uint64_t accountId;
};

namespace DbTsk {

bool Init();

bool CheckDeviceId(string &devid, VipTask &tskInfo);

};