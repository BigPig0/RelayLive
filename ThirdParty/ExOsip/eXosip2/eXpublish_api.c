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

#include <eXosip2/eXosip.h>

int
eXosip_build_publish (struct eXosip_t *excontext, osip_message_t ** message, const char *to, const char *from, const char *route, const char *event, const char *expires, const char *ctype, const char *body)
{
  int i;

  *message = NULL;

  if (to == NULL || to[0] == '\0')
    return OSIP_BADPARAMETER;
  if (from == NULL || from[0] == '\0')
    return OSIP_BADPARAMETER;
  if (event == NULL || event[0] == '\0')
    return OSIP_BADPARAMETER;
  if (ctype == NULL || ctype[0] == '\0') {
    if (body != NULL && body[0] != '\0')
      return OSIP_BADPARAMETER;
  }
  else {
    if (body == NULL || body[0] == '\0')
      return OSIP_BADPARAMETER;
  }

  i = _eXosip_generating_publish (excontext, message, to, from, route);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot send message (cannot build PUBLISH)! "));
    return i;
  }

  if (body != NULL && body[0] != '\0' && ctype != NULL && ctype[0] != '\0') {
    osip_message_set_content_type (*message, ctype);
    osip_message_set_body (*message, body, strlen (body));
    /*
       osip_message_set_header (*message, "Content-Disposition",
       "render;handling=required");
     */
  }
  if (expires != NULL && expires[0] != '\0')
    osip_message_set_expires (*message, expires);
  else
    osip_message_set_expires (*message, "3600");

  osip_message_set_header (*message, "Event", event);

  return OSIP_SUCCESS;
}

int
eXosip_publish (struct eXosip_t *excontext, osip_message_t * message, const char *to)
{
  osip_transaction_t *transaction;
  osip_event_t *sipevent;
  int i;
  eXosip_pub_t *pub = NULL;

  if (message == NULL)
    return OSIP_BADPARAMETER;
  if (message->cseq == NULL || message->cseq->number == NULL) {
    osip_message_free (message);
    return OSIP_SYNTAXERROR;
  }
  if (to == NULL) {
    osip_message_free (message);
    return OSIP_BADPARAMETER;
  }

  i = _eXosip_pub_find_by_aor (excontext, &pub, to);
  if (i != 0 || pub == NULL) {
    osip_header_t *expires;

    osip_message_get_expires (message, 0, &expires);
    if (expires == NULL || expires->hvalue == NULL) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: missing expires header in PUBLISH!"));
      osip_message_free (message);
      return OSIP_SYNTAXERROR;
    }
    else {
      /* start a new publication context */
      i = _eXosip_pub_init (excontext, &pub, to, expires->hvalue);
      if (i != 0) {
        osip_message_free (message);
        return i;
      }
      ADD_ELEMENT (excontext->j_pub, pub);
    }
  }
  else {
    if (pub->p_sip_etag[0] != '\0') {
      /* increase cseq */
      osip_message_set_header (message, "SIP-If-Match", pub->p_sip_etag);
    }

    {
      osip_header_t *expires;

      osip_message_get_expires (message, 0, &expires);
      if (expires == NULL || expires->hvalue == NULL) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: missing expires header in PUBLISH!"));
        osip_message_free (message);
        return OSIP_SYNTAXERROR;
      }
      pub->p_period = atoi (expires->hvalue);
    }

    if (pub->p_last_tr != NULL && pub->p_last_tr->cseq != NULL && pub->p_last_tr->cseq->number != NULL) {
      int osip_cseq_num = osip_atoi (pub->p_last_tr->cseq->number);
      int length = (int) strlen (pub->p_last_tr->cseq->number);

      osip_cseq_num++;
      osip_free (message->cseq->number);
      message->cseq->number = (char *) osip_malloc (length + 2);        /* +2 like for 9 to 10 */
      if (message->cseq->number == NULL) {
        osip_message_free (message);
        return OSIP_NOMEM;
      }
      snprintf (message->cseq->number, length + 2, "%i", osip_cseq_num);
    }
  }

  i = _eXosip_transaction_init (excontext, &transaction, NICT, excontext->j_osip, message);
  if (i != 0) {
    osip_message_free (message);
    return i;
  }

  if (pub->p_last_tr != NULL)
    osip_list_add (&excontext->j_transactions, pub->p_last_tr, 0);
  pub->p_last_tr = transaction;

  sipevent = osip_new_outgoing_sipmessage (message);
  sipevent->transactionid = transaction->transactionid;

  osip_transaction_add_event (transaction, sipevent);
  _eXosip_wakeup (excontext);
  return transaction->transactionid;
}

#endif
