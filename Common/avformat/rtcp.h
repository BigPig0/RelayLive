#ifndef AVFORMAT_RTCP
#define AVFORMAT_RTCP

#include "avtypes.h"

typedef struct _rtcp_ rtcp;
typedef void(*rtcp_callback)(void *user, AV_BUFF buff);

extern rtcp* create_rtcp(void *user, rtcp_callback cb);

extern void destory_rtcp(rtcp *h);

extern void rtcp_send_rtppkt(rtcp *h, int size);

extern void rtcp_recv_rtppkt(rtcp *h, int size);

extern void rtcp_recv(rtcp *h, AV_BUFF buff);

#endif