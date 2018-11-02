/*
  eXosip - This is the eXtended osip library.
  Copyright (C) 2001-2015 Aymeric MOIZARD amoizard@antisip.com
  
  eXosip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  eXosip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  In addition, as a special exception, the copyright holders give
  permission to link the code of portions of this program with the
  OpenSSL library under certain conditions as described in each
  individual source file, and distribute linked combinations
  including the two.
  You must obey the GNU General Public License in all respects
  for all of the code used other than OpenSSL.  If you modify
  file(s) with this exception, you may extend this exception to your
  version of the file(s), but you are not obligated to do so.  If you
  do not wish to do so, delete this exception statement from your
  version.  If you delete this exception statement from all source
  files in the program, then also delete it here.
*/


#ifndef __EXOSIP2_H__
#define __EXOSIP2_H__

#if defined (HAVE_CONFIG_H)
#include <exosip-config.h>
#endif

#if defined(__PALMOS__) && (__PALMOS__ >= 0x06000000)
#define HAVE_CTYPE_H 1
#define HAVE_STRING_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_TIME_H 1
#define HAVE_STDARG_H 1

#elif defined(__VXWORKS_OS__) || defined(__rtems__)
#define HAVE_STRING_H 1
#define HAVE_TIME_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_STDARG_H 1

#elif defined _WIN32_WCE

#define HAVE_CTYPE_H 1
#define HAVE_STRING_H 1
#define HAVE_TIME_H 1
#define HAVE_STDARG_H 1

#define snprintf  _snprintf

#elif defined(WIN32)

#define HAVE_CTYPE_H 1
#define HAVE_STRING_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_TIME_H 1
#define HAVE_STDARG_H 1
#define HAVE_SYS_STAT_H

#define snprintf _snprintf

/* use win32 crypto routines for random number generation */
/* only use for vs .net (compiler v. 1300) or greater */
#if _MSC_VER >= 1300
#define WIN32_USE_CRYPTO 1
#endif

#endif

#if defined (HAVE_STRING_H)
#include <string.h>
#elif defined (HAVE_STRINGS_H)
#include <strings.h>
#else
#include <string.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#if defined (HAVE_LIMITS_H)
#include <limits.h>
#endif

#if defined (HAVE_SYS_TYPES_H)
#include <sys/types.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#if defined (HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif

#if defined(__arc__)
#include "includes_api.h"
#include "os_cfg_pub.h"
#include <posix_time_pub.h>
#define USE_GETHOSTBYNAME
#endif

#ifdef __PSOS__
#define VA_START(a, f)  va_start(a, f)
#include "pna.h"
#include "stdlib.h"
#include "time.h"
#define timercmp(tvp, uvp, cmp) \
((tvp)->tv_sec cmp (uvp)->tv_sec || \
(tvp)->tv_sec == (uvp)->tv_sec && (tvp)->tv_usec cmp (uvp)->tv_usec)
#define snprintf  osip_snprintf
#ifndef INT_MAX
#define INT_MAX 0x7FFFFFFF
#endif
#endif

#ifdef _WIN32_WCE
#include <winsock2.h>
#include <osipparser2/osip_port.h>
#include <ws2tcpip.h>
#elif WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <osip2/osip.h>
#include <osip2/osip_dialog.h>

#include <eXosip2/eXosip.h>
#include "eXtransport.h"

#include "jpipe.h"

#define EXOSIP_VERSION	"5.0.0"

#ifdef WIN32
#define SOCKET_TYPE SOCKET
#else
#define SOCKET_TYPE int
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(USE_GETHOSTBYNAME)

#define NI_MAXHOST      1025
#define NI_MAXSERV      32
#define NI_NUMERICHOST  1

#define PF_INET6        AF_INET6

  struct sockaddr_storage {
    unsigned char sa_len;
    unsigned char sa_family;    /* Address family AF_XXX */
    char sa_data[14];           /* Protocol specific address */
  };

  struct addrinfo {
    int ai_flags;               /* Input flags.  */
    int ai_family;              /* Protocol family for socket.  */
    int ai_socktype;            /* Socket type.  */
    int ai_protocol;            /* Protocol for socket.  */
    socklen_t ai_addrlen;       /* Length of socket address.  */
    struct sockaddr *ai_addr;   /* Socket address for socket.  */
    char *ai_canonname;         /* Canonical name for service location.  */
    struct addrinfo *ai_next;   /* Pointer to next in list.  */
  };

  void _eXosip_freeaddrinfo (struct addrinfo *ai);

