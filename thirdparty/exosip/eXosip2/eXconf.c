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

#include "eXosip2.h"
#include <eXosip2/eXosip.h>

#include <osip2/osip_mt.h>
#include <osip2/osip_condv.h>

#if defined (_WIN32_WCE)
#include "inet_ntop.h"
#elif WIN32
#include "inet_ntop.h"
#endif

#ifndef OSIP_MONOTHREAD
static void *_eXosip_thread (void *arg);
#endif
static void _eXosip_keep_alive (struct eXosip_t *excontext);

const char *
eXosip_get_version (void)
{
  return EXOSIP_VERSION;
}

int
eXosip_set_cbsip_message (struct eXosip_t *excontext, CbSipCallback cbsipCallback)
{
  excontext->cbsipCallback = cbsipCallback;
  return 0;
}

void
eXosip_masquerade_contact (struct eXosip_t *excontext, const char *public_address, int port)
{
  if (excontext->eXtl_transport.tl_masquerade_contact == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "no transport protocol selected!\n"));
    if (public_address == NULL || public_address[0] == '\0') {
      memset (excontext->udp_firewall_ip, '\0', sizeof (excontext->udp_firewall_ip));
      memset (excontext->udp_firewall_port, '\0', sizeof (excontext->udp_firewall_port));
      memset (excontext->tcp_firewall_ip, '\0', sizeof (excontext->tcp_firewall_ip));
      memset (excontext->tcp_firewall_port, '\0', sizeof (excontext->tcp_firewall_port));
      memset (excontext->tls_firewall_ip, '\0', sizeof (excontext->tls_firewall_ip));
      memset (excontext->tls_firewall_port, '\0', sizeof (excontext->tls_firewall_port));
      memset (excontext->dtls_firewall_ip, '\0', sizeof (excontext->dtls_firewall_ip));
      memset (excontext->dtls_firewall_port, '\0', sizeof (excontext->dtls_firewall_port));
      return;
    }
    snprintf (excontext->udp_firewall_ip, sizeof (excontext->udp_firewall_ip), "%s", public_address);
    snprintf (excontext->tcp_firewall_ip, sizeof (excontext->tcp_firewall_ip), "%s", public_address);
    snprintf (excontext->tls_firewall_ip, sizeof (excontext->tls_firewall_ip), "%s", public_address);
    snprintf (excontext->dtls_firewall_ip, sizeof (excontext->dtls_firewall_ip), "%s", public_address);
    if (port > 0) {
      snprintf (excontext->udp_firewall_port, sizeof (excontext->udp_firewall_port), "%i", port);
      snprintf (excontext->tcp_firewall_port, sizeof (excontext->tcp_firewall_port), "%i", port);
      snprintf (excontext->tls_firewall_port, sizeof (excontext->tls_firewall_port), "%i", port);
      snprintf (excontext->dtls_firewall_port, sizeof (excontext->dtls_firewall_port), "%i", port);
    }
    return;
  }
  excontext->eXtl_transport.tl_masquerade_contact (excontext, public_address, port);
  return;
}

int
eXosip_guess_localip (struct eXosip_t *excontext, int family, char *address, int size)
{
  return _eXosip_guess_ip_for_via (excontext, family, address, size);
}

int
_eXosip_is_public_address (const char *c_address)
{
  return (0 != strncmp (c_address, "192.168", 7)
          && 0 != strncmp (c_address, "10.", 3)
          && 0 != strncmp (c_address, "172.16.", 7)
          && 0 != strncmp (c_address, "172.17.", 7)
          && 0 != strncmp (c_address, "172.18.", 7)
          && 0 != strncmp (c_address, "172.19.", 7)
          && 0 != strncmp (c_address, "172.20.", 7)
          && 0 != strncmp (c_address, "172.21.", 7)
          && 0 != strncmp (c_address, "172.22.", 7)
          && 0 != strncmp (c_address, "172.23.", 7)
          && 0 != strncmp (c_address, "172.24.", 7)
          && 0 != strncmp (c_address, "172.25.", 7)
          && 0 != strncmp (c_address, "172.26.", 7)
          && 0 != strncmp (c_address, "172.27.", 7)
          && 0 != strncmp (c_address, "172.28.", 7)
          && 0 != strncmp (c_address, "172.29.", 7)
          && 0 != strncmp (c_address, "172.30.", 7)
          && 0 != strncmp (c_address, "172.31.", 7)
          && 0 != strncmp (c_address, "169.254", 7));
}

void
eXosip_set_user_agent (struct eXosip_t *excontext, const char *user_agent)
{
  osip_free (excontext->user_agent);
  excontext->user_agent = osip_strdup (user_agent);
}

static void
_eXosip_kill_transaction (struct eXosip_t *excontext, osip_list_t * transactions)
{
  osip_transaction_t *transaction;

  if (!osip_list_eol (transactions, 0)) {
    /* some transaction are still used by osip,
       transaction should be released by modules! */
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "module sfp: _osip_kill_transaction transaction should be released by modules!\n"));
  }

  while (!osip_list_eol (transactions, 0)) {
    transaction = (osip_transaction_t *)osip_list_get (transactions, 0);
    _eXosip_transaction_free (excontext, transaction);
  }
}

void
eXosip_quit (struct eXosip_t *excontext)
{
  jauthinfo_t *jauthinfo;
  eXosip_call_t *jc;
  eXosip_reg_t *jreg;

#ifndef MINISIZE
  eXosip_notify_t *jn;
  eXosip_subscribe_t *js;
  eXosip_pub_t *jpub;
#endif
#ifndef OSIP_MONOTHREAD
  int i;
#endif

  if (excontext == NULL)
    return;

  if (excontext->j_stop_ua == -1) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "eXosip: already stopped!\n"));
    return;
  }

  excontext->j_stop_ua = 1;     /* ask to quit the application */
  _eXosip_wakeup (excontext);
  eXosip_wakeup_event (excontext);

#ifndef OSIP_MONOTHREAD
  if (excontext->j_thread != NULL) {
    i = osip_thread_join ((struct osip_thread *) excontext->j_thread);
    if (i != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: can't terminate thread!\n"));
    }
    osip_free ((struct osip_thread *) excontext->j_thread);
  }

  jpipe_close (excontext->j_socketctl);
  jpipe_close (excontext->j_socketctl_event);
