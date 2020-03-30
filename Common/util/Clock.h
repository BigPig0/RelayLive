/*!
 * \class Clock
 *
 * \brief 
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
 * \note 
 *
 * \author wlla
 *
 * \version 1.0
 *
 */
#pragma once
#include "util_public.h"
#include <stdint.h>

/**
 * �߾��ȼ�ʱ�������ڼ���һ�����������ĵ�ʱ��
 */
class UTIL_API Clock
{
public:
    Clock();
    
    /** ��ü�������ʱ��Ƶ�� */
    static double prequency();

    /** ������ʱ�� */
    void start();

    /** ֹͣ��ʱ�� */
    void end();

    /**
     * ��ȡstart��end֮���ʱ����
     * @return ����ļ�����������Ҫ���Լ�������ʱ��Ƶ�ʲ��Ǻ���
     * @note ���start��end����϶̣�get��ֵ̫С��Ӧ�ý����get�Ľ���������ٽ��г����㣬����᲻׼
     */
    uint64_t get();

private:
    uint64_t    m_llStart;
    uint64_t    m_llEnd;
    
};