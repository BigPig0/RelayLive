/**
 * 大顶堆结构内存池
 * 大顶堆结构的内存池实现是指将memory chunk set实现为大顶堆结构。这种方法的优缺点如下：
 * 优点：降低了分配内存的时间复杂度，O(log(n))。	
 * 缺点：增加了释放内存的时间复杂度，O(log(n))。
 */

#pragma once
#include "imemorypool.h"
#include "ExportDefine.h"

struct max_heap;

class COMMON_API CMemoryPoolMaxheap : public IMemoryPool
{
public:
    CMemoryPoolMaxheap(void);
    ~CMemoryPoolMaxheap(void);

    /**
     * 初始化内存池
     * @param nUnitCount 内存单元个数
     * @param nUnitSize  内存单元大小
     * @return 成功true；失败false
     */
    virtual bool init(int nUnitCount, int nUnitSize) override;

    /**
     * 清理内存池
     */
    virtual void clear() override;

    /**
     * 申请内存
     */
    virtual void* mp_malloc(size_t nSize) override;

    /**
     * 释放内存
     */
    virtual void mp_free(void* p) override;

    void showinfo();

protected:
    
    /**
     * 获取memory chunk pool 的大小
     */
    virtual size_t get_chunk_pool_size() override;

private:
    void*           m_pMaxHeap;
};

