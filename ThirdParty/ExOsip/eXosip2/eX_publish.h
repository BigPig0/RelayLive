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

#ifndef __EX_PUBLISH_H__
#define __EX_PUBLISH_H__

#include <osipparser2/osip_parser.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file eX_publish.h
 * @brief eXosip publish request API
 *
 * This file provide the API needed to control PUBLISH requests. You can
 * use it to:
 *
 * <ul>
 * <li>build PUBLISH requests.</li>
 * <li>send PUBLISH requests.</li>
 * </ul>
 */

/**
 * @defgroup eXosip2_publish eXosip2 Publication Management
 * @ingroup eXosip2_msg
 * @{
 */

/**
 * build publication for a user. (PUBLISH request)
 * 
 * @param excontext    eXosip_t instance.
 * @param message   returned published request.
 * @param to        SIP url for callee.
 * @param from      SIP url for caller.
 * @param route     Route used for publication.
 * @param event     SIP Event header.
 * @param expires   SIP Expires header.
 * @param ctype     Content-Type of body.
 * @param body     body for publication.
 */
  int eXosip_build_publish (struct eXosip_t *excontext, osip_message_t ** message, const char *to, const char *from, const char *route, const char *event, const char *expires, const char *ctype, const char *body);

/**
 * Send an Publication Message (PUBLISH request).
 * 
 * @param excontext    eXosip_t instance.
 * @param message is a ready to be sent publish message .
 * @param to the aor of the publish request 
 */
  int eXosip_publish (struct eXosip_t *excontext, osip_message_t * message, const char *to);


/** @} */


#ifdef __cplusplus
}
#endif
#endif
#endif
