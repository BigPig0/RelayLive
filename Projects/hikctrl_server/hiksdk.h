#pragma once
#include <string>

namespace HikPlat {
	bool Init();
	void Cleanup();
    std::string GetDevicesJson();
    void DeviceControl(std::string strDev, int nInOut = 0, int nUpDown = 0, int nLeftRight = 0);
};