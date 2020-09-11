#include "util_mutex.h"
#ifdef WINDOWS_IMPL
#include <windows.h>
#include <stdio.h>
#endif

namespace util {
////////////////////////////////////////////////////////////////////////////////
MutexLock::MutexLock(Lock *lock) : m_lock(lock)
{
    m_lock->lock();
}

MutexLock::~MutexLock()
{
    m_lock->unlock();
}

////////////////////////////////////////////////////////////////////////////////
MutexShareLock::MutexShareLock(SlimRWLock *lock) : m_lock(lock)
{
    m_lock->sharedLock();
}

MutexShareLock::~MutexShareLock()
{
    m_lock->sharedUnlock();
}

////////////////////////////////////////////////////////////////////////////////
CriticalSection::CriticalSection()
{
#ifdef WINDOWS_IMPL
    m_cs = new CRITICAL_SECTION;
    InitializeCriticalSection((CRITICAL_SECTION*)m_cs);
#endif
}

CriticalSection::~CriticalSection()
{
#ifdef WINDOWS_IMPL
    DeleteCriticalSection((CRITICAL_SECTION*)m_cs);
    delete m_cs;
#endif
}

void CriticalSection::lock()
{
#ifdef WINDOWS_IMPL
    EnterCriticalSection((CRITICAL_SECTION*)m_cs);
#else
    m_cs.lock();
#endif
}

void CriticalSection::unlock()
{
#ifdef WINDOWS_IMPL
    LeaveCriticalSection((CRITICAL_SECTION*)m_cs);
#else
    m_cs.unlock();
#endif
}

////////////////////////////////////////////////////////////////////////////////
SlimRWLock::SlimRWLock()
{
#ifdef WINDOWS_IMPL
    m_srw = new SRWLOCK;
    InitializeSRWLock((SRWLOCK*)m_srw);
#else
    read_cnt    = 0;
    write_cnt   = 0;
    inwriteflag = false;
#endif
}

SlimRWLock::~SlimRWLock()
{
#ifdef WINDOWS_IMPL
    delete m_srw;
#endif
}

void SlimRWLock::lock()
{
#ifdef WINDOWS_IMPL
    AcquireSRWLockExclusive((SRWLOCK*)m_srw);
#else
    std::unique_lock<std::mutex> ulk(counter_mutex);  
    ++write_cnt;  
    cond_w.wait(ulk, [=]()->bool {return read_cnt == 0 && !inwriteflag; });  
    inwriteflag = true;  
#endif
}

void SlimRWLock::unlock()
{
#ifdef WINDOWS_IMPL
    ReleaseSRWLockExclusive((SRWLOCK*)m_srw);
#else
    std::unique_lock<std::mutex> ulk(counter_mutex);  
    if (--write_cnt == 0)  
    {  
        cond_r.notify_all();  
    }  
    else  
    {  
        cond_w.notify_one();  
    }  
    inwriteflag = false;  
#endif
}

void SlimRWLock::sharedLock()
{
#ifdef WINDOWS_IMPL
    AcquireSRWLockShared((SRWLOCK*)m_srw);
#else
    std::unique_lock<std::mutex> ulk(counter_mutex);  
    cond_r.wait(ulk, [=]()->bool {return write_cnt == 0; });  
    ++read_cnt;  
#endif
}

void SlimRWLock::sharedUnlock()
{
#ifdef WINDOWS_IMPL
    ReleaseSRWLockShared((SRWLOCK*)m_srw);
#else
    std::unique_lock<std::mutex> ulk(counter_mutex);  
    if (--read_cnt == 0 && write_cnt > 0)  
    {  
        cond_w.notify_one();  
    }  
#endif
}

////////////////////////////////////////////////////////////////////////////////
#ifdef WINDOWS_IMPL

Mutex::Mutex(int i)
{
    char name[16];
    sprintf_s(name, 16, "Mutex%d", i);
    m_mutex = CreateMutex(NULL, TRUE, name);
}

Mutex::~Mutex()
{
    if (m_mutex)
        CloseHandle(m_mutex);
}

bool Mutex::exists()
{
    return GetLastError() == ERROR_ALREADY_EXISTS;
}

void Mutex::lock()
{
    WaitForSingleObject(m_mutex, INFINITE);
}

void Mutex::unlock()
{
    ReleaseMutex(m_mutex);
}
#endif

////////////////////////////////////////////////////////////////////////////////
Cond::Cond()
{
#ifdef WINDOWS_IMPL
    m_cond = new CONDITION_VARIABLE;
    InitializeConditionVariable((CONDITION_VARIABLE*)m_cond);
#endif
}

void Cond::wake()
{
#ifdef WINDOWS_IMPL
    WakeConditionVariable((CONDITION_VARIABLE*)m_cond);
#else
    m_cond.notify_one();
#endif
}

void Cond::wakeAll()
{
#ifdef WINDOWS_IMPL
    WakeAllConditionVariable((CONDITION_VARIABLE*)m_cond);
#else
    m_cond.notify_all();
#endif
}

bool Cond::wait(bool *wakeUpCondition, CriticalSection *cs)
{
#ifdef WINDOWS_IMPL
    while (!*wakeUpCondition)
    {
        if (!SleepConditionVariableCS((CONDITION_VARIABLE*)m_cond, (CRITICAL_SECTION*)cs->m_cs, INFINITE))
            return false;
    }
#else
    std::unique_lock<std::mutex> ulk(cs->m_cs);  
    m_cond.wait(ulk,[=]()->bool {return *wakeUpCondition; });
#endif
    return true;
}

bool Cond::wait(bool *wakeUpCondition, SlimRWLock *srw, bool shared)
{
#ifdef WINDOWS_IMPL
    while (!*wakeUpCondition)
    {
        if (!SleepConditionVariableSRW((CONDITION_VARIABLE*)m_cond, (SRWLOCK*)srw->m_srw, INFINITE, shared ? CONDITION_VARIABLE_LOCKMODE_SHARED : 0))
            return false;
    }
#else
    std::unique_lock<std::mutex> ulk(srw->counter_mutex);  
    m_cond.wait(ulk,[=]()->bool {return *wakeUpCondition; });
#endif
    return true;
}

////////////////////////////////////////////////////////////////////////////////
#ifdef WINDOWS_IMPL
SharedMemory::SharedMemory()
    : m_file(INVALID_HANDLE_VALUE)
    , m_shm(NULL)
    , m_a(NULL)
{}

SharedMemory::~SharedMemory()
{
    if (m_file != INVALID_HANDLE_VALUE)
        CloseHandle(m_file);
    if (m_a)
        UnmapViewOfFile(m_a);
    if (m_shm)
        CloseHandle(m_shm);
}

void *SharedMemory::create(const char *fileName, int len, int i)
{
    m_file = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_file == INVALID_HANDLE_VALUE)
        return NULL;

    char name[32];
    sprintf_s(name, 32, "SharedMemory%d", i);
    m_shm = CreateFileMapping(m_file, NULL, PAGE_READWRITE, 0, len, name);
    if (!m_shm)
        return NULL;

    m_a = MapViewOfFile(m_shm, FILE_MAP_ALL_ACCESS, 0, 0, len);
    if (!m_a)
        return NULL;
    return m_a;
}

bool SharedMemory::exists()
{
    return GetLastError() == ERROR_ALREADY_EXISTS;
}

void *SharedMemory::addr()
{
    return m_a;
}
#endif

}