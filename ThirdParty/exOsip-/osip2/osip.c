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
#include "xixt.h"

#include <osip2/osip_dialog.h>

#if defined(HAVE_DICT_DICT_H)
#include <dict/dict.h>
#endif

void
osip_response_get_destination (osip_message_t * response, char **address, int *portnum)
{
  osip_via_t *via;
  char *host = NULL;
  int port = 0;

  via = (osip_via_t *) osip_list_get (&response->vias, 0);
  if (via) {
    osip_generic_param_t *maddr;
    osip_generic_param_t *received;
    osip_generic_param_t *rport;

    osip_via_param_get_byname (via, "maddr", &maddr);
    osip_via_param_get_byname (via, "received", &received);
    osip_via_param_get_byname (via, "rport", &rport);
    /* 1: user should not use the provided information
       (host and port) if they are using a reliable
       transport. Instead, they should use the already
       open socket attached to this transaction. */
    /* 2: check maddr and multicast usage */
    if (maddr != NULL)
      host = maddr->gvalue;
    /* we should check if this is a multicast address and use
       set the "ttl" in this case. (this must be done in the
       UDP message (not at the SIP layer) */
    else if (received != NULL)
      host = received->gvalue;
    else
      host = via->host;

    if (rport == NULL || rport->gvalue == NULL) {
      if (via->port != NULL)
        port = osip_atoi (via->port);
      else
        port = 5060;
    }
    else
      port = osip_atoi (rport->gvalue);
  }
  *portnum = port;
  if (host != NULL)
    *address = osip_strdup (host);
  else
    *address = NULL;
}

static int
osip_ixt_lock (osip_t * osip)
{
#ifndef OSIP_MONOTHREAD
  return osip_mutex_lock (osip->ixt_fastmutex);
#else
  return OSIP_SUCCESS;
#endif
}

static int
osip_ixt_unlock (osip_t * osip)
{
#ifndef OSIP_MONOTHREAD
  return osip_mutex_unlock (osip->ixt_fastmutex);
#else
  return OSIP_SUCCESS;
#endif
}

int osip_id_mutex_lock (osip_t * osip);
int osip_id_mutex_unlock (osip_t * osip);

int
osip_id_mutex_lock (osip_t * osip)
{
#ifndef OSIP_MONOTHREAD
  return osip_mutex_lock (osip->id_mutex);
#else
  return OSIP_SUCCESS;
#endif
}

int
osip_id_mutex_unlock (osip_t * osip)
{
#ifndef OSIP_MONOTHREAD
  return osip_mutex_unlock (osip->id_mutex);
#else
  return OSIP_SUCCESS;
#endif
}

/* these are for transactions that would need retransmission not handled by state machines */
static void
osip_add_ixt (osip_t * osip, ixt_t * ixt)
{
  /* add in list osip_t->ixt */
  osip_ixt_lock (osip);
  osip_list_add (&osip->ixt_retransmissions, (void *) ixt, 0);
  osip_ixt_unlock (osip);
}

static int
ixt_init (ixt_t ** ixt)
{
  ixt_t *pixt;

  *ixt = pixt = (ixt_t *) osip_malloc (sizeof (ixt_t));
  if (pixt == NULL)
    return OSIP_NOMEM;
  pixt->dialog = NULL;
  pixt->msg2xx = NULL;
  pixt->ack = NULL;
  pixt->interval = DEFAULT_T1;
  osip_gettimeofday (&pixt->start, NULL);
  add_gettimeofday (&pixt->start, pixt->interval + 10);
  pixt->counter = 10;
  pixt->dest = NULL;
  pixt->port = 5060;
  pixt->sock = -1;
  return OSIP_SUCCESS;
}

static void
ixt_free (ixt_t * ixt)
{
  osip_message_free (ixt->ack);
  osip_message_free (ixt->msg2xx);
  osip_free (ixt->dest);
  osip_free (ixt);
}

/* usefull for UAs */
void
osip_start_200ok_retransmissions (osip_t * osip, osip_dialog_t * dialog, osip_message_t * msg200ok, int sock)
{
  int i;
  ixt_t *ixt;

  i = ixt_init (&ixt);
  if (i != 0)
    return;
  ixt->dialog = dialog;
  osip_message_clone (msg200ok, &ixt->msg2xx);
  ixt->sock = sock;
  osip_response_get_destination (msg200ok, &ixt->dest, &ixt->port);
  osip_add_ixt (osip, ixt);
}

void
osip_start_ack_retransmissions (osip_t * osip, osip_dialog_t * dialog, osip_message_t * ack, char *dest, int port, int sock)
{
  int i;
  ixt_t *ixt;

  i = ixt_init (&ixt);
  if (i != 0)
    return;
  ixt->dialog = dialog;
  osip_message_clone (ack, &ixt->ack);
  ixt->dest = osip_strdup (dest);
  ixt->port = port;
  ixt->sock = sock;
  osip_add_ixt (osip, ixt);
}

/* we stop the 200ok when receiving the corresponding ack */
struct osip_dialog *
osip_stop_200ok_retransmissions (osip_t * osip, osip_message_t * ack)
{
  osip_dialog_t *dialog = NULL;
  int i;
  ixt_t *ixt;

  if (ack==NULL || ack->cseq==NULL || ack->cseq->number==NULL)
    return NULL;

  osip_ixt_lock (osip);
  for (i = 0; !osip_list_eol (&osip->ixt_retransmissions, i); i++) {
    ixt = (ixt_t *) osip_list_get (&osip->ixt_retransmissions, i);
    if (ixt->msg2xx == NULL || ixt->msg2xx->cseq == NULL || ixt->msg2xx->cseq->number == NULL)
      continue;
    if (osip_dialog_match_as_uas (ixt->dialog, ack) == 0  && strcmp (ixt->msg2xx->cseq->number, ack->cseq->number) == 0) {
      osip_list_remove (&osip->ixt_retransmissions, i);
      dialog = ixt->dialog;
      ixt_free (ixt);
      break;
    }
  }
  osip_ixt_unlock (osip);
  return dialog;
}

/* when a dialog is destroyed by the application,
   it is safer to remove all ixt that are related to it */
void
osip_stop_retransmissions_from_dialog (osip_t * osip, osip_dialog_t * dialog)
{
  int i;
  ixt_t *ixt;

  osip_ixt_lock (osip);
  for (i = 0; !osip_list_eol (&osip->ixt_retransmissions, i); i++) {
    ixt = (ixt_t *) osip_list_get (&osip->ixt_retransmissions, i);
    if (ixt->dialog == dialog) {
      osip_list_remove (&osip->ixt_retransmissions, i);
      ixt_free (ixt);
      i--;
    }
  }
  osip_ixt_unlock (osip);
}

