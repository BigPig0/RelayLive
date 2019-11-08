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

#ifndef _FSM_H_
#define _FSM_H_

#include <osipparser2/osip_message.h>
#include <osip2/osip.h>

#include "xixt.h"

#ifndef DOXYGEN

typedef struct osip_statemachine osip_statemachine_t;

struct osip_statemachine {
  struct _transition_t *transitions;
};

/**
 * Allocate a sipevent.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param type The type of the event.
 * @param transactionid The transaction id for this event.
 */
osip_event_t *__osip_event_new (type_t type, int transactionid);


/* This is for internal use only.                      */
type_t evt_set_type_incoming_sipmessage (osip_message_t * sip);
type_t evt_set_type_outgoing_sipmessage (osip_message_t * sip);

typedef struct _transition_t transition_t;

struct _transition_t {
  state_t state;
  type_t type;
  void (*method) (void *, void *);
  struct _transition_t *next;
  struct _transition_t *parent;
};

int fsm_callmethod (type_t type, state_t state, osip_statemachine_t * statemachine, void *sipevent, void *transaction);


/*!! THESE ARE FOR INTERNAL USE ONLY!! */
/* These methods are the "exection method" for the finite */
/* state machine.                                         */

/************************/
/* FSM  ---- > ICT      */
/************************/

void ict_snd_invite (osip_transaction_t * ict, osip_event_t * evt);
void osip_ict_timeout_a_event (osip_transaction_t * ict, osip_event_t * evt);
void osip_ict_timeout_b_event (osip_transaction_t * ict, osip_event_t * evt);
void osip_ict_timeout_d_event (osip_transaction_t * ict, osip_event_t * evt);
void ict_rcv_1xx (osip_transaction_t * ict, osip_event_t * evt);
void ict_rcv_2xx (osip_transaction_t * ict, osip_event_t * evt);
osip_message_t *ict_create_ack (osip_transaction_t * ict, osip_message_t * response);
void ict_rcv_3456xx (osip_transaction_t * ict, osip_event_t * evt);
void ict_retransmit_ack (osip_transaction_t * ict, osip_event_t * evt);

/************************/
/* FSM  ---- > IST      */
/************************/

void ist_rcv_invite (osip_transaction_t * ist, osip_event_t * evt);
void osip_ist_timeout_g_event (osip_transaction_t * ist, osip_event_t * evt);
void osip_ist_timeout_h_event (osip_transaction_t * ist, osip_event_t * evt);
void osip_ist_timeout_i_event (osip_transaction_t * ist, osip_event_t * evt);
void ist_snd_1xx (osip_transaction_t * ist, osip_event_t * evt);
void ist_snd_2xx (osip_transaction_t * ist, osip_event_t * evt);
void ist_snd_3456xx (osip_transaction_t * ist, osip_event_t * evt);
void ist_rcv_ack (osip_transaction_t * ist, osip_event_t * evt);

/***********************/
/* FSM  ---- > NICT    */
/***********************/

void nict_snd_request (osip_transaction_t * nict, osip_event_t * evt);
void osip_nict_timeout_e_event (osip_transaction_t * nict, osip_event_t * evt);
void osip_nict_timeout_f_event (osip_transaction_t * nict, osip_event_t * evt);
void osip_nict_timeout_k_event (osip_transaction_t * nict, osip_event_t * evt);
void nict_rcv_1xx (osip_transaction_t * nict, osip_event_t * evt);
void nict_rcv_23456xx (osip_transaction_t * nict, osip_event_t * evt);

/* void nict_rcv_23456xx2(osip_transaction_t *nict, osip_event_t *evt); */

/************************/
/* FSM  ---- > NIST     */
/************************/

void nist_rcv_request (osip_transaction_t * nist, osip_event_t * evt);
void nist_snd_1xx (osip_transaction_t * nist, osip_event_t * evt);
void nist_snd_23456xx (osip_transaction_t * nist, osip_event_t * evt);
void osip_nist_timeout_j_event (osip_transaction_t * nist, osip_event_t * evt);

/************************/
/* Internal Methods     */
/************************/


void __osip_message_callback (int type, osip_transaction_t *, osip_message_t *);
void __osip_kill_transaction_callback (int type, osip_transaction_t *);
void __osip_transport_error_callback (int type, osip_transaction_t *, int error);

/**
 * Set the state of the transaction.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param transaction The element to work on.
 * @param state The new state.
 */
int __osip_transaction_set_state (osip_transaction_t * transaction, state_t state);

/**
 * Check if the response match a server transaction.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param tr The transaction.
 * @param resp The SIP response received.
 */
int
  __osip_transaction_matching_response_osip_to_xict_17_1_3 (osip_transaction_t * tr, osip_message_t * resp);

/**
 * Check if the request match a client transaction.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param tr The transaction.
 * @param request The SIP request received.
 */
int
  __osip_transaction_matching_request_osip_to_xist_17_2_3 (osip_transaction_t * tr, osip_message_t * request);

osip_event_t *__osip_transaction_need_timer_x_event (void *xixt, struct timeval *timer, int cond_state, int transactionid, int TIMER_VAL);

int __osip_transaction_snd_xxx (osip_transaction_t * ist, osip_message_t * msg);

#endif

#endif
