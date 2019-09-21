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


#ifndef _OSIP_ACCEPT_LANGUAGE_H_
#define _OSIP_ACCEPT_LANGUAGE_H_

#include <osipparser2/headers/osip_accept_encoding.h>

/**
 * @file osip_accept_language.h
 * @brief oSIP osip_accept_language header definition.
 */

/**
 * @defgroup oSIP_ACCEPT_LANGUAGE oSIP accept-language header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for Accept-Language headers.
 * @var osip_accept_language_t
 */
  typedef osip_accept_encoding_t osip_accept_language_t;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate an Accept-Language element.
 * @param header The element to work on.
 */
#define osip_accept_language_init(header)      osip_accept_encoding_init(header)
/**
 * Parse an Accept-Language element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
#define osip_accept_language_parse(header, hvalue)  osip_accept_encoding_parse(header, hvalue)
/**
 * Get a string representation of an Accept-Language element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
#define osip_accept_language_to_str  osip_accept_encoding_to_str
/**
 * Free an Accept-Language element.
 * @param header The element to work on.
 */
#define osip_accept_language_free      osip_accept_encoding_free
/**
 * Clone an Accept-Language element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
#define osip_accept_language_clone osip_accept_encoding_clone

/**
 * Get the value of an Accept-Language element.
 * @param header The element to work on.
 */
#define osip_accept_language_get_element(header)     osip_accept_encoding_get_element(header)
/**
 * Set the value of an Accept-Language element.
 * @param header The element to work on.
 * @param value The value to set.
 */
#define osip_accept_language_set_element(header, value)  osip_accept_encoding_set_element(header, value)
/**
 * Allocate and add a generic parameter element in an Accept-Language element.
 * @param header The element to work on.
 * @param name The token name.
 * @param value The token value.
 */
#define osip_accept_language_param_add(header,name,value)  osip_generic_param_add((&(header)->gen_params),name,value)
/**
 * Find a header parameter in a Accept-Language element.
 * @param header The element to work on.
 * @param name The token name to search.
 * @param dest A pointer on the element found.
 */
#define osip_accept_language_param_get_byname(header,name,dest) osip_generic_param_get_byname((&(header)->gen_params),name,dest)


#ifdef __cplusplus
}
#endif

/** @} */

#endif
