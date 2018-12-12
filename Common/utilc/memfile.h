#ifndef _UTIL_MEMFILE_
#define _UTIL_MEMFILE_

#ifdef __cplusplus
extern "C" {
#endif

#include "util_export.h"
#include "util_api.h"
#include "util_def.h"

/** 内存文件数据结构 */
typedef struct _memfile_
{
    char*   _buffer;      //< 缓存区域指针
    size_t  _bufLen;      //< 缓存区域大小

    size_t  _readPos;     //< 读指针位置
    size_t  _writePos;    //< 写指针位置
    size_t  _fileSize;    //< 文件大小

    size_t  _maxSize;     //< 缓存区最大大小
    size_t  _memInc;      //< 每次申请内存的大小
    bool    _selfAlloc;   //< 缓存区是否是内部申请
}memfile_t;

_UTIL_API memfile_t* create_memfile(size_t memInc, size_t maxSize);
_UTIL_API memfile_t* create_memfile_sz(void* buf, size_t len);
_UTIL_API void       destory_memfile(memfile_t* mf);
_UTIL_API void       mf_trunc(memfile_t* mf, bool freeBuf);
_UTIL_API bool       mf_reserve(memfile_t* mf, size_t r, void **buf, size_t *len);
_UTIL_API size_t     mf_write(memfile_t* mf, const void *buf, size_t len);
_UTIL_API size_t     mf_putc(memfile_t* mf, const char ch);
_UTIL_API size_t     mf_puts(memfile_t* mf, const char *buf);
_UTIL_API size_t     mf_tellp(memfile_t* mf);
_UTIL_API size_t     mf_read(memfile_t* mf, void *buf, size_t size);
_UTIL_API char       mf_getc(memfile_t* mf);
_UTIL_API size_t     mf_gets(memfile_t* mf, char *buf, size_t size);
_UTIL_API size_t     mf_seekg(memfile_t* mf, long offset, int origin);
_UTIL_API size_t     mf_seekp(memfile_t* mf, long offset, int origin);
_UTIL_API size_t     mf_tellg(memfile_t* mf);
_UTIL_API void*      mf_buffer(memfile_t* mf);
_UTIL_API bool       mf_eof(memfile_t* mf);


#ifdef __cplusplus
}
#endif
#endif