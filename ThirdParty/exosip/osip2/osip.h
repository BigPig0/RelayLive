/*
  The oSIP library implements the Session Initiation Protocol (SIP -rfc3261-)
  Copyright (C) 2001-2012 Aymeric MOIZARD amoizard@antisip.com
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef _OSIP_H_
#define _OSIP_H_

#include <osipparser2/osip_const.h>

/* Time-related functions and data types */
#include <osip2/osip_time.h>

#ifdef __sun
#include <sys/types.h>
#endif

#include <osipparser2/osip_parser.h>
#include <osip2/osip_fifo.h>

/**
 * @file osip.h
 * @brief oSIP fsm Routines
 *
 */

/**
 * @defgroup oSIP_FSM oSIP fsm Handling
 * @ingroup osip2_fsm
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Enumeration for transaction state.
 * Those states are extracted from the diagram found in rfc3261.txt
 *
 */
  typedef enum _state_t {
    /* STATES for invite client transaction */
    ICT_PRE_CALLING,
    ICT_CALLING,
    ICT_PROCEEDING,
    ICT_COMPLETED,
    ICT_TERMINATED,

    /* STATES for invite server transaction */
    IST_PRE_PROCEEDING,
    IST_PROCEEDING,
    IST_COMPLETED,
    IST_CONFIRMED,
    IST_TERMINATED,

    /* STATES for NON-invite client transaction */
    NICT_PRE_TRYING,
    NICT_TRYING,
    NICT_PROCEEDING,
    NICT_COMPLETED,
    NICT_TERMINATED,

    /* STATES for NON-invite server transaction */
    NIST_PRE_TRYING,
    NIST_TRYING,
    NIST_PROCEEDING,
    NIST_COMPLETED,
    NIST_TERMINATED,

#ifndef DOXYGEN
    DIALOG_EARLY,
    DIALOG_CONFIRMED,
    DIALOG_CLOSE                /* ?? */
#endif
  } state_t;

/**
 * Enumeration for event type.
 * <BR>The list of values that you need to know is reduced to this:
 * <BR> RCV_REQINVITE,
 * <BR> RCV_REQACK,
 * <BR> RCV_REQUEST,
 * <BR> RCV_STATUS_1XX,
 * <BR> RCV_STATUS_2XX,
 * <BR> RCV_STATUS_3456XX,
 *<BR>
 * <BR> SND_REQINVITE,
 * <BR> SND_REQACK,
 * <BR> SND_REQUEST,
 * <BR> SND_STATUS_1XX,
 * <BR> SND_STATUS_2XX,
 * <BR> SND_STATUS_3456XX,
 */
  typedef enum type_t {
    /* TIMEOUT EVENTS for ICT */
    TIMEOUT_A,                   /**< Timer A */
    TIMEOUT_B,                   /**< Timer B */
    TIMEOUT_D,                   /**< Timer D */

    /* TIMEOUT EVENTS for NICT */
    TIMEOUT_E,                   /**< Timer E */
    TIMEOUT_F,                   /**< Timer F */
    TIMEOUT_K,                   /**< Timer K */

    /* TIMEOUT EVENTS for IST */
    TIMEOUT_G,                   /**< Timer G */
    TIMEOUT_H,                   /**< Timer H */
    TIMEOUT_I,                   /**< Timer I */

    /* TIMEOUT EVENTS for NIST */
    TIMEOUT_J,                   /**< Timer J */

    /* FOR INCOMING MESSAGE */
    RCV_REQINVITE,               /**< Event is an incoming INVITE request */
    RCV_REQACK,                  /**< Event is an incoming ACK request */
    RCV_REQUEST,                 /**< Event is an incoming NON-INVITE and NON-ACK request */
    RCV_STATUS_1XX,              /**< Event is an incoming informational response */
    RCV_STATUS_2XX,              /**< Event is an incoming 2XX response */
    RCV_STATUS_3456XX,           /**< Event is an incoming final response (not 2XX) */

    /* FOR OUTGOING MESSAGE */
    SND_REQINVITE,               /**< Event is an outgoing INVITE request */
    SND_REQACK,                  /**< Event is an outgoing ACK request */
    SND_REQUEST,                 /**< Event is an outgoing NON-INVITE and NON-ACK request */
    SND_STATUS_1XX,              /**< Event is an outgoing informational response */
    SND_STATUS_2XX,              /**< Event is an outgoing 2XX response */
    SND_STATUS_3456XX,           /**< Event is an outgoing final response (not 2XX) */

    KILL_TRANSACTION,            /**< Event to 'kill' the transaction before termination */
    UNKNOWN_EVT                  /**< Max event */
  } type_t;

/**
 * Enumeration for transaction type.
 * A transaction can be either of:
 *  ICT,
 *  IST,
 *  NICT,
 *  NIST,
 */
  typedef enum osip_fsm_type_t {
    ICT,         /**< Invite Client (outgoing) Transaction */
    IST,         /**< Invite Server (incoming) Transaction */
    NICT,        /**< Non-Invite Client (outgoing) Transaction */
    NIST         /**< Non-Invite Server (incoming) Transaction */
  } osip_fsm_type_t;

#ifndef DEFAULT_T1
/**
 * You can re-define the default value for T1. (T1 is defined in rfcxxxx)
 * The default value is 500ms.
 */
#define DEFAULT_T1 500          /* 500 ms */
#endif
#ifndef DEFAULT_T1_TCP_PROGRESS
  /**
   * You can re-define the default value for T1_TCP_PROGRESS.
   * This is a trick to use non blocking socke for reliable protocol
   * On first attempt, the connection is not ready and the next
   * osip retransmission are used to check the progress of the connection
   * in order to send the message.
   * The default value is 50ms.
   */
#define DEFAULT_T1_TCP_PROGRESS 50         /* 50ms */
#endif
#ifndef DEFAULT_T2
  /**
   * You can re-define the default value for T2. (T2 is defined in rfcxxxx)
   * The default value is 4000ms.
   */
