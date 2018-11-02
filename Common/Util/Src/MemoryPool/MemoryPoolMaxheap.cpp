#include "stdafx.h"
#include "Mutex.h"
#include "MemoryPoolMaxheap.h"
#include "MaxHeap.h"


CMemoryPoolMaxheap::CMemoryPoolMaxheap(void)
{
}


CMemoryPoolMaxheap::~CMemoryPoolMaxheap(void)
{
    clear();
}

bool CMemoryPoolMaxheap::init(int nUnitCount, int nUnitSize)
{
    if(!IMemoryPool::init(nUnitCount,nUnitSize))
        return false;
    MutexLock  lock(&m_cs);

    // 创建大顶堆
    CMaxHeap* pMaxHeap = new CMaxHeap;
    if(pMaxHeap == nullptr) return false;
    m_pMaxHeap = (void*)pMaxHeap;

    // 初始化大顶堆，chunk pool是大顶堆形式
    pMaxHeap->init_max_heap(m_nUnitCount, (memory_chunk*)m_pChunkPool);

    // 初始化 chunk pool
    memory_chunk chunk;  
    chunk.chunk_size = m_nUnitCount;
    memory_chunk* pos = pMaxHeap->insert_heap(chunk);
    if (nullptr == pos)
    {
        Log::error("init insert failed");
        return false;
    }
    // 初始chunk指向起始memory block
    pos->pfree_mem_addr = m_pMemoryMapTable;

    // 初始化 memory map table  
    m_pMemoryMapTable[0].count = m_nUnitCount;  
    m_pMemoryMapTable[0].pmem_chunk = pos;  
    m_pMemoryMapTable[m_nUnitCount-1].start = 0;  

    m_nUsedSize = 0;

    m_bInit = true;
    return true;
}

void CMemoryPoolMaxheap::clear()
{
    if (m_bInit == false)
        return;
    MutexLock  lock(&m_cs);

    if(nullptr != m_pMemoryMapTable)
    {
        delete m_pMemoryMapTable;
        m_pMemoryMapTable = nullptr;
    }

    if (nullptr != m_pMaxHeap)
    {
        delete m_pMaxHeap;
        m_pMaxHeap = nullptr;
    }

    m_bInit = false;
}

