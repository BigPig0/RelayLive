/**
 * TimerQueue相关API是传统的旧api Windows XP/Windows Server 2003以上平台使用
 */
#pragma once
#include "util_public.h"
#include "Runnable.h"
#include "Mutex.h"
#include "LastError.h"
#include <stdint.h>

/**
 * 高性能时间获取工具
 */
class UTIL_API HighResolutionTimer
{
public:
    /** 若未初始化则初始化 */
    static void init();
    /** 返回当前时刻的计数 */
    static uint64_t now();
    /* 两个性能计数的时间间隔，单位毫秒 */
    static uint64_t diffPC(uint64_t *a, uint64_t *b);
    /** 毫秒时间转为计数 */
    static uint64_t getCounters(uint64_t ms);
    /** 计数转为毫秒时间 */
    static uint64_t getMs(uint64_t counter);
private:
    static uint64_t m_nFrequency; //定时器的频率
    static uint32_t m_nInitCount;
    static CriticalSection m_cs;
};

class UTIL_API TimerQueue
{
    friend class Timer;
public:
    /**
     * 创建全局的定时器队列
     */
    static bool init();

    /**
     * 删除全局定时器队列
     */
    static void cleanup();

private:
    /** 全局定时器队列实例句柄 */
    static HANDLE          m_timerQueue;
    static uint32_t        m_nInitCount;
    static CriticalSection m_cs;
};

/**
 * 使用定时器队列的定时器实现
 */
class UTIL_API Timer : public LastError
{
public:
    explicit Timer();
    ~Timer();

    /**
     * 注册一个定时器，时间达执行runnable的run()。
     * @param runnable[in] 业务实例。
     * @param milliseconds[in] 定时器间隔时间
     * @param immediately[in] 是否立即执行
     * @return 成功true，失败false
     */
    bool start(Runnable *runnable, int milliseconds, bool immediately = false);

    /**
     * 结束并删除定时器队列
     */
    bool stop(bool wait = true);

private:
    static void expired(void *p, BOOLEAN fired);

    HANDLE m_timer;
};

////////////////////////////////////////////////////////////////////////////////