#define DEFAULT_T2 4000         /* 4s */
#endif
#ifndef DEFAULT_T4
/**
 * You can re-define the default value for T4. (T1 is defined in rfcxxxx)
 * The default value is 5000ms.
 */
#define DEFAULT_T4 5000         /* 5s */
#endif


/**
 * Structure for INVITE CLIENT TRANSACTION (outgoing INVITE transaction).
 * @var osip_ict_t
 */
  typedef struct osip_ict osip_ict_t;

/**
 * Structure for INVITE CLIENT TRANSACTION.
 * @struct osip_ict
 */
  struct osip_ict {
    int timer_a_length;                           /**< Timer A A=T1, A=2xT1... (unreliable only) */
    struct timeval timer_a_start;                 /**< Timer A (retransmission) */
    int timer_b_length;                           /**< Timer B B = 64* T1 */
    struct timeval timer_b_start;                 /**< Timer B (fire when transaction timeout) */
    int timer_d_length;                           /**< Timer D D >= 32s for unreliable tr (or 0) */
    struct timeval timer_d_start;                 /**< Timer D */
    char *destination;                            /**< IP used to send requests */
    int port;                                     /**< port of next hop */
  };

/**
 * Structure for NON-INVITE CLIENT TRANSACTION (outgoing NON-INVITE transaction).
 * @var osip_nict_t
 */
  typedef struct osip_nict osip_nict_t;

/**
 * Structure for NON-INVITE CLIENT TRANSACTION.
 * @struct osip_nict
 */
  struct osip_nict {
    int timer_e_length;                           /**< Timer E A=T1, A=2xT1... (unreliable only) */
    struct timeval timer_e_start;                 /**< Timer E (retransmission) */
    int timer_f_length;                           /**< Timer F B = 64* T1 */
    struct timeval timer_f_start;                 /**< Timer F (fire when transaction timeout) */
    int timer_k_length;                           /**< Timer K K = T4 (else = 0) */
    struct timeval timer_k_start;                 /**< Timer K */
    char *destination;                            /**< IP used to send requests */
    int port;                                     /**< port of next hop */

  };

/**
 * Structure for INVITE SERVER TRANSACTION (incoming INVITE transaction).
 * @var osip_ist_t
 */
  typedef struct osip_ist osip_ist_t;

/**
 * Structure for INVITE SERVER TRANSACTION.
 * @struct osip_ist
 */
  struct osip_ist {
    int timer_g_length;                           /**< Timer G G=MIN(T1*2,T2) for unreliable trans. */
    struct timeval timer_g_start;                 /**< Timer G (0 when reliable transport is used) */
    int timer_h_length;                           /**< Timer H H = 64* T1 */
    struct timeval timer_h_start;                 /**< Timer H (fire if no ACK is received) */
    int timer_i_length;                           /**< Timer I I = T4 for unreliable (or 0) */
    struct timeval timer_i_start;                 /**< Timer I (absorb all ACK) */
  };

/**
 * Structure for NON-INVITE SERVER TRANSACTION (incoming SERVER transaction).
 * @var osip_nist_t
 */
  typedef struct osip_nist osip_nist_t;

/**
 * Structure for NON-INVITE SERVER TRANSACTION.
 * @struct osip_nist
 */
  struct osip_nist {
    int timer_j_length;                            /**< Timer J = 64*T1 (else 0) */
    struct timeval timer_j_start;                  /**< Timer J */
  };

/**
 * Structure for SRV record entry.
 * @var osip_srv_entry_t
 */
  typedef struct osip_srv_entry osip_srv_entry_t;

/**
 * Structure for SRV record entry.
 * @struct osip_srv_entry
 */

  struct osip_srv_entry {
    char srv[512];                   /**< srv */
    int priority;                    /**< priority */
    int weight;                      /**< weight  */
    int rweight;                     /**< rweight  */
    int port;                        /**< port  */
    char ipaddress[512];             /**< ipaddress result  */
    struct timeval srv_is_broken;    /**< time when we considered SRV entry broken */
  };

#define OSIP_SRV_STATE_UNKNOWN 0         /**< unknown */
#define OSIP_SRV_STATE_RETRYLATER 2      /**< retry later */
#define OSIP_SRV_STATE_COMPLETED 3       /**< completed */
#define OSIP_SRV_STATE_NOTSUPPORTED 4    /**< not supported */

/**
 * Structure for SRV record.
 * @var osip_srv_record_t
 */
  typedef struct osip_srv_record osip_srv_record_t;

/**
 * Structure for SRV record entry.
 * @struct osip_srv_record
 */
  struct osip_srv_record {
    char name[512];                  /**< name */
    int srv_state;                   /**< srv state */
    char protocol[64];               /**< transport protocol*/
    int order;                       /**< order */
    int preference;                  /**< preference */
    int index;                       /**< index */
    osip_srv_entry_t srventry[10];   /**< result table */
  };

#define OSIP_NAPTR_STATE_UNKNOWN 0         /**< unknown */
#define OSIP_NAPTR_STATE_INPROGRESS 1      /**< in progress */
#define OSIP_NAPTR_STATE_NAPTRDONE 2       /**< naptr done */
#define OSIP_NAPTR_STATE_SRVINPROGRESS 3   /**< srv in progress */
#define OSIP_NAPTR_STATE_SRVDONE 4         /**< srv done */
#define OSIP_NAPTR_STATE_RETRYLATER 5      /**< retry later */
#define OSIP_NAPTR_STATE_NOTSUPPORTED 6    /**< not supported */

/**
 * Structure for NAPTR record.
 * @var osip_naptr_t
 */
  typedef struct osip_naptr osip_naptr_t;

