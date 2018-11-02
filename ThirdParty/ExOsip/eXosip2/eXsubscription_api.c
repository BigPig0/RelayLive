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

int
_eXosip_subscription_transaction_find (struct eXosip_t *excontext, int tid, eXosip_subscribe_t ** js, eXosip_dialog_t ** jd, osip_transaction_t ** tr)
{
  for (*js = excontext->j_subscribes; *js != NULL; *js = (*js)->next) {
    if ((*js)->s_inc_tr != NULL && (*js)->s_inc_tr->transactionid == tid) {
      *tr = (*js)->s_inc_tr;
      *jd = (*js)->s_dialogs;
      return OSIP_SUCCESS;
    }
    if ((*js)->s_out_tr != NULL && (*js)->s_out_tr->transactionid == tid) {
      *tr = (*js)->s_out_tr;
      *jd = (*js)->s_dialogs;
      return OSIP_SUCCESS;
    }
    for (*jd = (*js)->s_dialogs; *jd != NULL; *jd = (*jd)->next) {
      osip_list_iterator_t it;
      osip_transaction_t* transaction;

      transaction = (osip_transaction_t*)osip_list_get_first((*jd)->d_inc_trs, &it);
      while (transaction != OSIP_SUCCESS) {
        if (transaction != NULL && transaction->transactionid == tid) {
          *tr = transaction;
          return OSIP_SUCCESS;
        }
        transaction = (osip_transaction_t *)osip_list_get_next(&it);
      }

      transaction = (osip_transaction_t*)osip_list_get_first((*jd)->d_out_trs, &it);
      while (transaction != OSIP_SUCCESS) {
        if (transaction != NULL && transaction->transactionid == tid) {
          *tr = transaction;
          return OSIP_SUCCESS;
        }
        transaction = (osip_transaction_t *)osip_list_get_next(&it);
      }
    }
  }
  *jd = NULL;
  *js = NULL;
  return OSIP_NOTFOUND;
}

int
eXosip_subscription_remove (struct eXosip_t *excontext, int did)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_subscribe_t *js = NULL;

  if (did <= 0)
    return OSIP_BADPARAMETER;

  if (did > 0) {
    _eXosip_subscription_dialog_find (excontext, did, &js, &jd);
  }
  if (js == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No outgoing subscription here?\n"));
    return OSIP_NOTFOUND;
  }
  REMOVE_ELEMENT (excontext->j_subscribes, js);
  _eXosip_subscription_free (excontext, js);
  return OSIP_SUCCESS;
}

int
eXosip_subscription_build_initial_subscribe (struct eXosip_t *excontext, osip_message_t ** sub, const char *to, const char *from, const char *route, const char *_event, int expires)
{
  char tmp[10];
  int i;
  osip_to_t *_to = NULL;

  *sub = NULL;
  if (to == NULL || *to == '\0')
    return OSIP_BADPARAMETER;
  if (from == NULL || *from == '\0')
    return OSIP_BADPARAMETER;
  if (_event == NULL || *_event == '\0')
    return OSIP_BADPARAMETER;
  if (route == NULL || *route == '\0')
    route = NULL;

  i = osip_to_init (&_to);
  if (i != 0)
    return i;

  i = osip_to_parse (_to, to);
  if (i != 0) {
    osip_to_free (_to);
    return i;
  }

  i = _eXosip_generating_request_out_of_dialog (excontext, sub, "SUBSCRIBE", to, from, route);
  osip_to_free (_to);
  if (i != 0)
    return i;
  _eXosip_dialog_add_contact (excontext, *sub);

  snprintf (tmp, 10, "%i", expires);
  osip_message_set_expires (*sub, tmp);

  osip_message_set_header (*sub, "Event", _event);

  return OSIP_SUCCESS;
}

