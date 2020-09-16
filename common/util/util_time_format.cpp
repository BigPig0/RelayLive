#include "util_time_format.h"
#include <time.h>

namespace util {

time_t CTimeFormat::scanTime(const char buf[32])
{
    struct tm tm;
    sscanf(buf, "%4d%2d%2d%2d%2d%2d",
        &tm.tm_year,
        &tm.tm_mon,
        &tm.tm_mday,
        &tm.tm_hour,
        &tm.tm_min,
        &tm.tm_sec);

    tm.tm_year -= 1900;
    tm.tm_mon--;
    tm.tm_isdst = -1;

    return mktime(&tm);
}

char *CTimeFormat::printNow(const char *fmt, char buf[32])
{
    time_t now = time(NULL);
    struct tm tm;
#if defined(WINDOWS_IMPL)
    localtime_s(&tm, &now);
#elif defined(LINUX_IMPL)
    localtime_r(&now, &tm);
#endif

    strftime(buf, 32, fmt, &tm);
    return buf;
}

string CTimeFormat::printNow(const char *fmt) {
    time_t now = time(NULL);
    struct tm tm;
#if defined(WINDOWS_IMPL)
    localtime_s(&tm, &now);
#elif defined(LINUX_IMPL)
    localtime_r(&now, &tm);
#endif

    char buf[32];
    strftime(buf, 32, fmt, &tm);
    return buf;
}

char *CTimeFormat::printTime(time_t time, const char *fmt, char buf[32])
{
    struct tm tm;
#if defined(WINDOWS_IMPL)
    localtime_s(&tm, &time);
#elif defined(LINUX_IMPL)
    localtime_r(&time, &tm);
#endif

    strftime(buf, 32, fmt, &tm);
    return buf;
}

string CTimeFormat::printTime(time_t time, const char *fmt) {
    struct tm tm;
#if defined(WINDOWS_IMPL)
    localtime_s(&tm, &time);
#elif defined(LINUX_IMPL)
    localtime_r(&time, &tm);
#endif

    char buf[32];
    strftime(buf, 32, fmt, &tm);
    return buf;
}

struct tm CTimeFormat::getTimeInfo(time_t time) {
    struct tm tm;
#if defined(WINDOWS_IMPL)
    localtime_s(&tm, &time);
#elif defined(LINUX_IMPL)
    localtime_r(&time, &tm);
#endif
    return tm;
}
}