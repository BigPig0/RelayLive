#include "util_clock.h"
#if defined(WINDOWS_IMPL)        /**Windows*/
#include <windows.h>
#elif defined(LINUX_IMPL)        /**Linux*/
#include <sys/time.h>        //gettimeofday()
#endif

namespace util {

static uint64_t m_dfFreq = -1;

Clock::Clock()
{
    if(m_dfFreq <= 0)
    {
#ifdef WINDOWS_IMPL
        LARGE_INTEGER litmp; 
        if(!QueryPerformanceFrequency(&litmp))
            m_dfFreq = 0;
        else
            m_dfFreq = (double)litmp.QuadPart;
#else
        m_dfFreq = 1;
#endif
    }
}

double Clock::prequency()
{
    if(0 == m_dfFreq || 1 == m_dfFreq)
        return 1.0;
    return m_dfFreq * 1.0 /1000;    // 系统时钟每秒钟滴答次数，需要改成每毫秒
}

void Clock::start()
{
#if defined(WINDOWS_IMPL)
    if(0 == m_dfFreq)
        m_llStart = GetTickCount();
    else {
        LARGE_INTEGER litmp; 
        QueryPerformanceCounter(&litmp);  
        m_llStart = litmp.QuadPart;
    }
#elif defined(LINUX_IMPL)
    struct timeval litmp;
    gettimeofday(&litmp,NULL);
    m_llStart = litmp.tv_sec * 1000000 + litmp.tv_usec;
#endif
}

void Clock::end()
{
#if defined(WINDOWS_IMPL)
    if(0 == m_dfFreq)
        m_llEnd = GetTickCount();
    else {
        LARGE_INTEGER litmp; 
        QueryPerformanceCounter(&litmp);  
        m_llEnd = litmp.QuadPart;
    }
#elif defined(LINUX_IMPL)
    struct timeval litmp;
    gettimeofday(&litmp,NULL);
    m_llEnd = litmp.tv_sec * 1000000 + litmp.tv_usec;
#endif
}

uint64_t Clock::get()
{
    return (m_llEnd - m_llStart);
}
}