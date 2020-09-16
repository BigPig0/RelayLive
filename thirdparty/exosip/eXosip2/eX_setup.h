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


#ifdef ENABLE_MPATROL
#include <mpatrol.h>
#endif

#ifndef __EX_SETUP_H__
#define __EX_SETUP_H__

#include <eXosip2/eXosip.h>
#include <osipparser2/osip_message.h>

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

  struct eXosip_t;
  struct osip_srv_record;
  struct osip_naptr;

/**
 * @file eX_setup.h
 * @brief eXosip setup API
 *
 * This file provide the API needed to setup and configure
 * the SIP endpoint.
 *
 */

/**
 * @defgroup eXosip2_conf eXosip2 configuration API
 * @ingroup eXosip2_setup
 * @{
 */

/**
 * Allocate an eXosip context.
 * 
 * @return  a new allocated eXosip_t instance.
 */
  struct eXosip_t *eXosip_malloc (void);

/**
 * Initiate the eXtented oSIP library.
 * 
 * @param excontext    eXosip_t instance.
 */
  int eXosip_init (struct eXosip_t *excontext);

/**
 * Release ressource used by the eXtented oSIP library.
 * 
 * @param excontext    eXosip_t instance.
 */
  void eXosip_quit (struct eXosip_t *excontext);

/**
 * Lock the eXtented oSIP library.
 * 
 * @param excontext    eXosip_t instance.
 */
  int eXosip_lock (struct eXosip_t *excontext);

/**
 * UnLock the eXtented oSIP library.
 * 
 * @param excontext    eXosip_t instance.
 */
  int eXosip_unlock (struct eXosip_t *excontext);

/**
 * Process (non-threaded mode ONLY) eXosip events.
 * 
 * @param excontext    eXosip_t instance.
 */
  int eXosip_execute (struct eXosip_t *excontext);

