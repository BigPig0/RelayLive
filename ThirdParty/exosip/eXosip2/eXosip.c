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

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#include <eXosip2/eXosip.h>



#ifndef  WIN32
#if !defined(__arc__)
#include <memory.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

/* this value should remain 120, at least, don't use very low values */
#define TRANSACTION_TIMEOUT_RETRY 120

/* Private functions */
static jauthinfo_t *eXosip_find_authentication_info (struct eXosip_t *excontext, const char *username, const char *realm);

void
_eXosip_wakeup (struct eXosip_t *excontext)
{
#ifndef OSIP_MONOTHREAD
  jpipe_write (excontext->j_socketctl, "w", 1);
#endif
}

void
eXosip_wakeup_event (struct eXosip_t *excontext)
{
#ifndef OSIP_MONOTHREAD
  jpipe_write (excontext->j_socketctl_event, "w", 1);
#endif
}

int
eXosip_lock (struct eXosip_t *excontext)
{
#ifndef OSIP_MONOTHREAD
  return osip_mutex_lock ((struct osip_mutex *) excontext->j_mutexlock);
#else
  return OSIP_WRONG_STATE;      /* COMPILED WITH OSIP_MONOTHREAD */
#endif
}

int
eXosip_unlock (struct eXosip_t *excontext)
{
#ifndef OSIP_MONOTHREAD
  return osip_mutex_unlock ((struct osip_mutex *) excontext->j_mutexlock);
#else
  return OSIP_WRONG_STATE;      /* COMPILED WITH OSIP_MONOTHREAD */
#endif
}

int
_eXosip_transaction_init (struct eXosip_t *excontext, osip_transaction_t ** transaction, osip_fsm_type_t ctx_type, osip_t * osip, osip_message_t * message)
{
  int i;

  i = osip_transaction_init (transaction, ctx_type, osip, message);
  if (i != 0) {
    return i;
  }

#ifndef MINISIZE
  {
    struct timeval now;
    excontext->statistics.allocated_transactions++;
    osip_gettimeofday(&now, NULL);
    _eXosip_counters_update(&excontext->average_transactions, 1, &now);
  }
#endif

  osip_transaction_set_reserved1 (*transaction, excontext);
  {
    osip_naptr_t *naptr_record = NULL;
    if (ctx_type==NICT || ctx_type==ICT)
      i = _eXosip_srv_lookup (excontext, message, &naptr_record);
    if (i < 0) {
      /* might be a simple DNS request or an IP */
      return OSIP_SUCCESS;
    }
    osip_transaction_set_naptr_record (*transaction, naptr_record);
  }
  return OSIP_SUCCESS;
}

void
_eXosip_transaction_free (struct eXosip_t *excontext, osip_transaction_t *transaction)
{
  _eXosip_delete_reserved (transaction);
  _eXosip_dnsutils_release (transaction->naptr_record);
  transaction->naptr_record = NULL;
  osip_transaction_free (transaction);
#ifndef MINISIZE
  excontext->statistics.allocated_transactions--;
#endif
}

int
_eXosip_transaction_find (struct eXosip_t *excontext, int tid, osip_transaction_t ** transaction)
{
  osip_list_iterator_t it;
  osip_transaction_t* tr = (osip_transaction_t*)osip_list_get_first(&excontext->j_transactions, &it);
  while (tr != OSIP_SUCCESS) {
    if (tr->transactionid == tid) {
      *transaction = tr;
      return OSIP_SUCCESS;
    }
    tr = (osip_transaction_t *)osip_list_get_next(&it);
  }
  return OSIP_NOTFOUND;
}

#ifndef MINISIZE

static int
_eXosip_retry_with_auth (struct eXosip_t *excontext, eXosip_dialog_t * jd, osip_transaction_t ** ptr, int *retry)
{
  osip_transaction_t *out_tr = NULL;
  osip_transaction_t *tr = NULL;
  osip_message_t *msg = NULL;
  osip_event_t *sipevent;
  void *reserved1;
  int cseq;
  osip_via_t *via;
  int i;

  if (!ptr)
    return OSIP_BADPARAMETER;

  if (jd != NULL) {
    if (jd->d_out_trs == NULL)
      return OSIP_UNDEFINED_ERROR;
  }

  out_tr = *ptr;

  if (out_tr == NULL || out_tr->orig_request == NULL || out_tr->last_response == NULL)
    return OSIP_BADPARAMETER;

  if (retry && (*retry >= 3))
    return OSIP_UNDEFINED_ERROR;

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

  osip_list_special_free (&msg->authorizations, (void (*)(void *)) &osip_authorization_free);
  osip_list_special_free (&msg->proxy_authorizations, (void (*)(void *)) &osip_proxy_authorization_free);

  if (out_tr != NULL && out_tr->last_response != NULL && (out_tr->last_response->status_code == 401 || out_tr->last_response->status_code == 407)) {
    i = _eXosip_add_authentication_information (excontext, msg, out_tr->last_response);
    if (i < 0) {
      osip_message_free (msg);
      return i;
    }
  }
  else {
    i = _eXosip_add_authentication_information (excontext, msg, NULL);
    if (i < 0) {
      osip_message_free (msg);
      return i;
    }
  }


  osip_message_force_update (msg);

  if (MSG_IS_INVITE (msg))
    i = _eXosip_transaction_init (excontext, &tr, ICT, excontext->j_osip, msg);
  else
    i = _eXosip_transaction_init (excontext, &tr, NICT, excontext->j_osip, msg);

  if (i != 0) {
    osip_message_free (msg);
    return i;
  }

  /* replace with the new tr */
  if (MSG_IS_PUBLISH (msg)) {
    /* old transaction is put in the garbage list */
    osip_list_add (&excontext->j_transactions, out_tr, 0);
    /* new transaction is put in the publish context */
    *ptr = tr;
  }
  else
    osip_list_add (&excontext->j_transactions, tr, 0);

  sipevent = osip_new_outgoing_sipmessage (msg);

  reserved1 = osip_transaction_get_reserved2 (out_tr);
  osip_transaction_set_reserved2 (out_tr, NULL);
  osip_transaction_set_reserved2 (tr, reserved1);

  reserved1 = osip_transaction_get_reserved3 (out_tr);
  osip_transaction_set_reserved3 (out_tr, NULL);
  osip_transaction_set_reserved3 (tr, reserved1);

  reserved1 = osip_transaction_get_reserved4 (out_tr);
  osip_transaction_set_reserved4 (out_tr, NULL);
  osip_transaction_set_reserved4 (tr, reserved1);

  reserved1 = osip_transaction_get_reserved5 (out_tr);
  osip_transaction_set_reserved5 (out_tr, NULL);
  osip_transaction_set_reserved5 (tr, reserved1);

  osip_transaction_add_event (tr, sipevent);

  if (retry)
    (*retry)++;

  _eXosip_update (excontext);   /* fixed? */
  _eXosip_wakeup (excontext);
  return OSIP_SUCCESS;
}

