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

#include <osipparser2/osip_md5.h>

/* TAKEN from rcf2617.txt */
#define HASHLEN 16
typedef char HASH[HASHLEN];

#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN + 1];

void CvtHex (HASH Bin, HASHHEX Hex);

int
_eXosip_reg_init (struct eXosip_t *excontext, eXosip_reg_t ** jr, const char *from, const char *proxy, const char *contact)
{
  static int r_id = 0;

  *jr = (eXosip_reg_t *) osip_malloc (sizeof (eXosip_reg_t));
  if (*jr == NULL)
    return OSIP_NOMEM;

  if (r_id == INT_MAX)            /* keep it non-negative */
    r_id = 0;

  memset (*jr, '\0', sizeof (eXosip_reg_t));

  (*jr)->r_id = ++r_id;
  (*jr)->r_reg_period = 3600;   /* delay between registration */
  (*jr)->r_aor = osip_strdup (from);    /* sip identity */
  if ((*jr)->r_aor == NULL) {
    osip_free (*jr);
    *jr = NULL;
    return OSIP_NOMEM;
  }
  (*jr)->r_contact = osip_strdup (contact);     /* sip identity */
  (*jr)->r_registrar = osip_strdup (proxy);     /* registrar */
  if ((*jr)->r_registrar == NULL) {
    osip_free ((*jr)->r_contact);
    osip_free ((*jr)->r_aor);
    osip_free (*jr);
    *jr = NULL;
    return OSIP_NOMEM;
  }

  {
    osip_MD5_CTX Md5Ctx;
    HASH hval;
    HASHHEX key_line;
    char localip[128];
    char firewall_ip[65];
    char firewall_port[10];
    char somerandom[31];

    memset (somerandom, 0, sizeof (somerandom));
    eXosip_generate_random (somerandom, 16);

    memset (localip, '\0', sizeof (localip));
    memset (firewall_ip, '\0', sizeof (firewall_ip));
    memset (firewall_port, '\0', sizeof (firewall_port));

    eXosip_guess_localip (excontext, AF_INET, localip, 128);
    if (excontext->eXtl_transport.tl_get_masquerade_contact != NULL) {
      excontext->eXtl_transport.tl_get_masquerade_contact (excontext, firewall_ip, sizeof (firewall_ip), firewall_port, sizeof (firewall_port));
    }

    osip_MD5Init (&Md5Ctx);
    osip_MD5Update (&Md5Ctx, (unsigned char *) from, (unsigned int) strlen (from));
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) proxy, (unsigned int) strlen (proxy));
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) localip, (unsigned int) strlen (localip));
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) firewall_ip, (unsigned int) strlen (firewall_ip));
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) firewall_port, (unsigned int) strlen (firewall_port));

    /* previously, "line" was common accross several identical restart. */
    /* including random will help to read a correct "expires" parameter */
    /* from 2xx REGISTER answers */
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) somerandom, (unsigned int) strlen (somerandom));

    osip_MD5Final ((unsigned char *) hval, &Md5Ctx);
    CvtHex (hval, key_line);

    osip_strncpy ((*jr)->r_line, key_line, sizeof ((*jr)->r_line) - 1);
  }

#ifndef MINISIZE
  {
    struct timeval now;
    excontext->statistics.allocated_registrations++;
    osip_gettimeofday(&now, NULL);
    _eXosip_counters_update(&excontext->average_registrations, 1, &now);
  }
#endif
  return OSIP_SUCCESS;
}

void
_eXosip_reg_free (struct eXosip_t *excontext, eXosip_reg_t * jreg)
{

  osip_free (jreg->r_aor);
  osip_free (jreg->r_contact);
  osip_free (jreg->r_registrar);

  if (jreg->r_last_tr != NULL) {
    if (jreg->r_last_tr != NULL && jreg->r_last_tr->orig_request != NULL && jreg->r_last_tr->orig_request->call_id != NULL && jreg->r_last_tr->orig_request->call_id->number != NULL)
      _eXosip_delete_nonce (excontext, jreg->r_last_tr->orig_request->call_id->number);

    if (jreg->r_last_tr->state == IST_TERMINATED || jreg->r_last_tr->state == ICT_TERMINATED || jreg->r_last_tr->state == NICT_TERMINATED || jreg->r_last_tr->state == NIST_TERMINATED) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Release a terminated transaction\n"));
      _eXosip_delete_reserved (jreg->r_last_tr);
      if (jreg->r_last_tr != NULL)
        osip_list_add (&excontext->j_transactions, jreg->r_last_tr, 0);
    }
    else {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Release a non-terminated transaction\n"));
      _eXosip_delete_reserved (jreg->r_last_tr);
      if (jreg->r_last_tr != NULL)
        osip_list_add (&excontext->j_transactions, jreg->r_last_tr, 0);
    }
  }

  osip_free (jreg);

#ifndef MINISIZE
  excontext->statistics.allocated_registrations--;
#endif
}

int
_eXosip_reg_find (struct eXosip_t *excontext, eXosip_reg_t ** reg, osip_transaction_t * tr)
{
  eXosip_reg_t *jreg;

  *reg = NULL;
  if (tr == NULL)
    return OSIP_BADPARAMETER;

  for (jreg = excontext->j_reg; jreg != NULL; jreg = jreg->next) {
    if (jreg->r_last_tr == tr) {
      *reg = jreg;
      return OSIP_SUCCESS;
    }
  }
  return OSIP_NOTFOUND;
}


int
_eXosip_reg_find_id (struct eXosip_t *excontext, eXosip_reg_t ** reg, int rid)
{
  eXosip_reg_t *jreg;

  *reg = NULL;
  if (rid <= 0)
    return OSIP_BADPARAMETER;

  for (jreg = excontext->j_reg; jreg != NULL; jreg = jreg->next) {
    if (jreg->r_id == rid) {
      *reg = jreg;
      return OSIP_SUCCESS;
    }
  }
  return OSIP_NOTFOUND;
}
