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


/* Add a header to a SIP message.                           */
/* INPUT :  char *hname | pointer to a header name.         */
/* INPUT :  char *hvalue | pointer to a header value.       */
/* OUTPUT: osip_message_t *sip | structure to save results.          */
/* returns -1 on error. */
int
osip_message_set_header (osip_message_t * sip, const char *hname, const char *hvalue)
{
  osip_header_t *h;
  int i;

  if (sip == NULL || hname == NULL)
    return OSIP_BADPARAMETER;

  i = osip_header_init (&h);
  if (i != 0)
    return i;

  h->hname = (char *) osip_malloc (strlen (hname) + 1);

  if (h->hname == NULL) {
    osip_header_free (h);
    return OSIP_NOMEM;
  }
  osip_clrncpy (h->hname, hname, strlen (hname));

  if (hvalue != NULL) {         /* some headers can be null ("subject:") */
    h->hvalue = (char *) osip_malloc (strlen (hvalue) + 1);
    if (h->hvalue == NULL) {
      osip_header_free (h);
      return OSIP_NOMEM;
    }
    osip_clrncpy (h->hvalue, hvalue, strlen (hvalue));
  }
  else
    h->hvalue = NULL;
  sip->message_property = 2;
  osip_list_add (&sip->headers, h, -1);
  return OSIP_SUCCESS;          /* ok */
}

/* Add a or replace exising header  to a SIP message.       */
/* INPUT :  char *hname | pointer to a header name.         */
/* INPUT :  char *hvalue | pointer to a header value.       */
/* OUTPUT: osip_message_t *sip | structure to save results. */
/* returns -1 on error. */
int
osip_message_replace_header (osip_message_t * sip, const char *hname, const char *hvalue)
{
  osip_header_t *h, *oldh;
  int i, oldpos = -1;

  if (sip == NULL || hname == NULL)
    return OSIP_BADPARAMETER;

  oldpos = osip_message_header_get_byname (sip, hname, 0, &oldh);

  i = osip_header_init (&h);
  if (i != 0)
    return i;

  h->hname = (char *) osip_malloc (strlen (hname) + 1);

  if (h->hname == NULL) {
    osip_header_free (h);
    return OSIP_NOMEM;
  }
  osip_clrncpy (h->hname, hname, strlen (hname));

  if (hvalue != NULL) {         /* some headers can be null ("subject:") */
    h->hvalue = (char *) osip_malloc (strlen (hvalue) + 1);
    if (h->hvalue == NULL) {
      osip_header_free (h);
      return OSIP_NOMEM;
    }
    osip_clrncpy (h->hvalue, hvalue, strlen (hvalue));
  }
  else
    h->hvalue = NULL;

  if (oldpos != -1) {
    osip_list_remove (&sip->headers, oldpos);
    osip_header_free (oldh);
  }

  sip->message_property = 2;
  osip_list_add (&sip->headers, h, -1);
  return OSIP_SUCCESS;          /* ok */
}


#ifndef MINISIZE
/* Add a header to a SIP message at the top of the list.    */
/* INPUT :  char *hname | pointer to a header name.         */
/* INPUT :  char *hvalue | pointer to a header value.       */
/* OUTPUT: osip_message_t *sip | structure to save results.          */
/* returns -1 on error. */
int
osip_message_set_topheader (osip_message_t * sip, const char *hname, const char *hvalue)
{
  osip_header_t *h;
  int i;

  if (sip == NULL || hname == NULL)
    return OSIP_BADPARAMETER;

  i = osip_header_init (&h);
  if (i != 0)
    return i;

  h->hname = (char *) osip_malloc (strlen (hname) + 1);

  if (h->hname == NULL) {
    osip_header_free (h);
    return OSIP_NOMEM;
  }
  osip_clrncpy (h->hname, hname, strlen (hname));

  if (hvalue != NULL) {         /* some headers can be null ("subject:") */
    h->hvalue = (char *) osip_malloc (strlen (hvalue) + 1);
    if (h->hvalue == NULL) {
      osip_header_free (h);
      return OSIP_NOMEM;
    }
    osip_clrncpy (h->hvalue, hvalue, strlen (hvalue));
  }
  else
    h->hvalue = NULL;
  sip->message_property = 2;
  osip_list_add (&sip->headers, h, 0);
  return OSIP_SUCCESS;          /* ok */
}