static int
_eXosip_publish_refresh (struct eXosip_t *excontext, eXosip_dialog_t * jd, osip_transaction_t ** ptr, int *retry)
{
  osip_transaction_t *out_tr = NULL;
  osip_transaction_t *tr = NULL;
  osip_message_t *msg = NULL;
  osip_event_t *sipevent;

  int cseq;
  osip_via_t *via;
  int i;

  if (!ptr)
    return OSIP_BADPARAMETER;

  if (jd != NULL) {
    if (jd->d_out_trs == NULL)
      return OSIP_BADPARAMETER;
  }

  out_tr = *ptr;

  if (out_tr == NULL || out_tr->orig_request == NULL || out_tr->last_response == NULL)
    return OSIP_BADPARAMETER;

  if (retry && (*retry >= 3))
    return OSIP_UNDEFINED_ERROR;

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

  if (out_tr != NULL && out_tr->last_response != NULL && (out_tr->last_response->status_code == 401 || out_tr->last_response->status_code == 407)) {
    _eXosip_add_authentication_information (excontext, msg, out_tr->last_response);
  }
  else
    _eXosip_add_authentication_information (excontext, msg, NULL);

  if (out_tr != NULL && out_tr->last_response != NULL && out_tr->last_response->status_code == 412) {
    /* remove SIP-If-Match header */
    osip_list_iterator_t it;
    osip_header_t* head = (osip_header_t*)osip_list_get_first(&msg->headers, &it);
    while (head != OSIP_SUCCESS) {

      if (head != NULL && 0 == osip_strcasecmp (head->hname, "sip-if-match")) {
        osip_list_iterator_remove (&it);
        osip_header_free (head);
        break;
      }
      head = (osip_header_t*)osip_list_get_next(&it);
    }
  }

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

  /* replace with the new tr */
  /* old transaction is put in the garbage list */
  osip_list_add (&excontext->j_transactions, out_tr, 0);
  /* new transaction is put in the publish context */
  *ptr = tr;

  sipevent = osip_new_outgoing_sipmessage (msg);

  osip_transaction_add_event (tr, sipevent);

  if (retry)
    (*retry)++;

  _eXosip_update (excontext);   /* fixed? */
  _eXosip_wakeup (excontext);
  return OSIP_SUCCESS;
}

static int
_eXosip_retry_register_with_auth (struct eXosip_t *excontext, eXosip_event_t * je)
{
  eXosip_reg_t *jr = NULL;
  int i;

  i = _eXosip_reg_find_id (excontext, &jr, je->rid);
  if (i < 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: registration not found\n"));
    return i;
  }

  if (jr->r_retry < 3) {
    jr->r_retry++;
    return eXosip_register_send_register (excontext, jr->r_id, NULL);
  }

  return OSIP_UNDEFINED_ERROR;
}

static int
_eXosip_retry_invite_with_auth (struct eXosip_t *excontext, eXosip_event_t * je)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  int *retry = NULL;
  osip_transaction_t *tr = NULL;
  int i;

  i = _eXosip_call_transaction_find (excontext, je->tid, &jc, &jd, &tr);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: call dialog not found\n"));
    return i;
  }

  if (jd && jd->d_dialog)
    retry = &jd->d_retry;
  else
    retry = &jc->c_retry;

  if (*retry < 3) {
    (*retry)++;
    return _eXosip_call_retry_request (excontext, jc, jd, tr);
  }
  return OSIP_UNDEFINED_ERROR;
}

static int
_eXosip_redirect_invite (struct eXosip_t *excontext, eXosip_event_t * je)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  osip_transaction_t *tr = NULL;
  int i;

  i = _eXosip_call_transaction_find (excontext, je->tid, &jc, &jd, &tr);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: call dialog not found\n"));
    return i;
  }

  return _eXosip_call_retry_request (excontext, jc, jd, tr);
}

static int
_eXosip_retry_subscribe_with_auth (struct eXosip_t *excontext, eXosip_event_t * je)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_subscribe_t *js = NULL;
  int *retry = NULL;
  osip_transaction_t *tr = NULL;
  int i;

  i = _eXosip_subscription_transaction_find (excontext, je->tid, &js, &jd, &tr);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: subscribe dialog not found\n"));
    return i;
  }

  if (jd && jd->d_dialog)
    retry = &jd->d_retry;
  else
    retry = &js->s_retry;

  if (*retry < 3) {
    (*retry)++;
    return _eXosip_subscription_send_request_with_credential (excontext, js, jd, tr);
  }
  return OSIP_UNDEFINED_ERROR;
}

static int
_eXosip_retry_publish_with_auth (struct eXosip_t *excontext, eXosip_event_t * je)
{
  eXosip_pub_t *jp = NULL;
  int i;

  i = _eXosip_pub_find_by_tid (excontext, &jp, je->tid);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: publish transaction not found\n"));
    return i;
  }

  return _eXosip_retry_with_auth (excontext, NULL, &jp->p_last_tr, NULL);
}

static int
_eXosip_retry_notify_with_auth (struct eXosip_t *excontext, eXosip_event_t * je)
{
  /* TODO untested */
  eXosip_dialog_t *jd = NULL;
  eXosip_notify_t *jn = NULL;
  osip_transaction_t *tr = NULL;
  int i;

  i = _eXosip_insubscription_transaction_find (excontext, je->tid, &jn, &jd, &tr);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: notify dialog not found\n"));
    return i;
  }

  return _eXosip_insubscription_send_request_with_credential (excontext, jn, jd, tr);

}

