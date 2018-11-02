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

#include <osip2/internal.h>
#include <osip2/osip.h>

#include "fsm.h"

transition_t nict_transition[12] = {
  {
   NICT_PRE_TRYING,
   SND_REQUEST,
   (void (*)(void *, void *)) &nict_snd_request,
   &nict_transition[1], NULL}
  ,
  {
   NICT_TRYING,
   TIMEOUT_F,
   (void (*)(void *, void *)) &osip_nict_timeout_f_event,
   &nict_transition[2], NULL}
  ,
  {
   NICT_TRYING,
   TIMEOUT_E,
   (void (*)(void *, void *)) &osip_nict_timeout_e_event,
   &nict_transition[3], NULL}
  ,
  {
   NICT_TRYING,
   RCV_STATUS_1XX,
   (void (*)(void *, void *)) &nict_rcv_1xx,
   &nict_transition[4], NULL}
  ,
  {
   NICT_TRYING,
   RCV_STATUS_2XX,
   (void (*)(void *, void *)) &nict_rcv_23456xx,
   &nict_transition[5], NULL}
  ,
  {
   NICT_TRYING,
   RCV_STATUS_3456XX,
   (void (*)(void *, void *)) &nict_rcv_23456xx,
   &nict_transition[6], NULL}
  ,
  {
   NICT_PROCEEDING,
   TIMEOUT_F,
   (void (*)(void *, void *)) &osip_nict_timeout_f_event,
   &nict_transition[7], NULL}
  ,
  {
   NICT_PROCEEDING,
   TIMEOUT_E,
   (void (*)(void *, void *)) &osip_nict_timeout_e_event,
   &nict_transition[8], NULL}
  ,
  {
   NICT_PROCEEDING,
   RCV_STATUS_1XX,
   (void (*)(void *, void *)) &nict_rcv_1xx,
   &nict_transition[9], NULL}
  ,
  {
   NICT_PROCEEDING,
   RCV_STATUS_2XX,
   (void (*)(void *, void *)) &nict_rcv_23456xx,
   &nict_transition[10], NULL}
  ,
  {
   NICT_PROCEEDING,
   RCV_STATUS_3456XX,
   (void (*)(void *, void *)) &nict_rcv_23456xx,
   &nict_transition[11], NULL}
  ,
  {
   NICT_COMPLETED,
   TIMEOUT_K,
   (void (*)(void *, void *)) &osip_nict_timeout_k_event,
   NULL, NULL}
};

osip_statemachine_t nict_fsm = { nict_transition };

static void
nict_handle_transport_error (osip_transaction_t * nict, int err)
{
  __osip_transport_error_callback (OSIP_NICT_TRANSPORT_ERROR, nict, err);
  __osip_transaction_set_state (nict, NICT_TERMINATED);
  __osip_kill_transaction_callback (OSIP_NICT_KILL_TRANSACTION, nict);
  /* TODO: MUST BE DELETED NOW */
}

