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

static int eXosip_create_transaction (struct eXosip_t *excontext, eXosip_call_t * jc, eXosip_dialog_t * jd, osip_message_t * request);
static int eXosip_create_cancel_transaction (struct eXosip_t *excontext, eXosip_call_t * jc, eXosip_dialog_t * jd, osip_message_t * request);

static int _eXosip_call_reuse_contact (osip_message_t * invite, osip_message_t * msg);

static int
eXosip_create_transaction (struct eXosip_t *excontext, eXosip_call_t * jc, eXosip_dialog_t * jd, osip_message_t * request)
{
  osip_event_t *sipevent;
  osip_transaction_t *tr;
  int i;

  i = _eXosip_transaction_init (excontext, &tr, NICT, excontext->j_osip, request);
  if (i != 0) {
    /* TODO: release the j_call.. */

    osip_message_free (request);
    return i;
  }

  if (jd != NULL)
    osip_list_add (jd->d_out_trs, tr, 0);

  sipevent = osip_new_outgoing_sipmessage (request);
  sipevent->transactionid = tr->transactionid;

  osip_transaction_set_reserved2 (tr, jc);
  osip_transaction_set_reserved3 (tr, jd);

  osip_transaction_add_event (tr, sipevent);
  _eXosip_wakeup (excontext);
  return OSIP_SUCCESS;
}

static int
eXosip_create_cancel_transaction (struct eXosip_t *excontext, eXosip_call_t * jc, eXosip_dialog_t * jd, osip_message_t * request)
{
  osip_event_t *sipevent;
  osip_transaction_t *tr;
  int i;

  i = _eXosip_transaction_init (excontext, &tr, NICT, excontext->j_osip, request);
  if (i != 0) {
    /* TODO: release the j_call.. */

    osip_message_free (request);
    return i;
  }

  osip_list_add (&excontext->j_transactions, tr, 0);

  sipevent = osip_new_outgoing_sipmessage (request);
  sipevent->transactionid = tr->transactionid;

  osip_transaction_add_event (tr, sipevent);
  _eXosip_wakeup (excontext);
  return OSIP_SUCCESS;
}

static int
_eXosip_call_reuse_contact (osip_message_t * invite, osip_message_t * msg)
{
  osip_contact_t *co_invite = NULL;
  osip_contact_t *co_msg = NULL;
  int i;

  i = osip_message_get_contact (invite, 0, &co_invite);
  if (i < 0 || co_invite == NULL || co_invite->url == NULL) {
    return i;
  }

  i = osip_message_get_contact (msg, 0, &co_msg);
  if (i >= 0 && co_msg != NULL) {
    osip_list_remove (&msg->contacts, 0);
    osip_contact_free (co_msg);
  }

  co_msg = NULL;
  i = osip_contact_clone (co_invite, &co_msg);
  if (i >= 0 && co_msg != NULL) {
    osip_list_add (&msg->contacts, co_msg, 0);
    return OSIP_SUCCESS;
  }
  return i;
}

