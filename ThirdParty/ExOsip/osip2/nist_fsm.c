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

transition_t nist_transition[10] = {
  {
   NIST_PRE_TRYING,
   RCV_REQUEST,
   (void (*)(void *, void *)) &nist_rcv_request,
   &nist_transition[1], NULL}
  ,
  {
   NIST_TRYING,
   SND_STATUS_1XX,
   (void (*)(void *, void *)) &nist_snd_1xx,
   &nist_transition[2], NULL}
  ,
  {
   NIST_TRYING,
   SND_STATUS_2XX,
   (void (*)(void *, void *)) &nist_snd_23456xx,
   &nist_transition[3], NULL}
  ,
  {
   NIST_TRYING,
   SND_STATUS_3456XX,
   (void (*)(void *, void *)) &nist_snd_23456xx,
   &nist_transition[4], NULL}
  ,
  {
   NIST_PROCEEDING,
   SND_STATUS_1XX,
   (void (*)(void *, void *)) &nist_snd_1xx,
   &nist_transition[5], NULL}
  ,
  {
   NIST_PROCEEDING,
   SND_STATUS_2XX,
   (void (*)(void *, void *)) &nist_snd_23456xx,
   &nist_transition[6], NULL}
  ,
  {
   NIST_PROCEEDING,
   SND_STATUS_3456XX,
   (void (*)(void *, void *)) &nist_snd_23456xx,
   &nist_transition[7], NULL}
  ,
  {
   NIST_PROCEEDING,
   RCV_REQUEST,
   (void (*)(void *, void *)) &nist_rcv_request,
   &nist_transition[8], NULL}
  ,
  {
   NIST_COMPLETED,
   TIMEOUT_J,
   (void (*)(void *, void *)) &osip_nist_timeout_j_event,
   &nist_transition[9], NULL}
  ,
  {
   NIST_COMPLETED,
   RCV_REQUEST,
   (void (*)(void *, void *)) &nist_rcv_request,
   NULL, NULL}
};

osip_statemachine_t nist_fsm = { nist_transition };

static void
nist_handle_transport_error (osip_transaction_t * nist, int err)
{
  __osip_transport_error_callback (OSIP_NIST_TRANSPORT_ERROR, nist, err);
  __osip_transaction_set_state (nist, NIST_TERMINATED);
  __osip_kill_transaction_callback (OSIP_NIST_KILL_TRANSACTION, nist);
  /* TODO: MUST BE DELETED NOW */
}

void
nist_rcv_request (osip_transaction_t * nist, osip_event_t * evt)
{
  int i;

  if (nist->state == NIST_PRE_TRYING) { /* announce new REQUEST */
    /* Here we have ist->orig_request == NULL */
    nist->orig_request = evt->sip;

    if (MSG_IS_REGISTER (evt->sip))
      __osip_message_callback (OSIP_NIST_REGISTER_RECEIVED, nist, nist->orig_request);
    else if (MSG_IS_BYE (evt->sip))
      __osip_message_callback (OSIP_NIST_BYE_RECEIVED, nist, nist->orig_request);
    else if (MSG_IS_OPTIONS (evt->sip))
      __osip_message_callback (OSIP_NIST_OPTIONS_RECEIVED, nist, nist->orig_request);
    else if (MSG_IS_INFO (evt->sip))
      __osip_message_callback (OSIP_NIST_INFO_RECEIVED, nist, nist->orig_request);
    else if (MSG_IS_CANCEL (evt->sip))
      __osip_message_callback (OSIP_NIST_CANCEL_RECEIVED, nist, nist->orig_request);
    else if (MSG_IS_NOTIFY (evt->sip))
      __osip_message_callback (OSIP_NIST_NOTIFY_RECEIVED, nist, nist->orig_request);
    else if (MSG_IS_SUBSCRIBE (evt->sip))
      __osip_message_callback (OSIP_NIST_SUBSCRIBE_RECEIVED, nist, nist->orig_request);
    else
      __osip_message_callback (OSIP_NIST_UNKNOWN_REQUEST_RECEIVED, nist, nist->orig_request);
  }
  else {                        /* NIST_PROCEEDING or NIST_COMPLETED */

    /* delete retransmission */
    osip_message_free (evt->sip);

    __osip_message_callback (OSIP_NIST_REQUEST_RECEIVED_AGAIN, nist, nist->orig_request);
    if (nist->last_response != NULL) {  /* retransmit last response */
      i = __osip_transaction_snd_xxx (nist, nist->last_response);
      if (i != 0) {
        nist_handle_transport_error (nist, i);
        return;
      }
      else {
        if (MSG_IS_STATUS_1XX (nist->last_response))
          __osip_message_callback (OSIP_NIST_STATUS_1XX_SENT, nist, nist->last_response);
        else if (MSG_IS_STATUS_2XX (nist->last_response))
          __osip_message_callback (OSIP_NIST_STATUS_2XX_SENT_AGAIN, nist, nist->last_response);
        else
          __osip_message_callback (OSIP_NIST_STATUS_3456XX_SENT_AGAIN, nist, nist->last_response);
        return;
      }
    }
    /* we are already in the proper state */
    return;
  }

  /* we come here only if it was the first REQUEST received */
  __osip_transaction_set_state (nist, NIST_TRYING);
}