static void
ixt_retransmit (osip_t * osip, ixt_t * ixt, struct timeval *current)
{
  if (osip_timercmp (current, &ixt->start, >)) {
    ixt->interval = ixt->interval * 2;
    if (ixt->interval > DEFAULT_T2)
      ixt->interval = DEFAULT_T2;
    add_gettimeofday (&ixt->start, ixt->interval);
    if (ixt->ack != NULL)
      osip->cb_send_message (NULL, ixt->ack, ixt->dest, ixt->port, ixt->sock);
    else if (ixt->msg2xx != NULL)
      osip->cb_send_message (NULL, ixt->msg2xx, ixt->dest, ixt->port, ixt->sock);
    ixt->counter--;
  }
}

void
osip_retransmissions_execute (osip_t * osip)
{
  int i;
  ixt_t *ixt;
  struct timeval current;

  osip_gettimeofday (&current, NULL);

  osip_ixt_lock (osip);
  for (i = 0; !osip_list_eol (&osip->ixt_retransmissions, i); i++) {
    ixt = (ixt_t *) osip_list_get (&osip->ixt_retransmissions, i);
    ixt_retransmit (osip, ixt, &current);
    if (ixt->counter == 0) {
      /* remove it */
      osip_list_remove (&osip->ixt_retransmissions, i);
      ixt_free (ixt);
      i--;
    }
  }
  osip_ixt_unlock (osip);
}

int
osip_ict_lock (osip_t * osip)
{
#ifndef OSIP_MONOTHREAD
  return osip_mutex_lock (osip->ict_fastmutex);
#else
  return OSIP_SUCCESS;
#endif
}

int
osip_ict_unlock (osip_t * osip)
{
#ifndef OSIP_MONOTHREAD
  return osip_mutex_unlock (osip->ict_fastmutex);
#else
  return OSIP_SUCCESS;
#endif
}

int
osip_ist_lock (osip_t * osip)
{
#ifndef OSIP_MONOTHREAD
  return osip_mutex_lock (osip->ist_fastmutex);
#else
  return OSIP_SUCCESS;
#endif
}

int
osip_ist_unlock (osip_t * osip)
{
#ifndef OSIP_MONOTHREAD
  return osip_mutex_unlock (osip->ist_fastmutex);
#else
  return OSIP_SUCCESS;
#endif
}

int
osip_nict_lock (osip_t * osip)
{
#ifndef OSIP_MONOTHREAD
  return osip_mutex_lock (osip->nict_fastmutex);
#else
  return OSIP_SUCCESS;
#endif
}

int
osip_nict_unlock (osip_t * osip)
{
#ifndef OSIP_MONOTHREAD
  return osip_mutex_unlock (osip->nict_fastmutex);
#else
  return OSIP_SUCCESS;
#endif
}

int
osip_nist_lock (osip_t * osip)
{
#ifndef OSIP_MONOTHREAD
  return osip_mutex_lock (osip->nist_fastmutex);
#else
  return OSIP_SUCCESS;
#endif
}

int
osip_nist_unlock (osip_t * osip)
{
#ifndef OSIP_MONOTHREAD
  return osip_mutex_unlock (osip->nist_fastmutex);
#else
  return OSIP_SUCCESS;
#endif
}

#if defined(HAVE_DICT_DICT_H)
#define HSIZE           200

unsigned s_hash (const unsigned char *p);

unsigned
s_hash (const unsigned char *p)
{
  unsigned hash = 0;

  while (*p) {
    hash *= 31;
    hash ^= *p++;
  }
  return hash;
}
#endif

int
__osip_add_ict (osip_t * osip, osip_transaction_t * ict)
{
#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->ict_fastmutex);
#endif
#if defined(HAVE_DICT_DICT_H)
  {
    osip_generic_param_t *b_request = NULL;
    int rv = -99;

    osip_via_param_get_byname (ict->topvia, "branch", &b_request);
    if (b_request != NULL && b_request->gvalue != NULL)
      rv = dict_insert ((dict*)osip->osip_ict_hastable, b_request->gvalue, (void *) ict, FALSE);

    if (rv == 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "New key inserted in ict hastable `%s'\n", b_request->gvalue));
    }
    else if (rv != -99) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "already inserted `%s'\n", b_request->gvalue));
    }
  }
#endif
  osip_list_add (&osip->osip_ict_transactions, ict, -1);
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->ict_fastmutex);
#endif
  return OSIP_SUCCESS;
}

int
__osip_add_ist (osip_t * osip, osip_transaction_t * ist)
{
#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->ist_fastmutex);
#endif
#if defined(HAVE_DICT_DICT_H)
  {
    osip_generic_param_t *b_request = NULL;
    int rv = -99;

    osip_via_param_get_byname (ist->topvia, "branch", &b_request);
    if (b_request != NULL && b_request->gvalue != NULL)
      rv = dict_insert ((dict*)osip->osip_ist_hastable, b_request->gvalue, (void *) ist, FALSE);
    if (rv == 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "New key inserted in ist hastable `%s'\n", b_request->gvalue));
    }
    else if (rv != -99) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "already inserted `%s'\n", b_request->gvalue));
    }
  }
#endif
  osip_list_add (&osip->osip_ist_transactions, ist, -1);
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->ist_fastmutex);
#endif
  return OSIP_SUCCESS;
}

int
__osip_add_nict (osip_t * osip, osip_transaction_t * nict)
{
#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->nict_fastmutex);
#endif
#if defined(HAVE_DICT_DICT_H)
  {
    osip_generic_param_t *b_request = NULL;
    int rv = -99;

    osip_via_param_get_byname (nict->topvia, "branch", &b_request);
    if (b_request != NULL && b_request->gvalue != NULL)
      rv = dict_insert ((dict*)osip->osip_nict_hastable, b_request->gvalue, (void *) nict, FALSE);

    if (rv == 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "New key inserted in nict hastable `%s'\n", b_request->gvalue));
    }
    else if (rv != -99) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "already inserted `%s'\n", b_request->gvalue));
    }
  }
#endif
  osip_list_add (&osip->osip_nict_transactions, nict, -1);
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->nict_fastmutex);
#endif
  return OSIP_SUCCESS;
}

