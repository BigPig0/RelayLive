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

#include <osipparser2/osip_message.h>
#include <osipparser2/osip_parser.h>

#ifndef MINISIZE
int
osip_to_init (osip_to_t ** to)
{
  return osip_from_init ((osip_from_t **) to);
}
#endif

/* adds the to header to message.              */
/* INPUT : const char *hvalue | value of header.    */
/* OUTPUT: osip_message_t *sip | structure to save results.  */
/* returns -1 on error. */
int
osip_message_set_to (osip_message_t * sip, const char *hvalue)
{
  int i;

  if (hvalue == NULL || hvalue[0] == '\0')
    return OSIP_SUCCESS;

  if (sip->to != NULL)
    return OSIP_SYNTAXERROR;
  i = osip_to_init (&(sip->to));
  if (i != 0)
    return i;
  sip->message_property = 2;
  i = osip_to_parse (sip->to, hvalue);
  if (i != 0) {
    osip_to_free (sip->to);
    sip->to = NULL;
    return i;
  }
  return OSIP_SUCCESS;
}

#ifndef MINISIZE
/* returns the to header.            */
/* INPUT : osip_message_t *sip | sip message.   */
/* returns null on error. */
osip_to_t *
osip_message_get_to (const osip_message_t * sip)
{
  return sip->to;
}
#endif

#ifndef MINISIZE

int
osip_to_parse (osip_to_t * to, const char *hvalue)
{
  return osip_from_parse ((osip_from_t *) to, hvalue);
}

/* returns the to header as a string.          */
/* INPUT : osip_to_t *to | to header.  */
/* returns null on error. */
int
osip_to_to_str (const osip_to_t * to, char **dest)
{
  return osip_from_to_str ((osip_from_t *) to, dest);
}

/* deallocates a osip_to_t structure.  */
/* INPUT : osip_to_t *to | to header. */
void
osip_to_free (osip_to_t * to)
{
  osip_from_free ((osip_from_t *) to);
}

int
osip_to_clone (const osip_to_t * to, osip_to_t ** dest)
{
  return osip_from_clone ((osip_from_t *) to, (osip_from_t **) dest);
}

int
osip_to_tag_match (osip_to_t * to1, osip_to_t * to2)
{
  return osip_from_tag_match ((osip_from_t *) to1, (osip_from_t *) to2);
}

#endif
