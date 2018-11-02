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
eXosip_message_build_request (struct eXosip_t *excontext, osip_message_t ** message, const char *method, const char *to, const char *from, const char *route)
{
  int i;

  *message = NULL;
  if (method != NULL && *method == '\0')
    return OSIP_BADPARAMETER;
  if (to != NULL && *to == '\0')
    return OSIP_BADPARAMETER;
  if (from != NULL && *from == '\0')
    return OSIP_BADPARAMETER;
  if (route != NULL && *route == '\0')
    route = NULL;

  i = _eXosip_generating_request_out_of_dialog (excontext, message, method, to, from, route);
  if (i != 0)
    return i;

  return OSIP_SUCCESS;
}

int
eXosip_message_send_request (struct eXosip_t *excontext, osip_message_t * message)
{
  osip_transaction_t *transaction;
  osip_event_t *sipevent;
  int i;

  i = _eXosip_transaction_init (excontext, &transaction, NICT, excontext->j_osip, message);
  if (i != 0) {
    osip_message_free (message);
    return i;
  }

  osip_list_add (&excontext->j_transactions, transaction, 0);

  sipevent = osip_new_outgoing_sipmessage (message);
  sipevent->transactionid = transaction->transactionid;

  osip_transaction_add_event (transaction, sipevent);

  _eXosip_wakeup (excontext);
  return transaction->transactionid;    /* return TID (>0) instead of OSIP_SUCCESS; */
}

int
eXosip_message_build_answer (struct eXosip_t *excontext, int tid, int status, osip_message_t ** answer)
{
  osip_transaction_t *tr = NULL;
  int i;

  *answer = NULL;

  if (tid <= 0)
    return OSIP_BADPARAMETER;
  if (status < 200 || status > 699)
    return OSIP_BADPARAMETER;

  if (tid > 0) {
    _eXosip_transaction_find (excontext, tid, &tr);
  }
  if (tr == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No call here?\n"));
    return OSIP_NOTFOUND;
  }

  i = -1;
  if (status < 300) {             /* 2xx answer */
    i = _eXosip_build_response_default (excontext, answer, NULL, status, tr->orig_request);

  }
  else if (status > 300)        /* 3456xx answer */
    i = _eXosip_build_response_default (excontext, answer, NULL, status, tr->orig_request);

  if (i != 0)
    return i;

  if (status < 300) {             /* 2xx answer */
    osip_header_t *refer_sub;
    osip_message_header_get_byname (tr->orig_request, "Refer-Sub", 0, &refer_sub);
    if (refer_sub!=NULL && refer_sub->hvalue!=NULL && osip_strncasecmp(refer_sub->hvalue, "false", 5)==0) {
      osip_message_set_header (*answer, "Refer-Sub", "false");
    }
  }
  return OSIP_SUCCESS;
}

int
eXosip_message_send_answer (struct eXosip_t *excontext, int tid, int status, osip_message_t * answer)
{
  osip_transaction_t *tr = NULL;
  osip_event_t *evt_answer;
  int i = -1;

  if (tid <= 0)
    return OSIP_BADPARAMETER;
  if (status <= 100 || status > 699)
    return OSIP_BADPARAMETER;
  if (answer == NULL && status > 100 && status < 200)
    return OSIP_BADPARAMETER;

  if (tid > 0) {
    _eXosip_transaction_find (excontext, tid, &tr);
  }
  if (tr == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No MESSAGE transaction found\n"));
    osip_message_free (answer);
    return OSIP_NOTFOUND;
  }

  /* is the transaction already answered? */
  if (tr->state == NIST_COMPLETED || tr->state == NIST_TERMINATED) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: transaction already answered\n"));
    osip_message_free (answer);
    return OSIP_WRONG_STATE;
  }

  if (answer == NULL) {
    i = -1;
    if (status > 199 && status < 300)
      i = _eXosip_build_response_default (excontext, &answer, NULL, status, tr->orig_request);
    else if (status > 300 && status <= 699)
      i = _eXosip_build_response_default (excontext, &answer, NULL, status, tr->orig_request);
    if (i != 0)
      return i;

    if (status < 300) {             /* 2xx answer */
      osip_header_t *refer_sub;
      osip_message_header_get_byname (tr->orig_request, "Refer-Sub", 0, &refer_sub);
      if (refer_sub!=NULL && refer_sub->hvalue!=NULL && osip_strncasecmp(refer_sub->hvalue, "false", 5)==0) {
        osip_message_set_header (answer, "Refer-Sub", "false");
      }
    }
  }

  evt_answer = osip_new_outgoing_sipmessage (answer);
  evt_answer->transactionid = tr->transactionid;

  osip_transaction_add_event (tr, evt_answer);
  _eXosip_wakeup (excontext);
  return OSIP_SUCCESS;
}