int
__osip_add_nist (osip_t * osip, osip_transaction_t * nist)
{
#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->nist_fastmutex);
#endif
#if defined(HAVE_DICT_DICT_H)
  {
    osip_generic_param_t *b_request = NULL;
    int rv = -99;

    osip_via_param_get_byname (nist->topvia, "branch", &b_request);
    if (b_request != NULL && b_request->gvalue != NULL)
      rv = dict_insert ((dict*)osip->osip_nist_hastable, b_request->gvalue, (void *) nist, FALSE);
    if (rv == 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "New key inserted in ict hastable `%s'\n", b_request->gvalue));
    }
    else if (rv != -99) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "already inserted `%s'\n", b_request->gvalue));
    }
  }
#endif
  osip_list_add (&osip->osip_nist_transactions, nist, -1);
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->nist_fastmutex);
#endif
  return OSIP_SUCCESS;
}

int
osip_remove_transaction (osip_t * osip, osip_transaction_t * tr)
{
  int i = -1;

  if (tr == NULL)
    return OSIP_BADPARAMETER;
  if (tr->ctx_type == ICT)
    i = __osip_remove_ict_transaction (osip, tr);
  else if (tr->ctx_type == IST)
    i = __osip_remove_ist_transaction (osip, tr);
  else if (tr->ctx_type == NICT)
    i = __osip_remove_nict_transaction (osip, tr);
  else if (tr->ctx_type == NIST)
    i = __osip_remove_nist_transaction (osip, tr);
  else
    return OSIP_BADPARAMETER;
  return i;
}

int
__osip_remove_ict_transaction (osip_t * osip, osip_transaction_t * ict)
{
  osip_list_iterator_t iterator;
  osip_transaction_t *tmp;

#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->ict_fastmutex);
#endif

#if defined(HAVE_DICT_DICT_H)
  {
    osip_generic_param_t *b_request = NULL;
    int rv;

    osip_via_param_get_byname (ict->topvia, "branch", &b_request);
    if (b_request != NULL && b_request->gvalue != NULL) {
      rv = dict_remove ((dict*)osip->osip_ict_hastable, b_request->gvalue, TRUE);
      if (rv == 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "New key deleted in ict hastable `%s'\n", b_request->gvalue));
      }
      else {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "key not removed `%s'\n", b_request->gvalue));
      }
    }
  }
#endif

  tmp = (osip_transaction_t *) osip_list_get_first (&osip->osip_ict_transactions, &iterator);
  while (osip_list_iterator_has_elem (iterator)) {
    if (tmp->transactionid == ict->transactionid) {
      osip_list_iterator_remove (&iterator);
#ifndef OSIP_MONOTHREAD
      osip_mutex_unlock (osip->ict_fastmutex);
#endif
      return OSIP_SUCCESS;
    }
    tmp = (osip_transaction_t *) osip_list_get_next (&iterator);
  }
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->ict_fastmutex);
#endif
  return OSIP_UNDEFINED_ERROR;
}

int
__osip_remove_ist_transaction (osip_t * osip, osip_transaction_t * ist)
{
  osip_list_iterator_t iterator;
  osip_transaction_t *tmp;

#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->ist_fastmutex);
#endif

#if defined(HAVE_DICT_DICT_H)
  {
    osip_generic_param_t *b_request = NULL;
    int rv;

    osip_via_param_get_byname (ist->topvia, "branch", &b_request);
    if (b_request != NULL && b_request->gvalue != NULL) {
      rv = dict_remove ((dict*)osip->osip_ist_hastable, b_request->gvalue, TRUE);

      if (rv == 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "New key deleted in ist hastable `%s'\n", b_request->gvalue));
      }
      else {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "key not removed `%s'\n", b_request->gvalue));
      }
    }
  }
#endif

  tmp = (osip_transaction_t *) osip_list_get_first (&osip->osip_ist_transactions, &iterator);
  while (osip_list_iterator_has_elem (iterator)) {
    if (tmp->transactionid == ist->transactionid) {
      osip_list_iterator_remove (&iterator);
#ifndef OSIP_MONOTHREAD
      osip_mutex_unlock (osip->ist_fastmutex);
#endif
      return OSIP_SUCCESS;
    }
    tmp = (osip_transaction_t *) osip_list_get_next (&iterator);
  }
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->ist_fastmutex);
#endif
  return OSIP_UNDEFINED_ERROR;
}

int
__osip_remove_nict_transaction (osip_t * osip, osip_transaction_t * nict)
{
  osip_list_iterator_t iterator;
  osip_transaction_t *tmp;

#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->nict_fastmutex);
#endif

#if defined(HAVE_DICT_DICT_H)
  {
    osip_generic_param_t *b_request = NULL;
    int rv;

    osip_via_param_get_byname (nict->topvia, "branch", &b_request);
    if (b_request != NULL && b_request->gvalue != NULL) {
      rv = dict_remove ((dict*)osip->osip_nict_hastable, b_request->gvalue, TRUE);

      if (rv == 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "New key deleted in nict hastable `%s'\n", b_request->gvalue));
      }
      else {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "key not removed `%s'\n", b_request->gvalue));
      }
    }
  }
#endif

  tmp = (osip_transaction_t *) osip_list_get_first (&osip->osip_nict_transactions, &iterator);
  while (osip_list_iterator_has_elem (iterator)) {
    if (tmp->transactionid == nict->transactionid) {
      osip_list_iterator_remove (&iterator);
#ifndef OSIP_MONOTHREAD
      osip_mutex_unlock (osip->nict_fastmutex);
#endif
      return OSIP_SUCCESS;
    }
    tmp = (osip_transaction_t *) osip_list_get_next (&iterator);
  }
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->nict_fastmutex);
#endif
  return OSIP_UNDEFINED_ERROR;
}

int
__osip_remove_nist_transaction (osip_t * osip, osip_transaction_t * nist)
{
  osip_list_iterator_t iterator;
  osip_transaction_t *tmp;

#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->nist_fastmutex);
#endif

#if defined(HAVE_DICT_DICT_H)
  {
    osip_generic_param_t *b_request = NULL;
    int rv;

    osip_via_param_get_byname (nist->topvia, "branch", &b_request);
    if (b_request != NULL && b_request->gvalue != NULL) {
      rv = dict_remove ((dict*)osip->osip_nist_hastable, b_request->gvalue, TRUE);

      if (rv == 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "New key deleted in ict hastable `%s'\n", b_request->gvalue));
      }
      else {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "key not removed `%s'\n", b_request->gvalue));
      }
    }
  }