int eXosip_subscription_build_initial_refer (struct eXosip_t *excontext, osip_message_t ** refer, const char *to, const char *from, const char *route, const char *refer_to)
{
  int i;
  osip_to_t *_to = NULL;

  *refer = NULL;
  if (to == NULL || *to == '\0')
    return OSIP_BADPARAMETER;
  if (from == NULL || *from == '\0')
    return OSIP_BADPARAMETER;
  if (refer_to == NULL || *refer_to == '\0')
    return OSIP_BADPARAMETER;
  if (route == NULL || *route == '\0')
    route = NULL;

  i = osip_to_init (&_to);
  if (i != 0)
    return i;

  i = osip_to_parse (_to, to);
  if (i != 0) {
    osip_to_free (_to);
    return i;
  }

  i = _eXosip_generating_request_out_of_dialog (excontext, refer, "REFER", to, from, route);
  osip_to_free (_to);
  if (i != 0)
    return i;
  _eXosip_dialog_add_contact (excontext, *refer);

  osip_message_set_header (*refer, "Refer-to", refer_to);

  return OSIP_SUCCESS;
}

int
eXosip_subscription_send_initial_request (struct eXosip_t *excontext, osip_message_t * subscribe)
{
  eXosip_subscribe_t *js = NULL;
  osip_transaction_t *transaction;
  osip_event_t *sipevent;
  int i;

  i = _eXosip_subscription_init (excontext, &js);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot subscribe."));
    osip_message_free (subscribe);
    return i;
  }

  i = _eXosip_transaction_init (excontext, &transaction, NICT, excontext->j_osip, subscribe);
  if (i != 0) {
    _eXosip_subscription_free (excontext, js);
    osip_message_free (subscribe);
    return i;
  }

  js->s_reg_period = 3600;
  _eXosip_subscription_set_refresh_interval (js, subscribe);
  js->s_out_tr = transaction;

  sipevent = osip_new_outgoing_sipmessage (subscribe);
  sipevent->transactionid = transaction->transactionid;

  osip_transaction_set_reserved5 (transaction, js);
  osip_transaction_add_event (transaction, sipevent);

  ADD_ELEMENT (excontext->j_subscribes, js);
  _eXosip_update (excontext);   /* fixed? */
  _eXosip_wakeup (excontext);
  return js->s_id;
}

int
eXosip_subscription_build_refresh_request (struct eXosip_t *excontext, int did, osip_message_t ** sub)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_subscribe_t *js = NULL;

  osip_transaction_t *transaction;
  int i;

  *sub = NULL;

  if (did <= 0)
    return OSIP_BADPARAMETER;

  _eXosip_subscription_dialog_find (excontext, did, &js, &jd);

  if (jd == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No subscribe/refer here?\n"));
    return OSIP_NOTFOUND;
  }

  transaction = _eXosip_find_last_out_subscribe (js, jd);

  if (transaction != NULL) {
    if (transaction->state != NICT_TERMINATED && transaction->state != NIST_TERMINATED && transaction->state != NICT_COMPLETED && transaction->state != NIST_COMPLETED) {
      return OSIP_WRONG_STATE;
    }
  }
  if (transaction == NULL || transaction->orig_request == NULL || transaction->orig_request->cseq == NULL || transaction->orig_request->cseq->method == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_BUG, NULL, "eXosip: is this a SUBSCRIBE or REFER?\n"));
    return OSIP_UNDEFINED_ERROR;
  }

  i = _eXosip_build_request_within_dialog (excontext, sub, transaction->orig_request->cseq->method, jd->d_dialog);

  if (i != 0)
    return i;

  {
    int pos = 0;
    osip_header_t *_header = NULL;
    osip_call_info_t *_call_info_header = NULL;

    pos = osip_message_get_supported (transaction->orig_request, pos, &_header);
    while (pos >= 0 && _header != NULL) {
      osip_header_t *_header2;

      i = osip_header_clone (_header, &_header2);
      if (i != 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Error in Supported header\n"));
        break;
      }
      osip_list_add (&(*sub)->headers, _header2, -1);
      _header = NULL;
      pos++;
      pos = osip_message_get_supported (transaction->orig_request, pos, &_header);
    }

    pos = 0;
    pos = osip_message_get_call_info (transaction->orig_request, pos, &_call_info_header);
    while (pos >= 0 && _call_info_header != NULL) {
      osip_call_info_t *_header2;

      i = osip_call_info_clone (_call_info_header, &_header2);
      if (i != 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Error in Call-Info header\n"));
        break;
      }
      osip_list_add (&(*sub)->call_infos, _header2, -1);
      _call_info_header = NULL;
      pos++;
      pos = osip_message_get_call_info (transaction->orig_request, pos, &_call_info_header);
    }
  }

  _eXosip_add_authentication_information (excontext, *sub, NULL);

  return OSIP_SUCCESS;
}

