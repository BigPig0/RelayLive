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


#ifndef _OSIP_CONTENT_LENGTH_H_
#define _OSIP_CONTENT_LENGTH_H_


/**
 * @file osip_content_length.h
 * @brief oSIP osip_content_length header definition.
 */

/**
 * @defgroup oSIP_CONTENT_LENGTH oSIP content-length definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for Content-Length headers.
 * @var osip_content_length_t
 */
  typedef struct osip_content_length osip_content_length_t;

/**
 * Definition of the Content-Length header.
 * @struct osip_content_length
 */
  struct osip_content_length
  {
    char *value;    /**< value for Content-Length (size of attachments) */
  };

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a Content-Length element.
 * @param header The element to work on.
 */
  int osip_content_length_init (osip_content_length_t ** header);
/**
 * Free a Content-Length element.
 * @param header The element to work on.
 */
  void osip_content_length_free (osip_content_length_t * header);
/**
 * Parse a Content-Length element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
  int osip_content_length_parse (osip_content_length_t * header, const char *hvalue);
/**
 * Get a string representation of a Content-Length element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
  int osip_content_length_to_str (const osip_content_length_t * header, char **dest);
/**
 * Clone a Content-Length element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
  int osip_content_length_clone (const osip_content_length_t * header,
			    osip_content_length_t ** dest);


#ifdef __cplusplus
}
#endif

/** @} */

#endif
