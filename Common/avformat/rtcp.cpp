#include "stdafx.h"
#include "rtcp.h"
#include "uv.h"
#include "cstl.h"
#include "utilc.h"
#include "Log.h"
#include <math.h>

#ifdef _MSC_VER   
static double drand48() { return (((double)rand()) / RAND_MAX); }   
static void srand48(long sv) { srand((unsigned int) sv); }   
#endif   

#define RTCP_BW         (160 * 50) // FIXME: default bandwidth (octet/second)   
#define CODEC_RATE      8000        // FIXME   
#define RTP_SEQ_MOD     (1 << 16)   
#define MAX_DROPOUT     3000   
#define MAX_MISORDER    100   
#define MIN_SEQUENTIAL  2   

typedef enum _rtcp_event_ {   
    EVENT_BYE,   
    EVENT_REPORT,   
    EVENT_RTP   
} rtcp_event_t;   

typedef enum _packet_type_ {   
    PACKET_RTCP_REPORT,   
    PACKET_BYE,   
    PACKET_RTP,   
} packet_type_t;   

typedef struct _rtcp_source_ {   
    uint32_t ssrc;           /* source's ssrc */   
    uint16_t max_seq;        /* highest seq. number seen */   
    uint32_t cycles;         /* shifted count of seq. number cycles */   
    uint32_t base_seq;       /* base seq number */   
    uint32_t bad_seq;        /* last 'bad' seq number + 1 */   
    uint32_t probation;      /* sequ. packets till source is valid */   
    uint32_t received;       /* packets received */   
    uint32_t expected_prior; /* packet expected at last interval */   
    uint32_t received_prior; /* packet received at last interval */   
    uint32_t transit;        /* relative trans time for prev pkt */   
    double   jitter;         /* estimated jitter */   

    uint32_t base_ts;        /* base timestamp */   
    uint32_t max_ts;         /* highest timestamp number seen */   
    uint32_t rate;           /* codec sampling rate */   

    uint32_t ntp_msw;        /* last received NTP timestamp from RTCP sender */   
    uint32_t ntp_lsw;        /* last received NTP timestamp from RTCP sender */   
    uint64_t dlsr;           /* delay since last SR */   
} rtcp_source_t;   

typedef enum _rtcp_sdes_item_type_ { 
    trtp_rtcp_sdes_item_type_end = 0, /**< end of SDES list */ 
    trtp_rtcp_sdes_item_type_cname = 1, /**< canonical name*/ 
    trtp_rtcp_sdes_item_type_name = 2, /**< user name */ 
    trtp_rtcp_sdes_item_type_email = 3, /**< user's electronic mail address*/ 
    trtp_rtcp_sdes_item_type_phone = 4, /**< user's phone number  */ 
    trtp_rtcp_sdes_item_type_loc = 5, /**< geographic user location*/ 
    trtp_rtcp_sdes_item_type_tool = 6, /**< name of application or tool*/ 
    trtp_rtcp_sdes_item_type_note = 7, /**< notice about the source*/ 
    trtp_rtcp_sdes_item_type_priv = 8, /**< private extensions*/ 
} rtcp_sdes_item_type_t; 

typedef struct _rtcp_sdes_item_ { 
    rtcp_sdes_item_type_t type; 
    char     *data; 
    uint32_t size;
} rtcp_sdes_item_t; 

typedef struct _rtcp_sdes_chunck_ {
    uint32_t ssrc; 
    list_t   *items;    //list<trtp_rtcp_sdes_item_t*>
} rtcp_sdes_chunck_t;

typedef struct _rtcp_session_ {
    uv_tcp_t       *tcp;
    uv_udp_t       *udp;
    uv_timer_t     report_timer;
    bool_t         started;
    const struct sockaddr  *remote_addr;   
   
    //const void* callback_data;   
    //trtp_rtcp_cb_f callback;   
 
    rtcp_source_t *source_local; /**< local source */   
    list_t        *sdes_items;  //list<rtcp_sdes_chunck_t>
    uint64_t      time_start; /**< Start time in millis (NOT in NTP unit yet) */   
       
    // <RTCP-FB>   
    uint8_t       fir_seqnr;   
    // </RTCP-FB>   
   
    // <sender>   
    string_t      *name;   
    bool_t        is_cname_defined;   
    uint32_t      packets_count;   
    uint32_t      octets_count;   
    // </sender>   
   
    // <others>   
    time_t        tp; /**< the last time an RTCP packet was transmitted; */   
    time_t        tc; /**< the current time */   
    time_t        tn; /**< the next scheduled transmission time of an RTCP packet */   
    int32_t       pmembers; /**< the estimated number of session members at the time tn was last recomputed */   
    int32_t       members; /**< the most current estimate for the number of session members */   
    int32_t       senders; /**< the most current estimate for the number of senders in the session */   
    double        rtcp_bw; /**< The target RTCP bandwidth, i.e., the total bandwidth  
      that will be used for RTCP packets by all members of this session,  
      in octets per second.  This will be a specified fraction of the  
      "session bandwidth" parameter supplied to the application at  
      startup*/   
    bool_t        we_sent; /**< Flag that is true if the application has sent data since the 2nd previous RTCP report was transmitted */   
    double        avg_rtcp_size; /**< The average compound RTCP packet size, in octets,  
      over all RTCP packets sent and received by this participant.  The  
      size includes lower-layer transport and network protocol headers  
      (e.g., UDP and IP) as explained in Section 6.2*/   
    bool_t        initial; /**< Flag that is true if the application has not yet sent an RTCP packet */   
    // </others>   
   
    list_t        *sources; 
} rtcp_session_t; 