/**
 * Structure for NAPTR record entry.
 * @struct osip_naptr
 */
  struct osip_naptr {
    char domain[512];                       /**< domain */
    int naptr_state;                        /**< naptr state */
    void *arg;                              /**< arg */
    int keep_in_cache;                      /**< keep in cache value */
    struct osip_srv_record sipudp_record;   /**< udp SRV result */
    struct osip_srv_record siptcp_record;   /**< tcp SRV result */
    struct osip_srv_record siptls_record;   /**< tls SRV result */
    struct osip_srv_record sipdtls_record;  /**< dtls SRV result */
    struct osip_srv_record sipsctp_record;  /**< sctp SRV result */
  };

/**
 * Structure for transaction handling.
 * @var osip_transaction_t
 */
  typedef struct osip_transaction osip_transaction_t;

/**
 * Structure for transaction handling
 * @struct osip_transaction
 */
  struct osip_transaction {

    void *your_instance;                /**< User Defined Pointer. */
    int transactionid;                  /**< Internal Transaction Identifier. */
    osip_fifo_t *transactionff;         /**< events must be added in this fifo */

    osip_via_t *topvia;                 /**< CALL-LEG definition (Top Via) */
    osip_from_t *from;                  /**< CALL-LEG definition (From)    */
    osip_to_t *to;                      /**< CALL-LEG definition (To)      */
    osip_call_id_t *callid;             /**< CALL-LEG definition (Call-ID) */
    osip_cseq_t *cseq;                  /**< CALL-LEG definition (CSeq)    */

    osip_message_t *orig_request;       /**< Initial request            */
    osip_message_t *last_response;      /**< Last response              */
    osip_message_t *ack;                /**< ack request sent           */

    state_t state;                      /**< Current state of the transaction */

    time_t birth_time;                  /**< birth date of transaction        */
    time_t completed_time;              /**< end   date of transaction        */

    int in_socket;                      /**< Optional socket for incoming message */
    int out_socket;                     /**< Optional place for outgoing message */

    void *config;                       /**< (internal) transaction is managed by osip_t  */

    osip_fsm_type_t ctx_type;           /**< Type of the transaction */
    osip_ict_t *ict_context;            /**< internal ict context */
    osip_ist_t *ist_context;            /**< internal ist context */
    osip_nict_t *nict_context;          /**< internal nict context */
    osip_nist_t *nist_context;          /**< internal nist context */

    osip_srv_record_t record;           /**< memory space for SRV record */
    osip_naptr_t *naptr_record;         /**< memory space for NAPTR record */
    void *reserved1;                    /**< User Defined Pointer. */
    void *reserved2;                    /**< User Defined Pointer. */
    void *reserved3;                    /**< User Defined Pointer. */
    void *reserved4;                    /**< User Defined Pointer. */
    void *reserved5;                    /**< User Defined Pointer. */
    void *reserved6;                    /**< User Defined Pointer. */
  };


