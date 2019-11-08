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

transition_t ist_transition[11] = {
  {
   IST_PRE_PROCEEDING,
   RCV_REQINVITE,
   (void (*)(void *, void *)) &ist_rcv_invite,
   &ist_transition[1], NULL}
  ,
  {
   IST_PROCEEDING,
   RCV_REQINVITE,
   (void (*)(void *, void *)) &ist_rcv_invite,
   &ist_transition[2], NULL}
  ,
  {
   IST_COMPLETED,
   RCV_REQINVITE,
   (void (*)(void *, void *)) &ist_rcv_invite,
   &ist_transition[3], NULL}
  ,
  {
   IST_COMPLETED,
   TIMEOUT_G,
   (void (*)(void *, void *)) &osip_ist_timeout_g_event,
   &ist_transition[4], NULL}
  ,
  {
   IST_COMPLETED,
   TIMEOUT_H,
   (void (*)(void *, void *)) &osip_ist_timeout_h_event,
   &ist_transition[5], NULL}
  ,
  {
   IST_PROCEEDING,
   SND_STATUS_1XX,
   (void (*)(void *, void *)) &ist_snd_1xx,
   &ist_transition[6], NULL}
  ,
  {
   IST_PROCEEDING,
   SND_STATUS_2XX,
   (void (*)(void *, void *)) &ist_snd_2xx,
   &ist_transition[7], NULL}
  ,
  {
   IST_PROCEEDING,
   SND_STATUS_3456XX,
   (void (*)(void *, void *)) &ist_snd_3456xx,
   &ist_transition[8], NULL}
  ,
  {
   IST_COMPLETED,
   RCV_REQACK,
   (void (*)(void *, void *)) &ist_rcv_ack,
   &ist_transition[9], NULL}
  ,
  {
   IST_CONFIRMED,
   RCV_REQACK,
   (void (*)(void *, void *)) &ist_rcv_ack,
   &ist_transition[10], NULL}
  ,
  {
   IST_CONFIRMED,
   TIMEOUT_I,
   (void (*)(void *, void *)) &osip_ist_timeout_i_event,
   NULL, NULL}
};

osip_statemachine_t ist_fsm = { ist_transition };

static void
ist_handle_transport_error (osip_transaction_t * ist, int err)
{
  __osip_transport_error_callback (OSIP_IST_TRANSPORT_ERROR, ist, err);
  __osip_transaction_set_state (ist, IST_TERMINATED);
  __osip_kill_transaction_callback (OSIP_IST_KILL_TRANSACTION, ist);
  /* TODO: MUST BE DELETED NOW */
}

void
ist_rcv_invite (osip_transaction_t * ist, osip_event_t * evt)
{
  int i;

  if (ist->state == IST_PRE_PROCEEDING) {       /* announce new INVITE */
    /* Here we have ist->orig_request == NULL */
    ist->orig_request = evt->sip;

    __osip_message_callback (OSIP_IST_INVITE_RECEIVED, ist, evt->sip);
  }
  else {                        /* IST_PROCEEDING or IST_COMPLETED */

    /* delete retransmission */
    osip_message_free (evt->sip);

    __osip_message_callback (OSIP_IST_INVITE_RECEIVED_AGAIN, ist, ist->orig_request);
    if (ist->last_response != NULL) {   /* retransmit last response */
      i = __osip_transaction_snd_xxx (ist, ist->last_response);
      if (i != 0) {
        ist_handle_transport_error (ist, i);
        return;
      }
      else {
        if (MSG_IS_STATUS_1XX (ist->last_response))
          __osip_message_callback (OSIP_IST_STATUS_1XX_SENT, ist, ist->last_response);
        else if (MSG_IS_STATUS_2XX (ist->last_response))
          __osip_message_callback (OSIP_IST_STATUS_2XX_SENT_AGAIN, ist, ist->last_response);
        else
          __osip_message_callback (OSIP_IST_STATUS_3456XX_SENT_AGAIN, ist, ist->last_response);
      }
    }
    return;
  }

  /* we come here only if it was the first INVITE received */
  __osip_transaction_set_state (ist, IST_PROCEEDING);
}

void
osip_ist_timeout_g_event (osip_transaction_t * ist, osip_event_t * evt)
{
  int i;

  ist->ist_context->timer_g_length = ist->ist_context->timer_g_length * 2;
  if (ist->ist_context->timer_g_length > DEFAULT_T2)
    ist->ist_context->timer_g_length = DEFAULT_T2;
  osip_gettimeofday (&ist->ist_context->timer_g_start, NULL);
  add_gettimeofday (&ist->ist_context->timer_g_start, ist->ist_context->timer_g_length);

  i = __osip_transaction_snd_xxx (ist, ist->last_response);
  if (i != 0) {
    ist_handle_transport_error (ist, i);
    return;
  }
  __osip_message_callback (OSIP_IST_STATUS_3456XX_SENT_AGAIN, ist, ist->last_response);
}

