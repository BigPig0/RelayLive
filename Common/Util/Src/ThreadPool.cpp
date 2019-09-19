#include "stdafx.h"
#include <Windows.h>
#include "LastError.h"
#include "Runnable.h"
#include "ThreadPool.h"


ThreadPool::ThreadPool(int threadNum)
    : m_pool(NULL), m_cleanupgroup(NULL)
{
    init();
    /** 创建线程池 */
    m_pool = CreateThreadpool(NULL);
    if (!m_pool)
        GOTO(failed);

    /** 设置最小线程数 */
    SetThreadpoolThreadMinimum(m_pool, 1);
    /** 设置最大线程数 */
    SetThreadpoolThreadMaximum(m_pool, threadNum);

    /** 创建清理组 */
    m_cleanupgroup = CreateThreadpoolCleanupGroup();
    if (!m_cleanupgroup)
        GOTO(failed);

    /** 设置回调环境 */
    SetThreadpoolCallbackPool(&m_env, m_pool);
    SetThreadpoolCallbackCleanupGroup(&m_env, m_cleanupgroup, NULL);

failed:
    ;
}


ThreadPool::~ThreadPool()
{
    stop(false);
    if (m_pool)
        CloseThreadpool(m_pool);
    cleanup();
}


void ThreadPool::init()
{
    InitializeThreadpoolEnvironment(&m_env);
}


void ThreadPool::cleanup()
{
    DestroyThreadpoolEnvironment(&m_env);
}


int ThreadPool::cpuNum()
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}


bool ThreadPool::startWork(ThreadRunnable *runnable, int repeats)
{
    runnable->m_pool = this;
    runnable->m_pWork = CreateThreadpoolWork((PTP_WORK_CALLBACK)&ThreadPool::doWork, runnable, &m_env);
    if (!runnable->m_pWork)
        GOTO(failed);
    for (int i = 0; i < repeats; i++)
        SubmitThreadpoolWork(runnable->m_pWork);
    return true;
failed:
    return false;
}


bool ThreadPool::closeWork(ThreadRunnable *runnable)
{
    if (nullptr == runnable)
    {
        return false;
    }

    CloseThreadpoolWork(runnable->m_pWork);
    delete runnable;

    return true;
}


bool ThreadPool::startTimer(TimerRunnable *runnable, int msDueTime, int msPeriod)
{
    runnable->m_pool = this;
    runnable->m_pWork = CreateThreadpoolTimer((PTP_TIMER_CALLBACK)&ThreadPool::expired, runnable, &m_env);
    if (!runnable->m_pWork)
        GOTO(failed);

    FILETIME due;
    if (0 ==msDueTime)
    {
        memset(&due, 0, sizeof(due));
    }
    else
    {
        ULARGE_INTEGER ulDue;
        ulDue.QuadPart = (ULONGLONG)(-(10 * 1000 * msDueTime)); // 100ns为时间单位
        due.dwHighDateTime = ulDue.HighPart;
        due.dwLowDateTime  = ulDue.LowPart;
    }

    SetThreadpoolTimer(runnable->m_pWork, &due, msPeriod, 5); //msWindowLength不设置为0，防止多个定时器冲突
    return true;
failed:
    return false;
}

bool ThreadPool::closeTimer(TimerRunnable *runnable)
{
    CHECKPOINT_BOOL(runnable)

    //SetThreadpoolTimer(runnable->m_pWork,NULL,0,0);
    //WaitForThreadpoolTimerCallbacks(runnable->m_pWork,TRUE);
    CloseThreadpoolTimer(runnable->m_pWork);
    delete runnable;
    return true;
}

bool ThreadPool::startWait(Runnable *runnable, HANDLE event)
{
    PTP_WAIT wait = CreateThreadpoolWait((PTP_WAIT_CALLBACK)&ThreadPool::fired, runnable, &m_env);
    if (!wait)
        GOTO(failed);

    SetThreadpoolWait(wait, event, NULL);
    //WaitForThreadpoolWaitCallbacks(wait, FALSE);
    return true;
failed:
    return false;
}


bool ThreadPool::startIo(IoRunnable *runnable, HANDLE handle)
{
    PTP_IO io = CreateThreadpoolIo(handle, (PTP_WIN32_IO_CALLBACK)&ThreadPool::ioCompletion, runnable, &m_env);
    if (!io)
        GOTO(failed);

    StartThreadpoolIo(io);
    return true;
failed:
    return false;
}


void ThreadPool::stop(bool wait)
{
    if (m_cleanupgroup)
    {
        CloseThreadpoolCleanupGroupMembers(m_cleanupgroup, wait ? FALSE : TRUE, NULL);
        CloseThreadpoolCleanupGroup(m_cleanupgroup);
        m_cleanupgroup = NULL;
    }
}


void ThreadPool::doWork(PTP_CALLBACK_INSTANCE inst, void *p, PTP_WORK work)
{
    UNUSED(inst);
    UNUSED(work);
    Runnable *runnable = (Runnable *)p;
    runnable->run();
}


void ThreadPool::expired(PTP_CALLBACK_INSTANCE inst, void *p, PTP_TIMER timer)
{
    UNUSED(inst);
    UNUSED(timer);
    Runnable *runnable = (Runnable *)p;
    runnable->run();
}


void ThreadPool::fired(PTP_CALLBACK_INSTANCE inst, void *p, PTP_WAIT wait, TP_WAIT_RESULT waitResult)
{
    UNUSED(inst);
    UNUSED(wait);
    UNUSED(waitResult);
    Runnable *runnable = (Runnable *)p;
    runnable->run();
}


void ThreadPool::ioCompletion(PTP_CALLBACK_INSTANCE inst, void *p, void *o, ULONG ioResult, ULONG_PTR bytes, PTP_IO io)
{
    UNUSED(inst);
    UNUSED(io);
    UNUSED(ioResult);
    IoRunnable *runnable = (IoRunnable *)p;
    runnable->run((OVERLAPPED *)o, ioResult, bytes, io);
}


////////////////////////////////////////////////////////////////////////////////

Tls::Tls()
{
    m_tls = TlsAlloc();
    if (m_tls == TLS_OUT_OF_INDEXES)
        GOTO(failed);
failed:
    ;
}


Tls::~Tls()
{
    if (m_tls != TLS_OUT_OF_INDEXES)
        TlsFree(m_tls);
}


bool Tls::get(void **pp)
{
    void *p = TlsGetValue(m_tls);
    int errCode = GetLastError();
    if (!p && errCode != ERROR_SUCCESS)
        XGOTO(errCode, failed);
    *pp = p;
    return true;
failed:
    return false;
}


bool Tls::set(void *p)
{
    if (!TlsSetValue(m_tls, p))
        GOTO(failed);
    return true;
failed:
    return false;
}
