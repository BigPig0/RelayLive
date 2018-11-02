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


#ifndef _OSIP_BODY_H_
#define _OSIP_BODY_H_

#include <osipparser2/headers/osip_content_type.h>

/**
 * @file osip_body.h
 * @brief oSIP SIP Message Body Routines
 *
 */

/**
 * @defgroup oSIP_BODY oSIP body API
 * @ingroup osip2_parser
 * @{
 */

/**
 * Structure for holding Body
 * @var osip_body_t
 */
typedef struct osip_body osip_body_t;

/**
 * Structure for holding Body
 * @struct osip_body
 */
struct osip_body {
  char *body;                                                    /**< buffer containing data */
  size_t length;                                         /**< length of data */
  osip_list_t *headers;                          /**< List of headers (when mime is used) */
  osip_content_type_t *content_type;
                                                                         /**< Content-Type (when mime is used) */
};


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocate a osip_body_t element.
 * @param body The element to work on.
 */
  int osip_body_init (osip_body_t ** body);
/**
 * Free a osip_body_t element.
 * @param body The element to work on.
 */
  void osip_body_free (osip_body_t * body);
/**
 * Parse a osip_body_t element.
 * @param body The element to work on.
 * @param buf The buffer to parse.
 * @param length The length of the buffer to parse.
 */
  int osip_body_parse (osip_body_t * body, const char *buf, size_t length);
/**
 * Clone a osip_body_t element.
 * @param body The element to clone.
 * @param dest The cloned element.
 */
  int osip_body_clone (const osip_body_t * body, osip_body_t ** dest);
/**
 * Parse a osip_body_t element. (for mime message format) (NOT TESTED, use with care)
 * @param body The element to work on.
 * @param buf The buffer to parse.
 * @param length The length of the buffer to parse.
 */
  int osip_body_parse_mime (osip_body_t * body, const char *buf, size_t length);
/**
 * Get a string representation of a osip_body_t element.
 * @param body The element to work on.
 * @param dest The resulting buffer.
 * @param length The length of the returned buffer.
 */
  int osip_body_to_str (const osip_body_t * body, char **dest, size_t * length);

/**
 * Set the Content-Type header in the osip_body_t element.
 * @param body The element to work on.
 * @param hvalue The content type string value.
 */
  int osip_body_set_contenttype (osip_body_t * body, const char *hvalue);

/**
 * Add a header in the osip_body_t element.
 * @param body The element to work on.
 * @param hname The header string name.
 * @param hvalue The header string value.
 */
  int osip_body_set_header (osip_body_t * body, const char *hname, const char *hvalue);

#ifdef __cplusplus
}
#endif
/** @} */
#endif