void
nict_snd_request (osip_transaction_t * nict, osip_event_t * evt)
{
  int i;
  osip_t *osip = (osip_t *) nict->config;

  /* Here we have ict->orig_request == NULL */
  nict->orig_request = evt->sip;

  i = osip->cb_send_message (nict, evt->sip, nict->nict_context->destination, nict->nict_context->port, nict->out_socket);

  if (i >= 0) {
    /* invoke the right callback! */
    if (MSG_IS_REGISTER (evt->sip))
      __osip_message_callback (OSIP_NICT_REGISTER_SENT, nict, nict->orig_request);
    else if (MSG_IS_BYE (evt->sip))
      __osip_message_callback (OSIP_NICT_BYE_SENT, nict, nict->orig_request);
    else if (MSG_IS_OPTIONS (evt->sip))
      __osip_message_callback (OSIP_NICT_OPTIONS_SENT, nict, nict->orig_request);
    else if (MSG_IS_INFO (evt->sip))
      __osip_message_callback (OSIP_NICT_INFO_SENT, nict, nict->orig_request);
    else if (MSG_IS_CANCEL (evt->sip))
      __osip_message_callback (OSIP_NICT_CANCEL_SENT, nict, nict->orig_request);
    else if (MSG_IS_NOTIFY (evt->sip))
      __osip_message_callback (OSIP_NICT_NOTIFY_SENT, nict, nict->orig_request);
    else if (MSG_IS_SUBSCRIBE (evt->sip))
      __osip_message_callback (OSIP_NICT_SUBSCRIBE_SENT, nict, nict->orig_request);
    else
      __osip_message_callback (OSIP_NICT_UNKNOWN_REQUEST_SENT, nict, nict->orig_request);
#ifndef USE_BLOCKINGSOCKET
    /*
       stop timer E in reliable transport - non blocking socket: 
       the message was just sent
     */
    {
      osip_via_t *via;
      char *proto;
      int k;
      k = osip_message_get_via (nict->orig_request, 0, &via);   /* get top via */
      if (k < 0) {
        nict_handle_transport_error (nict, -1);
        return;
      }
      proto = via_get_protocol (via);
      if (proto == NULL) {
        nict_handle_transport_error (nict, -1);
        return;
      }
      if (i == 0) {               /* but message was really sent */
        if (osip_strcasecmp (proto, "TCP") != 0 && osip_strcasecmp (proto, "TLS") != 0 && osip_strcasecmp (proto, "SCTP") != 0) {
        }
        else {                    /* reliable protocol is used: */
          nict->nict_context->timer_e_length = -1;        /* E is not ACTIVE */
          nict->nict_context->timer_e_start.tv_sec = -1;
        }
      } else {
        if (osip_strcasecmp (proto, "TCP") != 0 && osip_strcasecmp (proto, "TLS") != 0 && osip_strcasecmp (proto, "SCTP") != 0) {
        }
        else {                    /* reliable protocol is used: */
          nict->nict_context->timer_e_length = DEFAULT_T1_TCP_PROGRESS;
        }
      }
    }
#endif
    if (nict->nict_context->timer_e_length > 0) {
      osip_gettimeofday (&nict->nict_context->timer_e_start, NULL);
      add_gettimeofday (&nict->nict_context->timer_e_start, nict->nict_context->timer_e_length);
    }
    __osip_transaction_set_state (nict, NICT_TRYING);
  }
  else {
    nict_handle_transport_error (nict, i);
  }
}

void
osip_nict_timeout_e_event (osip_transaction_t * nict, osip_event_t * evt)
{
  osip_t *osip = (osip_t *) nict->config;
  int i;

  /* reset timer */
  if (nict->state == NICT_TRYING) {
    if (nict->nict_context->timer_e_length < DEFAULT_T1)
      nict->nict_context->timer_e_length = nict->nict_context->timer_e_length + DEFAULT_T1_TCP_PROGRESS;
    else
      nict->nict_context->timer_e_length = nict->nict_context->timer_e_length * 2;
    if (nict->nict_context->timer_e_length > DEFAULT_T2)
      nict->nict_context->timer_e_length = DEFAULT_T2;
  }
  else                          /* in PROCEEDING STATE, TIMER is always DEFAULT_T2 */
    nict->nict_context->timer_e_length = DEFAULT_T2;

  osip_gettimeofday (&nict->nict_context->timer_e_start, NULL);
  add_gettimeofday (&nict->nict_context->timer_e_start, nict->nict_context->timer_e_length);

  /* retransmit REQUEST */
  i = osip->cb_send_message (nict, nict->orig_request, nict->nict_context->destination, nict->nict_context->port, nict->out_socket);
  if (i < 0) {
    nict_handle_transport_error (nict, i);
    return;
  }
#ifndef USE_BLOCKINGSOCKET
  /*
     stop timer E in reliable transport - non blocking socket: 
     the message was just sent
   */
  if (i == 0) {                 /* but message was really sent */
    osip_via_t *via;
    char *proto;

    i = osip_message_get_via (nict->orig_request, 0, &via);     /* get top via */
    if (i < 0) {
      nict_handle_transport_error (nict, -1);
      return;
    }
    proto = via_get_protocol (via);
    if (proto == NULL) {
      nict_handle_transport_error (nict, -1);
      return;
    }
    if (osip_strcasecmp (proto, "TCP") != 0 && osip_strcasecmp (proto, "TLS") != 0 && osip_strcasecmp (proto, "SCTP") != 0) {
    }
    else {                      /* reliable protocol is used: */
      nict->nict_context->timer_e_length = -1;  /* E is not ACTIVE */
      nict->nict_context->timer_e_start.tv_sec = -1;
    }
  }
#endif
  if (i == 0)                   /* but message was really sent */
    __osip_message_callback (OSIP_NICT_REQUEST_SENT_AGAIN, nict, nict->orig_request);
}