static int
eXosip_retry_with_auth (struct eXosip_t *excontext, eXosip_event_t * je)
{
  int res = OSIP_UNDEFINED_ERROR;

  if (!je || !je->request || !je->response)
    return OSIP_BADPARAMETER;

  if (je->rid > 0) {
    return _eXosip_retry_register_with_auth (excontext, je);
  }
  else if (je->cid > 0) {
    res = _eXosip_retry_invite_with_auth (excontext, je);
    if (res == OSIP_NOTFOUND) {
      osip_transaction_t *tr = NULL;

      _eXosip_transaction_find (excontext, je->tid, &tr);
      if (tr != NULL) {
        return _eXosip_retry_with_auth (excontext, NULL, &tr, NULL);
      }
    }
    return res;
  }
#ifndef MINISIZE
  else if (je->sid > 0) {
    return _eXosip_retry_subscribe_with_auth (excontext, je);
  }
  else if (je->nid > 0) {
    return _eXosip_retry_notify_with_auth (excontext, je);
  }
  else if (MSG_IS_PUBLISH (je->request))
    return _eXosip_retry_publish_with_auth (excontext, je);
#endif
  else {
    osip_transaction_t *tr = NULL;

    _eXosip_transaction_find (excontext, je->tid, &tr);
    if (tr != NULL) {
      return _eXosip_retry_with_auth (excontext, NULL, &tr, NULL);
    }
  }

  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Can't retry event %d with auth\n", je->type));
  return OSIP_UNDEFINED_ERROR;
}

static int
_eXosip_redirect (struct eXosip_t *excontext, eXosip_event_t * je)
{
  switch (je->type) {
  case EXOSIP_CALL_REDIRECTED:
    return _eXosip_redirect_invite (excontext, je);

  case EXOSIP_CALL_MESSAGE_REDIRECTED:
  case EXOSIP_MESSAGE_REDIRECTED:
  case EXOSIP_SUBSCRIPTION_REDIRECTED:
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: not implemented\n"));
    return OSIP_UNDEFINED_ERROR;

  default:
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Can't redirect event %d\n", je->type));
    return OSIP_UNDEFINED_ERROR;
  }
}

int
eXosip_default_action (struct eXosip_t *excontext, eXosip_event_t * je)
{
  if (!je || !je->response)
    return OSIP_BADPARAMETER;

  if (je->response->status_code == 401 || je->response->status_code == 407)
    return eXosip_retry_with_auth (excontext, je);
  else if (je->response->status_code >= 300 && je->response->status_code <= 399)
    return _eXosip_redirect (excontext, je);
  else
    return 1;
}

#endif

void
_eXosip_retransmit_lost200ok (struct eXosip_t *excontext)
{
  eXosip_call_t *jc;
  eXosip_dialog_t *jd;
  time_t now;

  now = osip_getsystemtime (NULL);

  for (jc = excontext->j_calls; jc != NULL; jc = jc->next) {
    if (jc->c_id >= 1 && jc->c_dialogs != NULL) {
      for (jd = jc->c_dialogs; jd != NULL; jd = jd->next) {
        if (jd->d_id >= 1 && jd->d_dialog != NULL && jd->d_200Ok != NULL) {
          if (jd->d_count == 9) {
            int i;

            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: no ACK received during 20s: dropping call\n"));
            /* hard for users to detect than I sent this BYE... */
            jd->d_count = 0;
            osip_message_free (jd->d_200Ok);
            jd->d_200Ok = NULL;
            i = eXosip_call_terminate (excontext, jc->c_id, jd->d_id);
            if (i == OSIP_SUCCESS) {
              _eXosip_report_call_event (excontext, EXOSIP_CALL_CLOSED, jc, jd, NULL);
            }
          }
          else if (jd->d_timer < now) {
            /* a dialog exist: retransmit lost 200ok */
            jd->d_count++;
            if (jd->d_count == 1)
              jd->d_timer = osip_getsystemtime (NULL) + 1;
            if (jd->d_count == 2)
              jd->d_timer = osip_getsystemtime (NULL) + 2;
            if (jd->d_count >= 3)
              jd->d_timer = osip_getsystemtime (NULL) + 4;
            jd = jc->c_dialogs;
            /* TU retransmission */
            _eXosip_snd_message (excontext, NULL, jd->d_200Ok, NULL, 0, -1);
          }
        }
      }
    }
  }
  return;
}