#endif

  tmp = (osip_transaction_t *) osip_list_get_first (&osip->osip_nist_transactions, &iterator);
  while (osip_list_iterator_has_elem (iterator)) {
    if (tmp->transactionid == nist->transactionid) {
      osip_list_iterator_remove (&iterator);
#ifndef OSIP_MONOTHREAD
      osip_mutex_unlock (osip->nist_fastmutex);
#endif
      return OSIP_SUCCESS;
    }
    tmp = (osip_transaction_t *) osip_list_get_next (&iterator);
  }
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->nist_fastmutex);
#endif
  return OSIP_UNDEFINED_ERROR;
}

int
osip_find_transaction_and_add_event (osip_t * osip, osip_event_t * evt)
{
  osip_transaction_t *transaction = __osip_find_transaction (osip, evt, 1);

  if (transaction == NULL)
    return OSIP_UNDEFINED_ERROR;
  return OSIP_SUCCESS;
}

#ifdef OSIP_MONOTHREAD
osip_transaction_t *
osip_find_transaction (osip_t * osip, osip_event_t * evt)
{
  return __osip_find_transaction (osip, evt, 0);
}
#endif

osip_transaction_t *
__osip_find_transaction (osip_t * osip, osip_event_t * evt, int consume)
{
  osip_transaction_t *transaction = NULL;
  osip_list_t *transactions = NULL;

#ifndef OSIP_MONOTHREAD
  struct osip_mutex *mut = NULL;
#endif

  if (evt == NULL || evt->sip == NULL || evt->sip->cseq == NULL)
    return NULL;

  if (EVT_IS_INCOMINGMSG (evt)) {
    if (MSG_IS_REQUEST (evt->sip)) {
      if (0 == strcmp (evt->sip->cseq->method, "INVITE")
          || 0 == strcmp (evt->sip->cseq->method, "ACK")) {
        transactions = &osip->osip_ist_transactions;
#ifndef OSIP_MONOTHREAD
        mut = osip->ist_fastmutex;
#endif
      }
      else {
        transactions = &osip->osip_nist_transactions;
#ifndef OSIP_MONOTHREAD
        mut = osip->nist_fastmutex;
#endif
      }
    }
    else {
      if (0 == strcmp (evt->sip->cseq->method, "INVITE")) {
        transactions = &osip->osip_ict_transactions;
#ifndef OSIP_MONOTHREAD
        mut = osip->ict_fastmutex;
#endif
      }
      else {
        transactions = &osip->osip_nict_transactions;
#ifndef OSIP_MONOTHREAD
        mut = osip->nict_fastmutex;
#endif
      }
    }
  }
  else if (EVT_IS_OUTGOINGMSG (evt)) {
    if (MSG_IS_RESPONSE (evt->sip)) {
      if (0 == strcmp (evt->sip->cseq->method, "INVITE")) {
        transactions = &osip->osip_ist_transactions;
#ifndef OSIP_MONOTHREAD
        mut = osip->ist_fastmutex;
#endif
      }
      else {
        transactions = &osip->osip_nist_transactions;
#ifndef OSIP_MONOTHREAD
        mut = osip->nist_fastmutex;
#endif
      }
    }
    else {
      if (0 == strcmp (evt->sip->cseq->method, "INVITE")
          || 0 == strcmp (evt->sip->cseq->method, "ACK")) {
        transactions = &osip->osip_ict_transactions;
#ifndef OSIP_MONOTHREAD
        mut = osip->ict_fastmutex;
#endif
      }
      else {
        transactions = &osip->osip_nict_transactions;
#ifndef OSIP_MONOTHREAD
        mut = osip->nict_fastmutex;
#endif
      }
    }
  }
  if (transactions == NULL)
    return NULL;                /* not a message??? */

#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (mut);
#endif
  transaction = osip_transaction_find (transactions, evt);
  if (consume == 1) {           /* we add the event before releasing the mutex!! */
    if (transaction != NULL) {
      osip_transaction_add_event (transaction, evt);
#ifndef OSIP_MONOTHREAD
      osip_mutex_unlock (mut);
#endif
      return transaction;
    }
  }
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (mut);
#endif

  return transaction;
}

osip_transaction_t *
osip_create_transaction (osip_t * osip, osip_event_t * evt)
{
  osip_transaction_t *transaction;
  int i;
  osip_fsm_type_t ctx_type;

  if (evt == NULL)
    return NULL;
  if (evt->sip == NULL)
    return NULL;

  /* make sure the request's method reflect the cseq value. */
  if (MSG_IS_REQUEST (evt->sip)) {
    /* delete request where cseq method does not match
       the method in request-line */
    if (evt->sip->cseq == NULL || evt->sip->cseq->method == NULL || evt->sip->sip_method == NULL) {
      return NULL;
    }
    if (0 != strcmp (evt->sip->cseq->method, evt->sip->sip_method)) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "core module: Discard invalid message with method!=cseq!\n"));
      return NULL;
    }
  }

  if (MSG_IS_ACK (evt->sip))    /* ACK never create transactions */
    return NULL;

  if (EVT_IS_INCOMINGREQ (evt)) {
    /* we create a new context for this incoming request */
    if (0 == strcmp (evt->sip->cseq->method, "INVITE"))
      ctx_type = IST;
    else
      ctx_type = NIST;
  }
  else if (EVT_IS_OUTGOINGREQ (evt)) {
    if (0 == strcmp (evt->sip->cseq->method, "INVITE"))
      ctx_type = ICT;
    else
      ctx_type = NICT;
  }
  else {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Cannot build a transaction for this message!\n"));
    return NULL;
  }

  i = osip_transaction_init (&transaction, ctx_type, osip, evt->sip);
  if (i != 0) {
    return NULL;
  }
  evt->transactionid = transaction->transactionid;
  return transaction;
}