int
_eXosip_call_transaction_find (struct eXosip_t *excontext, int tid, eXosip_call_t ** jc, eXosip_dialog_t ** jd, osip_transaction_t ** tr)
{
  for (*jc = excontext->j_calls; *jc != NULL; *jc = (*jc)->next) {
    if ((*jc)->c_inc_tr != NULL && (*jc)->c_inc_tr->transactionid == tid) {
      *tr = (*jc)->c_inc_tr;
      *jd = (*jc)->c_dialogs;
      return OSIP_SUCCESS;
    }
    if ((*jc)->c_out_tr != NULL && (*jc)->c_out_tr->transactionid == tid) {
      *tr = (*jc)->c_out_tr;
      *jd = (*jc)->c_dialogs;
      return OSIP_SUCCESS;
    }
    for (*jd = (*jc)->c_dialogs; *jd != NULL; *jd = (*jd)->next) {
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
  *jc = NULL;
  return OSIP_NOTFOUND;
}

int
eXosip_call_set_reference (struct eXosip_t *excontext, int id, void *reference)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;

  if (id > 0) {
    _eXosip_call_dialog_find (excontext, id, &jc, &jd);
    if (jc == NULL) {
      _eXosip_call_find (excontext, id, &jc);
    }
  }
  if (jc == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No call here?\n"));
    return OSIP_NOTFOUND;
  }
  jc->external_reference = reference;
  return OSIP_SUCCESS;
}

void *
eXosip_call_get_reference (struct eXosip_t *excontext, int cid)
{
  eXosip_call_t *jc = NULL;

  _eXosip_call_find (excontext, cid, &jc);
  if (jc == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No call here?\n"));
    return NULL;
  }

  return jc->external_reference;
}

/* this method can't be called unless the previous
   INVITE transaction is over. */
int
eXosip_call_build_initial_invite (struct eXosip_t *excontext, osip_message_t ** invite, const char *to, const char *from, const char *route, const char *subject)
{
  int i;
  osip_to_t *_to = NULL;
  osip_header_t *subject_header;

  *invite = NULL;

  if (to != NULL && *to == '\0')
    return OSIP_BADPARAMETER;
  if (route != NULL && *route == '\0')
    route = NULL;
  if (subject != NULL && *subject == '\0')
    subject = NULL;

  i = osip_to_init (&_to);
  if (i != 0)
    return i;

  i = osip_to_parse (_to, to);
  if (i != 0) {
    osip_to_free (_to);
    return i;
  }

  i = _eXosip_generating_request_out_of_dialog (excontext, invite, "INVITE", to, from, route);
  osip_to_free (_to);
  if (i != 0)
    return i;
  _eXosip_dialog_add_contact (excontext, *invite);

  subject_header = NULL;
  osip_message_get_subject (*invite, 0, &subject_header);
  if (subject_header == NULL && subject != NULL)
    osip_message_set_subject (*invite, subject);

  return OSIP_SUCCESS;
}

int
eXosip_call_send_initial_invite (struct eXosip_t *excontext, osip_message_t * invite)
{
  eXosip_call_t *jc;
  osip_transaction_t *transaction;
  osip_event_t *sipevent;
  int i;

  if (invite == NULL) {
    return OSIP_BADPARAMETER;
  }

  i = _eXosip_call_init (excontext, &jc);
  if (i != 0) {
    osip_message_free (invite);
    return i;
  }

  i = _eXosip_transaction_init (excontext, &transaction, ICT, excontext->j_osip, invite);
  if (i != 0) {
    _eXosip_call_free (excontext, jc);
    osip_message_free (invite);
    return i;
  }

  jc->c_out_tr = transaction;

  sipevent = osip_new_outgoing_sipmessage (invite);
  sipevent->transactionid = transaction->transactionid;

  osip_transaction_set_reserved2 (transaction, jc);
  osip_transaction_add_event (transaction, sipevent);

  jc->external_reference = NULL;
  ADD_ELEMENT (excontext->j_calls, jc);

  _eXosip_update (excontext);   /* fixed? */
  _eXosip_wakeup (excontext);
  return jc->c_id;
}

int
eXosip_call_build_ack (struct eXosip_t *excontext, int did, osip_message_t ** _ack)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  osip_transaction_t *tr = NULL;

  osip_message_t *ack;
  int i;

  *_ack = NULL;

  if (did <= 0)
    return OSIP_BADPARAMETER;
  if (did > 0) {
    _eXosip_call_dialog_find (excontext, did, &jc, &jd);
  }
  if (jc == NULL || jd == NULL || jd->d_dialog == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No call here?\n"));
    return OSIP_NOTFOUND;
  }

  tr = _eXosip_find_last_invite (jc, jd);

  if (tr == NULL || tr->orig_request == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No transaction for call?\n"));
    return OSIP_NOTFOUND;
  }

  if (0 != osip_strcasecmp (tr->orig_request->sip_method, "INVITE")) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: ACK are only sent for invite transactions\n"));
    return OSIP_BADPARAMETER;
  }

  i = _eXosip_build_request_within_dialog (excontext, &ack, "ACK", jd->d_dialog);

  if (i != 0) {
    return i;
  }

  _eXosip_call_reuse_contact (tr->orig_request, ack);

  /* Fix CSeq Number when request has been exchanged during INVITE transactions */
  if (tr->orig_request->cseq != NULL && tr->orig_request->cseq->number != NULL) {
    if (ack != NULL && ack->cseq != NULL && ack->cseq->number != NULL) {
      osip_free (ack->cseq->number);
      ack->cseq->number = osip_strdup (tr->orig_request->cseq->number);
    }
  }

  /* copy all credentials from INVITE! */
  {
    int pos = 0;
    osip_proxy_authorization_t *pa = NULL;

    i = osip_message_get_proxy_authorization (tr->orig_request, pos, &pa);
    while (i >= 0 && pa != NULL) {
      osip_proxy_authorization_t *pa2;

      i = osip_proxy_authorization_clone (pa, &pa2);
      if (i != 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Error in credential from INVITE\n"));
        break;
      }
      osip_list_add (&ack->proxy_authorizations, pa2, -1);
      pa = NULL;
      pos++;
      i = osip_message_get_proxy_authorization (tr->orig_request, pos, &pa);
    }
  }

  *_ack = ack;
  return OSIP_SUCCESS;
}

int
eXosip_call_send_ack (struct eXosip_t *excontext, int did, osip_message_t * ack)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  int i;

  osip_route_t *route;
  char *host = NULL;
  int port;

  if (did <= 0) {
    if (ack != NULL)
      osip_message_free (ack);
    return OSIP_BADPARAMETER;
  }
  if (did > 0) {
    _eXosip_call_dialog_find (excontext, did, &jc, &jd);
  }

  if (jc == NULL || jd == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No call here?\n"));
    if (ack != NULL)
      osip_message_free (ack);
    return OSIP_NOTFOUND;
  }

  if (ack == NULL) {
    i = eXosip_call_build_ack (excontext, did, &ack);
    if (i != 0) {
      return i;
    }
  }

  if (host == NULL) {
    osip_message_get_route (ack, 0, &route);
    if (route != NULL) {
      osip_uri_param_t *lr_param = NULL;

      osip_uri_uparam_get_byname (route->url, "lr", &lr_param);
      if (lr_param == NULL)
        route = NULL;
    }

    if (route != NULL) {
      port = 5060;
      if (route->url->port != NULL)
        port = osip_atoi (route->url->port);
      host = route->url->host;
    }
    else {
      /* search for maddr parameter */
      osip_uri_param_t *maddr_param = NULL;

      osip_uri_uparam_get_byname (ack->req_uri, "maddr", &maddr_param);
      host = NULL;
      if (maddr_param != NULL && maddr_param->gvalue != NULL)
        host = maddr_param->gvalue;

      port = 5060;
      if (ack->req_uri->port != NULL)
        port = osip_atoi (ack->req_uri->port);

      if (host == NULL)
        host = ack->req_uri->host;
    }
  }

  i = _eXosip_snd_message (excontext, NULL, ack, host, port, -1);

  if (jd->d_ack != NULL)
    osip_message_free (jd->d_ack);
  jd->d_ack = ack;
  if (i < 0)
    return i;

  /* TODO: could be 1 for icmp... */
  return OSIP_SUCCESS;
}

