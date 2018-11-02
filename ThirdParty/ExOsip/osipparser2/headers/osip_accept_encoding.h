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


#ifndef _OSIP_ACCEPT_ENCONDING_H_
#define _OSIP_ACCEPT_ENCONDING_H_

#include <osipparser2/osip_list.h>

/**
 * @file osip_accept_encoding.h
 * @brief oSIP osip_accept_encoding header definition.
 */

/**
 * @defgroup oSIP_ACCEPT_ENCODING oSIP accept-encoding header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for Accept-Encoding header.
 * @var osip_accept_encoding_t
 */
  typedef struct osip_accept_encoding osip_accept_encoding_t;

/**
 * Definition of the Accept-Encoding header.
 * @struct osip_accept_encoding
 */
  struct osip_accept_encoding
  {
    char *element;           /**< accept encoding main value */
    osip_list_t gen_params;  /**< accept encoding parameters */
  };


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a Accept-Encoding element.
 * @param header The element to work on.
 */
  int osip_accept_encoding_init (osip_accept_encoding_t ** header);
/**
 * Parse a Accept-Encoding element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
  int osip_accept_encoding_parse (osip_accept_encoding_t * header, const char *hvalue);
/**
 * Get a string representation of a Accept-Encoding element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
  int osip_accept_encoding_to_str (const osip_accept_encoding_t * header, char **dest);
/**
 * Free a Accept-Encoding element.
 * @param header The element to work on.
 */
  void osip_accept_encoding_free (osip_accept_encoding_t * header);
/**
 * Clone a Accept-Encoding element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
  int osip_accept_encoding_clone (const osip_accept_encoding_t * header,
			     osip_accept_encoding_t ** dest);

/**
 * Set the value of an Accept-Encoding element.
 * @param header The element to work on.
 * @param value The token value to set.
 */
  void osip_accept_encoding_set_element (osip_accept_encoding_t * header,
				   char *value);
/**
 * Get the value of an Accept-Encoding element.
 * @param header The element to work on.
 */
  char *osip_accept_encoding_get_element (const osip_accept_encoding_t * header);
/**
 * Allocate and Add a header parameter in an Accept-Encoding element.
 * @param header The element to work on.
 * @param name The token name for the new parameter.
 * @param value The token value for the new parameter.
 */
#define osip_accept_encoding_param_add(header,name,value)  osip_generic_param_add((&(header)->gen_params),name,value)
/**
 * Find a header parameter in an Accept-Encoding element.
 * @param header The element to work on.
 * @param name The token name to search.
 * @param dest A pointer on the element found.
 */
#define osip_accept_encoding_param_get_byname(header,name,dest) osip_generic_param_get_byname((&(header)->gen_params),name,dest)


#ifdef __cplusplus
}
#endif

/** @} */

#endif
