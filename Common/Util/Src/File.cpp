#include "stdafx.h"
#include "File.h"
#include "Iconv.h"


File::File(HANDLE f) : AbstractFile(f)
{
}


bool File::createWatchedDir(const char *dir)
{
    m_handle = CreateFile(dir
                          , FILE_LIST_DIRECTORY
                          , FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE
                          , NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (m_handle == INVALID_HANDLE_VALUE)
        GOTO(failed);
    return true;
failed:
    return false;
}


bool File::createOnly(const char *fileName, RW rw)
{
    DWORD accessFlags = 0;
    if ((int)rw & (int)RW::read)
        accessFlags |= GENERIC_READ;
    if ((int)rw & (int)RW::write)
        accessFlags |= GENERIC_WRITE;
    m_handle = CreateFile(fileName, accessFlags, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_handle == INVALID_HANDLE_VALUE)
        GOTO(failed);
    return true;
failed:
    return false;
}


bool File::create(const char *fileName, RW rw)
{
    DWORD accessFlags = 0;
    if ((int)rw & (int)RW::read)
        accessFlags |= GENERIC_READ;
    if ((int)rw & (int)RW::write)
        accessFlags |= GENERIC_WRITE;
    m_handle = CreateFile(fileName, accessFlags, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_handle == INVALID_HANDLE_VALUE)
        GOTO(failed);
    return true;
failed:
    return false;
}


bool File::open(const char *fileName, RW rw)
{
    DWORD accessFlags = 0;
    if ((int)rw & (int)RW::read)
        accessFlags |= GENERIC_READ;
    if ((int)rw & (int)RW::write)
        accessFlags |= GENERIC_WRITE;
    m_handle = CreateFile(fileName, accessFlags, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_handle == INVALID_HANDLE_VALUE)
        GOTO(failed);
    return true;
failed:
    return false;
}


bool File::truncate(const char *fileName)
{
    m_handle = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_handle == INVALID_HANDLE_VALUE)
        GOTO(failed);
    return true;
failed:
    return false;
}


bool File::append(const char *fileName)
{
    m_handle = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_handle == INVALID_HANDLE_VALUE)
        GOTO(failed);
    if (SetFilePointer(m_handle, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
        GOTO(failed);
    return true;
failed:
    return false;
}


bool File::watchDir(void *buf, int len, int *rlen, bool watchSubtree, DWORD dwNotifyFilter)
{
    DWORD bytes;
    if (!ReadDirectoryChangesW(m_handle, buf, len, watchSubtree ? TRUE : FALSE, dwNotifyFilter
                               /*FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES
                               | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS
                               | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY*/, &bytes, NULL, NULL))
        GOTO(failed);
    if (rlen)
        *rlen = bytes;
    if (bytes == 0)
        return true;

    FILE_NOTIFY_INFORMATION *info = (FILE_NOTIFY_INFORMATION *)buf;
    DWORD offset;
    do
    {
        const char *fileName = Iconv::wtoc(info->FileName, (info->FileNameLength) / sizeof(WCHAR));
        m_runnable->run(info->Action, fileName);
        free((void *)fileName);
        offset = info->NextEntryOffset;
        info = (FILE_NOTIFY_INFORMATION *)((BYTE *)info + offset);
    }
    while (offset);
    return true;
failed:
    return false;
}


bool File::read(void *buf, int len, int *rlen)
{
    DWORD bytes;
    if (!ReadFile(m_handle, buf, len, &bytes, NULL))
        GOTO(failed);
    if (rlen)
        *rlen = bytes;
    return true;
failed:
    return false;
}


bool File::write(const void *buf, int len, int *rlen)
{
    DWORD bytes;
    if (!WriteFile(m_handle, buf, len, &bytes, NULL))
        GOTO(failed);
    if (rlen)
        *rlen = bytes;
    return true;
failed:
    return false;
}