int
eXosip_call_build_request (struct eXosip_t *excontext, int jid, const char *method, osip_message_t ** request)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;

  osip_transaction_t *transaction;
  int i;

  *request = NULL;
  if (jid <= 0)
    return OSIP_BADPARAMETER;
  if (method == NULL || method[0] == '\0')
    return OSIP_BADPARAMETER;

  if (jid > 0) {
    _eXosip_call_dialog_find (excontext, jid, &jc, &jd);
  }
  if (jd == NULL || jd->d_dialog == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No call here?\n"));
    return OSIP_NOTFOUND;
  }

  transaction = NULL;
  if (0 == osip_strcasecmp (method, "INVITE")) {
    transaction = _eXosip_find_last_invite (jc, jd);
  }
  else {                        /* OPTIONS, UPDATE, INFO, REFER, ?... */

    transaction = _eXosip_find_last_transaction (jc, jd, method);
  }

  if (transaction != NULL) {
    if (0 != osip_strcasecmp (method, "INVITE")) {
#ifndef DONOTWAIT_ENDOFTRANSACTION
      if (transaction->state != NICT_TERMINATED && transaction->state != NIST_TERMINATED && transaction->state != NICT_COMPLETED && transaction->state != NIST_COMPLETED)
        return OSIP_WRONG_STATE;
#endif
    }
    else {
      if (transaction->state != ICT_TERMINATED && transaction->state != IST_TERMINATED && transaction->state != IST_CONFIRMED && transaction->state != ICT_COMPLETED)
        return OSIP_WRONG_STATE;
    }
  }

  i = _eXosip_build_request_within_dialog (excontext, request, method, jd->d_dialog);
  if (i != 0)
    return i;

  _eXosip_add_authentication_information (excontext, *request, NULL);

  return OSIP_SUCCESS;
}

int
eXosip_call_send_request (struct eXosip_t *excontext, int jid, osip_message_t * request)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;

  osip_transaction_t *transaction;
  osip_event_t *sipevent;

  int i;

  if (request == NULL)
    return OSIP_BADPARAMETER;
  if (jid <= 0) {
    osip_message_free (request);
    return OSIP_BADPARAMETER;
  }

  if (request->sip_method == NULL) {
    osip_message_free (request);
    return OSIP_BADPARAMETER;
  }

  if (jid > 0) {
    _eXosip_call_dialog_find (excontext, jid, &jc, &jd);
  }
  if (jd == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No call here?\n"));
    osip_message_free (request);
    return OSIP_NOTFOUND;
  }

  transaction = NULL;
  if (0 == osip_strcasecmp (request->sip_method, "INVITE")) {
    transaction = _eXosip_find_last_invite (jc, jd);
  }
  else {                        /* OPTIONS, UPDATE, INFO, REFER, ?... */

    transaction = _eXosip_find_last_transaction (jc, jd, request->sip_method);
  }

  if (transaction != NULL) {
    if (0 != osip_strcasecmp (request->sip_method, "INVITE")) {
#ifndef DONOTWAIT_ENDOFTRANSACTION
      if (transaction->state != NICT_TERMINATED && transaction->state != NIST_TERMINATED && transaction->state != NICT_COMPLETED && transaction->state != NIST_COMPLETED) {
        osip_message_free (request);
        return OSIP_WRONG_STATE;
      }
#endif
    }
    else {
      if (transaction->state != ICT_TERMINATED && transaction->state != IST_TERMINATED && transaction->state != IST_CONFIRMED && transaction->state != ICT_COMPLETED) {
        osip_message_free (request);
        return OSIP_WRONG_STATE;
      }
    }
  }

  transaction = NULL;
  if (0 != osip_strcasecmp (request->sip_method, "INVITE")) {
    i = _eXosip_transaction_init (excontext, &transaction, NICT, excontext->j_osip, request);
  }
  else {
    i = _eXosip_transaction_init (excontext, &transaction, ICT, excontext->j_osip, request);
  }

  if (i != 0) {
    osip_message_free (request);
    return i;
  }

  osip_list_add (jd->d_out_trs, transaction, 0);

  sipevent = osip_new_outgoing_sipmessage (request);
  sipevent->transactionid = transaction->transactionid;

  osip_transaction_set_reserved2 (transaction, jc);
  osip_transaction_set_reserved3 (transaction, jd);

  osip_transaction_add_event (transaction, sipevent);
  _eXosip_wakeup (excontext);
  return OSIP_SUCCESS;
}

#ifndef MINISIZE

int
eXosip_call_build_refer (struct eXosip_t *excontext, int did, const char *refer_to, osip_message_t ** request)
{
  int i;

  *request = NULL;
  i = eXosip_call_build_request (excontext, did, "REFER", request);
  if (i != 0)
    return i;

  if (refer_to == NULL || refer_to[0] == '\0')
    return OSIP_SUCCESS;

  osip_message_set_header (*request, "Refer-to", refer_to);
  return OSIP_SUCCESS;
}

int
eXosip_call_build_options (struct eXosip_t *excontext, int did, osip_message_t ** request)
{
  int i;

  *request = NULL;
  i = eXosip_call_build_request (excontext, did, "OPTIONS", request);
  if (i != 0)
    return i;

  return OSIP_SUCCESS;
}

int
eXosip_call_build_info (struct eXosip_t *excontext, int did, osip_message_t ** request)
{
  int i;

  *request = NULL;
  i = eXosip_call_build_request (excontext, did, "INFO", request);
  if (i != 0)
    return i;

  return OSIP_SUCCESS;
}

int
eXosip_call_build_update (struct eXosip_t *excontext, int did, osip_message_t ** request)
{
  int i;

  *request = NULL;
  i = eXosip_call_build_request (excontext, did, "UPDATE", request);
  if (i != 0)
    return i;

  return OSIP_SUCCESS;
}

int
eXosip_call_build_notify (struct eXosip_t *excontext, int did, int subscription_status, osip_message_t ** request)
{
  char subscription_state[50];
  char *tmp;
  int i;

  *request = NULL;
  i = eXosip_call_build_request (excontext, did, "NOTIFY", request);
  if (i != 0)
    return i;

  if (subscription_status == EXOSIP_SUBCRSTATE_PENDING)
    osip_strncpy (subscription_state, "pending;expires=", 16);
  else if (subscription_status == EXOSIP_SUBCRSTATE_ACTIVE)
    osip_strncpy (subscription_state, "active;expires=", 15);
  else if (subscription_status == EXOSIP_SUBCRSTATE_TERMINATED) {
#if 0
    int reason = NORESOURCE;

    if (reason == DEACTIVATED)
      osip_strncpy (subscription_state, "terminated;reason=deactivated", 29);
    else if (reason == PROBATION)
      osip_strncpy (subscription_state, "terminated;reason=probation", 27);
    else if (reason == REJECTED)
      osip_strncpy (subscription_state, "terminated;reason=rejected", 26);
    else if (reason == TIMEOUT)
      osip_strncpy (subscription_state, "terminated;reason=timeout", 25);
    else if (reason == GIVEUP)
      osip_strncpy (subscription_state, "terminated;reason=giveup", 24);
    else if (reason == NORESOURCE)
#endif
      osip_strncpy (subscription_state, "terminated;reason=noresource", 29);
  }
  tmp = subscription_state + strlen (subscription_state);
  if (subscription_status != EXOSIP_SUBCRSTATE_TERMINATED)
    snprintf (tmp, 50 - (tmp - subscription_state), "%i", 180);
  osip_message_set_header (*request, "Subscription-State", subscription_state);

  return OSIP_SUCCESS;
}