osip_transaction_t *
osip_transaction_find (osip_list_t * transactions, osip_event_t * evt)
{
  osip_list_iterator_t iterator;
  osip_transaction_t *transaction;
  osip_t *osip = NULL;

  transaction = (osip_transaction_t *) osip_list_get_first (transactions, &iterator);
  if (transaction != NULL)
    osip = (osip_t *) transaction->config;
  if (osip == NULL)
    return NULL;

  if (EVT_IS_INCOMINGREQ (evt)) {
#ifdef HAVE_DICT_DICT_H
    /* search in hastable! */
    osip_generic_param_t *b_request;
    osip_via_t *topvia_request;

    topvia_request = osip_list_get (&evt->sip->vias, 0);
    if (topvia_request == NULL) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Remote UA is not compliant: missing a Via header!\n"));
      return NULL;
    }
    osip_via_param_get_byname (topvia_request, "branch", &b_request);
    if (b_request != NULL && b_request->gvalue != NULL) {
      if (MSG_IS_INVITE (evt->sip) || MSG_IS_ACK (evt->sip)) {
        transaction = (osip_transaction_t *) dict_search ((dict*)osip->osip_ist_hastable, b_request->gvalue);
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Find matching Via header for INVITE(ACK) REQUEST!\n"));
        if (transaction != NULL)
          return transaction;
      }
      else {
        transaction = (osip_transaction_t *) dict_search ((dict*)osip->osip_nist_hastable, b_request->gvalue);
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Find matching Via header for NON-INVITE REQUEST!\n"));
        if (transaction != NULL)
          return transaction;
      }
    }
#endif

    transaction = (osip_transaction_t *) osip_list_get_first (transactions, &iterator);
    while (osip_list_iterator_has_elem (iterator)) {
      if (0 == __osip_transaction_matching_request_osip_to_xist_17_2_3 (transaction, evt->sip))
        return transaction;
      transaction = (osip_transaction_t *) osip_list_get_next (&iterator);
    }
  }
  else if (EVT_IS_INCOMINGRESP (evt)) {
#ifdef HAVE_DICT_DICT_H
    /* search in hastable! */
    osip_generic_param_t *b_request;
    osip_via_t *topvia_request;

    topvia_request = osip_list_get (&evt->sip->vias, 0);
    if (topvia_request == NULL) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Remote UA is not compliant: missing a Via header!\n"));
      return NULL;
    }
    osip_via_param_get_byname (topvia_request, "branch", &b_request);
    if (b_request != NULL && b_request->gvalue != NULL) {
      if (MSG_IS_RESPONSE_FOR (evt->sip, "INVITE")) {
        transaction = (osip_transaction_t *) dict_search ((dict*)osip->osip_ict_hastable, b_request->gvalue);
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Find matching Via header for INVITE ANSWER!\n"));
        if (transaction != NULL)
          return transaction;
      }
      else {
        transaction = (osip_transaction_t *) dict_search ((dict*)osip->osip_nict_hastable, b_request->gvalue);
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Find matching Via header for NON-INVITE ANSWER!\n"));
        if (transaction != NULL)
          return transaction;
      }
    }
#endif

    transaction = (osip_transaction_t *) osip_list_get_first (transactions, &iterator);
    while (osip_list_iterator_has_elem (iterator)) {
      if (0 == __osip_transaction_matching_response_osip_to_xict_17_1_3 (transaction, evt->sip))
        return transaction;
      transaction = (osip_transaction_t *) osip_list_get_next (&iterator);
    }
  }
  else {                        /* handle OUTGOING message */
    /* THE TRANSACTION ID MUST BE SET */
    transaction = (osip_transaction_t *) osip_list_get_first (transactions, &iterator);
    while (osip_list_iterator_has_elem (iterator)) {
      if (transaction->transactionid == evt->transactionid)
        return transaction;
      transaction = (osip_transaction_t *) osip_list_get_next (&iterator);
    }
  }
  return NULL;
}

int
osip_init (osip_t ** osip)
{
  static int ref_count = 0;

  if (ref_count == 0) {
    ref_count++;
    /* load the parser configuration */
    parser_init ();
  }

  *osip = (osip_t *) osip_malloc (sizeof (osip_t));
  if (*osip == NULL)
    return OSIP_NOMEM;          /* allocation failed */

  memset (*osip, 0, sizeof (osip_t));

#ifndef OSIP_MONOTHREAD
  (*osip)->ict_fastmutex = osip_mutex_init ();
  (*osip)->ist_fastmutex = osip_mutex_init ();
  (*osip)->nict_fastmutex = osip_mutex_init ();
  (*osip)->nist_fastmutex = osip_mutex_init ();

  (*osip)->ixt_fastmutex = osip_mutex_init ();
  (*osip)->id_mutex = osip_mutex_init ();
#endif

  osip_list_init (&(*osip)->osip_ict_transactions);
  osip_list_init (&(*osip)->osip_ist_transactions);
  osip_list_init (&(*osip)->osip_nict_transactions);
  osip_list_init (&(*osip)->osip_nist_transactions);
  osip_list_init (&(*osip)->ixt_retransmissions);

  (*osip)->transactionid = 1;

#if defined(HAVE_DICT_DICT_H)
  (*osip)->osip_ict_hastable = (dict*)hashtable_dict_new ((dict_cmp_func) strcmp, (dict_hsh_func) s_hash, NULL, NULL, HSIZE);
  (*osip)->osip_ist_hastable = (dict*)hashtable_dict_new ((dict_cmp_func) strcmp, (dict_hsh_func) s_hash, NULL, NULL, HSIZE);
  (*osip)->osip_nict_hastable = (dict*)hashtable_dict_new ((dict_cmp_func) strcmp, (dict_hsh_func) s_hash, NULL, NULL, HSIZE);
  (*osip)->osip_nist_hastable = (dict*)hashtable_dict_new ((dict_cmp_func) strcmp, (dict_hsh_func) s_hash, NULL, NULL, HSIZE);
#endif

  return OSIP_SUCCESS;
}

void
osip_release (osip_t * osip)
{
#ifndef OSIP_MONOTHREAD
  osip_mutex_destroy (osip->ict_fastmutex);
  osip_mutex_destroy (osip->ist_fastmutex);
  osip_mutex_destroy (osip->nict_fastmutex);
  osip_mutex_destroy (osip->nist_fastmutex);

  osip_mutex_destroy (osip->ixt_fastmutex);
  osip_mutex_destroy (osip->id_mutex);
#endif

  osip_free (osip);
}

void
osip_set_application_context (osip_t * osip, void *pointer)
{
  osip->application_context = pointer;
}

void *
osip_get_application_context (osip_t * osip)
{
  if (osip == NULL)
    return NULL;
  return osip->application_context;
}

