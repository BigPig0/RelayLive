#pragma once
#include "util_public.h"

namespace Log
{

enum class Print {
    console = 0x1,
    file    = 0x2,
    both    = 0x3
};

enum class Level {
    debug   = 0,
    info,
    warning,
    error
};

bool UTIL_API open(Print print, Level level, const char *pathFileName);
void UTIL_API close();

void UTIL_API print(Level level, const char *file, int line, const char *func, const char *fmt, ...);

#ifdef WINDOWS_IMPL
#define debug(fmt, ...)         print(Log::Level::debug,   __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
#define info(fmt, ...)          print(Log::Level::info,    __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
#define warning(fmt, ...)       print(Log::Level::warning, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
#define error(fmt, ...)         print(Log::Level::error,   __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
#else
#define debug(fmt, args...)         print(Log::Level::debug,   __FILE__, __LINE__, __FUNCTION__, fmt, ## args)
#define info(fmt, args...)          print(Log::Level::info,    __FILE__, __LINE__, __FUNCTION__, fmt, ## args)
#define warning(fmt, args...)       print(Log::Level::warning, __FILE__, __LINE__, __FUNCTION__, fmt, ## args)
#define error(fmt, args...)         print(Log::Level::error,   __FILE__, __LINE__, __FUNCTION__, fmt, ## args)
#endif
}
