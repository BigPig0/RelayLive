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


#ifndef _JPIPE_H_
#define _JPIPE_H_

#ifndef OSIP_MONOTHREAD

#include <eXosip2/eXosip.h>

#ifdef _WIN32_WCE
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <osipparser2/osip_port.h>
#endif

#ifndef WIN32
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif

#if defined(__arc__)
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

/**
 * @file jpipe.h
 * @brief PPL Pipe Handling Routines
 */

/**
 * @defgroup JPIPE Pipe Handling
 * @ingroup PPL
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Structure for storing a pipe descriptor
 * @defvar jpipe_t
 */
  typedef struct jpipe_t jpipe_t;

  struct jpipe_t {
    int pipes[2];
  };

/**
 * Get New pipe pair.
 */
  jpipe_t *jpipe (void);

/**
 * Close pipe
 */
  int jpipe_close (jpipe_t * apipe);

/**
 * Write in a pipe.
 */
  int jpipe_write (jpipe_t * pipe, const void *buf, int count);

/**
 * Read in a pipe.
 */
  int jpipe_read (jpipe_t * pipe, void *buf, int count);

/**
 * Get descriptor of reading pipe.
 */
  int jpipe_get_read_descr (jpipe_t * pipe);

#ifdef __cplusplus
}
#endif
#endif
/** @} */
#endif