#endif

int
eXosip_call_build_answer (struct eXosip_t *excontext, int tid, int status, osip_message_t ** answer)
{
  int i = -1;
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  osip_transaction_t *tr = NULL;

  *answer = NULL;

  if (tid < 0)
    return OSIP_BADPARAMETER;
  if (status <= 100)
    return OSIP_BADPARAMETER;
  if (status > 699)
    return OSIP_BADPARAMETER;

  if (tid > 0) {
    _eXosip_call_transaction_find (excontext, tid, &jc, &jd, &tr);
  }
  if (tr == NULL || jd == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No call here?\n"));
    return OSIP_NOTFOUND;
  }

  if (0 == osip_strcasecmp (tr->orig_request->sip_method, "INVITE")) {
    i = _eXosip_answer_invite_123456xx (excontext, jc, jd, status, answer, 0);
  }
  else {
    i = _eXosip_build_response_default (excontext, answer, jd->d_dialog, status, tr->orig_request);
    if (i != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "ERROR: Could not create response for %s\n", tr->orig_request->sip_method));
      return i;
    }
    if (status > 100 && status < 300)
      i = _eXosip_complete_answer_that_establish_a_dialog (excontext, *answer, tr->orig_request);
  }

  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "ERROR: Could not create response for %s\n", tr->orig_request->sip_method));
    return i;
  }
  return OSIP_SUCCESS;
}

static osip_header_t *
_eXosip_header_strcasestr(osip_message_t *message, const char *hname, const char *hname_short, const char *value) {
  osip_header_t *header;
  int i;
  i = osip_message_header_get_byname (message, hname, 0, &header);
  while (i >= 0) {
    if (header == NULL)
      break;
    if (header->hvalue != NULL && osip_strcasestr (header->hvalue, value) != NULL) {
      /*found */
      break;
    }
    header = NULL;
    i = osip_message_header_get_byname (message, hname, i + 1, &header);
  }
  if (header == NULL) {
    i = osip_message_header_get_byname (message, hname_short, 0, &header);
    while (i >= 0) {
      if (header == NULL)
        break;
      if (header->hvalue != NULL && osip_strcasestr (header->hvalue, value) != NULL) {
        /*found */
        break;
      }
      header = NULL;
      i = osip_message_header_get_byname (message, hname_short, i + 1, &header);
    }
  }
  return header;
}

int
eXosip_call_send_answer (struct eXosip_t *excontext, int tid, int status, osip_message_t * answer)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  osip_transaction_t *tr = NULL;
  osip_event_t *evt_answer;

  if (tid < 0) {
    osip_message_free (answer);
    return OSIP_BADPARAMETER;
  }
  if (status <= 100) {
    osip_message_free (answer);
    return OSIP_BADPARAMETER;
  }
  if (status > 699) {
    osip_message_free (answer);
    return OSIP_BADPARAMETER;
  }

  if (tid > 0) {
    _eXosip_call_transaction_find (excontext, tid, &jc, &jd, &tr);
  }
  if (jd == NULL || tr == NULL || tr->orig_request == NULL || tr->orig_request->sip_method == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No call here or no transaction for call\n"));
    osip_message_free (answer);
    return OSIP_NOTFOUND;
  }

  if (answer == NULL) {
    if (0 == osip_strcasecmp (tr->orig_request->sip_method, "INVITE")) {
      if (status >= 200 && status <= 299) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Wrong parameter?\n"));
        osip_message_free (answer);
        return OSIP_BADPARAMETER;
      }
    }
  }

  /* is the transaction already answered? */
  if (tr->state == IST_COMPLETED || tr->state == IST_CONFIRMED || tr->state == IST_TERMINATED || tr->state == NIST_COMPLETED || tr->state == NIST_TERMINATED) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: transaction already answered\n"));
    osip_message_free (answer);
    return OSIP_WRONG_STATE;
  }

  if (answer == NULL) {
    if (0 == osip_strcasecmp (tr->orig_request->sip_method, "INVITE")) {
      osip_message_t *response;

      return _eXosip_answer_invite_123456xx (excontext, jc, jd, status, &response, 1);
    }
    osip_message_free (answer);
    return OSIP_BADPARAMETER;
  }

  if (0 == osip_strcasecmp (tr->orig_request->sip_method, "INVITE")
      || 0 == osip_strcasecmp (tr->orig_request->sip_method, "UPDATE")) {
    if (MSG_IS_STATUS_2XX (answer) && jd != NULL) {
      osip_header_t *supported = NULL;

      /* look for timer in supported header: must be added by user-application */
      supported = _eXosip_header_strcasestr(answer, "supported", "k", "timer");

      if (supported != NULL) {  /* timer is supported */
        /* copy session-expires */
        /* add refresher=uas, if it's not already there */
        osip_header_t *se_exp = NULL;

        osip_message_header_get_byname (tr->orig_request, "session-expires", 0, &se_exp);
        if (se_exp == NULL)
          osip_message_header_get_byname (tr->orig_request, "x", 0, &se_exp);
        if (se_exp != NULL) {
          osip_header_t *cp = NULL;

          osip_header_clone (se_exp, &cp);
          if (cp != NULL) {
            osip_content_disposition_t *exp_h = NULL;

            /* syntax of Session-Expires is equivalent to "Content-Disposition" */
            osip_content_disposition_init (&exp_h);
            if (exp_h == NULL) {
              osip_header_free (cp);
            }
            else {
              osip_content_disposition_parse (exp_h, se_exp->hvalue);
              if (exp_h->element == NULL) {
                osip_content_disposition_free (exp_h);
                osip_header_free (cp);
                exp_h = NULL;
              }
              else {
                osip_generic_param_t *param = NULL;

                osip_generic_param_get_byname (&exp_h->gen_params, "refresher", &param);
                if (param == NULL) {
                  osip_generic_param_add (&exp_h->gen_params, osip_strdup ("refresher"), osip_strdup ("uas"));
                  osip_free (cp->hvalue);
                  cp->hvalue = NULL;
                  osip_content_disposition_to_str (exp_h, &cp->hvalue);
                  jd->d_refresher = 0;
                }
                else {
                  if (osip_strcasecmp (param->gvalue, "uas") == 0)
                    jd->d_refresher = 0;
                  else
                    jd->d_refresher = 1;
                }
                jd->d_session_timer_start = osip_getsystemtime (NULL);
                jd->d_session_timer_length = atoi (exp_h->element);
                if (jd->d_session_timer_length <= 90)
                  jd->d_session_timer_length = 90;
                osip_list_add (&answer->headers, cp, 0);
              }
            }
            if (exp_h)
              osip_content_disposition_free (exp_h);
            exp_h = NULL;


            /* add Require only if remote UA support "timer" */
            supported = _eXosip_header_strcasestr(tr->orig_request, "supported", "k", "timer");
            if (supported != NULL) {    /* timer is supported */
              osip_message_set_header (answer, "Require", "timer");
            }
          }
        }
      }
    }
  }

  if (0 == osip_strcasecmp (tr->orig_request->sip_method, "INVITE")) {
    if (MSG_IS_STATUS_2XX (answer) && jd != NULL) {
      if (status >= 200 && status < 300 && jd != NULL) {
        _eXosip_dialog_set_200ok (jd, answer);
        /* wait for a ACK */
        osip_dialog_set_state (jd->d_dialog, DIALOG_CONFIRMED);
      }
    }
  }

  evt_answer = osip_new_outgoing_sipmessage (answer);
  evt_answer->transactionid = tr->transactionid;

  osip_transaction_add_event (tr, evt_answer);
  _eXosip_update (excontext);
  _eXosip_wakeup (excontext);
  return OSIP_SUCCESS;
}

