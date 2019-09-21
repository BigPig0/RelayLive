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
#include <osip2/osip_condv.h>

static int _eXosip_event_fill_messages (eXosip_event_t * je, osip_transaction_t * tr);

static int
_eXosip_event_fill_messages (eXosip_event_t * je, osip_transaction_t * tr)
{
  int i;

  if (tr != NULL && tr->orig_request != NULL) {
    i = osip_message_clone (tr->orig_request, &je->request);
    if (i != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "failed to clone request for event\n"));
    }
  }
  if (tr != NULL && tr->last_response != NULL) {
    i = osip_message_clone (tr->last_response, &je->response);
    if (i != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "failed to clone response for event\n"));
    }
  }
  if (tr != NULL && tr->ack != NULL) {
    i = osip_message_clone (tr->ack, &je->ack);
    if (i != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "failed to clone ACK for event\n"));
    }
  }
  return OSIP_SUCCESS;
}

eXosip_event_t *
_eXosip_event_init_for_call (int type, eXosip_call_t * jc, eXosip_dialog_t * jd, osip_transaction_t * tr)
{
  eXosip_event_t *je;

  if (jc == NULL)
    return NULL;
  _eXosip_event_init (&je, type);
  if (je == NULL)
    return NULL;

  je->cid = jc->c_id;
  if (jd != NULL)
    je->did = jd->d_id;
  if (tr != NULL)
    je->tid = tr->transactionid;

  je->external_reference = jc->external_reference;

  _eXosip_event_fill_messages (je, tr);
  return je;
}

#ifndef MINISIZE

eXosip_event_t *
_eXosip_event_init_for_subscription (int type, eXosip_subscribe_t * js, eXosip_dialog_t * jd, osip_transaction_t * tr)
{
  eXosip_event_t *je;

  if (js == NULL)
    return NULL;
  _eXosip_event_init (&je, type);
  if (je == NULL)
    return NULL;

  je->sid = js->s_id;
  if (jd != NULL)
    je->did = jd->d_id;
  if (tr != NULL)
    je->tid = tr->transactionid;

  je->ss_status = js->s_ss_status;
  je->ss_reason = js->s_ss_reason;

  /* je->external_reference = js->external_reference; */

  _eXosip_event_fill_messages (je, tr);

  return je;
}

eXosip_event_t *
_eXosip_event_init_for_notify (int type, eXosip_notify_t * jn, eXosip_dialog_t * jd, osip_transaction_t * tr)
{
  eXosip_event_t *je;

  if (jn == NULL)
    return NULL;
  _eXosip_event_init (&je, type);
  if (je == NULL)
    return NULL;

  je->nid = jn->n_id;
  if (jd != NULL)
    je->did = jd->d_id;
  if (tr != NULL)
    je->tid = tr->transactionid;

  je->ss_status = jn->n_ss_status;
  je->ss_reason = jn->n_ss_reason;

  /*je->external_reference = jc->external_reference; */

  _eXosip_event_fill_messages (je, tr);

  return je;
}

#endif

eXosip_event_t *
_eXosip_event_init_for_reg (int type, eXosip_reg_t * jr, osip_transaction_t * tr)
{
  eXosip_event_t *je;

  if (jr == NULL)
    return NULL;
  _eXosip_event_init (&je, type);
  if (je == NULL)
    return NULL;
  je->rid = jr->r_id;

  _eXosip_event_fill_messages (je, tr);
  return je;
}

eXosip_event_t *
_eXosip_event_init_for_message (int type, osip_transaction_t * tr)
{
  eXosip_event_t *je;

  _eXosip_event_init (&je, type);
  if (je == NULL)
    return NULL;

  if (tr != NULL)
    je->tid = tr->transactionid;

  _eXosip_event_fill_messages (je, tr);

  return je;
}

