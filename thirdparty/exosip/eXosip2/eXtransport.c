/*
  eXosip - This is the eXtended osip library.
  Copyright (C) 2001-2015 Aymeric MOIZARD amoizard@antisip.com
  
  eXosip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  eXosip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  In addition, as a special exception, the copyright holders give
  permission to link the code of portions of this program with the
  OpenSSL library under certain conditions as described in each
  individual source file, and distribute linked combinations
  including the two.
  You must obey the GNU General Public License in all respects
  for all of the code used other than OpenSSL.  If you modify
  file(s) with this exception, you may extend this exception to your
  version of the file(s), but you are not obligated to do so.  If you
  do not wish to do so, delete this exception statement from your
  version.  If you delete this exception statement from all source
  files in the program, then also delete it here.
*/

#include "eXosip2.h"

char *
_eXosip_transport_protocol (osip_message_t * msg)
{
  osip_via_t *via;

  via = (osip_via_t *) osip_list_get (&msg->vias, 0);
  if (via == NULL || via->protocol == NULL)
    return NULL;
  return via->protocol;
}

int
_eXosip_find_protocol (osip_message_t * msg)
{
  osip_via_t *via;

  via = (osip_via_t *) osip_list_get (&msg->vias, 0);
  if (via == NULL || via->protocol == NULL)
    return -1;
  else if (0 == osip_strcasecmp (via->protocol, "UDP"))
    return IPPROTO_UDP;
  else if (0 == osip_strcasecmp (via->protocol, "TCP"))
    return IPPROTO_TCP;
  return -1;;
}


int
eXosip_transport_set (osip_message_t * msg, const char *transport)
{
  osip_via_t *via;

  via = (osip_via_t *) osip_list_get (&msg->vias, 0);
  if (via == NULL || via->protocol == NULL)
    return -1;

  if (0 == osip_strcasecmp (via->protocol, transport))
    return OSIP_SUCCESS;

  osip_free (via->protocol);
  via->protocol = osip_strdup (transport);
  return OSIP_SUCCESS;
}
