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

int
__osip_ict_init (osip_ict_t ** ict, osip_t * osip, osip_message_t * invite)
{
  osip_route_t *route;
  int i;

  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "allocating ICT context\n"));

  *ict = (osip_ict_t *) osip_malloc (sizeof (osip_ict_t));
  if (*ict == NULL)
    return OSIP_NOMEM;

  memset (*ict, 0, sizeof (osip_ict_t));
  /* for INVITE retransmissions */
  {
    osip_via_t *via;
    char *proto;

    i = osip_message_get_via (invite, 0, &via); /* get top via */
    if (i < 0) {
      osip_free (*ict);
      return i;
    }
    proto = via_get_protocol (via);
    if (proto == NULL) {
      osip_free (*ict);
      return OSIP_SYNTAXERROR;
    }
#ifdef USE_BLOCKINGSOCKET
    if (osip_strcasecmp (proto, "TCP") != 0 && osip_strcasecmp (proto, "TLS") != 0 && osip_strcasecmp (proto, "SCTP") != 0) {   /* for other reliable protocol than TCP, the timer
                                                                                                                                   must be desactived by the external application */
      (*ict)->timer_a_length = DEFAULT_T1;
      if (64 * DEFAULT_T1 < 32000)
        (*ict)->timer_d_length = 32000;
      else
        (*ict)->timer_d_length = 64 * DEFAULT_T1;
      osip_gettimeofday (&(*ict)->timer_a_start, NULL);
      add_gettimeofday (&(*ict)->timer_a_start, (*ict)->timer_a_length);
      (*ict)->timer_d_start.tv_sec = -1;        /* not started */
    }
    else {                      /* reliable protocol is used: */
      (*ict)->timer_a_length = -1;      /* A is not ACTIVE */
      (*ict)->timer_d_length = 0;       /* MUST do the transition immediatly */
      (*ict)->timer_a_start.tv_sec = -1;        /* not started */
      (*ict)->timer_d_start.tv_sec = -1;        /* not started */
    }
  }
#else
    if (osip_strcasecmp (proto, "TCP") != 0 && osip_strcasecmp (proto, "TLS") != 0 && osip_strcasecmp (proto, "SCTP") != 0) {   /* for other reliable protocol than TCP, the timer
                                                                                                                                   must be desactived by the external application */
      (*ict)->timer_a_length = DEFAULT_T1;
      if (64 * DEFAULT_T1 < 32000)
        (*ict)->timer_d_length = 32000;
      else
        (*ict)->timer_d_length = 64 * DEFAULT_T1;
      osip_gettimeofday (&(*ict)->timer_a_start, NULL);
      add_gettimeofday (&(*ict)->timer_a_start, (*ict)->timer_a_length);
      (*ict)->timer_d_start.tv_sec = -1;        /* not started */
    }
    else {                      /* reliable protocol is used: */
      (*ict)->timer_a_length = DEFAULT_T1;
      (*ict)->timer_d_length = 0;       /* MUST do the transition immediatly */
      osip_gettimeofday (&(*ict)->timer_a_start, NULL);
      add_gettimeofday (&(*ict)->timer_a_start, (*ict)->timer_a_length);
      (*ict)->timer_d_start.tv_sec = -1;        /* not started */
    }
  }
#endif

  /* for PROXY, the destination MUST be set by the application layer,
     this one may not be correct. */
  osip_message_get_route (invite, 0, &route);
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
    osip_ict_set_destination ((*ict), osip_strdup (route->url->host), port);
  }
  else {
    int port = 5060;

    /* search for maddr parameter */
    osip_uri_param_t *maddr_param = NULL;
    osip_uri_param_t *obr_param = NULL;
    osip_uri_param_t *obp_param = NULL;

    port = 5060;
    if (invite->req_uri->port != NULL)
      port = osip_atoi (invite->req_uri->port);

    /* if ob was used in Contact, then exosip adds "x-obr" and "x-obp", thus, when
    processing request, the ip/port destination are re-used here */
    osip_uri_uparam_get_byname(invite->req_uri, "x-obr", &obr_param);
    osip_uri_uparam_get_byname(invite->req_uri, "x-obp", &obp_param);

    osip_uri_uparam_get_byname (invite->req_uri, "maddr", &maddr_param);
    if (maddr_param != NULL && maddr_param->gvalue != NULL)
      osip_ict_set_destination ((*ict), osip_strdup (maddr_param->gvalue), port);
    else if (obr_param != NULL && obr_param->gvalue != NULL && obp_param != NULL && obp_param->gvalue != NULL)
      osip_ict_set_destination ((*ict), osip_strdup (obr_param->gvalue), osip_atoi(obp_param->gvalue));
    else
      osip_ict_set_destination ((*ict), osip_strdup (invite->req_uri->host), port);
  }

  (*ict)->timer_b_length = 64 * DEFAULT_T1;
  osip_gettimeofday (&(*ict)->timer_b_start, NULL);
  add_gettimeofday (&(*ict)->timer_b_start, (*ict)->timer_b_length);

  /* Oups! A bug! */
  /*  (*ict)->port  = 5060; */

  return OSIP_SUCCESS;
}

int
__osip_ict_free (osip_ict_t * ict)
{
  if (ict == NULL)
    return OSIP_SUCCESS;
  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "free ict resource\n"));

  osip_free (ict->destination);
  osip_free (ict);
  return OSIP_SUCCESS;
}

int
osip_ict_set_destination (osip_ict_t * ict, char *destination, int port)
{
  if (ict == NULL)
    return OSIP_BADPARAMETER;
  if (ict->destination != NULL)
    osip_free (ict->destination);
  ict->destination = destination;
  ict->port = port;
  return OSIP_SUCCESS;
}

osip_event_t *
__osip_ict_need_timer_a_event (osip_ict_t * ict, state_t state, int transactionid)
{
  return __osip_transaction_need_timer_x_event (ict, &ict->timer_a_start, state == ICT_CALLING, transactionid, TIMEOUT_A);
}

osip_event_t *
__osip_ict_need_timer_b_event (osip_ict_t * ict, state_t state, int transactionid)
{
  return __osip_transaction_need_timer_x_event (ict, &ict->timer_b_start, state == ICT_CALLING, transactionid, TIMEOUT_B);
}

osip_event_t *
__osip_ict_need_timer_d_event (osip_ict_t * ict, state_t state, int transactionid)
{
  return __osip_transaction_need_timer_x_event (ict, &ict->timer_d_start, state == ICT_COMPLETED, transactionid, TIMEOUT_D);
}