void
eXosip_automatic_action (struct eXosip_t *excontext)
{
  eXosip_call_t *jc;
  eXosip_dialog_t *jd;

#ifndef MINISIZE
  eXosip_subscribe_t *js;
  eXosip_notify_t *jn;
  eXosip_pub_t *jpub;
#endif

  eXosip_reg_t *jr;

  time_t now;

  now = osip_getsystemtime (NULL);

  for (jc = excontext->j_calls; jc != NULL; jc = jc->next) {
    if (jc->c_id < 1) {
    }
    else if (jc->c_dialogs == NULL || jc->c_dialogs->d_dialog == NULL) {
      /* an EARLY dialog may have failed with 401,407 or 3Xx */

      osip_transaction_t *out_tr = NULL;

      out_tr = jc->c_out_tr;

      if (out_tr != NULL
          && (out_tr->state == ICT_TERMINATED
              || out_tr->state == NICT_TERMINATED
              || out_tr->state == ICT_COMPLETED
              || out_tr->state == NICT_COMPLETED) && now - out_tr->birth_time < TRANSACTION_TIMEOUT_RETRY && out_tr->orig_request != NULL && out_tr->last_response != NULL && (out_tr->last_response->status_code == 401 || out_tr->last_response->status_code == 407)) {
        /* retry with credential */
        if (jc->c_retry < 3) {
          int i;

          i = _eXosip_call_retry_request (excontext, jc, NULL, out_tr);
          if (i != 0) {
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: could not clone msg for authentication\n"));
          }
          jc->c_retry++;
        }
      }
      else if (out_tr != NULL
               && (out_tr->state == ICT_TERMINATED
                   || out_tr->state == NICT_TERMINATED
                   || out_tr->state == ICT_COMPLETED || out_tr->state == NICT_COMPLETED) && now - out_tr->birth_time < TRANSACTION_TIMEOUT_RETRY && out_tr->orig_request != NULL && out_tr->last_response != NULL && out_tr->last_response->status_code == 422) {
        /* retry with higher Session-Expires / Min-SE */
        if (jc->c_retry < 3) {
          int i;

          i = _eXosip_call_retry_request (excontext, jc, NULL, out_tr);
          if (i != 0) {
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: could not clone msg for updating Session-Expires\n"));
          }
          jc->c_retry++;
        }
      }
      else if (out_tr != NULL
               && (out_tr->state == ICT_TERMINATED
                   || out_tr->state == NICT_TERMINATED
                   || out_tr->state == ICT_COMPLETED
                   || out_tr->state == NICT_COMPLETED) && now - out_tr->birth_time < TRANSACTION_TIMEOUT_RETRY && out_tr->orig_request != NULL && out_tr->last_response != NULL && (out_tr->last_response->status_code >= 300 && out_tr->last_response->status_code <= 399)) {
        /* retry with credential */
        int i;

        i = _eXosip_call_retry_request (excontext, jc, NULL, out_tr);
        if (i != 0) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: could not clone msg for redirection\n"));
        }
      }
    }

    for (jd = jc->c_dialogs; jd != NULL; jd = jd->next) {
      if (jd->d_dialog == NULL) {       /* finished call */
      }
      else {
        osip_transaction_t *out_tr = NULL;

        out_tr = osip_list_get (jd->d_out_trs, 0);
        if (out_tr == NULL)
          out_tr = jc->c_out_tr;

        if (out_tr != NULL
            && (out_tr->state == ICT_TERMINATED
                || out_tr->state == NICT_TERMINATED
                || out_tr->state == ICT_COMPLETED
                || out_tr->state == NICT_COMPLETED) && now - out_tr->birth_time < TRANSACTION_TIMEOUT_RETRY && out_tr->orig_request != NULL && out_tr->last_response != NULL && (out_tr->last_response->status_code == 401 || out_tr->last_response->status_code == 407)) {
          /* retry with credential */
          if (jd->d_retry < 3) {
            int i;

            i = _eXosip_call_retry_request (excontext, jc, jd, out_tr);
            if (i != 0) {
              OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: could not clone msg for authentication\n"));
            }
            jd->d_retry++;
          }
        }
        else if (out_tr != NULL
                 && (out_tr->state == ICT_TERMINATED
                     || out_tr->state == NICT_TERMINATED
                     || out_tr->state == ICT_COMPLETED || out_tr->state == NICT_COMPLETED) && now - out_tr->birth_time < TRANSACTION_TIMEOUT_RETRY && out_tr->orig_request != NULL && out_tr->last_response != NULL && out_tr->last_response->status_code == 422) {
          /* retry with higher Session-Expires / Min-SE */
          if (jd->d_retry < 3) {
            int i;

            i = _eXosip_call_retry_request (excontext, jc, jd, out_tr);
            if (i != 0) {
              OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: could not clone msg for updating Session-Expires\n"));
            }
            jd->d_retry++;
          }
        }
        else if (out_tr != NULL
                 && (out_tr->state == ICT_TERMINATED
                     || out_tr->state == NICT_TERMINATED
                     || out_tr->state == ICT_COMPLETED
                     || out_tr->state == NICT_COMPLETED) && now - out_tr->birth_time < TRANSACTION_TIMEOUT_RETRY && out_tr->orig_request != NULL && out_tr->last_response != NULL && (out_tr->last_response->status_code >= 300 && out_tr->last_response->status_code <= 399)) {
          /* retry with credential */
          int i;

          i = _eXosip_call_retry_request (excontext, jc, jd, out_tr);
          if (i != 0) {
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: could not clone msg for redirection\n"));
          }
        }
        else {
          /* check if we need session-timer */
          if (jd->d_refresher == -1) {
          }
          else if (jd->d_refresher == 0) {
            /* I'm the refresher */
            if (now - (jd->d_session_timer_start + jd->d_session_timer_length) > 0) {
              /* means we tried hard... but certainly failed... */
            }
            else if ((jd->d_session_timer_start + jd->d_session_timer_length) - now < (jd->d_session_timer_length / 2)) {
              osip_message_t *request = NULL;

              if (jd->d_session_timer_use_update == 1) {
                eXosip_call_build_request (excontext, jd->d_id, "UPDATE", &request);
                if (request != NULL) {
                  char session_exp[32];
                  int i;

                  memset (session_exp, 0, sizeof (session_exp));
                  snprintf (session_exp, sizeof (session_exp) - 1, "%i;refresher=uac", jd->d_session_timer_length);
                  osip_message_set_header (request, "Session-Expires", session_exp);
                  osip_message_set_supported (request, "timer");
                  i = eXosip_call_send_request (excontext, jd->d_id, request);
                  if (i == 0) {
                    /* update timer */
                    jd->d_session_timer_start = osip_getsystemtime (NULL) + jd->d_session_timer_length;
                  }
                }
              }
              else {
                eXosip_call_build_request (excontext, jd->d_id, "INVITE", &request);
                if (request != NULL) {
                  char session_exp[32];
                  int i;

                  memset (session_exp, 0, sizeof (session_exp));
                  snprintf (session_exp, sizeof (session_exp) - 1, "%i;refresher=uac", jd->d_session_timer_length);
                  osip_message_set_header (request, "Session-Expires", session_exp);
                  osip_message_set_supported (request, "timer");
                  i = eXosip_call_send_request (excontext, jd->d_id, request);
                  if (i == 0) {
                    /* update timer */
                    jd->d_session_timer_start = osip_getsystemtime (NULL) + jd->d_session_timer_length;
                  }
                }
              }
            }
          }
          else if (jd->d_refresher == 1) {
            /* remote is the refresher */
          }
        }
      }
    }
  }

  for (jr = excontext->j_reg; jr != NULL; jr = jr->next) {
    if (jr->r_id >= 1 && jr->r_last_tr != NULL) {
      if (jr->r_reg_period != 0 && now - jr->r_last_tr->birth_time > 900) {
        /* automatic refresh */
        eXosip_register_send_register (excontext, jr->r_id, NULL);
      }
      else if (jr->r_reg_period != 0 && (now - jr->r_last_tr->birth_time > jr->r_reg_period - (jr->r_reg_period / 10) || now - jr->r_last_tr->birth_time > jr->r_reg_period - 6)) {
        /* automatic refresh */
        eXosip_register_send_register (excontext, jr->r_id, NULL);
      }
      else if (jr->r_reg_period != 0 && now - jr->r_last_tr->birth_time > TRANSACTION_TIMEOUT_RETRY && (jr->r_last_tr->last_response == NULL || (!MSG_IS_STATUS_2XX (jr->r_last_tr->last_response)))) {
        /* automatic refresh */
        eXosip_register_send_register (excontext, jr->r_id, NULL);
      }
      else if (now - jr->r_last_tr->birth_time < 120 &&
               jr->r_last_tr->orig_request != NULL && (jr->r_last_tr->last_response != NULL && (jr->r_last_tr->last_response->status_code == 401 || jr->r_last_tr->last_response->status_code == 407
               || jr->r_last_tr->last_response->status_code == 423
               || jr->r_last_tr->last_response->status_code == 606))) {
        if (jr->r_retry < 3) {
          /* TODO: improve support for several retries when
             several credentials are needed */
          eXosip_register_send_register (excontext, jr->r_id, NULL);
          jr->r_retry++;
        }
      }
      else if (jr->registration_step==RS_DELETIONREQUIRED && jr->r_last_tr->orig_request != NULL && jr->r_last_tr->last_response != NULL && MSG_IS_STATUS_2XX (jr->r_last_tr->last_response)) {
        jr->registration_step=RS_DELETIONPROCEEDING;
        if (OSIP_SUCCESS!=eXosip_register_send_register (excontext, jr->r_id, NULL)) {
          jr->registration_step=RS_DELETIONREQUIRED;
        }
      }
      else if (jr->registration_step==RS_MASQUERADINGREQUIRED && jr->r_last_tr->orig_request != NULL && jr->r_last_tr->last_response != NULL && MSG_IS_STATUS_2XX (jr->r_last_tr->last_response)) {
        jr->registration_step=RS_MASQUERADINGPROCEEDING;
        if (OSIP_SUCCESS!=eXosip_register_send_register (excontext, jr->r_id, NULL)) {
          jr->registration_step=RS_MASQUERADINGREQUIRED;
        }
      }
    }

    if (jr->r_last_deletion!=0 && jr->r_last_deletion+60<now)
      jr->r_last_deletion=0; /* automasquerading may happen later than 1 minutes after previous one, to avoid loop (happens with bad NAT) */
  }