void
osip_ist_timeout_h_event (osip_transaction_t * ist, osip_event_t * evt)
{
  ist->ist_context->timer_h_length = -1;
  ist->ist_context->timer_h_start.tv_sec = -1;

  __osip_transaction_set_state (ist, IST_TERMINATED);
  __osip_kill_transaction_callback (OSIP_IST_KILL_TRANSACTION, ist);
}

void
osip_ist_timeout_i_event (osip_transaction_t * ist, osip_event_t * evt)
{
  ist->ist_context->timer_i_length = -1;
  ist->ist_context->timer_i_start.tv_sec = -1;

  __osip_transaction_set_state (ist, IST_TERMINATED);
  __osip_kill_transaction_callback (OSIP_IST_KILL_TRANSACTION, ist);
}

void
ist_snd_1xx (osip_transaction_t * ist, osip_event_t * evt)
{
  int i;

  if (ist->last_response != NULL) {
    osip_message_free (ist->last_response);
  }
  ist->last_response = evt->sip;

  i = __osip_transaction_snd_xxx (ist, evt->sip);
  if (i != 0) {
    ist_handle_transport_error (ist, i);
    return;
  }
  else
    __osip_message_callback (OSIP_IST_STATUS_1XX_SENT, ist, ist->last_response);

  /* we are already in the proper state */
  return;
}

void
ist_snd_2xx (osip_transaction_t * ist, osip_event_t * evt)
{
  int i;

  if (ist->last_response != NULL) {
    osip_message_free (ist->last_response);
  }
  ist->last_response = evt->sip;

  i = __osip_transaction_snd_xxx (ist, evt->sip);
  if (i != 0) {
    ist_handle_transport_error (ist, i);
    return;
  }
  else {
    __osip_message_callback (OSIP_IST_STATUS_2XX_SENT, ist, ist->last_response);
    __osip_transaction_set_state (ist, IST_TERMINATED);
    __osip_kill_transaction_callback (OSIP_IST_KILL_TRANSACTION, ist);
  }
  return;
}

void
ist_snd_3456xx (osip_transaction_t * ist, osip_event_t * evt)
{
  int i;

  if (ist->last_response != NULL) {
    osip_message_free (ist->last_response);
  }
  ist->last_response = evt->sip;

  i = __osip_transaction_snd_xxx (ist, evt->sip);
  if (i != 0) {
    ist_handle_transport_error (ist, i);
    return;
  }
  else {
    if (MSG_IS_STATUS_3XX (ist->last_response))
      __osip_message_callback (OSIP_IST_STATUS_3XX_SENT, ist, ist->last_response);
    else if (MSG_IS_STATUS_4XX (ist->last_response))
      __osip_message_callback (OSIP_IST_STATUS_4XX_SENT, ist, ist->last_response);
    else if (MSG_IS_STATUS_5XX (ist->last_response))
      __osip_message_callback (OSIP_IST_STATUS_5XX_SENT, ist, ist->last_response);
    else
      __osip_message_callback (OSIP_IST_STATUS_6XX_SENT, ist, ist->last_response);
  }

  if (ist->ist_context->timer_g_length != -1) {
    osip_gettimeofday (&ist->ist_context->timer_g_start, NULL);
    add_gettimeofday (&ist->ist_context->timer_g_start, ist->ist_context->timer_g_length);
  }
  osip_gettimeofday (&ist->ist_context->timer_h_start, NULL);
  add_gettimeofday (&ist->ist_context->timer_h_start, ist->ist_context->timer_h_length);
  __osip_transaction_set_state (ist, IST_COMPLETED);
  return;
}

void
ist_rcv_ack (osip_transaction_t * ist, osip_event_t * evt)
{
  if (ist->ack != NULL) {
    osip_message_free (ist->ack);
  }

  ist->ack = evt->sip;

  if (ist->state == IST_COMPLETED)
    __osip_message_callback (OSIP_IST_ACK_RECEIVED, ist, ist->ack);
  else                          /* IST_CONFIRMED */
    __osip_message_callback (OSIP_IST_ACK_RECEIVED_AGAIN, ist, ist->ack);
  /* set the timer to 0 for reliable, and T4 for unreliable (already set) */
  osip_gettimeofday (&ist->ist_context->timer_i_start, NULL);
  add_gettimeofday (&ist->ist_context->timer_i_start, ist->ist_context->timer_i_length);
  __osip_transaction_set_state (ist, IST_CONFIRMED);
}
