#pragma once

#include "Mutex.h"
#include "ExportDefine.h"

#if 0
#define Log_Debug Log::debug
#else
#define Log_Debug
#endif

/** 内存映射表单元 */
struct memory_block  
{  
    size_t count;       //< 块的block个数，只对起始block有效
    size_t start;       //< 块的起始block索引，只对结束block有效
    void* pmem_chunk;   //< memory_chunk地址，只对起始block有效，为null表示被使用，不为null表示空闲
};
  
/**
 * 内存池接口
 */
class COMMON_API IMemoryPool
{
public:
    IMemoryPool(void);
    virtual ~IMemoryPool(void);

    /**
     * 初始化内存池
     * @param nUnitCount 内存单元个数
     * @param nUnitSize  内存单元大小
     * @return 成功true；失败false
     */
    virtual bool init(int nUnitCount, int nUnitSize);

    /**
     * 清理内存池
     */
    virtual void clear() = 0;

    /**
     * 申请内存
     */
    virtual void* mp_malloc(size_t nSize) = 0;

    /**
     * 释放内存
     */
    virtual void mp_free(void* p) = 0;

    virtual void mp_copy(void* p,void* drc,size_t nSize);

protected:

    /**
     * 获取memory chunk pool 的大小
     */
    virtual size_t get_chunk_pool_size() = 0;

    /**
     * 内存映射表中的索引转化为内存起始地址
     */
    void* index2addr(size_t index)  
    {
        char* p = (char*)m_pMemory;  
        void* ret = (void*)(p + index *m_nUnitSize);  

        Log_Debug("index2addr:%d--%d",index,ret);

        return ret;  
    }

    /**
     * 内存起始地址转化为内存映射表中的索引
     */
    size_t addr2index(void* addr)  
    {
        char* start = (char*)m_pMemory;  
        char* p = (char*)addr;  
        size_t index = (p - start) / m_nUnitSize;  
        Log_Debug("addr2index:%d--%d",index,addr);
        return index;  
    }

protected:
    size_t          m_nUnitCount;           //< 内存池单元数
    size_t          m_nUnitSize;            //< 一个内存池单元的大小
    memory_block*   m_pMemoryMapTable;      //< memory map table的地址
    void*           m_pChunkPool;           //< Memory chunk pool的地址
    void*           m_pMemory;              //< 实际可分配的内存区地址
    size_t          m_nUsedSize;            // 记录内存池中已经分配给用户的内存的大小
    size_t          m_nUsedCount;           // 已经分配的单元数
    bool            m_bInit;                //< 是否初始化
    CriticalSection m_cs;                   //< 内存锁
};

