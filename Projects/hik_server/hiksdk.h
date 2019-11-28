#pragma once

namespace HikPlat {
	bool Init();
	void Cleanup();
	void GetDevices();
	bool Play(void* user);
};