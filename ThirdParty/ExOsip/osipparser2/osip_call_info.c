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

int
osip_message_set_call_info (osip_message_t * sip, const char *hvalue)
{
  osip_call_info_t *call_info;
  int i;

  if (hvalue == NULL || hvalue[0] == '\0')
    return OSIP_SUCCESS;

  i = osip_call_info_init (&call_info);
  if (i != 0)
    return i;
  i = osip_call_info_parse (call_info, hvalue);
  if (i != 0) {                 /* allocation failed */
    osip_call_info_free (call_info);
    return i;
  }
  sip->message_property = 2;
  osip_list_add (&sip->call_infos, call_info, -1);
  return OSIP_SUCCESS;
}

int
osip_message_get_call_info (const osip_message_t * sip, int pos, osip_call_info_t ** dest)
{
  osip_call_info_t *call_info;

  *dest = NULL;
  if (osip_list_size (&sip->call_infos) <= pos)
    return OSIP_UNDEFINED_ERROR;        /* does not exist */
  call_info = (osip_call_info_t *) osip_list_get (&sip->call_infos, pos);
  *dest = call_info;
  return pos;
}

int
osip_call_info_init (osip_call_info_t ** call_info)
{
  *call_info = (osip_call_info_t *) osip_malloc (sizeof (osip_call_info_t));
  if (*call_info == NULL)
    return OSIP_NOMEM;

  (*call_info)->element = NULL;

  osip_list_init (&(*call_info)->gen_params);

  return OSIP_SUCCESS;
}

int
osip_call_info_parse (osip_call_info_t * call_info, const char *hvalue)
{
  const char *osip_call_info_params;
  int i;

  osip_call_info_params = strchr (hvalue, '<');
  if (osip_call_info_params == NULL)
    return OSIP_SYNTAXERROR;

  osip_call_info_params = strchr (osip_call_info_params + 1, '>');
  if (osip_call_info_params == NULL)
    return OSIP_SYNTAXERROR;

  osip_call_info_params = strchr (osip_call_info_params + 1, ';');

  if (osip_call_info_params != NULL) {
    i = __osip_generic_param_parseall (&call_info->gen_params, osip_call_info_params);
    if (i != 0)
      return i;
  }
  else
    osip_call_info_params = hvalue + strlen (hvalue);

  if (osip_call_info_params - hvalue + 1 < 2)
    return OSIP_SYNTAXERROR;
  call_info->element = (char *) osip_malloc (osip_call_info_params - hvalue + 1);
  if (call_info->element == NULL)
    return OSIP_NOMEM;
  osip_clrncpy (call_info->element, hvalue, osip_call_info_params - hvalue);

  return OSIP_SUCCESS;
}

/* returns the call_info header as a string.  */
/* INPUT : osip_call_info_t *call_info | call_info header.   */
/* returns null on error. */
int
osip_call_info_to_str (const osip_call_info_t * call_info, char **dest)
{
  char *buf;
  char *tmp;
  size_t len;
  size_t plen;

  *dest = NULL;
  if ((call_info == NULL) || (call_info->element == NULL))
    return OSIP_BADPARAMETER;

  len = strlen (call_info->element) + 2;
  buf = (char *) osip_malloc (len);
  if (buf == NULL)
    return OSIP_NOMEM;
  *dest = buf;

  sprintf (buf, "%s", call_info->element);

  {
    osip_list_iterator_t it;
    osip_generic_param_t *u_param = (osip_generic_param_t*) osip_list_get_first(&call_info->gen_params, &it);
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
        sprintf (tmp, ";%s", u_param->gname);
      else
        sprintf (tmp, ";%s=%s", u_param->gname, u_param->gvalue);
      u_param = (osip_generic_param_t *) osip_list_get_next(&it);
    }
  }
  *dest = buf;
  return OSIP_SUCCESS;
}


/* deallocates a osip_call_info_t structure.  */
/* INPUT : osip_call_info_t *call_info | call_info. */
void
osip_call_info_free (osip_call_info_t * call_info)
{
  if (call_info == NULL)
    return;
  osip_free (call_info->element);

  osip_generic_param_freelist (&call_info->gen_params);

  call_info->element = NULL;

  osip_free (call_info);
}

int
osip_call_info_clone (const osip_call_info_t * ctt, osip_call_info_t ** dest)
{
  int i;
  osip_call_info_t *ct;

  *dest = NULL;
  if (ctt == NULL)
    return OSIP_BADPARAMETER;
  if (ctt->element == NULL)
    return OSIP_BADPARAMETER;

  i = osip_call_info_init (&ct);
  if (i != 0)                   /* allocation failed */
    return i;
  ct->element = osip_strdup (ctt->element);
  if (ct->element == NULL) {
    osip_call_info_free (ct);
    return OSIP_NOMEM;
  }

  i = osip_list_clone (&ctt->gen_params, &ct->gen_params, (int (*)(void *, void **)) &osip_generic_param_clone);
  if (i != 0) {
    osip_call_info_free (ct);
    return i;
  }
  *dest = ct;
  return OSIP_SUCCESS;
}


char *
osip_call_info_get_uri (osip_call_info_t * ae)
{
  return ae->element;
}

void
osip_call_info_set_uri (osip_call_info_t * ae, char *uri)
{
  ae->element = uri;
}

