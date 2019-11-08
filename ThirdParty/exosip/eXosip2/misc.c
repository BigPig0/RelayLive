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

/* some methods to extract transaction information from a eXosip_call_t */

int
_eXosip_remove_transaction_from_call (osip_transaction_t * tr, eXosip_call_t * jc)
{
  osip_transaction_t *inc_tr;
  osip_transaction_t *out_tr;
  eXosip_dialog_t *jd;

  if (jc->c_inc_tr == tr) {
    jc->c_inc_tr = NULL;        /* can be NULL */
    return OSIP_SUCCESS;
  }

  for (jd = jc->c_dialogs; jd != NULL; jd = jd->next) {
    osip_list_iterator_t it;
    inc_tr = (osip_transaction_t*)osip_list_get_first(jd->d_inc_trs, &it);
    while (inc_tr != OSIP_SUCCESS) {
      if (inc_tr == tr) {
        osip_list_iterator_remove(&it);
        return OSIP_SUCCESS;
      }
      inc_tr = (osip_transaction_t *)osip_list_get_next(&it);
    }
  }

  if (jc->c_out_tr == tr) {
    jc->c_out_tr = NULL;        /* can be NULL */
    return OSIP_SUCCESS;
  }

  for (jd = jc->c_dialogs; jd != NULL; jd = jd->next) {
    osip_list_iterator_t it;
    out_tr = (osip_transaction_t*)osip_list_get_first(jd->d_out_trs, &it);
    while (out_tr != OSIP_SUCCESS) {
      if (out_tr == tr) {
        osip_list_iterator_remove(&it);
        return OSIP_SUCCESS;
      }
      out_tr = (osip_transaction_t *)osip_list_get_next(&it);
    }
  }

  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "eXosip: No information.\n"));
  return OSIP_NOTFOUND;
}

osip_transaction_t *
_eXosip_find_last_transaction (eXosip_call_t * jc, eXosip_dialog_t * jd, const char *method)
{
  osip_transaction_t *inc_tr;
  osip_transaction_t *out_tr;

  inc_tr = _eXosip_find_last_inc_transaction (jc, jd, method);
  out_tr = _eXosip_find_last_out_transaction (jc, jd, method);
  if (inc_tr == NULL)
    return out_tr;
  if (out_tr == NULL)
    return inc_tr;

  if (inc_tr->birth_time > out_tr->birth_time)
    return inc_tr;
  return out_tr;
}

osip_transaction_t *
_eXosip_find_last_inc_transaction (eXosip_call_t * jc, eXosip_dialog_t * jd, const char *method)
{
  osip_transaction_t *inc_tr;

  inc_tr = NULL;
  if (method == NULL || method[0] == '\0')
    return NULL;
  if (jd != NULL) {
    osip_list_iterator_t it;
    inc_tr = (osip_transaction_t*)osip_list_get_first(jd->d_inc_trs, &it);
    while (inc_tr != OSIP_SUCCESS) {
      if (0 == osip_strcasecmp (inc_tr->cseq->method, method))
        break;
      
      inc_tr = (osip_transaction_t *)osip_list_get_next(&it);
    }
  }

  return inc_tr;
}

osip_transaction_t *
_eXosip_find_last_out_transaction (eXosip_call_t * jc, eXosip_dialog_t * jd, const char *method)
{
  osip_transaction_t *out_tr;

  out_tr = NULL;
  if (jd == NULL && jc == NULL)
    return NULL;
  if (method == NULL || method[0] == '\0')
    return NULL;

  if (jd != NULL) {
    osip_list_iterator_t it;
    out_tr = (osip_transaction_t*)osip_list_get_first(jd->d_out_trs, &it);
    while (out_tr != OSIP_SUCCESS) {
      if (0 == osip_strcasecmp (out_tr->cseq->method, method))
        break;
      out_tr = (osip_transaction_t *)osip_list_get_next(&it);
    }
  }

  return out_tr;
}

