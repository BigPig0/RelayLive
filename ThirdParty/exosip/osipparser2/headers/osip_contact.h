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


#ifndef _OSIP_CONTACT_H_
#define _OSIP_CONTACT_H_

#include <osipparser2/headers/osip_from.h>

/**
 * @file osip_contact.h
 * @brief oSIP osip_contact header definition.
 */

/**
 * @defgroup oSIP_CONTACT oSIP contact header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for Contact headers.
 * @var osip_contact_t
 */
  typedef osip_from_t osip_contact_t;

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MINISIZE
/**
 * Allocate a Contact element.
 * @param header The element to work on.
 */
  int osip_contact_init (osip_contact_t ** header);
/**
 * Free a Contact element.
 * @param header The element to work on.
 */
  void osip_contact_free (osip_contact_t * header);
#endif
/**
 * Parse a Contact element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
  int osip_contact_parse (osip_contact_t * header, const char *hvalue);
/**
 * Get a string representation of a Contact element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
  int osip_contact_to_str (const osip_contact_t * header, char **dest);
#ifndef MINISIZE
/**
 * Clone a Contact element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
  int osip_contact_clone (const osip_contact_t * header, osip_contact_t ** dest);
#else
  #define osip_contact_init  osip_from_init
  #define osip_contact_free  osip_from_free
  #define osip_contact_clone osip_from_clone
#endif
/**
 * Get the displayname from a Contact header.
 * @param header The element to work on.
 */
#define osip_contact_get_displayname(header) osip_from_get_displayname((osip_from_t*)header)
/**
 * Set the displayname in the Contact element.
 * @param header The element to work on.
 * @param value The value of the element.
 */
#define osip_contact_set_displayname(header,value) osip_from_set_displayname((osip_from_t*)header, value)
/**
 * Get the url from a Contact header.
 * @param header The element to work on.
 */
#define osip_contact_get_url(header)         osip_from_get_url((osip_from_t*)header)
/**
 * Set the url in the Contact element.
 * @param header The element to work on.
 * @param url The value of the element.
 */
#define osip_contact_set_url(header,url)       osip_from_set_url((osip_from_t*)header,url)
/**
 * Get a header parameter from a Contact element.
 * @param header The element to work on.
 * @param pos The index of the element to get.
 * @param dest A pointer on the element found.
 */
#define osip_contact_param_get(header,pos,dest) osip_from_param_get((osip_from_t*)header,pos,dest)
/**
 * Allocate and add a generic parameter element in a list.
 * @param header The element to work on.
 * @param name The token name.
 * @param value The token value.
 */
#define osip_contact_param_add(header,name, value) osip_generic_param_add((&(header)->gen_params), name,value)
/**
 * Find a header parameter in a Contact element.
 * @param header The element to work on.
 * @param name The token name to search.
 * @param dest A pointer on the element found.
 */
#define osip_contact_param_get_byname(header,name,dest) osip_generic_param_get_byname((&(header)->gen_params),name,dest)


#ifdef __cplusplus
}
#endif

/** @} */

#endif