void* CMemoryPoolMaxheap::mp_malloc(size_t nSize)  
{
    if(!m_bInit || m_pChunkPool == nullptr)
        return nullptr;

    // 需要分配的block个数
    size_t count = (nSize + m_nUnitSize - 1) / m_nUnitSize;
    // 需要分配的大小
    size_t sMemorySize = count * m_nUnitSize;

    CMaxHeap* pMaxHeap = (CMaxHeap*)m_pMaxHeap;

    MutexLock  lock(&m_cs);
    // 获取堆顶
    memory_chunk* max_chunk = nullptr;  
    bool ret = pMaxHeap->get_max(max_chunk);
    if (nullptr == max_chunk || nullptr == max_chunk->pfree_mem_addr)
    {
        Log::error("null ptr max_chunk:%d",max_chunk);
        return nullptr;  
    }
    if (ret == false || max_chunk->chunk_size < count)  
    {
        Log::error("there is not enough memory in max chunk ret:%d,max chunk_size:%d,count:%d",ret,max_chunk->chunk_size,count);
        return nullptr;  
    } 

    m_nUsedSize += sMemorySize;
    m_nUsedCount += count;
    Log_Debug("mp_malloc UsedCount:%d/%d, UsedSize:%d,requestSize:%d/%d,requestCount:%d/%d", m_nUsedCount,m_nUnitCount,m_nUsedSize,nSize,sMemorySize,count,max_chunk->chunk_size);

    if (max_chunk->chunk_size == count)  
    {
        // 当要分配的内存大小与当前chunk中的内存大小相同时，从堆中删除此chunk  
        size_t current_index = (max_chunk->pfree_mem_addr - m_pMemoryMapTable);  
         max_chunk->pfree_mem_addr->pmem_chunk = nullptr; 
        if(!pMaxHeap->remove_max())
        {
            Log::error("mp_malloc remove_max failed");
            return nullptr;
        }

        Log_Debug("mp_malloc end");
        return index2addr(current_index);  
    }  
    else  // max_chunk->chunk_size > count
    {  
        // 当要分配的内存小于当前chunk中的内存时，更改堆中相应chunk的pfree_mem_addr  

        // 记录原先的连续block个数
        size_t old_block_count = max_chunk->pfree_mem_addr->count;

        // 堆顶chunk指向的block，current_block是分配使用的起始block
        memory_block* current_block = max_chunk->pfree_mem_addr;
        // 分配使用的block个数
        current_block->count = count;
        // 分配出来的起始block的位置
        size_t current_index = current_block - m_pMemoryMapTable;
        // 分配出来的结束block
        m_pMemoryMapTable[current_index+count-1].start = current_index; 
        // NULL表示当前内存块已被分配  
        current_block->pmem_chunk = nullptr; 
        Log_Debug("malloc current_index:%d,addr:%d",current_index,current_block);

        // chunk原来的block个数减掉用掉的个数，就是该chunk剩下的block数
        memory_chunk* pos = pMaxHeap->decrease_element_value(max_chunk, count);
        if (nullptr == pos)
        {
            Log::error("new chunk pos is null");
            return nullptr;
        }

        // 分配后剩余的起始block
        memory_block& new_first = m_pMemoryMapTable[current_index+count];
        // 剩余block的个数
        new_first.count = old_block_count - count;  
        // 分配后剩余的结束block，指向剩余的起始位置
        size_t end_index = current_index + old_block_count - 1;  
        m_pMemoryMapTable[end_index].start = current_index + count; 
        // 剩余block指向的chunk
        new_first.pmem_chunk = pos; 

        // 更新chunk指向的block
        pos->pfree_mem_addr = &new_first;  

        Log_Debug("mp_malloc end");
        return index2addr(current_index);  
    }
}

