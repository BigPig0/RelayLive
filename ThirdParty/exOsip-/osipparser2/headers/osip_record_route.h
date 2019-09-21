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


#ifndef _OSIP_RECORD_ROUTE_H_
#define _OSIP_RECORD_ROUTE_H_

#include <osipparser2/headers/osip_from.h>

/**
 * @file osip_record_route.h
 * @brief oSIP osip_record_route header definition.
 */

/**
 * @defgroup oSIP_RECORD_ROUTE oSIP record-route header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for Record-Route headers.
 * @var osip_record_route_t
 */
  typedef osip_from_t osip_record_route_t;

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MINISIZE
/**
 * Allocate a Record-Route element.
 * @param header The element to work on.
 */
  int osip_record_route_init (osip_record_route_t ** header);
/**
 * Free a Record-Route element.
 * @param header The element to work on.
 */
  void osip_record_route_free (osip_record_route_t * header);
/**
 * Parse a Record-Route element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
  int osip_record_route_parse (osip_record_route_t * header, const char *hvalue);
/**
 * Get a string representation of a Record-Route element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
  int osip_record_route_to_str (const osip_record_route_t * header, char **dest);
/**
 * Clone a Record-Route element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
#define osip_record_route_clone      osip_from_clone
#else
  #define osip_record_route_init   osip_from_init
  #define osip_record_route_free   osip_from_free
  #define osip_record_route_parse  osip_from_parse
  #define osip_record_route_to_str osip_from_to_str
  #define osip_record_route_clone  osip_from_clone
#endif
/**
 * Set the url in the Record-Route element.
 * @param header The element to work on.
 * @param url The value of the element.
 */
#define osip_record_route_set_url(header,url)      osip_from_set_url((osip_from_t*)header,url)
/**
 * Get the url from a Record-Route header.
 * @param header The element to work on.
 */
#define osip_record_route_get_url(header)          osip_from_get_url((osip_from_t*)header)
/**
 * Get a header parameter from a Record-Route element.
 * @param header The element to work on.
 * @param pos The index of the element to get.
 * @param dest A pointer on the element found.
 */
#define osip_record_route_param_get(header,pos,dest)  osip_from_param_get((osip_from_t*)header,pos,dest)
/**
 * Allocate and add a generic parameter element in a list.
 * @param header The element to work on.
 * @param name The token name.
 * @param value The token value.
 */
#define osip_record_route_param_add(header,name,value) osip_generic_param_add((&(header)->gen_params),name,value)
/**
 * Find a header parameter in a Record-Route element.
 * @param header The element to work on.
 * @param name The token name to search.
 * @param dest A pointer on the element found.
 */
#define osip_record_route_param_get_byname(header,name,dest) osip_generic_param_get_byname((&(header)->gen_params),name,dest)

#ifdef __cplusplus
}
#endif

/** @} */

#endif