void
osip_nict_timeout_f_event (osip_transaction_t * nict, osip_event_t * evt)
{
  nict->nict_context->timer_f_length = -1;
  nict->nict_context->timer_f_start.tv_sec = -1;

  __osip_message_callback (OSIP_NICT_STATUS_TIMEOUT, nict, evt->sip);
  __osip_transaction_set_state (nict, NICT_TERMINATED);
  __osip_kill_transaction_callback (OSIP_NICT_KILL_TRANSACTION, nict);
}

void
osip_nict_timeout_k_event (osip_transaction_t * nict, osip_event_t * evt)
{
  nict->nict_context->timer_k_length = -1;
  nict->nict_context->timer_k_start.tv_sec = -1;

  __osip_transaction_set_state (nict, NICT_TERMINATED);
  __osip_kill_transaction_callback (OSIP_NICT_KILL_TRANSACTION, nict);
}

void
nict_rcv_1xx (osip_transaction_t * nict, osip_event_t * evt)
{
  /* leave this answer to the core application */

  if (nict->last_response != NULL) {
    osip_message_free (nict->last_response);
  }
  nict->last_response = evt->sip;

  /* for unreliable transport increase the retransmission timeout */
  if (nict->nict_context->timer_e_length > 0) {
    nict->nict_context->timer_e_length = DEFAULT_T2;
    osip_gettimeofday (&nict->nict_context->timer_e_start, NULL);
    add_gettimeofday (&nict->nict_context->timer_e_start, nict->nict_context->timer_e_length);
  }

  __osip_message_callback (OSIP_NICT_STATUS_1XX_RECEIVED, nict, evt->sip);
  __osip_transaction_set_state (nict, NICT_PROCEEDING);
}

void
nict_rcv_23456xx (osip_transaction_t * nict, osip_event_t * evt)
{
  /* leave this answer to the core application */

  if (nict->last_response != NULL) {
    osip_message_free (nict->last_response);
  }
  nict->last_response = evt->sip;

  if (EVT_IS_RCV_STATUS_2XX (evt))
    __osip_message_callback (OSIP_NICT_STATUS_2XX_RECEIVED, nict, nict->last_response);
  else if (MSG_IS_STATUS_3XX (nict->last_response))
    __osip_message_callback (OSIP_NICT_STATUS_3XX_RECEIVED, nict, nict->last_response);
  else if (MSG_IS_STATUS_4XX (nict->last_response))
    __osip_message_callback (OSIP_NICT_STATUS_4XX_RECEIVED, nict, nict->last_response);
  else if (MSG_IS_STATUS_5XX (nict->last_response))
    __osip_message_callback (OSIP_NICT_STATUS_5XX_RECEIVED, nict, nict->last_response);
  else
    __osip_message_callback (OSIP_NICT_STATUS_6XX_RECEIVED, nict, nict->last_response);

  if (nict->state != NICT_COMPLETED) {  /* reset timer K */
    osip_gettimeofday (&nict->nict_context->timer_k_start, NULL);
    add_gettimeofday (&nict->nict_context->timer_k_start, nict->nict_context->timer_k_length);
  }
  __osip_transaction_set_state (nict, NICT_COMPLETED);
}
