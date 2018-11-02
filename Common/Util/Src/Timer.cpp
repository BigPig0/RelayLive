#include "stdafx.h"
#include "common.h"
#include "Timer.h"

uint32_t        HighResolutionTimer::m_nInitCount = false;
uint64_t        HighResolutionTimer::m_nFrequency = 0;
CriticalSection HighResolutionTimer::m_cs;

uint32_t        TimerQueue::m_nInitCount = false;
HANDLE          TimerQueue::m_timerQueue = NULL;
CriticalSection TimerQueue::m_cs;


void HighResolutionTimer::init()
{
    MutexLock lock(&m_cs);
    if (!m_nInitCount)
    {
        m_nInitCount = true;
        /* 使用高精度计数器 */
        LARGE_INTEGER freq;
        // 返回定时器的频率
        if(!QueryPerformanceFrequency(&freq))
        {
            /* 硬件不支持,只能使用毫秒作为计数器 */
            m_nFrequency = 0;
        }
        else
        {
            m_nFrequency = freq.QuadPart;
        }
    }
}

uint64_t HighResolutionTimer::now()
{
    /* 返回当前时间对应的counter*/
    LARGE_INTEGER counter;
    if(m_nFrequency == 0)
    {
        // 返回从操作系统启动所经过的毫秒数
        return GetTickCount();
    }
    else
    {
        // 返回定时器当前计数值
        if(!QueryPerformanceCounter(&counter))
            return GetTickCount();

        return counter.QuadPart;
    }
}

uint64_t HighResolutionTimer::getCounters(uint64_t ms)
{
    /* 把ms时间换算为计数个数 */
    if( 0 == m_nFrequency )
    {
        /* 硬件不支持高精度计数,毫秒数就是counter */
        return ms;
    }
    else
    {
        return ms * m_nFrequency / 1000;
    }
}

uint64_t HighResolutionTimer::getMs(uint64_t counter)
{
    if( 0 == m_nFrequency )
    {
        /* 硬件不支持高精度计数,此时 counter 就是 毫秒*/
        return counter;
    }
    else
    {
        return static_cast<__int64>((counter * 1.0 / m_nFrequency) * 1000);
    }
}

uint64_t HighResolutionTimer::diffPC(uint64_t *a, uint64_t *b)
{
    if( 0 == m_nFrequency )
    {
        /* 硬件不支持高精度计数,a和b 就是 毫秒*/
        return b - a;
    }
    else
    {
        return static_cast<uint64_t>((b - a) * 1.0 / m_nFrequency * 1000);
    }
}

////////////////////////////////////////////////////////////////////////////////

bool TimerQueue::init()
{
    MutexLock lock(&m_cs);
    if(0 == m_nInitCount)
    {
        m_timerQueue = CreateTimerQueue();
        if (!m_timerQueue)
            return false;
    }
    m_nInitCount++;
    return true;
}

void TimerQueue::cleanup()
{
    m_nInitCount--;

    if (0 == m_nInitCount && m_timerQueue)
        DeleteTimerQueueEx(m_timerQueue, NULL);
}

////////////////////////////////////////////////////////////////////////////////
Timer::Timer() : m_timer(NULL)
{}

Timer::~Timer()
{
    if (m_timer)
        stop(false);
}


bool Timer::start(Runnable *runnable, int milliseconds, bool immediately)
{
    if (!CreateTimerQueueTimer(&m_timer, TimerQueue::m_timerQueue, (WAITORTIMERCALLBACK)&Timer::expired
        , runnable, milliseconds, immediately ? 0 : milliseconds, WT_EXECUTEDEFAULT))
        GOTO(failed);
    return true;
failed:
    return false;
}

bool Timer::stop(bool wait)
{
    HANDLE event;
    if (wait)
        event = INVALID_HANDLE_VALUE;
    else
        event = NULL;

    if (!DeleteTimerQueueTimer(TimerQueue::m_timerQueue, m_timer, event))
        GOTO(failed);
    m_timer = NULL;
    return true;
failed:
    return false;
}

void Timer::expired(void *p, BOOLEAN fired)
{
    UNUSED(fired);
    Runnable *runnable = (Runnable *)p;
    runnable->run();
}
