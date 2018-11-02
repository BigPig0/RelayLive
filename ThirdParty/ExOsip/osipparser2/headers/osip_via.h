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


#ifndef _OSIP_VIA_H_
#define _OSIP_VIA_H_

#include <osipparser2/osip_list.h>

/**
 * @file osip_via.h
 * @brief oSIP osip_via header definition.
 */

/**
 * @defgroup oSIP_VIA oSIP via header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for Via headers.
 * @var osip_via_t
 */
  typedef struct osip_via osip_via_t;

/**
 * Definition of the Via header.
 * @struct osip_via
 */
  struct osip_via
  {
    char *version;              /**< SIP Version */
    char *protocol;             /**< Protocol used by SIP Agent */
    char *host;                 /**< Host where to send answers */
    char *port;                 /**< Port where to send answers */
    char *comment;              /**< Comments about SIP Agent */
    osip_list_t via_params;     /**< Via parameters */
  };

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a Via element.
 * @param header The element to work on.
 */
  int osip_via_init (osip_via_t ** header);
/**
 * Free a Via element.
 * @param header The element to work on.
 */
  void osip_via_free (osip_via_t * header);
/**
 * Parse a Via element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
  int osip_via_parse (osip_via_t * header, const char *hvalue);
/**
 * Get a string representation of a Via element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
  int osip_via_to_str (const osip_via_t * header, char **dest);
/**
 * Clone a Via element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
  int osip_via_clone (const osip_via_t * header, osip_via_t ** dest);
/**
 * Set the SIP version in the Via element.
 * @param header The element to work on.
 * @param value The value of the element.
 */
  void via_set_version (osip_via_t * header, char *value);
#define osip_via_set_version via_set_version
/**
 * Get the SIP version from a Via header.
 * @param header The element to work on.
 */
  char *via_get_version (osip_via_t * header);
#define osip_via_get_version via_get_version
/**
 * Set the protocol in the Via element.
 * @param header The element to work on.
 * @param value The value of the element.
 */
  void via_set_protocol (osip_via_t * header, char *value);
#define osip_via_set_protocol via_set_protocol
/**
 * Get the protocol from a Via header.
 * @param header The element to work on.
 */
  char *via_get_protocol (osip_via_t * header);
#define osip_via_get_protocol via_get_protocol
/**
 * Set the host in the Via element.
 * @param header The element to work on.
 * @param value The value of the element.
 */
  void via_set_host (osip_via_t * header, char *value);
#define osip_via_set_host via_set_host
/**
 * Get the host from a Via header.
 * @param header The element to work on.
 */
  char *via_get_host (osip_via_t * header);
#define osip_via_get_host via_get_host
/**
 * Set the port in the Via element.
 * @param header The element to work on.
 * @param value The value of the element.
 */
  void via_set_port (osip_via_t * header, char *value);
#define osip_via_set_port via_set_port
/**
 * Get the port from a Via header.
 * @param header The element to work on.
 */
  char *via_get_port (osip_via_t * header);
#define osip_via_get_port via_get_port
/**
 * Set the comment in the Via element.
 * @param header The element to work on.
 * @param value The value of the element.
 */
  void via_set_comment (osip_via_t * header, char *value);
#define osip_via_set_comment via_set_comment
/**
 * Get the comment from a Via header.
 * @param header The element to work on.
 */
  char *via_get_comment (osip_via_t * header);
#define osip_via_get_comment via_get_comment

/**
 * Allocate and add a hidden parameter element in a list.
 * @param header The element to work on.
 */
#define osip_via_set_hidden(header)    osip_generic_param_add((&(header)->via_params),osip_strdup("hidden"),NULL)
/**
 * Allocate and add a ttl parameter element in a list.
 * @param header The element to work on.
 * @param value The token value.
 */
#define osip_via_set_ttl(header,value)   osip_generic_param_add((&(header)->via_params),osip_strdup("ttl"),value)
/**
 * Allocate and add a maddr parameter element in a list.
 * @param header The element to work on.
 * @param value The token value.
 */
#define osip_via_set_maddr(header,value)   osip_generic_param_add((&(header)->via_params),osip_strdup("maddr"),value)
/**
 * Allocate and add a received parameter element in a list.
 * @param header The element to work on.
 * @param value The token value.
 */
#define osip_via_set_received(header,value) osip_generic_param_add((&(header)->via_params),osip_strdup("received"),value)
/**
 * Allocate and add a branch parameter element in a list.
 * @param header The element to work on.
 * @param value The token value.
 */
#define osip_via_set_branch(header,value)  osip_generic_param_add((&(header)->via_params),osip_strdup("branch"),value)

/**
 * Allocate and add a generic parameter element in a list.
 * @param header The element to work on.
 * @param name The token name.
 * @param value The token value.
 */
#define osip_via_param_add(header,name,value)      osip_generic_param_add((&(header)->via_params),name,value)
/**
 * Find a header parameter in a Via element.
 * @param header The element to work on.
 * @param name The token name to search.
 * @param dest A pointer on the element found.
 */
#define osip_via_param_get_byname(header,name,dest) osip_generic_param_get_byname((&(header)->via_params),name,dest)

/**
 * Check if the Via headers match.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param via1 The first Via header.
 * @param via2 The second Via header.
 */
  int osip_via_match (osip_via_t * via1, osip_via_t * via2);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
