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


#ifndef _OSIP_PROXY_AUHTHORIZATION_H_
#define _OSIP_PROXY_AUHTHORIZATION_H_

#include <osipparser2/headers/osip_authorization.h>

/**
 * @file osip_proxy_authorization.h
 * @brief oSIP osip_proxy_authorization header definition.
 */

/**
 * @defgroup oSIP_PROXY_AUTHORIZATION oSIP proxy-authorization header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for Proxy-Authorization headers.
 * @var osip_proxy_authorization_t
 */
  typedef osip_authorization_t osip_proxy_authorization_t;


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a Proxy-Authorization element.
 * @param header The element to work on.
 */
#define osip_proxy_authorization_init(header)     osip_authorization_init(header)
/**
 * Parse a Proxy-Authorization element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
#define osip_proxy_authorization_parse(header, hvalue) osip_authorization_parse(header, hvalue)
/**
 * Get a string representation of a Proxy-Authorization element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
#define osip_proxy_authorization_to_str(header, dest) osip_authorization_to_str(header, dest)
/**
 * Free a Proxy-Authorization element.
 * @param header The element to work on.
 */
#define osip_proxy_authorization_free     osip_authorization_free
/**
 * Clone a Proxy-Authorization element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
#define osip_proxy_authorization_clone  osip_authorization_clone

/**
 * Get value of the auth_type parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 */
#define osip_proxy_authorization_get_auth_type(header)    osip_authorization_get_auth_type(header)
/**
 * Add the auth_type parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authorization_set_auth_type(header, value) osip_authorization_set_auth_type(header, value)
/**
 * Get value of the username parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 */
#define osip_proxy_authorization_get_username(header)    osip_authorization_get_username(header)
/**
 * Add the username parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authorization_set_username(header, value) osip_authorization_set_username(header, value)
/**
 * Get value of the realm parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 */
#define osip_proxy_authorization_get_realm(header)       osip_authorization_get_realm(header)
/**
 * Add the realm parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authorization_set_realm(header, value)    osip_authorization_set_realm(header, value)
/**
 * Get value of the nonce parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 */
#define osip_proxy_authorization_get_nonce(header)       osip_authorization_get_nonce(header)
/**
 * Add the nonce parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authorization_set_nonce(header, value)    osip_authorization_set_nonce(header, value)
/**
 * Get value of the uri parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 */
#define osip_proxy_authorization_get_uri(header)         osip_authorization_get_uri(header)
/**
 * Add the uri parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authorization_set_uri(header, value)      osip_authorization_set_uri(header, value)
/**
 * Get value of the response parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 */
#define osip_proxy_authorization_get_response(header)    osip_authorization_get_response(header)
/**
 * Add the response parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authorization_set_response(header, value) osip_authorization_set_response(header, value)
/**
 * Get value of the digest parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 */
#define osip_proxy_authorization_get_digest(header)      osip_authorization_get_digest(header)
/**
 * Add the digest parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authorization_set_digest(header, value)   osip_authorization_set_digest(header, value)
/**
 * Get value of the algorithm parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 */
#define osip_proxy_authorization_get_algorithm(header)   osip_authorization_get_algorithm(header)
/**
 * Add the algorithm parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authorization_set_algorithm(header,value) osip_authorization_set_algorithm(header,value)
/**
 * Get value of the cnonce parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 */
#define osip_proxy_authorization_get_cnonce(header)      osip_authorization_get_cnonce(header)
/**
 * Add the cnonce parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authorization_set_cnonce(header, value)   osip_authorization_set_cnonce(header, value)
/**
 * Get value of the opaque parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 */
#define osip_proxy_authorization_get_opaque(header)      osip_authorization_get_opaque(header)
/**
 * Add the opaque parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authorization_set_opaque(header, value)   osip_authorization_set_opaque(header, value)
/**
 * Get value of the message_qop parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 */
#define osip_proxy_authorization_get_message_qop(header) osip_authorization_get_message_qop(header)
/**
 * Add the message_qop parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authorization_set_message_qop(header, value) osip_authorization_set_message_qop(header, value)
/**
 * Get value of the nonce_count parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 */
#define osip_proxy_authorization_get_nonce_count(header) osip_authorization_get_nonce_count(header)
/**
 * Add the nonce_count parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authorization_set_nonce_count(header, value) osip_authorization_set_nonce_count(header, value)
/**
 * Get value of the version parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 */
#define osip_proxy_authorization_get_version(header) osip_authorization_get_version(header)
/**
 * Add the version parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authorization_set_version(header, value) osip_authorization_set_version(header, value)
/**
 * Get value of the targetname parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 */
#define osip_proxy_authorization_get_targetname(header) osip_authorization_get_targetname(header)
/**
 * Add the targetname parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authorization_set_targetname(header, value) osip_authorization_set_targetname(header, value)
/**
 * Get value of the gssapi_data parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 */
#define osip_proxy_authorization_get_gssapi_data(header) osip_authorization_get_gssapi_data(header)
/**
 * Add the gssapi_data parameter from a Proxy-Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authorization_set_gssapi_data(header, value) osip_authorization_set_gssapi_data(header, value)

#ifdef __cplusplus
}
#endif

/** @} */

#endif