int
_eXosip_event_init (eXosip_event_t ** je, int type)
{
  *je = (eXosip_event_t *) osip_malloc (sizeof (eXosip_event_t));
  if (*je == NULL)
    return OSIP_NOMEM;

  memset (*je, 0, sizeof (eXosip_event_t));
  (*je)->type = type;

#if !defined(_WIN32_WCE)
  if (type == EXOSIP_CALL_NOANSWER) {
    sprintf ((*je)->textinfo, "No answer for this Call!");
  }
  else if (type == EXOSIP_CALL_PROCEEDING) {
    sprintf ((*je)->textinfo, "Call is being processed!");
  }
  else if (type == EXOSIP_CALL_RINGING) {
    sprintf ((*je)->textinfo, "Remote phone is ringing!");
  }
  else if (type == EXOSIP_CALL_ANSWERED) {
    sprintf ((*je)->textinfo, "Remote phone has answered!");
  }
  else if (type == EXOSIP_CALL_REDIRECTED) {
    sprintf ((*je)->textinfo, "Call is redirected!");
  }
  else if (type == EXOSIP_CALL_REQUESTFAILURE) {
    sprintf ((*je)->textinfo, "4xx received for Call!");
  }
  else if (type == EXOSIP_CALL_SERVERFAILURE) {
    sprintf ((*je)->textinfo, "5xx received for Call!");
  }
  else if (type == EXOSIP_CALL_GLOBALFAILURE) {
    sprintf ((*je)->textinfo, "6xx received for Call!");
  }
  else if (type == EXOSIP_CALL_INVITE) {
    sprintf ((*je)->textinfo, "New call received!");
  }
  else if (type == EXOSIP_CALL_ACK) {
    sprintf ((*je)->textinfo, "ACK received!");
  }
  else if (type == EXOSIP_CALL_CANCELLED) {
    sprintf ((*je)->textinfo, "Call has been cancelled!");
  }
  else if (type == EXOSIP_CALL_REINVITE) {
    sprintf ((*je)->textinfo, "INVITE within call received!");
  }
  else if (type == EXOSIP_CALL_CLOSED) {
    sprintf ((*je)->textinfo, "Bye Received!");
  }
  else if (type == EXOSIP_CALL_RELEASED) {
    sprintf ((*je)->textinfo, "Call Context is released!");
  }
  else if (type == EXOSIP_REGISTRATION_SUCCESS) {
    sprintf ((*je)->textinfo, "User is successfully registred!");
  }
  else if (type == EXOSIP_REGISTRATION_FAILURE) {
    sprintf ((*je)->textinfo, "Registration failed!");
  }
  else if (type == EXOSIP_CALL_MESSAGE_NEW) {
    sprintf ((*je)->textinfo, "New request received!");
  }
  else if (type == EXOSIP_CALL_MESSAGE_PROCEEDING) {
    sprintf ((*je)->textinfo, "request is being processed!");
  }
  else if (type == EXOSIP_CALL_MESSAGE_ANSWERED) {
    sprintf ((*je)->textinfo, "2xx received for request!");
  }
  else if (type == EXOSIP_CALL_MESSAGE_REDIRECTED) {
    sprintf ((*je)->textinfo, "3xx received for request!");
  }
  else if (type == EXOSIP_CALL_MESSAGE_REQUESTFAILURE) {
    sprintf ((*je)->textinfo, "4xx received for request!");
  }
  else if (type == EXOSIP_CALL_MESSAGE_SERVERFAILURE) {
    sprintf ((*je)->textinfo, "5xx received for request!");
  }
  else if (type == EXOSIP_CALL_MESSAGE_GLOBALFAILURE) {
    sprintf ((*je)->textinfo, "5xx received for request!");
  }
  else if (type == EXOSIP_MESSAGE_NEW) {
    sprintf ((*je)->textinfo, "New request outside call received!");
  }
  else if (type == EXOSIP_MESSAGE_PROCEEDING) {
    sprintf ((*je)->textinfo, "request outside call is being processed!");
  }
  else if (type == EXOSIP_MESSAGE_ANSWERED) {
    sprintf ((*je)->textinfo, "2xx received for request outside call!");
  }
  else if (type == EXOSIP_MESSAGE_REDIRECTED) {
    sprintf ((*je)->textinfo, "3xx received for request outside call!");
  }
  else if (type == EXOSIP_MESSAGE_REQUESTFAILURE) {
    sprintf ((*je)->textinfo, "4xx received for request outside call!");
  }
  else if (type == EXOSIP_MESSAGE_SERVERFAILURE) {
    sprintf ((*je)->textinfo, "5xx received for request outside call!");
  }
  else if (type == EXOSIP_MESSAGE_GLOBALFAILURE) {
    sprintf ((*je)->textinfo, "5xx received for request outside call!");
  }
  else if (type == EXOSIP_SUBSCRIPTION_NOANSWER) {
    sprintf ((*je)->textinfo, "No answer for this SUBSCRIBE!");
  }
  else if (type == EXOSIP_SUBSCRIPTION_PROCEEDING) {
    sprintf ((*je)->textinfo, "SUBSCRIBE is being processed!");
  }
  else if (type == EXOSIP_SUBSCRIPTION_ANSWERED) {
    sprintf ((*je)->textinfo, "2xx received for SUBSCRIBE!");
  }
  else if (type == EXOSIP_SUBSCRIPTION_REDIRECTED) {
    sprintf ((*je)->textinfo, "3xx received for SUBSCRIBE!");
  }
  else if (type == EXOSIP_SUBSCRIPTION_REQUESTFAILURE) {
    sprintf ((*je)->textinfo, "4xx received for SUBSCRIBE!");
  }
  else if (type == EXOSIP_SUBSCRIPTION_SERVERFAILURE) {
    sprintf ((*je)->textinfo, "5xx received for SUBSCRIBE!");
  }
  else if (type == EXOSIP_SUBSCRIPTION_GLOBALFAILURE) {
    sprintf ((*je)->textinfo, "5xx received for SUBSCRIBE!");
  }
  else if (type == EXOSIP_SUBSCRIPTION_NOTIFY) {
    sprintf ((*je)->textinfo, "NOTIFY request for subscription!");
  }
  else if (type == EXOSIP_IN_SUBSCRIPTION_NEW) {
    sprintf ((*je)->textinfo, "New incoming SUBSCRIBE!");
  }
  else {
    (*je)->textinfo[0] = '\0';
  }
#endif
  return OSIP_SUCCESS;
}

