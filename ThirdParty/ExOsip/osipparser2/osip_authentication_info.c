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

int
osip_authentication_info_init (osip_authentication_info_t ** dest)
{
  *dest = (osip_authentication_info_t *)
    osip_malloc (sizeof (osip_authentication_info_t));
  if (*dest == NULL)
    return OSIP_NOMEM;
  memset (*dest, 0, sizeof (osip_authentication_info_t));
  return OSIP_SUCCESS;
}

/* fills the www-authenticate header of message.               */
/* INPUT :  char *hvalue | value of header.   */
/* OUTPUT: osip_message_t *sip | structure to save results. */
/* returns -1 on error. */
int
osip_message_set_authentication_info (osip_message_t * sip, const char *hvalue)
{
  osip_authentication_info_t *authentication_info;
  int i;

  if (hvalue == NULL || hvalue[0] == '\0')
    return OSIP_SUCCESS;

  if (sip == NULL)
    return OSIP_BADPARAMETER;
  i = osip_authentication_info_init (&authentication_info);
  if (i != 0)
    return i;
  i = osip_authentication_info_parse (authentication_info, hvalue);
  if (i != 0) {
    osip_authentication_info_free (authentication_info);
    return i;
  }
  sip->message_property = 2;

  osip_list_add (&sip->authentication_infos, authentication_info, -1);
  return OSIP_SUCCESS;
}