rtcp_source_t* rtcp_source_create(uint32_t ssrc, uint16_t seq, uint32_t ts){
    SAFE_MALLOC(rtcp_source_t, source);

    source->ssrc = ssrc;
    source->base_seq = seq;
    source->max_seq = seq;
    source->max_seq = seq - 1;
    source->base_ts = ts;
    source->max_ts = ts;
    source->bad_seq = RTP_SEQ_MOD + 1;   /* so seq == bad_seq is false */     
    source->probation = MIN_SEQUENTIAL;   
    source->rate = CODEC_RATE;//FIXME   

    return source;   
}

int rtcp_session_add_source(rtcp_session_t* session, rtcp_source_t* source)   
{   
    if(!session || !source){
        Log::debug("Invalid parameter");
        return -1;
    }

    if(session->sources == NULL) {
        session->sources = create_list(void*);
        list_init(session->sources);
    }

    list_push_back(session->sources, source);

    return 0;   
} 

rtcp_session_t* rtcp_session_create(uint32_t ssrc) {
    SAFE_MALLOC(rtcp_session_t, session);

    session->source_local = rtcp_source_create(ssrc, 0, 0);

    rtcp_session_add_source(session, session->source_local);   
    session->initial = true;   
    session->we_sent = false;   
    session->senders = 1;   
    session->members = 1;   
    session->rtcp_bw = RTCP_BW;//FIXME: as parameter from the code, Also added possiblities to update this value

    return session;
} 

/**
 * 计算下一次发送时间间隔
 */
static double rtcp_interval(int32_t members,   
                     int32_t senders,   
                     double rtcp_bw,   
                     int32_t we_sent,   
                     double avg_rtcp_size,   
                     bool_t initial) {   
    double const RTCP_MIN_TIME = 5.;
    double const RTCP_SENDER_BW_FRACTION = 0.25;
    double const RTCP_RCVR_BW_FRACTION = (1-RTCP_SENDER_BW_FRACTION);
    double const COMPENSATION = 2.71828 - 1.5;
   
    double t;                   /* interval */   
    double rtcp_min_time = RTCP_MIN_TIME;   
    int n;                      /* no. of members for computation */   
   
    if (initial) {   
        rtcp_min_time /= 2;   
    }

    n = members;   
    if (senders <= members * RTCP_SENDER_BW_FRACTION) {   
        if (we_sent) {   
            rtcp_bw *= RTCP_SENDER_BW_FRACTION;   
            n = senders;   
        } else {   
            rtcp_bw *= RTCP_RCVR_BW_FRACTION;   
            n -= senders;   
        }   
    }

    t = avg_rtcp_size * n / rtcp_bw;   
    if (t < rtcp_min_time) t = rtcp_min_time;   

    t = t * (drand48() + 0.5);   
    t = t / COMPENSATION;   
   
    return (t * 1000);   
}  

