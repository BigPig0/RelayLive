#include "stdafx.h"
#include "IMemoryPool.h"
#include <stdlib.h>

IMemoryPool::IMemoryPool(void)
    : m_pMemoryMapTable(nullptr)
    , m_pChunkPool(nullptr)
    , m_pMemory(nullptr)
    , m_nUsedSize(0)
    , m_nUsedCount(0)
    , m_nUnitCount(0)
    , m_nUnitSize(0)
    , m_bInit(false)
{
}


IMemoryPool::~IMemoryPool(void)
{
}

bool IMemoryPool::init(int nUnitCount, int nUnitSize)
{
    if(m_bInit)
    {
        Log::error("MemoryPool is already init");
        return false;
    }

    m_nUnitCount = nUnitCount;
    m_nUnitSize = nUnitSize;

    // memory map table 所占空间大小
    size_t nMemMapSize = sizeof(memory_block) * m_nUnitCount;
    // memory chunk pool 所占空间大小
    size_t nChunkPoolSize = get_chunk_pool_size();
    // 可用内存memory所占空间大小
    size_t nMemorySize = m_nUnitSize * m_nUnitCount;
    Log_Debug("memory pool init nMemMapSize:%d,nChunkPoolSize:%d,nMemorySize:%d",nMemMapSize,nChunkPoolSize,nMemorySize);

    char* pBegin = (char*)malloc(nMemMapSize + nChunkPoolSize + nMemorySize);
    if(nullptr == pBegin)
    {
        Log::error("MemoryPool malloc failed");
        return false;
    }

    m_pMemoryMapTable = (memory_block*)pBegin;
    m_pChunkPool      = (void*)(pBegin + nMemMapSize);
    m_pMemory         = (void*)(pBegin + nMemMapSize + nChunkPoolSize);
    Log_Debug("memory pool init m_pMemoryMapTable:%d, m_pChunkPool:%d, m_pMemory:%d,memoryend:%d",m_pMemoryMapTable,m_pChunkPool,m_pMemory,(int)m_pMemory+nMemorySize-m_nUnitSize);
    return true;
}

void IMemoryPool::mp_copy(void* p,void* src,size_t nSize)
{
    if (nullptr == p || nullptr == src)
    {
        return;
    }

    MutexLock  lock(&m_cs);
    //memcpy(p,src,nSize);
    char* cp = (char*)p;
    char* cb = (char*)m_pMemory;
    int index = (cp-cb)/m_nUnitSize;
    Log_Debug("copy p:%d,nSize:%d, index:%d,count:%d,srccount:%d",p,nSize,index,m_pMemoryMapTable[index].count,(nSize+m_nUnitSize-1)/m_nUnitSize);
    memcpy_s(p,nSize,src,nSize);

}