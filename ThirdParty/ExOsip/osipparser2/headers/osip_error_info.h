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


#ifndef _OSIP_ERROR_INFO_H_
#define _OSIP_ERROR_INFO_H_

#include <osipparser2/headers/osip_call_info.h>

/**
 * @file osip_error_info.h
 * @brief oSIP osip_error_info header definition.
 */

/**
 * @defgroup oSIP_ERROR_INFO oSIP error-info definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for Error-Info headers.
 * @var osip_error_info_t
 */
  typedef osip_call_info_t osip_error_info_t;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a Error-Info element.
 * @param header The element to work on.
 */
#define  osip_error_info_init(header)      osip_call_info_init(header)
/**
 * Free a Error-Info element.
 * @param header The element to work on.
 */
#define  osip_error_info_free      osip_call_info_free
/**
 * Parse a Error-Info element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
#define  osip_error_info_parse(header, hvalue)  osip_call_info_parse(header, hvalue)
/**
 * Get a string representation of a Error-Info element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
#define  osip_error_info_to_str(header,dest)   osip_call_info_to_str(header,dest)
/**
 * Clone a Error-Info element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
#define  osip_error_info_clone  osip_call_info_clone

/**
 * Set the uri in the Error-Info element.
 * @param header The element to work on.
 * @param uri The uri of the element.
 */
#define  osip_error_info_set_uri(header, uri) osip_call_info_set_uri(header, uri)
/**
 * Get the uri from a Error-Info header.
 * @param header The element to work on.
 */
#define  osip_error_info_get_uri(header)    osip_call_info_get_uri(header)



#ifdef __cplusplus
}
#endif

/** @} */

#endif
