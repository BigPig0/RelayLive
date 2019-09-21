#pragma once
#include <windows.h>
#include <time.h>

class CTimeFormat
{
public:

    /**
     * 讲一个格式为yyyyMMddhhmmss格式的字符串转成时间整数
     */
    static time_t scanTime(const char buf[32]);

    /**
     * 打印当前时间
     * @param fmt 输出的格式(不包含毫秒，最后的子串会加上毫秒数)
     * @param buf 输出内容的内存
     */
    static char *printNow(const char *fmt, char buf[32]);

    /**
     * 打印指定时间
     * @param time 指定的时间
     * @param fmt 输出的格式
     * @param buf 输出内容的内存
     */
    static char *printTime(time_t *time, const char *fmt, char buf[32]);

private:

};

inline
time_t CTimeFormat::scanTime(const char buf[32])
{
    struct tm tm;

    sscanf_s(buf, "%4d%2d%2d%2d%2d%2d",
        &tm.tm_year,
        &tm.tm_mon,
        &tm.tm_mday,
        &tm.tm_hour,
        &tm.tm_min,
        &tm.tm_sec);

    tm.tm_year -= 1900;
    tm.tm_mon --;
    tm.tm_isdst = -1;

    return mktime(&tm);
}

inline
char *CTimeFormat::printNow(const char *fmt, char buf[32])
{
    SYSTEMTIME systime;
    GetLocalTime(&systime);

    struct tm tm;
    tm.tm_year      = systime.wYear - 1900;
    tm.tm_mon       = systime.wMonth - 1;
    tm.tm_mday      = systime.wDay;
    tm.tm_hour      = systime.wHour;
    tm.tm_min       = systime.wMinute;
    tm.tm_sec       = systime.wSecond;
    tm.tm_isdst     = -1;

    char stamp[32];
    strftime(stamp, 32, fmt, &tm);
    sprintf_s(buf, 32, "%s.%03d", stamp, systime.wMilliseconds);
    return buf;
}

inline
char *CTimeFormat::printTime(time_t *time, const char *fmt, char buf[32])
{
    struct tm tm;
    localtime_s(&tm, time);

    char stamp[32];
    strftime(stamp, 32, fmt, &tm);
    sprintf_s(buf, 32, "%s", stamp);
    return buf;
}