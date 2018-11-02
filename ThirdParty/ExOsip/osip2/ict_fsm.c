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
#include "xixt.h"

transition_t ict_transition[11] = {
  {
   ICT_PRE_CALLING,
   SND_REQINVITE,
   (void (*)(void *, void *)) &ict_snd_invite,
   &ict_transition[1], NULL}
  ,
  {
   ICT_CALLING,
   TIMEOUT_A,
   (void (*)(void *, void *)) &osip_ict_timeout_a_event,
   &ict_transition[2], NULL}
  ,
  {
   ICT_CALLING,
   TIMEOUT_B,
   (void (*)(void *, void *)) &osip_ict_timeout_b_event,
   &ict_transition[3], NULL}
  ,
  {ICT_CALLING,
   RCV_STATUS_1XX,
   (void (*)(void *, void *)) &ict_rcv_1xx,
   &ict_transition[4], NULL}
  ,
  {ICT_CALLING,
   RCV_STATUS_2XX,
   (void (*)(void *, void *)) &ict_rcv_2xx,
   &ict_transition[5], NULL}
  ,
  {ICT_CALLING,
   RCV_STATUS_3456XX,
   (void (*)(void *, void *)) &ict_rcv_3456xx,
   &ict_transition[6], NULL}
  ,
  {ICT_PROCEEDING,
   RCV_STATUS_1XX,
   (void (*)(void *, void *)) &ict_rcv_1xx,
   &ict_transition[7], NULL}
  ,
  {ICT_PROCEEDING,
   RCV_STATUS_2XX,
   (void (*)(void *, void *)) &ict_rcv_2xx,
   &ict_transition[8], NULL}
  ,
  {ICT_PROCEEDING,
   RCV_STATUS_3456XX,
   (void (*)(void *, void *)) &ict_rcv_3456xx,
   &ict_transition[9], NULL}
  ,
  {ICT_COMPLETED,
   RCV_STATUS_3456XX,
   (void (*)(void *, void *)) &ict_retransmit_ack,
   &ict_transition[10], NULL}
  ,
  {ICT_COMPLETED,
   TIMEOUT_D,
   (void (*)(void *, void *)) &osip_ict_timeout_d_event,
   NULL, NULL}
};

osip_statemachine_t ict_fsm = { ict_transition };

static void
ict_handle_transport_error (osip_transaction_t * ict, int err)
{
  __osip_transport_error_callback (OSIP_ICT_TRANSPORT_ERROR, ict, err);
  __osip_transaction_set_state (ict, ICT_TERMINATED);
  __osip_kill_transaction_callback (OSIP_ICT_KILL_TRANSACTION, ict);
  /* TODO: MUST BE DELETED NOW */
}

void
ict_snd_invite (osip_transaction_t * ict, osip_event_t * evt)
{
  int i;
  osip_t *osip = (osip_t *) ict->config;

  /* Here we have ict->orig_request == NULL */
  ict->orig_request = evt->sip;

  i = osip->cb_send_message (ict, evt->sip, ict->ict_context->destination, ict->ict_context->port, ict->out_socket);

  if (i < 0) {
    ict_handle_transport_error (ict, i);
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

    i = osip_message_get_via (ict->orig_request, 0, &via);      /* get top via */
    if (i < 0) {
      ict_handle_transport_error (ict, i);
      return;
    }
    proto = via_get_protocol (via);
    if (proto == NULL) {
      ict_handle_transport_error (ict, i);
      return;
    }
    if (osip_strcasecmp (proto, "TCP") != 0 && osip_strcasecmp (proto, "TLS") != 0 && osip_strcasecmp (proto, "SCTP") != 0) {
    }
    else {                      /* reliable protocol is used: */
      ict->ict_context->timer_a_length = -1;    /* A is not ACTIVE */
      ict->ict_context->timer_a_start.tv_sec = -1;
    }
  }
#endif

  __osip_message_callback (OSIP_ICT_INVITE_SENT, ict, ict->orig_request);
  __osip_transaction_set_state (ict, ICT_CALLING);
}

