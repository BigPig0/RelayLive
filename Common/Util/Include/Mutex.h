#pragma once
#include "LastError.h"
#ifdef WIN32
#include <windows.h>
#else
#include <mutex>
#include <condition_variable>
#endif

////////////////////////////////////////////////////////////////////////////////
class Lock : public LastError
{
public:
    virtual void lock() = 0;
    virtual void unlock() = 0;
};

class MutexLock
{
public:
    explicit MutexLock(Lock *lock);
    ~MutexLock();

private:
    Lock *m_lock;
};

class CriticalSection : public Lock
{
public:
    explicit CriticalSection();
    ~CriticalSection();

    void lock() override;
    void unlock() override;

private:
#ifdef WIN32
    CRITICAL_SECTION m_cs;
#else
    std::mutex m_cs;
#endif
    friend class Cond;
};

class SlimRWLock : public Lock
{
public:
    explicit SlimRWLock();

    void lock() override;
    void unlock() override;
    void sharedLock();
    void sharedUnlock();

private:
#ifdef WIN32
    SRWLOCK m_srw;
#else
    volatile size_t read_cnt;  
    volatile size_t write_cnt;  
    volatile bool inwriteflag;  
    std::mutex counter_mutex;  
    std::condition_variable cond_w;  
    std::condition_variable cond_r;  
#endif
    friend class Cond;
};

class MutexShareLock
{
public:
    explicit MutexShareLock(SlimRWLock *lock);
    ~MutexShareLock();

private:
    SlimRWLock *m_lock;
};

#ifdef WIN32
class Mutex : public Lock
{
public:
    explicit Mutex(int i);
    ~Mutex();

    bool exists();
    void lock() override;
    void unlock() override;

private:
    HANDLE m_mutex;
};
#endif

class Cond : public LastError
{
public:
    explicit Cond();

    void wake();
    void wakeAll();
    bool wait(bool *wakeUpCondition, CriticalSection *cs);
    bool wait(bool *wakeUpCondition, SlimRWLock *srw, bool shared = false);

private:
#ifdef WIN32
    CONDITION_VARIABLE m_cond;
#else
    std::condition_variable m_cond;
#endif
};

#ifdef WIN32
class SharedMemory : public LastError
{
public:
    explicit SharedMemory();
    ~SharedMemory();

    void *create(const char *fileName, int len, int i);
    bool exists();
    void *addr();

private:
    HANDLE m_file, m_shm;
    void *m_a;
};
#endif

////////////////////////////////////////////////////////////////////////////////
inline
MutexLock::MutexLock(Lock *lock) : m_lock(lock)
{
    m_lock->lock();
}

inline
MutexLock::~MutexLock()
{
    m_lock->unlock();
}

////////////////////////////////////////////////////////////////////////////////
inline
MutexShareLock::MutexShareLock(SlimRWLock *lock) : m_lock(lock)
{
    m_lock->sharedLock();
}

inline
MutexShareLock::~MutexShareLock()
{
    m_lock->sharedUnlock();
}

////////////////////////////////////////////////////////////////////////////////
inline
CriticalSection::CriticalSection()
{
#ifdef WIN32
    InitializeCriticalSection(&m_cs);
#endif
}

inline
CriticalSection::~CriticalSection()
{
#ifdef WIN32
    DeleteCriticalSection(&m_cs);
#endif
}

inline
void CriticalSection::lock()
{
#ifdef WIN32
    EnterCriticalSection(&m_cs);
#else
    m_cs.lock();
#endif
}

inline
void CriticalSection::unlock()
{
#ifdef WIN32
    LeaveCriticalSection(&m_cs);
#else
    m_cs.unlock();
#endif
}

////////////////////////////////////////////////////////////////////////////////
inline
SlimRWLock::SlimRWLock()
{
#ifdef WIN32
    InitializeSRWLock(&m_srw);
#else
    read_cnt    = 0;
    write_cnt   = 0;
    inwriteflag = false;
#endif
}

inline
void SlimRWLock::lock()
{
#ifdef WIN32
    AcquireSRWLockExclusive(&m_srw);
#else
    std::unique_lock<std::mutex> ulk(counter_mutex);  
    ++write_cnt;  
    cond_w.wait(ulk, [=]()->bool {return read_cnt == 0 && !inwriteflag; });  
    inwriteflag = true;  
#endif
}

