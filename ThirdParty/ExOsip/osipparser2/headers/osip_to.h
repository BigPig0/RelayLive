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


#ifndef _OSIP_TO_H_
#define _OSIP_TO_H_

#include <osipparser2/headers/osip_from.h>

/**
 * @file osip_to.h
 * @brief oSIP osip_to header definition.
 */

/**
 * @defgroup oSIP_TO oSIP to header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for To headers.
 * @var osip_to_t
 */
  typedef osip_from_t osip_to_t;

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MINISIZE
/**
 * Allocate a To element.
 * @param header The element to work on.
 */
  int osip_to_init (osip_to_t ** header);
/**
 * Free a To element.
 * @param header The element to work on.
 */
  void osip_to_free (osip_to_t * header);
/**
 * Parse a To element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
  int osip_to_parse (osip_to_t * header, const char *hvalue);
/**
 * Get a string representation of a To element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
  int osip_to_to_str (const osip_to_t * header, char **dest);
/**
 * Clone a To element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
  int osip_to_clone (const osip_to_t * header, osip_to_t ** dest);
/**
 * Check if the tags in the To headers match.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param to1 The first To header.
 * @param to2 The second To header.
 */
  int osip_to_tag_match (osip_to_t * to1, osip_to_t * to2);
#else
  #define osip_to_init   osip_from_init
  #define osip_to_free   osip_from_free
  #define osip_to_parse  osip_from_parse
  #define osip_to_to_str osip_from_to_str
  #define osip_to_clone  osip_from_clone
  #define osip_to_tag_match osip_from_tag_match
#endif
/**
 * Set the displayname in the To element.
 * @param header The element to work on.
 * @param value The value of the element.
 */
#define osip_to_set_displayname(header,value) osip_from_set_displayname((osip_from_t*)header,value)
/**
 * Get the displayname from a To header.
 * @param header The element to work on.
 */
#define osip_to_get_displayname(header)       osip_from_get_displayname((osip_from_t*)header)
/**
 * Set the url in the To element.
 * @param header The element to work on.
 * @param url The value of the element.
 */
#define osip_to_set_url(header,url)         osip_from_set_url((osip_from_t*)header,url)
/**
 * Get the url from a To header.
 * @param header The element to work on.
 */
#define osip_to_get_url(header)               osip_from_get_url((osip_from_t*)header)
/**
 * Get a header parameter from a To element.
 * @param header The element to work on.
 * @param pos The index of the element to get.
 * @param dest A pointer on the element found.
 */
#define osip_to_param_get(header,pos,dest) osip_from_param_get((osip_from_t*)header,pos,dest)
/**
 * Find a header parameter in a To element.
 * @param header The element to work on.
 * @param name The token name to search.
 * @param dest A pointer on the element found.
 */
#define osip_to_param_get_byname(header,name,dest) osip_generic_param_get_byname((&(header)->gen_params),name,dest)
/**
 * Allocate and add a generic parameter element in a list.
 * @param header The element to work on.
 * @param name The token name.
 * @param value The token value.
 */
#define osip_to_param_add(header,name,value) osip_generic_param_add((&(header)->gen_params),name,value)

/**
 * Allocate and add a tag parameter element in a list.
 * @param header The element to work on.
 * @param value The token value.
 */
#define osip_to_set_tag(header,value) osip_generic_param_add((&(header)->gen_params), osip_strdup("tag"),value)
/**
 * Find a tag parameter in a To element.
 * @param header The element to work on.
 * @param dest A pointer on the element found.
 */
#define osip_to_get_tag(header,dest) osip_generic_param_get_byname((&(header)->gen_params), "tag",dest)

#ifndef DOXYGEN			/* avoid DOXYGEN warning */
/* Compare the username, host and tag part of the two froms */
#define osip_to_compare(header1, header2) osip_from_compare((osip_from_t *)header1, (osip_from_t *)header2)
#endif

#ifdef __cplusplus
}
#endif

/** @} */

#endif
