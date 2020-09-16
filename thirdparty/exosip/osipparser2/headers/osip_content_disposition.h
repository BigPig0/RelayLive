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


#ifndef _OSIP_CONTENT_DISPOSITION_H_
#define _OSIP_CONTENT_DISPOSITION_H_

#include <osipparser2/headers/osip_call_info.h>

/**
 * @file osip_content_disposition.h
 * @brief oSIP osip_content_disposition header definition.
 */

/**
 * @defgroup oSIP_CONTENT_DISPOSITION oSIP content-disposition definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for Content-Disposition headers.
 * @var osip_content_disposition_t
 */
  typedef osip_call_info_t osip_content_disposition_t;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a Content-Disposition element.
 * @param header The element to work on.
 */
#define osip_content_disposition_init(header)      osip_call_info_init(header)
/**
 * Free a Content-Disposition element.
 * @param header The element to work on.
 */
#define osip_content_disposition_free(header)      osip_call_info_free(header)
/**
 * Parse a Content-Disposition element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
  int osip_content_disposition_parse (osip_content_disposition_t * header,
				 const char *hvalue);
/**
 * Get a string representation of a Content-Disposition element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
#define osip_content_disposition_to_str(header,dest)   osip_call_info_to_str(header,dest)
/**
 * Clone a Content-Disposition element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
#define osip_content_disposition_clone  osip_call_info_clone

/* type is of: "render" | "session" | "icon" | "alert" */
/**
 * Set the type in the Content-Disposition element.
 * @param header The element to work on.
 * @param value The value of the element.
 */
#define osip_content_disposition_set_type(header, value) osip_call_info_set_uri(header, value)
/**
 * Get the type from a Content-Disposition header.
 * @param header The element to work on.
 */
#define osip_content_disposition_get_type(header)    osip_call_info_get_uri(header)


#ifdef __cplusplus
}
#endif

/** @} */

#endif
