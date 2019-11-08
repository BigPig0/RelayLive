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

#ifndef _URLS_H_
#define _URLS_H_

#include <osipparser2/osip_const.h>
#include <osipparser2/osip_list.h>

/**
 * @file osip_uri.h
 * @brief oSIP url parser Routines
 *
 * This is the implementation of sip url scheme. It also partially support
 * any unrecognised scheme (not starting with 'sip:' or 'sips:'). Unrecognised
 * scheme are stored in url->string.
 */

/**
 * @defgroup oSIP_URLS oSIP url parser Handling
 * @ingroup osip2_parser
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Structure for referencing url parameters.
 * @var osip_uri_param_t
 */
  typedef struct osip_uri_param osip_uri_param_t;

/**
 * Structure for referencing url parameters.
 * @struct osip_uri_param
 */
  struct osip_uri_param {
    char *gname;            /**< uri parameter name */
    char *gvalue;
                                        /**< uri parameter value */
  };

/**
 * Structure for referencing url headers.
 * @var osip_uri_header_t
 */
  typedef osip_uri_param_t osip_uri_header_t;

/**
 * Allocate a url parameter element.
 * @param url_param The element to work on.
 */
  int osip_uri_param_init (osip_uri_param_t ** url_param);
/**
 * Free a url parameter element.
 * @param url_param The element to work on.
 */
  void osip_uri_param_free (osip_uri_param_t * url_param);
/**
 * Set values of a url parameter element.
 * @param url_param The element to work on.
 * @param name The token name.
 * @param value The token value.
 */
  int osip_uri_param_set (osip_uri_param_t * url_param, char *name, char *value);
/**
 * Clone a url parameter element.
 * @param url_param The element to work on.
 * @param dest The resulting new allocated element.
 */
  int osip_uri_param_clone (const osip_uri_param_t * url_param, osip_uri_param_t ** dest);
#ifndef DOXYGEN
/*
 * Free a list of a url parameter element.
 * @param url_params The list of url parameter element to free.
 */
  void osip_uri_param_freelist (osip_list_t * url_params);
#endif
/**
 * Allocate and add a url parameter element in a list.
 * @param url_params The list of url parameter element to work on.
 * @param name The token name.
 * @param value The token value.
 */
  int osip_uri_param_add (osip_list_t * url_params, char *name, char *value);
/**
 * Find in a url parameter element in a list.
 * @param url_params The list of url parameter element to work on.
 * @param name The name of the parameter element to find.
 * @param dest A pointer on the element found.
 */
  int osip_uri_param_get_byname (osip_list_t * url_params, char *name, osip_uri_param_t ** dest);

/**
 * Allocate a generic parameter element.
 * @param url_header The element to work on.
 */
#define osip_uri_header_init(url_header) osip_uri_param_init(url_header)
/**
 * Free a generic parameter element.
 * @param url_header The element to work on.
 */
#define osip_uri_header_free(url_header) osip_uri_param_free(url_header)
/**
 * Set values of a generic parameter element.
 * @param url_header The element to work on.
 * @param name The token name.
 * @param value The token value.
 */
#define osip_uri_header_set(url_header, name, value)  osip_uri_param_set(url_header, name, value)
/**
 * Clone a generic parameter element.
 * @param url_header The element to work on.
 * @param dest The resulting new allocated element.
 */
#define osip_uri_header_clone(url_header,dest)    osip_uri_param_clone(url_header,dest)
#ifndef DOXYGEN
/*
 * Free a list of a generic parameter element.
 * @param LIST The list of generic parameter element to free.
 */
#define osip_uri_header_freelist(LIST)       osip_uri_param_freelist(LIST)
#endif
/**
 * Allocate and add a generic parameter element in a list.
 * @param url_headers The list of generic parameter element to work on.
 * @param name The token name.
 * @param value The token value.
 */
#define osip_uri_header_add(url_headers,name,value)        osip_uri_param_add(url_headers,name,value)
/**
 * Find in a generic parameter element in a list.
 * @param url_headers The list of generic parameter element to work on.
 * @param name The name of the parameter element to find.
 * @param dest A pointer on the element found.
 */
#define osip_uri_header_get_byname(url_headers,name,dest) osip_uri_param_get_byname(url_headers,name,dest)

/**
 * Structure for referencing SIP urls.
 * @var osip_uri_t
 */
  typedef struct osip_uri osip_uri_t;

