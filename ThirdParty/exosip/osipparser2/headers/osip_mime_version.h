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


#ifndef _OSIP_MIME_VERSION_H_
#define _OSIP_MIME_VERSION_H_

#include <osipparser2/osip_list.h>
#include <osipparser2/osip_uri.h>

/**
 * @file osip_mime_version.h
 * @brief oSIP osip_mime_version header definition.
 */

/**
 * @defgroup oSIP_MIME_VERSION oSIP mime-version header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for Mime-Version headers.
 * @var osip_mime_version_t
 */
  typedef osip_content_length_t osip_mime_version_t;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a Mime-Version element.
 * @param header The element to work on.
 */
#define osip_mime_version_init(header)      osip_content_length_init(header)
/**
 * Parse a Mime-Version element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
#define osip_mime_version_parse(header, hvalue)  osip_content_length_parse(header, hvalue)
/**
 * Get a string representation of a Mime-Version element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
#define osip_mime_version_to_str(header, dest)  osip_content_length_to_str(header, dest)
/**
 * Free a Mime-Version element.
 * @param header The element to work on.
 */
#define osip_mime_version_free(header)      osip_content_length_free(header)
/**
 * Clone a Mime-Version element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
#define osip_mime_version_clone  osip_content_length_clone



#ifdef __cplusplus
}
#endif

/** @} */

#endif
