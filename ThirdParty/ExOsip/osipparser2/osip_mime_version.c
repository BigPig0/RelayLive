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


/* adds the mime_version header to message.       */
/* INPUT : const char *hvalue | value of header.    */
/* OUTPUT: osip_message_t *sip | structure to save results.  */
/* returns -1 on error. */
int
osip_message_set_mime_version (osip_message_t * sip, const char *hvalue)
{
  int i;

  if (hvalue == NULL || hvalue[0] == '\0')
    return OSIP_SUCCESS;

  if (sip->mime_version != NULL)
    return OSIP_SYNTAXERROR;
  i = osip_mime_version_init (&(sip->mime_version));
  if (i != 0)
    return i;
  sip->message_property = 2;
  i = osip_mime_version_parse (sip->mime_version, hvalue);
  if (i != 0) {
    osip_mime_version_free (sip->mime_version);
    sip->mime_version = NULL;
    return i;
  }
  return OSIP_SUCCESS;
}

#ifndef MINISIZE
/* returns the mime_version header.            */
/* INPUT : osip_message_t *sip | sip message.   */
/* returns null on error. */
osip_mime_version_t *
osip_message_get_mime_version (const osip_message_t * sip)
{
  return sip->mime_version;
}
#endif
