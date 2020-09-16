/*!
 * 计算操作经过了多少毫秒
 *
 * Clock c;
 * uint64_t num = 0;
 * c.start();
 * do something...
 * c.end();
 * num += c.get();
 * c.start();
 * do something...
 * c.end();
 * num += c.get();
 * uint64_t millisecs = num/c.prequency();
 *
 */

#pragma once
#include "util_public.h"
#include <stdint.h>

namespace util {

/**
 * 高精度计时器，用于计算一个操作所消耗的时间
 */
class UTIL_API Clock
{
public:
    Clock();
    
    /** 获得计数器的时钟频率 */
    static double prequency();

    /** 启动计时器 */
    void start();

    /** 停止计时器 */
    void end();

    /**
     * 获取start和end之间的时间间隔
     * @return 间隔的计数次数，需要除以计数器的时钟频率才是毫秒
     * @note 如果start和end间隔较短，get的值太小，应该将多次get的结果加起来再进行除计算，否则会不准
     */
    uint64_t get();

private:
    uint64_t    m_llStart;
    uint64_t    m_llEnd;
    
};
}