/*!
 * \file ring_buff.h
 *
 * \author wlla
 * \date 十一月 2018
 *
 * 无所的环形缓冲区，同一时间一个线程插入，一个线程读取
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

/** 获取ringbuff中空闲位置的个数 */
size_t ring_get_count_free_elements(struct ring_buff *ring);

/** 获取ringbuff中已经存放数据的位置个数 */
size_t ring_get_count_waiting_elements(struct ring_buff *ring, uint32_t *tail);

/** 从src处将max_count个元素尽可能的插入到ring中，知道全部插入或ring填满，返回实际插入的个数 */
size_t ring_insert(struct ring_buff *ring, const void *src, size_t max_count);

/**
 * 从ring中由tail指定的位置尽可能的移除max_count个元素拷贝的dest中
 * dest为空时不进行拷贝动作，直接在ring中移除
 * tail输出移除后的位置，如果tail为空，则从ring记录的oldest_tail处开始
 */
size_t ring_consume(struct ring_buff *ring, uint32_t *tail, void *dest, size_t max_count);

/** 从ring中由tail位置指定的位置找到第一个存在的元素
 * 调用后使用lws_ring_consume(ring, &tail, NULL, 1)来从ring中移走该元素
 */
const void* ring_get_element(struct ring_buff *ring, uint32_t *tail);

/**
 * 释放不在需要的元素。
 * 当有多个地方使用这个ringbuff时，每次获取元素后并不立即释放，而是通过这个方法统一释放不在需要的元素
 * @param tail 释放的元素位置
 */
void ring_update_oldest_tail(struct ring_buff *ring, uint32_t tail);

/** 获取ringbuff中第一个元素的位置 */
uint32_t ring_get_oldest_tail(struct ring_buff *ring);

int ring_next_linear_insert_range(struct ring_buff *ring, void **start, size_t *bytes);

void ring_bump_head(struct ring_buff *ring, size_t bytes);

void ring_dump(struct ring_buff *ring, uint32_t *tail);

#define ring_consume_and_update_oldest_tail(\
		___ring,    /* the ring_buff object */ \
		___type,    /* type of objects with tails */ \
		___ptail,   /* ptr to tail of obj with tail doing consuming */ \
		___count,   /* count of payload objects being consumed */ \
		___list_head,	/* head of list of objects with tails */ \
		___mtail,   /* member name of tail in ___type */ \
		___mlist  /* member name of next list member ptr in ___type */ \
	) { \
		int ___n, ___m; \
	\
	___n = ring_get_oldest_tail(___ring) == *(___ptail); \
	ring_consume(___ring, ___ptail, NULL, ___count); \
	if (___n) { \
		uint32_t ___oldest; \
		___n = 0; \
		___oldest = *(___ptail); \
		lws_start_foreach_llp(___type **, ___ppss, ___list_head) { \
			___m = ring_get_count_waiting_elements( \
					___ring, &(*___ppss)->tail); \
			if (___m >= ___n) { \
				___n = ___m; \
				___oldest = (*___ppss)->tail; \
			} \
		} lws_end_foreach_llp(___ppss, ___mlist); \
	\
		ring_update_oldest_tail(___ring, ___oldest); \
	} \
}

/*
 * This does the same as the ring_consume_and_update_oldest_tail()
 * helper, but for the simpler case there is only one consumer, so one
 * tail, and that tail is always the oldest tail.
 */

#define ring_consume_single_tail(\
		___ring,  /* the ring_buff object */ \
		___ptail, /* ptr to tail of obj with tail doing consuming */ \
		___count  /* count of payload objects being consumed */ \
	) { \
	ring_consume(___ring, ___ptail, NULL, ___count); \
	ring_update_oldest_tail(___ring, *(___ptail)); \
}

#ifdef __cplusplus
}
#endif
#endif