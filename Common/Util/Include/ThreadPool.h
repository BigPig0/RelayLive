/**
 * ThreadPool相关的api是Windows Vista/Windows Server 2008以上新平台才有的
 */

#pragma once
#include "ExportDefine.h"
#include "Runnable.h"

class ThreadPool;
class ThreadRunnable : public Runnable
{
public:
    ThreadRunnable(){};
    virtual ~ThreadRunnable(){};
    virtual void run() = 0;

    PTP_WORK        m_pWork;
    ThreadPool*     m_pool;
};
class TimerRunnable : public Runnable
{
public:
    PTP_TIMER       m_pWork;
    ThreadPool*     m_pool;
};

////////////////////////////////////////////////////////////////////////////////
class COMMON_API ThreadPool : public LastError
{
public:
    explicit ThreadPool(int threadNum = 4);
    ~ThreadPool();

    /**
     * 初始化线程池环境
     */
    /*static*/ void init();
    /**
     * 清理线程池环境
     */
    /*static*/ void cleanup();
    /**
     * 获取系统CPU内核数量
     * @return CPU内核数量
     */
    static int cpuNum();

    /**
     * 创建一个工作线程放到线程池中，启动时执行runnable的run()
     * @param runnable[in] 业务实例。该实例是运行业务逻辑的，与函数里面创建的工作线程实例不是同一个东西。
     * @param repeats[in] 业务功能执行次数。
     * @return 成功true，失败false
     */
    bool startWork(ThreadRunnable *runnable, int repeats = 1);

    /**
     * 结束一个工作线程
     * @param runnable[in] 业务实例.
     * @return 成功true，失败false
     */
    bool closeWork(ThreadRunnable *runnable);

    /**
     * 注册一个线程池定时器，时间达执行runnable的run()。线程池定时器是为线程池服务的，并不是为每个工作线程都创建定时器。
     * @param runnable[in] 业务实例。
     * @param msDueTime[in] 第一次启动的时间(毫秒) 如果需要立即执行填0
     * @param msPeriod[in] 执行间隔时间(毫秒)
     * @return 成功true，失败false
     */
    bool startTimer(TimerRunnable *runnable, int msDueTime, int msPeriod);

    /**
     * 结束一个定时器，对于只需要执行一次的定时任务，可以在runnable的run结束时调用
     */
    bool closeTimer(TimerRunnable *runnable);

    /**
     * 在内核对象触发时调用一个函数，执行runnable的run()
     * @param runnable[in] 业务实例.
     * @param event[in] 事件内核对象
     * @return 成功true，失败false
     */
    bool startWait(Runnable *runnable, HANDLE event);

    /**
     * 在异步I/O请求完成时调用一个函数，执行runnable的run()
     * @param runnable[in] 业务实例.
     * @param io[in] I/O句柄
     * @return 成功true，失败false
     */
    bool startIo(IoRunnable *runnable, HANDLE io);

    /**
     * 清理工作线程，并关闭线程池
     * @param wait[in] true等待已提交但尚未处理的工作完成；false当前工作完成，取消已提交但尚未执行的动作。
     */
    void stop(bool wait = true);

private:
    static void doWork(PTP_CALLBACK_INSTANCE inst, void *p, PTP_WORK work);
    static void expired(PTP_CALLBACK_INSTANCE inst, void *p, PTP_TIMER timer);
    static void fired(PTP_CALLBACK_INSTANCE inst, void *p, PTP_WAIT wait, TP_WAIT_RESULT waitResult);
    static void ioCompletion(PTP_CALLBACK_INSTANCE inst, void *p, void *o, ULONG ioResult, ULONG_PTR bytes, PTP_IO io);

private:
    /*static*/ TP_CALLBACK_ENVIRON  m_env;              //< 回调环境
    PTP_POOL                    m_pool;             //< 线程池
    PTP_CLEANUP_GROUP           m_cleanupgroup;     //< 清理组
};

/**
 * Thread Local Storage
 * 线程本地存储，作用是能将数据和执行的特定的线程联系起来。多线程中能访问全局变量，但不互相影响。
 * 该类实现的是动态TLS
 */
class COMMON_API Tls : public LastError
{
public:
    explicit Tls();
    ~Tls();

    bool get(void **pp);
    bool set(void *p);

private:
    DWORD m_tls;
};
