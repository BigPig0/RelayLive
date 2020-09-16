/**
 * ���ļ�������С��ת���ķ���
 */
#pragma once
#include <stdint.h>

namespace util {
namespace Endian {
static inline char* EndianChange(char* src, int bytes);
static inline uint16_t EndianChange16(uint16_t src);
static inline uint32_t EndianChange32(uint32_t src);
static inline uint64_t EndianChange64(uint64_t src);
static inline bool IsLittleEndian();

/**
 * ����С��ת�������ǰ��ֽ�ͷβ�ߵ�λ��
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

/** ��̬�жϱ����Ƿ���С�˱��� */
static inline bool IsLittleEndian()
{
     int i=1;  
     return (*(char *)&i == 1); 
}
};
};