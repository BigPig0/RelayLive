#ifndef AVFORMAT_RTP
#define AVFORMAT_RTP

#include "avtypes.h"

typedef struct _rtp_ rtp;
typedef void(*rtp_callback)(void *user, AV_BUFF buff);

/**
 * ����һ��rtp����Ļ������
 */
extern rtp* create_rtp(void *user, rtp_callback cb);

/**
 * rtp���������������
 */
extern void destory_rtp(rtp *h);

/**
 * rtp���,rtp��ͨ���ص����
 */
extern void rtp_packet(rtp *h, AV_BUFF buff);

/**
 * rtp�����֡����ͨ���ص����
 */
extern void rtp_unpacket(rtp *h, AV_BUFF buff);

#endif