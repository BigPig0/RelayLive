#pragma once
#include "uv.h"
#include "worker.h"
#include <string>

namespace HikSdk {
	bool Init(uv_loop_t *loop);
	void Cleanup();
	int Play(CLiveWorker *pWorker);
    void Stop(CLiveWorker *pWorker);
};