#endif

  osip_free (excontext->user_agent);

  for (jc = excontext->j_calls; jc != NULL; jc = excontext->j_calls) {
    REMOVE_ELEMENT (excontext->j_calls, jc);
    _eXosip_call_free (excontext, jc);
  }

#ifndef MINISIZE
  for (js = excontext->j_subscribes; js != NULL; js = excontext->j_subscribes) {
    REMOVE_ELEMENT (excontext->j_subscribes, js);
    _eXosip_subscription_free (excontext, js);
  }

  for (jn = excontext->j_notifies; jn != NULL; jn = excontext->j_notifies) {
    REMOVE_ELEMENT (excontext->j_notifies, jn);
    _eXosip_notify_free (excontext, jn);
  }
#endif

#ifndef OSIP_MONOTHREAD
  osip_mutex_destroy ((struct osip_mutex *) excontext->j_mutexlock);
#if !defined (_WIN32_WCE)
  osip_cond_destroy ((struct osip_cond *) excontext->j_cond);
#endif
#endif

  for (jreg = excontext->j_reg; jreg != NULL; jreg = excontext->j_reg) {
    REMOVE_ELEMENT (excontext->j_reg, jreg);
    _eXosip_reg_free (excontext, jreg);
  }

#ifndef MINISIZE
  for (jpub = excontext->j_pub; jpub != NULL; jpub = excontext->j_pub) {
    REMOVE_ELEMENT (excontext->j_pub, jpub);
    _eXosip_pub_free (excontext, jpub);
  }
#endif

  while (!osip_list_eol (&excontext->j_transactions, 0)) {
    osip_transaction_t *tr = (osip_transaction_t *) osip_list_get (&excontext->j_transactions, 0);

    if (tr->state == IST_TERMINATED || tr->state == ICT_TERMINATED || tr->state == NICT_TERMINATED || tr->state == NIST_TERMINATED) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Release a terminated transaction\n"));
    }
    osip_list_remove (&excontext->j_transactions, 0);
    _eXosip_transaction_free (excontext, tr);
  }

  _eXosip_kill_transaction (excontext, &excontext->j_osip->osip_ict_transactions);
  _eXosip_kill_transaction (excontext, &excontext->j_osip->osip_nict_transactions);
  _eXosip_kill_transaction (excontext, &excontext->j_osip->osip_ist_transactions);
  _eXosip_kill_transaction (excontext, &excontext->j_osip->osip_nist_transactions);
  osip_release (excontext->j_osip);

  {
    eXosip_event_t *ev;

    for (ev = osip_fifo_tryget (excontext->j_events); ev != NULL; ev = osip_fifo_tryget (excontext->j_events))
      eXosip_event_free (ev);
  }

  osip_fifo_free (excontext->j_events);

  for (jauthinfo = excontext->authinfos; jauthinfo != NULL; jauthinfo = excontext->authinfos) {
    REMOVE_ELEMENT (excontext->authinfos, jauthinfo);
    osip_free (jauthinfo);
  }

  {
    struct eXosip_http_auth *http_auth;
    int pos;

    /* update entries with same call_id */
    for (pos = 0; pos < MAX_EXOSIP_HTTP_AUTH; pos++) {
      http_auth = &excontext->http_auths[pos];
      if (http_auth->pszCallId[0] == '\0')
        continue;
      osip_proxy_authenticate_free (http_auth->wa);
      memset (http_auth, 0, sizeof (struct eXosip_http_auth));
    }
  }

  if (excontext->eXtl_transport.tl_free != NULL)
    excontext->eXtl_transport.tl_free (excontext);

  _eXosip_counters_free(&excontext->average_transactions);
  _eXosip_counters_free(&excontext->average_registrations);
  _eXosip_counters_free(&excontext->average_calls);
  _eXosip_counters_free(&excontext->average_publications);
  _eXosip_counters_free(&excontext->average_subscriptions);
  _eXosip_counters_free(&excontext->average_insubscriptions);

  memset (excontext, 0, sizeof (eXosip_t));
  excontext->j_stop_ua = -1;

#ifdef WIN32
  WSACleanup();
#endif
  return;
}

int
eXosip_set_socket (struct eXosip_t *excontext, int transport, int socket, int port)
{
  if (excontext->eXtl_transport.enabled > 0) {
    /* already set */
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: already listening somewhere\n"));
    return OSIP_WRONG_STATE;
  }

  if (transport == IPPROTO_UDP) {
    eXosip_transport_udp_init (excontext);
    if (excontext->eXtl_transport.tl_init != NULL)
      excontext->eXtl_transport.tl_init (excontext);
    excontext->eXtl_transport.proto_port = port;
    excontext->eXtl_transport.tl_set_socket (excontext, socket);
    snprintf (excontext->transport, sizeof (excontext->transport), "%s", "UDP");
  }
  else if (transport == IPPROTO_TCP) {
    eXosip_transport_tcp_init (excontext);
    if (excontext->eXtl_transport.tl_init != NULL)
      excontext->eXtl_transport.tl_init (excontext);
    excontext->eXtl_transport.proto_port = port;
    excontext->eXtl_transport.tl_set_socket (excontext, socket);
    snprintf (excontext->transport, sizeof (excontext->transport), "%s", "TCP");
  }
  else
    return OSIP_BADPARAMETER;

#ifndef OSIP_MONOTHREAD
  excontext->j_thread = (void *) osip_thread_create (20000, _eXosip_thread, excontext);
  if (excontext->j_thread == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot start thread!\n"));
    return OSIP_UNDEFINED_ERROR;
  }
#endif
  return OSIP_SUCCESS;
}

#ifdef IPV6_V6ONLY
int
setsockopt_ipv6only (int sock)
{
  int on = 1;

  return setsockopt (sock, IPPROTO_IPV6, IPV6_V6ONLY, (char *) &on, sizeof (on));
}
#endif /* IPV6_V6ONLY */

