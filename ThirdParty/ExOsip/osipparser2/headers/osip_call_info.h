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


#ifndef _OSIP_CALL_INFO_H_
#define _OSIP_CALL_INFO_H_

#include <osipparser2/osip_list.h>

/**
 * @file osip_call_info.h
 * @brief oSIP osip_call_info header definition.
 */

/**
 * @defgroup oSIP_CALL_INFO oSIP call-info header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for Call-Info headers.
 * @var osip_call_info_t
 */
  typedef struct osip_call_info osip_call_info_t;

/**
 * Definition of the Call-Info header.
 * @struct osip_call_info
 */
  struct osip_call_info
  {
    char *element;              /**< Call-Info main value */
    osip_list_t gen_params;     /**< Parameters for Call-Info header */
  };

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a Call-Info element.
 * @param header The element to work on.
 */
  int osip_call_info_init (osip_call_info_t ** header);
/**
 * Free a Call-Info element.
 * @param header The element to work on.
 */
  void osip_call_info_free (osip_call_info_t * header);
/**
 * Parse a Call-Info element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
  int osip_call_info_parse (osip_call_info_t * header, const char *hvalue);
/**
 * Get a string representation of a Call-Info element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
  int osip_call_info_to_str (const osip_call_info_t * header, char **dest);
/**
 * Clone a Call-Info element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
  int osip_call_info_clone (const osip_call_info_t * header, osip_call_info_t ** dest);
/**
 * Get the uri from a Call_Info header.
 * @param header The element to work on.
 */
  char *osip_call_info_get_uri (osip_call_info_t * header);
/**
 * Set the uri in the Call_Info element.
 * @param header The element to work on.
 * @param uri The value of the element.
 */
  void osip_call_info_set_uri (osip_call_info_t * header, char *uri);


#ifdef __cplusplus
}
#endif

/** @} */

#endif