int
osip_ict_execute (osip_t * osip)
{
  osip_transaction_t *transaction;
  osip_event_t *se;
  int more_event;
  osip_list_iterator_t iterator;
  void **array;
  int len;
  int index = 0;

  /* list must be copied because osip_transaction_execute() may change it */
#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->ict_fastmutex);
#endif
  len = osip_list_size (&osip->osip_ict_transactions);
  if (0 >= len) {
#ifndef OSIP_MONOTHREAD
    osip_mutex_unlock (osip->ict_fastmutex);
#endif
    return OSIP_SUCCESS;
  }
  array = osip_malloc (sizeof (void *) * len);
  if (array == NULL) {
#ifndef OSIP_MONOTHREAD
    osip_mutex_unlock (osip->ict_fastmutex);
#endif
    return OSIP_NOMEM;          /* OSIP_SUCCESS; */
  }
  transaction = (osip_transaction_t *) osip_list_get_first (&osip->osip_ict_transactions, &iterator);
  while (osip_list_iterator_has_elem (iterator)) {
    array[index++] = transaction;
    transaction = (osip_transaction_t *) osip_list_get_next (&iterator);
  }
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->ict_fastmutex);
#endif

  for (index = 0; index < len; ++index) {
    transaction = (osip_transaction_t *) array[index];
    more_event = 1;
    do {
      se = (osip_event_t *) osip_fifo_tryget (transaction->transactionff);
      if (se == NULL)           /* no more event for this transaction */
        more_event = 0;
      else
        osip_transaction_execute (transaction, se);
    }
    while (more_event == 1);
  }

  osip_free (array);

  return OSIP_SUCCESS;
}

int
osip_ist_execute (osip_t * osip)
{
  osip_transaction_t *transaction;
  osip_event_t *se;
  int more_event;
  osip_list_iterator_t iterator;
  void **array;
  int len;
  int index = 0;

  /* list must be copied because osip_transaction_execute() may change it */
#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->ist_fastmutex);
#endif
  len = osip_list_size (&osip->osip_ist_transactions);
  if (0 >= len) {
#ifndef OSIP_MONOTHREAD
    osip_mutex_unlock (osip->ist_fastmutex);
#endif
    return OSIP_SUCCESS;
  }
  array = osip_malloc (sizeof (void *) * len);
  if (array == NULL) {
#ifndef OSIP_MONOTHREAD
    osip_mutex_unlock (osip->ist_fastmutex);
#endif
    return OSIP_NOMEM;          /* OSIP_SUCCESS; */
  }
  transaction = (osip_transaction_t *) osip_list_get_first (&osip->osip_ist_transactions, &iterator);
  while (osip_list_iterator_has_elem (iterator)) {
    array[index++] = transaction;
    transaction = (osip_transaction_t *) osip_list_get_next (&iterator);
  }
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->ist_fastmutex);
#endif

  for (index = 0; index < len; ++index) {
    transaction = (osip_transaction_t *) array[index];
    more_event = 1;
    do {
      se = (osip_event_t *) osip_fifo_tryget (transaction->transactionff);
      if (se == NULL)           /* no more event for this transaction */
        more_event = 0;
      else
        osip_transaction_execute (transaction, se);
    }
    while (more_event == 1);

  }

  osip_free (array);

  return OSIP_SUCCESS;
}

int
osip_nict_execute (osip_t * osip)
{
  osip_transaction_t *transaction;
  osip_event_t *se;
  int more_event;
  osip_list_iterator_t iterator;
  void **array;
  int len;
  int index = 0;

  /* list must be copied because osip_transaction_execute() may change it */
#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->nict_fastmutex);
#endif
  len = osip_list_size (&osip->osip_nict_transactions);
  if (0 >= len) {
#ifndef OSIP_MONOTHREAD
    osip_mutex_unlock (osip->nict_fastmutex);
#endif
    return OSIP_SUCCESS;
  }
  array = osip_malloc (sizeof (void *) * len);
  if (array == NULL) {
#ifndef OSIP_MONOTHREAD
    osip_mutex_unlock (osip->nict_fastmutex);
#endif
    return OSIP_NOMEM;          /* OSIP_SUCCESS; */
  }
  transaction = (osip_transaction_t *) osip_list_get_first (&osip->osip_nict_transactions, &iterator);
  while (osip_list_iterator_has_elem (iterator)) {
    array[index++] = transaction;
    transaction = (osip_transaction_t *) osip_list_get_next (&iterator);
  }
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->nict_fastmutex);
#endif

  for (index = 0; index < len; ++index) {
    transaction = (osip_transaction_t *) array[index];
    more_event = 1;
    do {
      se = (osip_event_t *) osip_fifo_tryget (transaction->transactionff);
      if (se == NULL)           /* no more event for this transaction */
        more_event = 0;
      else
        osip_transaction_execute (transaction, se);
    }
    while (more_event == 1);
  }

  osip_free (array);

  return OSIP_SUCCESS;
}

int
osip_nist_execute (osip_t * osip)
{
  osip_transaction_t *transaction;
  osip_event_t *se;
  int more_event;
  osip_list_iterator_t iterator;
  void **array;
  int len;
  int index = 0;

  /* list must be copied because osip_transaction_execute() may change it */
#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->nist_fastmutex);
#endif
  len = osip_list_size (&osip->osip_nist_transactions);
  if (0 >= len) {
#ifndef OSIP_MONOTHREAD
    osip_mutex_unlock (osip->nist_fastmutex);
#endif
    return OSIP_SUCCESS;
  }
  array = osip_malloc (sizeof (void *) * len);
  if (array == NULL) {
#ifndef OSIP_MONOTHREAD
    osip_mutex_unlock (osip->nist_fastmutex);
#endif
    return OSIP_NOMEM;          /* OSIP_SUCCESS; */
  }
  transaction = (osip_transaction_t *) osip_list_get_first (&osip->osip_nist_transactions, &iterator);
  while (osip_list_iterator_has_elem (iterator)) {
    array[index++] = transaction;
    transaction = (osip_transaction_t *) osip_list_get_next (&iterator);
  }
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->nist_fastmutex);
#endif

  for (index = 0; index < len; ++index) {
    transaction = (osip_transaction_t *) array[index];
    more_event = 1;
    do {
      se = (osip_event_t *) osip_fifo_tryget (transaction->transactionff);
      if (se == NULL)           /* no more event for this transaction */
        more_event = 0;
      else
        osip_transaction_execute (transaction, se);
    }
    while (more_event == 1);
  }

  osip_free (array);

  return OSIP_SUCCESS;
}