#ifndef MINISIZE
int
eXosip_find_free_port (struct eXosip_t *excontext, int free_port, int transport)
{
  int res1;
  int res2;
  struct addrinfo *addrinfo_rtp = NULL;
  struct addrinfo *curinfo_rtp;
  struct addrinfo *addrinfo_rtcp = NULL;
  struct addrinfo *curinfo_rtcp;
  int sock;
  int count;

  if (free_port>0) {
    for (count = 0; count < 8; count++) {
      if (excontext->ipv6_enable == 0)
        res1 = _eXosip_get_addrinfo (excontext, &addrinfo_rtp, "0.0.0.0", free_port + count * 2, transport);
      else
        res1 = _eXosip_get_addrinfo (excontext, &addrinfo_rtp, "::", free_port + count * 2, transport);
      if (res1 != 0)
        return res1;
      if (excontext->ipv6_enable == 0)
        res2 = _eXosip_get_addrinfo (excontext, &addrinfo_rtcp, "0.0.0.0", free_port + count * 2 + 1, transport);
      else
        res2 = _eXosip_get_addrinfo (excontext, &addrinfo_rtcp, "::", free_port + count * 2 + 1, transport);
      if (res2 != 0) {
        _eXosip_freeaddrinfo (addrinfo_rtp);
        return res2;
      }

      sock = -1;
      for (curinfo_rtp = addrinfo_rtp; curinfo_rtp; curinfo_rtp = curinfo_rtp->ai_next) {
        if (curinfo_rtp->ai_protocol && curinfo_rtp->ai_protocol != transport) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO3, NULL, "eXosip: Skipping protocol %d\n", curinfo_rtp->ai_protocol));
          continue;
        }

        sock = (int) socket (curinfo_rtp->ai_family, curinfo_rtp->ai_socktype, curinfo_rtp->ai_protocol);
        if (sock < 0) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot create socket!\n"));
          continue;
        }

        if (curinfo_rtp->ai_family == AF_INET6) {
#ifdef IPV6_V6ONLY
          if (setsockopt_ipv6only (sock)) {
            _eXosip_closesocket (sock);
            sock = -1;
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot set socket option!\n"));
            continue;
          }
#endif /* IPV6_V6ONLY */
        }

        res1 = bind (sock, curinfo_rtp->ai_addr, (socklen_t)curinfo_rtp->ai_addrlen);
        if (res1 < 0) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "eXosip: Cannot bind socket node: 0.0.0.0 family:%d\n", curinfo_rtp->ai_family));
          _eXosip_closesocket (sock);
          sock = -1;
          continue;
        }
        break;
      }

      _eXosip_freeaddrinfo (addrinfo_rtp);

      if (sock == -1) {
        _eXosip_freeaddrinfo (addrinfo_rtcp);
        continue;
      }

      _eXosip_closesocket (sock);
      sock = -1;
      for (curinfo_rtcp = addrinfo_rtcp; curinfo_rtcp; curinfo_rtcp = curinfo_rtcp->ai_next) {
        if (curinfo_rtcp->ai_protocol && curinfo_rtcp->ai_protocol != transport) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO3, NULL, "eXosip: Skipping protocol %d\n", curinfo_rtcp->ai_protocol));
          continue;
        }

        sock = (int) socket (curinfo_rtcp->ai_family, curinfo_rtcp->ai_socktype, curinfo_rtcp->ai_protocol);
        if (sock < 0) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot create socket!\n"));
          continue;
        }

        if (curinfo_rtcp->ai_family == AF_INET6) {
#ifdef IPV6_V6ONLY
          if (setsockopt_ipv6only (sock)) {
            _eXosip_closesocket (sock);
            sock = -1;
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot set socket option!\n"));
            continue;
          }
#endif /* IPV6_V6ONLY */
        }


        res1 = bind (sock, curinfo_rtcp->ai_addr, (socklen_t)curinfo_rtcp->ai_addrlen);
        if (res1 < 0) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "eXosip: Cannot bind socket node: 0.0.0.0 family:%d\n", curinfo_rtp->ai_family));
          _eXosip_closesocket (sock);
          sock = -1;
          continue;
        }
        break;
      }

      _eXosip_freeaddrinfo (addrinfo_rtcp);

      /* the pair must be free */
      if (sock == -1)
        continue;

      _eXosip_closesocket (sock);
      sock = -1;
      return free_port + count * 2;
    }
  }

  for (count = 0; count < 8; count++) {
    /* just get a free port */
    if (excontext->ipv6_enable == 0)
      res1 = _eXosip_get_addrinfo (excontext, &addrinfo_rtp, "0.0.0.0", 0, transport);
    else
      res1 = _eXosip_get_addrinfo (excontext, &addrinfo_rtp, "::", 0, transport);

    if (res1)
      return res1;

    sock = -1;
    for (curinfo_rtp = addrinfo_rtp; curinfo_rtp; curinfo_rtp = curinfo_rtp->ai_next) {
      socklen_t len;
      struct sockaddr_storage ai_addr;

      if (curinfo_rtp->ai_protocol && curinfo_rtp->ai_protocol != transport) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO3, NULL, "eXosip: Skipping protocol %d\n", curinfo_rtp->ai_protocol));
        continue;
      }

      sock = (int) socket (curinfo_rtp->ai_family, curinfo_rtp->ai_socktype, curinfo_rtp->ai_protocol);
      if (sock < 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot create socket!\n"));
        continue;
      }

      if (curinfo_rtp->ai_family == AF_INET6) {
  #ifdef IPV6_V6ONLY
        if (setsockopt_ipv6only (sock)) {
          _eXosip_closesocket (sock);
          sock = -1;
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot set socket option!\n"));
          continue;
        }
  #endif /* IPV6_V6ONLY */
      }

      res1 = bind (sock, curinfo_rtp->ai_addr, (socklen_t)curinfo_rtp->ai_addrlen);
      if (res1 < 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "eXosip: Cannot bind socket node: 0.0.0.0 family:%d\n", curinfo_rtp->ai_family));
        _eXosip_closesocket (sock);
        sock = -1;
        continue;
      }

      len = sizeof (ai_addr);
      res1 = getsockname (sock, (struct sockaddr *) &ai_addr, &len);
      if (res1 != 0) {
        _eXosip_closesocket (sock);
        sock = -1;
        continue;
      }

      _eXosip_closesocket (sock);
      sock = -1;

      {
        int port_found;
        if (curinfo_rtp->ai_family == AF_INET)
          port_found = ntohs (((struct sockaddr_in *) &ai_addr)->sin_port);
        else
          port_found = ntohs (((struct sockaddr_in6 *) &ai_addr)->sin6_port);
        if (port_found%2==0) /* even port for RTP */ {
          _eXosip_freeaddrinfo (addrinfo_rtp);
          return port_found;
        }
        if (count==7) {
          /* return odd number anyway... */
          return port_found;
        }
      }
    }

    _eXosip_freeaddrinfo (addrinfo_rtp);

    if (sock != -1) {
      _eXosip_closesocket (sock);
      sock = -1;
    }
  }

  return OSIP_UNDEFINED_ERROR;
}
#endif