void CMemoryPoolMaxheap::mp_free(void* p)   
{
    if(!m_bInit || m_pChunkPool == nullptr)
        return;
    MutexLock  lock(&m_cs);

    CMaxHeap* pMaxHeap = (CMaxHeap*)m_pMaxHeap;
    size_t current_index = addr2index(p);
    if (current_index >= m_nUnitCount)
    {
        Log::error("free index is bigger than max %d/%d",current_index,m_nUnitCount);
        return;
    }
    
    // 打印信息
    memory_block& blockTmp = m_pMemoryMapTable[current_index];
    Log_Debug("mp_free current_index:%d,blockInfo addr:%d,count:%d,pChunk:%d,start:%d",current_index,&blockTmp,blockTmp.count,blockTmp.pmem_chunk,blockTmp.start);
 
    // 要释放的大小
    size_t count = m_pMemoryMapTable[current_index].count;
    size_t size =  count* m_nUnitSize;  
    m_nUsedCount -= count;
    m_nUsedSize -= size;
    Log_Debug("mp_free UsedCount:%d/%d, UsedSize:%d,freeCount:%d", m_nUsedCount,m_nUnitCount,m_nUsedSize,count);
    if (current_index + count > m_nUnitCount)
    {
        Log::error("free chunk end is bigger than max %d,%d/%d",current_index,count,m_nUnitCount);
        return;
    }

    // 判断与当前释放的内存块相邻的内存块是否可以与当前释放的内存块合并
    memory_block* pre_block = nullptr;  // 前一个块
    memory_block* next_block = nullptr;  // 后一个块
    memory_block* current_block = &(m_pMemoryMapTable[current_index]);  // 释放的当前块
    // 第一个
    if (current_index == 0)
    {
        if (count < m_nUnitCount)  
        {
            // 后一个块
            next_block = &(m_pMemoryMapTable[current_index+count]);  
            // 如果后一个内存块是空闲的，合并  
            if (next_block->pmem_chunk != nullptr)  
            {
                // 后一个chunk的大小增加，并重新排序
                memory_chunk* pos = pMaxHeap->increase_element_value((memory_chunk*)(next_block->pmem_chunk), count);  
                pos->pfree_mem_addr = current_block;
                // 下一个块指向的开头移到释放的块开头
                m_pMemoryMapTable[current_index+count+next_block->count-1].start = current_index;  
                current_block->count += next_block->count;  
                current_block->pmem_chunk = pos;  
                next_block->pmem_chunk = nullptr;  // 不再是块开头
            }  
            // 如果后一块内存不是空闲的，在pfree_mem_chunk中增加一个chunk  
            else  
            {
                memory_chunk new_chunk;  
                new_chunk.chunk_size = current_block->count;  
                new_chunk.pfree_mem_addr = current_block;  
                memory_chunk* pos = pMaxHeap->insert_heap(new_chunk);
                if(pos == nullptr) 
                {
                    Log::error("insert_heap failed");
                    return;
                }
                pos->pfree_mem_addr = current_block;
                current_block->pmem_chunk = pos;  
            }  
        }  
        else  // current_block->count == m_nUnitCount  current_index == 0
        {
            memory_chunk new_chunk;  
            new_chunk.chunk_size = current_block->count;  
            new_chunk.pfree_mem_addr = current_block;  
            memory_chunk* pos = pMaxHeap->insert_heap(new_chunk);  
            if(pos == nullptr) 
            {
                Log::error("insert_heap failed");
                return;
            }
            pos->pfree_mem_addr = current_block;
            current_block->pmem_chunk = pos;  
        }         
    }  
    // 最后一个  
    else if (current_index + count == m_nUnitCount)  
    {
        if (count < m_nUnitCount)  // current_index > 0
        {
            // 找到前一个块
            pre_block = &(m_pMemoryMapTable[current_index-1]); 
            size_t index = pre_block->start;  
            pre_block = &(m_pMemoryMapTable[index]);
            Log_Debug("current_index:%d,index:%d,pre_block->pmem_chunk:%d",current_index,index,pre_block->pmem_chunk);

            // 如果前一个内存块是空闲的，合并  
            if (pre_block->pmem_chunk != nullptr)  
            {
                memory_chunk* pos = pMaxHeap->increase_element_value((memory_chunk*)pre_block->pmem_chunk, count);  
                pos->pfree_mem_addr = pre_block;
                pre_block->pmem_chunk = pos;  
                // 释放的块的结尾指向开头
                m_pMemoryMapTable[current_index+count-1].start = current_index - pre_block->count;  
                pre_block->count += count;  
                current_block->pmem_chunk = nullptr;  // 不再是chunk开头
            }  
            // 如果前一块内存不是空闲的，在pfree_mem_chunk中增加一个chunk  
            else  
            {
                memory_chunk new_chunk;  
                new_chunk.chunk_size = current_block->count;  
                new_chunk.pfree_mem_addr = current_block;  
                memory_chunk* pos = pMaxHeap->insert_heap(new_chunk);  
                current_block->pmem_chunk = pos;  
            }  
        }  
        else  // current_block->count == m_nUnitCount  current_index == 0
        {
            memory_chunk new_chunk;  
            new_chunk.chunk_size = current_block->count;  
            new_chunk.pfree_mem_addr = current_block;  
            memory_chunk* pos = pMaxHeap->insert_heap(new_chunk);  
            if(pos == nullptr) 
            {
                Log::error("insert_heap failed");
                return;
            }
            pos->pfree_mem_addr = current_block;
            current_block->pmem_chunk = pos;  
        }  
    }  
    else // 不是开头块或结尾块 
    {
        //后一个块
        next_block = &(m_pMemoryMapTable[current_index+count]);
        Log_Debug("next block:%d,count:%d",next_block,next_block->count);
        //前一个块
        pre_block = &(m_pMemoryMapTable[current_index-1]);  
        size_t index = pre_block->start;  
        pre_block = &(m_pMemoryMapTable[index]);
        Log_Debug("pre block:%d,index:%d,count:%d",pre_block,index,pre_block->count);
        //是否与后一个块合并
        bool is_back_merge = false;  
        // 前后的块都被使用，直接新增chunk
        if (next_block->pmem_chunk == nullptr && pre_block->pmem_chunk == nullptr)  
        {
            memory_chunk new_chunk;  
            new_chunk.chunk_size = current_block->count;  
            new_chunk.pfree_mem_addr = current_block;  
            memory_chunk* pos = pMaxHeap->insert_heap(new_chunk);
            pos->pfree_mem_addr = current_block;
            current_block->pmem_chunk = pos;  
        }  
        // 后一个内存块未使用
        if (next_block->pmem_chunk != nullptr)  
        {
            Log_Debug("mp_free bb  next_block->pmem_chunk:%d",next_block->pmem_chunk);
            // 后一个位置的chunk增加大小
            memory_chunk* pos = pMaxHeap->increase_element_value((memory_chunk*)next_block->pmem_chunk, current_block->count);  
            pos->pfree_mem_addr = current_block; 
            //后一个块的指向
            m_pMemoryMapTable[current_index+current_block->count+next_block->count-1].start = current_index;
            //合并大小
            current_block->count += next_block->count;  
            current_block->pmem_chunk = pos;  
            next_block->pmem_chunk = nullptr;  // next block 不再是起始
            is_back_merge = true;  
        }  
        // 前一个内存块未使用
        if (pre_block->pmem_chunk != nullptr)  
        {
            // 块的末尾block指向前一个块开头
            m_pMemoryMapTable[current_index+current_block->count-1].start = current_index - pre_block->count;
            // 前一个块增加大小
            pre_block->count += current_block->count;
            // 前一个块对应的chunk增加大小
            memory_chunk* pos = pMaxHeap->increase_element_value((memory_chunk*)pre_block->pmem_chunk, current_block->count);  
            pre_block->pmem_chunk = pos;  
            pos->pfree_mem_addr = pre_block;  
            if (is_back_merge)  
            {
                // 之前向后合并过的，需要移除一个chunk
                pMaxHeap->remove_element((memory_chunk*)current_block->pmem_chunk);  
            }  
            current_block->pmem_chunk = nullptr;    // 不再是起始block          
        }     
    } 
    Log_Debug("mp_free end");
}