int
eXosip_call_terminate (struct eXosip_t *excontext, int cid, int did) {
  return eXosip_call_terminate_with_reason(excontext, cid, did, NULL);
}

int
eXosip_call_terminate_with_reason (struct eXosip_t *excontext, int cid, int did, const char *reason)
{
  int i;
  osip_transaction_t *tr;
  osip_message_t *request = NULL;
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;

  if (did <= 0 && cid <= 0)
    return OSIP_BADPARAMETER;
  if (did > 0) {
    _eXosip_call_dialog_find (excontext, did, &jc, &jd);
    if (jd == NULL) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No call here?\n"));
      return OSIP_NOTFOUND;
    }
  }
  else {
    _eXosip_call_find (excontext, cid, &jc);
  }

  if (jc == NULL) {
    return OSIP_NOTFOUND;
  }

  tr = _eXosip_find_last_out_invite (jc, jd);
  if (jd != NULL && jd->d_dialog != NULL && jd->d_dialog->state == DIALOG_CONFIRMED) {
    /* don't send CANCEL on re-INVITE: send BYE instead */
  }
  else if (tr != NULL && tr->last_response != NULL && MSG_IS_STATUS_1XX (tr->last_response)) {
    i = _eXosip_generating_cancel (excontext, &request, tr->orig_request);
    if (i != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot terminate this call!\n"));
      return i;
    }
    if (reason != NULL) {
      osip_message_set_header(request, "Reason", reason);
    }
    i = eXosip_create_cancel_transaction (excontext, jc, jd, request);
    if (i != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot initiate SIP transaction!\n"));
      return i;
    }
    if (jd != NULL) {
      /*Fix: keep dialog opened after the CANCEL.
         osip_dialog_free(jd->d_dialog);
         jd->d_dialog = NULL;
         _eXosip_update(excontext); */
    }
    return OSIP_SUCCESS + 1;
  }

  if (jd == NULL || jd->d_dialog == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No established dialog!\n"));
    return OSIP_WRONG_STATE;
  }

  if (tr == NULL) {
    /*this may not be enough if it's a re-INVITE! */
    tr = _eXosip_find_last_inc_invite (jc, jd);
    if (tr != NULL && tr->last_response != NULL && MSG_IS_STATUS_1XX (tr->last_response)) {     /* answer with 603 */
      osip_generic_param_t *to_tag;

      osip_from_param_get_byname (tr->orig_request->to, "tag", &to_tag);

      i = eXosip_call_build_answer (excontext, tr->transactionid, 603, &request);

      if (reason != NULL) {
        osip_message_set_header(request, "Reason", reason);
      }

      i = eXosip_call_send_answer (excontext, tr->transactionid, 603, request);
      if (to_tag == NULL)
        return i;
    }
  }

  if (jd->d_dialog == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot terminate this call!\n"));
    return OSIP_WRONG_STATE;
  }

  i = _eXosip_generating_bye (excontext, &request, jd->d_dialog);

  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot terminate this call!\n"));
    return i;
  }

	if (reason != NULL) {
		osip_message_set_header(request, "Reason", reason);
	}

  _eXosip_add_authentication_information (excontext, request, NULL);

  i = eXosip_create_transaction (excontext, jc, jd, request);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot initiate SIP transaction!\n"));
    return i;
  }

  osip_dialog_free (jd->d_dialog);
  jd->d_dialog = NULL;
  _eXosip_update (excontext);   /* AMD 30/09/05 */
  return OSIP_SUCCESS;
}

#ifndef MINISIZE

