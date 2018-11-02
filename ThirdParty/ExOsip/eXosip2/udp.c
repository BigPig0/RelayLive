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
#include <eXosip2/eXosip.h>

#ifndef WIN32
#ifdef __APPLE_CC__
#include <unistd.h>
#endif
#else
#include <windows.h>
#endif

#if !defined(_WIN32_WCE)
#include <errno.h>
#endif

#ifdef TSC_SUPPORT
#include "tsc_socket_api.h"
#include "tsc_control_api.h"
#endif


static void
_eXosip_send_default_answer (struct eXosip_t *excontext, eXosip_dialog_t * jd, osip_transaction_t * transaction, osip_event_t * evt, int status, char *reason_phrase, char *warning, int line)
{
  osip_event_t *evt_answer;
  osip_message_t *answer;
  int i;

  osip_transaction_set_reserved2 (transaction, NULL);

  /* THIS METHOD DOES NOT ACCEPT STATUS CODE BETWEEN 101 and 299 */
  if (status > 100 && status < 299 && MSG_IS_INVITE (evt->sip))
    return;

  if (jd != NULL)
    i = _eXosip_build_response_default (excontext, &answer, jd->d_dialog, status, evt->sip);
  else
    i = _eXosip_build_response_default (excontext, &answer, NULL, status, evt->sip);

  if (i != 0 || answer == NULL) {
    return;
  }

  if (reason_phrase != NULL) {
    char *_reason;

    _reason = osip_message_get_reason_phrase (answer);
    if (_reason != NULL)
      osip_free (_reason);
    _reason = osip_strdup (reason_phrase);
    osip_message_set_reason_phrase (answer, _reason);
  }

  osip_message_set_content_length (answer, "0");

  if (status == 500)
    osip_message_set_retry_after (answer, "10");

  evt_answer = osip_new_outgoing_sipmessage (answer);
  evt_answer->transactionid = transaction->transactionid;
  osip_transaction_add_event (transaction, evt_answer);
  _eXosip_wakeup (excontext);

}

static void
_eXosip_process_bye (struct eXosip_t *excontext, eXosip_call_t * jc, eXosip_dialog_t * jd, osip_transaction_t * transaction, osip_event_t * evt)
{
  osip_event_t *evt_answer;
  osip_message_t *answer;
  int i;

  osip_transaction_set_reserved2 (transaction, jc);

  if (excontext->autoanswer_bye == 0) {
    /* let the app build and send answers */
    osip_transaction_set_reserved3 (transaction, jd);
    osip_list_add (jd->d_inc_trs, transaction, 0);
    _eXosip_wakeup (excontext);
    return;
  }

  i = _eXosip_build_response_default (excontext, &answer, jd->d_dialog, 200, evt->sip);
  if (i != 0) {
    osip_list_add (&excontext->j_transactions, transaction, 0);
    return;
  }
  osip_message_set_content_length (answer, "0");

  evt_answer = osip_new_outgoing_sipmessage (answer);
  evt_answer->transactionid = transaction->transactionid;

  osip_list_add (jd->d_inc_trs, transaction, 0);

  /* Release the eXosip_dialog */
  osip_dialog_free (jd->d_dialog);
  jd->d_dialog = NULL;

  osip_transaction_add_event (transaction, evt_answer);

  osip_nist_execute (excontext->j_osip);
  _eXosip_report_call_event (excontext, EXOSIP_CALL_MESSAGE_NEW, jc, jd, transaction);
  _eXosip_report_call_event (excontext, EXOSIP_CALL_CLOSED, jc, jd, transaction);
  _eXosip_update (excontext);   /* AMD 30/09/05 */

  _eXosip_wakeup (excontext);
}

static void
_eXosip_process_ack (struct eXosip_t *excontext, eXosip_call_t * jc, eXosip_dialog_t * jd, osip_event_t * evt)
{
  /* TODO: We should find the matching transaction for this ACK
     and also add the ACK in the event. */
  eXosip_event_t *je;
  int i;


  je = _eXosip_event_init_for_call (EXOSIP_CALL_ACK, jc, jd, NULL);
  if (je != NULL) {
    osip_transaction_t *tr;

    tr = _eXosip_find_last_inc_invite (jc, jd);
    if (tr != NULL) {
      je->tid = tr->transactionid;
      /* fill request and answer */
      if (tr->orig_request != NULL) {
        i = osip_message_clone (tr->orig_request, &je->request);
        if (i != 0) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "failed to clone request for event\n"));
        }
      }
      if (tr->last_response != NULL) {
        i = osip_message_clone (tr->last_response, &je->response);
        if (i != 0) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "failed to clone response for event\n"));
        }
      }
    }

    i = osip_message_clone (evt->sip, &je->ack);
    if (i != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "failed to clone ACK for event\n"));
    }
  }

  /* stop ACK retransmission, in case there is any */
  jd->d_count = 0;
  osip_message_free (jd->d_200Ok);
  jd->d_200Ok = NULL;

  if (je != NULL)
    _eXosip_report_event (excontext, je, NULL);

  osip_event_free (evt);
}

static int
_cancel_match_invite (osip_transaction_t * invite, osip_message_t * cancel)
{
  osip_generic_param_t *br;
  osip_generic_param_t *br2;
  osip_via_t *via;

  osip_via_param_get_byname (invite->topvia, "branch", &br);
  via = osip_list_get (&cancel->vias, 0);
  if (via == NULL)
    return OSIP_SYNTAXERROR;    /* request without via??? */
  osip_via_param_get_byname (via, "branch", &br2);
  if (br != NULL && br2 == NULL)
    return OSIP_UNDEFINED_ERROR;
  if (br2 != NULL && br == NULL)
    return OSIP_UNDEFINED_ERROR;
  if (br2 != NULL && br != NULL) {      /* compliant UA  :) */
    if (br->gvalue != NULL && br2->gvalue != NULL && 0 == strcmp (br->gvalue, br2->gvalue))
      return OSIP_SUCCESS;
    return OSIP_UNDEFINED_ERROR;
  }
  /* old backward compatibility mechanism */
  if (0 != osip_call_id_match (invite->callid, cancel->call_id))
    return OSIP_UNDEFINED_ERROR;
  if (0 != osip_to_tag_match (invite->to, cancel->to))
    return OSIP_UNDEFINED_ERROR;
  if (0 != osip_from_tag_match (invite->from, cancel->from))
    return OSIP_UNDEFINED_ERROR;
  if (0 != osip_via_match (invite->topvia, via))
    return OSIP_UNDEFINED_ERROR;
  return OSIP_SUCCESS;
}

static void
_eXosip_process_cancel (struct eXosip_t *excontext, osip_transaction_t * transaction, osip_event_t * evt)
{
  osip_transaction_t *tr;
  osip_event_t *evt_answer;
  osip_message_t *answer;
  int i;

  eXosip_call_t *jc;
  eXosip_dialog_t *jd;

  tr = NULL;
  jd = NULL;
  /* first, look for a Dialog in the map of element */
  for (jc = excontext->j_calls; jc != NULL; jc = jc->next) {
    if (jc->c_inc_tr != NULL) {
      i = _cancel_match_invite (jc->c_inc_tr, evt->sip);
      if (i == 0) {
        tr = jc->c_inc_tr;
        /* fixed */
        if (jc->c_dialogs != NULL)
          jd = jc->c_dialogs;
        break;
      }
    }
    tr = NULL;
    for (jd = jc->c_dialogs; jd != NULL; jd = jd->next) {
      osip_list_iterator_t it;
      tr = (osip_transaction_t*)osip_list_get_first(jd->d_inc_trs, &it);
      while (tr != OSIP_SUCCESS) {
        i = _cancel_match_invite (tr, evt->sip);
        if (i == 0)
          break;
        tr = (osip_transaction_t *)osip_list_get_next(&it);;
      }
      if (tr != NULL)
        break;
    }
    if (jd != NULL)
      break;                    /* tr has just been found! */
  }

  if (tr == NULL) {             /* we didn't found the transaction to cancel */
    i = _eXosip_build_response_default (excontext, &answer, NULL, 481, evt->sip);
    if (i != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot cancel transaction.\n"));
      /*BUG fixed 32/12/2010
         osip_list_add(&excontext->j_transactions, tr, 0);
         osip_transaction_set_reserved2(tr, NULL);
         replaced with */
      osip_list_add (&excontext->j_transactions, transaction, 0);
      osip_transaction_set_reserved2 (transaction, NULL);
      return;
    }
    osip_message_set_content_length (answer, "0");
    evt_answer = osip_new_outgoing_sipmessage (answer);
    evt_answer->transactionid = transaction->transactionid;
    osip_transaction_add_event (transaction, evt_answer);

    osip_list_add (&excontext->j_transactions, transaction, 0);
    osip_transaction_set_reserved2 (transaction, NULL);
    _eXosip_wakeup (excontext);
    return;
  }

  if (tr->state == IST_TERMINATED || tr->state == IST_CONFIRMED || tr->state == IST_COMPLETED) {
    /* I can't find the status code in the rfc?
       (I read I must answer 200? wich I found strange)
       I probably misunderstood it... and prefer to send 481
       as the transaction has been answered. */
    if (jd == NULL)
      i = _eXosip_build_response_default (excontext, &answer, NULL, 481, evt->sip);
    else
      i = _eXosip_build_response_default (excontext, &answer, jd->d_dialog, 481, evt->sip);
    if (i != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot cancel transaction.\n"));
      /*BUG fixed 32/12/2010
         osip_list_add(&excontext->j_transactions, tr, 0);
         osip_transaction_set_reserved2(tr, NULL);
         replaced with */
      osip_list_add (&excontext->j_transactions, transaction, 0);
      osip_transaction_set_reserved2 (transaction, NULL);
      return;
    }
    osip_message_set_content_length (answer, "0");
    evt_answer = osip_new_outgoing_sipmessage (answer);
    evt_answer->transactionid = transaction->transactionid;
    osip_transaction_add_event (transaction, evt_answer);

    if (jd != NULL)
      osip_list_add (jd->d_inc_trs, transaction, 0);
    else
      osip_list_add (&excontext->j_transactions, transaction, 0);
    osip_transaction_set_reserved2 (transaction, NULL);
    _eXosip_wakeup (excontext);

    return;
  }

  {
    if (jd == NULL)
      i = _eXosip_build_response_default (excontext, &answer, NULL, 200, evt->sip);
    else
      i = _eXosip_build_response_default (excontext, &answer, jd->d_dialog, 200, evt->sip);
    if (i != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot cancel transaction.\n"));
      /*BUG fixed 32/12/2010
         osip_list_add(&excontext->j_transactions, tr, 0);
         osip_transaction_set_reserved2(tr, NULL);
         replaced with */
      osip_list_add (&excontext->j_transactions, transaction, 0);
      osip_transaction_set_reserved2 (transaction, NULL);
      return;
    }
    osip_message_set_content_length (answer, "0");
    evt_answer = osip_new_outgoing_sipmessage (answer);
    evt_answer->transactionid = transaction->transactionid;
    osip_transaction_add_event (transaction, evt_answer);
    _eXosip_wakeup (excontext);

    if (jd != NULL)
      osip_list_add (jd->d_inc_trs, transaction, 0);
    else
      osip_list_add (&excontext->j_transactions, transaction, 0);

    osip_transaction_set_reserved2 (transaction, jc);
    osip_transaction_set_reserved3 (transaction, jd);

    /* answer transaction to cancel */
    if (jd == NULL)
      i = _eXosip_build_response_default (excontext, &answer, NULL, 487, tr->orig_request);
    else
      i = _eXosip_build_response_default (excontext, &answer, jd->d_dialog, 487, tr->orig_request);
    if (i != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot cancel transaction.\n"));
      /*BUG fixed 32/12/2010
         osip_list_add(&excontext->j_transactions, tr, 0);
         osip_transaction_set_reserved2(tr, NULL); */
      return;
    }
    osip_message_set_content_length (answer, "0");
    evt_answer = osip_new_outgoing_sipmessage (answer);
    evt_answer->transactionid = tr->transactionid;
    osip_transaction_add_event (tr, evt_answer);
    _eXosip_wakeup (excontext);
  }
}