inline
void SlimRWLock::unlock()
{
#ifdef WIN32
    ReleaseSRWLockExclusive(&m_srw);
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

inline
void SlimRWLock::sharedLock()
{
#ifdef WIN32
    AcquireSRWLockShared(&m_srw);
#else
    std::unique_lock<std::mutex> ulk(counter_mutex);  
    cond_r.wait(ulk, [=]()->bool {return write_cnt == 0; });  
    ++read_cnt;  
#endif
}

inline
void SlimRWLock::sharedUnlock()
{
#ifdef WIN32
    ReleaseSRWLockShared(&m_srw);
#else
    std::unique_lock<std::mutex> ulk(counter_mutex);  
    if (--read_cnt == 0 && write_cnt > 0)  
    {  
        cond_w.notify_one();  
    }  
#endif
}

////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include <stdio.h>
inline
Mutex::Mutex(int i)
{
    char name[16];
    sprintf_s(name, 16, "Mutex%d", i);
    m_mutex = CreateMutex(NULL, TRUE, name);
    m_errCode = GetLastError();
}

inline
Mutex::~Mutex()
{
    if (m_mutex)
        CloseHandle(m_mutex);
}

inline
bool Mutex::exists()
{
    return m_errCode == ERROR_ALREADY_EXISTS;
}

inline
void Mutex::lock()
{
    if (WaitForSingleObject(m_mutex, INFINITE) != WAIT_OBJECT_0)
        GOTO(failed);
failed:
    ;
}

inline
void Mutex::unlock()
{
    if (!ReleaseMutex(m_mutex))
        GOTO(failed);
failed:
    ;
}
#endif

////////////////////////////////////////////////////////////////////////////////
inline
Cond::Cond()
{
#ifdef WIN32
    InitializeConditionVariable(&m_cond);
#endif
}

inline
void Cond::wake()
{
#ifdef WIN32
    WakeConditionVariable(&m_cond);
#else
    m_cond.notify_one();
#endif
}

inline
void Cond::wakeAll()
{
#ifdef WIN32
    WakeAllConditionVariable(&m_cond);
#else
    m_cond.notify_all();
#endif
}

inline
bool Cond::wait(bool *wakeUpCondition, CriticalSection *cs)
{
#ifdef WIN32
    while (!*wakeUpCondition)
    {
        if (!SleepConditionVariableCS(&m_cond, &cs->m_cs, INFINITE))
            GOTO(failed);
    }
#else
    std::unique_lock<std::mutex> ulk(cs->m_cs);  
    m_cond.wait(ulk,[=]()->bool {return *wakeUpCondition; });
#endif
    return true;
failed:
    return false;
}

inline
bool Cond::wait(bool *wakeUpCondition, SlimRWLock *srw, bool shared)
{
#ifdef WIN32
    while (!*wakeUpCondition)
    {
        if (!SleepConditionVariableSRW(&m_cond, &srw->m_srw, INFINITE, shared ? CONDITION_VARIABLE_LOCKMODE_SHARED : 0))
            GOTO(failed);
    }
#else
    std::unique_lock<std::mutex> ulk(srw->counter_mutex);  
    m_cond.wait(ulk,[=]()->bool {return *wakeUpCondition; });
#endif
    return true;
failed:
    return false;
}

////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
inline
SharedMemory::SharedMemory()
    : m_file(INVALID_HANDLE_VALUE), m_shm(NULL), m_a(NULL)
{}

inline
SharedMemory::~SharedMemory()
{
    if (m_file != INVALID_HANDLE_VALUE)
        CloseHandle(m_file);
    if (m_a)
        UnmapViewOfFile(m_a);
    if (m_shm)
        CloseHandle(m_shm);
}

inline
void *SharedMemory::create(const char *fileName, int len, int i)
{
    m_file = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_file == INVALID_HANDLE_VALUE)
        GOTO(failed);

    char name[32];
    sprintf_s(name, 32, "SharedMemory%d", i);
    m_shm = CreateFileMapping(m_file, NULL, PAGE_READWRITE, 0, len, name);
    if (!m_shm)
        GOTO(failed);

    m_errCode = GetLastError();
    m_a = MapViewOfFile(m_shm, FILE_MAP_ALL_ACCESS, 0, 0, len);
    if (!m_a)
        GOTO(failed);
    return m_a;
failed:
    return NULL;
}

inline
bool SharedMemory::exists()
{
    return m_errCode == ERROR_ALREADY_EXISTS;
}

inline
void *SharedMemory::addr()
{
    return m_a;
}
#endif