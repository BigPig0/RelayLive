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


#ifndef _OSIP_PROXY_AUHTHENTICATE_H_
#define _OSIP_PROXY_AUHTHENTICATE_H_

#include <osipparser2/headers/osip_www_authenticate.h>

/**
 * @file osip_proxy_authenticate.h
 * @brief oSIP osip_proxy_authenticate header definition.
 */

/**
 * @defgroup oSIP_PROXY_AUTHENTICATE oSIP proxy-authenticate header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for Proxy-Authenticate headers.
 * @var osip_proxy_authenticate_t
 */
  typedef osip_www_authenticate_t osip_proxy_authenticate_t;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a Proxy-Authenticate element.
 * @param header The element to work on.
 */
#define osip_proxy_authenticate_init(header)     osip_www_authenticate_init(header)
/**
 * Parse a Proxy-Authenticate element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
#define osip_proxy_authenticate_parse(header, hvalue) osip_www_authenticate_parse(header, hvalue)
/**
 * Get a string representation of a Proxy-Authenticate element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
#define osip_proxy_authenticate_to_str(header, dest) osip_www_authenticate_to_str(header, dest)
/**
 * Free a Proxy-Authenticate element.
 * @param header The element to work on.
 */
#define osip_proxy_authenticate_free     osip_www_authenticate_free
/**
 * Clone a Proxy-Authenticate element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
#define osip_proxy_authenticate_clone  osip_www_authenticate_clone

/**
 * Get value of the auth_type parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 */
#define osip_proxy_authenticate_get_auth_type(header)   osip_www_authenticate_get_auth_type(header)
/**
 * Add the auth_type parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authenticate_set_auth_type(header,value) osip_www_authenticate_set_auth_type(header, value)
/**
 * Get value of the realm parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 */
#define osip_proxy_authenticate_get_realm(header)     osip_www_authenticate_get_realm(header)
/**
 * Add the realm parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authenticate_set_realm(header, value)  osip_www_authenticate_set_realm(header, value)
/**
 * Get value of the domain parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 */
#define osip_proxy_authenticate_get_domain(header)    osip_www_authenticate_get_domain(header)
/**
 * Add the domain parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authenticate_set_domain(header, value) osip_www_authenticate_set_domain(header, value)
/**
 * Get value of the nonce parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 */
#define osip_proxy_authenticate_get_nonce(header)     osip_www_authenticate_get_nonce(header)
/**
 * Add the nonce parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authenticate_set_nonce(header, value)  osip_www_authenticate_set_nonce(header, value)
/**
 * Get value of the opaque parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 */
#define osip_proxy_authenticate_get_opaque(header)    osip_www_authenticate_get_opaque(header)
/**
 * Add the opaque parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authenticate_set_opaque(header, value) osip_www_authenticate_set_opaque(header, value)
/**
 * Get value of the stale parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 */
#define osip_proxy_authenticate_get_stale(header)     osip_www_authenticate_get_stale(header)
/**
 * Add the stale parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authenticate_set_stale(header, value)  osip_www_authenticate_set_stale(header, value)
/**
 * Add a stale parameter set to "true" in a proxy-Authenticate element.
 * @param header The element to work on.
 */
#define osip_proxy_authenticate_set_stale_true(header) osip_www_authenticate_set_stale(header,osip_strdup("true"))
/**
 * Add a stale parameter set to "false" in a Proxy-Authenticate element.
 * @param header The element to work on.
 */
#define osip_proxy_authenticate_set_stale_false(header) osip_www_authenticate_set_stale(header,osip_strdup("false"))
/**
 * Get value of the algorithm parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 */
#define osip_proxy_authenticate_get_algorithm(header) osip_www_authenticate_get_algorithm(header)
/**
 * Add the algorithm parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authenticate_set_algorithm(header, value) osip_www_authenticate_set_algorithm(header, value)
/**
 * Add the algorithm parameter set to "MD5" in a Proxy-Authenticate element.
 * @param header The element to work on.
 */
#define osip_proxy_authenticate_set_algorithm_MD5(header) osip_www_authenticate_set_algorithm(header,osip_strdup("MD5"))
/**
 * Get value of the qop_options parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 */
#define osip_proxy_authenticate_get_qop_options(header) osip_www_authenticate_get_qop_options(header)
/**
 * Add the qop_options parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authenticate_set_qop_options(header,value) osip_www_authenticate_set_qop_options(header,value)
/**
 * Get value of the version parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 */
#define osip_proxy_authenticate_get_version(header) osip_www_authenticate_get_version(header)
/**
 * Add the version parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authenticate_set_version(header,value) osip_www_authenticate_set_version(header,value)
/**
 * Get value of the targetname parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 */
#define osip_proxy_authenticate_get_targetname(header) osip_www_authenticate_get_targetname(header)
/**
 * Add the targetname parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authenticate_set_targetname(header,value) osip_www_authenticate_set_targetname(header,value)
/**
 * Get value of the gssapi_data parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 */
#define osip_proxy_authenticate_get_gssapi_data(header) osip_www_authenticate_get_gssapi_data(header)
/**
 * Add the gssapi_data parameter from a Proxy-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
#define osip_proxy_authenticate_set_gssapi_data(header,value) osip_www_authenticate_set_gssapi_data(header,value)


#ifdef __cplusplus
}
#endif

/** @} */

#endif
