#include "stdafx.h"
#include "AbstractFile.h"
#include <stdio.h>


AbstractFile::AbstractFile(HANDLE f) : m_handle(f)
{
}


AbstractFile::~AbstractFile()
{
    close();
}


bool AbstractFile::findAll(const char *dir, const char *pat, FindRunnable *runnable)
{
    char pathName[MAX_PATH];
    sprintf_s(pathName, MAX_PATH, "%s\\%s", dir, pat);
    WIN32_FIND_DATA findData;
    HANDLE handle = FindFirstFile(pathName, &findData);
    if (handle == INVALID_HANDLE_VALUE)
        GOTO(failed);

    do
    {
        if (strcmp(findData.cFileName, ".") == 0
            || strcmp(findData.cFileName, "..") == 0)
        {
            continue;
        }

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            char subdir[MAX_PATH];
            sprintf_s(subdir, MAX_PATH, "%s\\%s", dir, findData.cFileName);
            if (!findAll(subdir, pat, runnable))
                GOTO(failed);
        }
        else
        {
            char fileName[MAX_PATH];
            sprintf_s(fileName, MAX_PATH, "%s\\%s", dir, findData.cFileName);
            runnable->find(fileName);
        }
    }while (FindNextFile(handle, &findData));
    FindClose(handle);
    return true;
failed:
    if (handle != INVALID_HANDLE_VALUE)
        FindClose(handle);
    return false;
}


void AbstractFile::setWatchRunnable(WatchRunnable *runnable)
{
    m_runnable = runnable;
}


void AbstractFile::close()
{
    if (m_handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}


bool AbstractFile::flush()
{
    return FlushFileBuffers(m_handle) == TRUE;
}