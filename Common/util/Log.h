#pragma once

#ifdef LOG_EXPORTS
#define LOG_API __declspec(dllexport)
#else
#define LOG_API
#endif

namespace Log
{

enum class Print
{
    console = 0x1,
    file    = 0x2,
    both    = 0x3
};

enum class Level
{
    debug   = 1,
    warning = 2,
    error   = 3
};

bool LOG_API open(Print print, Level level, const char *pathFileName);
void LOG_API close();

void LOG_API print(Level level, const char *fmt, ...);

//#ifdef NDEBUG
#if 0
#define debug(fmt, ...)         Level::debug
#else
#define debug(fmt, ...)         print(Log::Level::debug,   fmt, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#endif

#define warning(fmt, ...)       print(Log::Level::warning, fmt, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define error(fmt, ...)         print(Log::Level::error,   fmt, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
}