int
eXosip_listen_addr (struct eXosip_t *excontext, int transport, const char *addr, int port, int family, int secure)
{
  int i = -1;

  if (excontext->eXtl_transport.enabled > 0) {
    /* already set */
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: already listening somewhere\n"));
    return OSIP_WRONG_STATE;
  }

  if (transport == IPPROTO_UDP && secure == 0)
    eXosip_transport_udp_init (excontext);
  else if (transport == IPPROTO_TCP && secure == 0)
    eXosip_transport_tcp_init (excontext);
#ifdef HAVE_OPENSSL_SSL_H
#if !(OPENSSL_VERSION_NUMBER < 0x00908000L)
  else if (transport == IPPROTO_UDP)
    eXosip_transport_dtls_init (excontext);
#endif
  else if (transport == IPPROTO_TCP)
    eXosip_transport_tls_init (excontext);
#endif
  else
    return OSIP_BADPARAMETER;

  if (excontext->eXtl_transport.tl_init != NULL)
    excontext->eXtl_transport.tl_init (excontext);

  excontext->eXtl_transport.proto_family = family;
  excontext->eXtl_transport.proto_port = port;
  if (addr != NULL)
    snprintf (excontext->eXtl_transport.proto_ifs, sizeof (excontext->eXtl_transport.proto_ifs), "%s", addr);

#ifdef	AF_INET6
  if (family == AF_INET6 && !addr)
    snprintf (excontext->eXtl_transport.proto_ifs, sizeof (excontext->eXtl_transport.proto_ifs), "::0");
#endif

  i = excontext->eXtl_transport.tl_open (excontext);

  if (i != 0) {
    if (excontext->eXtl_transport.tl_free != NULL)
      excontext->eXtl_transport.tl_free (excontext);
    excontext->eXtl_transport.enabled=0;
    return i;
  }

  if (transport == IPPROTO_UDP && secure == 0)
    snprintf (excontext->transport, sizeof (excontext->transport), "%s", "UDP");
  else if (transport == IPPROTO_TCP && secure == 0)
    snprintf (excontext->transport, sizeof (excontext->transport), "%s", "TCP");
  else if (transport == IPPROTO_UDP)
    snprintf (excontext->transport, sizeof (excontext->transport), "%s", "DTLS-UDP");
  else if (transport == IPPROTO_TCP)
    snprintf (excontext->transport, sizeof (excontext->transport), "%s", "TLS");

#ifndef OSIP_MONOTHREAD
  if (excontext->j_thread == NULL) {
    excontext->j_thread = (void *) osip_thread_create (20000, _eXosip_thread, excontext);
    if (excontext->j_thread == NULL) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot start thread!\n"));
      return OSIP_UNDEFINED_ERROR;
    }
  }
#endif

  return OSIP_SUCCESS;
}

int
eXosip_reset_transports (struct eXosip_t *excontext)
{
  int i = OSIP_WRONG_STATE;

  if (excontext->eXtl_transport.tl_reset)
    i = excontext->eXtl_transport.tl_reset (excontext);
  return i;
}

struct eXosip_t *
eXosip_malloc (void)
{
  struct eXosip_t *ptr = (struct eXosip_t *) osip_malloc (sizeof (eXosip_t));

  if (ptr) {
    memset (ptr, 0, sizeof (eXosip_t));
    ptr->j_stop_ua = -1;
  }
  return ptr;
}

int
eXosip_init (struct eXosip_t *excontext)
{
  osip_t *osip;
  int i;

  memset (excontext, 0, sizeof (eXosip_t));

  _eXosip_counters_init(&excontext->average_transactions, 0, 0);
  _eXosip_counters_init(&excontext->average_registrations, 0, 0);
  _eXosip_counters_init(&excontext->average_calls, 0, 0);
  _eXosip_counters_init(&excontext->average_publications, 0, 0);
  _eXosip_counters_init(&excontext->average_subscriptions, 0, 0);
  _eXosip_counters_init(&excontext->average_insubscriptions, 0, 0);

  excontext->max_message_to_read=1;
  excontext->dscp = 0x1A;

  snprintf (excontext->ipv4_for_gateway, 256, "%s", "217.12.3.11");
  snprintf (excontext->ipv6_for_gateway, 256, "%s", "2001:638:500:101:2e0:81ff:fe24:37c6");

#ifdef WIN32
  /* Initializing windows socket library */
  {
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD (2, 0);
    i = WSAStartup (wVersionRequested, &wsaData);
    if (i != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "eXosip: Unable to initialize WINSOCK, reason: %d\n", i));
    }
  }
#endif

  excontext->user_agent = osip_strdup ("eXosip/" EXOSIP_VERSION);
  if (excontext->user_agent == NULL)
    return OSIP_NOMEM;

  excontext->j_calls = NULL;
  excontext->j_stop_ua = 0;
#ifndef OSIP_MONOTHREAD
  excontext->j_thread = NULL;
#endif
  i = osip_list_init (&excontext->j_transactions);
  excontext->j_reg = NULL;

#ifndef OSIP_MONOTHREAD
#if !defined (_WIN32_WCE)
  excontext->j_cond = (struct osip_cond *) osip_cond_init ();
  if (excontext->j_cond == NULL) {
    osip_free (excontext->user_agent);
    excontext->user_agent = NULL;
    return OSIP_NOMEM;
  }
#endif

  excontext->j_mutexlock = (struct osip_mutex *) osip_mutex_init ();
  if (excontext->j_mutexlock == NULL) {
    osip_free (excontext->user_agent);
    excontext->user_agent = NULL;
#if !defined (_WIN32_WCE)
    osip_cond_destroy ((struct osip_cond *) excontext->j_cond);
    excontext->j_cond = NULL;
#endif
    return OSIP_NOMEM;
  }
#endif

  i = osip_init (&osip);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot initialize osip!\n"));
    return i;
  }

  osip_set_application_context (osip, &excontext);

  _eXosip_set_callbacks (osip);

  excontext->j_osip = osip;

