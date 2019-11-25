#pragma once
#include "util_def.h"
#include <windows.h>

/**
 * 线程池中的工作线程
 */
class Runnable
{
public:
    virtual ~Runnable() {}
    virtual void run() = 0;
};

/**
 * IOCP模型中的I/O线程
 */
class IoRunnable
{
public:
    virtual ~IoRunnable() {}
    virtual void run(OVERLAPPED *o, uint32_t ioResult, uint64_t bytes, PTP_IO io) = 0;
};