/**
 * Structure for referencing SIP urls.
 * @struct osip_uri
 */
  struct osip_uri {
    char *scheme;                          /**< Uri Scheme (sip or sips) */
    char *username;                        /**< Username */
    char *password;                        /**< Password */
    char *host;                                    /**< Domain */
    char *port;                                    /**< Port number */
    osip_list_t url_params;            /**< Uri parameters */
    osip_list_t url_headers;
                                                           /**< Uri headers */

    char *string;
                                   /**< Space for other url schemes. (http, mailto...) */
  };

/**
 * Allocate a url element.
 * @param url The element to work on.
 */
  int osip_uri_init (osip_uri_t ** url);
/**
 * Free a url element.
 * @param url The element to work on.
 */
  void osip_uri_free (osip_uri_t * url);
/**
 * Parse a url.
 * @param url The element to work on.
 * @param buf The buffer to parse.
 */
  int osip_uri_parse (osip_uri_t * url, const char *buf);
#ifndef DOXYGEN
/**
 * Parse the header part of a url.
 * @param url The element to work on.
 * @param buf The buffer to parse.
 */
  int osip_uri_parse_headers (osip_uri_t * url, const char *buf);
/**
 * Parse the parameter part of a url.
 * @param url The element to work on.
 * @param buf The buffer to parse.
 */
  int osip_uri_parse_params (osip_uri_t * url, const char *buf);
#endif
/**
 * Get a string representation of a url element.
 * @param url The element to work on.
 * @param dest The resulting new allocated buffer.
 */
  int osip_uri_to_str (const osip_uri_t * url, char **dest);
/**
 * Clone a url element.
 * @param url The element to work on.
 * @param dest The resulting new allocated element.
 */
  int osip_uri_clone (const osip_uri_t * url, osip_uri_t ** dest);
/**
* Get a canonical string representation of a url element.
* as defined in 10.3-5
* @param url The element to work on.
* @param dest The resulting new allocated buffer.
*/
  int osip_uri_to_str_canonical (const osip_uri_t * url, char **dest);

/**
 * Set the scheme of a url element.
 * @param url The element to work on.
 * @param value The token value.
 */
  void osip_uri_set_scheme (osip_uri_t * url, char *value);
/**
 * Get the scheme of a url element.
 * @param url The element to work on.
 */
  char *osip_uri_get_scheme (osip_uri_t * url);
/**
 * Set the host of a url element.
 * @param url The element to work on.
 * @param value The token value.
 */
  void osip_uri_set_host (osip_uri_t * url, char *value);
/**
 * Get the host of a url element.
 * @param url The element to work on.
 */
  char *osip_uri_get_host (osip_uri_t * url);
/**
 * Set the username of a url element.
 * @param url The element to work on.
 * @param value The token value.
 */
  void osip_uri_set_username (osip_uri_t * url, char *value);
/**
 * Get the username of a url element.
 * @param url The element to work on.
 */
  char *osip_uri_get_username (osip_uri_t * url);
/**
 * Set the password of a url element.
 * @param url The element to work on.
 * @param value The token value.
 */
  void osip_uri_set_password (osip_uri_t * url, char *value);
/**
 * Get the password of a url element.
 * @param url The element to work on.
 */
  char *osip_uri_get_password (osip_uri_t * url);
/**
 * Set the port of a url element.
 * @param url The element to work on.
 * @param value The token value.
 */
  void osip_uri_set_port (osip_uri_t * url, char *value);
/**
 * Get the port of a url element.
 * @param url The element to work on.
 */
  char *osip_uri_get_port (osip_uri_t * url);



/**
 * Set the transport parameter to UDP in a url element.
 * @param url The element to work on.
 */
#define osip_uri_set_transport_udp(url)   osip_uri_param_add((&(url)->url_params), osip_strdup("transport"), osip_strdup("udp"))
/**
 * Set the transport parameter to TCP in a url element.
 * @param url The element to work on.
 */
#define osip_uri_set_transport_tcp(url)   osip_uri_param_add((&(url)->url_params), osip_strdup("transport"), osip_strdup("tcp"))
/**
 * Set the transport parameter to SCTP in a url element.
 * @param url The element to work on.
 */
#define osip_uri_set_transport_sctp(url)  osip_uri_param_add((&(url)->url_params), osip_strdup("transport"), osip_strdup("sctp"))
/**
 * Set the transport parameter to TLS in a url element.
 * @param url The element to work on.
 */
#define osip_uri_set_transport_tls(url)   osip_uri_param_add((&(url)->url_params), osip_strdup("transport"), osip_strdup("tls"))
/**
 * Set the transport parameter to TLS in a url element.
 * @param url The element to work on.
 * @param value The value describing the transport protocol.
 */
