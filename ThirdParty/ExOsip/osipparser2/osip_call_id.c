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

/* fills the call_id of message.                    */
/* INPUT : const char *hvalue | value of header.    */
/* OUTPUT: osip_message_t *sip | structure to save results.  */
/* returns -1 on error. */
int
osip_message_set_call_id (osip_message_t * sip, const char *hvalue)
{
  int i;

  if (hvalue == NULL || hvalue[0] == '\0')
    return OSIP_SUCCESS;

  if (sip->call_id != NULL)
    return OSIP_SYNTAXERROR;
  i = osip_call_id_init (&(sip->call_id));
  if (i != 0)
    return i;
  sip->message_property = 2;
  i = osip_call_id_parse (sip->call_id, hvalue);
  if (i != 0) {
    osip_call_id_free (sip->call_id);
    sip->call_id = NULL;
    return i;
  }
  return OSIP_SUCCESS;
}

#ifndef MINISIZE
/* returns the call_id.               */
/* INPUT : osip_message_t *sip | sip message.  */
osip_call_id_t *
osip_message_get_call_id (const osip_message_t * sip)
{
  return sip->call_id;
}
#endif

int
osip_call_id_init (osip_call_id_t ** callid)
{
  *callid = (osip_call_id_t *) osip_malloc (sizeof (osip_call_id_t));
  if (*callid == NULL)
    return OSIP_NOMEM;
  (*callid)->number = NULL;
  (*callid)->host = NULL;
  return OSIP_SUCCESS;
}


/* deallocates a osip_call_id_t structure. */
/* INPUT : osip_call_id_t *| callid.       */
void
osip_call_id_free (osip_call_id_t * callid)
{
  if (callid == NULL)
    return;
  osip_free (callid->number);
  osip_free (callid->host);

  callid->number = NULL;
  callid->host = NULL;

  osip_free (callid);
}

/* fills the call_id structure.                           */
/* INPUT : char *hvalue | value of header.                */
/* OUTPUT: osip_call_id_t *callid | structure to save results.  */
/* returns -1 on error. */
int
osip_call_id_parse (osip_call_id_t * callid, const char *hvalue)
{
  const char *host;
  const char *end;

  callid->number = NULL;
  callid->host = NULL;

  host = strchr (hvalue, '@');  /* SEARCH FOR '@' */
  end = hvalue + strlen (hvalue);

  if (host == NULL)
    host = end;
  else {
    if (end - host + 1 < 2)
      return OSIP_SYNTAXERROR;
    callid->host = (char *) osip_malloc (end - host);
    if (callid->host == NULL)
      return OSIP_NOMEM;
    osip_clrncpy (callid->host, host + 1, end - host - 1);
  }
  if (host - hvalue + 1 < 2)
    return OSIP_SYNTAXERROR;
  callid->number = (char *) osip_malloc (host - hvalue + 1);
  if (callid->number == NULL)
    return OSIP_NOMEM;
  osip_clrncpy (callid->number, hvalue, host - hvalue);

  return OSIP_SUCCESS;          /* ok */
}

/* returns the call_id as a string.          */
/* INPUT : osip_call_id_t *call_id | call_id.  */
/* returns null on error. */
int
osip_call_id_to_str (const osip_call_id_t * callid, char **dest)
{
  *dest = NULL;
  if ((callid == NULL) || (callid->number == NULL))
    return OSIP_BADPARAMETER;

  if (callid->host == NULL) {
    *dest = (char *) osip_malloc (strlen (callid->number) + 1);
    if (*dest == NULL)
      return OSIP_NOMEM;
    sprintf (*dest, "%s", callid->number);
  }
  else {
    *dest = (char *) osip_malloc (strlen (callid->number) + strlen (callid->host) + 2);
    if (*dest == NULL)
      return OSIP_NOMEM;
    sprintf (*dest, "%s@%s", callid->number, callid->host);
  }
  return OSIP_SUCCESS;
}

char *
osip_call_id_get_number (osip_call_id_t * callid)
{
  if (callid == NULL)
    return NULL;
  return callid->number;
}

char *
osip_call_id_get_host (osip_call_id_t * callid)
{
  if (callid == NULL)
    return NULL;
  return callid->host;
}

void
osip_call_id_set_number (osip_call_id_t * callid, char *number)
{
  callid->number = number;
}

void
osip_call_id_set_host (osip_call_id_t * callid, char *host)
{
  callid->host = host;
}

int
osip_call_id_clone (const osip_call_id_t * callid, osip_call_id_t ** dest)
{
  int i;
  osip_call_id_t *ci;

  *dest = NULL;
  if (callid == NULL)
    return OSIP_BADPARAMETER;
  if (callid->number == NULL)
    return OSIP_BADPARAMETER;

  i = osip_call_id_init (&ci);
  if (i != 0)                   /* allocation failed */
    return i;
  ci->number = osip_strdup (callid->number);
  if (callid->host != NULL)
    ci->host = osip_strdup (callid->host);

  *dest = ci;
  return OSIP_SUCCESS;
}

int
osip_call_id_match (osip_call_id_t * callid1, osip_call_id_t * callid2)
{

  if (callid1 == NULL || callid2 == NULL)
    return OSIP_BADPARAMETER;
  if (callid1->number == NULL || callid2->number == NULL)
    return OSIP_BADPARAMETER;

  if (0 != strcmp (callid1->number, callid2->number))
    return OSIP_UNDEFINED_ERROR;

  if ((callid1->host == NULL) && (callid2->host == NULL))
    return OSIP_SUCCESS;
  if ((callid1->host == NULL) && (callid2->host != NULL))
    return OSIP_UNDEFINED_ERROR;
  if ((callid1->host != NULL) && (callid2->host == NULL))
    return OSIP_UNDEFINED_ERROR;
  if (0 != strcmp (callid1->host, callid2->host))
    return OSIP_UNDEFINED_ERROR;

  return OSIP_SUCCESS;
}