#ifndef OSIP_MONOTHREAD
  /* open a TCP socket to wake up the application when needed. */
  excontext->j_socketctl = jpipe ();
  if (excontext->j_socketctl == NULL)
    return OSIP_UNDEFINED_ERROR;

  excontext->j_socketctl_event = jpipe ();
  if (excontext->j_socketctl_event == NULL)
    return OSIP_UNDEFINED_ERROR;
#endif

  /* To be changed in osip! */
  excontext->j_events = (osip_fifo_t *) osip_malloc (sizeof (osip_fifo_t));
  if (excontext->j_events == NULL)
    return OSIP_NOMEM;
  osip_fifo_init (excontext->j_events);

  excontext->use_rport = 1;
  excontext->remove_prerouteset = 1;
  excontext->dns_capabilities = 2;
  excontext->enable_dns_cache = 1;
  excontext->ka_interval = 17000;
  snprintf(excontext->ka_crlf, sizeof(excontext->ka_crlf), "\r\n\r\n");
  excontext->ka_options = 0;
  excontext->autoanswer_bye = 1;
  excontext->auto_masquerade_contact = 1;
  excontext->masquerade_via=0;
  excontext->use_ephemeral_port=1;

  return OSIP_SUCCESS;
}


int
eXosip_execute (struct eXosip_t *excontext)
{
  struct timeval lower_tv;
  int i;

#ifndef OSIP_MONOTHREAD
  if (excontext->max_read_timeout>0) {
    lower_tv.tv_sec=0;
    lower_tv.tv_usec=excontext->max_read_timeout;
  } else {
    osip_timers_gettimeout (excontext->j_osip, &lower_tv);
    if (lower_tv.tv_sec > 10) {
      eXosip_reg_t *jr;
      time_t now;
      
      osip_compensatetime ();
      
      now = osip_getsystemtime (NULL);
      
      lower_tv.tv_sec = 10;
      
      eXosip_lock (excontext);
      for (jr = excontext->j_reg; jr != NULL; jr = jr->next) {
	if (jr->r_id >= 1 && jr->r_last_tr != NULL) {
	  if (jr->r_reg_period == 0) {
	    /* skip refresh! */
	  }
	  else if (now - jr->r_last_tr->birth_time > jr->r_reg_period - (jr->r_reg_period / 10)) {
	    /* automatic refresh at "timeout - 10%" */
	    lower_tv.tv_sec = 1;
	  }
	}
      }
      eXosip_unlock (excontext);
      
      if (lower_tv.tv_sec == 1) {
	OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: Reseting timer to 1s before waking up!\n"));
      }
      else {
	OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: Reseting timer to 10s before waking up!\n"));
      }
    }
    else {
      /* add a small amount of time on windows to avoid waking up too early. (probably a bad time precision) */
      if (lower_tv.tv_usec < 990000)
	lower_tv.tv_usec += 10000;        /* add 10ms */
      else {
	lower_tv.tv_usec = 10000; /* add 10ms */
	lower_tv.tv_sec++;
      }
    }
#if 0
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: timer sec:%i usec:%i!\n", lower_tv.tv_sec, lower_tv.tv_usec));
#endif
  }
#else
  lower_tv.tv_sec = 0;
  lower_tv.tv_usec = 0;
#endif
  i = _eXosip_read_message (excontext, excontext->max_message_to_read, (int) lower_tv.tv_sec, (int) lower_tv.tv_usec);

  if (i == -2000) {
    return -2000;
  }

  eXosip_lock (excontext);
  osip_timers_ict_execute (excontext->j_osip);
  osip_timers_nict_execute (excontext->j_osip);
  osip_timers_ist_execute (excontext->j_osip);
  osip_timers_nist_execute (excontext->j_osip);

  osip_nist_execute (excontext->j_osip);
  osip_nict_execute (excontext->j_osip);
  osip_ist_execute (excontext->j_osip);
  osip_ict_execute (excontext->j_osip);

  /* free all Calls that are in the TERMINATED STATE? */
  _eXosip_release_terminated_calls (excontext);
  _eXosip_release_terminated_registrations (excontext);
#ifndef MINISIZE
  _eXosip_release_terminated_publications (excontext);
  _eXosip_release_terminated_subscriptions (excontext);
  _eXosip_release_terminated_in_subscriptions (excontext);
#endif

  if (excontext->cbsipWakeLock!=NULL && excontext->outgoing_wake_lock_state==0) {
    int count = osip_list_size(&excontext->j_osip->osip_ict_transactions);
    count+=osip_list_size(&excontext->j_osip->osip_nict_transactions);
    if (count>0) {
      excontext->cbsipWakeLock(3);
      excontext->outgoing_wake_lock_state++;
    }
  } else if (excontext->cbsipWakeLock!=NULL && excontext->outgoing_wake_lock_state>0) {
    int count = osip_list_size(&excontext->j_osip->osip_ict_transactions);
    count+=osip_list_size(&excontext->j_osip->osip_nict_transactions);
    if (count==0) {
      excontext->cbsipWakeLock(2);
      excontext->outgoing_wake_lock_state=0;
    }
  }


  _eXosip_keep_alive (excontext);

  eXosip_unlock (excontext);

  return OSIP_SUCCESS;
}