/**
 * Enumeration for callback type.
 */
  typedef enum osip_message_callback_type {
    OSIP_ICT_INVITE_SENT = 0,                           /**< INVITE MESSAGE SENT */
    OSIP_ICT_INVITE_SENT_AGAIN,                         /**< INVITE MESSAGE RETRANSMITTED */
    OSIP_ICT_ACK_SENT,                                  /**< ACK MESSAGE SENT */
    OSIP_ICT_ACK_SENT_AGAIN,                            /**< ACK MESSAGE RETRANSMITTED */
    OSIP_ICT_STATUS_1XX_RECEIVED,                       /**< 1XX FOR INVITE RECEIVED */
    OSIP_ICT_STATUS_2XX_RECEIVED,                       /**< 2XX FOR INVITE RECEIVED */
    OSIP_ICT_STATUS_2XX_RECEIVED_AGAIN,                 /**< 2XX FOR INVITE RECEIVED AGAIN */
    OSIP_ICT_STATUS_3XX_RECEIVED,                       /**< 3XX FOR INVITE RECEIVED */
    OSIP_ICT_STATUS_4XX_RECEIVED,                       /**< 4XX FOR INVITE RECEIVED */
    OSIP_ICT_STATUS_5XX_RECEIVED,                       /**< 5XX FOR INVITE RECEIVED */
    OSIP_ICT_STATUS_6XX_RECEIVED,                       /**< 6XX FOR INVITE RECEIVED */
    OSIP_ICT_STATUS_3456XX_RECEIVED_AGAIN,              /**< RESPONSE RECEIVED AGAIN */

    OSIP_IST_INVITE_RECEIVED,                           /**< INVITE MESSAGE RECEIVED */
    OSIP_IST_INVITE_RECEIVED_AGAIN,                     /**< INVITE MESSAGE RECEIVED AGAN */
    OSIP_IST_ACK_RECEIVED,                              /**< ACK MESSAGE RECEIVED */
    OSIP_IST_ACK_RECEIVED_AGAIN,                        /**< ACK MESSAGE RECEIVED AGAIN */
    OSIP_IST_STATUS_1XX_SENT,                           /**< 1XX FOR INVITE SENT */
    OSIP_IST_STATUS_2XX_SENT,                           /**< 2XX FOR INVITE SENT */
    OSIP_IST_STATUS_2XX_SENT_AGAIN,                     /**< 2XX FOR INVITE RETRANSMITTED */
    OSIP_IST_STATUS_3XX_SENT,                           /**< 3XX FOR INVITE SENT */
    OSIP_IST_STATUS_4XX_SENT,                           /**< 4XX FOR INVITE SENT */
    OSIP_IST_STATUS_5XX_SENT,                           /**< 5XX FOR INVITE SENT */
    OSIP_IST_STATUS_6XX_SENT,                           /**< 6XX FOR INVITE SENT */
    OSIP_IST_STATUS_3456XX_SENT_AGAIN,                  /**< RESPONSE RETRANSMITTED */

    OSIP_NICT_REGISTER_SENT,                            /**< REGISTER MESSAGE SENT */
    OSIP_NICT_BYE_SENT,                                 /**< BYE MESSAGE SENT */
    OSIP_NICT_OPTIONS_SENT,                             /**< OPTIONS MESSAGE SENT */
    OSIP_NICT_INFO_SENT,                                /**< INFO MESSAGE SENT */
    OSIP_NICT_CANCEL_SENT,                              /**< CANCEL MESSAGE SENT */
    OSIP_NICT_NOTIFY_SENT,                              /**< NOTIFY MESSAGE SENT */
    OSIP_NICT_SUBSCRIBE_SENT,                           /**< SUBSCRIBE MESSAGE SENT */
    OSIP_NICT_UNKNOWN_REQUEST_SENT,                     /**< UNKNOWN REQUEST MESSAGE SENT */
    OSIP_NICT_REQUEST_SENT_AGAIN,                       /**< REQUEST MESSAGE RETRANMITTED */
    OSIP_NICT_STATUS_1XX_RECEIVED,                      /**< 1XX FOR MESSAGE RECEIVED */
    OSIP_NICT_STATUS_2XX_RECEIVED,                      /**< 2XX FOR MESSAGE RECEIVED */
    OSIP_NICT_STATUS_2XX_RECEIVED_AGAIN,                /**< 2XX FOR MESSAGE RECEIVED AGAIN */
    OSIP_NICT_STATUS_3XX_RECEIVED,                      /**< 3XX FOR MESSAGE RECEIVED */
    OSIP_NICT_STATUS_4XX_RECEIVED,                      /**< 4XX FOR MESSAGE RECEIVED */
    OSIP_NICT_STATUS_5XX_RECEIVED,                      /**< 5XX FOR MESSAGE RECEIVED */
    OSIP_NICT_STATUS_6XX_RECEIVED,                      /**< 6XX FOR MESSAGE RECEIVED */
    OSIP_NICT_STATUS_3456XX_RECEIVED_AGAIN,             /**< RESPONSE RECEIVED AGAIN */

    OSIP_NIST_REGISTER_RECEIVED,                        /**< REGISTER RECEIVED */
    OSIP_NIST_BYE_RECEIVED,                             /**< BYE RECEIVED */
    OSIP_NIST_OPTIONS_RECEIVED,                         /**< OPTIONS RECEIVED */
    OSIP_NIST_INFO_RECEIVED,                            /**< INFO RECEIVED */
    OSIP_NIST_CANCEL_RECEIVED,                          /**< CANCEL RECEIVED */
    OSIP_NIST_NOTIFY_RECEIVED,                          /**< NOTIFY RECEIVED */
    OSIP_NIST_SUBSCRIBE_RECEIVED,                       /**< SUBSCRIBE RECEIVED */

    OSIP_NIST_UNKNOWN_REQUEST_RECEIVED,                 /**< UNKNWON REQUEST RECEIVED */
    OSIP_NIST_REQUEST_RECEIVED_AGAIN,                   /**< UNKNWON REQUEST RECEIVED AGAIN */
    OSIP_NIST_STATUS_1XX_SENT,                          /**< 1XX FOR MESSAGE SENT */
    OSIP_NIST_STATUS_2XX_SENT,                          /**< 2XX FOR MESSAGE SENT */
    OSIP_NIST_STATUS_2XX_SENT_AGAIN,                    /**< 2XX FOR MESSAGE RETRANSMITTED */
    OSIP_NIST_STATUS_3XX_SENT,                          /**< 3XX FOR MESSAGE SENT */
    OSIP_NIST_STATUS_4XX_SENT,                          /**< 4XX FOR MESSAGE SENT */
    OSIP_NIST_STATUS_5XX_SENT,                          /**< 5XX FOR MESSAGE SENT */
    OSIP_NIST_STATUS_6XX_SENT,                          /**< 6XX FOR MESSAGE SENT */
    OSIP_NIST_STATUS_3456XX_SENT_AGAIN,                 /**< RESPONSE RETRANSMITTED */

    OSIP_ICT_STATUS_TIMEOUT,                            /**< TIMER B EXPIRATION: NO REMOTE ANSWER  */
    OSIP_NICT_STATUS_TIMEOUT,                           /**< TIMER F EXPIRATION: NO REMOTE ANSWER  */

    OSIP_MESSAGE_CALLBACK_COUNT                         /**< END OF ENUM */
  } osip_message_callback_type_t;

/**
 * Enumeration for callback type used when transaction is over.
 */
  typedef enum osip_kill_callback_type {
    OSIP_ICT_KILL_TRANSACTION,                  /**< end of Client INVITE transaction */
    OSIP_IST_KILL_TRANSACTION,                  /**< end of Server INVITE transaction */
    OSIP_NICT_KILL_TRANSACTION,                 /**< end of Client Non-INVITE transaction */
    OSIP_NIST_KILL_TRANSACTION,                 /**< end of Server Non-INVITE transaction */

    OSIP_KILL_CALLBACK_COUNT                    /**< END OF ENUM */
  } osip_kill_callback_type_t;

/**
 * Enumeration for callback type used when a transport error is detected.
 */
  typedef enum osip_transport_error_callback_type {
    OSIP_ICT_TRANSPORT_ERROR,                             /**< transport error for ICT */
    OSIP_IST_TRANSPORT_ERROR,                             /**< transport error for IST */
    OSIP_NICT_TRANSPORT_ERROR,                            /**< transport error for NICT */
    OSIP_NIST_TRANSPORT_ERROR,                            /**< transport error for NIST */

    OSIP_TRANSPORT_ERROR_CALLBACK_COUNT                   /**< END OF ENUM */
  } osip_transport_error_callback_type_t;

/**
 * Callback definition for message announcements.
 * @var osip_message_cb_t
 */
  typedef void (*osip_message_cb_t) (int type, osip_transaction_t *, osip_message_t *);
/**
 * Callback definition for end of transaction announcements.
 * @var osip_kill_transaction_cb_t
 */
  typedef void (*osip_kill_transaction_cb_t) (int type, osip_transaction_t *);
/**
 * Callback definition for transport error announcements.
 * @var osip_transport_error_cb_t
 */
  typedef void (*osip_transport_error_cb_t) (int type, osip_transaction_t *, int error);


  struct osip_dialog;

