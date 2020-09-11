#pragma once

namespace uvLogPlus {
#if defined(_WIN32) || defined(WIN32)
#define LINE_SEPARATOR "\r\n"
#elif defined(__APPLE__)
#define LINE_SEPARATOR "\r"
#else
#define LINE_SEPARATOR "\n"
#endif

    extern const char* levelNote[];

    extern int file_sys_check_path(const char *path);

    extern int file_sys_exist(const char *path);
}