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

int
__osip_ist_init (osip_ist_t ** ist, osip_t * osip, osip_message_t * invite)
{
  int i;

  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "allocating IST context\n"));

  *ist = (osip_ist_t *) osip_malloc (sizeof (osip_ist_t));
  if (*ist == NULL)
    return OSIP_NOMEM;
  memset (*ist, 0, sizeof (osip_ist_t));
  /* for INVITE retransmissions */
  {
    osip_via_t *via;
    char *proto;

    i = osip_message_get_via (invite, 0, &via); /* get top via */
    if (i < 0) {
      osip_free (*ist);
      *ist = NULL;
      return i;
    }
    proto = via_get_protocol (via);
    if (proto == NULL) {
      osip_free (*ist);
      *ist = NULL;
      return OSIP_UNDEFINED_ERROR;
    }

    if (osip_strcasecmp (proto, "TCP") != 0 && osip_strcasecmp (proto, "TLS") != 0 && osip_strcasecmp (proto, "SCTP") != 0) {   /* for other reliable protocol than TCP, the timer
                                                                                                                                   must be desactived by the external application */
      (*ist)->timer_g_length = DEFAULT_T1;
      (*ist)->timer_i_length = DEFAULT_T4;
      (*ist)->timer_g_start.tv_sec = -1;        /* not started */
      (*ist)->timer_i_start.tv_sec = -1;        /* not started */
    }
    else {                      /* reliable protocol is used: */
      (*ist)->timer_g_length = -1;      /* A is not ACTIVE */
      (*ist)->timer_i_length = 0;       /* MUST do the transition immediatly */
      (*ist)->timer_g_start.tv_sec = -1;        /* not started */
      (*ist)->timer_i_start.tv_sec = -1;        /* not started */
    }
  }

  (*ist)->timer_h_length = 64 * DEFAULT_T1;
  (*ist)->timer_h_start.tv_sec = -1;    /* not started */

  return OSIP_SUCCESS;
}

int
__osip_ist_free (osip_ist_t * ist)
{
  if (ist == NULL)
    return OSIP_SUCCESS;
  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "free ist resource\n"));
  osip_free (ist);
  return OSIP_SUCCESS;
}

osip_event_t *
__osip_ist_need_timer_g_event (osip_ist_t * ist, state_t state, int transactionid)
{
  return __osip_transaction_need_timer_x_event (ist, &ist->timer_g_start, state == IST_COMPLETED, transactionid, TIMEOUT_G);
}

osip_event_t *
__osip_ist_need_timer_h_event (osip_ist_t * ist, state_t state, int transactionid)
{
  return __osip_transaction_need_timer_x_event (ist, &ist->timer_h_start, state == IST_COMPLETED, transactionid, TIMEOUT_H);
}

osip_event_t *
__osip_ist_need_timer_i_event (osip_ist_t * ist, state_t state, int transactionid)
{
  return __osip_transaction_need_timer_x_event (ist, &ist->timer_i_start, state == IST_CONFIRMED, transactionid, TIMEOUT_I);
}
