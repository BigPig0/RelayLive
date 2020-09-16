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
osip_message_set_alert_info (osip_message_t * sip, const char *hvalue)
{
  osip_alert_info_t *alert_info;
  int i;

  if (hvalue == NULL || hvalue[0] == '\0')
    return OSIP_SUCCESS;

  i = osip_alert_info_init (&alert_info);
  if (i != 0)
    return i;
  i = osip_alert_info_parse (alert_info, hvalue);
  if (i != 0) {
    osip_alert_info_free (alert_info);
    return i;
  }
  sip->message_property = 2;
  osip_list_add (&sip->alert_infos, alert_info, -1);
  return OSIP_SUCCESS;
}

int
osip_message_get_alert_info (const osip_message_t * sip, int pos, osip_alert_info_t ** dest)
{
  osip_alert_info_t *alert_info;

  *dest = NULL;
  if (osip_list_size (&sip->alert_infos) <= pos)
    return OSIP_UNDEFINED_ERROR;        /* does not exist */
  alert_info = (osip_alert_info_t *) osip_list_get (&sip->alert_infos, pos);
  *dest = alert_info;
  return pos;
}

#endif