#ifndef MINISIZE

  for (js = excontext->j_subscribes; js != NULL; js = js->next) {
    if (js->s_id < 1) {
    }
    else if (js->s_dialogs == NULL) {
      osip_transaction_t *out_tr = NULL;

      out_tr = js->s_out_tr;

      if (out_tr != NULL
          && (out_tr->state == NICT_TERMINATED
              || out_tr->state == NICT_COMPLETED) &&
          now - out_tr->birth_time < TRANSACTION_TIMEOUT_RETRY && out_tr->orig_request != NULL && out_tr->last_response != NULL && (out_tr->last_response->status_code == 401 || out_tr->last_response->status_code == 407 || out_tr->last_response->status_code == 423)) {
        /* retry with credential */
        if (js->s_retry < 3) {
          int i;

          i = _eXosip_subscription_send_request_with_credential (excontext, js, NULL, out_tr);
          if (i != 0) {
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: could not clone msg for authentication\n"));
          }
          js->s_retry++;
        }
      }
    }

    for (jd = js->s_dialogs; jd != NULL; jd = jd->next) {
      if (jd->d_dialog != NULL) {       /* finished call */
        if (jd->d_id >= 1) {
          osip_transaction_t *out_tr = NULL;

          out_tr = osip_list_get (jd->d_out_trs, 0);
          if (out_tr == NULL)
            out_tr = js->s_out_tr;

          if (out_tr != NULL
              && (out_tr->state == NICT_TERMINATED
                  || out_tr->state == NICT_COMPLETED) && now - out_tr->birth_time < TRANSACTION_TIMEOUT_RETRY && out_tr->orig_request != NULL && out_tr->last_response != NULL && (out_tr->last_response->status_code == 401 || out_tr->last_response->status_code == 407)) {
            /* retry with credential */
            if (jd->d_retry < 3) {
              int i;

              i = _eXosip_subscription_send_request_with_credential (excontext, js, jd, out_tr);
              if (i != 0) {
                OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: could not clone suscbribe for authentication\n"));
              }
              jd->d_retry++;
            }
          }
          else if (js->s_reg_period == 0 || out_tr == NULL) {
          }
          else if ((out_tr->state == NICT_TERMINATED || out_tr->state == NICT_COMPLETED) && out_tr->orig_request != NULL && out_tr->last_response != NULL && (out_tr->last_response->status_code >= 300)) {
            /* refresh are not authorized after an error */
          }
          else if ((out_tr->state == NICT_TERMINATED || out_tr->state == NICT_COMPLETED) && (now - out_tr->birth_time > js->s_reg_period - (js->s_reg_period / 10) || now - out_tr->birth_time > js->s_reg_period - 6)) {       /* will expires in js->s_reg_period/10 sec OR 6 seconds: send refresh! */
            int i;
            if (out_tr->orig_request != NULL && MSG_IS_REFER(out_tr->orig_request)) {
                OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: subscription for REFER is expired\n"));
            } else {
              i = _eXosip_subscription_automatic_refresh (excontext, js, jd, out_tr);
              if (i != 0) {
                OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: could not clone subscribe for refresh\n"));
              }
            }
          }
        }
      }
    }
  }


  for (jn = excontext->j_notifies; jn != NULL; jn = jn->next) {
    for (jd = jn->n_dialogs; jd != NULL; jd = jd->next) {
      if (jd->d_dialog != NULL) {       /* finished call */
        if (jd->d_id >= 1) {
          osip_transaction_t *out_tr = NULL;

          out_tr = osip_list_get (jd->d_out_trs, 0);

          if (out_tr != NULL
              && (out_tr->state == NICT_TERMINATED
                  || out_tr->state == NICT_COMPLETED) && now - out_tr->birth_time < TRANSACTION_TIMEOUT_RETRY && out_tr->orig_request != NULL && out_tr->last_response != NULL && (out_tr->last_response->status_code == 401 || out_tr->last_response->status_code == 407)) {
            /* retry with credential */
            if (jd->d_retry < 3) {
              int i;

              i = _eXosip_insubscription_send_request_with_credential (excontext, jn, jd, out_tr);
              if (i != 0) {
                OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: could not clone notify for authentication\n"));
              }
              jd->d_retry++;
            }
          }
        }
      }
    }
  }

  for (jpub = excontext->j_pub; jpub != NULL; jpub = jpub->next) {
    if (jpub->p_id >= 1 && jpub->p_last_tr != NULL) {
      if (jpub->p_period != 0 && now - jpub->p_last_tr->birth_time > 900) {
        /* automatic refresh */
        _eXosip_publish_refresh (excontext, NULL, &jpub->p_last_tr, NULL);
      }
      else if (jpub->p_period != 0 && now - jpub->p_last_tr->birth_time > jpub->p_period - (jpub->p_period / 10)) {
        /* automatic refresh */
        _eXosip_publish_refresh (excontext, NULL, &jpub->p_last_tr, NULL);
      }
      else if (jpub->p_period != 0 && now - jpub->p_last_tr->birth_time > 120 && (jpub->p_last_tr->last_response == NULL || (!MSG_IS_STATUS_2XX (jpub->p_last_tr->last_response)))) {
        /* automatic refresh */
        _eXosip_publish_refresh (excontext, NULL, &jpub->p_last_tr, NULL);
      }
      else if (now - jpub->p_last_tr->birth_time < 120 && jpub->p_last_tr->orig_request != NULL && (jpub->p_last_tr->last_response != NULL && (jpub->p_last_tr->last_response->status_code == 401 || jpub->p_last_tr->last_response->status_code == 407))) {
        if (jpub->p_retry < 3) {
          /* TODO: improve support for several retries when
             several credentials are needed */
          _eXosip_retry_with_auth (excontext, NULL, &jpub->p_last_tr, NULL);
          jpub->p_retry++;
        }
      }
      else if (now - jpub->p_last_tr->birth_time < 120 && jpub->p_last_tr->orig_request != NULL && (jpub->p_last_tr->last_response != NULL && (jpub->p_last_tr->last_response->status_code == 412 || jpub->p_last_tr->last_response->status_code == 423))) {
        _eXosip_publish_refresh (excontext, NULL, &jpub->p_last_tr, NULL);
      }
    }
  }
