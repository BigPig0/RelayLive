#pragma once
#include "IMemoryPool.h"

/** 可用的内存块结构体 */
struct memory_chunk  
{
    memory_block* pfree_mem_addr;  
    memory_chunk* pre;  
    memory_chunk* next;  
};

/**
 * 链表结构
 */
class CChunkList
{
public:
    CChunkList(void);
    ~CChunkList(void);

    /**
     * 从链表中取出表头
     * @return 链表头节点
     */
    memory_chunk* pop_front();

    /**
     * 从链表中删除指定节点
     */
    void delete_chunk(memory_chunk* element);

    /**
     * 向链表头插入新的节点
     * @param element 新节点
     */
    void push_front(memory_chunk* element);

public:
    memory_chunk*       m_pHead;
    size_t              m_nCount;       //< 节点个数
};