#define EXOSIP_OPT_BASE_OPTION 0
#define EXOSIP_OPT_UDP_KEEP_ALIVE (EXOSIP_OPT_BASE_OPTION+1) /**< int *: interval for keep alive packets (UDP, TCP, TLS, DTLS) */
#define EXOSIP_OPT_AUTO_MASQUERADE_CONTACT (EXOSIP_OPT_BASE_OPTION+2) /**< int *: specific re-usage of "rport" */
#define EXOSIP_OPT_UDP_LEARN_PORT EXOSIP_OPT_AUTO_MASQUERADE_CONTACT /** EXOSIP_OPT_UDP_LEARN_PORT is obsolete / replaced by EXOSIP_OPT_AUTO_MASQUERADE_CONTACT */
#define EXOSIP_OPT_USE_RPORT (EXOSIP_OPT_BASE_OPTION+7) /**< int *: enable or disable rport in via */
#define EXOSIP_OPT_SET_IPV4_FOR_GATEWAY (EXOSIP_OPT_BASE_OPTION+8) /**< char *: usually, this is the proxy address */
#define EXOSIP_OPT_ADD_DNS_CACHE (EXOSIP_OPT_BASE_OPTION+9) /**< struct eXosip_dns_cache *: force some cache entry to avoid DNS */
#define EXOSIP_OPT_DELETE_DNS_CACHE (EXOSIP_OPT_BASE_OPTION+10) /**< struct eXosip_dns_cache *: force removal of some cache entry to avoid DNS */
#define EXOSIP_OPT_SET_IPV6_FOR_GATEWAY (EXOSIP_OPT_BASE_OPTION+12) /**< char *: usually, this is the proxy address */
#define EXOSIP_OPT_ADD_ACCOUNT_INFO (EXOSIP_OPT_BASE_OPTION+13) /**< struct eXosip_account_info *: internal stuff */
#define EXOSIP_OPT_DNS_CAPABILITIES (EXOSIP_OPT_BASE_OPTION+14) /**< int *: 0 to disable, 2 (default) to use NAPTR/SRV record */
#define EXOSIP_OPT_SET_DSCP (EXOSIP_OPT_BASE_OPTION+15) /**< int *: set a dscp value for SIP socket */
#define EXOSIP_OPT_REGISTER_WITH_DATE (EXOSIP_OPT_BASE_OPTION+16) /**< int *: enable usage of Date header in REGISTER */
#define EXOSIP_OPT_SET_HEADER_USER_AGENT (EXOSIP_OPT_BASE_OPTION+17) /**< char *: set the User-Agent header */
#define EXOSIP_OPT_ENABLE_DNS_CACHE (EXOSIP_OPT_BASE_OPTION+18) /**< int *: 0 to disable use of cache*/
#define EXOSIP_OPT_ENABLE_AUTOANSWERBYE (EXOSIP_OPT_BASE_OPTION+19) /**< int *: 0 to disable automatic answer of BYE */
#define EXOSIP_OPT_ENABLE_IPV6 (EXOSIP_OPT_BASE_OPTION+20) /**< int *: 0 to disable -this is a per-eXosip_t parameter for using IPv6 DNS request */
#define EXOSIP_OPT_ENABLE_REUSE_TCP_PORT (EXOSIP_OPT_BASE_OPTION+21) /**< int *: 0 to disable, 1 to enable reusing local tcp port for outgoing tcp connection */
#define EXOSIP_OPT_ENABLE_USE_EPHEMERAL_PORT (EXOSIP_OPT_BASE_OPTION+22) /**< int *: 0 to disable, 1 to enable usage of emphemeral tcp port in Contact headers instead of local listening port for TCP/TLS */
#define EXOSIP_OPT_SET_CALLBACK_WAKELOCK (EXOSIP_OPT_BASE_OPTION+23) /**< CbSipWakeLock *: set a callback to be called upon start/end of transaction */
#define EXOSIP_OPT_ENABLE_OUTBOUND (EXOSIP_OPT_BASE_OPTION+24) /**< int *: 0 to disable, 1 to enable usage of ob parmeter (rfc 5626) in dialog's contact headers */
#define EXOSIP_OPT_SET_OC_LOCAL_ADDRESS (EXOSIP_OPT_BASE_OPTION+25) /**< char *: set the ip address to bind for outbound connection */
#define EXOSIP_OPT_SET_OC_PORT_RANGE (EXOSIP_OPT_BASE_OPTION+26) /**< int[2] *: set the port range (min, max) to bind for outbound connection (if EXOSIP_OPT_SET_OC_LOCAL_ADDRESS is not used, this options apply to TCP/TLS only) */
#define EXOSIP_OPT_REMOVE_PREROUTESET (EXOSIP_OPT_BASE_OPTION+27) /**< int *: 0: keep pre-route set in initial INVITE/SUBSCRIBE/REFER, 1 (default): remove pre-route set*/
#define EXOSIP_OPT_SET_SIP_INSTANCE (EXOSIP_OPT_BASE_OPTION+28) /**< char *: define +sip.instance parameter in Contact headers  (example: f81d4fae-7dec-11d0-a765-00a0c91e6bf6) */
#define EXOSIP_OPT_SET_MAX_MESSAGE_TO_READ (EXOSIP_OPT_BASE_OPTION+29) /**< int: set the number of message to read at once for each network processing (high load traffic use-case: DO NOT USE FOR COMMON USAGE) */
#define EXOSIP_OPT_SET_MAX_READ_TIMEOUT (EXOSIP_OPT_BASE_OPTION+30) /**< long int: set the period in nano seconds during we read for sip message. (high load traffic use-case: DO NOT USE FOR COMMON USAGE)*/
#define EXOSIP_OPT_SET_DEFAULT_CONTACT_DISPLAYNAME (EXOSIP_OPT_BASE_OPTION+31) /**< char *: define a display name to be added in Contact headers  (example: "john Doe") */

#define EXOSIP_OPT_SET_TLS_VERIFY_CERTIFICATE (EXOSIP_OPT_BASE_OPTION+500) /**< int *: enable verification of certificate for TLS connection */
#define EXOSIP_OPT_SET_TLS_CERTIFICATES_INFO (EXOSIP_OPT_BASE_OPTION+501) /**< eXosip_tls_ctx_t *: client and/or server certificate/ca-root/key info */
#define EXOSIP_OPT_SET_TLS_CLIENT_CERTIFICATE_NAME (EXOSIP_OPT_BASE_OPTION+502) /**< char*: user can choose a specific certifcate present in Windows Certificate Store */
#define EXOSIP_OPT_SET_TLS_SERVER_CERTIFICATE_NAME (EXOSIP_OPT_BASE_OPTION+503) /**< char*: user can choose a specific certifcate present in Windows Certificate Store */

  /* non standard option: need a compilation flag to activate */
#define EXOSIP_OPT_KEEP_ALIVE_OPTIONS_METHOD (EXOSIP_OPT_BASE_OPTION+1000)
#define EXOSIP_OPT_SET_TSC_SERVER (EXOSIP_OPT_BASE_OPTION+1001) /**< void*: set the tsc tunnel handle */

