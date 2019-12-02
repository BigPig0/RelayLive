#pragma once
#include <string>

struct cameraInfo {
    std::string id;
    std::string name;
};

extern vector<cameraInfo> _cameras;

namespace HikPlat {
	bool Init();
	void Cleanup();
    void DeviceControl(string strDev, int nInOut = 0, int nUpDown = 0, int nLeftRight = 0);
};