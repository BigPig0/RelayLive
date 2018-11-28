#ifndef _UTIL_CAPI_
#define _UTIL_CAPI_
#include "util_export.h"

#if defined(WIN32) || defined(_WIN32)
#if _MSC_VER < 1700
#include <crtdefs.h>
#include <crtdbg.h>
typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
#else
#include <stdint.h>
#endif
#include <windows.h>
#endif

#include <string.h>
#if defined(WIN32) || defined(_WIN32)
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define sleep(ms) Sleep(ms)
#define strtok_r strtok_s
#else
#include <strings.h>
#endif

#endif