void
eXosip_event_free (eXosip_event_t * je)
{
  if (je == NULL)
    return;
  if (je->request != NULL)
    osip_message_free (je->request);
  if (je->response != NULL)
    osip_message_free (je->response);
  if (je->ack != NULL)
    osip_message_free (je->ack);
  osip_free (je);
}

void
_eXosip_report_event (struct eXosip_t *excontext, eXosip_event_t * je, osip_message_t * sip)
{
  if (je != NULL) {
    _eXosip_event_add (excontext, je);
  }
}

void
_eXosip_report_call_event (struct eXosip_t *excontext, int evt, eXosip_call_t * jc, eXosip_dialog_t * jd, osip_transaction_t * tr)
{
  eXosip_event_t *je;

  je = _eXosip_event_init_for_call (evt, jc, jd, tr);
  _eXosip_report_event (excontext, je, NULL);
}

int
_eXosip_event_add (struct eXosip_t *excontext, eXosip_event_t * je)
{
  int i = osip_fifo_add (excontext->j_events, (void *) je);

#ifndef OSIP_MONOTHREAD
#if !defined (_WIN32_WCE)
  osip_cond_signal ((struct osip_cond *) excontext->j_cond);
#endif
#endif

  eXosip_wakeup_event (excontext);
  return i;
}

#ifdef OSIP_MONOTHREAD