static void
_eXosip_process_reinvite (struct eXosip_t *excontext, eXosip_call_t * jc, eXosip_dialog_t * jd, osip_transaction_t * transaction, osip_event_t * evt)
{
  osip_transaction_set_reserved2 (transaction, jc);
  osip_transaction_set_reserved3 (transaction, jd);

  osip_list_add (jd->d_inc_trs, transaction, 0);
  osip_ist_execute (excontext->j_osip);
  _eXosip_report_call_event (excontext, EXOSIP_CALL_REINVITE, jc, jd, transaction);
}

static void
_eXosip_process_new_invite (struct eXosip_t *excontext, osip_transaction_t * transaction, osip_event_t * evt)
{
  osip_event_t *evt_answer;
  int i;
  eXosip_call_t *jc;
  eXosip_dialog_t *jd;
  osip_message_t *answer;
  osip_generic_param_t *to_tag = NULL;

  if (evt->sip != NULL && evt->sip->to != NULL)
    osip_from_param_get_byname (evt->sip->to, "tag", &to_tag);

  if (to_tag != NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "ERROR: Existing To-Tag in new INVITE -> reject with 481\n"));
    i = _eXosip_build_response_default (excontext, &answer, NULL, 481, evt->sip);
    if (i == 0) {
      evt_answer = osip_new_outgoing_sipmessage (answer);
      evt_answer->transactionid = transaction->transactionid;
      _eXosip_update (excontext);
      osip_transaction_add_event (transaction, evt_answer);
      return;
    }
    osip_message_set_content_length (answer, "0");
    osip_list_add (&excontext->j_transactions, transaction, 0);
    osip_transaction_set_reserved2 (transaction, NULL);
    return;
  }

  _eXosip_call_init (excontext, &jc);

  ADD_ELEMENT (excontext->j_calls, jc);

  i = _eXosip_build_response_default (excontext, &answer, NULL, 101, evt->sip);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot create dialog."));
    osip_list_add (&excontext->j_transactions, transaction, 0);
    osip_transaction_set_reserved2 (transaction, NULL);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "ERROR: Could not create response for invite\n"));
    return;
  }
  osip_message_set_content_length (answer, "0");
  i = _eXosip_complete_answer_that_establish_a_dialog (excontext, answer, evt->sip);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot complete answer!\n"));
    osip_list_add (&excontext->j_transactions, transaction, 0);
    osip_transaction_set_reserved2 (transaction, NULL);
    osip_message_free (answer);
    return;
  }

  i = _eXosip_dialog_init_as_uas (&jd, evt->sip, answer);
  osip_message_free (answer);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot create dialog!\n"));
    osip_list_add (&excontext->j_transactions, transaction, 0);
    osip_transaction_set_reserved2 (transaction, NULL);
    return;
  }

  _eXosip_check_allow_header (jd, evt->sip);

  ADD_ELEMENT (jc->c_dialogs, jd);

  osip_transaction_set_reserved2 (transaction, jc);
  osip_transaction_set_reserved3 (transaction, jd);

  _eXosip_update (excontext);
  jc->c_inc_tr = transaction;

  /* be sure the invite will be processed
     before any API call on this dialog */
  osip_ist_execute (excontext->j_osip);

  if (transaction->orig_request != NULL) {
    _eXosip_report_call_event (excontext, EXOSIP_CALL_INVITE, jc, jd, transaction);
  }

  _eXosip_wakeup (excontext);

}

#ifndef MINISIZE

static void
_eXosip_process_new_subscription (struct eXosip_t *excontext, osip_transaction_t * transaction, osip_event_t * evt)
{
  osip_event_t *evt_answer;
  eXosip_notify_t *jn;
  eXosip_dialog_t *jd;
  osip_message_t *answer;
  int i;
  osip_generic_param_t *to_tag = NULL;

  if (evt->sip != NULL && evt->sip->to != NULL)
    osip_from_param_get_byname (evt->sip->to, "tag", &to_tag);

  if (to_tag != NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "ERROR: Existing To-Tag in new SUBSCRIBE/REFER -> reject with 481\n"));
    i = _eXosip_build_response_default (excontext, &answer, NULL, 481, evt->sip);
    if (i == 0) {
      evt_answer = osip_new_outgoing_sipmessage (answer);
      evt_answer->transactionid = transaction->transactionid;
      _eXosip_update (excontext);
      osip_message_set_content_length (answer, "0");
      osip_transaction_add_event (transaction, evt_answer);
    }
    osip_list_add (&excontext->j_transactions, transaction, 0);
    osip_transaction_set_reserved2 (transaction, NULL);
    return;
  }

  i = _eXosip_notify_init (excontext, &jn, evt->sip);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "ERROR: missing contact or memory\n"));
    i = _eXosip_build_response_default (excontext, &answer, NULL, 400, evt->sip);
    if (i == 0) {
      evt_answer = osip_new_outgoing_sipmessage (answer);
      evt_answer->transactionid = transaction->transactionid;
      _eXosip_update (excontext);
      osip_message_set_content_length (answer, "0");
      osip_transaction_add_event (transaction, evt_answer);
    }
    osip_list_add (&excontext->j_transactions, transaction, 0);
    osip_transaction_set_reserved2 (transaction, NULL);
    return;
  }
  _eXosip_notify_set_refresh_interval (jn, evt->sip);

  i = _eXosip_build_response_default (excontext, &answer, NULL, 101, evt->sip);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "ERROR: Could not create response for SUBSCRIBE/REFER\n"));
    osip_list_add (&excontext->j_transactions, transaction, 0);
    _eXosip_notify_free (excontext, jn);
    return;
  }
  i = _eXosip_complete_answer_that_establish_a_dialog (excontext, answer, evt->sip);
  if (i != 0) {
    osip_message_free (answer);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot complete answer!\n"));
    osip_list_add (&excontext->j_transactions, transaction, 0);
    _eXosip_notify_free (excontext, jn);
    return;
  }

  i = _eXosip_dialog_init_as_uas (&jd, evt->sip, answer);
  if (i != 0) {
    osip_message_free (answer);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot create dialog!\n"));
    osip_list_add (&excontext->j_transactions, transaction, 0);
    _eXosip_notify_free (excontext, jn);
    return;
  }
  ADD_ELEMENT (jn->n_dialogs, jd);

  osip_transaction_set_reserved4 (transaction, jn);
  osip_transaction_set_reserved3 (transaction, jd);

  evt_answer = osip_new_outgoing_sipmessage (answer);
  evt_answer->transactionid = transaction->transactionid;
  osip_transaction_add_event (transaction, evt_answer);

  ADD_ELEMENT (excontext->j_notifies, jn);
  _eXosip_wakeup (excontext);

  jn->n_inc_tr = transaction;

  _eXosip_update (excontext);
  _eXosip_wakeup (excontext);
}

static void
_eXosip_process_subscribe_within_call (struct eXosip_t *excontext, eXosip_notify_t * jn, eXosip_dialog_t * jd, osip_transaction_t * transaction, osip_event_t * evt)
{
  _eXosip_notify_set_refresh_interval (jn, evt->sip);
  osip_transaction_set_reserved4 (transaction, jn);
  osip_transaction_set_reserved3 (transaction, jd);

  /* if subscribe request contains expires="0", close the subscription */
  {
    time_t now = osip_getsystemtime (NULL);

    if (jn->n_ss_expires - now <= 0) {
      jn->n_ss_status = EXOSIP_SUBCRSTATE_TERMINATED;
      jn->n_ss_reason = TIMEOUT;
    }
  }

  osip_list_add (jd->d_inc_trs, transaction, 0);
  _eXosip_wakeup (excontext);
  return;
}