static void OnExpire(rtcp_session_t* session, rtcp_event_t e) {   
    double t;     /* Interval */   
    double tn;    /* Next transmit time */   
    double tc;   
   
    /* In the case of a BYE, we use "timer reconsideration" to  
    * reschedule the transmission of the BYE if necessary */   
    if (e == EVENT_BYE) {   
        t = rtcp_interval(session->members,   
            session->senders,   
            session->rtcp_bw,   
            session->we_sent,   
            session->avg_rtcp_size,   
            session->initial);   
        tn = session->tp + t;   
        if (tn <= time(NULL)) {   
            //SendBYEPacket(session, e);   
        } else {   
            //Schedule(session, 0, e); 
        }   
   
    } else if (e == EVENT_REPORT) {
        t = rtcp_interval(session->members,   
            session->senders,   
            session->rtcp_bw,   
            session->we_sent,   
            session->avg_rtcp_size,   
            session->initial);   
        tn = session->tp + t;   
        if (tn <= (tc = time(NULL))) {
            size_t SentPacketSize = 0;//SendRTCPReport(session, e);   
            session->avg_rtcp_size = (1./16.)*SentPacketSize + (15./16.)*(session->avg_rtcp_size);   
            session->tp = tc;   
   
            /* We must redraw the interval.  Don't reuse the  
            one computed above, since its not actually  
            distributed the same, as we are conditioned  
            on it being small enough to cause a packet to  
            be sent */   
   
            t = rtcp_interval(session->members,   
                session->senders,   
                session->rtcp_bw,   
                session->we_sent,   
                session->avg_rtcp_size,   
                session->initial);   

            //Schedule(session, t, e);   
            session->initial = false;   
        } else {
            //Schedule(session, 0, e);   
        }   
        session->pmembers = session->members;   
    }   
}

static void uv_report_cb(uv_timer_t *h){
    rtcp_session_t* session = (rtcp_session_t*)h->data;
    OnExpire(session, EVENT_REPORT);
}

int rtcp_session_start(rtcp_session_t* self, uv_loop_t *uv, const struct sockaddr * remote_addr)   
{   
    int ret;   

    if(!self){   
        Log::error("Invalid parameter");   
        return -1;   
    }   
    if(self->started){   
        Log::warning("Already started");   
        return 0;   
    }   

    // start global timer manager
    uv_timer_init(uv, &self->report_timer);
    self->report_timer.data = self;
    if((ret = uv_timer_start(&self->report_timer, uv_report_cb,1,1))){   
        Log::error("Failed to start timer");   
        return ret;   
    }   
    self->started = true;   
  
    self->remote_addr = remote_addr;   

    // Send Initial RR (mandatory)   
    //Schedule(self, 0., EVENT_REPORT);   

    // set start time   
    self->time_start = time(NULL);   

    return ret;   
}

//////////////////////////////////////////////////////////////////////////
char* create_sr(uint32_t ssrc, uint32_t timestamp, int sendPacketsNum, int sendBytesNum)
{
    char *buffer = (char*)calloc(1, 40);
    *(uint8_t*)&buffer[0] = (uint8_t)(2 << 6);                              //  V=2,  P=RC=0
    *(uint8_t*)&buffer[1] = (uint8_t)200;                                 // PT=SR=200
    *(uint16_t*)&buffer[2] = (uint16_t)htons(6);                                         // length (7 32-bit words, minus one)
    *(uint32_t*)&buffer[4] = (uint32_t)htonl(ssrc);
    *(uint32_t*)&buffer[8] = (uint32_t)(timestamp >> 32);        // High 32-bits
    *(uint32_t*)&buffer[12] = (uint32_t)(timestamp & 0xFFFFFFFF); // Low 32-bits
    *(uint32_t*)&buffer[16] = (uint32_t)htonl(timestamp);
    *(uint32_t*)&buffer[20] = (uint32_t)htonl(sendPacketsNum);
    *(uint32_t*)&buffer[24] = (uint32_t)htonl(sendBytesNum);
    *(uint8_t*)&buffer[28] = (uint8_t)(2 << 6 | 1);                          //  V=2, P=0, SC=1
    *(uint8_t*)&buffer[29] = (uint8_t)202;
    *(uint16_t*)&buffer[30] = (uint16_t)htons(2);
    *(uint32_t*)&buffer[32] = (uint32_t)htonl(ssrc);
    *(uint8_t*)&buffer[36] = (uint8_t)1;
    *(uint8_t*)&buffer[37] = (uint8_t)0;
    *(uint16_t*)&buffer[38] = (uint16_t)0;
    return buffer;
}

/*  
        0                   1                   2                   3  
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1  
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  
header |V=2|P|    RC   |   PT=SR=200   |             length            |  
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  
*/   

typedef struct _rtcp_ {
    void *user;
    rtcp_callback cb;
} rtcp;

rtcp* create_rtcp(void *user, rtcp_callback cb) {
    SAFE_MALLOC(rtcp, ret);
    ret->user = user;
    ret->cb = cb;

    return ret;
}

void destory_rtcp(rtcp *h) {
    SAFE_FREE(h);
}

void rtcp_send_rtppkt(rtcp *h, int size) {

}

void rtcp_recv_rtppkt(rtcp *h, int size) {

}

void rtcp_recv(rtcp *h, AV_BUFF buff) {

}