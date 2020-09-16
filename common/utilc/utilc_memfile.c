#include "utilc_memfile.h"
#include <malloc.h>

#define SEEK_CUR    1
#define SEEK_END    2
#define SEEK_SET    0

static size_t space(memfile_t* mf)
{
    return mf->_bufLen - mf->_writePos;
}

static size_t reserve(memfile_t* mf, size_t s)
{
    size_t incSize, newSize;
    // �Ѿ�ӵ���㹻�Ŀռ�
    if( space(mf) >= s ) return s;

    // �ⲿ�ڴ治����������
    if( !mf->_selfAlloc ) return space(mf);

    // �ռ䲻��,�����µĿռ�

    // 1. һ�����ٷ��� _memInc ��С���ڴ�.
    incSize = s - space(mf);
    if( incSize < mf->_memInc ) incSize = mf->_memInc;

    // 2. ���ֻ���� _maxSize ��С���ڴ�.
    newSize = mf->_bufLen + incSize;
    if( newSize > mf->_maxSize ) newSize = mf->_maxSize;

    if( newSize <= mf->_bufLen )
    {
    }
    else
    {
        char *tmp = (char*)malloc(newSize);
        if(tmp)
        {
            memset(tmp, 0, newSize);
            if(mf->_buffer)
            {
                memcpy(tmp, mf->_buffer, mf->_bufLen);
                free(mf->_buffer);
            }
            mf->_buffer = tmp;
            mf->_bufLen = newSize;

            return space(mf) >= s ? s : space(mf);
        }
    }
    return space(mf);
}

memfile_t* create_memfile(size_t memInc, size_t maxSize) {
    memfile_t* mf = (memfile_t*)malloc(sizeof(memfile_t));
    memset(mf, 0, sizeof(memfile_t));
    mf->_selfAlloc = true;
    mf->_memInc = memInc > maxSize? maxSize : memInc;
    mf->_maxSize = maxSize;
    return mf;
}

memfile_t* create_memfile_sz(void* buf, size_t len) {
    memfile_t* mf = (memfile_t*)malloc(sizeof(memfile_t));
    memset(mf, 0, sizeof(memfile_t));
    mf->_selfAlloc = false;
    mf->_buffer = (char*)buf;
    mf->_fileSize = mf->_maxSize = mf->_bufLen = len;
    return mf;
}

void destory_memfile(memfile_t* mf) {
    mf_trunc(mf, true);
    free(mf);
}

void mf_trunc(memfile_t* mf, bool freeBuf)
{
    if( freeBuf && mf->_selfAlloc)
    {
        if(mf->_buffer != NULL) free(mf->_buffer);
        mf->_buffer = NULL;
        mf->_bufLen = 0;
    }

    mf->_readPos = 0;
    mf->_writePos = 0;
    mf->_fileSize = 0;
}

bool mf_reserve(memfile_t* mf, size_t r, void **buf, size_t *len)
{
    size_t sz = reserve(mf, r);
    if(len) *len = sz;
    if(buf) *buf = (void*)(mf->_buffer + mf->_writePos);
    return sz > 0;
}

size_t mf_write(memfile_t* mf, const void *buf, size_t len)
{
    // ����������д����ٸ��ֽ�
    size_t writeSize = reserve(mf, len);

    // д������
    if(writeSize > 0)
    {
        if(buf)
        {
            memcpy( mf->_buffer + mf->_writePos, buf, writeSize );
        }
        mf->_writePos += writeSize;

        // ��󵽴����λ��Ϊ�ļ���С.
        if(mf->_writePos > mf->_fileSize)
        {
            mf->_fileSize = mf->_writePos;
        }
    }

    return writeSize;
}

size_t mf_putc(memfile_t* mf, const char ch)
{
    return mf_write(mf, &ch, 1);
}

size_t mf_puts(memfile_t* mf, const char *buf)
{
    return mf_write(mf, buf, strlen(buf));
}

size_t mf_tellp(memfile_t* mf)
{
    return mf->_writePos;
}

size_t mf_read(memfile_t* mf, void *buf, size_t size)
{
    size_t realRead = mf->_fileSize - mf->_readPos;
    if( realRead > size ) realRead = size;
    if(buf == NULL || realRead <= 0) return 0;

    memcpy(buf, mf->_buffer + mf->_readPos, realRead);
    mf->_readPos += realRead;
    return realRead;
}

char mf_getc(memfile_t* mf)
{
    char ch = 0;
    mf_read(mf, &ch, 1);
    return ch;
}

size_t mf_gets(memfile_t* mf, char *buf, size_t size)
{
    size_t readed = 0;
    char ch = 0;
    while(readed < size)
    {
        ch = mf_getc(mf);
        buf[readed++] = ch;
        if( '\n' == ch ) break;
    }
    return readed;
}

size_t mf_seekg(memfile_t* mf, long offset, int origin)
{
    if( SEEK_CUR == origin )
    {
        mf->_readPos += offset;
    }
    else if( SEEK_END == origin )
    {
        mf->_readPos = mf->_fileSize - offset;
    }
    else // SEEK_SET
    {
        mf->_readPos = offset;
    }

    if( mf->_readPos > mf->_fileSize)
    {
        mf->_readPos = mf->_fileSize;
    }
    return mf->_readPos;
}

size_t mf_seekp(memfile_t* mf, long offset, int origin)
{
    if( SEEK_CUR == origin )
    {
        mf->_writePos += offset;
    }
    else if( SEEK_END == origin )
    {
        mf->_writePos = mf->_fileSize - offset;
    }
    else // SEEK_SET
    {
        mf->_writePos = offset;
    }

    if( mf->_writePos > mf->_fileSize )
    {
        mf->_writePos = mf->_fileSize;
    }

    if( mf->_writePos > mf->_bufLen )
    {
        mf->_writePos = mf->_bufLen;
        mf->_fileSize = mf->_bufLen;
    }
    return mf->_writePos;
}

size_t mf_tellg(memfile_t* mf)
{
    return mf->_readPos;
}

void* mf_buffer(memfile_t* mf)
{
    return mf->_buffer;
}

bool mf_eof(memfile_t* mf)
{
    return mf->_readPos >= mf->_fileSize;
}