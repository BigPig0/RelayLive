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

#include "eXosip2.h"

#ifndef MINISIZE

osip_transaction_t *
_eXosip_find_last_out_subscribe (eXosip_subscribe_t * js, eXosip_dialog_t * jd)
{
  osip_transaction_t *out_tr;

  out_tr = NULL;
  if (jd != NULL) {
    osip_list_iterator_t it;
    out_tr = (osip_transaction_t*)osip_list_get_first(jd->d_out_trs, &it);
    while (out_tr != OSIP_SUCCESS) {
      if (0 == strcmp (out_tr->cseq->method, "SUBSCRIBE"))
        break;
      else if (0 == strcmp (out_tr->cseq->method, "REFER"))
        break;
      out_tr = (osip_transaction_t *)osip_list_get_next(&it);
    }
  }

  if (out_tr == NULL)
    return js->s_out_tr;        /* can be NULL */

  return out_tr;
}

osip_transaction_t *
_eXosip_find_last_inc_notify (eXosip_subscribe_t * js, eXosip_dialog_t * jd)
{
  osip_transaction_t *out_tr;

  out_tr = NULL;
  if (jd != NULL) {
    osip_list_iterator_t it;
    out_tr = (osip_transaction_t*)osip_list_get_first(jd->d_out_trs, &it);
    while (out_tr != OSIP_SUCCESS) {
      if (0 == strcmp (out_tr->cseq->method, "NOTIFY"))
        return out_tr;
      out_tr = (osip_transaction_t *)osip_list_get_next(&it);
    }
  }

  return NULL;
}

int
_eXosip_subscription_init (struct eXosip_t *excontext, eXosip_subscribe_t ** js)
{
  *js = (eXosip_subscribe_t *) osip_malloc (sizeof (eXosip_subscribe_t));
  if (*js == NULL)
    return OSIP_NOMEM;
  memset (*js, 0, sizeof (eXosip_subscribe_t));

#ifndef MINISIZE
  {
    struct timeval now;
    excontext->statistics.allocated_subscriptions++;
    osip_gettimeofday(&now, NULL);
    _eXosip_counters_update(&excontext->average_subscriptions, 1, &now);
  }
#endif
  return OSIP_SUCCESS;
}

void
_eXosip_subscription_free (struct eXosip_t *excontext, eXosip_subscribe_t * js)
{
  eXosip_dialog_t *jd;

  if (js->s_inc_tr != NULL && js->s_inc_tr->orig_request != NULL && js->s_inc_tr->orig_request->call_id != NULL && js->s_inc_tr->orig_request->call_id->number != NULL)
    _eXosip_delete_nonce (excontext, js->s_inc_tr->orig_request->call_id->number);
  else if (js->s_out_tr != NULL && js->s_out_tr->orig_request != NULL && js->s_out_tr->orig_request->call_id != NULL && js->s_out_tr->orig_request->call_id->number != NULL)
    _eXosip_delete_nonce (excontext, js->s_out_tr->orig_request->call_id->number);

  for (jd = js->s_dialogs; jd != NULL; jd = js->s_dialogs) {
    REMOVE_ELEMENT (js->s_dialogs, jd);
    _eXosip_dialog_free (excontext, jd);
  }

  _eXosip_delete_reserved (js->s_inc_tr);
  _eXosip_delete_reserved (js->s_out_tr);
  if (js->s_inc_tr != NULL)
    osip_list_add (&excontext->j_transactions, js->s_inc_tr, 0);
  if (js->s_out_tr != NULL)
    osip_list_add (&excontext->j_transactions, js->s_out_tr, 0);

  osip_free (js);
#ifndef MINISIZE
  excontext->statistics.allocated_subscriptions--;
#endif
}

int
_eXosip_subscription_set_refresh_interval (eXosip_subscribe_t * js, osip_message_t * out_subscribe)
{
  osip_header_t *exp;

  if (js == NULL || out_subscribe == NULL)
    return OSIP_BADPARAMETER;

  osip_message_get_expires (out_subscribe, 0, &exp);
  if (exp != NULL && exp->hvalue != NULL) {
    int val = osip_atoi (exp->hvalue);

    if (val == 0)
      js->s_reg_period = 0;
    else if (val < js->s_reg_period - 15)
      js->s_reg_period = val;
  }

  return OSIP_SUCCESS;
}

#endif
