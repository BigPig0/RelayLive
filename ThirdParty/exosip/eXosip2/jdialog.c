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

int
_eXosip_call_dialog_find (struct eXosip_t *excontext, int jid, eXosip_call_t ** jc, eXosip_dialog_t ** jd)
{
  if (jid <= 0)
    return OSIP_BADPARAMETER;

  for (*jc = excontext->j_calls; *jc != NULL; *jc = (*jc)->next) {
    for (*jd = (*jc)->c_dialogs; *jd != NULL; *jd = (*jd)->next) {
      if ((*jd)->d_id == jid)
        return OSIP_SUCCESS;
    }
  }
  *jd = NULL;
  *jc = NULL;
  return OSIP_NOTFOUND;
}

#ifndef MINISIZE

int
_eXosip_notify_dialog_find (struct eXosip_t *excontext, int nid, eXosip_notify_t ** jn, eXosip_dialog_t ** jd)
{
  if (nid <= 0)
    return OSIP_BADPARAMETER;
  for (*jn = excontext->j_notifies; *jn != NULL; *jn = (*jn)->next) {
    for (*jd = (*jn)->n_dialogs; *jd != NULL; *jd = (*jd)->next) {
      if ((*jd)->d_id == nid)
        return OSIP_SUCCESS;
    }
  }
  *jd = NULL;
  *jn = NULL;
  return OSIP_NOTFOUND;
}

int
_eXosip_subscription_dialog_find (struct eXosip_t *excontext, int sid, eXosip_subscribe_t ** js, eXosip_dialog_t ** jd)
{
  if (sid <= 0)
    return OSIP_BADPARAMETER;
  for (*js = excontext->j_subscribes; *js != NULL; *js = (*js)->next) {
    *jd = NULL;
    if ((*js)->s_id == sid)
      return OSIP_SUCCESS;
    for (*jd = (*js)->s_dialogs; *jd != NULL; *jd = (*jd)->next) {
      if ((*jd)->d_id == sid)
        return OSIP_SUCCESS;
    }
  }
  *jd = NULL;
  *js = NULL;
  return OSIP_NOTFOUND;
}

#endif

int
_eXosip_dialog_set_200ok (eXosip_dialog_t * jd, osip_message_t * _200Ok)
{
  int i;

  if (jd == NULL)
    return OSIP_BADPARAMETER;
  if (jd->d_200Ok != NULL)
    osip_message_free (jd->d_200Ok);
  jd->d_timer = osip_getsystemtime (NULL) + 1;
  jd->d_count = 0;
  i = osip_message_clone (_200Ok, &(jd->d_200Ok));
  if (i != 0)
    return i;
  return OSIP_SUCCESS;
}

int
_eXosip_dialog_init_as_uac (eXosip_dialog_t ** _jd, osip_message_t * _200Ok)
{
  int i;
  eXosip_dialog_t *jd;

  *_jd = NULL;
  jd = (eXosip_dialog_t *) osip_malloc (sizeof (eXosip_dialog_t));
  if (jd == NULL)
    return OSIP_NOMEM;
  memset (jd, 0, sizeof (eXosip_dialog_t));

  jd->d_id = -1;                /* not yet available to user */

  if (MSG_IS_REQUEST (_200Ok)) {
    i = osip_dialog_init_as_uac_with_remote_request (&(jd->d_dialog), _200Ok, -1);
  }
  else {                        /* normal usage with response */
    i = osip_dialog_init_as_uac (&(jd->d_dialog), _200Ok);
  }
  if (i != 0) {
    osip_free (jd);
    return i;
  }

  jd->d_count = 0;
  jd->d_session_timer_start = 0;
  jd->d_session_timer_length = 0;
  jd->d_refresher = -1;         /* 0 -> me / 1 -> remote */
  jd->d_timer = osip_getsystemtime (NULL);
  jd->d_200Ok = NULL;
  jd->d_ack = NULL;
  jd->next = NULL;
  jd->parent = NULL;
  jd->d_out_trs = (osip_list_t *) osip_malloc (sizeof (osip_list_t));
  if (jd->d_out_trs == NULL) {
    osip_dialog_free (jd->d_dialog);
    osip_free (jd);
    return OSIP_NOMEM;
  }
  osip_list_init (jd->d_out_trs);
  jd->d_inc_trs = (osip_list_t *) osip_malloc (sizeof (osip_list_t));
  if (jd->d_inc_trs == NULL) {
    osip_dialog_free (jd->d_dialog);
    osip_free (jd->d_out_trs);
    osip_free (jd);
    return OSIP_NOMEM;
  }
  osip_list_init (jd->d_inc_trs);

  *_jd = jd;
  return OSIP_SUCCESS;
}

