#ifndef AVFORMAT_RTP
#define AVFORMAT_RTP

#include "avtypes.h"

typedef struct _rtp_ rtp;
typedef void(*rtp_callback)(void *user, AV_BUFF buff);

/**
 * 创建一个rtp处理的环境句柄
 */
extern rtp* create_rtp(void *user, rtp_callback cb);

/**
 * rtp环境句柄清理销毁
 */
extern void destory_rtp(rtp *h);

/**
 * rtp打包,rtp包通过回调输出
 */
extern void rtp_packet(rtp *h, AV_BUFF buff);

/**
 * rtp解包，帧数据通过回调输出
 */
extern void rtp_unpacket(rtp *h, AV_BUFF buff);

#endif