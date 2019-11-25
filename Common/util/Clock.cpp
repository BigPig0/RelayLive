#include "Clock.h"
#include <windows.h> 

Clock::Clock()
{
}

double Clock::prequency()
{
    static double m_dfFreq = -1;
    if(m_dfFreq <= 0)
    {
        LARGE_INTEGER litmp; 
        QueryPerformanceFrequency(&litmp);  
        m_dfFreq = (double)litmp.QuadPart;
    }
    return m_dfFreq;
}

void Clock::start()
{
    LARGE_INTEGER litmp; 
    QueryPerformanceCounter(&litmp);  
    m_llStart = litmp.QuadPart;
}

void Clock::end()
{
    LARGE_INTEGER litmp; 
    QueryPerformanceCounter(&litmp);  
    m_llEnd = litmp.QuadPart;
}

uint64_t Clock::get()
{
    return (m_llEnd - m_llStart);
}