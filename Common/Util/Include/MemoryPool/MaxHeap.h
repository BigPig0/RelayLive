#pragma once
#include "IMemoryPool.h"

/** 可用的内存块结构体 */
struct memory_chunk  
{  
    memory_block*   pfree_mem_addr;     //< chunk指向的memory map table中的未使用的起始block 
    size_t          chunk_size;         //< 连续的未使用的block的个数
};

class CMaxHeap
{
public:
    CMaxHeap(void);
    ~CMaxHeap(void);

    /**
     * 初始化大顶堆
     * @param max_heap_size 堆成员最大数
     * @param heap_arr 存放大顶堆内容的内存地址
     */
    void init_max_heap(size_t max_heap_size, memory_chunk* heap_arr);

    /** 大顶堆是否为空 */
    bool is_heap_empty();

    /** 大顶堆是否已满 */
    bool is_heap_full();

    /**
     * 从指定位置开始，向上进行排序
     * @param start 指定项的位置
     * @return 指定项排序后所在的位置
     * @note 大顶堆本身是有序的，只有其中一项值增大或增加新项，才需要从这项开始向上排序一次
     */
    memory_chunk* filter_up(size_t start);

    /**
     * 从指定位置开始，向下进行排序
     * @param start 指定项的位置
     * @return 指定项排序后所在的位置
     * @note 大顶堆本身是有序的，只有其中一项值减小或删除新项，才需要从这项开始向下排序一次
     */
    memory_chunk* filter_down(size_t start);

    /** 增加一个成员 */
    memory_chunk* insert_heap(memory_chunk& chunk);

    /**
     * 获取最大成员，即顶上的成员
     * @param chunk[out] 输出最大成员
     */
    bool get_max(memory_chunk*& chunk);

    /** 移除顶上的项(最大项) */
    bool remove_max();

    /** 移除指定项，只有一种情况会用到，就是释放内存后，新增chunk，其block块前后都有block块，三者合并成一个block块。其实就是原来的两个chunk变成一个 */
    void remove_element(memory_chunk* chunk);

    /**
     * 指定项的chunksize增加大小
     * @param chunk 要改动的项
     * @param increase_value 要增加的大小
     * @return 修改值后，该项移动后的地址
     */
    memory_chunk* increase_element_value(memory_chunk* chunk, size_t increase_value);

    /**
     * 指定项的chunksize减小大小
     * @param chunk 要改动的项
     * @param decrease_value 要减小的大小
     * @return 修改值后，该项移动后的地址
     */
    memory_chunk* decrease_element_value(memory_chunk* chunk, size_t decrease_value);

    public:
    memory_chunk*       m_pHeap;        //< 大顶堆起始地址
    size_t              m_nMaxSize;     //< 最大个数     
    size_t              m_nCurrentSize; //< 当前个数  
};

