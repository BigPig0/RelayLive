#ifndef _UTIL_CAPI_
#define _UTIL_CAPI_
#include "utilc_export.h"
#include <string.h>
#include <stdint.h>

#if defined(WINDOWS_IMPL)
#define strcasecmp  _stricmp
#define strncasecmp _strnicmp
#define strtok_r    strtok_s

#ifdef __cplusplus
extern "C" {
#endif
_UTILC_API int  getpid();
_UTILC_API int  gettid();
_UTILC_API void sleep(uint32_t seconds);
#ifdef __cplusplus
}
#endif

#elif defined(LINUX_IMPL)
#include <unistd.h>
#ifdef __cplusplus
extern "C" {
#endif
_UTILC_API int  gettid();
_UTILC_API void Sleep(uint32_t milliSeconds);
#ifdef __cplusplus
}
#endif
#endif

#endif