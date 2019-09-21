/*
  The oSIP library implements the Session Initiation Protocol (SIP -rfc3261-)
  Copyright (C) 2001-2012 Aymeric MOIZARD amoizard@antisip.com
  
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

#ifndef _DIALOG_H_
#define _DIALOG_H_

#include <osip2/osip.h>

/**
 * @file osip_dialog.h
 * @brief oSIP dialog Routines
 *
 */

/**
 * @defgroup oSIP_DIALOG oSIP dialog Handling
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif


#ifndef DOXYGEN
  typedef enum _osip_dialog_type_t {
    CALLER,
    CALLEE
  } osip_dialog_type_t;
#endif


/**
 * Structure for referencing a dialog.
 * @var osip_dialog_t
 */
  typedef struct osip_dialog osip_dialog_t;


/**
 * Structure for referencing a dialog.
 * @struct osip_dialog
 */
  struct osip_dialog {
    char *call_id;                                               /**< Call-ID*/
    char *local_tag;                                             /**< local tag */
    char *remote_tag;                                            /**< remote tag */
    char *line_param;                                            /**< line param from request uri for incoming calls */
    osip_list_t route_set;                              /**< route set */
    int local_cseq;                                              /**< last local cseq */
    int remote_cseq;                                             /**< last remote cseq*/
    osip_to_t *remote_uri;                               /**< remote_uri */
    osip_from_t *local_uri;                              /**< local_uri */
    osip_contact_t *remote_contact_uri;
                                                                                 /**< remote contact_uri */
    int secure;                                                          /**< use secure transport layer */

    osip_dialog_type_t type;                             /**< type of dialog (CALLEE or CALLER) */
    state_t state;                                               /**< DIALOG_EARLY || DIALOG_CONFIRMED || DIALOG_CLOSED */
    void *your_instance;                                 /**< for application data reference */
  };

/**
 * Link osip dialog to application
 * @param dialog The osip dialog
 * @param instance The application instance
 */
#define osip_dialog_set_instance(dialog,instance) (dialog)->your_instance = (void*)(instance)

/**
 * Retrieve application instance from dialog
 * @param dialog The osip dialog
 * @return instance The application instance
 */
#define osip_dialog_get_instance(dialog)          (dialog)->your_instance

/**
 * Allocate a osip_dialog_t element as a UAC.
 * NOTE1: The dialog should be created when the first response is received.
 *        (except for a 100 Trying)
 * NOTE2: Remote UA should be compliant! If not (not tag in the to header?)
 *        the old mechanism is used to match the request but if 2 uncompliant
 *        UA both answer 200 OK for the same transaction, they won't be detected.
 *        This is a major BUG in the old rfc.
 * @param dialog The element to allocate.
 * @param response The response containing the informations.
 */
  int osip_dialog_init_as_uac (osip_dialog_t ** dialog, osip_message_t * response);
/**
 * Allocate a osip_dialog_t element as a UAC.
 * <UL><LI>This could be used to initiate dialog with a NOTIFY coming
 * before the answer for a subscribe has reached us.</LI></UL>
 * @param dialog The element to allocate.
 * @param next_request The response containing the informations.
 * @param local_cseq The local cseq
 */
  int osip_dialog_init_as_uac_with_remote_request (osip_dialog_t ** dialog, osip_message_t * next_request, int local_cseq);

/**
 * Allocate a osip_dialog_t element as a UAS.
 * NOTE1: The dialog should be created when the first response is sent.
 *        (except for a 100 Trying)
 * @param dialog The element to allocate.
 * @param invite The INVITE request containing some informations.
 * @param response The response containing other informations.
 */
  int osip_dialog_init_as_uas (osip_dialog_t ** dialog, osip_message_t * invite, osip_message_t * response);
/**
 * Free all resource in a osip_dialog_t element.
 * @param dialog The element to free.
 */
  void osip_dialog_free (osip_dialog_t * dialog);
/**
 * Set the state of the dialog.
 * This is useful to keep information on who is the initiator of the call.
 * @param dialog The element to work on.
 * @param type The type of dialog (CALLEE or CALLER).
 */
  void osip_dialog_set_state (osip_dialog_t * dialog, state_t type);
/**
 * Update the Route-Set as UAS of a dialog.
 * NOTE: bis-09 says that only INVITE transactions can update the route-set.
 * NOTE: bis-09 says that updating the route-set means: update the contact
 *       field only (AND NOT THE ROUTE-SET). This method follow this behaviour.
 * NOTE: This method should be called for each request
 *       received for a dialog.
 * @param dialog The element to work on.
 * @param invite The invite received.
 */
  int osip_dialog_update_route_set_as_uas (osip_dialog_t * dialog, osip_message_t * invite);
/**
 * Update the CSeq (remote cseq) during a UAS transaction of a dialog.
 * NOTE: All INCOMING transactions MUST update the remote CSeq.
 * @param dialog The element to work on.
 * @param request The request received.
 */
  int osip_dialog_update_osip_cseq_as_uas (osip_dialog_t * dialog, osip_message_t * request);

/**
 * Match a response received with a dialog.
 * @param dialog The element to work on.
 * @param response The response received.
 */
  int osip_dialog_match_as_uac (osip_dialog_t * dialog, osip_message_t * response);
/**
 * Update the tag as UAC of a dialog?. (this could be needed if the 180
 * does not contains any tag, but the 200 contains one.
 * @param dialog The element to work on.
 * @param response The response received.
 */
  int osip_dialog_update_tag_as_uac (osip_dialog_t * dialog, osip_message_t * response);
/**
 * Update the Route-Set as UAC of a dialog.
 * NOTE: bis-09 says that only INVITE transactions can update the route-set.
 * NOTE: bis-09 says that updating the route-set means: update the contact
 *       field only (AND NOT THE ROUTE-SET). This method follow this behaviour.
 * NOTE: This method should be called for each request (except 100 Trying)
 *       received for a dialog.
 * @param dialog The element to work on.
 * @param response The response received.
 */
  int osip_dialog_update_route_set_as_uac (osip_dialog_t * dialog, osip_message_t * response);

/**
 * Match a request (response sent?) received with a dialog.
 * @param dialog The element to work on.
 * @param request The request received.
 */
  int osip_dialog_match_as_uas (osip_dialog_t * dialog, osip_message_t * request);

/**
 * Is dialog initiated by as CALLER
 * @param dialog The element to work on.
 */
  int osip_dialog_is_originator (osip_dialog_t * dialog);
/**
 * Is dialog initiated by as CALLEE
 * @param dialog The element to work on.
 */
  int osip_dialog_is_callee (osip_dialog_t * dialog);


#ifdef __cplusplus
}
#endif
/** @} */
#endif