int
eXosip_call_build_prack (struct eXosip_t *excontext, int tid, osip_message_t *response1xx, osip_message_t ** prack)
{
  osip_list_iterator_t it;
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  osip_transaction_t *tr = NULL;
  osip_transaction_t *old_prack_tr = NULL;
  char tmp[128];

  osip_header_t *rseq;
  int i;

  *prack = NULL;

  if (tid < 0)
    return OSIP_BADPARAMETER;

  if (tid > 0) {
    _eXosip_call_transaction_find (excontext, tid, &jc, &jd, &tr);
  }
  if (jc == NULL || jd == NULL || jd->d_dialog == NULL || tr == NULL || tr->orig_request == NULL || tr->orig_request->sip_method == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No call here or no transaction for call\n"));
    return OSIP_NOTFOUND;
  }

  if (0 != osip_strcasecmp (tr->orig_request->sip_method, "INVITE"))
    return OSIP_BADPARAMETER;

  /* PRACK are only send in the PROCEEDING state */
  if (tr->state != ICT_PROCEEDING)
    return OSIP_WRONG_STATE;

  if (tr->orig_request->cseq == NULL || tr->orig_request->cseq->number == NULL || tr->orig_request->cseq->method == NULL)
    return OSIP_SYNTAXERROR;

  osip_message_header_get_byname (response1xx, "RSeq", 0, &rseq);
  if (rseq == NULL || rseq->hvalue == NULL) {
    return OSIP_WRONG_FORMAT;
  }

  memset (tmp, '\0', sizeof (tmp));
  snprintf (tmp, 127, "%s %s %s", rseq->hvalue, tr->orig_request->cseq->number, tr->orig_request->cseq->method);

  old_prack_tr = (osip_transaction_t*)osip_list_get_first(jd->d_out_trs, &it);
  while (old_prack_tr != NULL) {
    if (old_prack_tr != NULL && old_prack_tr->orig_request != NULL && 0 == osip_strcasecmp (old_prack_tr->orig_request->sip_method, "PRACK")) {
      osip_header_t *rack_header = NULL;

      osip_message_header_get_byname (old_prack_tr->orig_request, "RAck", 0, &rack_header);
      if (rack_header != NULL && rack_header->hvalue != NULL && 0 == osip_strcasecmp (rack_header->hvalue, tmp)) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: PRACK already active for last answer.\n"));
        return OSIP_WRONG_STATE;
      }
    }
    old_prack_tr = (osip_transaction_t *)osip_list_get_next(&it);
  }

  {
    osip_dialog_t *_1xxok_dialog = NULL;
    i = osip_dialog_init_as_uac (&_1xxok_dialog, response1xx);
    if (i != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot build a dialog for this 1xx answer.\n"));
      return OSIP_WRONG_STATE;
    }

    i = _eXosip_build_request_within_dialog (excontext, prack, "PRACK", _1xxok_dialog);

    osip_dialog_free(_1xxok_dialog);

    if (i != 0)
      return i;
  }

  osip_message_set_header (*prack, "RAck", tmp);

  return OSIP_SUCCESS;
}

int
eXosip_call_send_prack (struct eXosip_t *excontext, int tid, osip_message_t * prack)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  osip_transaction_t *tr = NULL;

  osip_event_t *sipevent;
  int i;

  if (tid < 0)
    return OSIP_BADPARAMETER;
  if (prack == NULL)
    return OSIP_BADPARAMETER;

  if (tid > 0) {
    _eXosip_call_transaction_find (excontext, tid, &jc, &jd, &tr);
  }
  if (jc == NULL || jd == NULL || jd->d_dialog == NULL || tr == NULL || tr->orig_request == NULL || tr->orig_request->sip_method == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No call here or no transaction for call\n"));
    osip_message_free (prack);
    return OSIP_NOTFOUND;
  }

  if (0 != osip_strcasecmp (tr->orig_request->sip_method, "INVITE")) {
    osip_message_free (prack);
    return OSIP_BADPARAMETER;
  }

  /* PRACK are only send in the PROCEEDING state */
  if (tr->state != ICT_PROCEEDING) {
    osip_message_free (prack);
    return OSIP_WRONG_STATE;
  }

  tr = NULL;
  i = _eXosip_transaction_init (excontext, &tr, NICT, excontext->j_osip, prack);

  if (i != 0) {
    osip_message_free (prack);
    return i;
  }

  jd->d_mincseq++;

  osip_list_add (jd->d_out_trs, tr, 0);

  sipevent = osip_new_outgoing_sipmessage (prack);
  sipevent->transactionid = tr->transactionid;

  osip_transaction_set_reserved2 (tr, jc);
  osip_transaction_set_reserved3 (tr, jd);

  osip_transaction_add_event (tr, sipevent);
  _eXosip_wakeup (excontext);
  return OSIP_SUCCESS;
}

#endif