size_t CMemoryPoolMaxheap::get_chunk_pool_size()
{
    return sizeof(memory_chunk) * m_nUnitCount;
}

void CMemoryPoolMaxheap::showinfo()
{
    for (int i=0; i<m_nUnitCount; ++i)
    {
        memory_chunk* pPool = (memory_chunk*)m_pChunkPool;
        memory_chunk* pChunk = (memory_chunk*)m_pMemoryMapTable[i].pmem_chunk;
        Log_Debug("block info:%d,count:%d,start:%d,pchunk:%d,index:%d"
            ,i,m_pMemoryMapTable[i].count,m_pMemoryMapTable[i].start,pChunk
            ,pChunk?pChunk-pPool:-1);
    }
    for (int i=0; i<m_nUnitCount; ++i)
    {
        memory_chunk* pPool = (memory_chunk*)m_pChunkPool;
        Log_Debug("chunk info:%d,size:%d,pblock:%d,index:%d",i,pPool[i].chunk_size,pPool[i].pfree_mem_addr,pPool[i].pfree_mem_addr-m_pMemoryMapTable);
    }
    CMaxHeap* pMaxHeap = (CMaxHeap*)m_pMaxHeap;
    Log_Debug("chunk current Size:%d",pMaxHeap->m_nCurrentSize);
}