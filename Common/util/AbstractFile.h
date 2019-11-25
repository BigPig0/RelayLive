#pragma once
#include "LastError.h"
#include <windows.h>

////////////////////////////////////////////////////////////////////////////////
class  WatchRunnable
{
public:
    virtual ~WatchRunnable()
    {}
    virtual void run(DWORD action, const char *fileName) = 0;
};

class FindRunnable
{
public:
    virtual ~FindRunnable()
    {}
    virtual void find(const char *fileName) = 0;
};
////////////////////////////////////////////////////////////////////////////////
class AbstractFile : public LastError
{
public:
    explicit AbstractFile(HANDLE f = INVALID_HANDLE_VALUE);
    virtual ~AbstractFile();

    bool findAll(const char *dir, const char *pat, FindRunnable *runnable);

    enum class RW
    {
        read    = 0x01,
        write   = 0x02,
        both    = 0x03
    };

    virtual bool createWatchedDir(const char *dir) = 0;
    virtual bool createOnly(const char *fileName, RW rw) = 0;
    virtual bool create(const char *fileName, RW rw) = 0;
    virtual bool open(const char *fileName, RW rw) = 0;
    virtual bool truncate(const char *fileName) = 0;
    virtual bool append(const char *fileName) = 0;

    void setWatchRunnable(WatchRunnable *runnable);
    virtual bool watchDir(void *buf, int len, int *rlen, bool watchSubtree, DWORD dwNotifyFilter) = 0;
    virtual bool read(void *buf, int len, int *rlen) = 0;
    virtual bool write(const void *buf, int len, int *rlen) = 0;

    void close();
    bool flush();

protected:
    HANDLE m_handle;
    WatchRunnable *m_runnable;
};