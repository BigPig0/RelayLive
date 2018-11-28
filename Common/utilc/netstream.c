/*!
 * \file netstream.cpp
 * \date 2018/11/23 10:22
 *
 * \author wlla
 * Contact: user@company.com
 *
 * \brief 
 *
 * TODO: long description
 *
 * \note
*/

#include "netstream.h"

static uint64_t double2int( double value )
{
    return *(uint64_t*)(&value);
}

typedef struct _net_stream_maker_ {
    char*    pData;      //内存地址
    uint32_t nCurrent;   //当前写入数据的位置
    uint32_t nMax;       //内存大小
} net_stream_maker_t;

net_stream_maker_t* create_net_stream_maker() {
    SAFE_MALLOC(net_stream_maker_t, r);
    return r;
}

void destory_net_stream_maker(net_stream_maker_t* h) {
    SAFE_FREE(h->pData);
    SAFE_FREE(h);
}

char* get_net_stream_data(net_stream_maker_t* h) {
    return h->pData;
}

uint32_t get_net_stream_len(net_stream_maker_t* h) {
    return h->nCurrent;
}

void clear_net_stream(net_stream_maker_t* h) {
    h->nCurrent = 0;
}

bool net_stream_append_data(net_stream_maker_t* h, char* data, uint32_t size )
{
    unsigned ns = h->nCurrent + size;

    if( ns > h->nMax )
    {
        void *dp;
        unsigned dn = 16;
        
        while( ns > dn )
        {
            dn <<= 1;
        }
        dp = realloc( h->pData, dn );
        if( !dp )
        {
            return false;
        }

        h->pData = (char*)dp;
        h->nMax = dn;
    }

    memcpy( h->pData + h->nCurrent, data, size );

    h->nCurrent = ns;

    return true;
}

void net_stream_append_byte(net_stream_maker_t* h, uint8_t b)
{
    net_stream_append_data(h, (char*)&b, 1 );
}

void net_stream_append_string(net_stream_maker_t* h, const char *str)
{
    while( *str )
        net_stream_append_byte(h, *str++ );
}

void net_stream_append_be16(net_stream_maker_t* h, uint16_t val)
{
    net_stream_append_byte(h, val >> 8 );
    net_stream_append_byte(h, val );
}

void net_stream_append_be24(net_stream_maker_t* h, uint32_t val)
{
    net_stream_append_be16(h, val >> 8 );
    net_stream_append_byte(h, val );
}

void net_stream_append_be32(net_stream_maker_t* h, uint32_t val)
{
    net_stream_append_byte(h, val >> 24 );
    net_stream_append_byte(h, val >> 16 );
    net_stream_append_byte(h, val >> 8 );
    net_stream_append_byte(h, val );
}

void net_stream_append_be64(net_stream_maker_t* h, uint64_t val)
{
    net_stream_append_be32(h, val >> 32 );
    net_stream_append_be32(h, val );
}

void net_stream_append_bytes(net_stream_maker_t* h, uint8_t val, uint32_t num)
{
    int i;
    for (i=0; i<num; i++)
    {
        net_stream_append_byte(h, val);
    }
}

void net_stream_append_double(net_stream_maker_t* h, double val)
{
    net_stream_append_be64(h, double2int( val ) );
}

void rewrite_byte(net_stream_maker_t* h, uint32_t start, uint8_t  val)
{
    *(h->pData + start) = val;
}

void rewrite_be16(net_stream_maker_t* h, uint32_t start, uint16_t val)
{
    *(h->pData + start + 0) = val >> 8;
    *(h->pData + start + 1) = val >> 0;
}

void rewrite_be24(net_stream_maker_t* h, uint32_t start, uint32_t val)
{
    *(h->pData + start + 0) = val >> 16;
    *(h->pData + start + 1) = val >> 8;
    *(h->pData + start + 2) = val >> 0;
}

void rewrite_be32(net_stream_maker_t* h, uint32_t start, uint32_t val)
{
    *(h->pData + start + 0) = val >> 24;
    *(h->pData + start + 1) = val >> 16;
    *(h->pData + start + 2) = val >> 8;
    *(h->pData + start + 3) = val >> 0;
}

void rewrite_be64(net_stream_maker_t* h, uint32_t start, uint64_t val)
{
    rewrite_be32(h, start, val >> 32);
    rewrite_be32(h, start + 4, val);
}

void rewrite_double(net_stream_maker_t* h, uint32_t start, double val)
{
    rewrite_be64(h, start, double2int(val));
}

//////////////////////////////////////////////////////////////////////////

typedef struct _net_stream_parser_ {
    char*    pData;      //内存地址
    uint32_t nCurrent;   //当前读取数据的位置(位)
    uint32_t nLen;       //数据长度
} net_stream_parser_t;

net_stream_parser_t* create_net_stream_parser(char* buff, uint32_t len) {
    SAFE_MALLOC(net_stream_parser_t, r);
    r->pData = buff;
    r->nLen = len;
    return r;
}

void destory_net_stream_parser(net_stream_parser_t* h) {
    SAFE_FREE(h);
}

uint8_t net_stream_read_byte(net_stream_parser_t* h, uint8_t bitCount)
{
    uint8_t dwRet = 0;
    uint8_t i=0;
    for (; i<bitCount; i++)
    {
        dwRet <<= 1;
        if (h->pData[h->nCurrent / 8] & (0x80 >> (h->nCurrent % 8)))
        {
            dwRet += 1;
        }
        h->nCurrent++;
    }
    return dwRet;
}

uint16_t net_stream_read_be16(net_stream_parser_t* h, uint8_t bitCount)
{
    uint16_t dwRet = 0;
    uint8_t i=0;
    for (; i<bitCount; i++)
    {
        dwRet <<= 1;
        if (h->pData[h->nCurrent / 8] & (0x80 >> (h->nCurrent % 8)))
        {
            dwRet += 1;
        }
        h->nCurrent++;
    }
    return dwRet;
}

uint32_t net_stream_read_be32(net_stream_parser_t* h, uint8_t bitCount)
{
    uint32_t dwRet = 0;
    uint8_t i=0;
    for (; i<bitCount; i++)
    {
        dwRet <<= 1;
        if (h->pData[h->nCurrent / 8] & (0x80 >> (h->nCurrent % 8)))
        {
            dwRet += 1;
        }
        h->nCurrent++;
    }
    return dwRet;
}

uint64_t net_stream_read_be64(net_stream_parser_t* h, uint8_t bitCount)
{
    uint64_t dwRet = 0;
    uint8_t i=0;
    for (; i<bitCount; i++)
    {
        dwRet <<= 1;
        if (h->pData[h->nCurrent / 8] & (0x80 >> (h->nCurrent % 8)))
        {
            dwRet += 1;
        }
        h->nCurrent++;
    }
    return dwRet;
}

char* net_stream_read_buff(net_stream_parser_t* h, uint32_t len)
{
    char* ret = h->pData + (h->nCurrent / 8);
    h->nCurrent += len*8;
    return ret;
}

void net_stream_skip(net_stream_parser_t* h, uint32_t bitCount)
{
    h->nCurrent += bitCount;
}