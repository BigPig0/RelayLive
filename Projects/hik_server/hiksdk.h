#pragma once
#include <string>

namespace HikPlat {
	bool Init();
	void Cleanup();
	int Play(void* user, std::string devid);
    void Stop(int ret);
};