void
nist_snd_1xx (osip_transaction_t * nist, osip_event_t * evt)
{
  int i;

  if (nist->last_response != NULL) {
    osip_message_free (nist->last_response);
  }
  nist->last_response = evt->sip;

  i = __osip_transaction_snd_xxx (nist, nist->last_response);
  if (i != 0) {
    nist_handle_transport_error (nist, i);
    return;
  }
  else
    __osip_message_callback (OSIP_NIST_STATUS_1XX_SENT, nist, nist->last_response);

  __osip_transaction_set_state (nist, NIST_PROCEEDING);
}

void
nist_snd_23456xx (osip_transaction_t * nist, osip_event_t * evt)
{
  int i;

  if (nist->last_response != NULL) {
    osip_message_free (nist->last_response);
  }
  nist->last_response = evt->sip;

  i = __osip_transaction_snd_xxx (nist, nist->last_response);
  if (i != 0) {
    nist_handle_transport_error (nist, i);
    return;
  }
  else {
    if (EVT_IS_SND_STATUS_2XX (evt))
      __osip_message_callback (OSIP_NIST_STATUS_2XX_SENT, nist, nist->last_response);
    else if (MSG_IS_STATUS_3XX (nist->last_response))
      __osip_message_callback (OSIP_NIST_STATUS_3XX_SENT, nist, nist->last_response);
    else if (MSG_IS_STATUS_4XX (nist->last_response))
      __osip_message_callback (OSIP_NIST_STATUS_4XX_SENT, nist, nist->last_response);
    else if (MSG_IS_STATUS_5XX (nist->last_response))
      __osip_message_callback (OSIP_NIST_STATUS_5XX_SENT, nist, nist->last_response);
    else
      __osip_message_callback (OSIP_NIST_STATUS_6XX_SENT, nist, nist->last_response);
  }

  if (nist->state != NIST_COMPLETED) {  /* start J timer */
    osip_gettimeofday (&nist->nist_context->timer_j_start, NULL);
    add_gettimeofday (&nist->nist_context->timer_j_start, nist->nist_context->timer_j_length);
  }

  __osip_transaction_set_state (nist, NIST_COMPLETED);
}


void
osip_nist_timeout_j_event (osip_transaction_t * nist, osip_event_t * evt)
{
  nist->nist_context->timer_j_length = -1;
  nist->nist_context->timer_j_start.tv_sec = -1;

  __osip_transaction_set_state (nist, NIST_TERMINATED);
  __osip_kill_transaction_callback (OSIP_NIST_KILL_TRANSACTION, nist);
}