void
osip_ict_timeout_a_event (osip_transaction_t * ict, osip_event_t * evt)
{
  osip_t *osip = (osip_t *) ict->config;
  int i;

  /* reset timer */
  ict->ict_context->timer_a_length = ict->ict_context->timer_a_length * 2;
  osip_gettimeofday (&ict->ict_context->timer_a_start, NULL);
  add_gettimeofday (&ict->ict_context->timer_a_start, ict->ict_context->timer_a_length);

  /* retransmit REQUEST */
  i = osip->cb_send_message (ict, ict->orig_request, ict->ict_context->destination, ict->ict_context->port, ict->out_socket);
  if (i < 0) {
    ict_handle_transport_error (ict, i);
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

    i = osip_message_get_via (ict->orig_request, 0, &via);      /* get top via */
    if (i < 0) {
      ict_handle_transport_error (ict, i);
      return;
    }
    proto = via_get_protocol (via);
    if (proto == NULL) {
      ict_handle_transport_error (ict, i);
      return;
    }
    if (osip_strcasecmp (proto, "TCP") != 0 && osip_strcasecmp (proto, "TLS") != 0 && osip_strcasecmp (proto, "SCTP") != 0) {
    }
    else {                      /* reliable protocol is used: */
      ict->ict_context->timer_a_length = -1;    /* A is not ACTIVE */
      ict->ict_context->timer_a_start.tv_sec = -1;
    }
  }
#endif

  if (i == 0)
    __osip_message_callback (OSIP_ICT_INVITE_SENT_AGAIN, ict, ict->orig_request);
}

void
osip_ict_timeout_b_event (osip_transaction_t * ict, osip_event_t * evt)
{
  ict->ict_context->timer_b_length = -1;
  ict->ict_context->timer_b_start.tv_sec = -1;

  __osip_message_callback (OSIP_ICT_STATUS_TIMEOUT, ict, evt->sip);
  __osip_transaction_set_state (ict, ICT_TERMINATED);
  __osip_kill_transaction_callback (OSIP_ICT_KILL_TRANSACTION, ict);
}

void
ict_rcv_1xx (osip_transaction_t * ict, osip_event_t * evt)
{
  /* leave this answer to the core application */

  if (ict->last_response != NULL) {
    osip_message_free (ict->last_response);
  }
  ict->last_response = evt->sip;
  __osip_message_callback (OSIP_ICT_STATUS_1XX_RECEIVED, ict, evt->sip);
  __osip_transaction_set_state (ict, ICT_PROCEEDING);
}

void
ict_rcv_2xx (osip_transaction_t * ict, osip_event_t * evt)
{
  /* leave this answer to the core application */

  if (ict->last_response != NULL) {
    osip_message_free (ict->last_response);
  }
  ict->last_response = evt->sip;

  __osip_message_callback (OSIP_ICT_STATUS_2XX_RECEIVED, ict, evt->sip);

  __osip_transaction_set_state (ict, ICT_TERMINATED);
  __osip_kill_transaction_callback (OSIP_ICT_KILL_TRANSACTION, ict);
}