static void
_eXosip_process_notify_within_dialog (struct eXosip_t *excontext, eXosip_subscribe_t * js, eXosip_dialog_t * jd, osip_transaction_t * transaction, osip_event_t * evt)
{
  osip_message_t *answer;
  osip_event_t *sipevent;
  osip_header_t *sub_state;

#ifdef SUPPORT_MSN
  osip_header_t *expires;
#endif
  int i;

  if (jd == NULL) {
    osip_list_add (&excontext->j_transactions, transaction, 0);
    _eXosip_send_default_answer (excontext, jd, transaction, evt, 500, "Internal SIP Error", "No dialog for this NOTIFY", __LINE__);
    return;
  }

  /* if subscription-state has a reason state set to terminated,
     we close the dialog */
#ifndef SUPPORT_MSN
  osip_message_header_get_byname (evt->sip, "subscription-state", 0, &sub_state);
  if (sub_state == NULL || sub_state->hvalue == NULL) {
    osip_list_add (&excontext->j_transactions, transaction, 0);
    _eXosip_send_default_answer (excontext, jd, transaction, evt, 400, NULL, NULL, __LINE__);
    return;
  }
#endif

  i = _eXosip_build_response_default (excontext, &answer, jd->d_dialog, 200, evt->sip);
  if (i != 0) {
    osip_list_add (&excontext->j_transactions, transaction, 0);
    _eXosip_send_default_answer (excontext, jd, transaction, evt, 500, "Internal SIP Error", "Failed to build Answer for NOTIFY", __LINE__);
    return;
  }
#ifdef SUPPORT_MSN
  osip_message_header_get_byname (evt->sip, "expires", 0, &expires);
  if (expires != NULL && expires->hvalue != NULL && 0 == osip_strcasecmp (expires->hvalue, "0")) {
    /* delete the dialog! */
    js->s_ss_status = EXOSIP_SUBCRSTATE_TERMINATED;
    {
      eXosip_event_t *je;

      je = _eXosip_event_init_for_subscription (EXOSIP_SUBSCRIPTION_NOTIFY, js, jd);
      _eXosip_event_add (excontext, je);
    }

    sipevent = osip_new_outgoing_sipmessage (answer);
    sipevent->transactionid = transaction->transactionid;
    osip_transaction_add_event (transaction, sipevent);

    osip_list_add (&excontext->j_transactions, transaction, 0);

    REMOVE_ELEMENT (excontext->j_subscribes, js);
    _eXosip_subscription_free (excontext, js);
    _eXosip_wakeup (excontext);

    return;
  }
  else {
    osip_transaction_set_reserved5 (transaction, js);
    osip_transaction_set_reserved3 (transaction, jd);
    js->s_ss_status = EXOSIP_SUBCRSTATE_ACTIVE;
  }
#else
  /* modify the status of user */
  if (0 == osip_strncasecmp (sub_state->hvalue, "active", 6)) {
    js->s_ss_status = EXOSIP_SUBCRSTATE_ACTIVE;
  }
  else if (0 == osip_strncasecmp (sub_state->hvalue, "pending", 7)) {
    js->s_ss_status = EXOSIP_SUBCRSTATE_PENDING;
  }

  if (0 == osip_strncasecmp (sub_state->hvalue, "terminated", 10)) {
    /* delete the dialog! */
    js->s_ss_status = EXOSIP_SUBCRSTATE_TERMINATED;

    {
      eXosip_event_t *je;

      je = _eXosip_event_init_for_subscription (EXOSIP_SUBSCRIPTION_NOTIFY, js, jd, transaction);
      if (je->request == NULL && evt->sip != NULL) {
        i = osip_message_clone (evt->sip, &je->request);
        if (i != 0) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "failed to clone request for event\n"));
        }
      }

      _eXosip_event_add (excontext, je);
    }

    sipevent = osip_new_outgoing_sipmessage (answer);
    sipevent->transactionid = transaction->transactionid;
    osip_transaction_add_event (transaction, sipevent);

    osip_list_add (&excontext->j_transactions, transaction, 0);

    REMOVE_ELEMENT (excontext->j_subscribes, js);
    _eXosip_subscription_free (excontext, js);
    _eXosip_wakeup (excontext);
    return;
  }
  else {
    osip_transaction_set_reserved5 (transaction, js);
    osip_transaction_set_reserved3 (transaction, jd);
  }
#endif

  osip_list_add (jd->d_inc_trs, transaction, 0);

  sipevent = osip_new_outgoing_sipmessage (answer);
  sipevent->transactionid = transaction->transactionid;
  osip_transaction_add_event (transaction, sipevent);

  _eXosip_wakeup (excontext);
  return;
}

static int
_eXosip_match_notify_for_subscribe (eXosip_subscribe_t * js, osip_message_t * notify)
{
  osip_transaction_t *out_sub;

  if (js == NULL)
    return OSIP_BADPARAMETER;
  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Trying to match notify with subscribe\n"));

  out_sub = _eXosip_find_last_out_subscribe (js, NULL);
  if (out_sub == NULL || out_sub->orig_request == NULL)
    return OSIP_NOTFOUND;
  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "subscribe transaction found\n"));

  /* some checks to avoid crashing on bad requests */
  if (notify == NULL)
    return OSIP_BADPARAMETER;

  if (notify->cseq == NULL || notify->cseq->method == NULL || notify->to == NULL)
    return OSIP_SYNTAXERROR;

  if (0 != osip_call_id_match (out_sub->callid, notify->call_id))
    return OSIP_UNDEFINED_ERROR;

  {
    /* The From tag of outgoing request must match
       the To tag of incoming notify:
     */
    osip_generic_param_t *tag_from;
    osip_generic_param_t *tag_to;

    osip_from_param_get_byname (out_sub->from, "tag", &tag_from);
    osip_from_param_get_byname (notify->to, "tag", &tag_to);
    if (tag_to == NULL || tag_to->gvalue == NULL) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Uncompliant user agent: no tag in from of outgoing request\n"));
      return OSIP_SYNTAXERROR;
    }
    if (tag_from == NULL || tag_to->gvalue == NULL) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Uncompliant user agent: no tag in to of incoming request\n"));
      return OSIP_SYNTAXERROR;
    }

    if (0 != strcmp (tag_from->gvalue, tag_to->gvalue))
      return OSIP_UNDEFINED_ERROR;
  }

  return OSIP_SUCCESS;
}

#endif


static void
_eXosip_process_message_within_dialog (struct eXosip_t *excontext, eXosip_call_t * jc, eXosip_dialog_t * jd, osip_transaction_t * transaction, osip_event_t * evt)
{
  osip_list_add (jd->d_inc_trs, transaction, 0);
  osip_transaction_set_reserved2 (transaction, jc);
  osip_transaction_set_reserved3 (transaction, jd);
  _eXosip_wakeup (excontext);
  return;
}


static void
_eXosip_process_newrequest (struct eXosip_t *excontext, osip_event_t * evt, int socket)
{
  osip_transaction_t *transaction;
  int i;
  int ctx_type;
  eXosip_call_t *jc;
  eXosip_dialog_t *jd;
#ifndef MINISIZE
  eXosip_subscribe_t *js;
  eXosip_notify_t *jn;
  osip_event_t *evt_answer;
  osip_message_t *answer;
#endif

  if (MSG_IS_INVITE (evt->sip)) {
    ctx_type = IST;
  }
  else if (MSG_IS_ACK (evt->sip)) {     /* this should be a ACK for 2xx (but could be a late ACK!) */
    ctx_type = -1;
  }
  else if (MSG_IS_REQUEST (evt->sip)) {
    ctx_type = NIST;
  }
  else {                        /* We should handle late response and 200 OK before coming here. */
    ctx_type = -1;
    osip_event_free (evt);
    return;
  }

  transaction = NULL;
  if (ctx_type != -1) {
    i = _eXosip_transaction_init (excontext, &transaction, (osip_fsm_type_t) ctx_type, excontext->j_osip, evt->sip);
    if (i != 0) {
      osip_event_free (evt);
      return;
    }

    osip_transaction_set_in_socket (transaction, socket);
    osip_transaction_set_out_socket (transaction, socket);

    evt->transactionid = transaction->transactionid;
    osip_transaction_set_reserved2 (transaction, NULL);

    osip_transaction_add_event (transaction, evt);
  }

  if (MSG_IS_CANCEL (evt->sip)) {
    /* special handling for CANCEL */
    /* in the new spec, if the CANCEL has a Via branch, then it
       is the same as the one in the original INVITE */
    _eXosip_process_cancel (excontext, transaction, evt);
    return;
  }

  jd = NULL;
  /* first, look for a Dialog in the map of element */
  for (jc = excontext->j_calls; jc != NULL; jc = jc->next) {
    for (jd = jc->c_dialogs; jd != NULL; jd = jd->next) {
      if (jd->d_dialog != NULL) {
        if (osip_dialog_match_as_uas (jd->d_dialog, evt->sip) == 0)
          break;
      }
    }
    if (jd != NULL)
      break;
  }

  /* check CSeq */
  if (jd != NULL && transaction != NULL && evt->sip != NULL && evt->sip->cseq != NULL && evt->sip->cseq->number != NULL) {
    if (jd->d_dialog != NULL && jd->d_dialog->remote_cseq > 0) {
      int cseq = osip_atoi (evt->sip->cseq->number);

      if (cseq < jd->d_dialog->remote_cseq) {
        osip_list_add (&excontext->j_transactions, transaction, 0);
        _eXosip_send_default_answer (excontext, jd, transaction, evt, 500, NULL, "Wrong Lower CSeq", __LINE__);
        return;
      }
      if (cseq == jd->d_dialog->remote_cseq) {

        /* use-case: 1/ a duplicate of initial INVITE is received (same TOP Via header) after we replied -> discard */
        /* use-case: 2/ a duplicate of initial INVITE is received (different TOP Via header) */
        if (MSG_IS_INVITE (evt->sip) && jc->c_inc_tr != NULL) {
          osip_generic_param_t *tag_param_local = NULL;

          i = osip_to_get_tag (evt->sip->to, &tag_param_local);
          if (i != 0) {         /* no tag in request -> initial INVITE */
            osip_generic_param_t *br;
            osip_generic_param_t *br2;

            osip_via_param_get_byname (transaction->topvia, "branch", &br);
            osip_via_param_get_byname (jc->c_inc_tr->topvia, "branch", &br2);
            if (br != NULL && br2 != NULL && br->gvalue != NULL && br2->gvalue != NULL) {
              if (osip_strcasecmp (br->gvalue, br2->gvalue) == 0) {
                /* use-case: 1/ a duplicate of initial INVITE is received (same TOP Via header) after we replied -> discard */
                OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: drop INVITE retransmission after INVITE reply\n"));
                _eXosip_transaction_free (excontext, transaction);
                return;
              }
              else {
                jc = NULL;
                jd = NULL;
#ifdef ACCEPT_DUPLICATE_INVITE
#else
                /* use-case: 2/ a duplicate of initial INVITE is received (different TOP Via header) */
                osip_list_add (&excontext->j_transactions, transaction, 0);
                _eXosip_send_default_answer (excontext, NULL, transaction, evt, 486, NULL, "invite for duplicate registration", __LINE__);
                return;
#endif
              }
            }
          }
        }

        if (jd != NULL) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: receive a request with same cseq??\n"));
          _eXosip_transaction_free (excontext, transaction);
          return;
        }
      }
    }
  }
#ifndef MINISIZE
  if (ctx_type == IST) {
    i = _eXosip_build_response_default (excontext, &answer, NULL, 100, evt->sip);
    if (i != 0) {
      _eXosip_transaction_free (excontext, transaction);
      return;
    }

    osip_message_set_content_length (answer, "0");
    /*  send message to transaction layer */

    evt_answer = osip_new_outgoing_sipmessage (answer);
    evt_answer->transactionid = transaction->transactionid;

    /* add the REQUEST & the 100 Trying */
    osip_transaction_add_event (transaction, evt_answer);
    _eXosip_wakeup (excontext);
  }
