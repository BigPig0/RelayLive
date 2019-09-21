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

#ifndef __EXTRANSPORT_H__
#define __EXTRANSPORT_H__

#ifdef ENABLE_MPATROL
#include <mpatrol.h>
#endif

#include "eXosip2.h"

#ifdef HAVE_OPENSSL_SSL_H
/* to access version number of ssl from any file */
#include <openssl/opensslv.h>
#endif

#if defined (HAVE_SYS_SELECT_H)
#include <sys/select.h>
#endif

struct eXtl_protocol {
  int enabled;

  int proto_port;
  char proto_name[10];
  char proto_ifs[20];
  int proto_num;
  int proto_family;
  int proto_secure;
  int proto_reliable;
  int proto_local_port;

  int (*tl_init) (struct eXosip_t * excontext);
  int (*tl_free) (struct eXosip_t * excontext);
  int (*tl_open) (struct eXosip_t * excontext);
  int (*tl_set_fdset) (struct eXosip_t * excontext, fd_set * osip_fdset, fd_set * osip_wrset, int *fd_max);
  int (*tl_read_message) (struct eXosip_t * excontext, fd_set * osip_fdset, fd_set * osip_wrset);
  int (*tl_send_message) (struct eXosip_t * excontext, osip_transaction_t * tr, osip_message_t * sip, char *host, int port, int out_socket);
  int (*tl_keepalive) (struct eXosip_t * excontext);
  int (*tl_set_socket) (struct eXosip_t * excontext, int socket);
  int (*tl_masquerade_contact) (struct eXosip_t * excontext, const char *ip, int port);
  int (*tl_get_masquerade_contact) (struct eXosip_t * excontext, char *ip, int ip_size, char *port, int port_size);
  int (*_tl_update_contact) (struct eXosip_t * excontext, osip_message_t *sip);
  int (*tl_reset) (struct eXosip_t * excontext);
  int (*tl_check_connection) (struct eXosip_t * excontext);
};

void eXosip_transport_udp_init (struct eXosip_t *excontext);
void eXosip_transport_tcp_init (struct eXosip_t *excontext);
void eXosip_transport_tls_init (struct eXosip_t *excontext);
void eXosip_transport_dtls_init (struct eXosip_t *excontext);

#if defined (WIN32) || defined (_WIN32_WCE)
#define eXFD_SET(A, B)   FD_SET((unsigned int) A, B)
#else
#define eXFD_SET(A, B)   FD_SET(A, B)
#endif

#endif