#else

#define _eXosip_freeaddrinfo freeaddrinfo

#endif

  void _eXosip_update (struct eXosip_t *excontext);
  void _eXosip_wakeup (struct eXosip_t *excontext);

#ifndef DEFINE_SOCKADDR_STORAGE
#define __eXosip_sockaddr sockaddr_storage
#else
  struct __eXosip_sockaddr {
    u_char ss_len;
    u_char ss_family;
    u_char padding[128 - 2];
  };
#endif

  typedef struct eXosip_dialog_t eXosip_dialog_t;

  struct eXosip_dialog_t {

    int d_id;
    osip_dialog_t *d_dialog;    /* active dialog */

    time_t d_session_timer_start;       /* session-timer helper */
    int d_session_timer_length;
    int d_refresher;
    int d_session_timer_use_update;

    time_t d_timer;
    int d_count;
    osip_message_t *d_200Ok;
    osip_message_t *d_ack;

    osip_list_t *d_inc_trs;
    osip_list_t *d_out_trs;
    int d_retry;                /* avoid too many unsuccessful retry */
    int d_mincseq;              /* remember cseq after PRACK and UPDATE during setup */

    eXosip_dialog_t *next;
    eXosip_dialog_t *parent;
  };

  typedef struct eXosip_call_t eXosip_call_t;

  struct eXosip_call_t {

    int c_id;
    eXosip_dialog_t *c_dialogs;
    osip_transaction_t *c_inc_tr;
    osip_transaction_t *c_out_tr;
    int c_retry;                /* avoid too many unsuccessful retry */
    void *external_reference;

    time_t expire_time;

    eXosip_call_t *next;
    eXosip_call_t *parent;
  };


  typedef struct eXosip_reg_t eXosip_reg_t;

  struct eXosip_reg_t {

    int r_id;

    int r_reg_period;           /* delay between registration (modified by server) */
    int r_reg_expire;           /* delay between registration (requested by client) */

    char *r_aor;                /* sip identity */
    char *r_registrar;          /* registrar */
    char *r_contact;            /* list of contacts string */

    char r_line[16];            /* line identifier */
    char r_qvalue[16];          /* the q value used for routing */

    osip_transaction_t *r_last_tr;
    int r_retry;                /* avoid too many unsuccessful retry */
    int r_retryfailover;        /* avoid too many unsuccessful retry */
#define RS_DELETIONREQUIRED 2
#define RS_DELETIONPROCEEDING 3
#define RS_MASQUERADINGREQUIRED 4
#define RS_MASQUERADINGPROCEEDING 5
    int registration_step;      /* registration step for learning contact header binding */
    time_t r_last_deletion;     /* prevent loop for automasquerade: no more than one per minute. */

    struct __eXosip_sockaddr addr;
    socklen_t len;

    eXosip_reg_t *next;
    eXosip_reg_t *parent;
  };