#endif

}

void
_eXosip_update (struct eXosip_t *excontext)
{
  static int static_id = 1;
  eXosip_call_t *jc;

#ifndef MINISIZE
  eXosip_subscribe_t *js;
  eXosip_notify_t *jn;
#endif
  eXosip_dialog_t *jd;

  if (static_id == INT_MAX)
    static_id = 1;              /* loop */

  for (jc = excontext->j_calls; jc != NULL; jc = jc->next) {
    if (jc->c_id < 1) {
      jc->c_id = static_id;
      static_id++;
    }
    for (jd = jc->c_dialogs; jd != NULL; jd = jd->next) {
      if (jd->d_dialog != NULL) {       /* finished call */
        if (jd->d_id < 1) {
          jd->d_id = static_id;
          static_id++;
        }
      }
      else
        jd->d_id = -1;
    }
  }

#ifndef MINISIZE
  for (js = excontext->j_subscribes; js != NULL; js = js->next) {
    if (js->s_id < 1) {
      js->s_id = static_id;
      static_id++;
    }
    for (jd = js->s_dialogs; jd != NULL; jd = jd->next) {
      if (jd->d_dialog != NULL) {       /* finished call */
        if (jd->d_id < 1) {
          jd->d_id = static_id;
          static_id++;
        }
      }
      else
        jd->d_id = -1;
    }
  }

  for (jn = excontext->j_notifies; jn != NULL; jn = jn->next) {
    if (jn->n_id < 1) {
      jn->n_id = static_id;
      static_id++;
    }
    for (jd = jn->n_dialogs; jd != NULL; jd = jd->next) {
      if (jd->d_dialog != NULL) {       /* finished call */
        if (jd->d_id < 1) {
          jd->d_id = static_id;
          static_id++;
        }
      }
      else
        jd->d_id = -1;
    }
  }
#endif
}

static jauthinfo_t *
eXosip_find_authentication_info (struct eXosip_t *excontext, const char *username, const char *realm)
{
  jauthinfo_t *fallback = NULL;
  jauthinfo_t *authinfo;

  for (authinfo = excontext->authinfos; authinfo != NULL; authinfo = authinfo->next) {
    if (realm != NULL)
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO3, NULL, "INFO: authinfo: %s %s\n", realm, authinfo->realm));
    if (0 == osip_strcasecmp (authinfo->username, username)) {
      if (authinfo->realm[0] == '\0') {
        fallback = authinfo;
      }
      else if (realm == NULL || osip_strcasecmp (realm, authinfo->realm) == 0 || 0 == osip_strncasecmp (realm + 1, authinfo->realm, strlen (realm) - 2)) {
        return authinfo;
      }
    }
  }

  /* no matching username has been found for this realm,
     try with another username... */
  for (authinfo = excontext->authinfos; authinfo != NULL; authinfo = authinfo->next) {
    if (realm != NULL)
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO3, NULL, "INFO: authinfo: %s %s\n", realm, authinfo->realm));
    if (authinfo->realm[0] == '\0'
        && fallback == NULL) {
      fallback = authinfo;
    }
    else if (realm == NULL || osip_strcasecmp (realm, authinfo->realm) == 0 || 0 == osip_strncasecmp (realm + 1, authinfo->realm, strlen (realm) - 2)) {
      return authinfo;
    }
  }

  return fallback;
}

int
eXosip_clear_authentication_info (struct eXosip_t *excontext)
{
  jauthinfo_t *jauthinfo;

  for (jauthinfo = excontext->authinfos; jauthinfo != NULL; jauthinfo = excontext->authinfos) {
    REMOVE_ELEMENT (excontext->authinfos, jauthinfo);
    osip_free (jauthinfo);
  }
  return OSIP_SUCCESS;
}

