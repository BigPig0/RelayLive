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

static int _eXosip_register_build_register (struct eXosip_t *excontext, eXosip_reg_t * jr, osip_message_t ** _reg);

static eXosip_reg_t *
eXosip_reg_find (struct eXosip_t *excontext, int rid)
{
  eXosip_reg_t *jr;

  for (jr = excontext->j_reg; jr != NULL; jr = jr->next) {
    if (jr->r_id == rid) {
      return jr;
    }
  }
  return NULL;
}

int
eXosip_register_remove (struct eXosip_t *excontext, int rid)
{
  eXosip_reg_t *jr;

  if (rid <= 0)
    return OSIP_BADPARAMETER;

  jr = eXosip_reg_find (excontext, rid);
  if (jr == NULL)
    return OSIP_NOTFOUND;
  jr->r_reg_period = 0;
  jr->r_reg_expire = 0;
  REMOVE_ELEMENT (excontext->j_reg, jr);
  _eXosip_reg_free (excontext, jr);
  jr = NULL;
  return OSIP_SUCCESS;
}

static const char *days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

static void
_eXosip_register_set_date (osip_message_t * msg)
{
#if !defined(_WIN32_WCE)
  char tmp[256] = { 0 };
  time_t curtime = time (NULL);
  struct tm *ret;

#if !defined(WIN32)
  struct tm gmt;

  ret = gmtime_r (&curtime, &gmt);
#else
  ret = gmtime (&curtime);
#endif
  /*cannot use strftime because it is locale dependant */
  snprintf (tmp, sizeof (tmp) - 1, "%s, %i %s %i %02i:%02i:%02i GMT", days[ret->tm_wday], ret->tm_mday, months[ret->tm_mon], 1900 + ret->tm_year, ret->tm_hour, ret->tm_min, ret->tm_sec);
  osip_message_replace_header (msg, "Date", tmp);
#endif
}

static int
_eXosip_register_add_contact(struct eXosip_t *excontext, eXosip_reg_t * jreg, osip_message_t *reg)
{
  int i;

  osip_contact_t *new_contact;
  osip_uri_t *new_contact_url = NULL;

  osip_message_get_contact(reg, 0, &new_contact);
  if (new_contact!=NULL)
    return OSIP_SUCCESS; /* already a contact header */

  i = osip_contact_init (&new_contact);
  if (i == 0)
    i = osip_uri_init (&new_contact_url);

  new_contact->url = new_contact_url;

  if (i == 0 && reg->from != NULL && reg->from->url != NULL && reg->from->url->username != NULL) {
    new_contact_url->username = osip_strdup (reg->from->url->username);
  }

  if (excontext->default_contact_displayname[0]!='\0') {
    new_contact->displayname = osip_strdup (excontext->default_contact_displayname);
  }

  if (i != 0 || reg->from == NULL || reg->from->url == NULL) {
    osip_contact_free (new_contact);
    return OSIP_SYNTAXERROR;
  }

  /* special values to be replaced in transport layer (eXtl_*.c files) */
  new_contact_url->host = osip_strdup("999.999.999.999");
  new_contact_url->port = osip_strdup("99999");

  if (osip_strcasecmp (excontext->transport, "UDP") != 0) {
    osip_uri_uparam_add (new_contact_url, osip_strdup ("transport"), osip_strdup (excontext->transport));
  }

  if (jreg->r_line[0] != '\0') {
    osip_uri_uparam_add (new_contact_url, osip_strdup ("line"), osip_strdup (jreg->r_line));
  }
  if (jreg->r_qvalue[0] != 0)
    osip_contact_param_add (new_contact, osip_strdup ("q"), osip_strdup (jreg->r_qvalue));

  if (excontext->sip_instance[0] != 0) {
    char *sip_instance = (char *) osip_malloc(50); /* "<urn:uuid:f81d4fae-7dec-11d0-a765-00a0c91e6bf6>" */
    if (sip_instance!=NULL) {
      snprintf(sip_instance, 50, "\"<urn:uuid:%s>\"", excontext->sip_instance);
      osip_contact_param_add (new_contact, osip_strdup ("+sip.instance"), sip_instance);
    }
  }
  /* If the address-of-record in the To header field of a REGISTER request
   is a SIPS URI, then any Contact header field values in the request
   SHOULD also be SIPS URIs.
   */
  if (reg->to!=NULL && reg->to->url!=NULL && reg->to->url->scheme!=NULL && osip_strcasecmp(reg->to->url->scheme, "sips")==0) {
    if (new_contact->url->scheme!=NULL)
      osip_free(new_contact->url->scheme);
    new_contact->url->scheme = osip_strdup("sips");
  }
  osip_list_add (&reg->contacts, new_contact, -1);
  return OSIP_SUCCESS;
}