#define EXOSIP_OPT_GET_STATISTICS (EXOSIP_OPT_BASE_OPTION+2000) /**< struct eXosip_stats*: retreive numerous statistics about transactions, registrations, calls, publications and subscriptions... */

 /**
  * structure used to for inserting a DNS cache entry and avoid DNS resolution.
  * @struct eXosip_dns_cache
  */
  struct eXosip_dns_cache {
    char host[1024];
    char ip[256];
  };

  struct eXosip_account_info {
    char proxy[1024];
    char nat_ip[256];
    int nat_port;
  };

  struct eXosip_http_auth {
    char pszCallId[64];
    osip_proxy_authenticate_t *wa;
    char pszCNonce[64];
    int iNonceCount;
    int answer_code;
  };

#ifndef MINISIZE
  /**
   * Structure used to retrieve eXosip internal statistics.
   * Total numbers are provided since last start or restart of eXosip.
   * Average values are calculated over the last EXOSIP_STATS_PERIOD
   * which default to 3600 seconds.
   *
   * @struct eXosip_stats
   */
  struct eXosip_stats
  {
    int allocated_transactions;        /**< current number of allocated transactions. */
    float average_transactions;     /**< average number of new transactions/hour.  (default period: 1 hour) */
    int allocated_registrations;       /**< current number of allocated registrations. (should remains 1 in standard usage) */
    float average_registrations;    /**< average number of new registrations/hour. (default period: 1 hour) */
    int allocated_calls;               /**< current number of allocated calls. */
    float average_calls;            /**< average number of new calls/hour. (default period: 1 hour) */
    int allocated_publications;        /**< current number of allocated publications. */
    float average_publications;     /**< average number of new publication/hour. (default period: 1 hour) */
    int allocated_subscriptions;       /**< current number of allocated outgoing subscriptions. */
    float average_subscriptions;    /**< average number of new outgoing subscriptions/hour. (default period: 1 hour) */
    int allocated_insubscriptions;     /**< current number of allocated incoming subscriptions. */
    float average_insubscriptions;  /**< average number of new incoming subscriptions/hour. (default period: 1 hour) */

    int reserved1[20];               /**< reserved for future usage without breaking ABI */
  };
#endif
  
/**
 * Set eXosip options.
 * See eXosip_option for available options.
 *
 * @param excontext    eXosip_t instance.
 * @param opt     option to configure.
 * @param value   value for options.
 * 
 */
  int eXosip_set_option (struct eXosip_t *excontext, int opt, const void *value);


 /**
  * structure used to describe credentials for a client or server
  * consists of a certificate, a corresponding private key and its password
  * @struct eXosip_tls_credentials_s
  */
  typedef struct eXosip_tls_credentials_s {
    char priv_key[1024];
    char priv_key_pw[1024];
    char cert[1024];
  } eXosip_tls_credentials_t;

 /**
  * structure to describe the whole TLS-context for eXosip
  * consists of a certificate, a corresponding private key and its password
  * @struct eXosip_tls_ctx_s
  */
  typedef struct eXosip_tls_ctx_s {
    char random_file[1024];                                             /**< absolute path to a file with random(!) data */
    char dh_param[1024];                                                /**< absolute path to a file necessary for diffie hellman key exchange */
    char root_ca_cert[1024];                                            /**< absolute path to the file with known rootCAs */
    eXosip_tls_credentials_t client;            /**< credential of the client */
    eXosip_tls_credentials_t server;            /**< credential of the server */
  } eXosip_tls_ctx_t;

 /**
  * An enumeration which describes the error which can occur while setting the eXosip_tls_ctx
  */
  typedef enum {
    TLS_OK = 0,                                                         /**< yippieh, everything is fine :) */
    TLS_ERR_NO_RAND = -1,                               /**< no absolute path to the random file was specified */
    TLS_ERR_NO_DH_PARAM = -2,                           /**< no absolute path to the diifie hellman file was specified */
    TLS_ERR_NO_PW = -3,                                         /**< no password was specified */
    TLS_ERR_NO_ROOT_CA = -4,                            /**< no absolute path to the rootCA file was specified */
    TLS_ERR_MISSING_AUTH_PART = -5              /**< something is missing: the private key or the certificate */
  } eXosip_tls_ctx_error;

/**
 * Start and return osip_naptr context.
 * Note that DNS results might not yet be available.
 * 
 * @param excontext    eXosip_t instance.
 * @param domain         domain name for NAPTR record
 * @param protocol       protocol to use ("SIP")
 * @param transport      transport to use ("UDP")
 * @param keep_in_cache  keep result in cache if >0
 */
  struct osip_naptr *eXosip_dnsutils_naptr (struct eXosip_t *excontext, const char *domain, const char *protocol, const char *transport, int keep_in_cache);

