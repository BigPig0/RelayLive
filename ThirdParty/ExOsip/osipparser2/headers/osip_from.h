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


#ifndef _OSIP_FROM_H_
#define _OSIP_FROM_H_

#include <osipparser2/osip_list.h>
#include <osipparser2/osip_uri.h>

/**
 * @file osip_from.h
 * @brief oSIP osip_from header definition.
 */

/**
 * @defgroup oSIP_FROM oSIP from header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for From headers.
 * @var osip_from_t
 */
  typedef struct osip_from osip_from_t;

/**
 * Definition of the From header.
 * @struct osip_from
 */
  struct osip_from
  {
    char *displayname;       /**< Display Name */ 
    osip_uri_t *url;         /**< url */
    osip_list_t gen_params;  /**< other From parameters */
  };

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a From element.
 * @param header The element to work on.
 */
  int osip_from_init (osip_from_t ** header);
/**
 * Free a From element.
 * @param header The element to work on.
 */
  void osip_from_free (osip_from_t * header);
/**
 * Parse a From element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
  int osip_from_parse (osip_from_t * header, const char *hvalue);
/**
 * Get a string representation of a From element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
  int osip_from_to_str (const osip_from_t * header, char **dest);
/**
 * Clone a From element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
  int osip_from_clone (const osip_from_t * header, osip_from_t ** dest);
/**
 * Set the displayname in the From element.
 * @param header The element to work on.
 * @param value The value of the element.
 */
  void osip_from_set_displayname (osip_from_t * header, char *value);
/**
 * Get the displayname from a From header.
 * @param header The element to work on.
 */
  char *osip_from_get_displayname (osip_from_t * header);
/**
 * Set the url in the From element.
 * @param header The element to work on.
 * @param url The value of the element.
 */
  void osip_from_set_url (osip_from_t * header, osip_uri_t * url);
/**
 * Get the url from a From header.
 * @param header The element to work on.
 */
  osip_uri_t *osip_from_get_url (osip_from_t * header);
/**
 * Get a header parameter from a From element.
 * @param header The element to work on.
 * @param pos The index of the element to get.
 * @param dest A pointer on the element found.
 */
  int osip_from_param_get (osip_from_t * header, int pos, osip_generic_param_t ** dest);
/**
 * Allocate and add a generic parameter element in a list.
 * @param header The element to work on.
 * @param name The token name.
 * @param value The token value.
 */
#define osip_from_param_add(header,name,value)      osip_generic_param_add((&(header)->gen_params),name,value)
/**
 * Find a header parameter in a From element.
 * @param header The element to work on.
 * @param name The token name to search.
 * @param dest A pointer on the element found.
 */
#define osip_from_param_get_byname(header,name,dest) osip_generic_param_get_byname((&(header)->gen_params),name,dest)

/**
 * Find the tag parameter in a From element.
 * @param header The element to work on.
 * @param dest A pointer on the element found.
 */
#define osip_from_get_tag(header,dest)    osip_generic_param_get_byname((&(header)->gen_params), "tag",dest)
/**
 * Allocate and add a tag parameter element in a Contact element.
 * @param header The element to work on.
 * @param value The token value.
 */
#define osip_from_set_tag(header,value)     osip_generic_param_add((&(header)->gen_params), osip_strdup("tag"),value)

#ifndef DOXYGEN			/* avoid DOXYGEN warning */
/* Compare the username, host and tag part (if exist) of the two froms */
  int osip_from_compare (osip_from_t * header1, osip_from_t * header2);
#endif

/**
 * Check if the tags in the From headers match.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param from1 The first From header.
 * @param from2 The second From header.
 */
  int osip_from_tag_match (osip_from_t * from1, osip_from_t * from2);


#ifdef __cplusplus
}
#endif

/** @} */

#endif