void
osip_timers_gettimeout (osip_t * osip, struct timeval *lower_tv)
{
  struct timeval now;
  osip_transaction_t *tr;
  osip_list_iterator_t iterator;

  osip_gettimeofday (&now, NULL);
  lower_tv->tv_sec = now.tv_sec + 3600 * 24 * 365;      /* wake up evry year :-) */
  lower_tv->tv_usec = now.tv_usec;

#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->ict_fastmutex);
#endif
  /* handle ict timers */
  tr = (osip_transaction_t *) osip_list_get_first (&osip->osip_ict_transactions, &iterator);
  while (osip_list_iterator_has_elem (iterator)) {
    if (1 <= osip_fifo_size (tr->transactionff)) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO4, NULL, "1 Pending event already in transaction !\n"));
      lower_tv->tv_sec = 0;
      lower_tv->tv_usec = 0;
#ifndef OSIP_MONOTHREAD
      osip_mutex_unlock (osip->ict_fastmutex);
#endif
      return;
    }
    else {
      if (tr->state == ICT_CALLING)
        min_timercmp (lower_tv, &tr->ict_context->timer_b_start);
      if (tr->state == ICT_CALLING)
        min_timercmp (lower_tv, &tr->ict_context->timer_a_start);
      if (tr->state == ICT_COMPLETED)
        min_timercmp (lower_tv, &tr->ict_context->timer_d_start);
      if (osip_timercmp (&now, lower_tv, >)) {
        lower_tv->tv_sec = 0;
        lower_tv->tv_usec = 0;
#ifndef OSIP_MONOTHREAD
        osip_mutex_unlock (osip->ict_fastmutex);
#endif
        return;
      }
    }
    tr = (osip_transaction_t *) osip_list_get_next (&iterator);
  }
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->ict_fastmutex);
#endif

#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->ist_fastmutex);
#endif
  /* handle ist timers */
  tr = (osip_transaction_t *) osip_list_get_first (&osip->osip_ist_transactions, &iterator);
  while (osip_list_iterator_has_elem (iterator)) {
    if (tr->state == IST_CONFIRMED)
      min_timercmp (lower_tv, &tr->ist_context->timer_i_start);
    if (tr->state == IST_COMPLETED)
      min_timercmp (lower_tv, &tr->ist_context->timer_h_start);
    if (tr->state == IST_COMPLETED)
      min_timercmp (lower_tv, &tr->ist_context->timer_g_start);
    if (osip_timercmp (&now, lower_tv, >)) {
      lower_tv->tv_sec = 0;
      lower_tv->tv_usec = 0;
#ifndef OSIP_MONOTHREAD
      osip_mutex_unlock (osip->ist_fastmutex);
#endif
      return;
    }
    tr = (osip_transaction_t *) osip_list_get_next (&iterator);
  }
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->ist_fastmutex);
#endif

#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->nict_fastmutex);
#endif
  /* handle nict timers */
  tr = (osip_transaction_t *) osip_list_get_first (&osip->osip_nict_transactions, &iterator);
  while (osip_list_iterator_has_elem (iterator)) {
    if (tr->state == NICT_COMPLETED)
      min_timercmp (lower_tv, &tr->nict_context->timer_k_start);
    if (tr->state == NICT_PROCEEDING || tr->state == NICT_TRYING)
      min_timercmp (lower_tv, &tr->nict_context->timer_f_start);
    if (tr->state == NICT_PROCEEDING || tr->state == NICT_TRYING)
      min_timercmp (lower_tv, &tr->nict_context->timer_e_start);
    if (osip_timercmp (&now, lower_tv, >)) {
      lower_tv->tv_sec = 0;
      lower_tv->tv_usec = 0;
#ifndef OSIP_MONOTHREAD
      osip_mutex_unlock (osip->nict_fastmutex);
#endif
      return;
    }
    tr = (osip_transaction_t *) osip_list_get_next (&iterator);
  }
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->nict_fastmutex);
#endif

#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->nist_fastmutex);
#endif
  /* handle nist timers */
  tr = (osip_transaction_t *) osip_list_get_first (&osip->osip_nist_transactions, &iterator);
  while (osip_list_iterator_has_elem (iterator)) {
    if (tr->state == NIST_COMPLETED)
      min_timercmp (lower_tv, &tr->nist_context->timer_j_start);
    if (osip_timercmp (&now, lower_tv, >)) {
      lower_tv->tv_sec = 0;
      lower_tv->tv_usec = 0;
#ifndef OSIP_MONOTHREAD
      osip_mutex_unlock (osip->nist_fastmutex);
#endif
      return;
    }
    tr = (osip_transaction_t *) osip_list_get_next (&iterator);
  }
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->nist_fastmutex);
#endif

#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->ixt_fastmutex);
#endif
  {
    ixt_t *ixt;

    ixt = (ixt_t *) osip_list_get_first (&osip->ixt_retransmissions, &iterator);
    while (osip_list_iterator_has_elem (iterator)) {
      min_timercmp (lower_tv, &ixt->start);
      if (osip_timercmp (&now, lower_tv, >)) {
        lower_tv->tv_sec = 0;
        lower_tv->tv_usec = 0;
#ifndef OSIP_MONOTHREAD
        osip_mutex_unlock (osip->ixt_fastmutex);
#endif
        return;
      }

      ixt = (ixt_t *) osip_list_get_next (&iterator);
    }
  }
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->ixt_fastmutex);
#endif

  lower_tv->tv_sec = lower_tv->tv_sec - now.tv_sec;
  lower_tv->tv_usec = lower_tv->tv_usec - now.tv_usec;

  /* just make sure the value is correct! */
  if (lower_tv->tv_usec < 0) {
    lower_tv->tv_usec = lower_tv->tv_usec + 1000000;
    lower_tv->tv_sec--;
  }
  if (lower_tv->tv_sec < 0) {
    lower_tv->tv_sec = 0;
    lower_tv->tv_usec = 0;
  }
  if (lower_tv->tv_usec > 1000000) {
    lower_tv->tv_usec = lower_tv->tv_usec - 1000000;
    lower_tv->tv_sec++;
  }
  return;
}