int
eXosip_subscription_send_refresh_request (struct eXosip_t *excontext, int did, osip_message_t * sub)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_subscribe_t *js = NULL;

  osip_transaction_t *transaction;
  osip_event_t *sipevent;
  int i;

  if (did <= 0)
    return OSIP_BADPARAMETER;

  if (did > 0) {
    _eXosip_subscription_dialog_find (excontext, did, &js, &jd);
  }
  if (jd == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No subscribe here?\n"));
    osip_message_free (sub);
    return OSIP_NOTFOUND;
  }

  transaction = NULL;
  transaction = _eXosip_find_last_out_subscribe (js, jd);

  if (transaction != NULL) {
    if (transaction->state != NICT_TERMINATED && transaction->state != NIST_TERMINATED && transaction->state != NICT_COMPLETED && transaction->state != NIST_COMPLETED) {
      osip_message_free (sub);
      return OSIP_WRONG_STATE;
    }
    transaction = NULL;
  }

  transaction = NULL;
  i = _eXosip_transaction_init (excontext, &transaction, NICT, excontext->j_osip, sub);

  if (i != 0) {
    osip_message_free (sub);
    return i;
  }

  js->s_reg_period = 3600;
  _eXosip_subscription_set_refresh_interval (js, sub);
  osip_list_add (jd->d_out_trs, transaction, 0);

  sipevent = osip_new_outgoing_sipmessage (sub);
  sipevent->transactionid = transaction->transactionid;

  osip_transaction_set_reserved5 (transaction, js);
  osip_transaction_set_reserved3 (transaction, jd);

  osip_transaction_add_event (transaction, sipevent);
  _eXosip_wakeup (excontext);
  return OSIP_SUCCESS;
}

int
_eXosip_subscription_automatic_refresh (struct eXosip_t *excontext, eXosip_subscribe_t * js, eXosip_dialog_t * jd, osip_transaction_t * out_tr)
{
  osip_message_t *sub = NULL;
  osip_header_t *expires;
  int i;

  if (js == NULL || jd == NULL || out_tr == NULL || out_tr->orig_request == NULL)
    return OSIP_BADPARAMETER;

  i = eXosip_subscription_build_refresh_request (excontext, jd->d_id, &sub);
  if (i != 0)
    return i;

  i = osip_message_get_expires (out_tr->orig_request, 0, &expires);
  if (expires != NULL && expires->hvalue != NULL) {
    osip_message_set_expires (sub, expires->hvalue);
  }

  {
    int pos = 0;
    osip_accept_t *_accept = NULL;

    i = osip_message_get_accept (out_tr->orig_request, pos, &_accept);
    while (i >= 0 && _accept != NULL) {
      osip_accept_t *_accept2;

      i = osip_accept_clone (_accept, &_accept2);
      if (i != 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Error in Accept header\n"));
        break;
      }
      osip_list_add (&sub->accepts, _accept2, -1);
      _accept = NULL;
      pos++;
      i = osip_message_get_accept (out_tr->orig_request, pos, &_accept);
    }
  }

  {
    int pos = 0;
    osip_header_t *_event = NULL;

    pos = osip_message_header_get_byname (out_tr->orig_request, "Event", 0, &_event);
    while (pos >= 0 && _event != NULL) {
      osip_header_t *_event2;

      i = osip_header_clone (_event, &_event2);
      if (i != 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Error in Event header\n"));
        break;
      }
      osip_list_add (&sub->headers, _event2, -1);
      _event = NULL;
      pos++;
      pos = osip_message_header_get_byname (out_tr->orig_request, "Event", pos, &_event);
    }
  }

  i = eXosip_subscription_send_refresh_request (excontext, jd->d_id, sub);
  return i;
}

