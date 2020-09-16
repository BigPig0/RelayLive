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


#ifndef _OSIP_AUTHORIZATION_H_
#define _OSIP_AUTHORIZATION_H_


/**
 * @file osip_authorization.h
 * @brief oSIP osip_authorization header definition.
 */

/**
 * @defgroup oSIP_AUTHORIZATION oSIP authorization header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for Authorization headers.
 * @var osip_authorization_t
 */
  typedef struct osip_authorization osip_authorization_t;

/**
 * Definition of the Authorization header.
 * @struct osip_authorization
 */
  struct osip_authorization
  {
    char *auth_type;		/**< Authentication Type (Basic or Digest) */
    char *username;		/**< login */
    char *realm;		/**< realm (as a quoted-string) */
    char *nonce;		/**< nonce */
    char *uri;  		/**< uri */
    char *response;		/**< response */
    char *digest;		/**< digest */
    char *algorithm;		/**< algorithm (optionnal) */
    char *cnonce;		/**< cnonce (optionnal) */
    char *opaque;		/**< opaque (optionnal) */
    char *message_qop;		/**< message_qop (optionnal) */
    char *nonce_count;		/**< nonce_count (optionnal) */
    char *version;		/**< version (optional - NTLM) */
    char *targetname;		/**< targetname (optional - NTLM) */
    char *gssapi_data;		/**< gssapi-data (optional - NTLM) */
    char *crand;
	  char *cnum;
    char *auth_param;		/**< other parameters (optionnal) */
  };


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a Authorization element.
 * @param header The element to work on.
 */
  int osip_authorization_init (osip_authorization_t ** header);
/**
 * Parse a Authorization element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
  int osip_authorization_parse (osip_authorization_t * header, const char *hvalue);
/**
 * Get a string representation of a Authorization element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
  int osip_authorization_to_str (const osip_authorization_t * header, char **dest);
/**
 * Free a Authorization element.
 * @param header The element to work on.
 */
  void osip_authorization_free (osip_authorization_t * header);
/**
 * Clone a Authorization element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
  int osip_authorization_clone (const osip_authorization_t * header,
  			   osip_authorization_t ** dest);

/**
 * Get value of the auth_type parameter from a Authorization element.
 * @param header The element to work on.
 */
  char *osip_authorization_get_auth_type (const osip_authorization_t * header);
/**
 * Add the auth_type parameter from a Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authorization_set_auth_type (osip_authorization_t * header, char *value);
/**
 * Get value of the username parameter from a Authorization element.
 * @param header The element to work on.
 */
  char *osip_authorization_get_username (osip_authorization_t * header);
/**
 * Add the username parameter from a Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authorization_set_username (osip_authorization_t * header, char *value);
/**
 * Get value of the realm parameter from a Authorization element.
 * @param header The element to work on.
 */
  char *osip_authorization_get_realm (osip_authorization_t * header);
/**
 * Add the realm parameter from a Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authorization_set_realm (osip_authorization_t * header, char *value);
/**
 * Get value of the nonce parameter from a Authorization element.
 * @param header The element to work on.
 */
  char *osip_authorization_get_nonce (osip_authorization_t * header);
/**
 * Add the nonce parameter from a Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authorization_set_nonce (osip_authorization_t * header, char *value);
/**
 * Get value of the uri parameter from a Authorization element.
 * @param header The element to work on.
 */
  char *osip_authorization_get_uri (osip_authorization_t * header);
/**
 * Add the uri parameter from a Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authorization_set_uri (osip_authorization_t * header, char *value);
/**
 * Get value of the response parameter from a Authorization element.
 * @param header The element to work on.
 */
  char *osip_authorization_get_response (osip_authorization_t * header);
/**
 * Add the response parameter from a Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authorization_set_response (osip_authorization_t * header, char *value);
/**
 * Get value of the digest parameter from a Authorization element.
 * @param header The element to work on.
 */
  char *osip_authorization_get_digest (osip_authorization_t * header);
/**
 * Add the digest parameter from a Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authorization_set_digest (osip_authorization_t * header, char *value);
/**
 * Get value of the algorithm parameter from a Authorization element.
 * @param header The element to work on.
 */
  char *osip_authorization_get_algorithm (osip_authorization_t * header);
/**
 * Add the algorithm parameter from a Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authorization_set_algorithm (osip_authorization_t * header, char *value);
/**
 * Get value of the cnonce parameter from a Authorization element.
 * @param header The element to work on.
 */
  char *osip_authorization_get_cnonce (osip_authorization_t * header);
/**
 * Add the cnonce parameter from a Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authorization_set_cnonce (osip_authorization_t * header, char *value);
/**
 * Get value of the opaque parameter from a Authorization element.
 * @param header The element to work on.
 */
  char *osip_authorization_get_opaque (osip_authorization_t * header);
/**
 * Add the opaque parameter from a Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authorization_set_opaque (osip_authorization_t * header, char *value);
/**
 * Get value of the message_qop parameter from a Authorization element.
 * @param header The element to work on.
 */
  char *osip_authorization_get_message_qop (osip_authorization_t * header);
/**
 * Add the message_qop parameter from a Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authorization_set_message_qop (osip_authorization_t * header, char *value);
/**
 * Get value of the nonce_count parameter from a Authorization element.
 * @param header The element to work on.
 */
  char *osip_authorization_get_nonce_count (osip_authorization_t * header);
/**
 * Add the nonce_count parameter from a Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authorization_set_nonce_count (osip_authorization_t * header, char *value);
/**
 * Get value of the version parameter from a Authorization element.
 * @param header The element to work on.
 */
  char *osip_authorization_get_version (osip_authorization_t * header);
/**
 * Add the version parameter from a Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authorization_set_version (osip_authorization_t * header,
					char *value);
/**
 * Get value of the targetname parameter from a Authorization element.
 * @param header The element to work on.
 */
  char *osip_authorization_get_targetname (osip_authorization_t * header);
/**
 * Add the targetname parameter from a Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authorization_set_targetname (osip_authorization_t * header,
					char *value);
/**
 * Get value of the gssapi_data parameter from a Authorization element.
 * @param header The element to work on.
 */
  char *osip_authorization_get_gssapi_data (osip_authorization_t * header);
/**
 * Add the gssapi_data parameter from a Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authorization_set_gssapi_data (osip_authorization_t * header,
					char *value);
/**
 * Get value of the crand parameter from a Authorization element.
 * @param header The element to work on.
 */
  char *osip_authorization_get_crand (osip_authorization_t * header);
/**
 * Add the crand parameter from a Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authorization_set_crand (osip_authorization_t * header,
				     char *value);
/**
 * Get value of the cnum parameter from a Authorization element.
 * @param header The element to work on.
 */
  char *osip_authorization_get_cnum (osip_authorization_t * header);
/**
 * Add the gssapi_data parameter from a Authorization element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authorization_set_cnum (osip_authorization_t * header,
				    char *value);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
