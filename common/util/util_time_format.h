/**
 * time_t与字符串互相转换，只精确到秒
 * %y 不带世纪的十进制年份（值从0到99）
 * %Y 带世纪部分的十制年份
 * %m 十进制表示的月份
 * %d 十进制表示的每月的第几天
 * %H 24小时制的小时
 * %I 12小时制的小时
 * %M 十时制表示的分钟数
 * %S 十进制的秒数
 */

#pragma once
#include "util_public.h"

namespace util {

class UTIL_API CTimeFormat
{
public:

    /**
     * 讲一个格式为yyyyMMddhhmmss格式的字符串转成时间整数
     */
    static time_t scanTime(const char buf[32]);

    /**
     * 打印当前时间
     * @param fmt 输出的格式
     * @param buf 输出内容的内存
     */
    static char *printNow(const char *fmt, char buf[32]);

    static string printNow(const char *fmt);

    /**
     * 打印指定时间
     * @param time 指定的时间
     * @param fmt 输出的格式
     * @param buf 输出内容的内存
     */
    static char *printTime(time_t time, const char *fmt, char buf[32]);

    static string printTime(time_t time, const char *fmt);

    /**
     * 将一个时间转为结构
     */
    static struct tm getTimeInfo(time_t time);

};
};