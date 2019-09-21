/**
 * 该文件定义大端小端转换的方法
 */
#pragma once
#include <stdint.h>

namespace Util
{
static inline char* EndianChange(char* src, int bytes);
static inline uint16_t EndianChange16(uint16_t src);
static inline uint32_t EndianChange32(uint32_t src);
static inline uint64_t EndianChange64(uint64_t src);
static inline bool IsLittleEndian();

/**
 * 大码小码转换，就是按字节头尾颠倒位置
 */
static inline char* EndianChange(char* src, int bytes)
{
    int nCount = bytes/2;
    char c;
    for (int i=0; i<nCount; ++i)
    {
        c = src[i];
        src[i] = src[bytes-1-i];
        src[bytes-1-i] = c;
    }
    return src;
}

static inline uint16_t EndianChange16(uint16_t src)
{
    char* p = EndianChange((char*)&src, 2);
    return *(uint16_t*)p;
}

static inline uint32_t EndianChange32(uint32_t src)
{
    char* p = EndianChange((char*)&src, 4);
    return *(uint32_t*)p;
}

static inline uint64_t EndianChange64(uint64_t src)
{
    char* p = EndianChange((char*)&src, 8);
    return *(uint64_t*)p;
}

/** 动态判断本机是否是小端编码 */
static inline bool IsLittleEndian()
{
     int i=1;  
     return (*(char *)&i == 1); 
}
};