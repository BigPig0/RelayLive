/*
  eXosip - This is the eXtended osip library.
  Copyright (C) 2001-2015 Aymeric MOIZARD amoizard@antisip.com
  
  eXosip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  eXosip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  
  In addition, as a special exception, the copyright holders give
  permission to link the code of portions of this program with the
  OpenSSL library under certain conditions as described in each
  individual source file, and distribute linked combinations
  including the two.
  You must obey the GNU General Public License in all respects
  for all of the code used other than OpenSSL.  If you modify
  file(s) with this exception, you may extend this exception to your
  version of the file(s), but you are not obligated to do so.  If you
  do not wish to do so, delete this exception statement from your
  version.  If you delete this exception statement from all source
  files in the program, then also delete it here.
*/


#ifdef ENABLE_MPATROL
#include <mpatrol.h>
#endif

#ifndef __EX_CALL_H__
#define __EX_CALL_H__

#include <osipparser2/osip_parser.h>
#include <osipparser2/sdp_message.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file eX_call.h
 * @brief eXosip call API
 *
 * This file provide the API needed to control calls. You can
 * use it to:
 *
 * <ul>
 * <li>build initial invite.</li>
 * <li>send initial invite.</li>
 * <li>build request within the call.</li>
 * <li>send request within the call.</li>
 * </ul>
 *
 * This API can be used to build the following messages:
 * <pre>
 *    INVITE, INFO, OPTIONS, REFER, UPDATE, NOTIFY
 * </pre>
 */

/**
 * @defgroup eXosip2_call eXosip2 INVITE and Call Management
 * @ingroup eXosip2_msg
 * @{
 */

  struct eXosip_call_t;

/**
 * Set a new application context for an existing call
 *
 * @param excontext    eXosip_t instance.
 * @param id       call-id or dialog-id of call
 * @param reference New application context.
 */
  int eXosip_call_set_reference (struct eXosip_t *excontext, int id, void *reference);

/**
 * Get the application context pointer for an existing call.
 * 
 * @param excontext    eXosip_t instance.
 * @param cid            id of the call.
 * @return               Application context reference
 */
  void *eXosip_call_get_reference (struct eXosip_t *excontext, int cid);

/**
 * Build a default INVITE message for a new call.
 * 
 * @param excontext    eXosip_t instance.
 * @param invite    Pointer for the SIP element to hold.
 * @param to        SIP url for callee.
 * @param from      SIP url for caller.
 * @param route     Route header for INVITE. (optional)
 * @param subject   Subject for the call.
 */
  int eXosip_call_build_initial_invite (struct eXosip_t *excontext, osip_message_t ** invite, const char *to, const char *from, const char *route, const char *subject);

/**
 * Initiate a call.
 * 
 * @param excontext    eXosip_t instance.
 * @param invite          SIP INVITE message to send.
 */
  int eXosip_call_send_initial_invite (struct eXosip_t *excontext, osip_message_t * invite);

/**
 * Build a default request within a call. (INVITE, OPTIONS, INFO, REFER)
 * 
 * @param excontext    eXosip_t instance.
 * @param did          dialog id of call.
 * @param method       request type to build.
 * @param request      The sip request to build.
 */
  int eXosip_call_build_request (struct eXosip_t *excontext, int did, const char *method, osip_message_t ** request);

/**
 * Build a default ACK for a 200ok received.
 * 
 * @param excontext    eXosip_t instance.
 * @param did          dialog id of call.
 * @param ack          The sip request to build.
 */
  int eXosip_call_build_ack (struct eXosip_t *excontext, int did, osip_message_t ** ack);

/**
 * Send the ACK for the 200ok received..
 * 
 * @param excontext    eXosip_t instance.
 * @param did          dialog id of call.
 * @param ack          SIP ACK message to send.
 */
  int eXosip_call_send_ack (struct eXosip_t *excontext, int did, osip_message_t * ack);

/**
 * Build a default REFER for a call transfer.
 * 
 * @param excontext    eXosip_t instance.
 * @param did          dialog id of call.
 * @param refer_to     url for call transfer (Refer-To header).
 * @param request      The sip request to build.
 */
  int eXosip_call_build_refer (struct eXosip_t *excontext, int did, const char *refer_to, osip_message_t ** request);

