#pragma once
#include "commonDefine.h"

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
    virtual void run(OVERLAPPED *o, ulong ioResult, ulong64 bytes, PTP_IO io) = 0;
};
