#include "stdafx.h"
#include "MemoryPoolList.h"
#include "ChunkList.h"


CMemoryPoolList::CMemoryPoolList(void)
    : IMemoryPool()
    , m_pEmptyNodeList(nullptr)
    , m_pFreeChunkList(nullptr)
{
}


CMemoryPoolList::~CMemoryPoolList(void)
{
    clear();
}

bool CMemoryPoolList::init(int nUnitCount, int nUnitSize)
{
    MutexLock  lock(&m_cs);
    if(!IMemoryPool::init(nUnitCount,nUnitSize))
    {
        Log::error("IMemoryPool::init failed");
        return false;
    }
    Log_Debug("IMemoryPool::init sucess");

    CChunkList* pEmptyNodeList = new CChunkList;
    if(pEmptyNodeList == nullptr) return false;
    m_pEmptyNodeList = (void*)pEmptyNodeList;

    CChunkList* pFreeChunkList = new CChunkList;
    if(pFreeChunkList == nullptr) return false;
    m_pFreeChunkList = (void*)pFreeChunkList;

    // 初始化chunk pool
    memory_chunk* pNode = (memory_chunk*)m_pChunkPool;
    for (size_t i=0; i<(size_t)nUnitCount; ++i)
    {
        pEmptyNodeList->push_front(pNode);
        ++ pNode;
    }

    // 初始化memory map table
    memory_chunk* tmp = pEmptyNodeList->pop_front();
    m_pMemoryMapTable[0].count = m_nUnitCount;
    m_pMemoryMapTable[0].pmem_chunk = tmp;
    m_pMemoryMapTable[m_nUnitCount-1].start = 0;

    tmp->pfree_mem_addr = m_pMemoryMapTable;
    pFreeChunkList->push_front(tmp);

    m_bInit = true;
    return true;
}

void CMemoryPoolList::clear()
{
    if (m_bInit == false)
        return;
    MutexLock  lock(&m_cs);
    
    if(nullptr != m_pMemoryMapTable)
    {
        delete m_pMemoryMapTable;
        m_pMemoryMapTable = nullptr;
    }

    if (nullptr != m_pEmptyNodeList)
    {
        delete m_pEmptyNodeList;
        m_pEmptyNodeList = nullptr;
    }

    if (nullptr != m_pEmptyNodeList)
    {
        delete m_pEmptyNodeList;
        m_pEmptyNodeList = nullptr;
    }

    m_bInit = false;
}

void* CMemoryPoolList::mp_malloc(size_t nSize)  
{
    if(!m_bInit || m_pChunkPool == nullptr)
        return nullptr;

    // 需要分配的block个数
    size_t count = (nSize + m_nUnitSize - 1) / m_nUnitSize;

    CChunkList* pEmptyNodeList = (CChunkList*)m_pEmptyNodeList;
    CChunkList* pFreeChunkList = (CChunkList*)m_pFreeChunkList;
    if(nullptr == pEmptyNodeList || nullptr == pFreeChunkList)
        return nullptr;

    MutexLock  lock(&m_cs);
  
    // 查找链表中第一个大小足够的chunk
    memory_chunk* tmp = pFreeChunkList->m_pHead;
    while (nullptr != tmp && tmp->pfree_mem_addr->count < count)
    {
        tmp = tmp->next;
    }
    if (nullptr == tmp)
    {
        Log::error("there is no chunk has enough size");
        return nullptr;  
    }

    if (tmp->pfree_mem_addr->count == count)
    {
        // 当要分配的内存大小与当前chunk中的内存大小相同时，从链表中删除此chunk  
        size_t current_index = tmp->pfree_mem_addr - m_pMemoryMapTable;
        pFreeChunkList->delete_chunk(tmp);
        tmp->pfree_mem_addr->pmem_chunk = nullptr;
        pEmptyNodeList->push_front(tmp);

        return index2addr(current_index);
    }
    else
    {
        // 当要分配的内存小于当前chunk中的内存时，更改链表中相应chunk的pfree_mem_addr
        pFreeChunkList->delete_chunk(tmp);
        // 分配出去的block
        memory_block* current_block = tmp->pfree_mem_addr;
        size_t current_index = current_block - m_pMemoryMapTable;
        size_t old_count = current_block->count;
        current_block->count = count;
        memory_block* current_block_end = current_block + (count-1);
        current_block_end->start = current_index;
        current_block->pmem_chunk = nullptr;
        // 剩下的block
        memory_block* next_block = current_block_end + 1;
        next_block->count = old_count - count;
        memory_block* next_block_end = next_block + (next_block->count - 1);
        next_block_end->start = next_block - m_pMemoryMapTable;
        next_block->pmem_chunk = tmp;
        // chunk节点更新后再插入链表
        tmp->pfree_mem_addr = next_block;
        pFreeChunkList->push_front(tmp);

        return index2addr(current_index);
    }
}

