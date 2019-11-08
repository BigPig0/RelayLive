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
osip_www_authenticate_init (osip_www_authenticate_t ** dest)
{
  *dest = (osip_www_authenticate_t *) osip_malloc (sizeof (osip_www_authenticate_t));
  if (*dest == NULL)
    return OSIP_NOMEM;
  memset (*dest, 0, sizeof (osip_www_authenticate_t));
  return OSIP_SUCCESS;
}

/* fills the www-authenticate header of message.               */
/* INPUT :  char *hvalue | value of header.   */
/* OUTPUT: osip_message_t *sip | structure to save results. */
/* returns -1 on error. */
int
osip_message_set_www_authenticate (osip_message_t * sip, const char *hvalue)
{
  osip_www_authenticate_t *www_authenticate;
  int i;

  if (hvalue == NULL || hvalue[0] == '\0')
    return OSIP_SUCCESS;

  if (sip == NULL)
    return OSIP_BADPARAMETER;
  i = osip_www_authenticate_init (&www_authenticate);
  if (i != 0)
    return i;
  i = osip_www_authenticate_parse (www_authenticate, hvalue);
  if (i != 0) {
    osip_www_authenticate_free (www_authenticate);
    return i;
  }
  sip->message_property = 2;
  osip_list_add (&sip->www_authenticates, www_authenticate, -1);
  return OSIP_SUCCESS;
}

int
__osip_quoted_string_set (const char *name, const char *str, char **result, const char **next)
{
  *next = str;
  if (*result != NULL)
    return OSIP_SUCCESS;        /* already parsed */
  *next = NULL;
  while ((' ' == *str) || ('\t' == *str) || (',' == *str))
    if (*str)
      str++;
    else
      return OSIP_SYNTAXERROR;  /* bad header format */

  if (osip_strncasecmp (name, str, strlen (name)) == 0) {
    const char *quote1;
    const char *quote2;
    const char *tmp;
    const char *hack = strchr (str, '=');

    if (hack == NULL)
      return OSIP_SYNTAXERROR;

    while (' ' == *(hack - 1))  /* get rid of extra spaces */
      hack--;
    if ((size_t) (hack - str) != strlen (name)) {
      *next = str;
      return OSIP_SUCCESS;
    }

    quote1 = __osip_quote_find (str);
    if (quote1 == NULL)
      return OSIP_SYNTAXERROR;  /* bad header format... */
    quote2 = __osip_quote_find (quote1 + 1);
    if (quote2 == NULL)
      return OSIP_SYNTAXERROR;  /* bad header format... */
    if (quote2 - quote1 == 1) {
      /* this is a special case! The quote contains nothing! */
      /* example:   Digest opaque="",cnonce=""               */
      /* in this case, we just forget the parameter... this  */
      /* this should prevent from user manipulating empty    */
      /* strings */
      tmp = quote2 + 1;         /* next element start here */
      for (; *tmp == ' ' || *tmp == '\t'; tmp++) {
      }
      for (; *tmp == '\n' || *tmp == '\r'; tmp++) {
      }                         /* skip LWS */
      *next = NULL;
      if (*tmp == '\0')         /* end of header detected */
        return OSIP_SUCCESS;
      if (*tmp != '\t' && *tmp != ' ')
        /* LWS here ? */
        *next = tmp;
      else {                    /* it is: skip it... */
        for (; *tmp == ' ' || *tmp == '\t'; tmp++) {
        }
        if (*tmp == '\0')       /* end of header detected */
          return OSIP_SUCCESS;
        *next = tmp;
      }
      return OSIP_SUCCESS;
    }
    *result = (char *) osip_malloc (quote2 - quote1 + 3);
    if (*result == NULL)
      return OSIP_NOMEM;
    osip_strncpy (*result, quote1, quote2 - quote1 + 1);
    tmp = quote2 + 1;           /* next element start here */
    for (; *tmp == ' ' || *tmp == '\t'; tmp++) {
    }
    for (; *tmp == '\n' || *tmp == '\r'; tmp++) {
    }                           /* skip LWS */
    *next = NULL;
    if (*tmp == '\0')           /* end of header detected */
      return OSIP_SUCCESS;
    if (*tmp != '\t' && *tmp != ' ')
      /* LWS here ? */
      *next = tmp;
    else {                      /* it is: skip it... */
      for (; *tmp == ' ' || *tmp == '\t'; tmp++) {
      }
      if (*tmp == '\0')         /* end of header detected */
        return OSIP_SUCCESS;
      *next = tmp;
    }
  }
  else
    *next = str;                /* wrong header asked! */
  return OSIP_SUCCESS;
}

