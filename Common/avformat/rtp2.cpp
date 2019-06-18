#include "stdafx.h"
#include "rtp2.h"
#include "utilc.h"
#include "buflist.h"

typedef struct _rtp_ {
    void        *user;
    buflist_t   *buff;
    rtp_callback cb;    //»Øµ÷
} rtp;


rtp* create_rtp(void *user, rtp_callback cb) {
    SAFE_MALLOC(rtp, ret);
    ret->user = user;
    ret->cb   = cb;

    return ret;
}

void destory_rtp(rtp *h) {
    SAFE_FREE(h);
}

void rtp_packet(rtp *h, AV_BUFF buff) {

}

void rtp_unpacket(rtp *h, AV_BUFF buff) {

}