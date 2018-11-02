#pragma once
#include "imemorypool.h"
#include "ExportDefine.h"

/**
 * 链表结构的内存池
 * 链表结构的内存池实现是指将memory chunk set实现为双向链表结构。这种方法的优缺点如下：
 * 优点：释放内存很快，O(1)复杂度。	
 * 缺点：分配内存较慢，O(n)复杂度。
 */
class COMMON_API CMemoryPoolList : public IMemoryPool
{
public:
    CMemoryPoolList(void);
    ~CMemoryPoolList(void);

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

protected:
    
    /**
     * 获取memory chunk pool 的大小
     */
    virtual size_t get_chunk_pool_size() override;

private:
    void*           m_pEmptyNodeList;   //< chunk pool 中未使用的chunk节点组成链表，以备使用
    void*           m_pFreeChunkList;   //< 指向空闲内存块的chunk链表
};