int
__osip_token_set (const char *name, const char *str, char **result, const char **next)
{
  const char *beg;
  const char *tmp;

  *next = str;
  if (*result != NULL)
    return OSIP_SUCCESS;        /* already parsed */
  *next = NULL;

  beg = strchr (str, '=');
  if (beg == NULL)
    return OSIP_SYNTAXERROR;    /* bad header format... */

  if (strlen (str) < 6)
    return OSIP_SUCCESS;        /* end of header... */

  while ((' ' == *str) || ('\t' == *str) || (',' == *str))
    if (*str)
      str++;
    else
      return OSIP_SYNTAXERROR;  /* bad header format */

  if (osip_strncasecmp (name, str, strlen (name)) == 0) {
    const char *end;

    end = strchr (str, ',');
    if (end == NULL)
      end = str + strlen (str); /* This is the end of the header */

    if (end - beg < 2)
      return OSIP_SYNTAXERROR;
    *result = (char *) osip_malloc (end - beg);
    if (*result == NULL)
      return OSIP_NOMEM;
    osip_clrncpy (*result, beg + 1, end - beg - 1);

    /* make sure the element does not contain more parameter */
    tmp = (*end) ? (end + 1) : end;
    for (; *tmp == ' ' || *tmp == '\t'; tmp++) {
    }
    for (; *tmp == '\n' || *tmp == '\r'; tmp++) {
    }                           /* skip LWS */
    *next = NULL;
    if (*tmp == '\0')           /* end of header detected */
      return OSIP_SUCCESS;
    if (*tmp != '\t' && *tmp != ' ')
      /* LWS here ? */
      *next = tmp;
    else {                      /* it is: skip it... */
      for (; *tmp == ' ' || *tmp == '\t'; tmp++) {
      }
      if (*tmp == '\0')         /* end of header detected */
        return OSIP_SUCCESS;
      *next = tmp;
    }
  }
  else
    *next = str;                /* next element start here */
  return OSIP_SUCCESS;
}

