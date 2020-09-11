#include "util_netstream.h"
#include <stdint.h>
#include <string.h>

namespace util {

CNetStreamMaker::CNetStreamMaker()
    : m_pData(nullptr)
    , m_nCurrent(0)
    , m_nMax(0)
{

}

CNetStreamMaker::~CNetStreamMaker()
{
    if (nullptr != m_pData)
    {
        free(m_pData);
    }
}

bool CNetStreamMaker::append_data(char* data, uint32_t size )
{
    unsigned ns = m_nCurrent + size;

    if( ns > m_nMax )
    {
        void *dp;
        unsigned dn = 16;
        
        //Log::debug("ns:%d, dn:%d\n",ns,dn);
        //Log::debug("before while\n");        
        while( ns > dn )
        {
            dn <<= 1;
            //Log::debug("ns:%d, dn:%d\n",ns,dn);
        }
		//Log::debug("after while\n");
        dp = realloc( m_pData, dn );
        if( !dp )
        {
            //Log::error("realloc failed");
            return false;
        }

        m_pData = (char*)dp;
        m_nMax = dn;
    }

    memcpy( m_pData + m_nCurrent, data, size );

    m_nCurrent = ns;

    return true;
}

void CNetStreamMaker::append_string(const char *str)
{
    while( *str )
        append_byte( *str++ );
}

void CNetStreamMaker::append_byte(uint8_t b)
{
    append_data((char*)&b, 1 );
}

void CNetStreamMaker::append_be16(uint16_t val)
{
    append_byte( val >> 8 );
    append_byte( val );
}

void CNetStreamMaker::append_be24(uint32_t val)
{
    append_be16( val >> 8 );
    append_byte( val );
}

void CNetStreamMaker::append_be32(uint32_t val)
{
    append_byte( val >> 24 );
    append_byte( val >> 16 );
    append_byte( val >> 8 );
    append_byte( val );
}

void CNetStreamMaker::append_be64(uint64_t val)
{
    append_be32( val >> 32 );
    append_be32( val );
}

void CNetStreamMaker::append_bytes(uint8_t val, uint32_t num)
{
    for (uint32_t i=0; i<num; i++)
    {
        append_byte(val);
    }
}

void CNetStreamMaker::append_double(double val)
{
    append_be64( dbl2int( val ) );
}

void CNetStreamMaker::rewrite_data(uint32_t start, char* data, uint32_t size)
{
    uint32_t ns = start + size;

    if( ns > m_nMax ) {
        void *dp;
        uint32_t dn = 16;
       
        while( ns > dn ) {
            dn <<= 1;
        }

        dp = realloc( m_pData, dn );
        if( !dp ) {
            return;
        }

        m_pData = (char*)dp;
        m_nMax = dn;
    }
    memcpy(m_pData + start, data, size);
}

void CNetStreamMaker::rewrite_byte(uint32_t start, uint8_t  val)
{
    *(m_pData + start) = val;
}

void CNetStreamMaker::rewrite_be16(uint32_t start, uint16_t val)
{
    *(m_pData + start + 0) = val >> 8;
    *(m_pData + start + 1) = val >> 0;
}

void CNetStreamMaker::rewrite_be24(uint32_t start, uint32_t val)
{
    *(m_pData + start + 0) = val >> 16;
    *(m_pData + start + 1) = val >> 8;
    *(m_pData + start + 2) = val >> 0;
}

void CNetStreamMaker::rewrite_be32(uint32_t start, uint32_t val)
{
    *(m_pData + start + 0) = val >> 24;
    *(m_pData + start + 1) = val >> 16;
    *(m_pData + start + 2) = val >> 8;
    *(m_pData + start + 3) = val >> 0;
}

void CNetStreamMaker::rewrite_be64(uint32_t start, uint64_t val)
{
    rewrite_be32(start, val >> 32);
    rewrite_be32(start + 4, val);
}

void CNetStreamMaker::rewrite_double(uint32_t start, double val)
{
    rewrite_be64(start, dbl2int(val));
}

uint64_t CNetStreamMaker::dbl2int( double value )
{
    union UNIONTYPE{
        double f; 
        uint64_t i;
    };
    UNIONTYPE tmp;
    tmp.f = value;
    return tmp.i;
}

//////////////////////////////////////////////////////////////////////////

CNetStreamParser::CNetStreamParser(char* buff, uint32_t len)
    : pData(buff)
    , nCurrent(0)
    , nLen(len)
{
}

CNetStreamParser::~CNetStreamParser()
{
}

uint8_t CNetStreamParser::read_byte(uint8_t bitCount)
{
    uint8_t dwRet = 0;
    uint8_t i=0;
    for (; i<bitCount; i++)
    {
        dwRet <<= 1;
        if (pData[nCurrent / 8] & (0x80 >> (nCurrent % 8)))
        {
            dwRet += 1;
        }
        nCurrent++;
    }
    return dwRet;
}

uint16_t CNetStreamParser::read_be16(uint8_t bitCount)
{
    uint16_t dwRet = 0;
    uint8_t i=0;
    for (; i<bitCount; i++)
    {
        dwRet <<= 1;
        if (pData[nCurrent / 8] & (0x80 >> (nCurrent % 8)))
        {
            dwRet += 1;
        }
        nCurrent++;
    }
    return dwRet;
}

uint32_t CNetStreamParser::read_be32(uint8_t bitCount)
{
    uint32_t dwRet = 0;
    uint8_t i=0;
    for (; i<bitCount; i++)
    {
        dwRet <<= 1;
        if (pData[nCurrent / 8] & (0x80 >> (nCurrent % 8)))
        {
            dwRet += 1;
        }
        nCurrent++;
    }
    return dwRet;
}

uint64_t CNetStreamParser::read_be64(uint8_t bitCount)
{
    uint64_t dwRet = 0;
    uint8_t i=0;
    for (; i<bitCount; i++)
    {
        dwRet <<= 1;
        if (pData[nCurrent / 8] & (0x80 >> (nCurrent % 8)))
        {
            dwRet += 1;
        }
        nCurrent++;
    }
    return dwRet;
}

char* CNetStreamParser::read_buff(uint32_t len)
{
    char* ret = pData + (nCurrent / 8);
    nCurrent += len*8;
    return ret;
}

void CNetStreamParser::skip(uint32_t bitCount)
{
    nCurrent += bitCount;
}

}