#pragma once

#include "AbstractFile.h"

class File : public AbstractFile
{
public:
    explicit File(HANDLE f = INVALID_HANDLE_VALUE);

    bool createWatchedDir(const char *dir) override;
    bool createOnly(const char *fileName, RW rw) override;
    bool create(const char *fileName, RW rw) override;
    bool open(const char *fileName, RW rw) override;
    bool truncate(const char *fileName) override;
    bool append(const char *fileName) override;

    bool watchDir(void *buf, int len, int *rlen, bool watchSubtree, DWORD dwNotifyFilter) override;
    bool read(void *buf, int len, int *rlen) override;
    bool write(const void *buf, int len, int *rlen) override;
};