/* fills the authentication_info strucuture.                      */
/* INPUT : char *hvalue | value of header.         */
/* OUTPUT: osip_message_t *sip | structure to save results. */
/* returns -1 on error. */
/* TODO:
   digest-challenge tken has no order preference??
   verify many situations (extra SP....)
*/
int
osip_authentication_info_parse (osip_authentication_info_t * ainfo, const char *hvalue)
{
	const char *space, *hack;
  const char *next = NULL;
  int i;

  space = strchr (hvalue, ' ');
  hack = strchr( hvalue, '=');
  if(space && hack && hack > space) {
	  ainfo->auth_type = (char *) osip_malloc (space - hvalue + 1);
	  if (ainfo->auth_type==NULL)
		  return OSIP_NOMEM;
	  osip_strncpy (ainfo->auth_type, hvalue, space - hvalue);
  }
  else
    space = hvalue;

  for (;;) {
    int parse_ok = 0;

    i = __osip_quoted_string_set ("nextnonce", space, &(ainfo->nextnonce), &next);
    if (i != 0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;      /* end of header detected! */
    else if (next != space) {
      space = next;
      parse_ok++;
    }
    i = __osip_quoted_string_set ("cnonce", space, &(ainfo->cnonce), &next);
    if (i != 0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;      /* end of header detected! */
    else if (next != space) {
      space = next;
      parse_ok++;
    }
    i = __osip_quoted_string_set ("rspauth", space, &(ainfo->rspauth), &next);
    if (i != 0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;      /* end of header detected! */
    else if (next != space) {
      space = next;
      parse_ok++;
    }
    i = __osip_token_set ("nc", space, &(ainfo->nonce_count), &next);
    if (i != 0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;      /* end of header detected! */
    else if (next != space) {
      space = next;
      parse_ok++;
    }
    i = __osip_token_set ("qop", space, &(ainfo->qop_options), &next);
    if (i != 0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;      /* end of header detected! */
    else if (next != space) {
      space = next;
      parse_ok++;
    }
    i = __osip_quoted_string_set ("snum", space, &(ainfo->snum), &next);
    if (i!=0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;               /* end of header detected! */
    else if (next != space) {
      space = next;
      parse_ok++;
    }
    i = __osip_quoted_string_set ("srand", space, &(ainfo->srand), &next);
    if (i!=0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;               /* end of header detected! */
    else if (next != space) {
      space = next;
      parse_ok++;
    }
    i = __osip_quoted_string_set ("targetname", space, &(ainfo->targetname), &next);
    if (i!=0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;               /* end of header detected! */
    else if (next != space) {
      space = next;
      parse_ok++;
    }
    i = __osip_quoted_string_set ("realm", space, &(ainfo->realm), &next);
    if (i!=0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;               /* end of header detected! */
    else if (next != space) {
      space = next;
      parse_ok++;
    }
    i = __osip_quoted_string_set ("opaque", space, &(ainfo->opaque), &next);
    if (i!=0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;               /* end of header detected! */
    else if (next != space) {
      space = next;
      parse_ok++;
    }
    if (0 == parse_ok) {
      const char *quote1, *quote2, *tmp;

      /* CAUTION */
      /* parameter not understood!!! I'm too lazy to handle IT */
      /* let's simply bypass it */
      if (strlen (space) < 1)
        return OSIP_SUCCESS;
      tmp = strchr (space + 1, ',');
      if (tmp == NULL)          /* it was the last header */
        return OSIP_SUCCESS;
      quote1 = __osip_quote_find (space);
      if ((quote1 != NULL) && (quote1 < tmp)) { /* this may be a quoted string! */
        quote2 = __osip_quote_find (quote1 + 1);
        if (quote2 == NULL)
          return OSIP_SYNTAXERROR;      /* bad header format... */
        if (tmp < quote2)       /* the comma is inside the quotes! */
          space = strchr (quote2, ',');
        else
          space = tmp;
        if (space == NULL)      /* it was the last header */
          return OSIP_SUCCESS;
      }
      else
        space = tmp;
      /* continue parsing... */
    }
  }
  return OSIP_SUCCESS;          /* ok */
}

/* returns the authentication_info header.            */
/* INPUT : osip_message_t *sip | sip message.   */
/* returns null on error. */
int
osip_message_get_authentication_info (const osip_message_t * sip, int pos, osip_authentication_info_t ** dest)
{
  osip_authentication_info_t *authentication_info;

  *dest = NULL;
  if (osip_list_size (&sip->authentication_infos) <= pos)
    return OSIP_UNDEFINED_ERROR;        /* does not exist */

  authentication_info = (osip_authentication_info_t *) osip_list_get (&sip->authentication_infos, pos);

  *dest = authentication_info;
  return pos;
}

char *
osip_authentication_info_get_auth_type (osip_authentication_info_t *authentication_info)
{
  return authentication_info->auth_type;
}

void
osip_authentication_info_set_auth_type (osip_authentication_info_t *authentication_info, char *auth_type)
{
  authentication_info->auth_type = (char *) auth_type;
}

char *
osip_authentication_info_get_nextnonce (osip_authentication_info_t * authentication_info)
{
  return authentication_info->nextnonce;
}

void
osip_authentication_info_set_nextnonce (osip_authentication_info_t * authentication_info, char *nextnonce)
{
  authentication_info->nextnonce = (char *) nextnonce;
}

char *
osip_authentication_info_get_cnonce (osip_authentication_info_t * authentication_info)
{
  return authentication_info->cnonce;
}

void
osip_authentication_info_set_cnonce (osip_authentication_info_t * authentication_info, char *cnonce)
{
  authentication_info->cnonce = (char *) cnonce;
}

char *
osip_authentication_info_get_rspauth (osip_authentication_info_t * authentication_info)
{
  return authentication_info->rspauth;
}

void
osip_authentication_info_set_rspauth (osip_authentication_info_t * authentication_info, char *rspauth)
{
  authentication_info->rspauth = (char *) rspauth;
}

char *
osip_authentication_info_get_nonce_count (osip_authentication_info_t * authentication_info)
{
  return authentication_info->nonce_count;
}

void
osip_authentication_info_set_nonce_count (osip_authentication_info_t * authentication_info, char *nonce_count)
{
  authentication_info->nonce_count = (char *) nonce_count;
}

char *
osip_authentication_info_get_qop_options (osip_authentication_info_t * authentication_info)
{
  return authentication_info->qop_options;
}

void
osip_authentication_info_set_qop_options (osip_authentication_info_t * authentication_info, char *qop_options)
{
  authentication_info->qop_options = (char *) qop_options;
}

char *
osip_authentication_info_get_snum (osip_authentication_info_t *
				   authentication_info)
{
  return authentication_info->snum;
}

void
osip_authentication_info_set_snum (osip_authentication_info_t *
				   authentication_info, char *snum)
{
  authentication_info->snum = (char *) snum;
}

char *
osip_authentication_info_get_srand (osip_authentication_info_t *
				   authentication_info)
{
  return authentication_info->srand;
}

void
osip_authentication_info_set_srand (osip_authentication_info_t *
				   authentication_info, char *srand)
{
  authentication_info->srand = (char *) srand;
}

char *
osip_authentication_info_get_targetname (osip_authentication_info_t *
				   authentication_info)
{
  return authentication_info->targetname;
}

void
osip_authentication_info_set_targetname (osip_authentication_info_t *
				   authentication_info, char *targetname)
{
  authentication_info->targetname = (char *) targetname;
}

char *
osip_authentication_info_get_realm (osip_authentication_info_t *
				   authentication_info)
{
  return authentication_info->realm;
}

void
osip_authentication_info_set_realm (osip_authentication_info_t *
				   authentication_info, char *realm)
{
  authentication_info->realm = (char *) realm;
}

char *
osip_authentication_info_get_opaque (osip_authentication_info_t *
				   authentication_info)
{
  return authentication_info->opaque;
}

void
osip_authentication_info_set_opaque (osip_authentication_info_t *
				   authentication_info, char *opaque)
{
  authentication_info->opaque = (char *) opaque;
}


/* returns the authentication_info header as a string.          */
/* INPUT : osip_authentication_info_t *authentication_info | authentication_info header.  */
/* returns null on error. */
int
osip_authentication_info_to_str (const osip_authentication_info_t * ainfo, char **dest)
{
  size_t len;
  char *tmp, *start;

  *dest = NULL;
  if (ainfo == NULL)
    return OSIP_BADPARAMETER;

  len = 0;
  if (ainfo->auth_type != NULL)
    len = len + strlen (ainfo->auth_type) + 1;
  if (ainfo->nextnonce != NULL)
    len = len + strlen (ainfo->nextnonce) + 11;
  if (ainfo->rspauth != NULL)
    len = len + strlen (ainfo->rspauth) + 10;
  if (ainfo->cnonce != NULL)
    len = len + strlen (ainfo->cnonce) + 9;
  if (ainfo->nonce_count != NULL)
    len = len + strlen (ainfo->nonce_count) + 5;
  if (ainfo->qop_options != NULL)
    len = len + strlen (ainfo->qop_options) + 6;
  if (ainfo->snum != NULL)
    len = len + strlen (ainfo->snum) + 7;
  if (ainfo->srand != NULL)
    len = len + strlen (ainfo->srand) + 8;
  if (ainfo->targetname != NULL)
    len = len + strlen (ainfo->targetname) + 13;
  if (ainfo->realm != NULL)
    len = len + strlen (ainfo->realm) + 8;
  if (ainfo->opaque != NULL)
    len = len + strlen (ainfo->opaque) + 8;

  if (len == 0)
    return OSIP_BADPARAMETER;

  tmp = (char *) osip_malloc (len);
  if (tmp == NULL)
    return OSIP_NOMEM;
  *dest = tmp;

  start = tmp;
  if (ainfo->auth_type != NULL) {
    tmp = osip_str_append (tmp, ainfo->auth_type);
    tmp = osip_str_append (tmp, " ");
    start = tmp;
  }

  if (ainfo->qop_options != NULL) {
    if (tmp != start) {
      tmp = osip_strn_append (tmp, ", ", 2);
    }
    tmp = osip_strn_append (tmp, "qop=", 4);
    tmp = osip_str_append (tmp, ainfo->qop_options);
  }
  if (ainfo->nextnonce != NULL) {
    if (tmp != start) {
      tmp = osip_strn_append (tmp, ", ", 2);
    }
    tmp = osip_strn_append (tmp, "nextnonce=", 10);
    tmp = osip_str_append (tmp, ainfo->nextnonce);
  }
  if (ainfo->rspauth != NULL) {
    if (tmp != start) {
      tmp = osip_strn_append (tmp, ", ", 2);
    }
    tmp = osip_strn_append (tmp, "rspauth=", 8);
    tmp = osip_str_append (tmp, ainfo->rspauth);
  }
  if (ainfo->cnonce != NULL) {
    if (tmp != start) {
      tmp = osip_strn_append (tmp, ", ", 2);
    }
    tmp = osip_strn_append (tmp, "cnonce=", 7);
    tmp = osip_str_append (tmp, ainfo->cnonce);
  }
  if (ainfo->nonce_count != NULL) {
    if (tmp != start) {
      tmp = osip_strn_append (tmp, ", ", 2);
    }
    tmp = osip_strn_append (tmp, "nc=", 3);
    tmp = osip_str_append (tmp, ainfo->nonce_count);
  }


  if (ainfo->snum != NULL) {
    if (tmp != start) {
      tmp = osip_strn_append (tmp, ", ", 2);
    }
    tmp = osip_strn_append (tmp, "snum=", 5);
    tmp = osip_str_append (tmp, ainfo->snum);
  }
  if (ainfo->srand != NULL) {
    if (tmp != start) {
      tmp = osip_strn_append (tmp, ", ", 2);
    }
    tmp = osip_strn_append (tmp, "srand=", 6);
    tmp = osip_str_append (tmp, ainfo->srand);
    }
  if (ainfo->targetname != NULL) {
    if (tmp != start) {
      tmp = osip_strn_append (tmp, ", ", 2);
    }
    tmp = osip_strn_append (tmp, "targetname=", 11);
    tmp = osip_str_append (tmp, ainfo->targetname);
  }
  if (ainfo->realm != NULL) {
    if (tmp != start) {
      tmp = osip_strn_append (tmp, ", ", 2);
    }
    tmp = osip_strn_append (tmp, "realm=", 6);
    tmp = osip_str_append (tmp, ainfo->realm);
  }
  if (ainfo->opaque != NULL) {
    if (tmp != start) {
      tmp = osip_strn_append (tmp, ", ", 2);
    }
    tmp = osip_strn_append (tmp, "opaque=", 7);
    tmp = osip_str_append (tmp, ainfo->opaque);
  }
  return OSIP_SUCCESS;
}

/* deallocates a osip_authentication_info_t structure.  */
/* INPUT : osip_authentication_info_t *authentication_info | authentication_info. */
void
osip_authentication_info_free (osip_authentication_info_t * authentication_info)
{
  if (authentication_info == NULL)
    return;

  osip_free (authentication_info->auth_type);
  osip_free (authentication_info->nextnonce);
  osip_free (authentication_info->rspauth);
  osip_free (authentication_info->cnonce);
  osip_free (authentication_info->nonce_count);
  osip_free (authentication_info->qop_options);
  osip_free (authentication_info->snum);
  osip_free (authentication_info->srand);
  osip_free (authentication_info->targetname);
  osip_free (authentication_info->realm);
  osip_free (authentication_info->opaque);
  osip_free (authentication_info);
}

int
osip_authentication_info_clone (const osip_authentication_info_t * ainfo, osip_authentication_info_t ** dest)
{
  int i;
  osip_authentication_info_t *wa;

  *dest = NULL;
  if (ainfo == NULL)
    return OSIP_BADPARAMETER;

  i = osip_authentication_info_init (&wa);
  if (i != 0)                   /* allocation failed */
    return i;
  if (ainfo->auth_type != NULL)
    wa->auth_type = osip_strdup (ainfo->auth_type);
  if (ainfo->nextnonce != NULL)
    wa->nextnonce = osip_strdup (ainfo->nextnonce);
  if (ainfo->cnonce != NULL)
    wa->cnonce = osip_strdup (ainfo->cnonce);
  if (ainfo->rspauth != NULL)
    wa->rspauth = osip_strdup (ainfo->rspauth);
  if (ainfo->nonce_count != NULL)
    wa->nonce_count = osip_strdup (ainfo->nonce_count);
  if (ainfo->qop_options != NULL)
    wa->qop_options = osip_strdup (ainfo->qop_options);
  if (ainfo->snum != NULL)
    wa->snum = osip_strdup (ainfo->snum);
  if (ainfo->srand != NULL)
    wa->srand = osip_strdup (ainfo->srand);
  if (ainfo->targetname != NULL)
    wa->targetname = osip_strdup (ainfo->targetname);
  if (ainfo->realm != NULL)
    wa->realm = osip_strdup (ainfo->realm);
  if (ainfo->opaque != NULL)
    wa->opaque = osip_strdup (ainfo->opaque);

  *dest = wa;
  return OSIP_SUCCESS;
}

#endif
