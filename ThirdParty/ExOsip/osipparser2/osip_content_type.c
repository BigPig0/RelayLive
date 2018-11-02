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
osip_content_type_init (osip_content_type_t ** content_type)
{
  *content_type = (osip_content_type_t *) osip_malloc (sizeof (osip_content_type_t));
  if (*content_type == NULL)
    return OSIP_NOMEM;

  (*content_type)->type = NULL;
  (*content_type)->subtype = NULL;

  osip_list_init (&(*content_type)->gen_params);

  return OSIP_SUCCESS;
}

/* adds the content_type header to message.              */
/* INPUT : char *hvalue | value of header.    */
/* OUTPUT: osip_message_t *sip | structure to save results.  */
/* returns -1 on error. */
int
osip_message_set_content_type (osip_message_t * sip, const char *hvalue)
{
  int i;

  if (sip->content_type != NULL)
    return OSIP_BADPARAMETER;

  if (hvalue == NULL || hvalue[0] == '\0')
    return OSIP_SUCCESS;

  i = osip_content_type_init (&(sip->content_type));
  if (i != 0)
    return i;
  sip->message_property = 2;
  i = osip_content_type_parse (sip->content_type, hvalue);
  if (i != 0) {
    osip_content_type_free (sip->content_type);
    sip->content_type = NULL;
    return i;
  }
  return OSIP_SUCCESS;
}

#ifndef MINISIZE
/* returns the content_type header.            */
/* INPUT : osip_message_t *sip | sip message.   */
/* returns null on error. */
osip_content_type_t *
osip_message_get_content_type (const osip_message_t * sip)
{
  return sip->content_type;
}
#endif

/* parses the string as a content_type header.                   */
/* INPUT : const char *string | pointer to a content_type string.*/
/* OUTPUT: osip_content_type_t *content_type | structure to save results.     */
/* returns -1 on error. */
int
osip_content_type_parse (osip_content_type_t * content_type, const char *hvalue)
{
  char *subtype;
  char *osip_content_type_params;
  int i;

  /* How to parse:

     we'll place the pointers:
     subtype              =>  beginning of subtype
     osip_content_type_params  =>  beginning of params

     examples:

     application/multipart ; boundary=
     ^          ^
   */
  if (hvalue == NULL || hvalue[0] == '\0')
    return OSIP_SUCCESS;        /* It's valid to add empty Accept header! */

  subtype = strchr (hvalue, '/');
  osip_content_type_params = strchr (hvalue, ';');

  if (subtype == NULL)
    return OSIP_SYNTAXERROR;    /* do we really mind such an error */

  if (osip_content_type_params != NULL) {
    i = __osip_generic_param_parseall (&content_type->gen_params, osip_content_type_params);
    if (i != 0)
      return i;
  }
  else
    osip_content_type_params = subtype + strlen (subtype);

  if (subtype - hvalue + 1 < 2)
    return OSIP_SYNTAXERROR;
  content_type->type = (char *) osip_malloc (subtype - hvalue + 1);
  if (content_type->type == NULL)
    return OSIP_NOMEM;
  osip_clrncpy (content_type->type, hvalue, subtype - hvalue);

  if (osip_content_type_params - subtype < 2)
    return OSIP_SYNTAXERROR;
  content_type->subtype = (char *) osip_malloc (osip_content_type_params - subtype);
  if (content_type->subtype == NULL)
    return OSIP_NOMEM;
  osip_clrncpy (content_type->subtype, subtype + 1, osip_content_type_params - subtype - 1);

  return OSIP_SUCCESS;
}


/* returns the content_type header as a string.  */
/* INPUT : osip_content_type_t *content_type | content_type header.   */
/* returns null on error. */
int
osip_content_type_to_str (const osip_content_type_t * content_type, char **dest)
{
  char *buf;
  char *tmp;
  size_t len;

  *dest = NULL;
  if ((content_type == NULL) || (content_type->type == NULL)
      || (content_type->subtype == NULL))
    return OSIP_BADPARAMETER;

  /* try to guess a long enough length */
  len = strlen (content_type->type) + strlen (content_type->subtype) + 4        /* for '/', ' ', ';' and '\0' */
    + 10 * osip_list_size (&content_type->gen_params);

  buf = (char *) osip_malloc (len);
  if (buf == NULL)
    return OSIP_NOMEM;
  tmp = buf;

  sprintf (tmp, "%s/%s", content_type->type, content_type->subtype);

  tmp = tmp + strlen (tmp);
  {
    osip_list_iterator_t it;
    osip_generic_param_t *u_param = (osip_generic_param_t*) osip_list_get_first(&content_type->gen_params, &it);
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


/* deallocates a osip_content_type_t structure.  */
/* INPUT : osip_content_type_t *content_type | content_type. */
void
osip_content_type_free (osip_content_type_t * content_type)
{
  if (content_type == NULL)
    return;
  osip_free (content_type->type);
  osip_free (content_type->subtype);

  osip_generic_param_freelist (&content_type->gen_params);

  content_type->type = NULL;
  content_type->subtype = NULL;

  osip_free (content_type);
}

int
osip_content_type_clone (const osip_content_type_t * ctt, osip_content_type_t ** dest)
{
  int i;
  osip_content_type_t *ct;

  *dest = NULL;
  if (ctt == NULL)
    return OSIP_BADPARAMETER;

  i = osip_content_type_init (&ct);
  if (i != 0)                   /* allocation failed */
    return i;
  if (ctt->type != NULL)
    ct->type = osip_strdup (ctt->type);
  if (ctt->subtype != NULL)
    ct->subtype = osip_strdup (ctt->subtype);

  {
    osip_generic_param_t *dest_param;
    osip_list_iterator_t it;
    osip_generic_param_t *u_param = (osip_generic_param_t*) osip_list_get_first(&ctt->gen_params, &it);
    while (u_param != OSIP_SUCCESS) {
      i = osip_generic_param_clone (u_param, &dest_param);
      if (i != 0) {
        osip_content_type_free (ct);
        return i;
      }
      osip_list_add (&ct->gen_params, dest_param, -1);
      u_param = (osip_generic_param_t *) osip_list_get_next(&it);
    }
  }
  *dest = ct;
  return OSIP_SUCCESS;
}