/**
 * Structure for 2XX retransmission management.
 * @var ixt_t
 */
  typedef struct ixt ixt_t;

/**
 * Structure for 2XX retransmission management.
 * @struct ixt
 */
  struct ixt {
    /* any ACK received that match this context will set counter to -1 */
    struct osip_dialog *dialog;         /**< related dialog */
    osip_message_t *msg2xx;             /**< buffer to retransmit */
    osip_message_t *ack;                /**< ack message if needed */
    struct timeval start;               /**< Time of first retransmission */
    int interval;                       /**< delay between retransmission, in ms */
    char *dest;                         /**< destination host */
    int port;                           /**< destination port */
    int sock;                           /**< socket to use */
    int counter;                        /**< start at 7 */
  };


/**
 * Structure for osip handling.
 * In order to use osip, you have to manage at least one global instance
 * of an osip_t element. Then, you'll register a set of required callbacks
 * and a set of optional ones.
 * @var osip_t
 */
  typedef struct osip osip_t;

/**
 * Structure for osip handling.
 * @struct osip
 */
  struct osip {

    void *application_context;     /**< User defined Pointer */

    void *ict_fastmutex;           /**< mutex for ICT transaction */
    void *ist_fastmutex;           /**< mutex for IST transaction */
    void *nict_fastmutex;          /**< mutex for NICT transaction */
    void *nist_fastmutex;          /**< mutex for NIST transaction */
    void *ixt_fastmutex;           /**< mutex for IXT transaction */
    void *id_mutex;                /**< mutex for unique transaction id generation */
    int transactionid;             /**< previous unique transaction id generation */

    /* list of transactions for ict, ist, nict, nist */
    osip_list_t osip_ict_transactions;          /**< list of ict transactions */
    osip_list_t osip_ist_transactions;          /**< list of ist transactions */
    osip_list_t osip_nict_transactions;         /**< list of nict transactions */
    osip_list_t osip_nist_transactions;         /**< list of nist transactions */

    osip_list_t ixt_retransmissions;            /**< list of ixt elements */

    osip_message_cb_t msg_callbacks[OSIP_MESSAGE_CALLBACK_COUNT];                /**< message callbacks */
    osip_kill_transaction_cb_t kill_callbacks[OSIP_KILL_CALLBACK_COUNT];         /**< kill callbacks */
    osip_transport_error_cb_t tp_error_callbacks[OSIP_TRANSPORT_ERROR_CALLBACK_COUNT];     /**< transport error callback */

    int (*cb_send_message) (osip_transaction_t *, osip_message_t *, char *, int, int);     /**< callback to send message */

    void *osip_ict_hastable;                              /**< htable of ict transactions */
    void *osip_ist_hastable;                              /**< htable of ist transactions */
    void *osip_nict_hastable;                             /**< htable of nict transactions */
    void *osip_nist_hastable;                             /**< htable of nist transactions */

  };

/**
 * Set a callback for each transaction operation. 
 * @param osip The element to work on.
 * @param type The event type to hook on.
 * @param cb The method to be called upon the event.
 */
  int osip_set_message_callback (osip_t * osip, int type, osip_message_cb_t cb);

/**
 * Set a callback for transaction operation related to the end of transactions. 
 * @param osip The element to work on.
 * @param type The event type to hook on.
 * @param cb The method to be called upon the event.
 */
  int osip_set_kill_transaction_callback (osip_t * osip, int type, osip_kill_transaction_cb_t cb);

/**
 * Set a callback for each transaction operation related to network error.
 * @param osip The element to work on.
 * @param type The event type to hook on.
 * @param cb The method to be called upon the event.
 */
  int osip_set_transport_error_callback (osip_t * osip, int type, osip_transport_error_cb_t cb);

/**
 * Structure for osip event handling.
 * A osip_event_t element will have a type and will be related
 * to a transaction. In the general case, it is used by the
 * application layer to give SIP messages to the oSIP finite
 * state machine.
 * @var osip_event_t
 */
  typedef struct osip_event osip_event_t;

/**
 * Structure for osip event handling.
 * @struct osip_event
 */
  struct osip_event {
    type_t type;                     /**< Event Type */
    int transactionid;               /**< identifier of the related osip transaction */
    osip_message_t *sip;             /**< SIP message (optional) */
  };



/**
 * Allocate an osip_transaction_t element.
 * @param transaction The element to allocate.
 * @param ctx_type The type of transaction. (ICT, IST, NICT, NIST)
 * @param osip The global instance of oSIP.
 * @param request The SIP request that initiate the transaction.
 */
  int osip_transaction_init (osip_transaction_t ** transaction, osip_fsm_type_t ctx_type, osip_t * osip, osip_message_t * request);
/**
 * Free all resource in a osip_transaction_t element.
 * @param transaction The element to free.
 */
  int osip_transaction_free (osip_transaction_t * transaction);
/**
 * Free all resource in a osip_transaction_t element.
 * This method does the same than osip_transaction_free() but it assumes
 * that the transaction is already removed from the list of transaction
 * in the osip stack. (to remove it use osip_xixt_remove(osip, transaction);
 * @param transaction The element to free.
 */
  int osip_transaction_free2 (osip_transaction_t * transaction);

/**
 * Search in a SIP response the destination where the message
 * should be sent.
 * @param response the message to work on.
 * @param address a pointer to receive the allocated host address.
 * @param portnum a pointer to receive the host port.
 */
  void osip_response_get_destination (osip_message_t * response, char **address, int *portnum);
/**
 * Set the host and port destination used for sending the SIP message.
 * This can be useful for an application with 'DIRECT ROOTING MODE'
 * NOTE: Instead, you should use the 'Route' header facility which
 * leads to the same behaviour.
 * @param ict The element to work on.
 * @param destination The destination host.
 * @param port The destination port.
 */
  int osip_ict_set_destination (osip_ict_t * ict, char *destination, int port);