#endif

  if (jd != NULL) {
    osip_transaction_t *old_trn;

    /* it can be:
       1: a new INVITE offer.
       2: a REFER request from one of the party.
       2: a BYE request from one of the party.
       3: a REQUEST with a wrong CSeq.
       4: a NOT-SUPPORTED method with a wrong CSeq.
     */

    if (transaction == NULL) {
      /* cannot answer ACK transaction */
    }
    else if (!MSG_IS_BYE (evt->sip)) {
      /* reject all requests for a closed dialog */
      old_trn = _eXosip_find_last_inc_transaction (jc, jd, "BYE");
      if (old_trn == NULL)
        old_trn = _eXosip_find_last_out_transaction (jc, jd, "BYE");

      if (old_trn != NULL) {
        osip_list_add (&excontext->j_transactions, transaction, 0);
        _eXosip_send_default_answer (excontext, jd, transaction, evt, 481, NULL, NULL, __LINE__);
        return;
      }
    }

    if (transaction != NULL)    /* NOT for ACK */
      osip_dialog_update_osip_cseq_as_uas (jd->d_dialog, evt->sip);

    if (MSG_IS_INVITE (evt->sip)) {
      /* the previous transaction MUST be freed */
      old_trn = _eXosip_find_last_inc_invite (jc, jd);

      if (old_trn != NULL && old_trn->state != IST_COMPLETED && old_trn->state != IST_CONFIRMED && old_trn->state != IST_TERMINATED) {
        osip_list_add (&excontext->j_transactions, transaction, 0);
        _eXosip_send_default_answer (excontext, jd, transaction, evt, 500, "Retry Later", "An INVITE is not terminated", __LINE__);
        return;
      }

      old_trn = _eXosip_find_last_out_invite (jc, jd);
      if (old_trn != NULL && old_trn->state != ICT_COMPLETED && old_trn->state != ICT_TERMINATED) {
        osip_list_add (&excontext->j_transactions, transaction, 0);
        _eXosip_send_default_answer (excontext, jd, transaction, evt, 491, NULL, NULL, __LINE__);
        return;
      }

      /* osip_dialog_update_osip_cseq_as_uas (jd->d_dialog, evt->sip); */
      osip_dialog_update_route_set_as_uas (jd->d_dialog, evt->sip);

      _eXosip_process_reinvite (excontext, jc, jd, transaction, evt);
    }
    else if (MSG_IS_BYE (evt->sip)) {
      osip_generic_param_t *tag_to = NULL;

      if (evt->sip->to != NULL)
        osip_from_param_get_byname (evt->sip->to, "tag", &tag_to);
      if (tag_to == NULL || tag_to->gvalue == NULL) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Uncompliant user agent: missing a tag in To of incoming BYE\n"));
        osip_list_add (&excontext->j_transactions, transaction, 0);
        _eXosip_send_default_answer (excontext, jd, transaction, evt, 481, "Missing tags in BYE", "Missing tags in BYE", __LINE__);
        return;
      }

      old_trn = _eXosip_find_last_inc_transaction (jc, jd, "BYE");

      if (old_trn != NULL) {
        /* && old_trn->state!=NIST_TERMINATED) */
        /* this situation should NEVER occur?? (we can't receive two different BYE for one call! */
        osip_list_add (&excontext->j_transactions, transaction, 0);
        _eXosip_send_default_answer (excontext, jd, transaction, evt, 500, "Call Already Terminated", "A pending BYE has already terminate this call", __LINE__);
        return;
      }
      _eXosip_process_bye (excontext, jc, jd, transaction, evt);
    }
    else if (MSG_IS_ACK (evt->sip)) {
      _eXosip_process_ack (excontext, jc, jd, evt);
    }
    else {
      _eXosip_process_message_within_dialog (excontext, jc, jd, transaction, evt);
    }
    return;
  }

  if (MSG_IS_ACK (evt->sip)) {
    /* no transaction has been found for this ACK! */
    osip_event_free (evt);
    return;
  }

  if (MSG_IS_INFO (evt->sip)) {
    osip_list_add (&excontext->j_transactions, transaction, 0);
    _eXosip_send_default_answer (excontext, jd, transaction, evt, 481, NULL, NULL, __LINE__);
    return;                     /* fixed */
  }
  if (MSG_IS_INVITE (evt->sip)) {
    _eXosip_process_new_invite (excontext, transaction, evt);
    return;
  }
  else if (MSG_IS_BYE (evt->sip)) {
    osip_list_add (&excontext->j_transactions, transaction, 0);
    _eXosip_send_default_answer (excontext, jd, transaction, evt, 481, NULL, NULL, __LINE__);
    return;
  }
#ifndef MINISIZE
  js = NULL;
  /* first, look for a Dialog in the map of element */
  for (js = excontext->j_subscribes; js != NULL; js = js->next) {
    for (jd = js->s_dialogs; jd != NULL; jd = jd->next) {
      if (jd->d_dialog != NULL) {
        if (osip_dialog_match_as_uas (jd->d_dialog, evt->sip) == 0)
          break;
      }
    }
    if (jd != NULL)
      break;
  }

  if (js != NULL) {
    /* dialog found */
    osip_transaction_t *old_trn;

    /* it can be:
       1: a NOTIFY.
     */
    if (MSG_IS_NOTIFY (evt->sip)) {
      /* the previous transaction MUST be freed */
      old_trn = _eXosip_find_last_inc_notify (js, jd);

      /* shouldn't we wait for the COMPLETED state? */
      if (old_trn != NULL && old_trn->state != NIST_TERMINATED) {
        /* retry later? */
        osip_list_add (&excontext->j_transactions, transaction, 0);
        _eXosip_send_default_answer (excontext, jd, transaction, evt, SIP_INTERNAL_SERVER_ERROR, "Retry Later", "A pending NOTIFY is not terminated", __LINE__);
        return;
      }

      osip_dialog_update_osip_cseq_as_uas (jd->d_dialog, evt->sip);
      osip_dialog_update_route_set_as_uas (jd->d_dialog, evt->sip);

      _eXosip_process_notify_within_dialog (excontext, js, jd, transaction, evt);
    }
    else {
      osip_list_add (&excontext->j_transactions, transaction, 0);
      _eXosip_send_default_answer (excontext, jd, transaction, evt, SIP_NOT_IMPLEMENTED, NULL, "Just Not Implemented", __LINE__);
    }
    return;
  }

  if (MSG_IS_NOTIFY (evt->sip)) {
    /* let's try to check if the NOTIFY is related to an existing
       subscribe */
    js = NULL;
    /* first, look for a Dialog in the map of element */
    for (js = excontext->j_subscribes; js != NULL; js = js->next) {
      if (_eXosip_match_notify_for_subscribe (js, evt->sip) == 0) {
        i = _eXosip_dialog_init_as_uac (&jd, evt->sip);
        if (i != 0) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot establish a dialog\n"));
          return;
        }

        /* update local cseq from subscribe request */
        if (js->s_out_tr != NULL && js->s_out_tr->cseq != NULL && js->s_out_tr->cseq->number != NULL) {
          jd->d_dialog->local_cseq = atoi (js->s_out_tr->cseq->number);
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: local cseq has been updated\n"));
        }

        ADD_ELEMENT (js->s_dialogs, jd);
        _eXosip_update (excontext);

        _eXosip_process_notify_within_dialog (excontext, js, jd, transaction, evt);
        return;
      }
    }

    osip_list_add (&excontext->j_transactions, transaction, 0);
    return;
  }

  jn = NULL;
  /* first, look for a Dialog in the map of element */
  for (jn = excontext->j_notifies; jn != NULL; jn = jn->next) {
    for (jd = jn->n_dialogs; jd != NULL; jd = jd->next) {
      if (jd->d_dialog != NULL) {
        if (osip_dialog_match_as_uas (jd->d_dialog, evt->sip) == 0)
          break;
      }
    }
    if (jd != NULL)
      break;
  }

  if (jn != NULL) {
    /* dialog found */
    osip_transaction_t *old_trn;

    /* it can be:
       1: a SUBSCRIBE refresh.
       2: a REFER refresh.
     */
    if (MSG_IS_SUBSCRIBE (evt->sip) || MSG_IS_REFER (evt->sip)) {
      /* the previous transaction MUST be freed */
      old_trn = _eXosip_find_last_inc_subscribe (jn, jd);

      /* shouldn't we wait for the COMPLETED state? */
      if (old_trn != NULL && old_trn->state != NIST_TERMINATED && old_trn->state != NIST_COMPLETED) {
        /* retry later? */
        osip_list_add (&excontext->j_transactions, transaction, 0);
        _eXosip_send_default_answer (excontext, jd, transaction, evt, SIP_INTERNAL_SERVER_ERROR, "Retry Later", "A SUBSCRIBE/REFER is not terminated", __LINE__);
        return;
      }

      osip_dialog_update_osip_cseq_as_uas (jd->d_dialog, evt->sip);
      osip_dialog_update_route_set_as_uas (jd->d_dialog, evt->sip);

      _eXosip_process_subscribe_within_call (excontext, jn, jd, transaction, evt);
    }
    else {
      osip_list_add (&excontext->j_transactions, transaction, 0);
      _eXosip_send_default_answer (excontext, jd, transaction, evt, SIP_NOT_IMPLEMENTED, NULL, NULL, __LINE__);
    }
    return;
  }

  if (MSG_IS_REFER (evt->sip)) {
    osip_header_t *refer_sub;
    osip_message_header_get_byname (evt->sip, "Refer-Sub", 0, &refer_sub);
    if (refer_sub==NULL || refer_sub->hvalue==NULL || osip_strncasecmp(refer_sub->hvalue, "true", 4)==0)
    {
      _eXosip_process_new_subscription (excontext, transaction, evt);
      return;
    }
    /* rfc4488.txt: This REFER will not generate a subscription, it will be processed as an out of dialog message */
  } else if (MSG_IS_SUBSCRIBE (evt->sip)) {
    _eXosip_process_new_subscription (excontext, transaction, evt);
    return;
  }
#endif

  /* default answer */
  osip_list_add (&excontext->j_transactions, transaction, 0);
  _eXosip_wakeup (excontext);   /* needed? */
}

