/*
  The oSIP library implements the Session Initiation Protocol (SIP -rfc3261-)
  Copyright (C) 2001-2015 Aymeric MOIZARD amoizard@antisip.com
  
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

#include <osipparser2/internal.h>

#include <osipparser2/osip_port.h>
#include <osipparser2/osip_message.h>
#include <osipparser2/osip_parser.h>
#include "parser.h"

#ifndef MINISIZE

/* Accept-Encoding = token
   token possible values are gzip,compress,deflate,identity
*/
int
osip_message_set_accept_encoding (osip_message_t * sip, const char *hvalue)
{
  osip_accept_encoding_t *accept_encoding;
  int i;

  if (hvalue == NULL || hvalue[0] == '\0')
    return OSIP_SUCCESS;

  i = osip_accept_encoding_init (&accept_encoding);
  if (i != 0)
    return i;
  i = osip_accept_encoding_parse (accept_encoding, hvalue);
  if (i != 0) {
    osip_accept_encoding_free (accept_encoding);
    return i;
  }
  sip->message_property = 2;
  osip_list_add (&sip->accept_encodings, accept_encoding, -1);
  return OSIP_SUCCESS;
}

int
osip_message_get_accept_encoding (const osip_message_t * sip, int pos, osip_accept_encoding_t ** dest)
{
  osip_accept_encoding_t *accept_encoding;

  *dest = NULL;
  if (osip_list_size (&sip->accept_encodings) <= pos)
    return OSIP_UNDEFINED_ERROR;        /* does not exist */
  accept_encoding = (osip_accept_encoding_t *) osip_list_get (&sip->accept_encodings, pos);
  *dest = accept_encoding;
  return pos;
}

int
osip_accept_encoding_init (osip_accept_encoding_t ** accept_encoding)
{
  *accept_encoding = (osip_accept_encoding_t *) osip_malloc (sizeof (osip_accept_encoding_t));
  if (*accept_encoding == NULL)
    return OSIP_NOMEM;
  (*accept_encoding)->element = NULL;

  osip_list_init (&(*accept_encoding)->gen_params);

  return OSIP_SUCCESS;
}

int
osip_accept_encoding_parse (osip_accept_encoding_t * accept_encoding, const char *hvalue)
{
  int i;
  const char *osip_accept_encoding_params;

  osip_accept_encoding_params = strchr (hvalue, ';');

  if (osip_accept_encoding_params != NULL) {
    i = __osip_generic_param_parseall (&accept_encoding->gen_params, osip_accept_encoding_params);
    if (i != 0)
      return i;
  }
  else
    osip_accept_encoding_params = hvalue + strlen (hvalue);

  if (osip_accept_encoding_params - hvalue + 1 < 2)
    return OSIP_SYNTAXERROR;
  accept_encoding->element = (char *) osip_malloc (osip_accept_encoding_params - hvalue + 1);
  if (accept_encoding->element == NULL)
    return OSIP_NOMEM;
  osip_clrncpy (accept_encoding->element, hvalue, osip_accept_encoding_params - hvalue);

  return OSIP_SUCCESS;
}

/* returns the accept_encoding header as a string.  */
/* INPUT : osip_accept_encoding_t *accept_encoding | accept_encoding header.   */
/* returns null on error. */
int
osip_accept_encoding_to_str (const osip_accept_encoding_t * accept_encoding, char **dest)
{
  char *buf;
  char *tmp;
  size_t len;

  *dest = NULL;
  if ((accept_encoding == NULL) || (accept_encoding->element == NULL))
    return OSIP_BADPARAMETER;

  len = strlen (accept_encoding->element) + 2;
  buf = (char *) osip_malloc (len);
  if (buf == NULL)
    return OSIP_NOMEM;

  sprintf (buf, "%s", accept_encoding->element);
  {
    size_t plen;
    osip_list_iterator_t it;
    osip_generic_param_t *u_param = (osip_generic_param_t*) osip_list_get_first(&accept_encoding->gen_params, &it);
    while (u_param != OSIP_SUCCESS) {
      if (u_param->gvalue == NULL)
        plen = strlen (u_param->gname) + 2;
      else
        plen = strlen (u_param->gname) + strlen (u_param->gvalue) + 3;
      len = len + plen;
      buf = (char *) osip_realloc (buf, len);
      tmp = buf;
      tmp = tmp + strlen (tmp);
      if (u_param->gvalue == NULL)
        snprintf (tmp, len - (tmp - buf), ";%s", u_param->gname);
      else
        snprintf (tmp, len - (tmp - buf), ";%s=%s", u_param->gname, u_param->gvalue);
      u_param = (osip_generic_param_t *) osip_list_get_next(&it);
    }
  }
  *dest = buf;
  return OSIP_SUCCESS;
}

/* deallocates a osip_accept_encoding_t structure.  */
/* INPUT : osip_accept_encoding_t *accept_encoding | accept_encoding. */
void
osip_accept_encoding_free (osip_accept_encoding_t * accept_encoding)
{
  if (accept_encoding == NULL)
    return;
  osip_free (accept_encoding->element);

  osip_generic_param_freelist (&accept_encoding->gen_params);

  accept_encoding->element = NULL;
  osip_free (accept_encoding);
}

int
osip_accept_encoding_clone (const osip_accept_encoding_t * ctt, osip_accept_encoding_t ** dest)
{
  int i;
  osip_accept_encoding_t *ct;

  *dest = NULL;
  if (ctt == NULL)
    return OSIP_BADPARAMETER;
  if (ctt->element == NULL)
    return OSIP_BADPARAMETER;

  i = osip_accept_encoding_init (&ct);
  if (i != 0)                   /* allocation failed */
    return i;
  ct->element = osip_strdup (ctt->element);
  if (ct->element == NULL) {
    osip_accept_encoding_free (ct);
    return OSIP_NOMEM;
  }
  {
    osip_generic_param_t *dest_param;
    osip_list_iterator_t it;
    osip_generic_param_t *u_param = (osip_generic_param_t*) osip_list_get_first(&ctt->gen_params, &it);
    while (u_param != OSIP_SUCCESS) {
      i = osip_generic_param_clone (u_param, &dest_param);
      if (i != 0) {
        osip_accept_encoding_free (ct);
        return i;
      }
      osip_list_add (&ct->gen_params, dest_param, -1);
      u_param = (osip_generic_param_t *) osip_list_get_next(&it);
    }
  }
  *dest = ct;
  return OSIP_SUCCESS;
}


char *
osip_accept_encoding_get_element (const osip_accept_encoding_t * ae)
{
  return ae->element;
}

void
osip_accept_encoding_set_element (osip_accept_encoding_t * ae, char *element)
{
  ae->element = element;
}

#endif