/**
 * Set the host and port destination used for sending the SIP message.
 * This can be useful for an application with 'DIRECT ROOTING MODE'
 * NOTE: Instead, you should use the 'Route' header facility which
 * leads to the same behaviour.
 * @param nict The element to work on.
 * @param destination The destination host.
 * @param port The destination port.
 */
  int osip_nict_set_destination (osip_nict_t * nict, char *destination, int port);

/**
 * Add a SIP event in the fifo of a osip_transaction_t element.
 * @param transaction The element to work on.
 * @param evt The event to add.
 */
  int osip_transaction_add_event (osip_transaction_t * transaction, osip_event_t * evt);
/**
 * Consume one osip_event_t element previously added in the fifo.
 * NOTE: This method MUST NEVER be called within another call
 * of this method. (For example, you can't call osip_transaction_execute()
 * in a callback registered in the osip_t element.)
 * @param transaction The element to free.
 * @param evt The element to consume.
 */
  int osip_transaction_execute (osip_transaction_t * transaction, osip_event_t * evt);
/**
 * Set a pointer to your personal context associated with this transaction.
 * OBSOLETE: see osip_transaction_set_reserved1...
 * NOTE: this is a very useful method that allow you to avoid searching
 * for your personal context inside the registered callbacks.
 * You can initialise this pointer to your context right after
 * the creation of the osip_transaction_t element. Then, you'll be
 * able to get the address of your context by calling
 * osip_transaction_get_your_instance().
 * @param transaction The element to work on.
 * @param ptr The address of your context.
 */
  int osip_transaction_set_your_instance (osip_transaction_t * transaction, void *ptr);

/**
 * Set a pointer to your personal context associated with this transaction.
 * NOTE: this is a very useful method that allow you to avoid searching
 * for your personal context inside the registered callbacks.
 * You can initialise this pointer to your context right after
 * the creation of the osip_transaction_t element. Then, you'll be
 * able to get the address of your context by calling
 * osip_transaction_get_reserved1().
 * @param transaction The element to work on.
 * @param ptr The address of your context.
 */
  int osip_transaction_set_reserved1 (osip_transaction_t * transaction, void *ptr);
/**
 * Set a pointer to your personal context associated with this transaction.
 * NOTE: see osip_transaction_set_reserved1
 * @param transaction The element to work on.
 * @param ptr The address of your context.
 */
  int osip_transaction_set_reserved2 (osip_transaction_t * transaction, void *ptr);

/**
 * Set a pointer to your personal context associated with this transaction.
 * NOTE: see osip_transaction_set_reserved1
 * @param transaction The element to work on.
 * @param ptr The address of your context.
 */
  int osip_transaction_set_reserved3 (osip_transaction_t * transaction, void *ptr);

/**
 * Set a pointer to your personal context associated with this transaction.
 * NOTE: see osip_transaction_set_reserved1
 * @param transaction The element to work on.
 * @param ptr The address of your context.
 */
  int osip_transaction_set_reserved4 (osip_transaction_t * transaction, void *ptr);

/**
 * Set a pointer to your personal context associated with this transaction.
 * NOTE: see osip_transaction_set_reserved1
 * @param transaction The element to work on.
 * @param ptr The address of your context.
 */
  int osip_transaction_set_reserved5 (osip_transaction_t * transaction, void *ptr);

/**
 * Set a pointer to your personal context associated with this transaction.
 * NOTE: see osip_transaction_set_reserved1
 * @param transaction The element to work on.
 * @param ptr The address of your context.
 */
  int osip_transaction_set_reserved6 (osip_transaction_t * transaction, void *ptr);

/**
 * Get a pointer to your personal context associated with this transaction.
 * OBSOLETE: see osip_transaction_get_reserved1...
 * @param transaction The element to work on.
 */
  void *osip_transaction_get_your_instance (osip_transaction_t * transaction);

/**
 * Get a pointer to your personal context associated with this transaction.
 * @param transaction The element to work on.
 */
  void *osip_transaction_get_reserved1 (osip_transaction_t * transaction);

/**
 * Get a pointer to your personal context associated with this transaction.
 * @param transaction The element to work on.
 */
  void *osip_transaction_get_reserved2 (osip_transaction_t * transaction);

/**
 * Get a pointer to your personal context associated with this transaction.
 * @param transaction The element to work on.
 */
  void *osip_transaction_get_reserved3 (osip_transaction_t * transaction);

/**
 * Get a pointer to your personal context associated with this transaction.
 * @param transaction The element to work on.
 */
  void *osip_transaction_get_reserved4 (osip_transaction_t * transaction);

/**
 * Get a pointer to your personal context associated with this transaction.
 * @param transaction The element to work on.
 */
  void *osip_transaction_get_reserved5 (osip_transaction_t * transaction);

/**
 * Get a pointer to your personal context associated with this transaction.
 * @param transaction The element to work on.
 */
  void *osip_transaction_get_reserved6 (osip_transaction_t * transaction);

/**
 * Get target ip and port for this request.
 * (automaticly set by osip_transaction_init() for ict and nict)
 * @param transaction The element to work on.
 * @param ip The ip of host where to send initial request.
 * @param port The port where to send initial request.
 */
  int osip_transaction_get_destination (osip_transaction_t * transaction, char **ip, int *port);


/**
 * Set SRV lookup information to be used by state machine.
 *
 * @param transaction The element to work on.
 * @param record The SRV lookup results for this transaction.
 */
  int osip_transaction_set_srv_record (osip_transaction_t * transaction, osip_srv_record_t * record);

/**
 * Set NAPTR lookup information to be used by state machine.
 *
 * @param transaction The element to work on.
 * @param record The NAPTR lookup results for this transaction.
 */
  int osip_transaction_set_naptr_record (osip_transaction_t * transaction, osip_naptr_t * record);

/**
 * Set the socket for incoming message.
 *
 * @param transaction The element to work on.
 * @param sock The socket for incoming message.
 */
  int osip_transaction_set_in_socket (osip_transaction_t * transaction, int sock);