static void
_eXosip_process_response_out_of_transaction (struct eXosip_t *excontext, osip_event_t * evt)
{
  eXosip_call_t *jc = NULL;
  eXosip_dialog_t *jd = NULL;

  if (evt->sip == NULL || evt->sip->cseq == NULL || evt->sip->cseq->number == NULL || evt->sip->to == NULL || evt->sip->from == NULL) {
    osip_event_free (evt);
    return;
  }

  if (!MSG_IS_RESPONSE_FOR (evt->sip, "INVITE")) {
    osip_event_free (evt);
    return;
  }

  /* search for existing dialog: match branch & to tag */
  for (jc = excontext->j_calls; jc != NULL; jc = jc->next) {
    /* search for calls with only ONE outgoing transaction */
    if (jc->c_id >= 1 && jc->c_dialogs != NULL && jc->c_out_tr != NULL) {
      for (jd = jc->c_dialogs; jd != NULL; jd = jd->next) {
        if (jd->d_id >= 1 && jd->d_dialog != NULL) {
          /* match answer with dialog */
          osip_generic_param_t *tag;

          osip_from_get_tag (evt->sip->to, &tag);

          if (jd->d_dialog->remote_tag == NULL || tag == NULL)
            continue;
          if (jd->d_dialog->remote_tag != NULL && tag != NULL && tag->gvalue != NULL && 0 == strcmp (jd->d_dialog->remote_tag, tag->gvalue))
            break;
        }
      }
      if (jd != NULL)
        break;                  /* found a matching dialog! */

      /* check if the transaction match this from tag */
      if (jc->c_out_tr->orig_request != NULL && jc->c_out_tr->orig_request->from != NULL) {
        osip_generic_param_t *tag_invite;
        osip_generic_param_t *tag;

        osip_from_get_tag (jc->c_out_tr->orig_request->from, &tag_invite);
        osip_from_get_tag (evt->sip->from, &tag);

        if (tag_invite == NULL || tag == NULL)
          continue;
        if (tag_invite->gvalue != NULL && tag->gvalue != NULL && 0 == strcmp (tag_invite->gvalue, tag->gvalue))
          break;
      }
    }
  }

  if (jc == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Incoming 2xx has no relations with current calls: Message discarded.\r\n"));
    osip_event_free (evt);
    return;
  }

  if (jc != NULL && jd != NULL) {
    /* we have to restransmit the ACK (if already available) */
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "2xx restransmission receveid.\r\n"));
    /* check if the 2xx is for the same ACK */
    if (jd->d_ack != NULL && jd->d_ack->cseq != NULL && jd->d_ack->cseq->number != NULL) {
      if (0 == osip_strcasecmp (jd->d_ack->cseq->number, evt->sip->cseq->number)) {
        _eXosip_snd_message (excontext, NULL, jd->d_ack, NULL, 0, -1);
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "ACK restransmission sent.\r\n"));
      }
    }

    osip_event_free (evt);
    return;
  }

  if (jc != NULL) {
    /* match answer with dialog */
    osip_dialog_t *dlg;
    osip_transaction_t *last_tr;

    int i;

    /* we match an existing dialog: send a retransmission of ACK */
    i = osip_dialog_init_as_uac (&dlg, evt->sip);
    if (i != 0 || dlg == NULL) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Cannot build dialog for 200ok.\r\n"));
      osip_event_free (evt);
      return;
    }

    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "sending ACK for 2xx out of transaction.\r\n"));

    {
      osip_message_t *bye = NULL;
      osip_message_t *ack;

      i = _eXosip_build_request_within_dialog (excontext, &ack, "ACK", dlg);
      if (i != 0) {
        osip_dialog_free (dlg);
        osip_event_free (evt);
        return;
      }
      /* copy all credentials from INVITE! */
      last_tr = jc->c_out_tr;
      if (last_tr != NULL) {
        int pos = 0;
        int i;
        osip_proxy_authorization_t *pa = NULL;

        i = osip_message_get_proxy_authorization (last_tr->orig_request, pos, &pa);
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
          i = osip_message_get_proxy_authorization (last_tr->orig_request, pos, &pa);
        }
      }
      _eXosip_snd_message (excontext, NULL, ack, NULL, 0, -1);
      osip_message_free (ack);

      /* in some case, PRACK and UPDATE may have been sent
         so we have to send a cseq which is above the previous
         one. */
      dlg->local_cseq = dlg->local_cseq + 4;

      /* ready to send a BYE */
      i = _eXosip_generating_bye (excontext, &bye, dlg);
      if (bye != NULL && i == OSIP_SUCCESS)
        _eXosip_snd_message (excontext, NULL, bye, NULL, 0, -1);
      osip_message_free (bye);
    }

    osip_dialog_free (dlg);
    osip_event_free (evt);
    return;
  }

  /* ...code not reachable... */
}


static int
_eXosip_handle_rfc5626_ob (osip_message_t * message, char *remote_host, int remote_port)
{
  osip_contact_t *co;
  osip_uri_param_t *u_param = NULL;
  char _remote_port[10];

  if (message == NULL)
    return OSIP_BADPARAMETER;
  if (message->cseq == NULL)
    return OSIP_SYNTAXERROR;
  if (message->cseq->method == NULL)
    return OSIP_SYNTAXERROR;

  if (0==strcmp(message->cseq->method,"REGISTER"))
    return OSIP_SUCCESS;

  if (osip_list_size(&message->record_routes) > 0)
    return OSIP_SYNTAXERROR;

  snprintf(_remote_port, sizeof(_remote_port), "%i", remote_port);

  co = (osip_contact_t *) osip_list_get (&message->contacts, 0);

  if (co == NULL || co->url == NULL)
    return OSIP_SUCCESS;

  osip_uri_uparam_get_byname (co->url, "ob", &u_param);
  if (u_param == NULL || u_param->gname == NULL)
    return OSIP_SUCCESS;

  /* add internal x-ob parameters with connection info */
  osip_uri_uparam_add (co->url, osip_strdup ("x-obr"), osip_strdup (remote_host));
  osip_uri_uparam_add (co->url, osip_strdup ("x-obp"), osip_strdup (_remote_port));
  return OSIP_SUCCESS;
}

static int
_eXosip_handle_received_rport (osip_message_t * response, char *received_host, int *rport_port)
{
  osip_generic_param_t *rport;
  osip_generic_param_t *received;
  osip_via_t *via;

  /* get Top most Via header: */
  if (response == NULL)
    return OSIP_BADPARAMETER;
  if (MSG_IS_REQUEST (response))
    return OSIP_SUCCESS;
  if (received_host == NULL)
    return OSIP_SUCCESS;
  if (rport_port == NULL)
    return OSIP_SUCCESS;

  via = osip_list_get (&response->vias, 0);
  if (via == NULL || via->host == NULL)
    return OSIP_BADPARAMETER;

  osip_via_param_get_byname (via, "rport", &rport);
  if (rport != NULL) {
    if (rport->gvalue != NULL) {
      *rport_port = atoi (rport->gvalue);
    }
  }
  osip_via_param_get_byname (via, "received", &received);
  if (received != NULL) {
    if (received->gvalue != NULL && strlen (received->gvalue) > 0) {
      snprintf (received_host, 65, "%s", received->gvalue);
    }
  } else { /* fix 11/09/2015: if no received, the local IP may still have changed. */
      snprintf (received_host, 65, "%s", via->host);
  }
  return 0;
}

static void
udp_tl_learn_port_from_via (struct eXosip_t *excontext, osip_message_t * sip)
{
  struct eXtludp *reserved = (struct eXtludp *) excontext->eXtludp_reserved;

  if (reserved == NULL) {
    return;
  }

  /* EXOSIP_OPT_AUTO_MASQUERADE_CONTACT option set */
  if (excontext->auto_masquerade_contact > 0) {
    osip_via_t *via = NULL;
    osip_generic_param_t *br_rport;
    osip_generic_param_t *br_received;
    int i;

    i = osip_message_get_via (sip, 0, &via);
    if (i >= 0 && via != NULL && via->protocol != NULL && (osip_strcasecmp (via->protocol, "udp") == 0 || osip_strcasecmp (via->protocol, "dtls") == 0)) {
      struct eXosip_account_info ainfo;
      osip_via_param_get_byname (via, "rport", &br_rport);
      osip_via_param_get_byname (via, "received", &br_received);

      if (br_rport == NULL && br_received == NULL)
        return; /* no change */
      if (br_rport != NULL && br_rport->gvalue == NULL && br_received == NULL)
        return; /* no change */

      memset (&ainfo, 0, sizeof (struct eXosip_account_info));
      if (br_rport != NULL && br_rport->gvalue != NULL) {
        ainfo.nat_port = atoi (br_rport->gvalue);
      } else if (via->port!=NULL) {
        ainfo.nat_port = atoi (via->port);
      } else {
        if (osip_strcasecmp(via->protocol, "DTLS")==0 || osip_strcasecmp(via->protocol, "TLS")==0)
          ainfo.nat_port = 5061;
        else
          ainfo.nat_port = 5060;
      }

      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "SIP port modified from rport in SIP answer\r\n"));

      if (br_received != NULL && br_received->gvalue != NULL) {
        snprintf (ainfo.nat_ip, sizeof (ainfo.nat_ip), "%s", br_received->gvalue);
      } else {
        snprintf (ainfo.nat_ip, sizeof (ainfo.nat_ip), "%s", via->host);
      }
      if (sip->from != NULL && sip->from->url != NULL && sip->from->url->host != NULL) {
        snprintf (ainfo.proxy, sizeof (ainfo.proxy), "%s", sip->from->url->host);
        if (eXosip_set_option (excontext, EXOSIP_OPT_ADD_ACCOUNT_INFO, &ainfo)==OSIP_SUCCESS) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "we now appear as %s:%i for server %s\r\n", ainfo.nat_ip, ainfo.nat_port, ainfo.proxy));
        }
      }
    }
  }
  return;
}

int
_eXosip_handle_incoming_message (struct eXosip_t *excontext, char *buf, size_t length, int socket, char *host, int port, char *received_host, int *rport_port)
{
  int i;
  osip_event_t *se;
  int tmp;

  se = (osip_event_t *) osip_malloc (sizeof (osip_event_t));
  if (se == NULL)
    return OSIP_NOMEM;
  se->type = UNKNOWN_EVT;
  se->sip = NULL;
  se->transactionid = 0;

  tmp = buf[length];
  buf[length] = 0;
  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Received message len=%i from %s:%i:\n%s\n", length, host, port, buf));
  buf[length] = tmp;

  /* parse message and set up an event */
  i = osip_message_init (&(se->sip));
  if (i != 0) {
    osip_free (se);
    return i;
  }
  i = osip_message_parse (se->sip, buf, length);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "could not parse message\n"));
    osip_message_free (se->sip);
    osip_free (se);
    return i;
  }

  if (se->sip->call_id != NULL && se->sip->call_id->number != NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO3, NULL, "MESSAGE REC. CALLID:%s\n", se->sip->call_id->number));
  }

  if (excontext->cbsipCallback != NULL) {
    excontext->cbsipCallback (se->sip, 1);
  }

  if (MSG_IS_REQUEST (se->sip)) {
    if (se->sip->sip_method == NULL || se->sip->req_uri == NULL) {
      osip_message_free (se->sip);
      osip_free (se);
      return OSIP_SYNTAXERROR;
    }
  }

  if (MSG_IS_REQUEST (se->sip)) {
    if (MSG_IS_INVITE (se->sip))
      se->type = RCV_REQINVITE;
    else if (MSG_IS_ACK (se->sip))
      se->type = RCV_REQACK;
    else
      se->type = RCV_REQUEST;
  }
  else {
    if (se->sip->status_code < 100 || se->sip->status_code > 699) {
      osip_message_free (se->sip);
      osip_free (se);
      return OSIP_SYNTAXERROR;
    }
    else if (MSG_IS_STATUS_1XX (se->sip))
      se->type = RCV_STATUS_1XX;
    else if (MSG_IS_STATUS_2XX (se->sip))
      se->type = RCV_STATUS_2XX;
    else
      se->type = RCV_STATUS_3456XX;
  }

  osip_message_fix_last_via_header (se->sip, host, port);
  _eXosip_handle_rfc5626_ob (se->sip, host, port);

  if (MSG_IS_RESPONSE (se->sip)) {
    _eXosip_handle_received_rport (se->sip, received_host, rport_port);
    udp_tl_learn_port_from_via (excontext, se->sip);
  }

  i = osip_find_transaction_and_add_event (excontext->j_osip, se);
  if (i != 0) {
    /* this event has no transaction, */
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "no transaction for message\n"));
    eXosip_lock (excontext);
    if (MSG_IS_REQUEST (se->sip))
      _eXosip_process_newrequest (excontext, se, socket);
    else if (MSG_IS_RESPONSE (se->sip))
      _eXosip_process_response_out_of_transaction (excontext, se);
    eXosip_unlock (excontext);
  }
  else {
    /* handled by oSIP ! */
    return OSIP_SUCCESS;
  }
  return OSIP_SUCCESS;
}