/* fills the www-authenticate strucuture.                      */
/* INPUT : char *hvalue | value of header.         */
/* OUTPUT: osip_message_t *sip | structure to save results. */
/* returns -1 on error. */
/* TODO:
   digest-challenge tken has no order preference??
   verify many situations (extra SP....)
*/
int
osip_www_authenticate_parse (osip_www_authenticate_t * wwwa, const char *hvalue)
{
  const char *space;
  const char *next = NULL;
  int i;

  space = strchr (hvalue, ' '); /* SEARCH FOR SPACE */
  if (space == NULL)
    return OSIP_SYNTAXERROR;

  if (space - hvalue + 1 < 2)
    return OSIP_SYNTAXERROR;
  wwwa->auth_type = (char *) osip_malloc (space - hvalue + 1);
  if (wwwa->auth_type == NULL)
    return OSIP_NOMEM;
  osip_strncpy (wwwa->auth_type, hvalue, space - hvalue);

  for (;;) {
    int parse_ok = 0;

    i = __osip_quoted_string_set ("realm", space, &(wwwa->realm), &next);
    if (i != 0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;      /* end of header detected! */
    else if (next != space) {
      space = next;
      parse_ok++;
    }
    i = __osip_quoted_string_set ("domain", space, &(wwwa->domain), &next);
    if (i != 0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;      /* end of header detected! */
    else if (next != space) {
      space = next;
      parse_ok++;
    }
    i = __osip_quoted_string_set ("nonce", space, &(wwwa->nonce), &next);
    if (i != 0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;      /* end of header detected! */
    else if (next != space) {
      space = next;
      parse_ok++;
    }
    i = __osip_quoted_string_set ("opaque", space, &(wwwa->opaque), &next);
    if (i != 0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;      /* end of header detected! */
    else if (next != space) {
      space = next;
      parse_ok++;
    }
    i = __osip_token_set ("stale", space, &(wwwa->stale), &next);
    if (i != 0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;      /* end of header detected! */
    else if (next != space) {
      space = next;
      parse_ok++;
    }
    i = __osip_token_set ("algorithm", space, &(wwwa->algorithm), &next);
    if (i != 0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;      /* end of header detected! */
    else if (next != space) {
      space = next;
      parse_ok++;
    }
    i = __osip_quoted_string_set ("qop", space, &(wwwa->qop_options), &next);
    if (i != 0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;      /* end of header detected! */
    else if (next != space) {
      space = next;
      parse_ok++;
    }
    i = __osip_token_set ("version", space, &(wwwa->version), &next);
    if (i!=0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;               /* end of header detected! */
    else if (next != space)
      {
        space = next;
        parse_ok++;
      }
       i = __osip_quoted_string_set ("targetname", space, &(wwwa->targetname), &next);
    if (i!=0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;               /* end of header detected! */
    else if (next != space)
      {
        space = next;
        parse_ok++;
      }
       i = __osip_quoted_string_set ("gssapi-data", space, &(wwwa->gssapi_data), &next);
    if (i!=0)
      return i;
    if (next == NULL)
      return OSIP_SUCCESS;               /* end of header detected! */
    else if (next != space)
      {
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

#ifndef MINISIZE
/* returns the www_authenticate header.            */
/* INPUT : osip_message_t *sip | sip message.   */
/* returns null on error. */
int
osip_message_get_www_authenticate (const osip_message_t * sip, int pos, osip_www_authenticate_t ** dest)
{
  osip_www_authenticate_t *www_authenticate;

  *dest = NULL;
  if (osip_list_size (&sip->www_authenticates) <= pos)
    return OSIP_UNDEFINED_ERROR;        /* does not exist */

  www_authenticate = (osip_www_authenticate_t *) osip_list_get (&sip->www_authenticates, pos);

  *dest = www_authenticate;
  return pos;
}
#endif

char *
osip_www_authenticate_get_auth_type (osip_www_authenticate_t * www_authenticate)
{
  return www_authenticate->auth_type;
}

void
osip_www_authenticate_set_auth_type (osip_www_authenticate_t * www_authenticate, char *auth_type)
{
  www_authenticate->auth_type = (char *) auth_type;
}

char *
osip_www_authenticate_get_realm (osip_www_authenticate_t * www_authenticate)
{
  return www_authenticate->realm;
}

void
osip_www_authenticate_set_realm (osip_www_authenticate_t * www_authenticate, char *realm)
{
  www_authenticate->realm = (char *) realm;
}

char *
osip_www_authenticate_get_domain (osip_www_authenticate_t * www_authenticate)
{
  return www_authenticate->domain;
}

void
osip_www_authenticate_set_domain (osip_www_authenticate_t * www_authenticate, char *domain)
{
  www_authenticate->domain = (char *) domain;
}

char *
osip_www_authenticate_get_nonce (osip_www_authenticate_t * www_authenticate)
{
  return www_authenticate->nonce;
}

void
osip_www_authenticate_set_nonce (osip_www_authenticate_t * www_authenticate, char *nonce)
{
  www_authenticate->nonce = (char *) nonce;
}

char *
osip_www_authenticate_get_stale (osip_www_authenticate_t * www_authenticate)
{
  return www_authenticate->stale;
}

void
osip_www_authenticate_set_stale (osip_www_authenticate_t * www_authenticate, char *stale)
{
  www_authenticate->stale = (char *) stale;
}

char *
osip_www_authenticate_get_opaque (osip_www_authenticate_t * www_authenticate)
{
  return www_authenticate->opaque;
}

void
osip_www_authenticate_set_opaque (osip_www_authenticate_t * www_authenticate, char *opaque)
{
  www_authenticate->opaque = (char *) opaque;
}

char *
osip_www_authenticate_get_algorithm (osip_www_authenticate_t * www_authenticate)
{
  return www_authenticate->algorithm;
}

void
osip_www_authenticate_set_algorithm (osip_www_authenticate_t * www_authenticate, char *algorithm)
{
  www_authenticate->algorithm = (char *) algorithm;
}

char *
osip_www_authenticate_get_qop_options (osip_www_authenticate_t * www_authenticate)
{
  return www_authenticate->qop_options;
}

void
osip_www_authenticate_set_qop_options (osip_www_authenticate_t * www_authenticate, char *qop_options)
{
  www_authenticate->qop_options = (char *) qop_options;
}

char *
osip_www_authenticate_get_version (osip_www_authenticate_t * www_authenticate)
{
  return www_authenticate->version;
}

void
osip_www_authenticate_set_version (osip_www_authenticate_t *
				   www_authenticate, char *version)
{
  www_authenticate->version = (char *) version;
}

char *
osip_www_authenticate_get_targetname (osip_www_authenticate_t * www_authenticate)
{
  return www_authenticate->targetname;
}

void
osip_www_authenticate_set_targetname (osip_www_authenticate_t *
				      www_authenticate, char *targetname)
{
  www_authenticate->targetname = (char *) targetname;
}

char *
osip_www_authenticate_get_gssapi_data (osip_www_authenticate_t * www_authenticate)
{
  return www_authenticate->gssapi_data;
}

void
osip_www_authenticate_set_gssapi_data (osip_www_authenticate_t *
                                       www_authenticate, char *gssapi_data)
{
  www_authenticate->gssapi_data = (char *) gssapi_data;
}



/* returns the www_authenticate header as a string.          */
/* INPUT : osip_www_authenticate_t *www_authenticate | www_authenticate header.  */
/* returns null on error. */
int
osip_www_authenticate_to_str (const osip_www_authenticate_t * wwwa, char **dest)
{
  size_t len;
  char *tmp;

  *dest = NULL;
  if ((wwwa == NULL) || (wwwa->auth_type == NULL))
    return OSIP_BADPARAMETER;

  len = strlen (wwwa->auth_type) + 1;

  if (wwwa->realm != NULL)
    len = len + strlen (wwwa->realm) + 7;
  if (wwwa->nonce != NULL)
    len = len + strlen (wwwa->nonce) + 8;
  len = len + 2;
  if (wwwa->domain != NULL)
    len = len + strlen (wwwa->domain) + 9;
  if (wwwa->opaque != NULL)
    len = len + strlen (wwwa->opaque) + 9;
  if (wwwa->stale != NULL)
    len = len + strlen (wwwa->stale) + 8;
  if (wwwa->algorithm != NULL)
    len = len + strlen (wwwa->algorithm) + 12;
  if (wwwa->qop_options != NULL)
    len = len + strlen (wwwa->qop_options) + 6;
  if (wwwa->version != NULL)
    len = len + strlen (wwwa->version) + 10;
  if (wwwa->targetname != NULL)
    len = len + strlen (wwwa->targetname) + 13;
  if (wwwa->gssapi_data != NULL)
    len = len + strlen (wwwa->gssapi_data) + 14;

  tmp = (char *) osip_malloc (len);
  if (tmp == NULL)
    return OSIP_NOMEM;
  *dest = tmp;

  tmp = osip_str_append (tmp, wwwa->auth_type);

  if (wwwa->realm != NULL) {
    tmp = osip_strn_append (tmp, " realm=", 7);
    tmp = osip_str_append (tmp, wwwa->realm);
  }
  if (wwwa->domain != NULL) {
    tmp = osip_strn_append (tmp, ", domain=", 9);
    tmp = osip_str_append (tmp, wwwa->domain);
  }
  if (wwwa->nonce != NULL) {
    tmp = osip_strn_append (tmp, ", nonce=", 8);
    tmp = osip_str_append (tmp, wwwa->nonce);
  }
  if (wwwa->opaque != NULL) {
    tmp = osip_strn_append (tmp, ", opaque=", 9);
    tmp = osip_str_append (tmp, wwwa->opaque);
  }
  if (wwwa->stale != NULL) {
    tmp = osip_strn_append (tmp, ", stale=", 8);
    tmp = osip_str_append (tmp, wwwa->stale);
  }
  if (wwwa->algorithm != NULL) {
    tmp = osip_strn_append (tmp, ", algorithm=", 12);
    tmp = osip_str_append (tmp, wwwa->algorithm);
  }
  if (wwwa->qop_options != NULL) {
    tmp = osip_strn_append (tmp, ", qop=", 6);
    tmp = osip_str_append (tmp, wwwa->qop_options);
  }
  if (wwwa->version != NULL) {
      tmp = osip_strn_append (tmp, ", version=", 10);
      tmp = osip_str_append (tmp, wwwa->version);
  }
  if (wwwa->targetname != NULL) {
      tmp = osip_strn_append (tmp, ", targetname=", 13);
      tmp = osip_str_append (tmp, wwwa->targetname);
  }
  if (wwwa->gssapi_data != NULL) {
      tmp = osip_strn_append (tmp, ", gssapi-data=", 14);
      tmp = osip_str_append (tmp, wwwa->gssapi_data);
  }
  if (wwwa->realm == NULL) {
    /* remove comma */
    len = strlen (wwwa->auth_type);
    if ((*dest)[len] == ',')
      (*dest)[len] = ' ';
  }

  return OSIP_SUCCESS;
}

/* deallocates a osip_www_authenticate_t structure.  */
/* INPUT : osip_www_authenticate_t *www_authenticate | www_authenticate. */
void
osip_www_authenticate_free (osip_www_authenticate_t * www_authenticate)
{
  if (www_authenticate == NULL)
    return;

  osip_free (www_authenticate->auth_type);
  osip_free (www_authenticate->realm);
  osip_free (www_authenticate->domain);
  osip_free (www_authenticate->nonce);
  osip_free (www_authenticate->opaque);
  osip_free (www_authenticate->stale);
  osip_free (www_authenticate->algorithm);
  osip_free (www_authenticate->qop_options);
  osip_free (www_authenticate->version);
  osip_free (www_authenticate->targetname);
  osip_free (www_authenticate->gssapi_data);

  osip_free (www_authenticate);
}

int
osip_www_authenticate_clone (const osip_www_authenticate_t * wwwa, osip_www_authenticate_t ** dest)
{
  int i;
  osip_www_authenticate_t *wa;

  *dest = NULL;
  if (wwwa == NULL)
    return OSIP_BADPARAMETER;
  if (wwwa->auth_type == NULL)
    return OSIP_BADPARAMETER;

  i = osip_www_authenticate_init (&wa);
  if (i != 0)                   /* allocation failed */
    return i;
  wa->auth_type = osip_strdup (wwwa->auth_type);
  if (wa->auth_type == NULL && wwwa->auth_type != NULL) {
    osip_www_authenticate_free (wa);
    return OSIP_NOMEM;
  }
  if (wwwa->realm != NULL)
    wa->realm = osip_strdup (wwwa->realm);
  if (wa->realm == NULL && wwwa->realm != NULL) {
    osip_www_authenticate_free (wa);
    return OSIP_NOMEM;
  }
  if (wwwa->domain != NULL)
    wa->domain = osip_strdup (wwwa->domain);
  if (wa->domain == NULL && wwwa->domain != NULL) {
    osip_www_authenticate_free (wa);
    return OSIP_NOMEM;
  }
  if (wwwa->nonce != NULL)
    wa->nonce = osip_strdup (wwwa->nonce);
  if (wa->nonce == NULL && wwwa->nonce != NULL) {
    osip_www_authenticate_free (wa);
    return OSIP_NOMEM;
  }
  if (wwwa->opaque != NULL)
    wa->opaque = osip_strdup (wwwa->opaque);
  if (wa->opaque == NULL && wwwa->opaque != NULL) {
    osip_www_authenticate_free (wa);
    return OSIP_NOMEM;
  }
  if (wwwa->stale != NULL)
    wa->stale = osip_strdup (wwwa->stale);
  if (wa->stale == NULL && wwwa->stale != NULL) {
    osip_www_authenticate_free (wa);
    return OSIP_NOMEM;
  }
  if (wwwa->algorithm != NULL)
    wa->algorithm = osip_strdup (wwwa->algorithm);
  if (wa->algorithm == NULL && wwwa->algorithm != NULL) {
    osip_www_authenticate_free (wa);
    return OSIP_NOMEM;
  }
  if (wwwa->qop_options != NULL)
    wa->qop_options = osip_strdup (wwwa->qop_options);
  if (wa->qop_options == NULL && wwwa->qop_options != NULL) {
    osip_www_authenticate_free (wa);
    return OSIP_NOMEM;
  }
  if (wwwa->version != NULL)
    wa->version = osip_strdup (wwwa->version);
  if (wa->version==NULL && wwwa->version!=NULL)
  {
	  osip_www_authenticate_free (wa);
	  return OSIP_NOMEM;
  }
  if (wwwa->targetname != NULL)
    wa->targetname = osip_strdup (wwwa->targetname);
  if (wa->targetname==NULL && wwwa->targetname!=NULL)
  {
	  osip_www_authenticate_free (wa);
	  return OSIP_NOMEM;
  }
  if (wwwa->gssapi_data != NULL)
    wa->gssapi_data = osip_strdup (wwwa->gssapi_data);
  if (wa->gssapi_data==NULL && wwwa->gssapi_data!=NULL)
  {
	  osip_www_authenticate_free (wa);
	  return OSIP_NOMEM;
  }

  *dest = wa;
  return OSIP_SUCCESS;
}