osip_message_t *
ict_create_ack (osip_transaction_t * ict, osip_message_t * response)
{
  int i;
  osip_message_t *ack;

  i = osip_message_init (&ack);
  if (i != 0)
    return NULL;

  /* Section 17.1.1.3: Construction of the ACK request: */
  i = osip_from_clone (response->from, &(ack->from));
  if (i != 0) {
    osip_message_free (ack);
    return NULL;
  }
  i = osip_to_clone (response->to, &(ack->to)); /* include the tag! */
  if (i != 0) {
    osip_message_free (ack);
    return NULL;
  }
  i = osip_call_id_clone (response->call_id, &(ack->call_id));
  if (i != 0) {
    osip_message_free (ack);
    return NULL;
  }
  i = osip_cseq_clone (response->cseq, &(ack->cseq));
  if (i != 0) {
    osip_message_free (ack);
    return NULL;
  }
  osip_free (ack->cseq->method);
  ack->cseq->method = osip_strdup ("ACK");
  if (ack->cseq->method == NULL) {
    osip_message_free (ack);
    return NULL;
  }

  ack->sip_method = (char *) osip_malloc (5);
  if (ack->sip_method == NULL) {
    osip_message_free (ack);
    return NULL;
  }
  sprintf (ack->sip_method, "ACK");
  ack->sip_version = osip_strdup (ict->orig_request->sip_version);
  if (ack->sip_version == NULL) {
    osip_message_free (ack);
    return NULL;
  }

  ack->status_code = 0;
  ack->reason_phrase = NULL;

  /* MUST copy REQUEST-URI from Contact header! */
  i = osip_uri_clone (ict->orig_request->req_uri, &(ack->req_uri));
  if (i != 0) {
    osip_message_free (ack);
    return NULL;
  }

  /* ACK MUST contain only the TOP Via field from original request */
  {
    osip_via_t *via;
    osip_via_t *orig_via;

    osip_message_get_via (ict->orig_request, 0, &orig_via);
    if (orig_via == NULL) {
      osip_message_free (ack);
      return NULL;
    }
    i = osip_via_clone (orig_via, &via);
    if (i != 0) {
      osip_message_free (ack);
      return NULL;
    }
    osip_list_add (&ack->vias, via, -1);
  }

  /* ack MUST contains the ROUTE headers field from the original request */
  /* IS IT TRUE??? */
  /* if the answers contains a set of route (or record route), then it */
  /* should be used instead?? ......May be not..... */
  {
    int pos = 0;
    osip_route_t *route;
    osip_route_t *orig_route;

    while (!osip_list_eol (&ict->orig_request->routes, pos)) {
      orig_route = (osip_route_t *) osip_list_get (&ict->orig_request->routes, pos);
      i = osip_route_clone (orig_route, &route);
      if (i != 0) {
        osip_message_free (ack);
        return NULL;
      }
      osip_list_add (&ack->routes, route, -1);
      pos++;
    }
  }

  if (response->status_code!=401 && response->status_code!=407) {
    /* ack MUST contains the Authorization headers field from the original request */
    if (osip_list_size (&ict->orig_request->authorizations) > 0) {
      i = osip_list_clone (&ict->orig_request->authorizations, &ack->authorizations, (int (*)(void *, void **)) &osip_authorization_clone);
      if (i != 0) {
        osip_message_free (ack);
        return NULL;
      }
    }

    /* ack MUST contains the Proxy-Authorization headers field from the original request */
    if (osip_list_size (&ict->orig_request->proxy_authorizations) > 0) {
      i = osip_list_clone (&ict->orig_request->proxy_authorizations, &ack->proxy_authorizations, (int (*)(void *, void **)) &osip_proxy_authorization_clone);
      if (i != 0) {
        osip_message_free (ack);
        return NULL;
      }
    }
  }

  /* may be we could add some other headers: */
  /* For example "Max-Forward" */

  return ack;
}

