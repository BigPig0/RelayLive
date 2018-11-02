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


#ifndef _OSIP_CALL_ID_H_
#define _OSIP_CALL_ID_H_

/**
 * @file osip_call_id.h
 * @brief oSIP osip_call_id header definition.
 */

/**
 * @defgroup oSIP_CALL_ID oSIP call-id header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for Call-Id headers.
 * @var osip_call_id_t
 */
  typedef struct osip_call_id osip_call_id_t;

/**
 * Definition of the Call-Id header.
 * @struct osip_call_id
 */
  struct osip_call_id
  {
    char *number;    /**< Call-ID number */
    char *host;      /**< Call-ID host information */
  };

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a Call-id element.
 * @param header The element to work on.
 */
  int osip_call_id_init (osip_call_id_t ** header);
/**
 * Free a Call-id element.
 * @param header The element to work on.
 */
  void osip_call_id_free (osip_call_id_t * header);
/**
 * Parse a Call-id element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
  int osip_call_id_parse (osip_call_id_t * header, const char *hvalue);
/**
 * Get a string representation of a Call-id element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
  int osip_call_id_to_str (const osip_call_id_t * header, char **dest);
/**
 * Clone a Call-id element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
  int osip_call_id_clone (const osip_call_id_t * header, osip_call_id_t ** dest);
/**
 * Set the number in the Call-Id element.
 * @param header The element to work on.
 * @param value The value of the element.
 */
  void osip_call_id_set_number (osip_call_id_t * header, char *value);
/**
 * Get the number from a Call-Id header.
 * @param header The element to work on.
 */
  char *osip_call_id_get_number (osip_call_id_t * header);
/**
 * Set the host in the Call-Id element.
 * @param header The element to work on.
 * @param value The value of the element.
 */
  void osip_call_id_set_host (osip_call_id_t * header, char *value);
/**
 * Get the host from a Call-Id header.
 * @param header The element to work on.
 */
  char *osip_call_id_get_host (osip_call_id_t * header);

/**
 * Check if the Call-Id headers match.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param callid1 The initial Call-Id header.
 * @param callid2 The new Call-Id header.
 */
  int osip_call_id_match (osip_call_id_t * callid1, osip_call_id_t * callid2);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