/**
 * Set the socket for outgoing message.
 *
 * @param transaction The element to work on.
 * @param sock The socket for outgoing message.
 */
  int osip_transaction_set_out_socket (osip_transaction_t * transaction, int sock);



/** 
 * Allocate an osip_t element.
 * @param osip the element to allocate.
 */
  int osip_init (osip_t ** osip);
/**
 * Free all resource in a osip_t element.
 * @param osip The element to release.
 */
  void osip_release (osip_t * osip);

/**
 * Set a pointer in a osip_t element.
 * This help to find your application layer in callbacks.
 * @param osip The element to work on.
 * @param pointer The element to set.
 */
  void osip_set_application_context (osip_t * osip, void *pointer);

/**
 * Get a pointer in a osip_t element.
 * This help to find your application layer in callbacks.
 * @param osip The element to work on.
 */
  void *osip_get_application_context (osip_t * osip);


/**
 * Remove a transaction from the osip stack.
 * @param osip The element to work on.
 * @param ict The transaction to add.
 */
  int osip_remove_transaction (osip_t * osip, osip_transaction_t * ict);


/**
 * Consume ALL pending osip_event_t previously added in the fifos of ict transactions.
 * @param osip The element to work on.
 */
  int osip_ict_execute (osip_t * osip);
/**
 * Consume ALL pending osip_event_t previously added in the fifos of ist transactions.
 * @param osip The element to work on.
 */
  int osip_ist_execute (osip_t * osip);
/**
 * Consume ALL pending osip_event_t previously added in the fifos of nict transactions.
 * @param osip The element to work on.
 */
  int osip_nict_execute (osip_t * osip);
/**
 * Consume ALL pending osip_event_t previously added in the fifos of nist transactions.
 * @param osip The element to work on.
 */
  int osip_nist_execute (osip_t * osip);

/**
 * Retreive the minimum timer value to be used by an application
 * so that the osip_timer_*_execute method don't have to be called
 * often.
 * 
 * @param osip The element to work on.
 * @param lower_tv The minimum timer when the application should wake up.
 */
  void osip_timers_gettimeout (osip_t * osip, struct timeval *lower_tv);

/**
 * Check if an ict transactions needs a timer event.
 * @param osip The element to work on.
 */
  void osip_timers_ict_execute (osip_t * osip);
/**
 * Check if an ist transactions needs a timer event.
 * @param osip The element to work on.
 */
  void osip_timers_ist_execute (osip_t * osip);
/**
 * Check if a nict transactions needs a timer event.
 * @param osip The element to work on.
 */
  void osip_timers_nict_execute (osip_t * osip);
/**
 * Check if a nist transactions needs a timer event.
 * @param osip The element to work on.
 */
  void osip_timers_nist_execute (osip_t * osip);

/* Take care of mutlithreading issuewhile using this method */
/**
 * Search for a transaction that match this event (MUST be a MESSAGE event).
 * @param transactions The list of transactions to work on.
 * @param evt The element representing the SIP MESSAGE.
 */
  osip_transaction_t *osip_transaction_find (osip_list_t * transactions, osip_event_t * evt);


#ifndef DOXYGEN
/**
 * Some race conditions can happen in multi threaded applications.
 * Use this method carefully.
 * <BR>Search for a transaction that match this event (MUST be a MESSAGE event).
 * @param osip The element to work on.
 * @param evt The element representing the SIP MESSAGE.
 */
#ifdef OSIP_MONOTHREAD
  osip_transaction_t *osip_find_transaction (osip_t * osip, osip_event_t * evt);
#endif

  osip_transaction_t *__osip_find_transaction (osip_t * osip, osip_event_t * evt, int consume);
#endif

/**
 * Search for a transaction that match this event (MUST be a MESSAGE event)
 * and add this event if a transaction is found..
 * @param osip The element to work on.
 * @param evt The element representing the SIP MESSAGE.
 */
  int osip_find_transaction_and_add_event (osip_t * osip, osip_event_t * evt);

/**
 * Create a transaction for this event (MUST be a SIP REQUEST event).
 * @param osip The element to work on.
 * @param evt The element representing the new SIP REQUEST.
 */
  osip_transaction_t *osip_create_transaction (osip_t * osip, osip_event_t * evt);

/**
 * Create a sipevent from a SIP message string.
 * @param buf The SIP message as a string.
 * @param length The length of the buffer to parse.
 */
  osip_event_t *osip_parse (const char *buf, size_t length);


/**
 * Send required retransmissions
 * @param osip The element to work on.
 */
  void osip_retransmissions_execute (osip_t * osip);

/**
 * Start out of fsm 200 Ok retransmissions. This is usefull for user-agents.
 * @param osip The osip_t structure.
 * @param dialog The dialog the 200 Ok is part of.
 * @param msg200ok The 200 ok response.
 * @param sock The socket to be used to send the message. (optional).
 */
  void osip_start_200ok_retransmissions (osip_t * osip, struct osip_dialog *dialog, osip_message_t * msg200ok, int sock);

/**
 * Start out of fsm ACK retransmissions. This is usefull for user-agents.
 * @param osip The osip_t structure.
 * @param dialog The dialog the ACK is part of.
 * @param ack The ACK that has just been sent in response to a 200 Ok.
 * @param dest The destination host.
 * @param port The destination port.
 * @param sock The socket to be used to send the message. (optional).
 */
  void osip_start_ack_retransmissions (osip_t * osip, struct osip_dialog *dialog, osip_message_t * ack, char *dest, int port, int sock);

/**
 * Stop the out of fsm 200 Ok retransmissions matching an incoming ACK.
 * @param osip The osip_t structure.
 * @param ack  The ack that has just been received.
 */
  struct osip_dialog *osip_stop_200ok_retransmissions (osip_t * osip, osip_message_t * ack);