int
_eXosip_call_retry_request (struct eXosip_t *excontext, eXosip_call_t * jc, eXosip_dialog_t * jd, osip_transaction_t * out_tr)
{
  osip_transaction_t *tr = NULL;
  osip_message_t *msg = NULL;
  osip_event_t *sipevent;

  int cseq;
  osip_via_t *via;
  osip_contact_t *co;
  int i;

  if (jc == NULL)
    return OSIP_BADPARAMETER;
  if (jd != NULL) {
    if (jd->d_out_trs == NULL)
      return OSIP_BADPARAMETER;
  }
  if (out_tr == NULL || out_tr->orig_request == NULL || out_tr->last_response == NULL)
    return OSIP_BADPARAMETER;

  i = osip_message_clone (out_tr->orig_request, &msg);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: could not clone msg for authentication\n"));
    return i;
  }

  via = (osip_via_t *) osip_list_get (&msg->vias, 0);
  if (via == NULL || msg->cseq == NULL || msg->cseq->number == NULL) {
    osip_message_free (msg);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: missing via or cseq header\n"));
    return OSIP_SYNTAXERROR;
  }

  if (MSG_IS_STATUS_3XX (out_tr->last_response)) {
    osip_contact_t *co_usable=NULL;
    osip_list_iterator_t it;
    co = (osip_contact_t*)osip_list_get_first(&out_tr->last_response->contacts, &it);
    while (co != NULL) {
      if (co->url != NULL && (osip_strcasestr(co->url->scheme, "sip")!=NULL || osip_strcasestr(co->url->scheme, "tel")!=NULL)) {
        /* check tranport? */
        osip_uri_param_t *u_param;

        u_param = NULL;
        osip_uri_uparam_get_byname (co->url, "transport", &u_param);
        if (u_param == NULL || u_param->gname == NULL || u_param->gvalue == NULL) {
          if (0 == osip_strcasecmp (excontext->transport, "udp"))
            break;              /* no transport param in uri & we want udp */
        }
        else if (0 == osip_strcasecmp (u_param->gvalue, excontext->transport)) {
          break;                /* transport param in uri & match our protocol */
        }
        if (co_usable==NULL)
          co_usable = co;
      }
      co = (osip_contact_t *)osip_list_get_next(&it);
    }

    if (co == NULL || co->url == NULL) {
      /* revert anyway to first(tel/sip/sips) contact // we don't care: we use our own transport with proxy */
      co = co_usable;
    }

    if (co == NULL || co->url == NULL) {
      osip_message_free (msg);
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "eXosip: no contact header usable for SIP redirection\n"));
      return OSIP_SYNTAXERROR;
    }

    /* TODO:
       remove extra parameter from new request-uri
       check usual parameter like "transport"
     */

    if (msg->req_uri != NULL && msg->req_uri->host != NULL && co->url->host != NULL && 0 == osip_strcasecmp (co->url->host, msg->req_uri->host)) {
      osip_uri_param_t *maddr_param = NULL;

      osip_uri_uparam_get_byname (co->url, "maddr", &maddr_param);
      if (maddr_param != NULL && maddr_param->gvalue != NULL) {
        /* This is a redirect server, the route should probably be removed? */
        osip_route_t *route = NULL;
        osip_generic_param_t *tag = NULL;

        osip_message_get_route (msg, 0, &route);
        if (route != NULL) {
          osip_to_get_tag (msg->to, &tag);
          if (tag == NULL && route != NULL && route->url != NULL) {
            osip_list_remove (&msg->routes, 0);
            osip_route_free (route);
          }
        }
      }
    }

    /* replace request-uri with NEW contact address */
    osip_uri_free (msg->req_uri);
    msg->req_uri = NULL;
    i = osip_uri_clone (co->url, &msg->req_uri);
    if (i != 0) {
      osip_message_free (msg);
      return i;
    }

    /* support for diversions headers/draft! */
    {
      osip_header_t *head = (osip_header_t*)osip_list_get_first(&out_tr->last_response->headers, &it);
      while (head != NULL) {
        osip_header_t *copy = NULL;

        if (0 == osip_strcasecmp (head->hname, "diversion")) {
          i = osip_header_clone (head, &copy);
          if (i == 0) {
            osip_list_add (&msg->headers, copy, -1);
          }
        }
        head = (osip_header_t *)osip_list_get_next(&it);
      }
    }

  }
  /* remove all previous authentication headers */
  osip_list_special_free (&msg->authorizations, (void (*)(void *)) &osip_authorization_free);
  osip_list_special_free (&msg->proxy_authorizations, (void (*)(void *)) &osip_proxy_authorization_free);

  /* increment cseq */
  cseq = atoi (msg->cseq->number);
  osip_free (msg->cseq->number);
  msg->cseq->number = _eXosip_strdup_printf ("%i", cseq + 1);
  if (jd != NULL && jd->d_dialog != NULL) {
    jd->d_dialog->local_cseq++;
  }

  i = _eXosip_update_top_via (excontext, msg);
  if (i != 0) {
    osip_message_free (msg);
    return i;
  }

  if (out_tr->last_response->status_code == 422) {
    /* increase expires value to "min-se" value */
    osip_header_t *exp;
    osip_header_t *min_se;

    /* syntax of Min-SE & Session-Expires are equivalent to "Content-Disposition" */
    osip_content_disposition_t *exp_h = NULL;
    osip_content_disposition_t *min_se_h = NULL;

    osip_message_header_get_byname (msg, "session-expires", 0, &exp);
    if (exp == NULL)
      osip_message_header_get_byname (msg, "x", 0, &exp);
    osip_message_header_get_byname (out_tr->last_response, "min-se", 0, &min_se);
    if (exp != NULL && exp->hvalue != NULL && min_se != NULL && min_se->hvalue != NULL) {
      osip_content_disposition_init (&exp_h);
      osip_content_disposition_init (&min_se_h);
      if (exp_h == NULL || min_se_h == NULL) {
        osip_content_disposition_free (exp_h);
        osip_content_disposition_free (min_se_h);
        exp_h = NULL;
        min_se_h = NULL;
      }
      else {
        osip_content_disposition_parse (exp_h, exp->hvalue);
        osip_content_disposition_parse (min_se_h, min_se->hvalue);
        if (exp_h->element == NULL || min_se_h->element == NULL) {
          osip_content_disposition_free (exp_h);
          osip_content_disposition_free (min_se_h);
          exp_h = NULL;
          min_se_h = NULL;
        }
      }
    }

    if (exp_h != NULL && exp_h->element != NULL && min_se_h != NULL && min_se_h->element != NULL) {
      osip_header_t *min_se_new = NULL;
      char minse_new[32];

      memset (minse_new, 0, sizeof (minse_new));

      osip_free (exp_h->element);
      exp_h->element = osip_strdup (min_se_h->element);

      /* rebuild session-expires with new value/same paramters */
      osip_free (exp->hvalue);
      exp->hvalue = NULL;
      osip_content_disposition_to_str (exp_h, &exp->hvalue);

      /* add or update Min-SE in INVITE: */
      osip_message_header_get_byname (msg, "min-se", 0, &min_se_new);
      if (min_se_new != NULL && min_se_new->hvalue != NULL) {
        osip_free (min_se_new->hvalue);
        min_se_new->hvalue = osip_strdup (min_se_h->element);
      }
      else {
        osip_message_set_header (msg, "Min-SE", min_se_h->element);
      }
    }
    else {
      osip_message_free (msg);
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: missing Min-SE or Session-Expires in dialog\n"));
      return OSIP_SYNTAXERROR;
    }

    osip_content_disposition_free (exp_h);
    osip_content_disposition_free (min_se_h);

  }
  else {
    osip_header_t *exp;

    osip_message_header_get_byname (msg, "session-expires", 0, &exp);
    if (exp == NULL) {
      osip_message_header_get_byname (msg, "x", 0, &exp);
    }
    if (exp == NULL) {
      /* add missing one? */
    }
  }

  if (out_tr->last_response->status_code == 401 || out_tr->last_response->status_code == 407)
    _eXosip_add_authentication_information (excontext, msg, out_tr->last_response);
  else
    _eXosip_add_authentication_information (excontext, msg, NULL);
  osip_message_force_update (msg);

  if (0 != osip_strcasecmp (msg->sip_method, "INVITE")) {
    i = _eXosip_transaction_init (excontext, &tr, NICT, excontext->j_osip, msg);
  }
  else {
    i = _eXosip_transaction_init (excontext, &tr, ICT, excontext->j_osip, msg);
  }

  if (i != 0) {
    osip_message_free (msg);
    return i;
  }

  if (out_tr == jc->c_out_tr) {
    /* replace with the new tr */
    osip_list_add (&excontext->j_transactions, jc->c_out_tr, 0);
    jc->c_out_tr = tr;

    /* fix dialog issue */
    if (jd != NULL) {
      REMOVE_ELEMENT (jc->c_dialogs, jd);
      _eXosip_dialog_free (excontext, jd);
      jd = NULL;
    }
  }
  else {
    /* add the new tr for the current dialog */
    osip_list_add (jd->d_out_trs, tr, 0);
  }

  sipevent = osip_new_outgoing_sipmessage (msg);

  osip_transaction_set_reserved2 (tr, jc);
  osip_transaction_set_reserved3 (tr, jd);

  osip_transaction_add_event (tr, sipevent);

  _eXosip_update (excontext);   /* fixed? */
  _eXosip_wakeup (excontext);
  return OSIP_SUCCESS;
}

