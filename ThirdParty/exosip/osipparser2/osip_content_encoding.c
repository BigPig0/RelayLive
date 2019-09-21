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

/* content-Encoding = token
   token possible values are gzip,compress,deflate
*/
int
osip_message_set_content_encoding (osip_message_t * sip, const char *hvalue)
{
  osip_content_encoding_t *content_encoding;
  int i;

  if (hvalue == NULL || hvalue[0] == '\0')
    return OSIP_SUCCESS;

  i = osip_content_encoding_init (&content_encoding);
  if (i != 0)
    return i;
  i = osip_content_encoding_parse (content_encoding, hvalue);
  if (i != 0) {
    osip_content_encoding_free (content_encoding);
    return i;
  }
  sip->message_property = 2;
  osip_list_add (&sip->content_encodings, content_encoding, -1);
  return OSIP_SUCCESS;
}

int
osip_message_get_content_encoding (const osip_message_t * sip, int pos, osip_content_encoding_t ** dest)
{
  osip_content_encoding_t *ce;

  *dest = NULL;
  if (osip_list_size (&sip->content_encodings) <= pos)
    return OSIP_UNDEFINED_ERROR;        /* does not exist */
  ce = (osip_content_encoding_t *) osip_list_get (&sip->content_encodings, pos);
  *dest = ce;
  return pos;
}

#endif
