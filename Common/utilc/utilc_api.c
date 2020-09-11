#include "utilc_api.h"

#if defined(WINDOWS_IMPL)
#include <windows.h>

int getpid() {
    return GetCurrentProcessId();
}

int gettid() {
    return GetCurrentThreadId();
}

void sleep(uint32_t seconds) {
    Sleep(seconds*1000);
}
#endif

#if defined(LINUX_IMPL)
#include <unistd.h>
#include <pthread.h>
int gettid() {
    return pthread_self();
}

_UTILC_API void Sleep(uint32_t milliSeconds) {
    usleep(milliSeconds*1000);
}
#endif