int
eXosip_set_option (struct eXosip_t *excontext, int opt, const void *value)
{
  int val;
  char *tmp;

  switch (opt) {
  case EXOSIP_OPT_ADD_ACCOUNT_INFO:
    {
      struct eXosip_account_info *ainfo;
      int i;

      ainfo = (struct eXosip_account_info *) value;
      if (ainfo == NULL || ainfo->proxy[0] == '\0') {
        return OSIP_BADPARAMETER;
      }
      for (i = 0; i < MAX_EXOSIP_ACCOUNT_INFO; i++) {
        if (excontext->account_entries[i].proxy[0] != '\0' && 0 == osip_strcasecmp (excontext->account_entries[i].proxy, ainfo->proxy)) {
          /* update ainfo */
          if (ainfo->nat_ip[0] != '\0') {
            if (0 == osip_strcasecmp (excontext->account_entries[i].nat_ip, ainfo->nat_ip) && excontext->account_entries[i].nat_port == ainfo->nat_port)
              return OSIP_SUCCESS+1; /* NOT MODIFIED */
            snprintf (excontext->account_entries[i].nat_ip, sizeof (excontext->account_entries[i].nat_ip), "%s", ainfo->nat_ip);
            excontext->account_entries[i].nat_port = ainfo->nat_port;
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "eXosip option set: account info updated:%s -> %s:%i\n", ainfo->proxy, ainfo->nat_ip, ainfo->nat_port));
          }
          else {
            excontext->account_entries[i].proxy[0] = '\0';
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip option set: account info deleted :%s\n", ainfo->proxy));
          }
          return OSIP_SUCCESS;
        }
      }
      if (ainfo->nat_ip[0] == '\0') {
        return OSIP_BADPARAMETER;
      }
      /* not found case: */
      for (i = 0; i < MAX_EXOSIP_ACCOUNT_INFO; i++) {
        if (excontext->account_entries[i].proxy[0] == '\0') {
          /* add ainfo */
          snprintf (excontext->account_entries[i].proxy, sizeof (ainfo->proxy), "%s", ainfo->proxy);
          snprintf (excontext->account_entries[i].nat_ip, sizeof (ainfo->nat_ip), "%s", ainfo->nat_ip);
          excontext->account_entries[i].nat_port = ainfo->nat_port;
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "eXosip option set: account info added:%s -> %s:%i\n", ainfo->proxy, ainfo->nat_ip, ainfo->nat_port));
          return OSIP_SUCCESS;
        }
      }
      return OSIP_UNDEFINED_ERROR;
    }
    break;
  case EXOSIP_OPT_ADD_DNS_CACHE:
    {
      struct eXosip_dns_cache *entry;
      int i;

      entry = (struct eXosip_dns_cache *) value;
      if (entry == NULL || entry->host[0] == '\0') {
        return OSIP_BADPARAMETER;
      }
      for (i = 0; i < MAX_EXOSIP_DNS_ENTRY; i++) {
        if (excontext->dns_entries[i].host[0] != '\0' && 0 == osip_strcasecmp (excontext->dns_entries[i].host, entry->host)) {
          /* update entry */
          if (entry->ip[0] != '\0') {
            snprintf (excontext->dns_entries[i].ip, sizeof (excontext->dns_entries[i].ip), "%s", entry->ip);
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "eXosip option set: dns cache updated:%s -> %s\n", entry->host, entry->ip));
          }
          else {
            /* return previously added cache */
            snprintf (entry->ip, sizeof (entry->ip), "%s", excontext->dns_entries[i].ip);
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip option set: dns cache returned:%s ->%s\n", entry->host, entry->ip));
          }
          return OSIP_SUCCESS;
        }
      }
      if (entry->ip[0] == '\0') {
        char ipbuf[INET6_ADDRSTRLEN];
        struct __eXosip_sockaddr addr;
        struct addrinfo *addrinfo;

        /* create the A record */
        i = _eXosip_get_addrinfo (excontext, &addrinfo, entry->host, 0, IPPROTO_UDP);
        if (i != 0)
          return OSIP_BADPARAMETER;

        memcpy (&addr, addrinfo->ai_addr, addrinfo->ai_addrlen);

        _eXosip_freeaddrinfo (addrinfo);
        switch (((struct sockaddr *) &addr)->sa_family) {
        case AF_INET:
          inet_ntop (((struct sockaddr *) &addr)->sa_family, &(((struct sockaddr_in *) &addr)->sin_addr), ipbuf, sizeof (ipbuf));
          break;
        case AF_INET6:
          inet_ntop (((struct sockaddr *) &addr)->sa_family, &(((struct sockaddr_in6 *) &addr)->sin6_addr), ipbuf, sizeof (ipbuf));
          break;
        default:
          return OSIP_BADPARAMETER;
        }

        if (osip_strcasecmp (ipbuf, entry->host) == 0)
          return OSIP_SUCCESS;
        snprintf (entry->ip, sizeof (entry->ip), "%s", ipbuf);
      }
      /* not found case: */
      for (i = 0; i < MAX_EXOSIP_DNS_ENTRY; i++) {
        if (excontext->dns_entries[i].host[0] == '\0') {
          /* add entry */
          snprintf (excontext->dns_entries[i].host, sizeof (entry->host), "%s", entry->host);
          snprintf (excontext->dns_entries[i].ip, sizeof (entry->ip), "%s", entry->ip);
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip option set: dns cache added:%s -> %s\n", entry->host, entry->ip));
          return OSIP_SUCCESS;
        }
      }
      return OSIP_UNDEFINED_ERROR;
    }
    break;
  case EXOSIP_OPT_DELETE_DNS_CACHE:
    {
      struct eXosip_dns_cache *entry;
      int i;

      entry = (struct eXosip_dns_cache *) value;
      if (entry == NULL || entry->host[0] == '\0') {
        return OSIP_BADPARAMETER;
      }
      for (i = 0; i < MAX_EXOSIP_DNS_ENTRY; i++) {
        if (excontext->dns_entries[i].host[0] != '\0' && 0 == osip_strcasecmp (excontext->dns_entries[i].host, entry->host)) {
          excontext->dns_entries[i].host[0] = '\0';
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip option set: dns cache deleted :%s\n", entry->host));
          return OSIP_SUCCESS;
        }
      }
      return OSIP_UNDEFINED_ERROR;
    }
    break;
  case EXOSIP_OPT_UDP_KEEP_ALIVE:
    val = *((int *) value);
    excontext->ka_interval = val;        /* value in ms */
    break;
#ifdef ENABLE_KEEP_ALIVE_OPTIONS_METHOD
  case EXOSIP_OPT_KEEP_ALIVE_OPTIONS_METHOD:
    val = *((int *) value);
    excontext->ka_options = val;        /* value 0 or 1 */
    break;
#endif
  case EXOSIP_OPT_AUTO_MASQUERADE_CONTACT:
    val = *((int *) value);
    excontext->auto_masquerade_contact = val;        /* 1 to learn port */
    break;

  case EXOSIP_OPT_USE_RPORT:
    val = *((int *) value);
    excontext->use_rport = val; /* 0 to disable (for broken NAT only?) */
    break;

  case EXOSIP_OPT_SET_IPV4_FOR_GATEWAY:
    tmp = (char *) value;
    memset (excontext->ipv4_for_gateway, '\0', sizeof (excontext->ipv4_for_gateway));
    if (tmp != NULL && tmp[0] != '\0')
      osip_strncpy (excontext->ipv4_for_gateway, tmp, sizeof (excontext->ipv4_for_gateway) - 1);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "eXosip option set: ipv4_for_gateway:%s!\n", excontext->ipv4_for_gateway));
    break;
