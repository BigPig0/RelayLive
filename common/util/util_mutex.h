#pragma once
#include "util_public.h"
#include <mutex>
#include <condition_variable>
typedef void * HANDLE;

namespace util {
////////////////////////////////////////////////////////////////////////////////
class UTIL_API Lock
{
public:
    virtual void lock() = 0;
    virtual void unlock() = 0;
};

class UTIL_API MutexLock
{
public:
    explicit MutexLock(Lock *lock);
    ~MutexLock();

private:
    Lock *m_lock;
};

class UTIL_API CriticalSection : public Lock
{
public:
    explicit CriticalSection();
    ~CriticalSection();

    void lock() override;
    void unlock() override;

private:
#ifdef WINDOWS_IMPL
    void* m_cs;
#else
    std::mutex m_cs;
#endif
    friend class Cond;
};

class UTIL_API SlimRWLock : public Lock
{
public:
    explicit SlimRWLock();
    ~SlimRWLock();

    void lock() override;
    void unlock() override;
    void sharedLock();
    void sharedUnlock();

private:
#ifdef WINDOWS_IMPL
    void* m_srw;
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

class UTIL_API MutexShareLock
{
public:
    explicit MutexShareLock(SlimRWLock *lock);
    ~MutexShareLock();

private:
    SlimRWLock *m_lock;
};

#ifdef WINDOWS_IMPL
class UTIL_API Mutex : public Lock
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

class UTIL_API Cond
{
public:
    explicit Cond();

    void wake();
    void wakeAll();
    bool wait(bool *wakeUpCondition, CriticalSection *cs);
    bool wait(bool *wakeUpCondition, SlimRWLock *srw, bool shared = false);

private:
#ifdef WINDOWS_IMPL
    void* m_cond;
#else
    std::condition_variable m_cond;
#endif
};

#ifdef WINDOWS_IMPL
class UTIL_API SharedMemory
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
};