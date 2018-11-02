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
osip_message_set_allow (osip_message_t * sip, const char *hvalue)
{
  osip_allow_t *allow;
  int i;

  if (hvalue == NULL || hvalue[0] == '\0')
    return OSIP_SUCCESS;

  i = osip_allow_init (&allow);
  if (i != 0)
    return i;
  i = osip_allow_parse (allow, hvalue);
  if (i != 0) {
    osip_allow_free (allow);
    return i;
  }
  sip->message_property = 2;
  osip_list_add (&sip->allows, allow, -1);
  return OSIP_SUCCESS;
}

int
osip_message_get_allow (const osip_message_t * sip, int pos, osip_allow_t ** dest)
{
  osip_allow_t *allow;

  *dest = NULL;
  if (osip_list_size (&sip->allows) <= pos)
    return OSIP_UNDEFINED_ERROR;        /* does not exist */
  allow = (osip_allow_t *) osip_list_get (&sip->allows, pos);
  *dest = allow;
  return pos;
}

#endif