#ifndef MINISIZE
  case EXOSIP_OPT_SET_IPV6_FOR_GATEWAY:
    tmp = (char *) value;
    memset (excontext->ipv6_for_gateway, '\0', sizeof (excontext->ipv6_for_gateway));
    if (tmp != NULL && tmp[0] != '\0')
      osip_strncpy (excontext->ipv6_for_gateway, tmp, sizeof (excontext->ipv6_for_gateway) - 1);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "eXosip option set: ipv6_for_gateway:%s!\n", excontext->ipv6_for_gateway));
    break;
#endif
  case EXOSIP_OPT_DNS_CAPABILITIES:    /*EXOSIP_OPT_SRV_WITH_NAPTR: */
    val = *((int *) value);
    /* 0: A request, 1: SRV support, 2: NAPTR+SRV support */
    excontext->dns_capabilities = val;
    break;
  case EXOSIP_OPT_REMOVE_PREROUTESET:
    val = *((int *) value);
    /* 0: keep pre-route set in initial INVITE/SUBSCRIBE/REFER, 1: remove pre-route set */
      excontext->remove_prerouteset = val;
    break;
  case EXOSIP_OPT_SET_SIP_INSTANCE:
    tmp = (char *) value;
    memset (excontext->sip_instance, '\0', sizeof (excontext->sip_instance));
    if (tmp != NULL && tmp[0] != '\0')
      osip_strncpy (excontext->sip_instance, tmp, sizeof (excontext->sip_instance) - 1);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "eXosip option set: +sip.instance:%s!\n", excontext->sip_instance));
    break;
  case EXOSIP_OPT_SET_DEFAULT_CONTACT_DISPLAYNAME:
    tmp = (char *) value;
    memset (excontext->default_contact_displayname, '\0', sizeof (excontext->default_contact_displayname));
    if (tmp != NULL && tmp[0] != '\0')
      osip_strncpy (excontext->default_contact_displayname, tmp, sizeof (excontext->default_contact_displayname) - 1);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "eXosip option set: default_contact_displayname:%s!\n", excontext->default_contact_displayname));
    break;
  case EXOSIP_OPT_SET_DSCP:
    val = *((int *) value);
    /* 0x1A by default */
    excontext->dscp = val;
    break;
  case EXOSIP_OPT_REGISTER_WITH_DATE:
    val = *((int *) value);
    excontext->register_with_date = val;
    break;
  case EXOSIP_OPT_SET_HEADER_USER_AGENT:
    {
      const char *user_agent = (const char *) value;

      osip_free (excontext->user_agent);
      if (user_agent == NULL || user_agent[0] == '\0')
        excontext->user_agent = osip_strdup ("eXosip/" EXOSIP_VERSION);
      else
        excontext->user_agent = osip_strdup (user_agent);
    }
    break;
  case EXOSIP_OPT_ENABLE_DNS_CACHE:
    val = *((int *) value);
    excontext->enable_dns_cache = val;
    break;
  case EXOSIP_OPT_SET_TLS_VERIFY_CERTIFICATE:
    val = *((int *) value);
    eXosip_tls_verify_certificate (excontext, val);
    break;
  case EXOSIP_OPT_SET_TLS_CERTIFICATES_INFO:
    {
      eXosip_tls_ctx_t *tlsval = (eXosip_tls_ctx_t *) value;

      eXosip_set_tls_ctx (excontext, tlsval);
    }
    break;
  case EXOSIP_OPT_SET_TLS_CLIENT_CERTIFICATE_NAME:
    eXosip_tls_use_client_certificate (excontext, (const char *) value);
    break;
  case EXOSIP_OPT_SET_TLS_SERVER_CERTIFICATE_NAME:
    eXosip_tls_use_server_certificate (excontext, (const char *) value);
    break;
  case EXOSIP_OPT_SET_TSC_SERVER:
#ifdef TSC_SUPPORT
    excontext->tunnel_handle = (void *) value;
#endif
    break;
  case EXOSIP_OPT_ENABLE_AUTOANSWERBYE:
    val = *((int *) value);
    excontext->autoanswer_bye = val;
    break;
  case EXOSIP_OPT_ENABLE_IPV6:
    val = *((int *) value);
    excontext->ipv6_enable = val;
    break;
  case EXOSIP_OPT_ENABLE_REUSE_TCP_PORT:
    val = *((int *) value);
    excontext->reuse_tcp_port = val;
    break;
  case EXOSIP_OPT_ENABLE_USE_EPHEMERAL_PORT:
    val = *((int *) value);
    excontext->use_ephemeral_port = val;
    break;
  case EXOSIP_OPT_SET_CALLBACK_WAKELOCK:
    excontext->cbsipWakeLock = (CbSipWakeLock) value;
    break;
  case EXOSIP_OPT_ENABLE_OUTBOUND:
    val = *((int *) value);
    excontext->enable_outbound = val;
    break;
  case EXOSIP_OPT_SET_OC_LOCAL_ADDRESS:
    tmp = (char *) value;
    memset (excontext->oc_local_address, '\0', sizeof (excontext->oc_local_address));
    if (tmp != NULL && tmp[0] != '\0')
      osip_strncpy (excontext->oc_local_address, tmp, sizeof (excontext->oc_local_address) - 1);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "eXosip option set: oc_local_address:%s!\n", excontext->oc_local_address));
    break;
  case EXOSIP_OPT_SET_OC_PORT_RANGE:
    {
      int *range = ((int *) value);
      excontext->oc_local_port_range[0] = range[0];
      excontext->oc_local_port_range[1] = range[1];
      break;
    }
  case EXOSIP_OPT_SET_MAX_MESSAGE_TO_READ:
    {
      excontext->max_message_to_read = *((int *) value);
      break;
    }
  case EXOSIP_OPT_SET_MAX_READ_TIMEOUT:
    {
      excontext->max_read_timeout= *((long int *) value);
      break;
    }
  case EXOSIP_OPT_GET_STATISTICS:
    {
      struct eXosip_stats *stats = (struct eXosip_stats *) value;
      struct timeval now;
      osip_gettimeofday(&now, NULL);
      _eXosip_counters_update(&excontext->average_transactions, 0, &now);
      _eXosip_counters_update(&excontext->average_registrations, 0, &now);
      _eXosip_counters_update(&excontext->average_calls, 0, &now);
      _eXosip_counters_update(&excontext->average_publications, 0, &now);
      _eXosip_counters_update(&excontext->average_subscriptions, 0, &now);
      _eXosip_counters_update(&excontext->average_insubscriptions, 0, &now);
      excontext->statistics.average_transactions=excontext->average_transactions.current_average;
      excontext->statistics.average_registrations=excontext->average_registrations.current_average;
      excontext->statistics.average_calls=excontext->average_calls.current_average;
      excontext->statistics.average_publications=excontext->average_publications.current_average;
      excontext->statistics.average_subscriptions=excontext->average_subscriptions.current_average;
      excontext->statistics.average_insubscriptions=excontext->average_insubscriptions.current_average;
      memcpy(stats, &excontext->statistics, sizeof(struct eXosip_stats));
    }
  default:
    return OSIP_BADPARAMETER;
  }
  return OSIP_SUCCESS;
}

