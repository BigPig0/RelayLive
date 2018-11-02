#include "stdafx.h"
#include "Maxheap.h"


CMaxHeap::CMaxHeap(void)
{
}


CMaxHeap::~CMaxHeap(void)
{
}
                                                                       
void CMaxHeap::init_max_heap(size_t max_heap_size, memory_chunk* heap_arr)  
{  
    m_nMaxSize = max_heap_size;  
    m_nCurrentSize = 0;  
    m_pHeap = heap_arr;  
} 

bool CMaxHeap::is_heap_empty()  
{  
    return m_nCurrentSize == 0;    
}

bool CMaxHeap::is_heap_full()  
{  
    return m_nCurrentSize == m_nMaxSize;    
}

memory_chunk* CMaxHeap::filter_up(size_t start)  
{  
    if (start < 0 || start >= m_nCurrentSize)
    {
        Log::error("filter_up failed, start:%d,m_nCurrentSize:%d",start,m_nCurrentSize);
        return nullptr;
    }

    size_t i = start;
    size_t j = ( i - 1 ) / 2;   // 计算父节点位置
    memory_chunk temp = m_pHeap[i];
    while(i > 0)
    {    
        if(temp.chunk_size <= m_pHeap[j].chunk_size)
            break;    
        else  
        {             
            m_pHeap[i] = m_pHeap[j];    // 满足交换条件，将父节点拷贝到叶子节点
            if(m_pHeap[j].pfree_mem_addr == nullptr)
            {
                Log::error("m_pHeap[j=%d].pfree_mem_addr is null",j);
                return nullptr;
            }
            m_pHeap[j].pfree_mem_addr->pmem_chunk = &(m_pHeap[i]);

            i = j;    // 向上一级遍历
            j = (i - 1) / 2;    
        }    
    }
    // 最后遍历到的节点，将输入位置的值付给他，完成排序
    m_pHeap[i] = temp;

    // 返回的是移动位置后的地址
    // 此处并没有将temp指向的block重新指向m_pHeap[i]，这个操作在外面处理
    // 有三种情况会用到这个函数:
    // 1是删除该节点，其对应的block指向的chunk会改成null，
    // 2是增加大小，调用increase_element_value后将block指向的chunk设置成其返回值
    // 3是增加节点，调用insert_heap后，将block指向的chunk设置成其返回值
    return &(m_pHeap[i]);
}

memory_chunk* CMaxHeap::filter_down(size_t start)  
{
    if (start < 0 || start >= m_nCurrentSize)
    {
        Log::error("filter_up failed, start:%d,m_nCurrentSize:%d",start,m_nCurrentSize);
        return nullptr;
    }
    size_t i = start;  
    size_t j = i * 2 + 1;   // 计算左儿子的位置，j+1就是右儿子的位置
    size_t endOfHeap = m_nCurrentSize-1;
    memory_chunk temp = m_pHeap[i];
    while(j <= endOfHeap)  
    {
        // j和j+1是i的两个叶子节点，取大的一个
        if(j < endOfHeap && m_pHeap[j].chunk_size < m_pHeap[j+1].chunk_size)
            j++;

        if(temp.chunk_size > m_pHeap[j].chunk_size)
            break;
        else  
        {
            m_pHeap[i] = m_pHeap[j];
            if(m_pHeap[j].pfree_mem_addr == nullptr)
            {
                Log::error("m_pHeap[j].pfree_mem_addr is null");
                return nullptr;
            }
            m_pHeap[j].pfree_mem_addr->pmem_chunk = &(m_pHeap[i]);
            i = j; 
            j = 2 * i + 1;
        }
    }
    // 最后遍历到的节点，将第一个遍历的值付给他，完成排序
    m_pHeap[i] = temp;  

    // 返回的是移动位置后的地址
    // 此处并没有将temp指向的block重新指向m_pHeap[i]，这个操作在外面处理
    // 有两种情况会用到这个函数:
    // 1是删除堆顶节点，将最右叶子覆盖堆顶后排序，在remove_max里为最右叶子对应的block设置指向的chunk的新位置
    // 2是减小大小，调用decrease_element_value后将block指向的chunk设置成其返回值
    return &(m_pHeap[i]);
}

memory_chunk* CMaxHeap::insert_heap(memory_chunk& chunk)
{
    if (is_heap_full())
    {
        Log::error("heap is full");
        return nullptr;
    }
    m_pHeap[m_nCurrentSize] = chunk;  
    ++m_nCurrentSize;
    memory_chunk* ret = filter_up(m_nCurrentSize-1); 
    return ret;
}

bool CMaxHeap::get_max(memory_chunk*& chunk)  
{  
    if(is_heap_empty())  
    {    
        return false;    
    }    
    chunk = m_pHeap;    
    return true;  
}

bool CMaxHeap::remove_max()  
{  
    if(is_heap_empty())  
    {    
        return false;    
    }

    if (m_nCurrentSize > 1)  
    {
        //将最后一个覆盖堆顶，然后再向下排序
        m_pHeap[0] = m_pHeap[m_nCurrentSize - 1];
        --m_nCurrentSize; // chunk数减1
        // 原先的堆顶指向的block被分配掉，其pmem_chunk变为null，但新堆顶是原来最后一个叶子，它对应的block的pmem_chunk要重设
        memory_block* pBlock = m_pHeap[0].pfree_mem_addr;  
        memory_chunk* pTmp = filter_down(0);
        pBlock->pmem_chunk = pTmp;    
    }
    else // m_nCurrentSize == 1
    {
        --m_nCurrentSize; // chunk数减1
    }
    return true;    
}

void CMaxHeap::remove_element(memory_chunk* chunk)  
{   
    // 将该元素size增至最大（大于max element）
    chunk->chunk_size = m_nMaxSize + 1;  
    // 然后将其上移至堆顶，该chunk对应的block的pmem_chunk调用的地方设为null
    filter_up(chunk - m_pHeap);
    // 删除堆顶元素     
    remove_max();  
}

memory_chunk* CMaxHeap::increase_element_value(memory_chunk* chunk, size_t increase_value)  
{  
    size_t index = chunk - m_pHeap;  
    chunk->chunk_size += increase_value;  
    return filter_up(index);  
}

memory_chunk* CMaxHeap::decrease_element_value(memory_chunk* chunk, size_t decrease_value)  
{
    size_t index = chunk - m_pHeap;  
    chunk->chunk_size -= decrease_value;  
    return filter_down(index);  
}  