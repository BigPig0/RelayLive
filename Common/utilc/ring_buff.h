/*!
 * \file ring_buff.h
 *
 * \author wlla 从libwebsockets移植过来
 * \date 十一月 2018
 *
 * 无锁的环形缓冲区，同一时间一个线程插入，一个线程读取
 * 元素element ringbuff中每一个小段的数据，ringbuff的内存分成由n个位置，每个位置的内容称为元素，
 * * 每个位置的状态分为free和waiting。
 * * * free 指没有在该位置存放元素内容，
 * * * waiting 指改位置已存放元素内容，等待被读取。
 * 头head 最新添加元素的位置，即最新的waiting的位置
 * 尾oldest_tail 剩余的最早一个waiting元素的位置
 * 生产insert 指将元素数据拷贝到head之前的状态为free的连续n个位置，并将状态置为waiting
 * 消费consume 指从oldest_tail之前的连续n个状态为waiting位置取出元素数据
 *
 * 生产者inserter 向ringbuff中插入元素的任务
 * 消费者consumer 从ringbuff中读取并删除元素的任务
 * 
 * possible ringbuf patterns[*:waiting -:free]
 * h == t
 * |--------t***h---|
 * |**h-----------t*|
 * |t**************h|
 * |*****ht*********|
 */


#ifndef UTIL_RING_BUFF
#define UTIL_RING_BUFF

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct ring_buff ring_buff_t;

/**
 * 创建ringbuff
 * @param element_len 单个元素的大小
 * @param count 元素个数
 * @param destroy_element 元素删除的方法
 */
ring_buff_t* create_ring_buff(size_t element_len, size_t count, void(*destroy_element)(void *));

/**
 * 销毁ringbuff
 */
void destroy_ring_buff(struct ring_buff *ring);

/** 获取ringbuff中free状态的位置的个数 */
size_t ring_get_count_free_elements(struct ring_buff *ring);

/** 获取ringbuff中waiting状态的位置个数 */
size_t ring_get_count_waiting_elements(struct ring_buff *ring, uint32_t *tail);

/** 从src处将max_count个元素尽可能的插入到ring中，直到src全部插入或ring填满，返回实际插入的个数 */
size_t ring_insert(struct ring_buff *ring, const void *src, size_t max_count);

/**
 * 从ring中由tail指定的位置尽可能的读取max_count个元素拷贝到dest中
 * @param tail[inout] consumer中记录的tail的变量的指针，如果tail为空，则使用ring中的oldest_tail
 * @param dest[inout] 输出读取的内容，从ring的内存中拷贝到dest。为空时不进行拷贝，但是会更新tail
 * @param max_count[in] 尝试取出的个数，不一定能取出这么多个
 * @return 实际取出的个数
 * @remark tail不为空时，只更新tail的值，ring中的oldest_tail不更新。tail为空的时候会更新oldest_tail
 */
size_t ring_consume(struct ring_buff *ring, uint32_t *tail, void *dest, size_t max_count);

/** 从ring中由tail指定的位置找到第一个waiting的元素
 * 调用后使用lws_ring_consume(ring, &tail, NULL, 1)来更新consumer中记录的tail
 */
const void* ring_get_element(struct ring_buff *ring, uint32_t *tail);

/**
 * 将ring中比tail更早的元素状态设置位free。
 * 也就是将ring中oldest_tail的值设置为tail。
 * 当有多个consumer使用这个ringbuff时，任何一个consumer都不再需要的元素通过这个api删除
 * @param tail 删除的元素位置(通过遍历所有consumer中记录的tail信息取最小值得到)
 */
void ring_update_oldest_tail(struct ring_buff *ring, uint32_t tail);

/** 获取ringbuff中第一个waiting元素的位置(oldest_tail的值) */
uint32_t ring_get_oldest_tail(struct ring_buff *ring);

/**
 * 获取free元素的开始位置和字节大小
 * @param start 输出free元素的位置
 * @bytes 输出所有free元素的字节数
 * 使用这个api可以直接写入元素，而不用经过内存拷贝
 */
int ring_next_linear_insert_range(struct ring_buff *ring, void **start, size_t *bytes);

/**
 * 直接向ringbuff的free位置写入数据后，通过改api设置ring中header的位置
 * ring_next_linear_insert_range 和 ring_bump_head配合使用
 */
void ring_bump_head(struct ring_buff *ring, size_t bytes);

/** 打印ring中的信息 */
void ring_dump(struct ring_buff *ring, uint32_t *tail);

/**
 * consumer从ring中读取元素，并更新ring的状态
 * 如果consumer的tail等同于ring的oldest_tail，需要遍历所有consumer的tail值来更新ring的oldest_tail
 * 使用这个宏需要consumer数据由指定格式的链表来存储
 *
 * consumer结构体定义如下:
 * struct ___type {
 *     uint32_t ___mtail;
 *     struct ___type *___mlist;
 *     ***
 * }
 *
 * struct ___type *___list_head;
 *
 * @param ___ring   ringbuff对象的指针
 * @param ___type   consumer链表定义的结构类型名称
 * @param ___ptail  consumer中的tail变量的地址
 * @param ___count  consumer需要删除的元素个数
 * @param ___list_head  consumer链表
 * @param ___mtail  ___type中的tail变量名称
 * @param ___mlist  ___type中的链表next变量名称
 */
#define ring_consume_and_update_oldest_tail(\
		___ring, \
		___type, \
		___ptail, \
		___count, \
		___list_head, \
		___mtail, \
		___mlist \
	) \
{ \
	int ___n, ___m; \
	___n = ring_get_oldest_tail(___ring) == *(___ptail); \
	ring_consume(___ring, ___ptail, NULL, ___count); \
	if (___n) { \
		uint32_t ___oldest; \
        ___type **___ppss = &(___list_head); \
		___n = 0; \
		___oldest = *(___ptail); \
        while (*(___ppss)) { \
            ___m = ring_get_count_waiting_elements( ___ring, &(*___ppss)->tail); \
            if (___m >= ___n) { \
                ___n = ___m; \
                ___oldest = (*___ppss)->tail; \
            } \
            ___ppss = &(*(___ppss))->___mlist; \
		}  \
		ring_update_oldest_tail(___ring, ___oldest); \
	} \
}

/*
 * 和 ring_consume_and_update_oldest_tail()作用一样
 * 这用于更简单的只有一个consumer, 以及一个tail的情况
 * 这个tail永远都是ring的 oldest tail.
 */

#define ring_consume_single_tail(\
		___ring, \
		___ptail, \
		___count \
	) { \
	ring_consume(___ring, ___ptail, NULL, ___count); \
	ring_update_oldest_tail(___ring, *(___ptail)); \
}

#ifdef __cplusplus
}
#endif
#endif