int
eXosip_add_authentication_info (struct eXosip_t *excontext, const char *username, const char *userid, const char *passwd, const char *ha1, const char *realm)
{
  jauthinfo_t *authinfos;

  if (username == NULL || username[0] == '\0')
    return OSIP_BADPARAMETER;
  if (userid == NULL || userid[0] == '\0')
    return OSIP_BADPARAMETER;

  if (passwd != NULL && passwd[0] != '\0') {
  }
  else if (ha1 != NULL && ha1[0] != '\0') {
  }
  else
    return OSIP_BADPARAMETER;

  authinfos = (jauthinfo_t *) osip_malloc (sizeof (jauthinfo_t));
  if (authinfos == NULL)
    return OSIP_NOMEM;
  memset (authinfos, 0, sizeof (jauthinfo_t));

  eXosip_remove_authentication_info(excontext, username, realm);

  snprintf (authinfos->username, 50, "%s", username);
  snprintf (authinfos->userid, 50, "%s", userid);
  if (passwd != NULL && passwd[0] != '\0')
    snprintf (authinfos->passwd, 50, "%s", passwd);
  else if (ha1 != NULL && ha1[0] != '\0')
    snprintf (authinfos->ha1, 50, "%s", ha1);
  if (realm != NULL && realm[0] != '\0')
    snprintf (authinfos->realm, 50, "%s", realm);

  ADD_ELEMENT (excontext->authinfos, authinfos);
  return OSIP_SUCCESS;
}

int
eXosip_remove_authentication_info (struct eXosip_t *excontext, const char *username, const char *realm)
{
  jauthinfo_t *authinfo;

  if (username == NULL || username[0] == '\0')
    return OSIP_BADPARAMETER;

  for (authinfo = excontext->authinfos; authinfo != NULL; authinfo = authinfo->next) {
    if (osip_strcasecmp (username, authinfo->username) == 0) {
      if (realm != NULL && osip_strcasecmp (realm, authinfo->realm) != 0)
        continue;
      if (realm == NULL && authinfo->realm[0] != '\0')
        continue;
      REMOVE_ELEMENT (excontext->authinfos, authinfo);
      osip_free (authinfo);
      return OSIP_SUCCESS;
    }
  }
  return OSIP_NOTFOUND;
}

int
_eXosip_add_authentication_information (struct eXosip_t *excontext, osip_message_t * req, osip_message_t * last_response)
{
  osip_authorization_t *aut = NULL;
  osip_www_authenticate_t *wwwauth = NULL;
  osip_proxy_authorization_t *proxy_aut = NULL;
  osip_proxy_authenticate_t *proxyauth = NULL;
  jauthinfo_t *authinfo = NULL;
  int pos;
  int i;

  if (req == NULL || req->from == NULL || req->from->url == NULL || req->from->url->username == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "authinfo: Invalid message\n"));
    return OSIP_BADPARAMETER;
  }

  if (last_response == NULL) {
    /* we can add all credential that belongs to the same call-id */
    struct eXosip_http_auth *http_auth;

    /* update entries with same call_id */
    for (pos = 0; pos < MAX_EXOSIP_HTTP_AUTH; pos++) {
      http_auth = &excontext->http_auths[pos];
      if (http_auth->pszCallId[0] == '\0')
        continue;
      if (osip_strcasecmp (http_auth->pszCallId, req->call_id->number) == 0) {
        char *uri;

        authinfo = eXosip_find_authentication_info (excontext, req->from->url->username, http_auth->wa->realm);
        if (authinfo == NULL) {
          if (http_auth->wa->realm != NULL)
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "authinfo: No authentication found for %s %s\n", req->from->url->username, http_auth->wa->realm));
          return OSIP_NOTFOUND;
        }

        i = osip_uri_to_str (req->req_uri, &uri);
        if (i != 0)
          return i;

        http_auth->iNonceCount++;
        i = _eXosip_create_proxy_authorization_header (http_auth->wa, uri, authinfo->userid, authinfo->passwd, authinfo->ha1, &aut, req->sip_method, http_auth->pszCNonce, http_auth->iNonceCount);

        osip_free (uri);
        if (i != 0)
          return i;

        if (aut != NULL) {
          if (http_auth->answer_code == 401)
            osip_list_add (&req->authorizations, aut, -1);
          else
            osip_list_add (&req->proxy_authorizations, aut, -1);
          osip_message_force_update (req);
        }
      }
    }
    return OSIP_SUCCESS;
  }

  pos = 0;
  osip_message_get_www_authenticate (last_response, pos, &wwwauth);
  osip_message_get_proxy_authenticate (last_response, pos, &proxyauth);
  if (wwwauth == NULL && proxyauth == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "authinfo: No WWW-Authenticate or Proxy-Authenticate\n"));
    return OSIP_SYNTAXERROR;
  }

  while (wwwauth != NULL) {
    char *uri;

    authinfo = eXosip_find_authentication_info (excontext, req->from->url->username, wwwauth->realm);
    if (authinfo == NULL) {
      if (wwwauth->realm != NULL)
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "authinfo: No authentication found for %s %s\n", req->from->url->username, wwwauth->realm));
      return OSIP_NOTFOUND;
    }

    i = osip_uri_to_str (req->req_uri, &uri);
    if (i != 0)
      return i;

    i = _eXosip_create_proxy_authorization_header (wwwauth, uri, authinfo->userid, authinfo->passwd, authinfo->ha1, &aut, req->sip_method, "0a4f113b", 1);
    osip_free (uri);
    if (i != 0)
      return i;

    if (aut != NULL) {
      osip_list_add (&req->authorizations, aut, -1);
      osip_message_force_update (req);
    }
#if defined(AVOID_REFRESH_WITHOUT_CREDENTIAL)
    if (wwwauth->qop_options != NULL) {
#endif
      if (osip_strcasecmp (req->sip_method, "REGISTER") == 0 || osip_strcasecmp (req->sip_method, "INVITE") == 0 || osip_strcasecmp (req->sip_method, "SUBSCRIBE") == 0|| osip_strcasecmp (req->sip_method, "REFER") == 0)
        _eXosip_store_nonce (excontext, req->call_id->number, wwwauth, 401);
      else {
        osip_generic_param_t *to_tag = NULL;

        osip_from_param_get_byname (req->to, "tag", &to_tag);
        if (to_tag != NULL) {
          /* if message is part of a dialog */
          _eXosip_store_nonce (excontext, req->call_id->number, wwwauth, 401);
        }
      }
#if defined(AVOID_REFRESH_WITHOUT_CREDENTIAL)
    }