/**
 * Build a default INFO within a call.
 * 
 * @param excontext    eXosip_t instance.
 * @param did          dialog id of call.
 * @param request      The sip request to build.
 */
  int eXosip_call_build_info (struct eXosip_t *excontext, int did, osip_message_t ** request);

/**
 * Build a default OPTIONS within a call.
 * 
 * @param excontext    eXosip_t instance.
 * @param did          dialog id of call.
 * @param request      The sip request to build.
 */
  int eXosip_call_build_options (struct eXosip_t *excontext, int did, osip_message_t ** request);

/**
 * Build a default UPDATE within a call.
 * 
 * @param excontext    eXosip_t instance.
 * @param did          dialog id of call.
 * @param request      The sip request to build.
 */
  int eXosip_call_build_update (struct eXosip_t *excontext, int did, osip_message_t ** request);

/**
 * Build a default NOTIFY within a call.
 * 
 * @param excontext    eXosip_t instance.
 * @param did                   dialog id of call.
 * @param subscription_status   Subscription status of the request.
 * @param request               The sip request to build.
 */
  int eXosip_call_build_notify (struct eXosip_t *excontext, int did, int subscription_status, osip_message_t ** request);

/**
 * send the request within call. (INVITE, OPTIONS, INFO, REFER, UPDATE)
 * 
 * @param excontext    eXosip_t instance.
 * @param did          dialog id of call.
 * @param request      The sip request to send.
 */
  int eXosip_call_send_request (struct eXosip_t *excontext, int did, osip_message_t * request);

/**
 * Build default Answer for request.
 * 
 * @param excontext    eXosip_t instance.
 * @param tid          id of transaction to answer.
 * @param status       Status code to use.
 * @param answer       The sip answer to build.
 */
  int eXosip_call_build_answer (struct eXosip_t *excontext, int tid, int status, osip_message_t ** answer);

/**
 * Send Answer for invite.
 * 
 * @param excontext    eXosip_t instance.
 * @param tid          id of transaction to answer.
 * @param status       response status if answer is NULL. (not allowed for 2XX)
 * @param answer       The sip answer to send.
 */
  int eXosip_call_send_answer (struct eXosip_t *excontext, int tid, int status, osip_message_t * answer);

/**
 * Terminate a call.
 * send CANCEL, BYE or 603 Decline.
 * 
 * @param excontext    eXosip_t instance.
 * @param cid          call id of call.
 * @param did          dialog id of call.
 */
  int eXosip_call_terminate (struct eXosip_t *excontext, int cid, int did);

/**
 * Terminate a call and add a Reason header.
 * send CANCEL, BYE or 603 Decline.
 * 
 * @param excontext    eXosip_t instance.
 * @param cid          call id of call.
 * @param did          dialog id of call.
 * @param reason       Reason header.
 */
  int eXosip_call_terminate_with_reason (struct eXosip_t *excontext, int cid, int did, const char *reason);
  
/**
 * Build a PRACK for invite.
 * 
 * @param excontext    eXosip_t instance.
 * @param tid          id of the invite transaction.
 * @param response1xx  The sip response for which we build a PRACK.
 * @param prack        The sip prack to build.
 */
  int eXosip_call_build_prack (struct eXosip_t *excontext, int tid, osip_message_t *response1xx, osip_message_t ** prack);

/**
 * Send a PRACK for invite.
 * 
 * @param excontext    eXosip_t instance.
 * @param tid          id of the invite transaction.
 * @param prack        The sip prack to send.
 */
  int eXosip_call_send_prack (struct eXosip_t *excontext, int tid, osip_message_t * prack);

/**
 * Get Refer-To header with Replace parameter from dialog.
 * 
 * @param excontext    eXosip_t instance.
 * @param did          id of the dialog.
 * @param refer_to     buffer to be filled with refer-to info.
 * @param refer_to_len size of refer_to buffer.
 */
  int eXosip_call_get_referto (struct eXosip_t *excontext, int did, char *refer_to, size_t refer_to_len);


/**
 * Return did (or cid) for the replace header.
 * 
 * @param excontext    eXosip_t instance.
 * @param replaces     buffer to be filled with refer-to info.
 */
  int eXosip_call_find_by_replaces (struct eXosip_t *excontext, char *replaces);

/** @} */


#ifdef __cplusplus
}
#endif
#endif