int
_eXosip_dialog_init_as_uas (eXosip_dialog_t ** _jd, osip_message_t * _invite, osip_message_t * _200Ok)
{
  int i;
  eXosip_dialog_t *jd;

  *_jd = NULL;
  jd = (eXosip_dialog_t *) osip_malloc (sizeof (eXosip_dialog_t));
  if (jd == NULL)
    return OSIP_NOMEM;
  memset (jd, 0, sizeof (eXosip_dialog_t));
  jd->d_id = -1;                /* not yet available to user */
  i = osip_dialog_init_as_uas (&(jd->d_dialog), _invite, _200Ok);
  if (i != 0) {
    osip_free (jd);
    return i;
  }

  jd->d_count = 0;
  jd->d_session_timer_start = 0;
  jd->d_session_timer_length = 0;
  jd->d_session_timer_use_update = -1;
  jd->d_refresher = -1;         /* 0 -> me / 1 -> remote */
  jd->d_timer = osip_getsystemtime (NULL);
  jd->d_200Ok = NULL;
  jd->d_ack = NULL;
  jd->next = NULL;
  jd->parent = NULL;
  jd->d_out_trs = (osip_list_t *) osip_malloc (sizeof (osip_list_t));
  if (jd->d_out_trs == NULL) {
    osip_dialog_free (jd->d_dialog);
    osip_free (jd);
    return OSIP_NOMEM;
  }
  osip_list_init (jd->d_out_trs);
  jd->d_inc_trs = (osip_list_t *) osip_malloc (sizeof (osip_list_t));
  if (jd->d_inc_trs == NULL) {
    osip_dialog_free (jd->d_dialog);
    osip_free (jd->d_out_trs);
    osip_free (jd);
    return OSIP_NOMEM;
  }
  osip_list_init (jd->d_inc_trs);

  jd->d_dialog->local_cseq = 1;

  *_jd = jd;
  return OSIP_SUCCESS;
}

void
_eXosip_dialog_free (struct eXosip_t *excontext, eXosip_dialog_t * jd)
{
  while (!osip_list_eol (jd->d_inc_trs, 0)) {
    osip_transaction_t *tr;

    tr = (osip_transaction_t *) osip_list_get (jd->d_inc_trs, 0);
    osip_list_remove (jd->d_inc_trs, 0);
    _eXosip_delete_reserved (tr);
    osip_list_add (&excontext->j_transactions, tr, 0);
  }

  while (!osip_list_eol (jd->d_out_trs, 0)) {
    osip_transaction_t *tr;

    tr = (osip_transaction_t *) osip_list_get (jd->d_out_trs, 0);
    osip_list_remove (jd->d_out_trs, 0);
    _eXosip_delete_reserved (tr);
    osip_list_add (&excontext->j_transactions, tr, 0);
  }

  osip_message_free (jd->d_200Ok);
  osip_message_free (jd->d_ack);

  osip_dialog_free (jd->d_dialog);

  osip_free (jd->d_out_trs);
  osip_free (jd->d_inc_trs);
  osip_free (jd);

  _eXosip_update (excontext);
}