static int
_eXosip_generating_register (struct eXosip_t *excontext, eXosip_reg_t * jreg, osip_message_t ** reg, char *transport, char *from, char *proxy, char *contact, int expires)
{
  int i;

  if (excontext->eXtl_transport.enabled <= 0)
    return OSIP_NO_NETWORK;

  i = _eXosip_generating_request_out_of_dialog (excontext, reg, "REGISTER", NULL, from, proxy);
  if (i != 0)
    return i;

  if (contact == NULL) {
    _eXosip_register_add_contact(excontext, jreg, *reg);
  }
  else {
    osip_message_set_contact (*reg, contact);
  }

  {
    char exp[10];               /* MUST never be ouside 1 and 3600 */

    snprintf (exp, 9, "%i", expires);
    osip_message_set_expires (*reg, exp);
  }

  osip_message_set_content_length (*reg, "0");

  return OSIP_SUCCESS;
}

static int
_eXosip_register_build_register (struct eXosip_t *excontext, eXosip_reg_t * jr, osip_message_t ** _reg)
{
  osip_message_t *reg = NULL;
  int i;

  *_reg = NULL;

  if (jr == NULL)
    return OSIP_BADPARAMETER;

  if (jr->r_last_tr != NULL) {
    osip_message_t *last_response = NULL;
    osip_transaction_t *tr;

    if (jr->r_last_tr->state != NICT_TERMINATED && jr->r_last_tr->state != NICT_COMPLETED)
      return OSIP_WRONG_STATE;

    i = osip_message_clone (jr->r_last_tr->orig_request, &reg);
    if (i != 0)
      return i;
    if (jr->r_last_tr->last_response != NULL) {
      i = osip_message_clone (jr->r_last_tr->last_response, &last_response);
      if (i != 0) {
        osip_message_free (reg);
        return i;
      }
    }

    _eXosip_delete_reserved (jr->r_last_tr);
    tr = jr->r_last_tr;
    jr->r_last_tr = NULL;
    osip_list_add (&excontext->j_transactions, tr, 0);

    /* modify the REGISTER request */
    {
      int osip_cseq_num = osip_atoi (reg->cseq->number);
      int length = (int) strlen (reg->cseq->number);


      osip_list_special_free (&reg->authorizations, (void (*)(void *))
        &osip_authorization_free);
      osip_list_special_free (&reg->proxy_authorizations, (void (*)(void *))
        &osip_proxy_authorization_free);


      i = _eXosip_update_top_via (excontext, reg);
      if (i != 0) {
        osip_message_free (reg);
        if (last_response != NULL)
          osip_message_free (last_response);
        return i;
      }

      osip_cseq_num++;
      osip_free (reg->cseq->number);
      reg->cseq->number = (char *) osip_malloc (length + 2);  /* +2 like for 9 to 10 */
      if (reg->cseq->number == NULL) {
        osip_message_free (reg);
        if (last_response != NULL)
          osip_message_free (last_response);
        return OSIP_NOMEM;
      }
      snprintf (reg->cseq->number, length + 2, "%i", osip_cseq_num);


      if (last_response != NULL && last_response->status_code == 423) {
        /* increase expires value to "min-expires" value */
        osip_header_t *exp;
        osip_header_t *min_exp;

        osip_message_header_get_byname (reg, "expires", 0, &exp);
        osip_message_header_get_byname (last_response, "min-expires", 0, &min_exp);
        if (exp != NULL && exp->hvalue != NULL && min_exp != NULL && min_exp->hvalue != NULL) {
          osip_free (exp->hvalue);
          exp->hvalue = osip_strdup (min_exp->hvalue);
          jr->r_reg_period = atoi (min_exp->hvalue);
          jr->r_reg_expire = jr->r_reg_period;
        }
        else {
          osip_message_free (reg);
          if (last_response != NULL)
            osip_message_free (last_response);
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: missing Min-Expires or Expires in REGISTER\n"));
          return OSIP_SYNTAXERROR;
        }
      }
      else {
        osip_header_t *exp;

        osip_message_header_get_byname (reg, "expires", 0, &exp);
        if (exp != NULL) {
          if (exp->hvalue != NULL)
            osip_free (exp->hvalue);
          exp->hvalue = (char *) osip_malloc (10);
          if (exp->hvalue == NULL) {
            osip_message_free (reg);
            if (last_response != NULL)
              osip_message_free (last_response);
            return OSIP_NOMEM;
          }
          snprintf (exp->hvalue, 9, "%i", jr->r_reg_expire);
        }
      }

      osip_message_force_update (reg);
    }

    if (jr->registration_step==RS_DELETIONPROCEEDING) {
      /* remove previous contact */
      osip_contact_t *contact;
      osip_message_get_contact(reg, 0, &contact);
      if (contact!=NULL) {
        osip_generic_param_t *exp_param = NULL;
        osip_contact_param_get_byname(contact, "expires", &exp_param);
        if (exp_param == NULL) {
          osip_contact_param_add (contact, osip_strdup ("expires"), osip_strdup ("0"));
        }
      }
    } else if (jr->registration_step==RS_MASQUERADINGPROCEEDING) {
      osip_contact_t *contact;
      osip_message_get_contact(reg, 0, &contact);
      if (contact!=NULL) {
        osip_list_iterator_t it;
        osip_generic_param_t* exp_param = (osip_generic_param_t*)osip_list_get_first(&contact->gen_params, &it);
        while (exp_param != NULL) {
          if (exp_param->gname!=NULL && osip_strcasecmp (exp_param->gname, "expires") == 0) {
            osip_list_iterator_remove(&it);
            osip_generic_param_free(exp_param);
            break;
          }
          exp_param = (osip_generic_param_t *)osip_list_get_next(&it);
        }
      }
      if (excontext->eXtl_transport._tl_update_contact!=NULL)
        excontext->eXtl_transport._tl_update_contact(excontext, reg);
      jr->registration_step=RS_MASQUERADINGPROCEEDING+1; /* do only once: keep previous one after */
    } else if (jr->registration_step==0) {
      if (excontext->eXtl_transport._tl_update_contact!=NULL)
        excontext->eXtl_transport._tl_update_contact(excontext, reg);
    }

    if (last_response != NULL) {
      if (last_response->status_code == 401 || last_response->status_code == 407) {
        _eXosip_add_authentication_information (excontext, reg, last_response);
      }
      else
        _eXosip_add_authentication_information (excontext, reg, NULL);
      osip_message_free (last_response);
    }

  }

  if (reg == NULL) {
    i = _eXosip_generating_register (excontext, jr, &reg, excontext->transport, jr->r_aor, jr->r_registrar, jr->r_contact, jr->r_reg_expire);
    if (i != 0)
      return i;
  }

  if (reg) {
    if (excontext->register_with_date)
      _eXosip_register_set_date (reg);
  }

  *_reg = reg;
  return OSIP_SUCCESS;
}