/**
 * Stop out of fsm retransmissions (ACK or 200 Ok) associated to a given dialog.
 * This function must be called before freeing a dialog if out of fsm retransmissions
 * have been scheduled.
 * @param osip The osip_t structure
 * @param dialog The dialog.
 */
  void osip_stop_retransmissions_from_dialog (osip_t * osip, struct osip_dialog *dialog);


/**
 * Allocate a sipevent (we know this message is an OUTGOING SIP message).
 * @param sip The SIP message we want to send.
 */
  osip_event_t *osip_new_outgoing_sipmessage (osip_message_t * sip);

/**
 * Free all resource in a sipevent.
 * @param event The event to free.
 */
  void osip_event_free (osip_event_t * event);

/**
 * Register the callback used to send SIP message.
 * @param cf The osip element attached to the transaction.
 * @param cb The method we want to register.
 */
  void osip_set_cb_send_message (osip_t * cf, int (*cb) (osip_transaction_t *, osip_message_t *, char *, int, int));

/* FOR INCOMING TRANSACTION */
/**
 * Check if the sipevent is of type RCV_REQINVITE.
 * @param event the event to check.
 */
#define EVT_IS_RCV_INVITE(event)       (event->type==RCV_REQINVITE)
/**
 * Check if the sipevent is of type RCV_REQACK.
 * @param event the event to check.
 */
#define EVT_IS_RCV_ACK(event)          (event->type==RCV_REQACK)
/**
 * Check if the sipevent is of type RCV_REQUEST.
 * @param event the event to check.
 */
#define EVT_IS_RCV_REQUEST(event)      (event->type==RCV_REQUEST)
/**
 * Check if the sipevent is of type RCV_STATUS_1XX.
 * @param event the event to check.
 */
#define EVT_IS_RCV_STATUS_1XX(event)   (event->type==RCV_STATUS_1XX)
/**
 * Check if the sipevent is of type RCV_STATUS_2XX.
 * @param event the event to check.
 */
#define EVT_IS_RCV_STATUS_2XX(event)   (event->type==RCV_STATUS_2XX)
/**
 * Check if the sipevent is of type RCV_STATUS_3456XX.
 * @param event the event to check.
 */
#define EVT_IS_RCV_STATUS_3456XX(event)   (event->type==RCV_STATUS_3456XX)


/* FOR OUTGOING TRANSACTION */
/**
 * Check if the sipevent is of type SND_REQINVITE.
 * @param event the event to check.
 */
#define EVT_IS_SND_INVITE(event)       (event->type==SND_REQINVITE)
/**
 * Check if the sipevent is of type SND_REQACK.
 * @param event the event to check.
 */
#define EVT_IS_SND_ACK(event)          (event->type==SND_REQACK)
/**
 * Check if the sipevent is of type SND_REQUEST.
 * @param event the event to check.
 */
#define EVT_IS_SND_REQUEST(event)      (event->type==SND_REQUEST)
/**
 * Check if the sipevent is of type SND_STATUS_1XX.
 * @param event the event to check.
 */
#define EVT_IS_SND_STATUS_1XX(event)   (event->type==SND_STATUS_1XX)
/**
 * Check if the sipevent is of type SND_STATUS_2XX.
 * @param event the event to check.
 */
#define EVT_IS_SND_STATUS_2XX(event)   (event->type==SND_STATUS_2XX)
/**
 * Check if the sipevent is of type SND_STATUS_3456XX.
 * @param event the event to check.
 */
#define EVT_IS_SND_STATUS_3456XX(event)   (event->type==SND_STATUS_3456XX)
/**
 * Check if the sipevent is of an incoming SIP MESSAGE.
 * @param event the event to check.
 */
#define EVT_IS_INCOMINGMSG(event)      (event->type>=RCV_REQINVITE \
                	               &&event->type<=RCV_STATUS_3456XX)
/**
 * Check if the sipevent is of an incoming SIP REQUEST.
 * @param event the event to check.
 */
#define EVT_IS_INCOMINGREQ(event)      (EVT_IS_RCV_INVITE(event) \
                                       ||EVT_IS_RCV_ACK(event) \
                                       ||EVT_IS_RCV_REQUEST(event))
/**
 * Check if the sipevent is of an incoming SIP RESPONSE.
 * @param event the event to check.
 */
#define EVT_IS_INCOMINGRESP(event)     (EVT_IS_RCV_STATUS_1XX(event) \
                                       ||EVT_IS_RCV_STATUS_2XX(event) \
				       ||EVT_IS_RCV_STATUS_3456XX(event))
/**
 * Check if the sipevent is of an outgoing SIP MESSAGE.
 * @param event the event to check.
 */
#define EVT_IS_OUTGOINGMSG(event)      (event->type>=SND_REQINVITE \
                	               &&event->type<=SND_STATUS_3456XX)
/**
 * Check if the sipevent is of an outgoing SIP REQUEST.
 * @param event the event to check.
 */
#define EVT_IS_OUTGOINGREQ(event)      (EVT_IS_SND_INVITE(event) \
                                       ||EVT_IS_SND_ACK(event) \
                                       ||EVT_IS_SND_REQUEST(event))
/**
 * Check if the sipevent is of an outgoing SIP RESPONSE.
 * @param event the event to check.
 */
#define EVT_IS_OUTGOINGRESP(event)     (EVT_IS_SND_STATUS_1XX(event) \
                                       ||EVT_IS_SND_STATUS_2XX(event) \
				       ||EVT_IS_SND_STATUS_3456XX(event))

/**
 * Check if the sipevent is a SIP MESSAGE.
 * @param event the event to check.
 */
#define EVT_IS_MSG(event)              (event->type>=RCV_REQINVITE \
                	               &&event->type<=SND_STATUS_3456XX)
/**
 * Check if the sipevent is of type KILL_TRANSACTION.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param event the event to check.
 */
#define EVT_IS_KILL_TRANSACTION(event) (event->type==KILL_TRANSACTION)

#ifdef __cplusplus
}
#endif
/** @} */
#endif