void CMemoryPoolList::mp_free(void* p)   
{
    if(!m_bInit || m_pChunkPool == nullptr)
        return;
    MutexLock  lock(&m_cs);

    size_t current_index = addr2index(p);

    memory_block* current_block = &(m_pMemoryMapTable[current_index]);
    memory_block* pre_block = nullptr;  
    memory_block* next_block = nullptr;   

    CChunkList* pEmptyNodeList = (CChunkList*)m_pEmptyNodeList;
    CChunkList* pFreeChunkList = (CChunkList*)m_pFreeChunkList;
    if(nullptr == pEmptyNodeList || nullptr == pFreeChunkList)
        return;

    if (current_index == 0)  // 第一个
    {
        if (current_block->count < m_nUnitCount)  
        {
            next_block = &(m_pMemoryMapTable[current_index+current_block->count]);  
            // 如果后一个内存块是空闲的，合并  
            if (next_block->pmem_chunk != nullptr)  
            {  
                ((memory_chunk*)(next_block->pmem_chunk))->pfree_mem_addr = current_block;  
                m_pMemoryMapTable[current_index+current_block->count+next_block->count-1].start = current_index;  
                current_block->count += next_block->count;  
                current_block->pmem_chunk = next_block->pmem_chunk;  
                next_block->pmem_chunk = nullptr;  
            }  
            // 如果后一块内存不是空闲的，在pfree_mem_chunk中增加一个chunk  
            else  
            {  
                memory_chunk* new_chunk = pEmptyNodeList->pop_front();  
                new_chunk->pfree_mem_addr = current_block;  
                current_block->pmem_chunk = new_chunk;  
                pFreeChunkList->push_front(new_chunk);  
            }
        }
        else
        {
            Log::error("current_block->count > m_nUnitCount");
        }
    }
    else if (current_index + current_block->count == m_nUnitCount) // 最后一个
    {
        if (current_block->count < m_nUnitCount)  
        {  
            pre_block = &(m_pMemoryMapTable[current_index-1]);  
            size_t index = pre_block->start;  
            pre_block = &(m_pMemoryMapTable[index]);  

            // 如果前一个内存块是空闲的，合并  
            if (pre_block->pmem_chunk != nullptr)  
            {
                // end block
                m_pMemoryMapTable[current_index+current_block->count-1].start = index;  
                pre_block->count += current_block->count;  
                current_block->pmem_chunk = nullptr;  
            }  
            // 如果前一块内存不是空闲的，在pFreeChunkList中增加一个chunk  
            else  
            {  
                memory_chunk* new_chunk = pEmptyNodeList->pop_front();  
                new_chunk->pfree_mem_addr = current_block;  
                current_block->pmem_chunk = new_chunk;  
                pFreeChunkList->push_front(new_chunk);  
            }  
        }  
        else  
        {  
            Log::error("current_block->count > m_nUnitCount");
        }  
    }
    else // 中间
    {
        next_block = &(m_pMemoryMapTable[current_index+current_block->count]);  
        pre_block = &(m_pMemoryMapTable[current_index-1]);  
        size_t index = pre_block->start;  
        pre_block = &(m_pMemoryMapTable[index]);  

        // 前后内存块都已经分配，在pFreeChunkList中增加一个chunk
        if (next_block->pmem_chunk == nullptr && pre_block->pmem_chunk == nullptr)  
        {  
            memory_chunk* new_chunk = pEmptyNodeList->pop_front();  
            new_chunk->pfree_mem_addr = current_block;  
            current_block->pmem_chunk = new_chunk;  
            pFreeChunkList->push_front(new_chunk);   
        }  
        bool is_back_merge = false;  
        // 后一个内存块未分配
        if (next_block->pmem_chunk != nullptr)  
        {  
            ((memory_chunk*)(next_block->pmem_chunk))->pfree_mem_addr = current_block;  
            m_pMemoryMapTable[current_index+current_block->count+next_block->count-1].start = current_index;  
            current_block->count += next_block->count;  
            current_block->pmem_chunk = next_block->pmem_chunk;  
            next_block->pmem_chunk = nullptr;  
            is_back_merge = true;  
        }  
        // 前一个内存块未分配
        if (pre_block->pmem_chunk != nullptr)  
        {  
            m_pMemoryMapTable[current_index+current_block->count-1].start = current_index - pre_block->count;  
            pre_block->count += current_block->count;  
            if (is_back_merge)  
            {  
                pFreeChunkList->delete_chunk((memory_chunk*)current_block->pmem_chunk);  
                pEmptyNodeList->push_front((memory_chunk*)current_block->pmem_chunk);  
            }  
            current_block->pmem_chunk = nullptr;              
        }    
    }
}

size_t CMemoryPoolList::get_chunk_pool_size()
{
    return sizeof(memory_chunk) * m_nUnitCount;
}