/* Get a header in a SIP message.                       */
/* INPUT : int pos | position of number in message.     */
/* OUTPUT: osip_message_t *sip | structure to scan for a header .*/
/* return null on error. */
int
osip_message_get_header (const osip_message_t * sip, int pos, osip_header_t ** dest)
{
  *dest = NULL;
  if (osip_list_size (&sip->headers) <= pos)
    return OSIP_UNDEFINED_ERROR;        /* NULL */
  *dest = (osip_header_t *) osip_list_get (&sip->headers, pos);
  return pos;
}
#endif

/* Get a header in a SIP message.                       */
/* INPUT : int pos | position where we start the search */
/* OUTPUT: osip_message_t *sip | structure to look for header.   */
/* return the current position of the header found      */
/* and -1 on error. */
int
osip_message_header_get_byname (const osip_message_t * sip, const char *hname, int pos, osip_header_t ** dest)
{
  int i;
  osip_header_t *tmp;

  *dest = NULL;
  i = pos;
  if (osip_list_size (&sip->headers) <= pos)
    return OSIP_UNDEFINED_ERROR;        /* NULL */
  while (osip_list_size (&sip->headers) > i) {
    tmp = (osip_header_t *) osip_list_get (&sip->headers, i);
    if (osip_strcasecmp (tmp->hname, hname) == 0) {
      *dest = tmp;
      return i;
    }
    i++;
  }
  return OSIP_UNDEFINED_ERROR;  /* not found */
}

int
osip_header_init (osip_header_t ** header)
{
  *header = (osip_header_t *) osip_malloc (sizeof (osip_header_t));
  if (*header == NULL)
    return OSIP_NOMEM;
  (*header)->hname = NULL;
  (*header)->hvalue = NULL;
  return OSIP_SUCCESS;
}

void
osip_header_free (osip_header_t * header)
{
  if (header == NULL)
    return;
  osip_free (header->hname);
  osip_free (header->hvalue);
  header->hname = NULL;
  header->hvalue = NULL;

  osip_free (header);
}

/* returns the header as a string.    */
/* INPUT : osip_header_t *header | header. */
/* returns null on error. */
int
osip_header_to_str (const osip_header_t * header, char **dest)
{
  size_t len, hlen;

  *dest = NULL;
  if ((header == NULL) || (header->hname == NULL))
    return OSIP_BADPARAMETER;

  len = 0;
  hlen = strlen (header->hname);
  if (header->hvalue != NULL)
    len = strlen (header->hvalue);

  *dest = (char *) osip_malloc (hlen + len + 3);
  if (*dest == NULL)
    return OSIP_NOMEM;

  if (header->hvalue != NULL)
    snprintf (*dest, hlen + len + 3, "%s: %s", header->hname, header->hvalue);
  else
    snprintf (*dest, hlen + len + 3, "%s: ", header->hname);

  if (*dest[0] >= 'a' && *dest[0] <= 'z')
    *dest[0] = (*dest[0] - 32);
  return OSIP_SUCCESS;
}

char *
osip_header_get_name (const osip_header_t * header)
{
  if (header == NULL)
    return NULL;
  return header->hname;
}

void
osip_header_set_name (osip_header_t * header, char *name)
{
  header->hname = name;
}

char *
osip_header_get_value (const osip_header_t * header)
{
  if (header == NULL)
    return NULL;
  return header->hvalue;
}

void
osip_header_set_value (osip_header_t * header, char *value)
{
  header->hvalue = value;
}

int
osip_header_clone (const osip_header_t * header, osip_header_t ** dest)
{
  int i;
  osip_header_t *he;

  *dest = NULL;
  if (header == NULL)
    return OSIP_BADPARAMETER;
  if (header->hname == NULL)
    return OSIP_BADPARAMETER;

  i = osip_header_init (&he);
  if (i != 0)
    return i;
  he->hname = osip_strdup (header->hname);

  if (he->hname == NULL) {
    osip_header_free (he);
    return OSIP_NOMEM;
  }
  if (header->hvalue != NULL) {
    he->hvalue = osip_strdup (header->hvalue);
    if (he->hvalue == NULL) {
      osip_header_free (he);
      return OSIP_NOMEM;
    }
  }

  *dest = he;
  return OSIP_SUCCESS;
}