#ifndef MINISIZE

  typedef struct eXosip_subscribe_t eXosip_subscribe_t;

  struct eXosip_subscribe_t {

    int s_id;
    int s_ss_status;
    int s_ss_reason;
    int s_reg_period;
    eXosip_dialog_t *s_dialogs;

    int s_retry;                /* avoid too many unsuccessful retry */
    osip_transaction_t *s_inc_tr;
    osip_transaction_t *s_out_tr;

    eXosip_subscribe_t *next;
    eXosip_subscribe_t *parent;
  };

  typedef struct eXosip_notify_t eXosip_notify_t;

  struct eXosip_notify_t {

    int n_id;
    int n_online_status;

    int n_ss_status;
    int n_ss_reason;
    time_t n_ss_expires;
    eXosip_dialog_t *n_dialogs;

    osip_transaction_t *n_inc_tr;
    osip_transaction_t *n_out_tr;

    eXosip_notify_t *next;
    eXosip_notify_t *parent;
  };

  typedef struct eXosip_pub_t eXosip_pub_t;

  struct eXosip_pub_t {
    int p_id;

    int p_period;               /* delay between registration */
    char p_aor[256];            /* sip identity */
    char p_sip_etag[64];        /* sip_etag from 200ok */

    osip_transaction_t *p_last_tr;
    int p_retry;
    eXosip_pub_t *next;
    eXosip_pub_t *parent;
  };

  int _eXosip_pub_update (struct eXosip_t *excontext, eXosip_pub_t ** pub, osip_transaction_t * tr, osip_message_t * answer);
  int _eXosip_pub_find_by_aor (struct eXosip_t *excontext, eXosip_pub_t ** pub, const char *aor);
  int _eXosip_pub_find_by_tid (struct eXosip_t *excontext, eXosip_pub_t ** pjp, int tid);
  int _eXosip_pub_init (struct eXosip_t *excontext, eXosip_pub_t ** pub, const char *aor, const char *exp);
  void _eXosip_pub_free (struct eXosip_t *excontext, eXosip_pub_t * pub);

#endif

  typedef struct jauthinfo_t jauthinfo_t;

  struct jauthinfo_t {
    char username[50];
    char userid[50];
    char passwd[50];
    char ha1[50];
    char realm[50];
    jauthinfo_t *parent;
    jauthinfo_t *next;
  };

  int _eXosip_create_proxy_authorization_header (osip_proxy_authenticate_t * wa, const char *rquri, const char *username, const char *passwd, const char *ha1, osip_proxy_authorization_t ** auth, const char *method, const char *pszCNonce, int iNonceCount);
  int _eXosip_store_nonce (struct eXosip_t *excontext, const char *call_id, osip_proxy_authenticate_t * wa, int answer_code);
  int _eXosip_delete_nonce (struct eXosip_t *excontext, const char *call_id);

  eXosip_event_t *_eXosip_event_init_for_call (int type, eXosip_call_t * jc, eXosip_dialog_t * jd, osip_transaction_t * tr);

#ifndef MINISIZE
  eXosip_event_t *_eXosip_event_init_for_subscription (int type, eXosip_subscribe_t * js, eXosip_dialog_t * jd, osip_transaction_t * tr);
  eXosip_event_t *_eXosip_event_init_for_notify (int type, eXosip_notify_t * jn, eXosip_dialog_t * jd, osip_transaction_t * tr);
#endif

  eXosip_event_t *_eXosip_event_init_for_reg (int type, eXosip_reg_t * jr, osip_transaction_t * tr);
  eXosip_event_t *_eXosip_event_init_for_message (int type, osip_transaction_t * tr);

  int _eXosip_event_init (eXosip_event_t ** je, int type);
  void _eXosip_report_call_event (struct eXosip_t *excontext, int evt, eXosip_call_t * jc, eXosip_dialog_t * jd, osip_transaction_t * tr);
  void _eXosip_report_event (struct eXosip_t *excontext, eXosip_event_t * je, osip_message_t * sip);
  int _eXosip_event_add (struct eXosip_t *excontext, eXosip_event_t * je);

  typedef void (*eXosip_callback_t) (int type, eXosip_event_t *);

  char *_eXosip_strdup_printf (const char *fmt, ...);

  char *_eXosip_transport_protocol (osip_message_t * msg);
  int _eXosip_find_protocol (osip_message_t * msg);
  int setsockopt_ipv6only (int sock);


#ifndef MAX_EXOSIP_DNS_ENTRY
#define MAX_EXOSIP_DNS_ENTRY 10
#endif

#ifndef MAX_EXOSIP_ACCOUNT_INFO
#define MAX_EXOSIP_ACCOUNT_INFO 10
#endif

#ifndef MAX_EXOSIP_HTTP_AUTH
#define MAX_EXOSIP_HTTP_AUTH 100
#endif