eXosip_event_t *
eXosip_event_wait (struct eXosip_t * excontext, int tv_s, int tv_ms)
{
  eXosip_event_t *je = NULL;

  je = (eXosip_event_t *) osip_fifo_tryget (excontext->j_events);
  if (je != NULL)
    return je;

  eXosip_lock (excontext);
  _eXosip_retransmit_lost200ok (excontext);
  eXosip_unlock (excontext);

  return NULL;
}

#else

eXosip_event_t *
eXosip_event_wait (struct eXosip_t * excontext, int tv_s, int tv_ms)
{
  eXosip_event_t *je = NULL;

  fd_set fdset;
  struct timeval tv;
  int max, i;

  if (excontext==NULL) {
    return NULL;
  }

  FD_ZERO (&fdset);
#if defined (WIN32) || defined (_WIN32_WCE)
  FD_SET ((unsigned int) jpipe_get_read_descr (excontext->j_socketctl_event), &fdset);
#else
  FD_SET (jpipe_get_read_descr (excontext->j_socketctl_event), &fdset);
#endif
  max = jpipe_get_read_descr (excontext->j_socketctl_event);

  je = (eXosip_event_t *) osip_fifo_tryget (excontext->j_events);
  if (je != NULL)
    return je;

  tv.tv_sec = 0;
  tv.tv_usec = 0;
  i = select (max + 1, &fdset, NULL, NULL, &tv);
  if (FD_ISSET (jpipe_get_read_descr (excontext->j_socketctl_event), &fdset)) {
    char buf[500];

    jpipe_read (excontext->j_socketctl_event, buf, 499);
  }

  eXosip_lock (excontext);
  _eXosip_retransmit_lost200ok (excontext);
  eXosip_unlock (excontext);

  FD_ZERO (&fdset);
#if defined (WIN32) || defined (_WIN32_WCE)
  FD_SET ((unsigned int) jpipe_get_read_descr (excontext->j_socketctl_event), &fdset);
#else
  FD_SET (jpipe_get_read_descr (excontext->j_socketctl_event), &fdset);
#endif
  tv.tv_sec = tv_s;
  tv.tv_usec = tv_ms * 1000;

  if (tv_s == 0 && tv_ms == 0)
    return NULL;

  i = select (max + 1, &fdset, NULL, NULL, &tv);
  if (i <= 0)
    return OSIP_SUCCESS;

  if (excontext->j_stop_ua)
    return NULL;

  if (FD_ISSET (jpipe_get_read_descr (excontext->j_socketctl_event), &fdset)) {
    char buf[500];

    jpipe_read (excontext->j_socketctl_event, buf, 499);
  }

  je = (eXosip_event_t *) osip_fifo_tryget (excontext->j_events);
  if (je != NULL)
    return je;

  return je;
}

int
eXosip_event_geteventsocket (struct eXosip_t *excontext)
{
  return jpipe_get_read_descr (excontext->j_socketctl_event);
}

eXosip_event_t *
eXosip_event_get (struct eXosip_t * excontext)
{
  eXosip_event_t *je;
  fd_set fdset;
  struct timeval tv;
  int max;

  FD_ZERO (&fdset);
#if defined (WIN32) || defined (_WIN32_WCE)
  FD_SET ((unsigned int) jpipe_get_read_descr (excontext->j_socketctl_event), &fdset);
#else
  FD_SET (jpipe_get_read_descr (excontext->j_socketctl_event), &fdset);
#endif
  max = jpipe_get_read_descr (excontext->j_socketctl_event);

  tv.tv_sec = 0;
  tv.tv_usec = 0;
  select (max + 1, &fdset, NULL, NULL, &tv);
  if (FD_ISSET (jpipe_get_read_descr (excontext->j_socketctl_event), &fdset)) {
    char buf[500];

    jpipe_read (excontext->j_socketctl_event, buf, 499);
  }

  je = (eXosip_event_t *) osip_fifo_get (excontext->j_events);
  return je;
}

#endif