int
eXosip_register_build_initial_register_withqvalue (struct eXosip_t *excontext, const char *from, const char *proxy, const char *contact, int expires, const char *qvalue, osip_message_t ** reg)
{
  eXosip_reg_t *jr = NULL;
  int i;

  *reg = NULL;

  if (from == NULL || proxy == NULL)
    return OSIP_BADPARAMETER;

#ifdef REJECT_DOUBLE_REGISTRATION
  /* Avoid adding the same registration info twice to prevent mem leaks */
  for (jr = excontext->j_reg; jr != NULL; jr = jr->next) {
    if (strcmp (jr->r_aor, from) == 0 && strcmp (jr->r_registrar, proxy) == 0) {
      REMOVE_ELEMENT (excontext->j_reg, jr);
      _eXosip_reg_free (excontext, jr);
      jr = NULL;
      break;
    }
  }
#endif

  if (jr == NULL) {
    /* Add new registration info */
    i = _eXosip_reg_init (excontext, &jr, from, proxy, contact);
    if (i != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot register! "));
      return i;
    }
    ADD_ELEMENT (excontext->j_reg, jr);
  }

  /* build register */
  jr->r_reg_period = expires;
  if (jr->r_reg_period <= 0)    /* too low */
    jr->r_reg_period = 0;
  else if (jr->r_reg_period < 30)       /* too low */
    jr->r_reg_period = 30;

  jr->r_reg_expire = jr->r_reg_period;

  if (qvalue)
    osip_strncpy (jr->r_qvalue, qvalue, sizeof (jr->r_qvalue));

  if (excontext->auto_masquerade_contact>0)
    jr->registration_step=RS_MASQUERADINGPROCEEDING;

  i = _eXosip_register_build_register (excontext, jr, reg);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot build REGISTER!\n"));
    *reg = NULL;
    return i;
  }

  return jr->r_id;
}