void
ict_rcv_3456xx (osip_transaction_t * ict, osip_event_t * evt)
{
  osip_route_t *route;
  int i;
  osip_t *osip = (osip_t *) ict->config;

  /* leave this answer to the core application */

  if (ict->last_response != NULL)
    osip_message_free (ict->last_response);

  ict->last_response = evt->sip;
  if (ict->state != ICT_COMPLETED) {    /* not a retransmission */
    /* automatic handling of ack! */
    osip_message_t *ack = ict_create_ack (ict, evt->sip);

    ict->ack = ack;

    if (ict->ack == NULL) {
      __osip_transaction_set_state (ict, ICT_TERMINATED);
      __osip_kill_transaction_callback (OSIP_ICT_KILL_TRANSACTION, ict);
      return;
    }

    /* reset ict->ict_context->destination only if
       it is not yet set. */
    if (ict->ict_context->destination == NULL) {
      osip_message_get_route (ack, 0, &route);
      if (route != NULL && route->url != NULL) {
        osip_uri_param_t *lr_param;

        osip_uri_uparam_get_byname (route->url, "lr", &lr_param);
        if (lr_param == NULL) {
          /* using uncompliant proxy: destination is the request-uri */
          route = NULL;
        }
      }

      if (route != NULL && route->url != NULL) {
        int port = 5060;

        if (route->url->port != NULL)
          port = osip_atoi (route->url->port);
        osip_ict_set_destination (ict->ict_context, osip_strdup (route->url->host), port);
      }
      else {
        int port = 5060;

        /* search for maddr parameter */
        osip_uri_param_t *maddr_param = NULL;
        osip_uri_param_t *obr_param = NULL;
        osip_uri_param_t *obp_param = NULL;

        port = 5060;
        if (ack->req_uri->port != NULL)
          port = osip_atoi (ack->req_uri->port);

        /* if ob was used in Contact, then exosip adds "x-obr" and "x-obp", thus, when
        processing request, the ip/port destination are re-used here */
        osip_uri_uparam_get_byname(ack->req_uri, "x-obr", &obr_param);
        osip_uri_uparam_get_byname(ack->req_uri, "x-obp", &obp_param);

        osip_uri_uparam_get_byname (ack->req_uri, "maddr", &maddr_param);
        if (maddr_param != NULL && maddr_param->gvalue != NULL)
          osip_ict_set_destination (ict->ict_context, osip_strdup (maddr_param->gvalue), port);
        else if (obr_param != NULL && obr_param->gvalue != NULL && obp_param != NULL && obp_param->gvalue != NULL)
          osip_ict_set_destination (ict->ict_context, osip_strdup (obr_param->gvalue), osip_atoi(obp_param->gvalue));
        else
          osip_ict_set_destination (ict->ict_context, osip_strdup (ack->req_uri->host), port);
      }
    }
    i = osip->cb_send_message (ict, ack, ict->ict_context->destination, ict->ict_context->port, ict->out_socket);
    if (i != 0) {
      ict_handle_transport_error (ict, i);
      return;
    }
    if (MSG_IS_STATUS_3XX (evt->sip))
      __osip_message_callback (OSIP_ICT_STATUS_3XX_RECEIVED, ict, evt->sip);
    else if (MSG_IS_STATUS_4XX (evt->sip))
      __osip_message_callback (OSIP_ICT_STATUS_4XX_RECEIVED, ict, evt->sip);
    else if (MSG_IS_STATUS_5XX (evt->sip))
      __osip_message_callback (OSIP_ICT_STATUS_5XX_RECEIVED, ict, evt->sip);
    else
      __osip_message_callback (OSIP_ICT_STATUS_6XX_RECEIVED, ict, evt->sip);

    __osip_message_callback (OSIP_ICT_ACK_SENT, ict, ack);
  }

  /* start timer D (length is set to MAX (64*DEFAULT_T1 or 32000) */
  osip_gettimeofday (&ict->ict_context->timer_d_start, NULL);
  add_gettimeofday (&ict->ict_context->timer_d_start, ict->ict_context->timer_d_length);
  __osip_transaction_set_state (ict, ICT_COMPLETED);
}

void
osip_ict_timeout_d_event (osip_transaction_t * ict, osip_event_t * evt)
{
  ict->ict_context->timer_d_length = -1;
  ict->ict_context->timer_d_start.tv_sec = -1;

  __osip_transaction_set_state (ict, ICT_TERMINATED);
  __osip_kill_transaction_callback (OSIP_ICT_KILL_TRANSACTION, ict);
}

void
ict_retransmit_ack (osip_transaction_t * ict, osip_event_t * evt)
{
  int i;
  osip_t *osip = (osip_t *) ict->config;

  /* this could be another 3456xx ??? */
  /* we should make a new ACK and send it!!! */
  /* TODO */

  __osip_message_callback (OSIP_ICT_STATUS_3456XX_RECEIVED_AGAIN, ict, evt->sip);

  osip_message_free (evt->sip);

  i = osip->cb_send_message (ict, ict->ack, ict->ict_context->destination, ict->ict_context->port, ict->out_socket);

  if (i == 0) {
    __osip_message_callback (OSIP_ICT_ACK_SENT_AGAIN, ict, ict->ack);
    __osip_transaction_set_state (ict, ICT_COMPLETED);
  }
  else {
    ict_handle_transport_error (ict, i);
  }
}