#if defined (WIN32) || defined (_WIN32_WCE)
#define eXFD_SET(A, B)   FD_SET((unsigned int) A, B)
#else
#define eXFD_SET(A, B)   FD_SET(A, B)
#endif

/* if second==-1 && useconds==-1  -> wait for ever
   if max_message_nb<=0  -> infinite loop....  */
int
_eXosip_read_message (struct eXosip_t *excontext, int max_message_nb, int sec_max, int usec_max)
{
  fd_set osip_fdset;
  fd_set osip_wrset;
  struct timeval tv;

  tv.tv_sec = sec_max;
  tv.tv_usec = usec_max;

  while (max_message_nb != 0 && excontext->j_stop_ua == 0) {
    int i;
    int max = 0;

#ifndef OSIP_MONOTHREAD
    int wakeup_socket = jpipe_get_read_descr (excontext->j_socketctl);
#endif

    FD_ZERO (&osip_fdset);
    FD_ZERO (&osip_wrset);
    excontext->eXtl_transport.tl_set_fdset (excontext, &osip_fdset, &osip_wrset, &max);
#ifndef OSIP_MONOTHREAD
    eXFD_SET (wakeup_socket, &osip_fdset);
    if (wakeup_socket > max)
      max = wakeup_socket;
#endif

#ifdef TSC_SUPPORT
    if (excontext->tunnel_handle) {
      int udp_socket = max;

      if ((sec_max != -1) && (usec_max != -1)) {
        int32_t total_time = sec_max * 1000 + usec_max / 1000;

        while (total_time > 0) {
          struct timeval tv;
          struct tsc_timeval ttv;
          tsc_fd_set tsc_fdset;

          FD_ZERO (&osip_fdset);
          eXFD_SET (wakeup_socket, &osip_fdset);

          tv.tv_sec = 0;
          tv.tv_usec = 1000;

          i = select (wakeup_socket + 1, &osip_fdset, NULL, NULL, &tv);
          if (i > 0) {
            break;
          }
          else if (i == -1) {
            return -1;
          }

          ttv.tv_sec = 0;
          ttv.tv_usec = 1000;
          TSC_FD_ZERO (&tsc_fdset);
          TSC_FD_SET (udp_socket, &tsc_fdset);

          i = tsc_select (udp_socket + 1, &tsc_fdset, NULL, NULL, &ttv);
          if (i > 0) {
            eXFD_SET (udp_socket, &osip_fdset);

            break;
          }
          else if (i == -1) {
            return -1;
          }

          tsc_sleep (100);

          total_time -= 100;
        }
      }
    }
    else {
      if ((sec_max == -1) || (usec_max == -1))
        i = select (max + 1, &osip_fdset, NULL, NULL, NULL);
      else
        i = select (max + 1, &osip_fdset, NULL, NULL, &tv);
    }
#else
    if ((sec_max == -1) || (usec_max == -1))
      i = select (max + 1, &osip_fdset, NULL, NULL, NULL);
    else
      i = select (max + 1, &osip_fdset, NULL, NULL, &tv);
#endif

#if defined (_WIN32_WCE)
    /* TODO: fix me for wince */
    /* if (i == -1)
       continue; */
#else
    if ((i == -1) && (errno == EINTR || errno == EAGAIN)) {

      if (excontext->cbsipWakeLock!=NULL && excontext->incoming_wake_lock_state>0) {
        int count = osip_list_size(&excontext->j_osip->osip_ist_transactions);
        count+=osip_list_size(&excontext->j_osip->osip_nist_transactions);
        if (count==0) {
          excontext->cbsipWakeLock(0);
          excontext->incoming_wake_lock_state=0;
        }
      }

      continue;
    }
#endif

    osip_compensatetime ();

#ifndef OSIP_MONOTHREAD
    if ((i > 0) && FD_ISSET (wakeup_socket, &osip_fdset)) {
      char buf2[500];

      jpipe_read (excontext->j_socketctl, buf2, 499);
    }
#endif

    if (0 == i || excontext->j_stop_ua != 0) {
      return OSIP_SUCCESS;
    }
    else if (-1 == i) {
#if !defined (_WIN32_WCE)       /* TODO: fix me for wince */
      return -2000;             /* error */
#endif
    }
    else {

      if (excontext->cbsipWakeLock!=NULL && excontext->incoming_wake_lock_state==0)
        excontext->cbsipWakeLock(++excontext->incoming_wake_lock_state);

      excontext->eXtl_transport.tl_read_message (excontext, &osip_fdset, &osip_wrset);

    }

    if (excontext->cbsipWakeLock!=NULL && excontext->incoming_wake_lock_state>0) {
      int count = osip_list_size(&excontext->j_osip->osip_ist_transactions);
      count+=osip_list_size(&excontext->j_osip->osip_nist_transactions);
      if (count==0) {
        excontext->cbsipWakeLock(0);
        excontext->incoming_wake_lock_state=0;
      }
    }

    /* avoid infinite select if a message was read at the very end of period */
    if (tv.tv_sec == 0 && tv.tv_usec == 0 && (sec_max != 0 || usec_max != 0)) {
      return OSIP_SUCCESS;
    }
    max_message_nb--;
  }
  return OSIP_SUCCESS;
}

static int
_eXosip_pendingosip_transaction_exist (struct eXosip_t *excontext, eXosip_call_t * jc, eXosip_dialog_t * jd)
{
  osip_transaction_t *tr;
  time_t now = osip_getsystemtime (NULL);

  tr = _eXosip_find_last_inc_transaction (jc, jd, "BYE");
  if (tr != NULL && tr->state != NIST_TERMINATED) {     /* Don't want to wait forever on broken transaction!! */
    if (tr->birth_time + 180 < now) {   /* Wait a max of 2 minutes */
      /* remove the transaction from oSIP: */
      osip_remove_transaction (excontext->j_osip, tr);
      _eXosip_remove_transaction_from_call (tr, jc);
      osip_list_add (&excontext->j_transactions, tr, 0);
    }
    else
      return OSIP_SUCCESS;
  }

  tr = _eXosip_find_last_out_transaction (jc, jd, "BYE");
  if (tr != NULL && tr->state != NICT_TERMINATED) {     /* Don't want to wait forever on broken transaction!! */
    if (tr->birth_time + 180 < now) {   /* Wait a max of 2 minutes */
      /* remove the transaction from oSIP: */
      osip_remove_transaction (excontext->j_osip, tr);
      _eXosip_remove_transaction_from_call (tr, jc);
      osip_list_add (&excontext->j_transactions, tr, 0);
    }
    else
      return OSIP_SUCCESS;
  }

  tr = _eXosip_find_last_inc_invite (jc, jd);
  if (tr != NULL && tr->state != IST_TERMINATED) {      /* Don't want to wait forever on broken transaction!! */
    if (tr->birth_time + 180 < now) {   /* Wait a max of 2 minutes */
    }
    else
      return OSIP_SUCCESS;
  }

  tr = _eXosip_find_last_out_invite (jc, jd);
  if (tr != NULL && tr->state != ICT_TERMINATED) {      /* Don't want to wait forever on broken transaction!! */
    if (jc->expire_time < now) {
    }
    else
      return OSIP_SUCCESS;
  }

  tr = _eXosip_find_last_inc_transaction (jc, jd, "REFER");
  if (tr != NULL && tr->state != NIST_TERMINATED) {     /* Don't want to wait forever on broken transaction!! */
    if (tr->birth_time + 180 < now) {   /* Wait a max of 2 minutes */
      /* remove the transaction from oSIP: */
      osip_remove_transaction (excontext->j_osip, tr);
      _eXosip_remove_transaction_from_call (tr, jc);
      osip_list_add (&excontext->j_transactions, tr, 0);
    }
    else
      return OSIP_SUCCESS;
  }

  tr = _eXosip_find_last_out_transaction (jc, jd, "REFER");
  if (tr != NULL && tr->state != NICT_TERMINATED) {     /* Don't want to wait forever on broken transaction!! */
    if (tr->birth_time + 180 < now) {   /* Wait a max of 2 minutes */
      /* remove the transaction from oSIP: */
      osip_remove_transaction (excontext->j_osip, tr);
      _eXosip_remove_transaction_from_call (tr, jc);
      osip_list_add (&excontext->j_transactions, tr, 0);
    }
    else
      return OSIP_SUCCESS;
  }

  return OSIP_UNDEFINED_ERROR;
}

