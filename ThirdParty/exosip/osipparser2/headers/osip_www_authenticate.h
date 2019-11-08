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


#ifndef _OSIP_WWW_AUTHENTICATE_H_
#define _OSIP_WWW_AUTHENTICATE_H_


/**
 * @file osip_www_authenticate.h
 * @brief oSIP osip_www_authenticate header definition.
 */

/**
 * @defgroup oSIP_WWW_AUTHENTICATE oSIP www-authenticate header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for WWW-Authenticate headers.
 * @var osip_www_authenticate_t
 */
  typedef struct osip_www_authenticate osip_www_authenticate_t;

/**
 * Definition of the WWW-Authenticate header.
 * @struct osip_www_authenticate
 */
  struct osip_www_authenticate
  {
    char *auth_type;		/**< Authentication Type (Basic or Digest */
    char *realm;		/**< realm (as a quoted-string) */
    char *domain;		/**< domain (optional) */
    char *nonce;		/**< nonce (optional)*/
    char *opaque;		/**< opaque (optional) */
    char *stale;		/**< stale (optional) */
    char *algorithm;		/**< algorythm (optional) */
    char *qop_options;		/**< qop option (optional)  */
    char *version;		/**< version (optional - NTLM) */
    char *targetname;		/**< targetname (optional - NTLM) */
    char *gssapi_data;		/**< gssapi-data (optional - NTLM) */
    char *auth_param;		/**< other parameters (optional) */
  };


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a Www-Authenticate element.
 * @param header The element to work on.
 */
  int osip_www_authenticate_init (osip_www_authenticate_t ** header);
/**
 * Parse a Www-Authenticate element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
  int osip_www_authenticate_parse (osip_www_authenticate_t * header, const char *hvalue);
/**
 * Get a string representation of a Www-Authenticate element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
  int osip_www_authenticate_to_str (const osip_www_authenticate_t * header, char **dest);
/**
 * Free a Www-Authenticate element.
 * @param header The element to work on.
 */
  void osip_www_authenticate_free (osip_www_authenticate_t * header);
/**
 * Clone a Www-Authenticate element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
  int osip_www_authenticate_clone (const osip_www_authenticate_t * header,
			      osip_www_authenticate_t ** dest);

/**
 * Get value of the auth_type parameter from a Www-Authenticate element.
 * @param header The element to work on.
 */
  char *osip_www_authenticate_get_auth_type (osip_www_authenticate_t * header);
/**
 * Add the auth_type parameter from a Www-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_www_authenticate_set_auth_type (osip_www_authenticate_t * header,
				      char *value);
/**
 * Get value of the realm parameter from a Www-Authenticate element.
 * @param header The element to work on.
 */
  char *osip_www_authenticate_get_realm (osip_www_authenticate_t * header);
/**
 * Add the realm parameter from a Www-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_www_authenticate_set_realm (osip_www_authenticate_t * header, char *value);
/**
 * Get value of the domain parameter from a Www-Authenticate element.
 * @param header The element to work on.
 */
  char *osip_www_authenticate_get_domain (osip_www_authenticate_t * header);
/**
 * Add the domain parameter from a Www-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_www_authenticate_set_domain (osip_www_authenticate_t * header, char *value);
/**
 * Get value of the nonce parameter from a Www-Authenticate element.
 * @param header The element to work on.
 */
  char *osip_www_authenticate_get_nonce (osip_www_authenticate_t * header);
/**
 * Add the nonce parameter from a Www-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_www_authenticate_set_nonce (osip_www_authenticate_t * header, char *value);
/**
 * Get value of the opaque parameter from a Www-Authenticate element.
 * @param header The element to work on.
 */
  char *osip_www_authenticate_get_opaque (osip_www_authenticate_t * header);
/**
 * Add the opaque parameter from a Www-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_www_authenticate_set_opaque (osip_www_authenticate_t * header, char *value);
/**
 * Get value of the stale parameter from a Www-Authenticate element.
 * @param header The element to work on.
 */
  char *osip_www_authenticate_get_stale (osip_www_authenticate_t * header);
/**
 * Add the stale parameter in a Www-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_www_authenticate_set_stale (osip_www_authenticate_t * header, char *value);
/**
 * Add a stale parameter set to "true" in a Www-Authenticate element.
 * @param header The element to work on.
 */
#define osip_www_authenticate_set_stale_true(header) osip_www_authenticate_set_stale(header,osip_strdup("true"))
/**
 * Add a stale parameter set to "false" in a Www-Authenticate element.
 * @param header The element to work on.
 */
#define osip_www_authenticate_set_stale_false(header) osip_www_authenticate_set_stale(header,osip_strdup("false"))
/**
 * Get value of the algorithm parameter from a Www-Authenticate element.
 * @param header The element to work on.
 */
  char *osip_www_authenticate_get_algorithm (osip_www_authenticate_t * header);
/**
 * Add the algorithm parameter in a Www-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_www_authenticate_set_algorithm (osip_www_authenticate_t * header,
				      char *value);
/**
 * Add the algorithm parameter set to "MD5" in a Www-Authenticate element.
 * @param header The element to work on.
 */
#define osip_www_authenticate_set_algorithm_MD5(header) osip_www_authenticate_set_algorithm(header,osip_strdup("MD5"))
/**
 * Get value of the qop_options parameter from a Www-Authenticate element.
 * @param header The element to work on.
 */
  char *osip_www_authenticate_get_qop_options (osip_www_authenticate_t * header);
/**
 * Add the qop_options parameter from a Www-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_www_authenticate_set_qop_options (osip_www_authenticate_t * header,
					char *value);
/**
 * Get value of the version parameter from a Www-Authenticate element.
 * @param header The element to work on.
 */
  char *osip_www_authenticate_get_version (osip_www_authenticate_t * header);
/**
 * Add the version parameter from a Www-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_www_authenticate_set_version (osip_www_authenticate_t * header,
					char *value);
/**
 * Get value of the targetname parameter from a Www-Authenticate element.
 * @param header The element to work on.
 */
  char *osip_www_authenticate_get_targetname (osip_www_authenticate_t * header);
/**
 * Add the targetname parameter from a Www-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_www_authenticate_set_targetname (osip_www_authenticate_t * header,
					char *value);
/**
 * Get value of the gssapi_data parameter from a Www-Authenticate element.
 * @param header The element to work on.
 */
  char *osip_www_authenticate_get_gssapi_data (osip_www_authenticate_t * header);
/**
 * Add the gssapi_data parameter from a Www-Authenticate element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_www_authenticate_set_gssapi_data (osip_www_authenticate_t * header,
					char *value);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