#ifndef MINISIZE

int
eXosip_call_get_referto (struct eXosip_t *excontext, int did, char *refer_to, size_t refer_to_len)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  osip_transaction_t *tr = NULL;
  osip_uri_t *referto_uri;
  char atmp[256];
  char *referto_tmp = NULL;
  int i;

  if (did <= 0)
    return OSIP_BADPARAMETER;

  _eXosip_call_dialog_find (excontext, did, &jc, &jd);
  if (jc == NULL || jd == NULL || jd->d_dialog == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No call here?\n"));
    return OSIP_NOTFOUND;
  }

  tr = _eXosip_find_last_invite (jc, jd);

  if (tr == NULL || tr->orig_request == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No transaction for call?\n"));
    return OSIP_NOTFOUND;
  }

  i = osip_uri_clone (jd->d_dialog->remote_uri->url, &referto_uri);
  if (i != 0)
    return i;

  snprintf (atmp, sizeof (atmp), "%s;to-tag=%s;from-tag=%s", jd->d_dialog->call_id, jd->d_dialog->remote_tag, jd->d_dialog->local_tag);

  osip_uri_uheader_add (referto_uri, osip_strdup ("Replaces"), osip_strdup (atmp));
  i = osip_uri_to_str (referto_uri, &referto_tmp);
  if (i != 0) {
    osip_uri_free (referto_uri);
    return i;
  }

  snprintf (refer_to, refer_to_len, "%s", referto_tmp);
  osip_uri_free (referto_uri);
  osip_free (referto_tmp);

  return OSIP_SUCCESS;
}

int
eXosip_call_find_by_replaces (struct eXosip_t *excontext, char *replaces_str)
{
  eXosip_call_t *jc = NULL;
  eXosip_dialog_t *jd = NULL;
  char *call_id;
  char *to_tag;
  char *from_tag;
  char *early_flag;
  char *semicolon;
  char *totag_str = (char *) "to-tag=";
  char *fromtag_str = (char *) "from-tag=";
  char *earlyonly_str = (char *) "early-only";

  /* copy replaces string */
  if (replaces_str == NULL)
    return OSIP_SYNTAXERROR;
  call_id = osip_strdup (replaces_str);
  if (call_id == NULL)
    return OSIP_NOMEM;

  /* parse replaces string */
  to_tag = strstr (call_id, totag_str);
  from_tag = strstr (call_id, fromtag_str);
  early_flag = strstr (call_id, earlyonly_str);

  if ((to_tag == NULL) || (from_tag == NULL)) {
    osip_free (call_id);
    return OSIP_SYNTAXERROR;
  }
  to_tag += strlen (totag_str);
  from_tag += strlen (fromtag_str);

  while ((semicolon = strrchr (call_id, ';')) != NULL) {
    *semicolon = '\0';
  }

  for (jc = excontext->j_calls; jc != NULL; jc = jc->next) {
    for (jd = jc->c_dialogs; jd != NULL; jd = jd->next) {
      if (jd->d_dialog == NULL) {
        /* skip */
      }
      else if (((0 == strcmp (jd->d_dialog->call_id, call_id)) && (0 == strcmp (jd->d_dialog->remote_tag, to_tag)) && (0 == strcmp (jd->d_dialog->local_tag, from_tag)))
               || ((0 == strcmp (jd->d_dialog->call_id, call_id)) && (0 == strcmp (jd->d_dialog->local_tag, to_tag)) && (0 == strcmp (jd->d_dialog->remote_tag, from_tag)))) {
        /* This dialog match! */
        if (jd->d_dialog->state == DIALOG_CONFIRMED && early_flag != NULL) {
          osip_free (call_id);
          return OSIP_WRONG_STATE;      /* confirmed dialog but already answered with 486 */
        }
        if (jd->d_dialog->state == DIALOG_EARLY && jd->d_dialog->type == CALLEE) {
          osip_free (call_id);
          return OSIP_BADPARAMETER;     /* confirmed dialog but already answered with 481 */
        }

        osip_free (call_id);
        return jc->c_id;        /* match anyway */
      }
    }
  }
  osip_free (call_id);
  return OSIP_NOTFOUND;         /* answer with 481 */
}

#endif
