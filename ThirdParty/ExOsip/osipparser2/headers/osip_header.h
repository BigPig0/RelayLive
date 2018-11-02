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


#ifndef _OSIP_HEADER_H_
#define _OSIP_HEADER_H_

#include <osipparser2/osip_uri.h>

/**
 * @file osip_header.h
 * @brief oSIP osip_header definition.
 *
 */

/**
 * @defgroup oSIP_HEADER oSIP header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for 'unknown' headers.
 * NOTE: 'unknown' header' are used in oSIP for all header that are not
 * defined by oSIP in the osip_message_t structure. This means that all
 * 'unknown' header has to be handled with the API related to this 
 * structure.
 * @var osip_header_t
 */
  typedef struct osip_header osip_header_t;

/**
 * Definition of a generic sip header.
 * @struct osip_header
 */
  struct osip_header
  {
    char *hname;     /**< Name of header */
    char *hvalue;    /**< Value for header */
  };

/**
 * Structure for generic parameter headers.
 * Generic parameter are used in a lot of headers. (To, From, Route,
 * Record-Route...) All those headers use a common API but this is
 * hidden by MACROs that you can be found in smsg.h.
 * @var osip_generic_param_t
 */
  typedef osip_uri_param_t osip_generic_param_t;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a header element.
 * @param header The element to work on.
 */
  int osip_header_init (osip_header_t ** header);
/**
 * Free a header element.
 * @param header The element to work on.
 */
  void osip_header_free (osip_header_t * header);
/**
 * Get a string representation of a header element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated buffer.
 */
  int osip_header_to_str (const osip_header_t * header, char **dest);
/**
 * Get the token name a header element.
 * @param header The element to work on.
 */
  char *osip_header_get_name (const osip_header_t * header);
/**
 * Set the token name a header element.
 * @param header The element to work on.
 * @param pname The token name to set.
 */
  void osip_header_set_name (osip_header_t * header, char *pname);
/**
 * Get the token value a header element.
 * @param header The element to work on.
 */
  char *osip_header_get_value (const osip_header_t * header);
/**
 * Set the token value a header element.
 * @param header The element to work on.
 * @param pvalue The token value to set.
 */
  void osip_header_set_value (osip_header_t * header, char *pvalue);
/**
 * Clone a header element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
  int osip_header_clone (const osip_header_t * header, osip_header_t ** dest);


#ifdef __cplusplus
}
#endif

/** @} */

#endif
