#pragma once
#include "ExportDefine.h"

#include <windows.h> 

/**
 * 高精度计时器，用于计算一个操作所消耗的时间
 */
class Clock
{
public:
    Clock()
    {
    }
    
    /** 获得计数器的时钟频率 */
    static double prequency()
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

    /** 启动计时器 */
    void start()
    {
        LARGE_INTEGER litmp; 
        QueryPerformanceCounter(&litmp);  
        m_llStart = litmp.QuadPart;
    }

    /** 停止计时器 */
    void end()
    {
        LARGE_INTEGER litmp; 
        QueryPerformanceCounter(&litmp);  
        m_llEnd = litmp.QuadPart;
    }

    /**
     * 获取start和end之间的时间间隔
     * @return 间隔的计数次数，需要除以计数器的时钟频率才是毫秒
     * @note 如果start和end间隔较短，get的值太小，应该将多次get的结果加起来再进行除计算，否则会不准
     */
    LONGLONG get()
    {
        return (m_llEnd - m_llStart);
    }
private:
    LONGLONG    m_llStart;
    LONGLONG    m_llEnd;
    
};