osip_transaction_t *
_eXosip_find_last_invite (eXosip_call_t * jc, eXosip_dialog_t * jd)
{
  osip_transaction_t *inc_tr;
  osip_transaction_t *out_tr;

  inc_tr = _eXosip_find_last_inc_invite (jc, jd);
  out_tr = _eXosip_find_last_out_invite (jc, jd);
  if (inc_tr == NULL)
    return out_tr;
  if (out_tr == NULL)
    return inc_tr;

  if (inc_tr->birth_time > out_tr->birth_time)
    return inc_tr;
  return out_tr;
}

osip_transaction_t *
_eXosip_find_last_inc_invite (eXosip_call_t * jc, eXosip_dialog_t * jd)
{
  osip_transaction_t *inc_tr;

  inc_tr = NULL;
  if (jd != NULL) {
    osip_list_iterator_t it;
    inc_tr = (osip_transaction_t*)osip_list_get_first(jd->d_inc_trs, &it);
    while (inc_tr != OSIP_SUCCESS) {
      if (0 == strcmp (inc_tr->cseq->method, "INVITE"))
        break;
      inc_tr = (osip_transaction_t *)osip_list_get_next(&it);
    }
  }

  if (inc_tr == NULL)
    return jc->c_inc_tr;        /* can be NULL */

  return inc_tr;
}

osip_transaction_t *
_eXosip_find_last_out_invite (eXosip_call_t * jc, eXosip_dialog_t * jd)
{
  osip_transaction_t *out_tr;

  out_tr = NULL;
  if (jd == NULL && jc == NULL)
    return NULL;

  if (jd != NULL) {
    osip_list_iterator_t it;
    out_tr = (osip_transaction_t*)osip_list_get_first(jd->d_out_trs, &it);
    while (out_tr != OSIP_SUCCESS) {
      if (0 == strcmp (out_tr->cseq->method, "INVITE"))
        break;
      out_tr = (osip_transaction_t *)osip_list_get_next(&it);
    }
  }

  if (out_tr == NULL)
    return jc->c_out_tr;        /* can be NULL */

  return out_tr;
}

#ifndef MINISIZE

osip_transaction_t *
_eXosip_find_previous_invite (eXosip_call_t * jc, eXosip_dialog_t * jd, osip_transaction_t * last_invite)
{
  osip_transaction_t *inc_tr;
  osip_transaction_t *out_tr;

  inc_tr = NULL;
  if (jd != NULL) {
    osip_list_iterator_t it;
    inc_tr = (osip_transaction_t*)osip_list_get_first(jd->d_inc_trs, &it);
    while (inc_tr != OSIP_SUCCESS) {
      if (inc_tr != last_invite) {
        /* we don't want the current one */
        if (0 == strcmp (inc_tr->cseq->method, "INVITE"))
          break;
      }
      
      inc_tr = (osip_transaction_t *)osip_list_get_next(&it);
    }
  }

  if (inc_tr == NULL)
    inc_tr = jc->c_inc_tr;      /* can be NULL */
  if (inc_tr == last_invite) {
    /* we don't want the current one */
    inc_tr = NULL;
  }

  out_tr = NULL;

  if (jd != NULL) {
    osip_list_iterator_t it;
    out_tr = (osip_transaction_t*)osip_list_get_first(jd->d_out_trs, &it);
    while (out_tr != OSIP_SUCCESS) {
      if (out_tr == last_invite) {
        /* we don't want the current one */
        out_tr = NULL;
      }
      else if (0 == strcmp (out_tr->cseq->method, "INVITE"))
        break;
      
      out_tr = (osip_transaction_t *)osip_list_get_next(&it);
    }
  }

  if (out_tr == NULL)
    out_tr = jc->c_out_tr;      /* can be NULL */

  if (out_tr == last_invite) {
    /* we don't want the current one */
    out_tr = NULL;
  }

  if (inc_tr == NULL)
    return out_tr;
  if (out_tr == NULL)
    return inc_tr;

  if (inc_tr->birth_time > out_tr->birth_time)
    return inc_tr;
  return out_tr;
}

#endif