struct eXosip_counters {
  float current_average;
  unsigned int num_entries;
  unsigned short period; /* total max duration */
  unsigned short interval; /* minimum interval */
  unsigned short *values;
  struct timeval *times;
  unsigned int index_last;
  unsigned int total_values;
};

  typedef struct eXosip_t eXosip_t;

  struct eXosip_t {
#ifndef MINISIZE
    struct eXosip_stats statistics;
    struct eXosip_counters average_transactions;
    struct eXosip_counters average_registrations;
    struct eXosip_counters average_calls;
    struct eXosip_counters average_publications;
    struct eXosip_counters average_subscriptions;
    struct eXosip_counters average_insubscriptions;
#endif

    struct eXtl_protocol eXtl_transport;
    void *eXtludp_reserved;
    void *eXtltcp_reserved;
#ifndef DISABLE_TLS
    void *eXtltls_reserved;
    void *eXtldtls_reserved;
#endif
    void *tunnel_handle;
    char transport[10];
    char *user_agent;

    eXosip_reg_t *j_reg;        /* my registrations */
    eXosip_call_t *j_calls;     /* my calls        */
#ifndef MINISIZE
    eXosip_subscribe_t *j_subscribes;   /* my friends      */
    eXosip_notify_t *j_notifies;        /* my susbscribers */
    eXosip_pub_t *j_pub;        /* my publications  */
#endif
    osip_list_t j_transactions;

    osip_t *j_osip;
    int j_stop_ua;
#ifndef OSIP_MONOTHREAD
    void *j_cond;
    void *j_mutexlock;
    void *j_thread;
    jpipe_t *j_socketctl;
    jpipe_t *j_socketctl_event;
#endif
    int max_message_to_read;
    long int max_read_timeout;
    
    osip_fifo_t *j_events;

    jauthinfo_t *authinfos;

    struct timeval cc_timer;
    struct timeval ka_timer;
    int ka_interval;
    char ka_crlf[5];
    int ka_options;
    int learn_port;
    int use_rport;
    int remove_prerouteset;
    int dns_capabilities;
    int enable_dns_cache;
    int dscp;
    int register_with_date;
    int autoanswer_bye;
    int ipv6_enable;
    char ipv4_for_gateway[256];
    char ipv6_for_gateway[256];
    struct eXosip_dns_cache dns_entries[MAX_EXOSIP_DNS_ENTRY];
    struct eXosip_account_info account_entries[MAX_EXOSIP_ACCOUNT_INFO];
    struct eXosip_http_auth http_auths[MAX_EXOSIP_HTTP_AUTH];

    /* udp pre-config */
    char udp_firewall_ip[64];
    char udp_firewall_port[10];

    /* tcp pre-config */
    char tcp_firewall_ip[64];
    char tcp_firewall_port[10];

    /* tls pre-config */
    char tls_firewall_ip[64];
    char tls_firewall_port[10];
    int tls_verify_client_certificate;
    eXosip_tls_ctx_t eXosip_tls_ctx_params;
    char tls_local_cn_name[128];
    char tls_client_local_cn_name[128];

    /* dtls pre-config */
    char dtls_firewall_ip[64];
    char dtls_firewall_port[10];

    CbSipCallback cbsipCallback;
    int masquerade_via;
    int auto_masquerade_contact;
    int reuse_tcp_port;
    int use_ephemeral_port;
    int enable_outbound;
    char oc_local_address[64];
    int oc_local_port_range[2];
    int oc_local_port_current;

    CbSipWakeLock cbsipWakeLock;
    int outgoing_wake_lock_state;
    int incoming_wake_lock_state;

    char sip_instance[37]; /* can only be used if ONE excontext is used for ONE registration only */
    char default_contact_displayname[256];
  };

  int _eXosip_guess_ip_for_via (struct eXosip_t *excontext, int family, char *address, int size);
  int _eXosip_guess_ip_for_destination (struct eXosip_t *excontext, int family, char *destination, char *address, int size);
  int _eXosip_guess_ip_for_destinationsock (struct eXosip_t *excontext, int family, int proto, struct sockaddr_storage *udp_local_bind, int sock, char *destination, char *address, int size);

  int _eXosip_closesocket(SOCKET_TYPE sock);
  int _eXosip_getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags);
  int _eXosip_getport(const struct sockaddr *sa, socklen_t salen);
  int _eXosip_get_addrinfo (struct eXosip_t *excontext, struct addrinfo **addrinfo, const char *hostname, int service, int protocol);

  int _eXosip_set_callbacks (osip_t * osip);
  int _eXosip_snd_message (struct eXosip_t *excontext, osip_transaction_t * tr, osip_message_t * sip, char *host, int port, int out_socket);
  char *_eXosip_malloc_new_random (void);
  void _eXosip_delete_reserved (osip_transaction_t * transaction);

  int _eXosip_dialog_init_as_uac (eXosip_dialog_t ** jd, osip_message_t * _200Ok);
  int _eXosip_dialog_init_as_uas (eXosip_dialog_t ** jd, osip_message_t * _invite, osip_message_t * _200Ok);
  void _eXosip_dialog_free (struct eXosip_t *excontext, eXosip_dialog_t * jd);

  int _eXosip_generating_request_out_of_dialog (struct eXosip_t *excontext, osip_message_t ** dest, const char *method, const char *to, const char *from, const char *proxy);
  int _eXosip_generating_publish (struct eXosip_t *excontext, osip_message_t ** message, const char *to, const char *from, const char *route);
  int _eXosip_generating_cancel (struct eXosip_t *excontext, osip_message_t ** dest, osip_message_t * request_cancelled);
  int _eXosip_generating_bye (struct eXosip_t *excontext, osip_message_t ** bye, osip_dialog_t * dialog);
  int _eXosip_request_viamanager(struct eXosip_t *excontext, osip_transaction_t * tr, osip_message_t * sip, int proto, struct sockaddr_storage *udp_local_bind, int local_port, int sock, char *host);
  int _eXosip_message_contactmanager(struct eXosip_t *excontext, osip_transaction_t * tr, osip_message_t * sip, int proto, struct sockaddr_storage *udp_local_bind, int local_port, int sock, char *host);
  
  int _eXosip_update_top_via (struct eXosip_t *excontext, osip_message_t * sip);
  int _eXosip_request_add_via (struct eXosip_t *excontext, osip_message_t * request);

  void _eXosip_mark_all_registrations_expired (struct eXosip_t *excontext);
  void _eXosip_mark_registration_expired (struct eXosip_t *excontext, const char *call_id);
  int _eXosip_check_allow_header (eXosip_dialog_t * jd, osip_message_t * message);

  int _eXosip_add_authentication_information (struct eXosip_t *excontext, osip_message_t * req, osip_message_t * last_response);
  int _eXosip_reg_find (struct eXosip_t *excontext, eXosip_reg_t ** reg, osip_transaction_t * tr);
  int _eXosip_reg_find_id (struct eXosip_t *excontext, eXosip_reg_t ** reg, int rid);
  int _eXosip_reg_init (struct eXosip_t *excontext, eXosip_reg_t ** jr, const char *from, const char *proxy, const char *contact);
  void _eXosip_reg_free (struct eXosip_t *excontext, eXosip_reg_t * jreg);

  int _eXosip_call_transaction_find (struct eXosip_t *excontext, int tid, eXosip_call_t ** jc, eXosip_dialog_t ** jd, osip_transaction_t ** tr);
  int _eXosip_call_retry_request (struct eXosip_t *excontext, eXosip_call_t * jc, eXosip_dialog_t * jd, osip_transaction_t * out_tr);
  int _eXosip_transaction_find (struct eXosip_t *excontext, int tid, osip_transaction_t ** transaction);
  int _eXosip_call_dialog_find (struct eXosip_t *excontext, int jid, eXosip_call_t ** jc, eXosip_dialog_t ** jd);
  int _eXosip_call_find (struct eXosip_t *excontext, int cid, eXosip_call_t ** jc);
  int _eXosip_dialog_set_200ok (eXosip_dialog_t * _jd, osip_message_t * _200Ok);

  int _eXosip_answer_invite_123456xx (struct eXosip_t *excontext, eXosip_call_t * jc, eXosip_dialog_t * jd, int code, osip_message_t ** answer, int send);

  int _eXosip_build_response_default (struct eXosip_t *excontext, osip_message_t ** dest, osip_dialog_t * dialog, int status, osip_message_t * request);
  int _eXosip_complete_answer_that_establish_a_dialog (struct eXosip_t *excontext, osip_message_t * response, osip_message_t * request);
  int _eXosip_build_request_within_dialog (struct eXosip_t *excontext, osip_message_t ** dest, const char *method, osip_dialog_t * dialog);
  int _eXosip_remove_transaction_from_call (osip_transaction_t * tr, eXosip_call_t * jc);

  osip_transaction_t *_eXosip_find_last_transaction (eXosip_call_t * jc, eXosip_dialog_t * jd, const char *method);
  osip_transaction_t *_eXosip_find_last_inc_transaction (eXosip_call_t * jc, eXosip_dialog_t * jd, const char *method);
  osip_transaction_t *_eXosip_find_last_out_transaction (eXosip_call_t * jc, eXosip_dialog_t * jd, const char *method);
  osip_transaction_t *_eXosip_find_last_invite (eXosip_call_t * jc, eXosip_dialog_t * jd);
  osip_transaction_t *_eXosip_find_last_inc_invite (eXosip_call_t * jc, eXosip_dialog_t * jd);
  osip_transaction_t *_eXosip_find_last_out_invite (eXosip_call_t * jc, eXosip_dialog_t * jd);
  osip_transaction_t *_eXosip_find_previous_invite (eXosip_call_t * jc, eXosip_dialog_t * jd, osip_transaction_t * last_invite);

  int _eXosip_call_init (struct eXosip_t *excontext, eXosip_call_t ** jc);
  void _eXosip_call_renew_expire_time (eXosip_call_t * jc);
  void _eXosip_call_free (struct eXosip_t *excontext, eXosip_call_t * jc);
  void _eXosip_call_remove_dialog_reference_in_call (eXosip_call_t * jc, eXosip_dialog_t * jd);
  int _eXosip_read_message (struct eXosip_t *excontext, int max_message_nb, int sec_max, int usec_max);
  void _eXosip_release_terminated_calls (struct eXosip_t *excontext);
  void _eXosip_release_terminated_registrations (struct eXosip_t *excontext);
  void _eXosip_release_terminated_publications (struct eXosip_t *excontext);

