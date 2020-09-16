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


#ifndef _OSIP_CSEQ_H_
#define _OSIP_CSEQ_H_

/**
 * @file osip_cseq.h
 * @brief oSIP osip_cseq header definition.
 */

/**
 * @defgroup oSIP_CSEQ oSIP cseq header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for CSeq headers.
 * @var osip_cseq_t
 */
  typedef struct osip_cseq osip_cseq_t;

/**
 * Definition of the CSeq header.
 * @struct osip_cseq
 */
  struct osip_cseq
  {
    char *method;    /**< CSeq method */
    char *number;    /**< CSeq number */
  };

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a CSeq element.
 * @param header The element to work on.
 */
  int osip_cseq_init (osip_cseq_t ** header);
/**
 * Free a CSeq element.
 * @param header The element to work on.
 */
  void osip_cseq_free (osip_cseq_t * header);
/**
 * Parse a CSeq element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
  int osip_cseq_parse (osip_cseq_t * header, const char *hvalue);
/**
 * Get a string representation of a CSeq element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
  int osip_cseq_to_str (const osip_cseq_t * header, char **dest);
/**
 * Clone a CSeq element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
  int osip_cseq_clone (const osip_cseq_t * header, osip_cseq_t ** dest);
/**
 * Set the number in the CSeq element.
 * @param header The element to work on.
 * @param value The value of the element.
 */
  void osip_cseq_set_number (osip_cseq_t * header, char *value);
/**
 * Get the number from a CSeq header.
 * @param header The element to work on.
 */
  char *osip_cseq_get_number (osip_cseq_t * header);
/**
 * Set the method in the CSeq element.
 * @param header The element to work on.
 * @param value The value of the element.
 */
  void osip_cseq_set_method (osip_cseq_t * header, char *value);
/**
 * Get the method from a CSeq header.
 * @param header The element to work on.
 */
  char *osip_cseq_get_method (osip_cseq_t * header);

/**
 * Check if the CSeq headers match.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param cseq1 The initial CSeq header.
 * @param cseq2 The new CSeq header.
 */
  int osip_cseq_match (osip_cseq_t * cseq1, osip_cseq_t * cseq2);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