int
eXosip_register_build_initial_register (struct eXosip_t *excontext, const char *from, const char *proxy, const char *contact, int expires, osip_message_t ** reg)
{
  return eXosip_register_build_initial_register_withqvalue (excontext, from, proxy, contact, expires, NULL, reg);
}

int
eXosip_register_build_register (struct eXosip_t *excontext, int rid, int expires, osip_message_t ** reg)
{
  eXosip_reg_t *jr;
  int i;

  *reg = NULL;

  if (rid <= 0)
    return OSIP_BADPARAMETER;

  jr = eXosip_reg_find (excontext, rid);
  if (jr == NULL)
    return OSIP_NOTFOUND;
  jr->r_reg_period = expires;
  if (jr->r_reg_period == 0) {
  }                             /* unregistration */
  else if (jr->r_reg_period < 30)       /* too low */
    jr->r_reg_period = 30;

  jr->r_reg_expire = jr->r_reg_period;

  if (jr->r_last_tr != NULL) {
    if (jr->r_last_tr->state != NICT_TERMINATED && jr->r_last_tr->state != NICT_COMPLETED) {
      return OSIP_WRONG_STATE;
    }
  }

  /* reset to allow automasquerading when user request a REGISTER refresh */
  jr->r_last_deletion=0;

  i = _eXosip_register_build_register (excontext, jr, reg);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot build REGISTER!"));
    *reg = NULL;
    return i;
  }
  return OSIP_SUCCESS;
}

int
eXosip_register_send_register (struct eXosip_t *excontext, int rid, osip_message_t * reg)
{
  osip_transaction_t *transaction;
  osip_event_t *sipevent;
  eXosip_reg_t *jr;
  int i;

  if (rid <= 0) {
    osip_message_free (reg);
    return OSIP_BADPARAMETER;
  }

  jr = eXosip_reg_find (excontext, rid);
  if (jr == NULL) {
    osip_message_free (reg);
    return OSIP_NOTFOUND;
  }

  if (jr->r_last_tr != NULL) {
    if (jr->r_last_tr->state != NICT_TERMINATED && jr->r_last_tr->state != NICT_COMPLETED) {
      osip_message_free (reg);
      return OSIP_WRONG_STATE;
    }
  }

  if (reg == NULL) {
    i = _eXosip_register_build_register (excontext, jr, &reg);
    if (i != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: cannot build REGISTER!"));
      return i;
    }
  }

  i = _eXosip_transaction_init (excontext, &transaction, NICT, excontext->j_osip, reg);
  if (i != 0) {
    osip_message_free (reg);
    return i;
  }

  jr->r_last_tr = transaction;

  /* send REGISTER */
  sipevent = osip_new_outgoing_sipmessage (reg);
  sipevent->transactionid = transaction->transactionid;
  osip_message_force_update (reg);

  osip_transaction_add_event (transaction, sipevent);
  _eXosip_wakeup (excontext);
  return OSIP_SUCCESS;
}
