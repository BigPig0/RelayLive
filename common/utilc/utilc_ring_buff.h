/*!
 * \file ring_buff.h
 *
 * \author wlla ��libwebsockets��ֲ����
 * \date ʮһ�� 2018
 *
 * �����Ļ��λ�������ͬһʱ��һ���̲߳��룬һ���̶߳�ȡ
 * Ԫ��element ringbuff��ÿһ��С�ε����ݣ�ringbuff���ڴ�ֳ���n��λ�ã�ÿ��λ�õ����ݳ�ΪԪ�أ�
 * * ÿ��λ�õ�״̬��Ϊfree��waiting��
 * * * free ָû���ڸ�λ�ô��Ԫ�����ݣ�
 * * * waiting ָ��λ���Ѵ��Ԫ�����ݣ��ȴ�����ȡ��
 * ͷhead �������Ԫ�ص�λ�ã������µ�waiting��λ��
 * βoldest_tail ʣ�������һ��waitingԪ�ص�λ��
 * ����insert ָ��Ԫ�����ݿ�����head֮ǰ��״̬Ϊfree������n��λ�ã�����״̬��Ϊwaiting
 * ����consume ָ��oldest_tail֮ǰ������n��״̬Ϊwaitingλ��ȡ��Ԫ������
 *
 * ������inserter ��ringbuff�в���Ԫ�ص�����
 * ������consumer ��ringbuff�ж�ȡ��ɾ��Ԫ�ص�����
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
 * ����ringbuff
 * @param element_len ����Ԫ�صĴ�С
 * @param count Ԫ�ظ���
 * @param destroy_element Ԫ��ɾ���ķ���
 */
ring_buff_t* create_ring_buff(size_t element_len, size_t count, void(*destroy_element)(void *));

/**
 * ����ringbuff
 */
void destroy_ring_buff(struct ring_buff *ring);

/** ��ȡringbuff��free״̬��λ�õĸ��� */
size_t ring_get_count_free_elements(struct ring_buff *ring);

/** ��ȡringbuff��waiting״̬��λ�ø��� */
size_t ring_get_count_waiting_elements(struct ring_buff *ring, uint32_t *tail);

/** ��src����max_count��Ԫ�ؾ����ܵĲ��뵽ring�У�ֱ��srcȫ�������ring����������ʵ�ʲ���ĸ��� */
size_t ring_insert(struct ring_buff *ring, const void *src, size_t max_count);

/**
 * ��ring����tailָ����λ�þ����ܵĶ�ȡmax_count��Ԫ�ؿ�����dest��
 * @param tail[inout] consumer�м�¼��tail�ı�����ָ�룬���tailΪ�գ���ʹ��ring�е�oldest_tail
 * @param dest[inout] �����ȡ�����ݣ���ring���ڴ��п�����dest��Ϊ��ʱ�����п��������ǻ����tail
 * @param max_count[in] ����ȡ���ĸ�������һ����ȡ����ô���
 * @return ʵ��ȡ���ĸ���
 * @remark tail��Ϊ��ʱ��ֻ����tail��ֵ��ring�е�oldest_tail�����¡�tailΪ�յ�ʱ������oldest_tail
 */
size_t ring_consume(struct ring_buff *ring, uint32_t *tail, void *dest, size_t max_count);

/** ��ring����tailָ����λ���ҵ���һ��waiting��Ԫ��
 * ���ú�ʹ��lws_ring_consume(ring, &tail, NULL, 1)������consumer�м�¼��tail
 */
const void* ring_get_element(struct ring_buff *ring, uint32_t *tail);

/**
 * ��ring�б�tail�����Ԫ��״̬����λfree��
 * Ҳ���ǽ�ring��oldest_tail��ֵ����Ϊtail��
 * ���ж��consumerʹ�����ringbuffʱ���κ�һ��consumer��������Ҫ��Ԫ��ͨ�����apiɾ��
 * @param tail ɾ����Ԫ��λ��(ͨ����������consumer�м�¼��tail��Ϣȡ��Сֵ�õ�)
 */
void ring_update_oldest_tail(struct ring_buff *ring, uint32_t tail);

/** ��ȡringbuff�е�һ��waitingԪ�ص�λ��(oldest_tail��ֵ) */
uint32_t ring_get_oldest_tail(struct ring_buff *ring);

/**
 * ��ȡfreeԪ�صĿ�ʼλ�ú��ֽڴ�С
 * @param start ���freeԪ�ص�λ��
 * @bytes �������freeԪ�ص��ֽ���
 * ʹ�����api����ֱ��д��Ԫ�أ������þ����ڴ濽��
 */
int ring_next_linear_insert_range(struct ring_buff *ring, void **start, size_t *bytes);

/**
 * ֱ����ringbuff��freeλ��д�����ݺ�ͨ����api����ring��header��λ��
 * ring_next_linear_insert_range �� ring_bump_head���ʹ��
 */
void ring_bump_head(struct ring_buff *ring, size_t bytes);

/** ��ӡring�е���Ϣ */
void ring_dump(struct ring_buff *ring, uint32_t *tail);

/**
 * consumer��ring�ж�ȡԪ�أ�������ring��״̬
 * ���consumer��tail��ͬ��ring��oldest_tail����Ҫ��������consumer��tailֵ������ring��oldest_tail
 * ʹ���������Ҫconsumer������ָ����ʽ���������洢
 *
 * consumer�ṹ�嶨������:
 * struct ___type {
 *     uint32_t ___mtail;
 *     struct ___type *___mlist;
 *     ***
 * }
 *
 * struct ___type *___list_head;
 *
 * @param ___ring   ringbuff�����ָ��
 * @param ___type   consumer������Ľṹ��������
 * @param ___ptail  consumer�е�tail�����ĵ�ַ
 * @param ___count  consumer��Ҫɾ����Ԫ�ظ���
 * @param ___list_head  consumer����
 * @param ___mtail  ___type�е�tail��������
 * @param ___mlist  ___type�е�����next��������
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
 * �� ring_consume_and_update_oldest_tail()����һ��
 * �����ڸ��򵥵�ֻ��һ��consumer, �Լ�һ��tail�����
 * ���tail��Զ����ring�� oldest tail.
 */

#define ring_consume_single_tail(\
		___ring, \
		___ptail, \
		___count \
	) { \
	ring_consume(___ring, ___ptail, NULL, ___count); \
	ring_update_oldest_tail(___ring, *(___ptail)); \
}


/**
 * �ڲ�ά��tail�ļ򵥵�ringbuff
 */
#define simple_ring_insert(ring, src) ring_insert(ring, src, 1);
#define simple_ring_get_element(ring) ring_get_element(ring, NULL);
#define simple_ring_cosume(ring) ring_consume(ring, NULL, NULL, 1);

#ifdef __cplusplus
}
#endif
#endif