#define osip_uri_set_transport(url,value) osip_uri_param_add((&(url)->url_params), osip_strdup("transport"), value)

/**
 * Set the user parameter to PHONE in a url element.
 * @param url The element to work on.
 */
#define osip_uri_set_user_phone(url)   osip_uri_param_add((&(url)->url_params), osip_strdup("user"), osip_strdup("phone"))
/**
 * Set the user parameter to IP in a url element.
 * @param url The element to work on.
 */
#define osip_uri_set_user_ip(url)      osip_uri_param_add((&(url)->url_params), osip_strdup("user"), osip_strdup("ip"))
/**
 * Set a method parameter to INVITE in a url element.
 * @param url The element to work on.
 */
#define osip_uri_set_method_invite(url)  osip_uri_param_add((&(url)->url_params), osip_strdup("method"), osip_strdup("INVITE"))
/**
 * Set a method parameter to ACK in a url element.
 * @param url The element to work on.
 */
#define osip_uri_set_method_ack(url)     osip_uri_param_add((&(url)->url_params), osip_strdup("method"), osip_strdup("ACK"))
/**
 * Set a method parameter to OPTIONS in a url element.
 * @param url The element to work on.
 */
#define osip_uri_set_method_options(url) osip_uri_param_add((&(url)->url_params), osip_strdup("method"), osip_strdup("OPTIONS"))
/**
 * Set a method parameter to BYE in a url element.
 * @param url The element to work on.
 */
#define osip_uri_set_method_bye(url)     osip_uri_param_add((&(url)->url_params), osip_strdup("method"), osip_strdup("BYE"))
/**
 * Set a method parameter to CANCEL in a url element.
 * @param url The element to work on.
 */
#define osip_uri_set_method_cancel(url)  osip_uri_param_add((&(url)->url_params), osip_strdup("method"), osip_strdup("CANCEL"))
/**
 * Set a method parameter to REGISTER in a url element.
 * @param url The element to work on.
 */
#define osip_uri_set_method_register(url) osip_uri_param_add((&(url)->url_params),osip_strdup("method"), osip_strdup("REGISTER"))
/**
 * Set a method parameter in a url element.
 * @param url The element to work on.
 * @param value The value for the method parameter.
 */
#define osip_uri_set_method(url, value) osip_uri_param_add((&(url)->url_params), osip_strdup("method"), value)
/**
 * Set a ttl parameter in a url element.
 * @param url The element to work on.
 * @param value The value for the ttl parameter.
 */
#define osip_uri_set_ttl(url, value)    osip_uri_param_add((&(url)->url_params), osip_strdup("ttl"), value)
/**
 * Set a maddr parameter in a url element.
 * @param url The element to work on.
 * @param value The value for the maddr parameter.
 */
#define osip_uri_set_maddr(url, value)  osip_uri_param_add((&(url)->url_params), osip_strdup("maddr"), value)

/**
 * Allocate and add a url parameter element in a url element.
 * @param url The element to work on.
 * @param name The token name.
 * @param value The token value.
 */
#define osip_uri_uparam_add(url,name,value) osip_uri_param_add((&(url)->url_params),name,value)
/**
 * Find in a url parameter element in a url element.
 * @param url The element to work on.
 * @param name The name of the url parameter element to find.
 * @param dest A pointer on the element found.
 */
#define osip_uri_uparam_get_byname(url,name,dest)  osip_uri_param_get_byname((&(url)->url_params),name,dest)

/**
 * Allocate and add a url header element in a url element.
 * @param url The element to work on.
 * @param name The token name.
 * @param value The token value.
 */
#define osip_uri_uheader_add(url,name,value)    osip_uri_header_add((&(url)->url_headers),name,value)
/**
 * Find in a url header element in a url element.
 * @param url The element to work on.
 * @param name The name of the url header element to find.
 * @param dest A pointer on the element found.
 */
#define osip_uri_uheader_get_byname(url,name,dest) osip_uri_header_get_byname((&(url)->url_headers),name,dest)

#ifndef DOXYGEN
/* internal method */
  const char *next_separator (const char *ch, int separator_osip_to_find, int before_separator);

  char *__osip_uri_escape_nonascii_and_nondef (const char *string, const char *def);
  char *__osip_uri_escape_userinfo (const char *string);
  char *__osip_uri_escape_password (const char *string);
  char *__osip_uri_escape_uri_param (char *string);
  char *__osip_uri_escape_header_param (char *string);
  void __osip_uri_unescape (char *string);

#endif

/** @} */

#ifdef __cplusplus
}
#endif
#endif                          /*  _URLS_H_ */