static int
_eXosip_release_finished_transactions (struct eXosip_t *excontext, eXosip_call_t * jc, eXosip_dialog_t * jd)
{
  osip_list_iterator_t it;
  time_t now = osip_getsystemtime (NULL);
  osip_transaction_t *inc_tr;
  osip_transaction_t *out_tr;
  osip_transaction_t *last_invite;
  int ret;

  ret = -1;

  last_invite = _eXosip_find_last_inc_invite (jc, jd);

  if (jd != NULL) {
    /* go through all incoming transactions of this dialog */
    inc_tr = (osip_transaction_t*)osip_list_get_first(jd->d_inc_trs, &it);
    if (inc_tr!=NULL) inc_tr = (osip_transaction_t *)osip_list_get_next(&it); /* skip first one */
    while (inc_tr != OSIP_SUCCESS) {
      if (0 != osip_strcasecmp (inc_tr->cseq->method, "INVITE")) {
        /* remove, if transaction too old, independent of the state */
        if ((inc_tr->state == NIST_TERMINATED) && (inc_tr->birth_time + 30 < now)) {    /* Wait a max of 30 seconds */
          /* remove the transaction from oSIP */
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: release non-INVITE server transaction (did=%i)\n", jd->d_id));
          osip_remove_transaction (excontext->j_osip, inc_tr);
          osip_list_iterator_remove(&it);
          osip_list_add (&excontext->j_transactions, inc_tr, 0);

          ret = 0;
          break;
        }
      }
      else {
        /* remove, if transaction too old, independent of the state */
        if (last_invite != inc_tr && (inc_tr->state == IST_TERMINATED) && (inc_tr->birth_time + 30 < now)) {    /* Wait a max of 30 seconds */
          /* remove the transaction from oSIP */
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: release INVITE server transaction (did=%i)\n", jd->d_id));
          osip_remove_transaction (excontext->j_osip, inc_tr);
          osip_list_iterator_remove(&it);
          osip_list_add (&excontext->j_transactions, inc_tr, 0);

          ret = 0;
          break;
        }
      }
      inc_tr = (osip_transaction_t *)osip_list_get_next(&it);
    }

    last_invite = _eXosip_find_last_out_invite (jc, jd);

    /* go through all outgoing transactions of this dialog */
    out_tr = (osip_transaction_t*)osip_list_get_first(jd->d_out_trs, &it);
    if (out_tr!=NULL) out_tr = (osip_transaction_t *)osip_list_get_next(&it); /* skip first one */
    while (out_tr != OSIP_SUCCESS) {
      if (0 != osip_strcasecmp (out_tr->cseq->method, "INVITE")) {
        /* remove, if transaction too old, independent of the state */
        if ((out_tr->state == NICT_TERMINATED) && (out_tr->birth_time + 30 < now)) {    /* Wait a max of 30 seconds */
          /* remove the transaction from oSIP */
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: release non INVITE client transaction (did=%i)\n", jd->d_id));
          osip_remove_transaction (excontext->j_osip, out_tr);
          osip_list_iterator_remove(&it);
          osip_list_add (&excontext->j_transactions, out_tr, 0);

          ret = 0;
          break;
        }
      }
      else {
        /* remove, if transaction too old, independent of the state */
        if (last_invite != out_tr && (out_tr->state == ICT_TERMINATED) && (out_tr->birth_time + 30 < now)) {    /* Wait a max of 30 seconds */
          /* remove the transaction from oSIP */
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: release INVITE client transaction (did=%i)\n", jd->d_id));
          osip_remove_transaction (excontext->j_osip, out_tr);
          osip_list_iterator_remove(&it);
          osip_list_add (&excontext->j_transactions, out_tr, 0);

          ret = 0;
          break;
        }
      }
      out_tr = (osip_transaction_t *)osip_list_get_next(&it);
    }
  }

  return ret;
}

static int
_eXosip_release_finished_calls (struct eXosip_t *excontext, eXosip_call_t * jc, eXosip_dialog_t * jd)
{
  osip_transaction_t *tr;

  tr = _eXosip_find_last_inc_transaction (jc, jd, "BYE");
  if (tr == NULL)
    tr = _eXosip_find_last_out_transaction (jc, jd, "BYE");

  if (tr != NULL && (tr->state == NIST_TERMINATED || tr->state == NICT_TERMINATED)) {
    int did = -2;

    if (jd != NULL)
      did = jd->d_id;
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: _eXosip_release_finished_calls remove a dialog (cid=%i did=%i)\n", jc->c_id, did));
    /* Remove existing reference to the dialog from transactions! */
    _eXosip_call_remove_dialog_reference_in_call (jc, jd);
    REMOVE_ELEMENT (jc->c_dialogs, jd);
    _eXosip_dialog_free (excontext, jd);
    return OSIP_SUCCESS;
  }
  return OSIP_UNDEFINED_ERROR;
}


static void
_eXosip_release_call (struct eXosip_t *excontext, eXosip_call_t * jc, eXosip_dialog_t * jd)
{
  REMOVE_ELEMENT (excontext->j_calls, jc);
  _eXosip_report_call_event (excontext, EXOSIP_CALL_RELEASED, jc, jd, NULL);
  _eXosip_call_free (excontext, jc);
  _eXosip_wakeup (excontext);
}


static int
_eXosip_release_aborted_calls (struct eXosip_t *excontext, eXosip_call_t * jc, eXosip_dialog_t * jd)
{
  time_t now = osip_getsystemtime (NULL);
  osip_transaction_t *tr;

  /* close calls only when the initial INVITE failed */
  tr = jc->c_inc_tr;
  if (tr == NULL)
    tr = jc->c_out_tr;

  if (tr == NULL) {
    if (jd != NULL) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: _eXosip_release_aborted_calls remove an empty dialog\n"));
      _eXosip_call_remove_dialog_reference_in_call (jc, jd);
      REMOVE_ELEMENT (jc->c_dialogs, jd);
      _eXosip_dialog_free (excontext, jd);
      return OSIP_SUCCESS;
    }
    return OSIP_UNDEFINED_ERROR;
  }

  if (tr != NULL && tr->state != IST_TERMINATED && tr->state != ICT_TERMINATED && tr->birth_time + 180 < now) { /* Wait a max of 2 minutes */
    if (jd != NULL) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: _eXosip_release_aborted_calls remove a dialog for an unfinished transaction\n"));
      _eXosip_call_remove_dialog_reference_in_call (jc, jd);
      REMOVE_ELEMENT (jc->c_dialogs, jd);
      /* _eXosip_report_call_event(excontext, EXOSIP_CALL_NOANSWER, jc, jd, NULL); */
      _eXosip_report_call_event (excontext, EXOSIP_CALL_NOANSWER, jc, jd, tr);
      _eXosip_dialog_free (excontext, jd);
      _eXosip_wakeup (excontext);
      return OSIP_SUCCESS;
    }
  }

  if (tr != NULL && (tr->state == IST_TERMINATED || tr->state == ICT_TERMINATED)) {
    if (tr == jc->c_inc_tr) {
      if (jc->c_inc_tr->last_response == NULL) {
        /* OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,
           "eXosip: _eXosip_release_aborted_calls transaction with no answer\n")); */
      }
      else if (jc->c_inc_tr->last_response->status_code >= 300) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: _eXosip_release_aborted_calls (answer sent = %i %s)\n", jc->c_inc_tr->last_response->status_code, jc->c_inc_tr->last_response->reason_phrase));
        _eXosip_release_call (excontext, jc, jd);
        return OSIP_SUCCESS;
      }
    }
    else if (tr == jc->c_out_tr) {
      if (jc->c_out_tr->last_response == NULL) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: _eXosip_release_aborted_calls (answer received = 0 Timeout)\n"));
        _eXosip_release_call (excontext, jc, jd);
        return OSIP_SUCCESS;
      }
      else if (jc->c_out_tr->last_response->status_code >= 300 && tr->completed_time + 2 < now) {
        /* wait for 3xx to be processed */
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: _eXosip_release_aborted_calls (answer received = %i %s)\n", jc->c_out_tr->last_response->status_code, jc->c_out_tr->last_response->reason_phrase));
        _eXosip_release_call (excontext, jc, jd);
        return OSIP_SUCCESS;
      }
      else if (jc->c_out_tr->last_response->status_code >= 400) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: _eXosip_release_aborted_calls (answer received = %i %s)\n", jc->c_out_tr->last_response->status_code, jc->c_out_tr->last_response->reason_phrase));
        _eXosip_release_call (excontext, jc, jd);
        return OSIP_SUCCESS;
      }
    }
  }

  return OSIP_UNDEFINED_ERROR;
}


void
_eXosip_release_terminated_calls (struct eXosip_t *excontext)
{
  osip_list_iterator_t it;
  osip_transaction_t *tr;
  eXosip_dialog_t *jd;
  eXosip_dialog_t *jdnext;
  eXosip_call_t *jc;
  eXosip_call_t *jcnext;
  time_t now = osip_getsystemtime (NULL);


  for (jc = excontext->j_calls; jc != NULL;) {
    jcnext = jc->next;
    /* free call terminated with a BYE */

    for (jd = jc->c_dialogs; jd != NULL;) {
      jdnext = jd->next;
      if (0 == _eXosip_pendingosip_transaction_exist (excontext, jc, jd)) {
      }
      else if (0 == _eXosip_release_finished_transactions (excontext, jc, jd)) {
      }
      else if (0 == _eXosip_release_finished_calls (excontext, jc, jd)) {
        jd = jc->c_dialogs;
      }
      else if (0 == _eXosip_release_aborted_calls (excontext, jc, jd)) {
        jdnext = NULL;
      }
      else if (jd->d_id == -1) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: _eXosip_release_terminated_calls delete a removed dialog (cid=%i did=%i)\n", jc->c_id, jd->d_id));
        /* Remove existing reference to the dialog from transactions! */
        _eXosip_call_remove_dialog_reference_in_call (jc, jd);
        REMOVE_ELEMENT (jc->c_dialogs, jd);
        _eXosip_dialog_free (excontext, jd);

        jd = jc->c_dialogs;
      }
      jd = jdnext;
    }
    jc = jcnext;
  }

  for (jc = excontext->j_calls; jc != NULL;) {
    jcnext = jc->next;
    if (jc->c_dialogs == NULL) {
      if (jc->c_inc_tr != NULL && jc->c_inc_tr->state != IST_TERMINATED && jc->c_inc_tr->birth_time + 180 < now) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "eXosip: remove an incoming call with no final answer\n"));
        _eXosip_release_call (excontext, jc, NULL);
      }
      else if (jc->c_out_tr != NULL && jc->c_out_tr->state != ICT_TERMINATED && jc->c_out_tr->birth_time + 180 < now) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "eXosip: remove an outgoing call with no final answer\n"));
        _eXosip_release_call (excontext, jc, NULL);
      }
      else if (jc->c_inc_tr != NULL && jc->c_inc_tr->state != IST_TERMINATED) {
      }
      else if (jc->c_out_tr != NULL && jc->c_out_tr->state != ICT_TERMINATED) {
      }
      else if (jc->c_out_tr != NULL && jc->c_out_tr->state == ICT_TERMINATED && jc->c_out_tr->completed_time + 10 > now) {
        /* With unreliable protocol, the transaction enter the terminated
           state right after the ACK is sent: In this case, we really want
           to wait for additionnal user/automatic action to be processed
           before we decide to delete the call.
         */


      }
      else {                    /* no active pending transaction */

        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "eXosip: remove a call\n"));
        _eXosip_release_call (excontext, jc, NULL);
      }
    }
    jc = jcnext;
  }

  
  tr = (osip_transaction_t*)osip_list_get_first(&excontext->j_transactions, &it);
  while (tr != NULL) {

    if (tr->state == NICT_TERMINATED && tr->last_response != NULL && tr->completed_time + 5 > now) {
      /* keep transaction until authentication or ... */
    }
    else if (tr->state == IST_TERMINATED || tr->state == ICT_TERMINATED || tr->state == NICT_TERMINATED || tr->state == NIST_TERMINATED) {      /* free (transaction is already removed from the oSIP stack) */
      _eXosip_transaction_free (excontext, tr);
      tr = (osip_transaction_t *)osip_list_iterator_remove(&it);
      continue;
    }
    else if (tr->birth_time + 180 < now) {      /* Wait a max of 2 minutes */
      _eXosip_transaction_free (excontext, tr);
      tr = (osip_transaction_t *)osip_list_iterator_remove(&it);
      continue;
    }
    
    tr = (osip_transaction_t *)osip_list_get_next(&it);
  }
}