static void
_eXosip_keep_alive (struct eXosip_t *excontext)
{
  struct timeval now;

  osip_gettimeofday (&now, NULL);

  /* 
  the stack is waking up every 10 seconds when no action is required. So
  this method will be called at a maximum interval of 10 seconds and minimum
  2 seconds.
  The objective is to detect broken connections that do not fire any error
  within an acceptable timeframe.
  */
  if (excontext->cc_timer.tv_sec == 0 && excontext->cc_timer.tv_usec == 0) {
    /* first init */
    osip_gettimeofday (&excontext->cc_timer, NULL);
    add_gettimeofday (&excontext->cc_timer, 2);
  }

  if (osip_timercmp (&now, &excontext->cc_timer, >=)) {
    /* reset timer */
    osip_gettimeofday (&excontext->cc_timer, NULL);
    add_gettimeofday (&excontext->cc_timer, 2);

    if (excontext->eXtl_transport.tl_check_connection != NULL)
      excontext->eXtl_transport.tl_check_connection (excontext);
  }

  if (excontext->ka_timer.tv_sec == 0 && excontext->ka_timer.tv_usec == 0) {
    /* first init */
    osip_gettimeofday (&excontext->ka_timer, NULL);
    add_gettimeofday (&excontext->ka_timer, excontext->ka_interval);
  }

  if (osip_timercmp (&now, &excontext->ka_timer, <)) {
    return;                     /* not yet time */
  }

  /* reset timer */
  osip_gettimeofday (&excontext->ka_timer, NULL);
  add_gettimeofday (&excontext->ka_timer, excontext->ka_interval);

  if (excontext->eXtl_transport.tl_keepalive != NULL)
    excontext->eXtl_transport.tl_keepalive (excontext);
}

#ifndef OSIP_MONOTHREAD
void *
_eXosip_thread (void *arg)
{
  struct eXosip_t *excontext = (struct eXosip_t *) arg;
  int i;

  while (excontext->j_stop_ua == 0) {
    i = eXosip_execute (excontext);
    if (i == -2000)
      osip_thread_exit ();
  }
  osip_thread_exit ();
  return NULL;
}

#endif

#ifndef MINISIZE

void _eXosip_counters_init(struct eXosip_counters *bw_stats, int period, int interval) {
  bw_stats->period=period;
  bw_stats->interval=interval;
  if (bw_stats->period<=0)
    bw_stats->period=EXOSIP_STATS_PERIOD;
  if (bw_stats->interval<=0)
    bw_stats->interval=EXOSIP_STATS_INTERVAL;

  bw_stats->num_entries = (bw_stats->period/bw_stats->interval);
  bw_stats->values = (unsigned short*)osip_malloc(sizeof(unsigned short)*bw_stats->num_entries);
  memset(bw_stats->values, 0, sizeof(unsigned short)*bw_stats->num_entries);
  bw_stats->times = (struct timeval*)osip_malloc(sizeof(struct timeval)*bw_stats->num_entries);
  memset(bw_stats->times, 0, sizeof(struct timeval)*bw_stats->num_entries);
}

void _eXosip_counters_free(struct eXosip_counters *bw_stats) {
  osip_free(bw_stats->values);
  osip_free(bw_stats->times);
  bw_stats->values=NULL;
  bw_stats->times=NULL;
  bw_stats->total_values=0;
  bw_stats->index_last=0;
}

static float compute_average(struct timeval *orig, unsigned int values){
  struct timeval current;
  float time;
  if (values==0) return 0;
  osip_gettimeofday(&current,NULL);
  time=(float)(current.tv_sec - orig->tv_sec)/3600; /* calculate num/hour */
  if (time==0) return 0;
  return ((float)values)/(time+0.000001f);
}

void _eXosip_counters_update(struct eXosip_counters *bw_stats, int nvalues, struct timeval *now) {
  unsigned long interval;
  unsigned long last_interval;
  if (bw_stats->values==NULL)
    _eXosip_counters_init(bw_stats, 0, 0);

  interval = (now->tv_sec-bw_stats->times[0].tv_sec);
  if (bw_stats->index_last>0 && interval<=bw_stats->interval) {
    bw_stats->values[0]+=nvalues;
    bw_stats->total_values+=nvalues;
    bw_stats->current_average = compute_average(&bw_stats->times[bw_stats->index_last-1], bw_stats->total_values);
    return;
  }

  last_interval=0;
  while (bw_stats->index_last>0) {
    last_interval = (now->tv_sec-bw_stats->times[bw_stats->index_last-1].tv_sec);
    if (last_interval<bw_stats->period && bw_stats->index_last<bw_stats->num_entries)
      break;
    bw_stats->total_values-=bw_stats->values[bw_stats->index_last-1];
    bw_stats->index_last--;
  }

  if (nvalues>0) {
    bw_stats->total_values+=nvalues;
    memmove(((unsigned char*)bw_stats->values)+sizeof(unsigned short), bw_stats->values, sizeof(unsigned short)*(bw_stats->index_last));
    memmove(((unsigned char*)bw_stats->times)+sizeof(struct timeval), bw_stats->times, sizeof(struct timeval)*(bw_stats->index_last));
    bw_stats->values[0]=nvalues;
    bw_stats->times[0].tv_sec = now->tv_sec;
    bw_stats->times[0].tv_usec = now->tv_usec;
    bw_stats->index_last++;
  }

  if (bw_stats->index_last>0) {
    bw_stats->current_average = compute_average(&bw_stats->times[bw_stats->index_last-1], bw_stats->total_values);
  } else {
    bw_stats->current_average=0;
  }
}

#endif
