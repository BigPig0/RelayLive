#pragma once
#include <string>

namespace HikPlat {
	bool Init();
	void Cleanup();
	void GetDevices();
	int Play(void* user, std::string devid);
    void Stop(int ret);
};