#endif

    pos++;
    osip_message_get_www_authenticate (last_response, pos, &wwwauth);
  }

  pos = 0;
  while (proxyauth != NULL) {
    char *uri;

    authinfo = eXosip_find_authentication_info (excontext, req->from->url->username, proxyauth->realm);
    if (authinfo == NULL) {
      if (proxyauth->realm != NULL)
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "authinfo: No authentication found for %s %s\n", req->from->url->username, proxyauth->realm));
      return OSIP_NOTFOUND;
    }
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "authinfo: %s\n", authinfo->username));
    i = osip_uri_to_str (req->req_uri, &uri);
    if (i != 0)
      return i;

    i = _eXosip_create_proxy_authorization_header (proxyauth, uri, authinfo->userid, authinfo->passwd, authinfo->ha1, &proxy_aut, req->sip_method, "0a4f113b", 1);
    osip_free (uri);
    if (i != 0)
      return i;

    if (proxy_aut != NULL) {
      osip_list_add (&req->proxy_authorizations, proxy_aut, -1);
      osip_message_force_update (req);
    }
#if defined(AVOID_REFRESH_WITHOUT_CREDENTIAL)
    if (proxyauth->qop_options != NULL) {
#endif
      if (osip_strcasecmp (req->sip_method, "REGISTER") == 0 || osip_strcasecmp (req->sip_method, "INVITE") == 0 || osip_strcasecmp (req->sip_method, "SUBSCRIBE") == 0|| osip_strcasecmp (req->sip_method, "REFER") == 0)
        _eXosip_store_nonce (excontext, req->call_id->number, proxyauth, 407);
      else {
        osip_generic_param_t *to_tag = NULL;

        osip_from_param_get_byname (req->to, "tag", &to_tag);
        if (to_tag != NULL) {
          /* if message is part of a dialog */
          _eXosip_store_nonce (excontext, req->call_id->number, proxyauth, 407);
        }
      }
#if defined(AVOID_REFRESH_WITHOUT_CREDENTIAL)
    }
#endif

    pos++;
    osip_message_get_proxy_authenticate (last_response, pos, &proxyauth);
  }

  return OSIP_SUCCESS;
}

int
_eXosip_update_top_via (struct eXosip_t *excontext, osip_message_t * sip)
{
  unsigned int number;
  char tmp[40];
  osip_generic_param_t *br = NULL;
  osip_via_t *via = (osip_via_t *) osip_list_get (&sip->vias, 0);

  if (via == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "missing via in SIP message\n"));
    return OSIP_SYNTAXERROR;
  }

  osip_free(via->host);
  /* special values to be replaced in transport layer (eXtl_*.c files) */
  via->host = osip_strdup("999.999.999.999");
  if (via->host==NULL)
    return OSIP_NOMEM;

  /* browse parameter and replace "branch" */
  osip_via_param_get_byname (via, "branch", &br);

  if (br == NULL || br->gvalue == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "missing branch parameter via in SIP message\n"));
    return OSIP_SYNTAXERROR;
  }

  osip_free (br->gvalue);
  number = osip_build_random_number ();

  snprintf (tmp, 40, "z9hG4bK%u", number);
  br->gvalue = osip_strdup (tmp);
  return OSIP_SUCCESS;
}

/* vivox: mark all registrations as needing refreshing */
void
_eXosip_mark_all_registrations_expired (struct eXosip_t *excontext)
{
  eXosip_reg_t *jr;
  int wakeup = 0;

  for (jr = excontext->j_reg; jr != NULL; jr = jr->next) {
    if (jr->r_id >= 1 && jr->r_last_tr != NULL) {
      jr->r_last_tr->birth_time -= jr->r_reg_period;
      wakeup = 1;
    }
  }
  if (wakeup) {
    _eXosip_wakeup (excontext);
  }
}

void
_eXosip_mark_registration_expired (struct eXosip_t *excontext, const char *call_id)
{
  eXosip_reg_t *jr;
  int wakeup = 0;

  for (jr = excontext->j_reg; jr != NULL; jr = jr->next) {
    if (jr->r_id < 1 || jr->r_last_tr == NULL)
      continue;
    if (jr->r_last_tr->orig_request==NULL || jr->r_last_tr->orig_request->call_id==NULL || jr->r_last_tr->orig_request->call_id->number==NULL)
      continue;
    if (osip_strcasecmp(jr->r_last_tr->orig_request->call_id->number, call_id)==0) {
      time_t now;
      now = osip_getsystemtime (NULL);
      if (jr->r_reg_period - (jr->r_reg_period / 10)>TRANSACTION_TIMEOUT_RETRY)
        jr->r_last_tr->birth_time = now-120;
      else
        jr->r_last_tr->birth_time = jr->r_reg_period - (jr->r_reg_period / 10);
      if (jr->r_retryfailover<60)
        jr->r_retryfailover++;
      jr->r_last_tr->birth_time+=+jr->r_retryfailover; /* wait "RETRY" (counter) seconds before retrying: avoid flooding */
      wakeup = 1;
    }
  }
  if (wakeup) {
    _eXosip_wakeup (excontext);
  }
}

int
_eXosip_check_allow_header (eXosip_dialog_t * jd, osip_message_t * message)
{
#ifndef MINISIZE
  osip_list_iterator_t it;
  osip_allow_t* dest = (osip_allow_t*)osip_list_get_first(&message->allows, &it);
  while (dest != NULL) {
    if (dest->value != NULL && osip_strcasecmp (dest->value, "update") == 0) {
      jd->d_session_timer_use_update = 1;
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Allow header contains UPDATE\n"));
      break;
    }
    dest = (osip_allow_t*)osip_list_get_next(&it);
  }
#else
  osip_list_iterator_t it;
  osip_header_t* dest = (osip_header_t*)osip_list_get_first(&message->headers, &it);
  while (dest != OSIP_SUCCESS) {
    if (dest->hvalue != NULL && osip_strcasecmp (dest->hname, "allow") == 0) {
      if (osip_strcasestr (dest->hvalue, "UPDATE") != NULL) {
        jd->d_session_timer_use_update = 1;
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Allow header contains UPDATE\n"));
        break;
      }
    }
    dest = (osip_header_t*)osip_list_get_next(&it);
  }
#endif
  return 0;
}