#ifndef MINISIZE
  int _eXosip_insubscription_transaction_find (struct eXosip_t *excontext, int tid, eXosip_notify_t ** jn, eXosip_dialog_t ** jd, osip_transaction_t ** tr);
  int _eXosip_notify_dialog_find (struct eXosip_t *excontext, int nid, eXosip_notify_t ** jn, eXosip_dialog_t ** jd);
  int _eXosip_subscription_transaction_find (struct eXosip_t *excontext, int tid, eXosip_subscribe_t ** js, eXosip_dialog_t ** jd, osip_transaction_t ** tr);
  int _eXosip_subscription_dialog_find (struct eXosip_t *excontext, int nid, eXosip_subscribe_t ** js, eXosip_dialog_t ** jd);
  int _eXosip_insubscription_answer_1xx (struct eXosip_t *excontext, eXosip_notify_t * jc, eXosip_dialog_t * jd, int code);
  int _eXosip_insubscription_answer_2xx (eXosip_notify_t * jn, eXosip_dialog_t * jd, int code);
  int _eXosip_insubscription_answer_3456xx (struct eXosip_t *excontext, eXosip_notify_t * jn, eXosip_dialog_t * jd, int code);
  osip_transaction_t *_eXosip_find_last_inc_notify (eXosip_subscribe_t * js, eXosip_dialog_t * jd);
  osip_transaction_t *_eXosip_find_last_out_notify (eXosip_notify_t * jn, eXosip_dialog_t * jd);
  osip_transaction_t *_eXosip_find_last_inc_subscribe (eXosip_notify_t * jn, eXosip_dialog_t * jd);
  osip_transaction_t *_eXosip_find_last_out_subscribe (eXosip_subscribe_t * js, eXosip_dialog_t * jd);
  void _eXosip_release_terminated_subscriptions (struct eXosip_t *excontext);
  void _eXosip_release_terminated_in_subscriptions (struct eXosip_t *excontext);
  int _eXosip_subscription_init (struct eXosip_t *excontext, eXosip_subscribe_t ** js);
  void _eXosip_subscription_free (struct eXosip_t *excontext, eXosip_subscribe_t * js);
  int _eXosip_subscription_set_refresh_interval (eXosip_subscribe_t * js, osip_message_t * inc_subscribe);
  int _eXosip_subscription_send_request_with_credential (struct eXosip_t *excontext, eXosip_subscribe_t * js, eXosip_dialog_t * jd, osip_transaction_t * out_tr);
  int _eXosip_subscription_automatic_refresh (struct eXosip_t *excontext, eXosip_subscribe_t * js, eXosip_dialog_t * jd, osip_transaction_t * out_tr);
  int _eXosip_notify_init (struct eXosip_t *excontext, eXosip_notify_t ** jn, osip_message_t * inc_subscribe);
  void _eXosip_notify_free (struct eXosip_t *excontext, eXosip_notify_t * jn);
  int _eXosip_notify_set_contact_info (eXosip_notify_t * jn, char *uri);
  int _eXosip_notify_set_refresh_interval (eXosip_notify_t * jn, osip_message_t * inc_subscribe);
  void _eXosip_notify_add_expires_in_2XX_for_subscribe (eXosip_notify_t * jn, osip_message_t * answer);
  int _eXosip_insubscription_send_request_with_credential (struct eXosip_t *excontext, eXosip_notify_t * jn, eXosip_dialog_t * jd, osip_transaction_t * out_tr);
