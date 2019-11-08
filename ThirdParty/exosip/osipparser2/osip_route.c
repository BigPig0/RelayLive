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

#ifndef MINISIZE
int
osip_route_init (osip_route_t ** route)
{
  return osip_from_init ((osip_from_t **) route);
}
#endif

/* adds the route header to message.         */
/* INPUT : const char *hvalue | value of header.    */
/* OUTPUT: osip_message_t *sip | structure to save results.  */
/* returns -1 on error. */
int
osip_message_set_route (osip_message_t * sip, const char *hvalue)
{
  osip_route_t *route;
  int i;

  if (hvalue == NULL || hvalue[0] == '\0')
    return OSIP_SUCCESS;

#ifdef __VXWORKS_OS__
  i = osip_route_init2 (&route);
#else
  i = osip_route_init (&route);
#endif
  if (i != 0)
    return i;
  i = osip_route_parse (route, hvalue);
  if (i != 0) {
    osip_route_free (route);
    return i;
  }
  sip->message_property = 2;
  osip_list_add (&sip->routes, route, -1);
  return OSIP_SUCCESS;
}

#ifndef MINISIZE
/* returns the route header.    */
/* INPUT : osip_message_t *sip | sip message.   */
/* returns null on error. */
int
osip_message_get_route (const osip_message_t * sip, int pos, osip_route_t ** dest)
{
  osip_route_t *route;

  *dest = NULL;
  if (osip_list_size (&sip->routes) <= pos)
    return OSIP_UNDEFINED_ERROR;        /* does not exist */
  route = (osip_route_t *) osip_list_get (&sip->routes, pos);
  *dest = route;
  return pos;
}
#endif

#ifndef MINISIZE
int
osip_route_parse (osip_route_t * route, const char *hvalue)
{
  return osip_from_parse ((osip_from_t *) route, hvalue);
}

/* returns the route header as a string.          */
/* INPUT : osip_route_t *route | route header.  */
/* returns null on error. */
int
osip_route_to_str (const osip_route_t * route, char **dest)
{
  /* we can't use osip_from_to_str(): route and record_route */
  /* always use brackets. */
  return osip_record_route_to_str ((osip_record_route_t *) route, dest);
}

/* deallocates a osip_route_t structure.  */
/* INPUT : osip_route_t *route | route header. */
void
osip_route_free (osip_route_t * route)
{
  osip_from_free ((osip_from_t *) route);
}
#endif
