#pragma once
#include "worker.h"
#include <string>

namespace HikSdk {
	bool Init();
	void Cleanup();
	int Play(CLiveWorker *pWorker);
    void Stop(CLiveWorker *pWorker);
};