#endif

  int _eXosip_is_public_address (const char *addr);

  void _eXosip_retransmit_lost200ok (struct eXosip_t *excontext);
  int _eXosip_dialog_add_contact (struct eXosip_t *excontext, osip_message_t * request);

  int _eXosip_transaction_init (struct eXosip_t *excontext, osip_transaction_t ** transaction, osip_fsm_type_t ctx_type, osip_t * osip, osip_message_t * message);
  void _eXosip_transaction_free (struct eXosip_t *excontext, osip_transaction_t *transaction);

  int _eXosip_srv_lookup (struct eXosip_t *excontext, osip_message_t * sip, osip_naptr_t ** naptr_record);

  void _eXosip_dnsutils_release (osip_naptr_t * naptr_record);

  int _eXosip_handle_incoming_message (struct eXosip_t *excontext, char *buf, size_t len, int socket, char *host, int port, char *received_host, int *rport_port);

  int _eXosip_transport_set_dscp (struct eXosip_t *excontext, int family, int sock);

 /**
  * sets the parameters for the TLS context, which is used for encrypted connections
  * @return  the eXosip_tls_ctx_error code
  */
  eXosip_tls_ctx_error eXosip_set_tls_ctx (struct eXosip_t *excontext, eXosip_tls_ctx_t * ctx);

