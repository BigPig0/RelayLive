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


#ifndef _OSIP_CONTENT_TYPE_H_
#define _OSIP_CONTENT_TYPE_H_

#include <osipparser2/osip_list.h>

/**
 * @file osip_content_type.h
 * @brief oSIP osip_content_type header definition.
 */

/**
 * @defgroup oSIP_CONTENT_TYPE oSIP content-type header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for Content-Type headers.
 * @var osip_content_type_t
 */
  typedef struct osip_content_type osip_content_type_t;

/**
 * Definition of the Content-Type header.
 * @struct osip_content_type
 */
  struct osip_content_type
  {
    char *type;                 /**< Type of attachement */
    char *subtype;              /**< Sub-Type of attachement */
    osip_list_t gen_params;     /**< Content-Type parameters */
  };

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a Content-Type element.
 * @param header The element to work on.
 */
  int osip_content_type_init (osip_content_type_t ** header);
/**
 * Free a Content-Type element.
 * @param header The element to work on.
 */
  void osip_content_type_free (osip_content_type_t * header);
/**
 * Parse a Content-Type element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
  int osip_content_type_parse (osip_content_type_t * header, const char *hvalue);
/**
 * Get a string representation of a Content-Type element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
  int osip_content_type_to_str (const osip_content_type_t * header, char **dest);
/**
 * Clone a Content-Type element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
  int osip_content_type_clone (const osip_content_type_t * header,
  			  osip_content_type_t ** dest);

/**
 * Allocate and add a generic parameter element in a list.
 * @param header The element to work on.
 * @param name The token name.
 * @param value The token value.
 */
#define osip_content_type_param_add(header,name,value)  osip_generic_param_add((&(header)->gen_params),name,value)
/**
 * Find a header parameter in a Content-Type element.
 * @param header The element to work on.
 * @param name The token name to search.
 * @param dest A pointer on the element found.
 */
#define osip_content_type_param_get_byname(header,name,dest) osip_generic_param_get_byname((&(header)->gen_params),name,dest)


#ifdef __cplusplus
}
#endif

/** @} */

#endif