int
_eXosip_subscription_send_request_with_credential (struct eXosip_t *excontext, eXosip_subscribe_t * js, eXosip_dialog_t * jd, osip_transaction_t * out_tr)
{
  osip_transaction_t *tr = NULL;
  osip_message_t *msg = NULL;
  osip_event_t *sipevent;

  int cseq;
  osip_via_t *via;
  int i;

  if (js == NULL)
    return OSIP_BADPARAMETER;
  if (jd != NULL) {
    if (jd->d_out_trs == NULL)
      return OSIP_BADPARAMETER;
  }

  if (out_tr == NULL) {
    out_tr = _eXosip_find_last_out_subscribe (js, jd);
  }

  if (out_tr == NULL || out_tr->orig_request == NULL || out_tr->last_response == NULL)
    return OSIP_NOTFOUND;

  i = osip_message_clone (out_tr->orig_request, &msg);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: could not clone msg for authentication\n"));
    return i;
  }

  {
    osip_generic_param_t *tag = NULL;

    osip_to_get_tag (msg->to, &tag);
    if (NULL == tag && jd != NULL && jd->d_dialog != NULL && jd->d_dialog->remote_tag != NULL) {
      osip_to_set_tag (msg->to, osip_strdup (jd->d_dialog->remote_tag));
    }
  }

  via = (osip_via_t *) osip_list_get (&msg->vias, 0);
  if (via == NULL || msg->cseq == NULL || msg->cseq->number == NULL) {
    osip_message_free (msg);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: missing via or cseq header\n"));
    return OSIP_SYNTAXERROR;
  }

  /* increment cseq */
  cseq = atoi (msg->cseq->number);
  osip_free (msg->cseq->number);
  msg->cseq->number = _eXosip_strdup_printf ("%i", cseq + 1);
  if (msg->cseq->number == NULL) {
    osip_message_free (msg);
    return OSIP_NOMEM;
  }

  if (jd != NULL && jd->d_dialog != NULL) {
    jd->d_dialog->local_cseq++;
  }

  i = _eXosip_update_top_via (excontext, msg);
  if (i != 0) {
    osip_message_free (msg);
    return i;
  }

  osip_list_special_free (&msg->authorizations, (void (*)(void *)) &osip_authorization_free);
  osip_list_special_free (&msg->proxy_authorizations, (void (*)(void *)) &osip_proxy_authorization_free);

  if (out_tr->last_response->status_code == 401 || out_tr->last_response->status_code == 407) {
    _eXosip_add_authentication_information (excontext, msg, out_tr->last_response);
  }
  else
    _eXosip_add_authentication_information (excontext, msg, NULL);


  if (out_tr != NULL && out_tr->last_response != NULL && out_tr->last_response->status_code == 423) {
    /* increase expires value to "min-expires" value */
    osip_header_t *exp;
    osip_header_t *min_exp;

    osip_message_header_get_byname (msg, "expires", 0, &exp);
    osip_message_header_get_byname (out_tr->last_response, "min-expires", 0, &min_exp);
    if (exp != NULL && exp->hvalue != NULL && min_exp != NULL && min_exp->hvalue != NULL) {
      osip_free (exp->hvalue);
      exp->hvalue = osip_strdup (min_exp->hvalue);
    }
    else {
      osip_message_free (msg);
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: missing Min-Expires or Expires in PUBLISH\n"));
      return OSIP_SYNTAXERROR;
    }
  }


  osip_message_force_update (msg);

  i = _eXosip_transaction_init (excontext, &tr, NICT, excontext->j_osip, msg);

  if (i != 0) {
    osip_message_free (msg);
    return i;
  }

  if (out_tr == js->s_out_tr) {
    /* replace with the new tr */
    osip_list_add (&excontext->j_transactions, js->s_out_tr, 0);
    js->s_out_tr = tr;
  }
  else {
    /* add the new tr for the current dialog */
    osip_list_add (jd->d_out_trs, tr, 0);
  }

  sipevent = osip_new_outgoing_sipmessage (msg);

  osip_transaction_set_reserved5 (tr, js);
  osip_transaction_set_reserved3 (tr, jd);

  osip_transaction_add_event (tr, sipevent);

  _eXosip_update (excontext);   /* fixed? */
  _eXosip_wakeup (excontext);
  return OSIP_SUCCESS;
}

#endif
