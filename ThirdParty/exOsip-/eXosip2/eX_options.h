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

#ifndef MINISIZE

#ifndef __EX_OPTIONS_H__
#define __EX_OPTIONS_H__

#include <osipparser2/osip_parser.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file eX_options.h
 * @brief eXosip options request API
 *
 * This file provide the API needed to control OPTIONS requests. You can
 * use it to:
 *
 * <ul>
 * <li>build OPTIONS requests.</li>
 * <li>send OPTIONS requests.</li>
 * <li>build OPTIONS answers.</li>
 * <li>send OPTIONS answers.</li>
 * </ul>
 */

/**
 * @defgroup eXosip2_options eXosip2 OPTIONS and UA capabilities Management
 * @ingroup eXosip2_msg
 * @{
 */

/**
 * Build a default OPTIONS message.
 * 
 * @param excontext    eXosip_t instance.
 * @param options   Pointer for the SIP request to build.
 * @param to        SIP url for callee.
 * @param from      SIP url for caller.
 * @param route     Route header for INVITE. (optional)
 */
  int eXosip_options_build_request (struct eXosip_t *excontext, osip_message_t ** options, const char *to, const char *from, const char *route);

/**
 * Send an OPTIONS request.
 * 
 * @param excontext    eXosip_t instance.
 * @param options          SIP OPTIONS message to send.
 */
  int eXosip_options_send_request (struct eXosip_t *excontext, osip_message_t * options);

/**
 * Build answer for an OPTIONS request.
 * 
 * @param excontext    eXosip_t instance.
 * @param tid             id of OPTIONS transaction.
 * @param status          status for SIP answer to build.
 * @param answer          The SIP answer to build.
 */
  int eXosip_options_build_answer (struct eXosip_t *excontext, int tid, int status, osip_message_t ** answer);

/**
 * Send answer for an OPTIONS request.
 * 
 * @param excontext    eXosip_t instance.
 * @param tid             id of OPTIONS transaction.
 * @param status          status for SIP answer to send.
 * @param answer          The SIP answer to send. (default will be sent if NULL)
 */
  int eXosip_options_send_answer (struct eXosip_t *excontext, int tid, int status, osip_message_t * answer);

/** @} */


#ifdef __cplusplus
}
#endif
#endif
#endif
