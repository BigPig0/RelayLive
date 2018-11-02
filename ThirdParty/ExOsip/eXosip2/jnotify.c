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
_eXosip_find_last_inc_subscribe (eXosip_notify_t * jn, eXosip_dialog_t * jd)
{
  osip_transaction_t *inc_tr;

  inc_tr = NULL;
  if (jd != NULL) {
    osip_list_iterator_t it;
    inc_tr = (osip_transaction_t *)osip_list_get_first(jd->d_inc_trs, &it);
    while (inc_tr != NULL) {
      if (0 == strcmp (inc_tr->cseq->method, "SUBSCRIBE"))
        break;
      else if (0 == strcmp (inc_tr->cseq->method, "REFER"))
        break;
      
      inc_tr = (osip_transaction_t *)osip_list_get_next(&it);
    }
  }

  if (inc_tr == NULL)
    return jn->n_inc_tr;        /* can be NULL */

  return inc_tr;
}


osip_transaction_t *
_eXosip_find_last_out_notify (eXosip_notify_t * jn, eXosip_dialog_t * jd)
{
  osip_transaction_t *out_tr;

  out_tr = NULL;
  if (jd != NULL) {
    osip_list_iterator_t it;
    out_tr = (osip_transaction_t *)osip_list_get_first(jd->d_out_trs, &it);
    while (out_tr != OSIP_SUCCESS) {
      if (0 == strcmp (out_tr->cseq->method, "NOTIFY"))
        return out_tr;
      out_tr = (osip_transaction_t *)osip_list_get_next(&it);
    }
  }

  return NULL;
}

int
_eXosip_notify_init (struct eXosip_t *excontext, eXosip_notify_t ** jn, osip_message_t * inc_subscribe)
{
  osip_contact_t *co;

  *jn = NULL;
  co = (osip_contact_t *) osip_list_get (&inc_subscribe->contacts, 0);
  if (co == NULL || co->url == NULL)
    return -1;

  *jn = (eXosip_notify_t *) osip_malloc (sizeof (eXosip_notify_t));
  if (*jn == NULL)
    return OSIP_NOMEM;
  memset (*jn, 0, sizeof (eXosip_notify_t));

#ifndef MINISIZE
  {
    struct timeval now;
    excontext->statistics.allocated_insubscriptions++;
    osip_gettimeofday(&now, NULL);
    _eXosip_counters_update(&excontext->average_insubscriptions, 1, &now);
  }
#endif
  return OSIP_SUCCESS;
}

void
_eXosip_notify_free (struct eXosip_t *excontext, eXosip_notify_t * jn)
{
  /* ... */

  eXosip_dialog_t *jd;

  if (jn->n_inc_tr != NULL && jn->n_inc_tr->orig_request != NULL && jn->n_inc_tr->orig_request->call_id != NULL && jn->n_inc_tr->orig_request->call_id->number != NULL)
    _eXosip_delete_nonce (excontext, jn->n_inc_tr->orig_request->call_id->number);
  else if (jn->n_out_tr != NULL && jn->n_out_tr->orig_request != NULL && jn->n_out_tr->orig_request->call_id != NULL && jn->n_out_tr->orig_request->call_id->number != NULL)
    _eXosip_delete_nonce (excontext, jn->n_out_tr->orig_request->call_id->number);

  for (jd = jn->n_dialogs; jd != NULL; jd = jn->n_dialogs) {
    REMOVE_ELEMENT (jn->n_dialogs, jd);
    _eXosip_dialog_free (excontext, jd);
  }

  _eXosip_delete_reserved (jn->n_inc_tr);
  _eXosip_delete_reserved (jn->n_out_tr);
  if (jn->n_inc_tr != NULL)
    osip_list_add (&excontext->j_transactions, jn->n_inc_tr, 0);
  if (jn->n_out_tr != NULL)
    osip_list_add (&excontext->j_transactions, jn->n_out_tr, 0);
  osip_free (jn);

#ifndef MINISIZE
  excontext->statistics.allocated_insubscriptions--;
#endif
}

int
_eXosip_notify_set_refresh_interval (eXosip_notify_t * jn, osip_message_t * inc_subscribe)
{
  osip_header_t *exp;
  time_t now;
  int default_expires=600;

  now = osip_getsystemtime (NULL);
  if (jn == NULL || inc_subscribe == NULL)
    return -1;
  if (MSG_IS_REFER(inc_subscribe))
    default_expires=120;
  osip_message_get_expires (inc_subscribe, 0, &exp);
  if (exp == NULL || exp->hvalue == NULL)
    jn->n_ss_expires = now + default_expires;
  else {
    jn->n_ss_expires = osip_atoi (exp->hvalue);
    if (jn->n_ss_expires != -1)
      jn->n_ss_expires = now + jn->n_ss_expires;
    else                        /* on error, set it to default */
      jn->n_ss_expires = now + default_expires;
  }

  return OSIP_SUCCESS;
}

void
_eXosip_notify_add_expires_in_2XX_for_subscribe (eXosip_notify_t * jn, osip_message_t * answer)
{
  char tmp[20];
  time_t now;

  now = osip_getsystemtime (NULL);

  if (jn->n_ss_expires - now < 0) {
    tmp[0] = '0';
    tmp[1] = '\0';
  }
  else {
    snprintf (tmp, 20, "%li", jn->n_ss_expires - now);
  }
  osip_message_set_expires (answer, tmp);
}

#endif
