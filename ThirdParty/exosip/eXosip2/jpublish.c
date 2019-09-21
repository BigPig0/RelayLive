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
_eXosip_pub_update (struct eXosip_t *excontext, eXosip_pub_t ** pub, osip_transaction_t * tr, osip_message_t * answer)
{
  eXosip_pub_t *jpub;

  *pub = NULL;

  for (jpub = excontext->j_pub; jpub != NULL; jpub = jpub->next) {
    if (jpub->p_last_tr == NULL) {      /*bug? */
    }
    else if (tr == jpub->p_last_tr) {
      /* update the sip_etag parameter */
      if (answer == NULL) {     /* bug? */
      }
      else if (MSG_IS_STATUS_2XX (answer)) {
        osip_header_t *sip_etag = NULL;

        osip_message_header_get_byname (answer, "SIP-ETag", 0, &sip_etag);
        if (sip_etag != NULL && sip_etag->hvalue != NULL)
          snprintf (jpub->p_sip_etag, 64, "%s", sip_etag->hvalue);
      }
      *pub = jpub;
      return OSIP_SUCCESS;
    }
  }
  return OSIP_NOTFOUND;
}

int
_eXosip_pub_find_by_aor (struct eXosip_t *excontext, eXosip_pub_t ** pub, const char *aor)
{
  eXosip_pub_t *jpub;

  *pub = NULL;

  for (jpub = excontext->j_pub; jpub != NULL; jpub = jpub->next) {
    if (osip_strcasecmp (aor, jpub->p_aor) == 0) {
      *pub = jpub;
      return OSIP_SUCCESS;
    }
  }
  return OSIP_NOTFOUND;
}

int
_eXosip_pub_find_by_tid (struct eXosip_t *excontext, eXosip_pub_t ** pjp, int tid)
{
  eXosip_pub_t *pub = excontext->j_pub;

  *pjp = NULL;
  while (pub) {
    if (pub->p_last_tr && pub->p_last_tr->transactionid == tid) {
      *pjp = pub;
      return OSIP_SUCCESS;
    }

    pub = pub->next;
  }

  return OSIP_NOTFOUND;
}

int
_eXosip_pub_init (struct eXosip_t *excontext, eXosip_pub_t ** pub, const char *aor, const char *exp)
{
  static int p_id = 0;
  eXosip_pub_t *jpub;

  if (p_id == INT_MAX)            /* keep it non-negative */
    p_id = 0;

  *pub = NULL;

  jpub = (eXosip_pub_t *) osip_malloc (sizeof (eXosip_pub_t));
  if (jpub == 0)
    return OSIP_NOMEM;
  memset (jpub, 0, sizeof (eXosip_pub_t));
  snprintf (jpub->p_aor, 256, "%s", aor);

  jpub->p_period = atoi (exp);
  jpub->p_id = ++p_id;

  *pub = jpub;

#ifndef MINISIZE
  {
    struct timeval now;
    excontext->statistics.allocated_publications++;
    osip_gettimeofday(&now, NULL);
    _eXosip_counters_update(&excontext->average_publications, 1, &now);
  }
#endif
  return OSIP_SUCCESS;
}

void
_eXosip_pub_free (struct eXosip_t *excontext, eXosip_pub_t * pub)
{
  if (pub->p_last_tr != NULL) {
    if (pub->p_last_tr != NULL && pub->p_last_tr->orig_request != NULL && pub->p_last_tr->orig_request->call_id != NULL && pub->p_last_tr->orig_request->call_id->number != NULL)
      _eXosip_delete_nonce (excontext, pub->p_last_tr->orig_request->call_id->number);

    osip_list_add (&excontext->j_transactions, pub->p_last_tr, 0);
  }
  osip_free (pub);
#ifndef MINISIZE
  excontext->statistics.allocated_publications--;
#endif
}

#endif
