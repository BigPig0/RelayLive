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

#ifndef MINISIZE

/* adds the accept header to message.              */
/* INPUT : char *hvalue | value of header.    */
/* OUTPUT: osip_message_t *sip | structure to save results.  */
/* returns -1 on error. */
int
osip_message_set_accept (osip_message_t * sip, const char *hvalue)
{
  osip_accept_t *accept;
  int i;

  i = accept_init (&accept);
  if (i != 0)
    return i;
  i = osip_accept_parse (accept, hvalue);
  if (i != 0) {
    osip_accept_free (accept);
    return i;
  }
  sip->message_property = 2;

  osip_list_add (&sip->accepts, accept, -1);
  return OSIP_SUCCESS;
}


int
osip_message_get_accept (const osip_message_t * sip, int pos, osip_accept_t ** dest)
{
  osip_accept_t *accept;

  *dest = NULL;
  if (osip_list_size (&sip->accepts) <= pos)
    return OSIP_UNDEFINED_ERROR;        /* does not exist */
  accept = (osip_accept_t *) osip_list_get (&sip->accepts, pos);
  *dest = accept;
  return pos;
}

/* returns the content_type header as a string.  */
/* INPUT : osip_content_type_t *content_type | content_type header.   */
/* returns null on error. */
int
osip_accept_to_str (const osip_accept_t * accept, char **dest)
{
  char *buf;
  char *tmp;
  size_t len = 0;

  *dest = NULL;
  if (accept == NULL)
    return OSIP_BADPARAMETER;

  if (accept->type != NULL)
    len += strlen (accept->type);

  if (accept->subtype != NULL)
    len += strlen (accept->subtype);

  if (len == 0) {
    /* Empty header ! */
    buf = (char *) osip_malloc (2);
    if (buf == NULL)
      return OSIP_NOMEM;
    buf[0] = ' ';
    buf[1] = '\0';
    *dest = buf;
    return OSIP_SUCCESS;
  }

  /* try to guess a long enough length */
  len += 4                      /* for '/', ' ', ';' and '\0' */
    + 10 * osip_list_size (&accept->gen_params);

  buf = (char *) osip_malloc (len);
  if (buf == NULL)
    return OSIP_NOMEM;
  tmp = buf;

  sprintf (tmp, "%s/%s", accept->type, accept->subtype);

  tmp = tmp + strlen (tmp);
  {
    osip_list_iterator_t it;
    osip_generic_param_t *u_param = (osip_generic_param_t*) osip_list_get_first(&accept->gen_params, &it);
    while (u_param != OSIP_SUCCESS) {
      size_t tmp_len;
      if (u_param->gvalue == NULL) {
        osip_free (buf);
        return OSIP_SYNTAXERROR;
      }
      tmp_len = strlen (buf) + 4 + strlen (u_param->gname)
        + strlen (u_param->gvalue) + 1;
      if (len < tmp_len) {
        buf = osip_realloc (buf, tmp_len);
        len = tmp_len;
        tmp = buf + strlen (buf);
      }
      snprintf (tmp, len - (tmp - buf), "; %s=%s", u_param->gname, u_param->gvalue);
      tmp = tmp + strlen (tmp);
      u_param = (osip_generic_param_t *) osip_list_get_next(&it);
    }
  }
  *dest = buf;
  return OSIP_SUCCESS;
}

#endif