/**
  * Select by CN name the server certificate from OS store.
  * 12/11/2009 -> implemented only for "Windows Certificate Store"
  */
  eXosip_tls_ctx_error eXosip_tls_use_server_certificate (struct eXosip_t *excontext, const char *local_certificate_cn);

/**
  * Select by CN name the client certificate from OS store.
  * 31/1/2011 -> implemented only for "Windows Certificate Store"
  */
  eXosip_tls_ctx_error eXosip_tls_use_client_certificate (struct eXosip_t *excontext, const char *local_certificate_cn);

/**
  * Configure to accept/reject self signed and expired certificates.
  */
  eXosip_tls_ctx_error eXosip_tls_verify_certificate (struct eXosip_t *excontext, int _tls_verify_client_certificate);

  
#ifndef EXOSIP_STATS_PERIOD
#define EXOSIP_STATS_PERIOD 3600  /* default period in seconds */
#endif
#ifndef EXOSIP_STATS_INTERVAL
#define EXOSIP_STATS_INTERVAL 60 /* default interval in seconds */
#endif

#ifndef MINISIZE
  void _eXosip_counters_init(struct eXosip_counters *bw_stats, int period, int interval);
  void _eXosip_counters_update(struct eXosip_counters *bw_stats, int nvalues, struct timeval *now);
  void _eXosip_counters_free(struct eXosip_counters *bw_stats);
#else
#define _eXosip_counters_init(A, B, C)
#define _eXosip_counters_update(A, B, C)
#define _eXosip_counters_free(A)
#endif

#ifdef __cplusplus
}
#endif
#endif