void
osip_timers_ict_execute (osip_t * osip)
{
  osip_transaction_t *tr;
  osip_list_iterator_t iterator;

#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->ict_fastmutex);
#endif
  /* handle ict timers */
  tr = (osip_transaction_t *) osip_list_get_first (&osip->osip_ict_transactions, &iterator);
  while (osip_list_iterator_has_elem (iterator)) {
    osip_event_t *evt;

    if (1 <= osip_fifo_size (tr->transactionff)) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO4, NULL, "1 Pending event already in transaction !\n"));
    }
    else {
      evt = __osip_ict_need_timer_b_event (tr->ict_context, tr->state, tr->transactionid);
      if (evt != NULL)
        osip_fifo_add (tr->transactionff, evt);
      else {
        evt = __osip_ict_need_timer_a_event (tr->ict_context, tr->state, tr->transactionid);
        if (evt != NULL)
          osip_fifo_add (tr->transactionff, evt);
        else {
          evt = __osip_ict_need_timer_d_event (tr->ict_context, tr->state, tr->transactionid);
          if (evt != NULL)
            osip_fifo_add (tr->transactionff, evt);
        }
      }
    }
    tr = (osip_transaction_t *) osip_list_get_next (&iterator);
  }
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->ict_fastmutex);
#endif
}

void
osip_timers_ist_execute (osip_t * osip)
{
  osip_transaction_t *tr;
  osip_list_iterator_t iterator;

#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->ist_fastmutex);
#endif
  /* handle ist timers */
  tr = (osip_transaction_t *) osip_list_get_first (&osip->osip_ist_transactions, &iterator);
  while (osip_list_iterator_has_elem (iterator)) {
    osip_event_t *evt;

    evt = __osip_ist_need_timer_i_event (tr->ist_context, tr->state, tr->transactionid);
    if (evt != NULL)
      osip_fifo_add (tr->transactionff, evt);
    else {
      evt = __osip_ist_need_timer_h_event (tr->ist_context, tr->state, tr->transactionid);
      if (evt != NULL)
        osip_fifo_add (tr->transactionff, evt);
      else {
        evt = __osip_ist_need_timer_g_event (tr->ist_context, tr->state, tr->transactionid);
        if (evt != NULL)
          osip_fifo_add (tr->transactionff, evt);
      }
    }
    tr = (osip_transaction_t *) osip_list_get_next (&iterator);
  }
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->ist_fastmutex);
#endif
}

void
osip_timers_nict_execute (osip_t * osip)
{
  osip_transaction_t *tr;
  osip_list_iterator_t iterator;

#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->nict_fastmutex);
#endif
  /* handle nict timers */
  tr = (osip_transaction_t *) osip_list_get_first (&osip->osip_nict_transactions, &iterator);
  while (osip_list_iterator_has_elem (iterator)) {
    osip_event_t *evt;

    evt = __osip_nict_need_timer_k_event (tr->nict_context, tr->state, tr->transactionid);
    if (evt != NULL)
      osip_fifo_add (tr->transactionff, evt);
    else {
      evt = __osip_nict_need_timer_f_event (tr->nict_context, tr->state, tr->transactionid);
      if (evt != NULL)
        osip_fifo_add (tr->transactionff, evt);
      else {
        evt = __osip_nict_need_timer_e_event (tr->nict_context, tr->state, tr->transactionid);
        if (evt != NULL)
          osip_fifo_add (tr->transactionff, evt);
      }
    }
    tr = (osip_transaction_t *) osip_list_get_next (&iterator);
  }
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->nict_fastmutex);
#endif
}


void
osip_timers_nist_execute (osip_t * osip)
{
  osip_transaction_t *tr;
  osip_list_iterator_t iterator;

#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (osip->nist_fastmutex);
#endif
  /* handle nist timers */
  tr = (osip_transaction_t *) osip_list_get_first (&osip->osip_nist_transactions, &iterator);
  while (osip_list_iterator_has_elem (iterator)) {
    osip_event_t *evt;

    evt = __osip_nist_need_timer_j_event (tr->nist_context, tr->state, tr->transactionid);
    if (evt != NULL)
      osip_fifo_add (tr->transactionff, evt);
    tr = (osip_transaction_t *) osip_list_get_next (&iterator);
  }
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (osip->nist_fastmutex);
#endif
}

void
osip_set_cb_send_message (osip_t * cf, int (*cb) (osip_transaction_t *, osip_message_t *, char *, int, int))
{
  cf->cb_send_message = cb;
}

void
__osip_message_callback (int type, osip_transaction_t * tr, osip_message_t * msg)
{
  osip_t *config = tr->config;

  if (type >= OSIP_MESSAGE_CALLBACK_COUNT) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_BUG, NULL, "invalid callback type %d\n", type));
    return;
  }
  if (config->msg_callbacks[type] == NULL)
    return;
  config->msg_callbacks[type] (type, tr, msg);
}

void
__osip_kill_transaction_callback (int type, osip_transaction_t * tr)
{
  osip_t *config = tr->config;

  if (type >= OSIP_KILL_CALLBACK_COUNT) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_BUG, NULL, "invalid callback type %d\n", type));
    return;
  }
  tr->completed_time = osip_getsystemtime (NULL);
  if (config->kill_callbacks[type] == NULL)
    return;
  config->kill_callbacks[type] (type, tr);
}

void
__osip_transport_error_callback (int type, osip_transaction_t * tr, int error)
{
  osip_t *config = tr->config;

  if (type >= OSIP_TRANSPORT_ERROR_CALLBACK_COUNT) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_BUG, NULL, "invalid callback type %d\n", type));
    return;
  }
  if (config->tp_error_callbacks[type] == NULL)
    return;
  config->tp_error_callbacks[type] (type, tr, error);
}


int
osip_set_message_callback (osip_t * config, int type, osip_message_cb_t cb)
{
  if (type >= OSIP_MESSAGE_CALLBACK_COUNT) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "invalid callback type %d\n", type));
    return OSIP_BADPARAMETER;
  }
  config->msg_callbacks[type] = cb;

  return OSIP_SUCCESS;
}

int
osip_set_kill_transaction_callback (osip_t * config, int type, osip_kill_transaction_cb_t cb)
{
  if (type >= OSIP_KILL_CALLBACK_COUNT) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "invalid callback type %d\n", type));
    return OSIP_BADPARAMETER;
  }
  config->kill_callbacks[type] = cb;
  return OSIP_SUCCESS;
}

int
osip_set_transport_error_callback (osip_t * config, int type, osip_transport_error_cb_t cb)
{
  if (type >= OSIP_TRANSPORT_ERROR_CALLBACK_COUNT) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "invalid callback type %d\n", type));
    return OSIP_BADPARAMETER;
  }
  config->tp_error_callbacks[type] = cb;
  return OSIP_SUCCESS;
}