void
_eXosip_release_terminated_registrations (struct eXosip_t *excontext)
{
  eXosip_reg_t *jr;
  eXosip_reg_t *jrnext;
  time_t now = osip_getsystemtime (NULL);

  for (jr = excontext->j_reg; jr != NULL;) {
    jrnext = jr->next;
    if (jr->r_reg_period == 0 && jr->r_last_tr != NULL) {
      if (now - jr->r_last_tr->birth_time > 75) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Release a terminated registration\n"));
        REMOVE_ELEMENT (excontext->j_reg, jr);
        _eXosip_reg_free (excontext, jr);
      }
      else if (jr->r_last_tr->last_response != NULL && jr->r_last_tr->last_response->status_code >= 200 && jr->r_last_tr->last_response->status_code <= 299) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Release a terminated registration with 2xx\n"));
        REMOVE_ELEMENT (excontext->j_reg, jr);
        _eXosip_reg_free (excontext, jr);
      }
    }

    jr = jrnext;
  }

  return;
}

#ifndef MINISIZE

void
_eXosip_release_terminated_publications (struct eXosip_t *excontext)
{
  eXosip_pub_t *jpub;
  eXosip_pub_t *jpubnext;
  time_t now = osip_getsystemtime (NULL);

  for (jpub = excontext->j_pub; jpub != NULL;) {
    jpubnext = jpub->next;
    if (jpub->p_period == 0 && jpub->p_last_tr != NULL) {
      if (now - jpub->p_last_tr->birth_time > 75) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Release a terminated publication\n"));
        REMOVE_ELEMENT (excontext->j_pub, jpub);
        _eXosip_pub_free (excontext, jpub);
      }
      else if (jpub->p_last_tr->last_response != NULL && jpub->p_last_tr->last_response->status_code >= 200 && jpub->p_last_tr->last_response->status_code <= 299) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Release a terminated publication with 2xx\n"));
        REMOVE_ELEMENT (excontext->j_pub, jpub);
        _eXosip_pub_free (excontext, jpub);
      }
    }

    jpub = jpubnext;
  }

}


static int
_eXosip_release_finished_transactions_for_subscription (struct eXosip_t *excontext, eXosip_dialog_t * jd)
{
  osip_list_iterator_t it;
  time_t now = osip_getsystemtime (NULL);
  osip_transaction_t *inc_tr;
  osip_transaction_t *out_tr;
  int skip_first = 0;
  int ret;

  ret = OSIP_UNDEFINED_ERROR;

  if (jd != NULL) {
    /* go through all incoming transactions of this dialog */
    inc_tr = (osip_transaction_t*)osip_list_get_first(jd->d_inc_trs, &it);
    while (inc_tr != OSIP_SUCCESS) {
      /* remove, if transaction too old, independent of the state */
      if ((skip_first == 1) && (inc_tr->state == NIST_TERMINATED) && (inc_tr->birth_time + 30 < now)) { /* keep it for 30 seconds */
        /* remove the transaction from oSIP */
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: release non-INVITE server transaction (did=%i)\n", jd->d_id));
        osip_remove_transaction (excontext->j_osip, inc_tr);
        osip_list_iterator_remove(&it);
        osip_list_add (&excontext->j_transactions, inc_tr, 0);

        ret = OSIP_SUCCESS;     /* return "released" */
        break;
      }
      if (0 == osip_strcasecmp (inc_tr->cseq->method, "SUBSCRIBE"))
        skip_first = 1;
      if (0 == osip_strcasecmp (inc_tr->cseq->method, "REFER"))
        skip_first = 1;
      if (0 == osip_strcasecmp (inc_tr->cseq->method, "NOTIFY"))
        skip_first = 1;
      inc_tr = (osip_transaction_t *)osip_list_get_next(&it);
    }

    skip_first = 0;

    /* go through all outgoing transactions of this dialog */
    
    out_tr = (osip_transaction_t*)osip_list_get_first(jd->d_out_trs, &it);
    while (out_tr != OSIP_SUCCESS) {
      /* remove, if transaction too old, independent of the state */
      if ((skip_first == 1) && (out_tr->state == NICT_TERMINATED) && (out_tr->birth_time + 30 < now)) { /* Wait a max of 30 seconds */
        /* remove the transaction from oSIP */
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: release non INVITE client transaction (did=%i)\n", jd->d_id));
        osip_remove_transaction (excontext->j_osip, out_tr);
        osip_list_iterator_remove(&it);
        osip_list_add (&excontext->j_transactions, out_tr, 0);

        ret = OSIP_SUCCESS;     /* return "released" */
        break;
      }
      if (0 == osip_strcasecmp (out_tr->cseq->method, "SUBSCRIBE"))
        skip_first = 1;
      if (0 == osip_strcasecmp (out_tr->cseq->method, "REFER"))
        skip_first = 1;
      if (0 == osip_strcasecmp (out_tr->cseq->method, "NOTIFY"))
        skip_first = 1;

      out_tr = (osip_transaction_t *)osip_list_get_next(&it);
    }
  }

  return ret;
}

void
_eXosip_release_terminated_subscriptions (struct eXosip_t *excontext)
{
  time_t now = osip_getsystemtime (NULL);
  eXosip_dialog_t *jd;
  eXosip_dialog_t *jdnext;
  eXosip_subscribe_t *js;
  eXosip_subscribe_t *jsnext;

  for (js = excontext->j_subscribes; js != NULL;) {
    jsnext = js->next;

    if (js->s_dialogs == NULL) {
      if (js->s_out_tr != NULL && js->s_out_tr->birth_time + 64 < now) {        /* Wait a max of 64 sec */
        /* destroy non established contexts after max of 64 sec */
        REMOVE_ELEMENT (excontext->j_subscribes, js);
        _eXosip_subscription_free (excontext, js);
        _eXosip_wakeup (excontext);
        return;
      }
    }
    else {
      /* fix 14/07/11. NULL pointer */
      jd = js->s_dialogs;
      if (jd != NULL) {
        osip_transaction_t *transaction = _eXosip_find_last_out_subscribe (js, jd);

        if (transaction != NULL && transaction->orig_request != NULL && transaction->state == NICT_TERMINATED && transaction->birth_time + 15 < now) {
          osip_header_t *expires;

          if (MSG_IS_REFER(transaction->orig_request)) {
            /* exosip2 don't send REFRESH, so if it's expired, then, drop subscription */
            if (now - transaction->birth_time > js->s_reg_period) {

              osip_transaction_t *transaction_notify =  _eXosip_find_last_inc_notify(js, jd);
              if (transaction_notify==NULL || (transaction_notify->state == NIST_TERMINATED && now - transaction_notify->birth_time > js->s_reg_period)) {
                /* NOTIFY is usually removed from the list of transaction.... */
                REMOVE_ELEMENT (excontext->j_subscribes, js);
                _eXosip_subscription_free (excontext, js);
                _eXosip_wakeup (excontext);
                return;
              }
            }
          } else {
            osip_message_get_expires (transaction->orig_request, 0, &expires);
            if (expires == NULL || expires->hvalue == NULL) {
            }
            else if (0 == strcmp (expires->hvalue, "0")) {
              /* In TCP mode, we don't have enough time to authenticate */
              REMOVE_ELEMENT (excontext->j_subscribes, js);
              _eXosip_subscription_free (excontext, js);
              _eXosip_wakeup (excontext);
              return;
            }
          }
        }
      }

      for (jd = js->s_dialogs; jd != NULL;) {
        jdnext = jd->next;
        _eXosip_release_finished_transactions_for_subscription (excontext, jd);

        if (jd->d_dialog == NULL || jd->d_dialog->state == DIALOG_EARLY) {
          if (js->s_out_tr != NULL && js->s_out_tr->birth_time + 64 < now) {    /* Wait a max of 2 minutes */
            /* destroy non established contexts after max of 64 sec */
            REMOVE_ELEMENT (excontext->j_subscribes, js);
            _eXosip_subscription_free (excontext, js);
            _eXosip_wakeup (excontext);
            return;
          }
        }

        jd = jdnext;
      }
    }
    js = jsnext;
  }

}

void
_eXosip_release_terminated_in_subscriptions (struct eXosip_t *excontext)
{
  time_t now = osip_getsystemtime (NULL);
  eXosip_dialog_t *jd;
  eXosip_dialog_t *jdnext;
  eXosip_notify_t *jn;
  eXosip_notify_t *jnnext;

  for (jn = excontext->j_notifies; jn != NULL;) {
    jnnext = jn->next;

    for (jd = jn->n_dialogs; jd != NULL;) {
      osip_transaction_t *transaction_notify;

      jdnext = jd->next;

      /* if a SUBSCRIBE is rejected, the context will be released automatically */
      if (jn->n_inc_tr->state == NIST_TERMINATED) {
	if (jn->n_inc_tr->last_response==NULL) {
	  REMOVE_ELEMENT (excontext->j_notifies, jn);
	  _eXosip_notify_free (excontext, jn);
	  break;
	} else if (jn->n_inc_tr->last_response->status_code >= 300) {
	  REMOVE_ELEMENT (excontext->j_notifies, jn);
	  _eXosip_notify_free (excontext, jn);
	  break;
	}
      }
      
      _eXosip_release_finished_transactions_for_subscription (excontext, jd);

      transaction_notify =  _eXosip_find_last_out_notify(jn, jd);
      if (transaction_notify!=NULL && transaction_notify->state == NICT_TERMINATED && now > jn->n_ss_expires ) {
        REMOVE_ELEMENT (excontext->j_notifies, jn);
        _eXosip_notify_free (excontext, jn);
	break;
      }

      jd = jdnext;
    }
    jn = jnnext;
  }
}

#endif
