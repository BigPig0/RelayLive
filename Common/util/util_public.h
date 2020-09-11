#pragma once

#ifdef UTIL_EXPORTS
#define UTIL_API __declspec(dllexport)
#else
#define UTIL_API
#endif

#if defined(_WIN32) || defined(WIN32)        /**Windows*/
#define WINDOWS_IMPL
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(BSD)    /**Linux*/
#define LINUX_IMPL
#endif

#include <string>
using namespace std;