/**
 * Continue to process asynchronous DNS request (if implemented).
 * 
 * @param output_record  result structure.
 * @param force          force waiting for final answer if >0
 */
  int eXosip_dnsutils_dns_process (struct osip_naptr *output_record, int force);

/**
 * Rotate first SRV entry to last SRV entry.
 * 
 * @param output_record  result structure.
 */
  int eXosip_dnsutils_rotate_srv (struct osip_srv_record *output_record);

/**
 * Listen on a specified socket.
 * 
 * @param excontext    eXosip_t instance.
 * @param transport IPPROTO_UDP for udp. (soon to come: TCP/TLS?)
 * @param addr      the address to bind (NULL for all interface)
 * @param port      the listening port. (0 for random port)
 * @param family    the IP family (AF_INET or AF_INET6).
 * @param secure    0 for UDP or TCP, 1 for TLS (with TCP).
 */
  int eXosip_listen_addr (struct eXosip_t *excontext, int transport, const char *addr, int port, int family, int secure);

/**
 * Reset transport sockets.
 * 
 * @param excontext    eXosip_t instance.
 */
  int eXosip_reset_transports (struct eXosip_t *excontext);


/**
 * Listen on a specified socket.
 * 
 * @param excontext    eXosip_t instance.
 * @param transport IPPROTO_UDP for udp. (soon to come: TCP/TLS?)
 * @param socket socket to use for listening to UDP sip messages.
 * @param port the listening port for masquerading.
 */
  int eXosip_set_socket (struct eXosip_t *excontext, int transport, int socket, int port);

/**
 * Set the SIP User-Agent: header string.
 *
 * @param excontext    eXosip_t instance.
 * @param user_agent the User-Agent header to insert in messages.
 */
  void eXosip_set_user_agent (struct eXosip_t *excontext, const char *user_agent);

 /**
  * Get the eXosip version as a sring
  *
  */
  const char *eXosip_get_version (void);

#ifdef WIN32
  typedef void (__stdcall *CbSipCallback) (osip_message_t * msg, int received);
  typedef void (__stdcall *CbSipWakeLock) (int state);
#else
  typedef void (*CbSipCallback) (osip_message_t * msg, int received);
  typedef void (*CbSipWakeLock) (int state);
#endif

/**
 * Set a callback to get sent and received SIP messages.
 *
 * @param excontext    eXosip_t instance.
 * @param cbsipCallback the callback to retreive messages.
 */
  int eXosip_set_cbsip_message (struct eXosip_t *excontext, CbSipCallback cbsipCallback);

/**
 * This method is used to replace contact address with
 * the public address of your NAT. The ip address should
 * be retreived manually (fixed IP address) or with STUN.
 * This address will only be used when the remote
 * correspondant appears to be on an DIFFERENT LAN.
 *
 * @param excontext    eXosip_t instance.
 * @param public_address 	the ip address.
 * @param port          	the port for masquerading.
 * 
 * If set to NULL, then the local ip address will be guessed 
 * automatically (returns to default mode).
 */
  void eXosip_masquerade_contact (struct eXosip_t *excontext, const char *public_address, int port);

/**
 * This method is used to find out an free IPPROTO_UDP or IPPROTO_TCP port.
 *
 * @param excontext    eXosip_t instance.
 * @param free_port          	initial port for search.
 * @param transport          	IPPROTO_UDP or IPPROTO_TCP protocol.
 * 
 */
  int eXosip_find_free_port (struct eXosip_t *excontext, int free_port, int transport);

#ifndef DOXYGEN

/**
 * Wake Up the eXosip_event_wait method.
 * 
 * @param excontext    eXosip_t instance.
 */
  void eXosip_wakeup_event (struct eXosip_t *excontext);

#endif

/** @} */


/**
 * @defgroup eXosip2_network eXosip2 network API
 * @ingroup eXosip2_setup
 * @{
 */

/**
 * Modify the transport protocol used to send SIP message.
 * 
 * @param msg         The SIP message to modify
 * @param transport   transport protocol to use ("UDP", "TCP" or "TLS")
 */
  int eXosip_transport_set (osip_message_t * msg, const char *transport);

/**
 * Find the current localip (interface with default route).
 * 
 * @param excontext    eXosip_t instance.
 * @param family    AF_INET or AF_INET6
 * @param address   a string containing the local IP address.
 * @param size      The size of the string
 */
  int eXosip_guess_localip (struct eXosip_t *excontext, int family, char *address, int size);

/** @} */


#ifdef __cplusplus
}
#endif
#endif
