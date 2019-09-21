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
#include "eXtransport.h"

#if defined(_MSC_VER) && defined(WIN32) && !defined(_WIN32_WCE)
#define HAVE_MSTCPIP_H
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
#undef HAVE_MSTCPIP_H
#endif
#endif

#ifdef HAVE_MSTCPIP_H
#include <Mstcpip.h>
#else
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#endif

#if !defined(_WIN32_WCE)
#include <errno.h>
#endif

#if defined(HAVE_NETINET_TCP_H)
#include <netinet/tcp.h>
#endif

#if defined(_WIN32_WCE) || defined(WIN32)
#define strerror(X) "-1"
#define ex_errno WSAGetLastError()
#define is_wouldblock_error(r) ((r)==WSAEINTR||(r)==WSAEWOULDBLOCK)
#define is_connreset_error(r) ((r)==WSAECONNRESET || (r)==WSAECONNABORTED || (r)==WSAETIMEDOUT || (r)==WSAENETRESET || (r)==WSAENOTCONN)
#else
#define ex_errno errno
#endif
#ifndef is_wouldblock_error
#define is_wouldblock_error(r) ((r)==EINTR||(r)==EWOULDBLOCK||(r)==EAGAIN)
#define is_connreset_error(r) ((r)==ECONNRESET || (r)==ECONNABORTED || (r)==ETIMEDOUT || (r)==ENETRESET || (r)==ENOTCONN)
#endif

#ifdef __APPLE_CC__
#include "TargetConditionals.h"
#endif

#if TARGET_OS_IPHONE
#include <CoreFoundation/CFStream.h>
#include <CFNetwork/CFSocketStream.h>
#define MULTITASKING_ENABLED
#endif

/* persistent connection */
struct _tcp_stream {
  int socket;
  struct sockaddr ai_addr;
  socklen_t ai_addrlen;
  char remote_ip[65];
  int remote_port;
  char *buf;                    /* recv buffer */
  size_t bufsize;               /* allocated size of buf */
  size_t buflen;                /* current length of buf */
  char *sendbuf;                /* send buffer */
  size_t sendbufsize;
  size_t sendbuflen;
#ifdef MULTITASKING_ENABLED
  CFReadStreamRef readStream;
  CFWriteStreamRef writeStream;
#endif
  char natted_ip[65];
  int natted_port;
  int ephemeral_port;
  int invalid;
  int is_server;
  time_t tcp_max_timeout;
  time_t tcp_inprogress_max_timeout;
  char reg_call_id[64];
};

#ifndef SOCKET_TIMEOUT
/* when stream has sequence error: */
/* having SOCKET_TIMEOUT > 0 helps the system to recover */
#define SOCKET_TIMEOUT 0
#endif

#ifndef EXOSIP_MAX_SOCKETS
#define EXOSIP_MAX_SOCKETS 200
#endif

static int _tcp_tl_send_sockinfo (struct _tcp_stream *sockinfo, const char *msg, int msglen);

struct eXtltcp {
  int tcp_socket;
  struct sockaddr_storage ai_addr;
  int ai_addr_len;
  
  struct _tcp_stream socket_tab[EXOSIP_MAX_SOCKETS];
};

static int
tcp_tl_init (struct eXosip_t *excontext)
{
  struct eXtltcp *reserved = (struct eXtltcp *) osip_malloc (sizeof (struct eXtltcp));
  
  if (reserved == NULL)
    return OSIP_NOMEM;
  reserved->tcp_socket = 0;
  memset (&reserved->ai_addr, 0, sizeof (struct sockaddr_storage));
  reserved->ai_addr_len=0;
  memset (&reserved->socket_tab, 0, sizeof (struct _tcp_stream) * EXOSIP_MAX_SOCKETS);
  
  excontext->eXtltcp_reserved = reserved;
  return OSIP_SUCCESS;
}

static void
_tcp_tl_close_sockinfo (struct _tcp_stream *sockinfo)
{
  _eXosip_closesocket (sockinfo->socket);
  if (sockinfo->buf != NULL)
    osip_free (sockinfo->buf);
  if (sockinfo->sendbuf != NULL)
    osip_free (sockinfo->sendbuf);
#ifdef MULTITASKING_ENABLED
  if (sockinfo->readStream != NULL) {
    CFReadStreamClose (sockinfo->readStream);
    CFRelease (sockinfo->readStream);
  }
  if (sockinfo->writeStream != NULL) {
    CFWriteStreamClose (sockinfo->writeStream);
    CFRelease (sockinfo->writeStream);
  }
#endif
  memset (sockinfo, 0, sizeof (*sockinfo));
}

static int
tcp_tl_free (struct eXosip_t *excontext)
{
  struct eXtltcp *reserved = (struct eXtltcp *) excontext->eXtltcp_reserved;
  int pos;
  
  if (reserved == NULL)
    return OSIP_SUCCESS;
  
  memset (&reserved->ai_addr, 0, sizeof (struct sockaddr_storage));
  reserved->ai_addr_len=0;
  if (reserved->tcp_socket > 0)
    _eXosip_closesocket (reserved->tcp_socket);
  
  for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
    if (reserved->socket_tab[pos].socket > 0) {
      _tcp_tl_close_sockinfo (&reserved->socket_tab[pos]);
    }
  }
  
  osip_free (reserved);
  excontext->eXtltcp_reserved = NULL;
  return OSIP_SUCCESS;
}

static int
tcp_tl_open (struct eXosip_t *excontext)
{
  struct eXtltcp *reserved = (struct eXtltcp *) excontext->eXtltcp_reserved;
  int res;
  struct addrinfo *addrinfo = NULL;
  struct addrinfo *curinfo;
  int sock = -1;
  
  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }
  
  excontext->eXtl_transport.proto_local_port = excontext->eXtl_transport.proto_port;
  if (excontext->eXtl_transport.proto_local_port < 0)
    excontext->eXtl_transport.proto_local_port = 5060;
  
  
  res = _eXosip_get_addrinfo (excontext, &addrinfo, excontext->eXtl_transport.proto_ifs, excontext->eXtl_transport.proto_local_port, excontext->eXtl_transport.proto_num);
  if (res)
    return -1;
  
  for (curinfo = addrinfo; curinfo; curinfo = curinfo->ai_next) {
    socklen_t len;
    
    if (curinfo->ai_protocol && curinfo->ai_protocol != excontext->eXtl_transport.proto_num) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO3, NULL, "Skipping protocol %d\n", curinfo->ai_protocol));
      continue;
    }
    
    sock = (int) socket (curinfo->ai_family, curinfo->ai_socktype, curinfo->ai_protocol);
    if (sock < 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Cannot create socket %s!\n", strerror (ex_errno)));
      continue;
    }
    
    if (curinfo->ai_family == AF_INET6) {
#ifdef IPV6_V6ONLY
      if (setsockopt_ipv6only (sock)) {
        _eXosip_closesocket (sock);
        sock = -1;
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Cannot set socket option %s!\n", strerror (ex_errno)));
        continue;
      }
#endif /* IPV6_V6ONLY */
    }
    
    {
      int valopt = 1;
      
      setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (void *) &valopt, sizeof (valopt));
    }
    
#ifndef DISABLE_MAIN_SOCKET
    res = bind (sock, curinfo->ai_addr, (socklen_t)curinfo->ai_addrlen);
    if (res < 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Cannot bind socket node:%s family:%d %s\n", excontext->eXtl_transport.proto_ifs, curinfo->ai_family, strerror (ex_errno)));
      _eXosip_closesocket (sock);
      sock = -1;
      continue;
    }
    len = sizeof (reserved->ai_addr);
    res = getsockname (sock, (struct sockaddr *) &reserved->ai_addr, &len);
    if (res != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Cannot get socket name (%s)\n", strerror (ex_errno)));
      memcpy (&reserved->ai_addr, curinfo->ai_addr, curinfo->ai_addrlen);
    }
    reserved->ai_addr_len=len;
    
    if (excontext->eXtl_transport.proto_num == IPPROTO_TCP) {
      res = listen (sock, SOMAXCONN);
      if (res < 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Cannot bind socket node:%s family:%d %s\n", excontext->eXtl_transport.proto_ifs, curinfo->ai_family, strerror (ex_errno)));
        _eXosip_closesocket (sock);
        sock = -1;
        continue;
      }
    }
#endif
    
    break;
  }
  
  _eXosip_freeaddrinfo (addrinfo);
  
  if (sock < 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Cannot bind on port: %i\n", excontext->eXtl_transport.proto_local_port));
    return -1;
  }
  
  reserved->tcp_socket = sock;
  
  if (excontext->eXtl_transport.proto_local_port == 0) {
    /* get port number from socket */
    if (excontext->eXtl_transport.proto_family == AF_INET)
      excontext->eXtl_transport.proto_local_port = ntohs (((struct sockaddr_in *) &reserved->ai_addr)->sin_port);
    else
      excontext->eXtl_transport.proto_local_port = ntohs (((struct sockaddr_in6 *) &reserved->ai_addr)->sin6_port);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Binding on port %i!\n", excontext->eXtl_transport.proto_local_port));
  }
  return OSIP_SUCCESS;
}

static int
tcp_tl_reset (struct eXosip_t *excontext)
{
  struct eXtltcp *reserved = (struct eXtltcp *) excontext->eXtltcp_reserved;
  int pos;
  
  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }
  
  for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
    if (reserved->socket_tab[pos].socket > 0)
      reserved->socket_tab[pos].invalid = 1;
  }
  return OSIP_SUCCESS;
}

static int
tcp_tl_set_fdset (struct eXosip_t *excontext, fd_set * osip_fdset, fd_set * osip_wrset, int *fd_max)
{
  struct eXtltcp *reserved = (struct eXtltcp *) excontext->eXtltcp_reserved;
  int pos;
  
  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }
  
#ifndef DISABLE_MAIN_SOCKET
  if (reserved->tcp_socket <= 0)
    return -1;
  
  eXFD_SET (reserved->tcp_socket, osip_fdset);
  
  if (reserved->tcp_socket > *fd_max)
    *fd_max = reserved->tcp_socket;
#endif
  
  for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
    if (reserved->socket_tab[pos].socket > 0) {
      eXFD_SET (reserved->socket_tab[pos].socket, osip_fdset);
      if (reserved->socket_tab[pos].socket > *fd_max)
        *fd_max = reserved->socket_tab[pos].socket;
      if (reserved->socket_tab[pos].sendbuflen > 0)
        eXFD_SET (reserved->socket_tab[pos].socket, osip_wrset);
    }
  }
  
  return OSIP_SUCCESS;
}

/* Like strstr, but works for haystack that may contain binary data and is
 not NUL-terminated. */
static char *
buffer_find (const char *haystack, size_t haystack_len, const char *needle)
{
  const char *search = haystack, *end = haystack + haystack_len;
  char *p;
  size_t len = strlen (needle);
  
  while (search < end && (p = memchr (search, *needle, end - search)) != NULL) {
    if (p + len > end)
      break;
    if (memcmp (p, needle, len) == 0)
      return (p);
    search = p + 1;
  }
  
  return (NULL);
}

#define END_HEADERS_STR "\r\n\r\n"
#define CLEN_HEADER_STR "\r\ncontent-length:"
#define CLEN_HEADER_COMPACT_STR "\r\nl:"
#define CLEN_HEADER_STR2 "\r\ncontent-length "
#define CLEN_HEADER_COMPACT_STR2 "\r\nl "
#define const_strlen(x) (sizeof((x)) - 1)

/* consume any complete messages in sockinfo->buf and
 return the total number of bytes consumed */
static int
handle_messages (struct eXosip_t *excontext, struct _tcp_stream *sockinfo)
{
  int consumed = 0;
  char *buf = sockinfo->buf;
  size_t buflen = sockinfo->buflen;
  char *end_headers;
  
  while (buflen > 0 && (end_headers = buffer_find (buf, buflen, END_HEADERS_STR)) != NULL) {
    int clen, msglen;
    char *clen_header;
    
    if (buf == end_headers) {
      /* skip tcp standard keep-alive */
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "socket %s:%i: standard keep alive received (CRLFCRLF)\n", sockinfo->remote_ip, sockinfo->remote_port, buf));
      consumed += 4;
      buflen -= 4;
      buf += 4;
      continue;
    }
    
    /* stuff a nul in so we can use osip_strcasestr */
    *end_headers = '\0';
    
    /* ok we have complete headers, find content-length: or l: */
    clen_header = osip_strcasestr (buf, CLEN_HEADER_STR);
    if (!clen_header)
      clen_header = osip_strcasestr (buf, CLEN_HEADER_STR2);
    if (!clen_header)
      clen_header = osip_strcasestr (buf, CLEN_HEADER_COMPACT_STR);
    if (!clen_header)
      clen_header = osip_strcasestr (buf, CLEN_HEADER_COMPACT_STR2);
    if (clen_header != NULL) {
      clen_header = strchr (clen_header, ':');
      clen_header++;
    }
    if (!clen_header) {
      /* Oops, no content-length header.      Presume 0 (below) so we
       consume the headers and make forward progress.  This permits
       server-side keepalive of "\r\n\r\n". */
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "socket %s:%i: message has no content-length: <%s>\n", sockinfo->remote_ip, sockinfo->remote_port, buf));
    }
    clen = clen_header ? atoi (clen_header) : 0;
    if (clen<0)
      return (int)sockinfo->buflen; /* discard data */
    /* undo our overwrite and advance end_headers */
    *end_headers = END_HEADERS_STR[0];
    end_headers += const_strlen (END_HEADERS_STR);
    
    /* do we have the whole message? */
    msglen = (int) (end_headers - buf + clen);
    if (msglen > buflen) {
      /* nope */
      return consumed;
    }
    /* yep; handle the message */
    _eXosip_handle_incoming_message (excontext, buf, msglen, sockinfo->socket, sockinfo->remote_ip, sockinfo->remote_port, sockinfo->natted_ip, &sockinfo->natted_port);
    consumed += msglen;
    buflen -= msglen;
    buf += msglen;
  }
  
  return consumed;
}

static int
_tcp_tl_recv (struct eXosip_t *excontext, struct _tcp_stream *sockinfo)
{
  int r;
  
  if (!sockinfo->buf) {
    sockinfo->buf = (char *) osip_malloc (SIP_MESSAGE_MAX_LENGTH);
    if (sockinfo->buf == NULL)
      return OSIP_NOMEM;
    sockinfo->bufsize = SIP_MESSAGE_MAX_LENGTH;
    sockinfo->buflen = 0;
  }
  
  /* buffer is 100% full -> realloc with more size */
  if (sockinfo->bufsize - sockinfo->buflen <= 0) {
    sockinfo->buf = (char *) osip_realloc (sockinfo->buf, sockinfo->bufsize + 1000);
    if (sockinfo->buf == NULL)
      return OSIP_NOMEM;
    sockinfo->bufsize = sockinfo->bufsize + 1000;
  }
  
  /* buffer is 100% empty-> realloc with initial size */
  if (sockinfo->buflen == 0 && sockinfo->bufsize > SIP_MESSAGE_MAX_LENGTH) {
    osip_free (sockinfo->buf);
    sockinfo->buf = (char *) osip_malloc (SIP_MESSAGE_MAX_LENGTH);
    if (sockinfo->buf == NULL)
      return OSIP_NOMEM;
    sockinfo->bufsize = SIP_MESSAGE_MAX_LENGTH;
  }

  r = (int) recv (sockinfo->socket, sockinfo->buf + sockinfo->buflen, (int)(sockinfo->bufsize - sockinfo->buflen), 0);
  if (r == 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "socket %s:%i: eof\n", sockinfo->remote_ip, sockinfo->remote_port));
    _eXosip_mark_registration_expired (excontext, sockinfo->reg_call_id);
    _tcp_tl_close_sockinfo (sockinfo);
    return OSIP_UNDEFINED_ERROR;
  }
  else if (r < 0) {
    int status = ex_errno;
    
    if (is_wouldblock_error (status))
      return OSIP_SUCCESS;
    /* Do we need next line ? */
    /* else if (is_connreset_error(status)) */
    _eXosip_mark_registration_expired (excontext, sockinfo->reg_call_id);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "socket %s:%i: error %d\n", sockinfo->remote_ip, sockinfo->remote_port, status));
    _tcp_tl_close_sockinfo (sockinfo);
    return OSIP_UNDEFINED_ERROR;
  }
  else {
    int consumed;
    sockinfo->tcp_max_timeout=0;
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "socket %s:%i: read %d bytes\n", sockinfo->remote_ip, sockinfo->remote_port, r));
    sockinfo->buflen += r;
    consumed = handle_messages (excontext, sockinfo);
    if (consumed == 0) {
      return OSIP_SUCCESS;
    }
    else {
      if (sockinfo->buflen > consumed) {
        memmove (sockinfo->buf, sockinfo->buf + consumed, sockinfo->buflen - consumed);
        sockinfo->buflen -= consumed;
      }
      else {
        sockinfo->buflen = 0;
      }
      return OSIP_SUCCESS;
    }
  }
}

static int
tcp_tl_read_message (struct eXosip_t *excontext, fd_set * osip_fdset, fd_set * osip_wrset)
{
  struct eXtltcp *reserved = (struct eXtltcp *) excontext->eXtltcp_reserved;
  int pos = 0;
  
  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }
  
  if (FD_ISSET (reserved->tcp_socket, osip_fdset)) {
    /* accept incoming connection */
    char src6host[NI_MAXHOST];
    int recvport = 0;
    struct sockaddr_storage sa;
    int sock;
    int i;
    
    socklen_t slen;
    
    if (excontext->eXtl_transport.proto_family == AF_INET)
      slen = sizeof (struct sockaddr_in);
    else
      slen = sizeof (struct sockaddr_in6);
    
    for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
      if (reserved->socket_tab[pos].socket == 0)
        break;
    }
    if (pos == EXOSIP_MAX_SOCKETS) {
      /* delete an old one! */
      pos = 0;
      if (reserved->socket_tab[pos].socket > 0) {
        _tcp_tl_close_sockinfo (&reserved->socket_tab[pos]);
      }
      memset (&reserved->socket_tab[pos], 0, sizeof (reserved->socket_tab[pos]));
    }
    
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO3, NULL, "creating TCP socket at index: %i\n", pos));

    sock = (int)accept (reserved->tcp_socket, (struct sockaddr *) &sa, (socklen_t*)&slen);
    if (sock < 0) {
#if defined(EBADF)
      int status = ex_errno;
#endif
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Error accepting TCP socket\n"));
#if defined(EBADF)
      if (status == EBADF) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Error accepting TCP socket: EBADF\n"));
        memset (&reserved->ai_addr, 0, sizeof (struct sockaddr_storage));
        if (reserved->tcp_socket > 0) {
          _eXosip_closesocket (reserved->tcp_socket);
          for (i = 0; i < EXOSIP_MAX_SOCKETS; i++) {
            if (reserved->socket_tab[i].socket > 0 && reserved->socket_tab[i].is_server > 0)
              _tcp_tl_close_sockinfo (&reserved->socket_tab[i]);
          }
        }
        tcp_tl_open (excontext);
      }
#endif
    }
    else {
      reserved->socket_tab[pos].socket = sock;
      reserved->socket_tab[pos].is_server = 1;
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "New TCP connection accepted\n"));
      
      {
        int valopt = 1;
        
        setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (void *) &valopt, sizeof (valopt));
      }
      
      memset (src6host, 0, NI_MAXHOST);
      recvport = _eXosip_getport((struct sockaddr *) &sa, slen);
      _eXosip_getnameinfo((struct sockaddr *) &sa, slen, src6host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
      
      _eXosip_transport_set_dscp (excontext, excontext->eXtl_transport.proto_family, sock);
      
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Message received from: %s:%i\n", src6host, recvport));
      osip_strncpy (reserved->socket_tab[pos].remote_ip, src6host, sizeof (reserved->socket_tab[pos].remote_ip) - 1);
      reserved->socket_tab[pos].remote_port = recvport;
    }
  }
  
  for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
    if (reserved->socket_tab[pos].socket > 0) {
      if (FD_ISSET (reserved->socket_tab[pos].socket, osip_wrset))
        _tcp_tl_send_sockinfo (&reserved->socket_tab[pos], NULL, 0);
      if (reserved->socket_tab[pos].tcp_inprogress_max_timeout==0 && FD_ISSET (reserved->socket_tab[pos].socket, osip_fdset))
        _tcp_tl_recv (excontext, &reserved->socket_tab[pos]);
    }
  }
  
  return OSIP_SUCCESS;
}

static struct _tcp_stream *
_tcp_tl_find_sockinfo (struct eXosip_t *excontext, int sock)
{
  struct eXtltcp *reserved = (struct eXtltcp *) excontext->eXtltcp_reserved;
  int pos;
  
  for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
    if (reserved->socket_tab[pos].socket == sock) {
      return &reserved->socket_tab[pos];
    }
  }
  return NULL;
}

static int
_tcp_tl_find_socket (struct eXosip_t *excontext, char *host, int port)
{
  struct eXtltcp *reserved = (struct eXtltcp *) excontext->eXtltcp_reserved;
  int pos;
  
  for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
    if (reserved->socket_tab[pos].socket != 0) {
      if (0 == osip_strcasecmp (reserved->socket_tab[pos].remote_ip, host)
          && port == reserved->socket_tab[pos].remote_port)
        return pos;
    }
  }
  return -1;
}

static int
_tcp_tl_is_connected (int sock)
{
  int res;
  struct timeval tv;
  fd_set wrset;
  int valopt;
  socklen_t sock_len;
  
  tv.tv_sec = SOCKET_TIMEOUT / 1000;
  tv.tv_usec = (SOCKET_TIMEOUT % 1000) * 1000;
  
  FD_ZERO (&wrset);
  FD_SET (sock, &wrset);
  
  res = select (sock + 1, NULL, &wrset, NULL, &tv);
  if (res > 0) {
    sock_len = sizeof (int);
    if (getsockopt (sock, SOL_SOCKET, SO_ERROR, (void *) (&valopt), &sock_len)
        == 0) {
      if (valopt) {
#if defined(_WIN32_WCE) || defined(WIN32)
        if (ex_errno == WSAEWOULDBLOCK) {
#else
          if (ex_errno == EINPROGRESS) {
#endif
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot connect socket node(%i) / %s[%d]\n", valopt, strerror (ex_errno), ex_errno));
            return 1;
          }
          if (is_wouldblock_error(ex_errno)) {
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot connect socket node(%i) would block / %s[%d]\n", valopt, strerror (ex_errno), ex_errno));
            return 1;
          }
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot connect socket node / %s[%d]\n", strerror (ex_errno), ex_errno));
          return -1;
        }
        else {
          return 0;
        }
      }
      else {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot connect socket node / error in getsockopt %s[%d]\n", strerror (ex_errno), ex_errno));
        return -1;
      }
    }
    else if (res < 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot connect socket node / error in select %s[%d]\n", strerror (ex_errno), ex_errno));
      return -1;
    }
    else {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot connect socket node / select timeout (%d ms)\n", SOCKET_TIMEOUT));
      return 1;
    }
  }
  
  static int
  _tcp_tl_check_connected (struct eXosip_t *excontext)
  {
    struct eXtltcp *reserved = (struct eXtltcp *) excontext->eXtltcp_reserved;
    int pos;
    int res;
    
    for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
      if (reserved->socket_tab[pos].invalid > 0) {
        OSIP_TRACE (osip_trace
                    (__FILE__, __LINE__, OSIP_INFO2, NULL,
                     "_tcp_tl_check_connected: socket node is in invalid state:%s:%i, socket %d [pos=%d], family:%d\n",
                     reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port, reserved->socket_tab[pos].socket, pos, reserved->socket_tab[pos].ai_addr.sa_family));
        _tcp_tl_close_sockinfo (&reserved->socket_tab[pos]);
        continue;
      }
      
      if (reserved->socket_tab[pos].socket > 0 && reserved->socket_tab[pos].ai_addrlen > 0) {
        res = _tcp_tl_is_connected (reserved->socket_tab[pos].socket);
        if (res > 0) {
#if 0
          /* bug: calling connect several times for TCP is not allowed by specification */
          res = connect (reserved->socket_tab[pos].socket, &reserved->socket_tab[pos].ai_addr, reserved->socket_tab[pos].ai_addrlen);
          if (res<0) {
            int status = ex_errno;
            OSIP_TRACE (osip_trace
                        (__FILE__, __LINE__, OSIP_INFO2, NULL,
                         "_tcp_tl_check_connected: connect being called again (res=%i) (errno=%i) (%s)\n", res, status, strerror (status)));
          }
#endif
          OSIP_TRACE (osip_trace
                      (__FILE__, __LINE__, OSIP_INFO2, NULL,
                       "_tcp_tl_check_connected: socket node:%s:%i, socket %d [pos=%d], family:%d, in progress\n",
                       reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port, reserved->socket_tab[pos].socket, pos, reserved->socket_tab[pos].ai_addr.sa_family));
          continue;
        }
        else if (res == 0) {
          OSIP_TRACE (osip_trace
                      (__FILE__, __LINE__, OSIP_INFO1, NULL,
                       "_tcp_tl_check_connected: socket node:%s:%i , socket %d [pos=%d], family:%d, connected\n",
                       reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port, reserved->socket_tab[pos].socket, pos, reserved->socket_tab[pos].ai_addr.sa_family));
          /* stop calling "connect()" */
          reserved->socket_tab[pos].ai_addrlen = 0;
          reserved->socket_tab[pos].tcp_inprogress_max_timeout=0;
          continue;
        }
        else {
          OSIP_TRACE (osip_trace
                      (__FILE__, __LINE__, OSIP_INFO2, NULL,
                       "_tcp_tl_check_connected: socket node:%s:%i, socket %d [pos=%d], family:%d, error\n",
                       reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port, reserved->socket_tab[pos].socket, pos, reserved->socket_tab[pos].ai_addr.sa_family));
          _tcp_tl_close_sockinfo (&reserved->socket_tab[pos]);
          continue;
        }
      }
    }
    return 0;
  }
  
  static int
  _tcp_tl_connect_socket (struct eXosip_t *excontext, char *host, int port)
  {
    struct eXtltcp *reserved = (struct eXtltcp *) excontext->eXtltcp_reserved;
    int pos;
    int res;
    struct addrinfo *addrinfo = NULL;
    struct addrinfo *curinfo;
    int sock = -1;
    struct sockaddr selected_ai_addr;
    socklen_t selected_ai_addrlen;
    
    char src6host[NI_MAXHOST];
    
    memset (src6host, 0, sizeof (src6host));
    
    selected_ai_addrlen = 0;
    memset (&selected_ai_addr, 0, sizeof (struct sockaddr));
    
    for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
      if (reserved->socket_tab[pos].socket == 0) {
        break;
      }
    }
    
    if (pos == EXOSIP_MAX_SOCKETS) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "reserved->socket_tab is full - cannot create new socket!\n"));
#ifdef DELETE_OLD_SOCKETS
      /* delete an old one! */
      pos = 0;
      if (reserved->socket_tab[pos].socket > 0) {
        _tcp_tl_close_sockinfo (&reserved->socket_tab[pos]);
      }
      memset (&reserved->socket_tab[pos], 0, sizeof (reserved->socket_tab[pos]));
#else
      return -1;
#endif
    }
    
    res = _eXosip_get_addrinfo (excontext, &addrinfo, host, port, IPPROTO_TCP);
    if (res)
      return -1;
    
    for (curinfo = addrinfo; curinfo; curinfo = curinfo->ai_next) {
      int i;
      
      if (curinfo->ai_protocol && curinfo->ai_protocol != IPPROTO_TCP)
        continue;
      
      res = getnameinfo ((struct sockaddr *) curinfo->ai_addr, (socklen_t)curinfo->ai_addrlen, src6host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
      if (res != 0)
        continue;
      
      i = _tcp_tl_find_socket (excontext, src6host, port);
      if (i >= 0) {
        _eXosip_freeaddrinfo (addrinfo);
        return i;
      }
    }
    
    for (curinfo = addrinfo; curinfo; curinfo = curinfo->ai_next) {
      if (curinfo->ai_protocol && curinfo->ai_protocol != IPPROTO_TCP) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Skipping protocol %d\n", curinfo->ai_protocol));
        continue;
      }
      
      res = getnameinfo ((struct sockaddr *) curinfo->ai_addr, (socklen_t)curinfo->ai_addrlen, src6host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
      
      if (res == 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "New binding with %s:%i\n", src6host, port));
      }
      
      sock = (int) socket (curinfo->ai_family, curinfo->ai_socktype, curinfo->ai_protocol);
      if (sock < 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot create socket %s!\n", strerror (ex_errno)));
        continue;
      }
      
      if (curinfo->ai_family == AF_INET6) {
#ifdef IPV6_V6ONLY
        if (setsockopt_ipv6only (sock)) {
          _eXosip_closesocket (sock);
          sock = -1;
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot set socket option %s!\n", strerror (ex_errno)));
          continue;
        }
#endif /* IPV6_V6ONLY */
      }
      
      if (reserved->ai_addr_len>0)
      {
        if (excontext->reuse_tcp_port>0) {
          struct sockaddr_storage ai_addr;
          int valopt = 1;
          
          setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (void *) &valopt, sizeof (valopt));
          
          memcpy(&ai_addr, &reserved->ai_addr, reserved->ai_addr_len);
          if (ai_addr.ss_family == AF_INET)
            ((struct sockaddr_in *) &ai_addr)->sin_port = htons(excontext->eXtl_transport.proto_local_port);
          else
            ((struct sockaddr_in6 *) &ai_addr)->sin6_port = htons(excontext->eXtl_transport.proto_local_port);
          res = bind (sock, (const struct sockaddr *)&ai_addr, reserved->ai_addr_len);
          if (res < 0) {
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "Cannot bind socket node:%s family:%d %s\n", excontext->eXtl_transport.proto_ifs, ai_addr.ss_family, strerror (ex_errno)));
          }
        } else if (excontext->oc_local_address[0]=='\0') {
          struct sockaddr_storage ai_addr;
          int count=0;
          memcpy(&ai_addr, &reserved->ai_addr, reserved->ai_addr_len);
          while (count<100) {
            if (excontext->oc_local_port_range[0]<1024) {
              if (ai_addr.ss_family == AF_INET)
                ((struct sockaddr_in *) &ai_addr)->sin_port = htons(0);
              else
                ((struct sockaddr_in6 *) &ai_addr)->sin6_port = htons(0);
            } else {
              if (excontext->oc_local_port_current==0)
                excontext->oc_local_port_current = excontext->oc_local_port_range[0];
              /* reset value */
              if (excontext->oc_local_port_current>=excontext->oc_local_port_range[1])
                excontext->oc_local_port_current = excontext->oc_local_port_range[0];
              
              if (ai_addr.ss_family == AF_INET)
                ((struct sockaddr_in *) &ai_addr)->sin_port = htons(excontext->oc_local_port_current);
              else
                ((struct sockaddr_in6 *) &ai_addr)->sin6_port = htons(excontext->oc_local_port_current);
              
            }
            res = bind (sock, (const struct sockaddr *)&ai_addr, reserved->ai_addr_len);
            if (res < 0) {
              OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "Cannot bind socket node:%s family:%d (port=%i) %s\n", excontext->eXtl_transport.proto_ifs, ai_addr.ss_family, excontext->oc_local_port_current, strerror (ex_errno)));
              count++;
              if (excontext->oc_local_port_range[0]>=1024)
                excontext->oc_local_port_current++;
              continue;
            }
            if (excontext->oc_local_port_range[0]>=1024)
              excontext->oc_local_port_current++;
            break;
          }
        } else {
          int count=0;
          
          if (excontext->oc_local_port_range[0]<1024)
            excontext->oc_local_port_range[0]=0;
          while (count<100) {
            struct addrinfo *oc_addrinfo = NULL;
            struct addrinfo *oc_curinfo;
            if (excontext->oc_local_port_current==0)
              excontext->oc_local_port_current = excontext->oc_local_port_range[0];
            if (excontext->oc_local_port_current>=excontext->oc_local_port_range[1])
              excontext->oc_local_port_current = excontext->oc_local_port_range[0];
            
            _eXosip_get_addrinfo(excontext, &oc_addrinfo, excontext->oc_local_address, excontext->oc_local_port_current, IPPROTO_TCP);
            
            for (oc_curinfo = oc_addrinfo; oc_curinfo; oc_curinfo = oc_curinfo->ai_next) {
              if  (oc_curinfo->ai_protocol && oc_curinfo->ai_protocol != IPPROTO_TCP) {
                OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Skipping protocol %d\n", oc_curinfo->ai_protocol));
                continue;
              }
              break;
            }
            if (oc_curinfo==NULL) {
              OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "not able to find any address to bind\n"));
              _eXosip_freeaddrinfo (oc_addrinfo);
              break;
            }
            res = bind (sock, (const struct sockaddr *)oc_curinfo->ai_addr, (socklen_t)oc_curinfo->ai_addrlen);
            if (res < 0) {
              OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "Cannot bind socket node:%s family:%d (port=%i) %s\n", excontext->oc_local_address, oc_curinfo->ai_addr->sa_family, excontext->oc_local_port_current, strerror (ex_errno)));
              _eXosip_freeaddrinfo (oc_addrinfo);
              count++;
              if (excontext->oc_local_port_range[0]!=0)
                excontext->oc_local_port_current++;
              continue;
            }
            _eXosip_freeaddrinfo (oc_addrinfo);
            if (excontext->oc_local_port_range[0]!=0)
              excontext->oc_local_port_current++;
            break;
          }
        }
      }
      
      /* set NON-BLOCKING MODE */
#if defined(_WIN32_WCE) || defined(WIN32)
      {
        unsigned long nonBlock = 1;
        int val;
        
        ioctlsocket (sock, FIONBIO, &nonBlock);
        
        val = 1;
        if (setsockopt (sock, SOL_SOCKET, SO_KEEPALIVE, (char *) &val, sizeof (val)) == -1) {
          _eXosip_closesocket (sock);
          sock = -1;
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot get socket flag!\n"));
          continue;
        }
      }
#ifdef HAVE_MSTCPIP_H
      {
        DWORD err = 0L;
        DWORD dwBytes = 0L;
        struct tcp_keepalive kalive = { 0 };
        struct tcp_keepalive kaliveOut = { 0 };
        kalive.onoff = 1;
        kalive.keepalivetime = 30000;     /* Keep Alive in 5.5 sec. */
        kalive.keepaliveinterval = 3000;  /* Resend if No-Reply */
        err = WSAIoctl (sock, SIO_KEEPALIVE_VALS, &kalive, sizeof (kalive), &kaliveOut, sizeof (kaliveOut), &dwBytes, NULL, NULL);
        if (err != 0) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "Cannot set keepalive interval!\n"));
        }
      }
#endif
#else
      {
        int val;
        
        val = fcntl (sock, F_GETFL);
        if (val < 0) {
          _eXosip_closesocket (sock);
          sock = -1;
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot get socket flag!\n"));
          continue;
        }
        val |= O_NONBLOCK;
        if (fcntl (sock, F_SETFL, val) < 0) {
          _eXosip_closesocket (sock);
          sock = -1;
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot set socket flag!\n"));
          continue;
        }
#if SO_KEEPALIVE
        val = 1;
        if (setsockopt (sock, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof (val)) == -1) {
        }
#endif
#if 0
        val = 30;                 /* 30 sec before starting probes */
        setsockopt (sock, SOL_TCP, TCP_KEEPIDLE, &val, sizeof (val));
        val = 2;                  /* 2 probes max */
        setsockopt (sock, SOL_TCP, TCP_KEEPCNT, &val, sizeof (val));
        val = 10;                 /* 10 seconds between each probe */
        setsockopt (sock, SOL_TCP, TCP_KEEPINTVL, &val, sizeof (val));
#endif
#if SO_NOSIGPIPE
        val = 1;
        setsockopt (sock, SOL_SOCKET, SO_NOSIGPIPE, (void *) &val, sizeof (int));
#endif
        
      }
#endif
      
#if TCP_NODELAY
      {
        int val;
        
        val = 1;
        if (setsockopt (sock, IPPROTO_TCP, TCP_NODELAY, (char *) &val, sizeof (int)) != 0) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot set socket flag (TCP_NODELAY)\n"));
        }
      }
#endif
      
      _eXosip_transport_set_dscp (excontext, excontext->eXtl_transport.proto_family, sock);
      
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "socket node:%s , socket %d, family:%d set to non blocking mode\n", host, sock, curinfo->ai_family));
      res = connect (sock, curinfo->ai_addr, (socklen_t)curinfo->ai_addrlen);
      if (res < 0) {
        int connect_err = ex_errno;
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "connecting socket node:%s, socket %d [pos=%d], family:%d, %s[%d]\n", host, sock, pos, curinfo->ai_family, strerror (connect_err), connect_err));
#if defined(_WIN32_WCE) || defined(WIN32)
        if (connect_err != WSAEWOULDBLOCK) {
#else
          if (connect_err != EINPROGRESS) {
#endif
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot connect socket node:%s family:%d %s[%d]\n", host, curinfo->ai_family, strerror (connect_err), connect_err));
            _eXosip_closesocket (sock);
            sock = -1;
            continue;
          }
          else {
            res = _tcp_tl_is_connected (sock);
            if (res > 0) {
              OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "socket node:%s, socket %d [pos=%d], family:%d, in progress\n", host, sock, pos, curinfo->ai_family));
              selected_ai_addrlen = (socklen_t)curinfo->ai_addrlen;
              memcpy (&selected_ai_addr, curinfo->ai_addr, sizeof (struct sockaddr));
              break;
            }
            else if (res == 0) {
#ifdef MULTITASKING_ENABLED
              reserved->socket_tab[pos].readStream = NULL;
              reserved->socket_tab[pos].writeStream = NULL;
              CFStreamCreatePairWithSocket (kCFAllocatorDefault, sock, &reserved->socket_tab[pos].readStream, &reserved->socket_tab[pos].writeStream);
              if (reserved->socket_tab[pos].readStream != NULL)
                CFReadStreamSetProperty (reserved->socket_tab[pos].readStream, kCFStreamNetworkServiceType, kCFStreamNetworkServiceTypeVoIP);
              if (reserved->socket_tab[pos].writeStream != NULL)
                CFWriteStreamSetProperty (reserved->socket_tab[pos].writeStream, kCFStreamNetworkServiceType, kCFStreamNetworkServiceTypeVoIP);
              if (CFReadStreamOpen (reserved->socket_tab[pos].readStream)) {
                OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "CFReadStreamOpen Succeeded!\n"));
              }
              
              CFWriteStreamOpen (reserved->socket_tab[pos].writeStream);
#endif
              OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "socket node:%s , socket %d [pos=%d], family:%d, connected\n", host, sock, pos, curinfo->ai_family));
              selected_ai_addrlen = 0;
              memcpy (&selected_ai_addr, curinfo->ai_addr, sizeof (struct sockaddr));
              reserved->socket_tab[pos].tcp_inprogress_max_timeout=0;
              break;
            }
            else {
              _eXosip_closesocket (sock);
              sock = -1;
              continue;
            }
          }
        }
        
        break;
      }
      
      _eXosip_freeaddrinfo (addrinfo);
      
      if (sock > 0) {
        reserved->socket_tab[pos].socket = sock;
        
        reserved->socket_tab[pos].ai_addrlen = selected_ai_addrlen;
        memset (&reserved->socket_tab[pos].ai_addr, 0, sizeof (struct sockaddr));
        if (selected_ai_addrlen > 0)
          memcpy (&reserved->socket_tab[pos].ai_addr, &selected_ai_addr, selected_ai_addrlen);
        
        if (src6host[0] == '\0')
          osip_strncpy (reserved->socket_tab[pos].remote_ip, host, sizeof (reserved->socket_tab[pos].remote_ip) - 1);
        else
          osip_strncpy (reserved->socket_tab[pos].remote_ip, src6host, sizeof (reserved->socket_tab[pos].remote_ip) - 1);
        
        reserved->socket_tab[pos].remote_port = port;
        
        {
          struct sockaddr_storage local_ai_addr;
          socklen_t selected_ai_addrlen;
          
          memset (&local_ai_addr, 0, sizeof (struct sockaddr_storage));
          selected_ai_addrlen = sizeof (struct sockaddr_storage);
          res = getsockname (sock, (struct sockaddr *) &local_ai_addr, &selected_ai_addrlen);
          if (res == 0) {
            if (local_ai_addr.ss_family == AF_INET)
              reserved->socket_tab[pos].ephemeral_port = ntohs (((struct sockaddr_in *) &local_ai_addr)->sin_port);
            else
              reserved->socket_tab[pos].ephemeral_port = ntohs (((struct sockaddr_in6 *) &local_ai_addr)->sin6_port);
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Outgoing socket created on port %i!\n", reserved->socket_tab[pos].ephemeral_port));
          }
        }
        
        reserved->socket_tab[pos].tcp_inprogress_max_timeout = osip_getsystemtime (NULL) + 32;
        return pos;
      }
      
      return -1;
    }
    
    static int
    _tcp_tl_send_sockinfo (struct _tcp_stream *sockinfo, const char *msg, int msglen)
    {
      int i;
      
      while (1) {
        i = (int) send (sockinfo->socket, (const void *) msg, msglen, 0);
        if (i < 0) {
          int status = ex_errno;
          
          if (is_wouldblock_error (status)) {
            struct timeval tv;
            fd_set wrset;
            
            tv.tv_sec = SOCKET_TIMEOUT / 1000;
            tv.tv_usec = (SOCKET_TIMEOUT % 1000) * 1000;
            if (tv.tv_usec == 0)
              tv.tv_usec += 10000;
            
            FD_ZERO (&wrset);
            FD_SET (sockinfo->socket, &wrset);
            
            i = select (sockinfo->socket + 1, NULL, &wrset, NULL, &tv);
            if (i > 0) {
              continue;
            }
            else if (i < 0) {
              OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "TCP select error: %s:%i\n", strerror (ex_errno), ex_errno));
              return -1;
            }
            else {
              OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "TCP timeout: %d ms\n", SOCKET_TIMEOUT));
              continue;
            }
          }
          else {
            /* SIP_NETWORK_ERROR; */
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "TCP error: %s\n", strerror (status)));
            return -1;
          }
        }
        else if (i == 0) {
          break;                    /* what's the meaning here? */
        }
        else if (i < msglen) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "TCP partial write: wrote %i instead of %i\n", i, msglen));
          msglen -= i;
          msg += i;
          continue;
        }
        break;
      }
      return OSIP_SUCCESS;
    }
    
    static int
    _tcp_tl_send (struct eXosip_t *excontext, int sock, const char *msg, int msglen)
    {
      struct _tcp_stream *sockinfo = _tcp_tl_find_sockinfo (excontext, sock);
      
      if (sockinfo == NULL) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "could not find sockinfo for socket %d! dropping message\n", sock));
        return -1;
      }
      return _tcp_tl_send_sockinfo (sockinfo, msg, msglen);
    }
    
    static int
    _tcp_tl_update_contact (struct eXosip_t *excontext, osip_message_t * req, char *natted_ip, int natted_port)
    {
      if (req->application_data != (void*) 0x1)
        return OSIP_SUCCESS;
      
      if ((natted_ip != NULL && natted_ip[0] != '\0') || natted_port > 0) {
        osip_list_iterator_t it;
        osip_contact_t* co = (osip_contact_t *)osip_list_get_first(&req->contacts, &it);
        while (co != NULL) {
          if (co != NULL && co->url != NULL && co->url->host != NULL) {
            if (natted_port > 0) {
              if (co->url->port)
                osip_free (co->url->port);
              co->url->port = (char *) osip_malloc (10);
              snprintf (co->url->port, 9, "%i", natted_port);
              osip_message_force_update (req);
            }
            if (natted_ip != NULL && natted_ip[0] != '\0') {
              osip_free (co->url->host);
              co->url->host = osip_strdup (natted_ip);
              osip_message_force_update (req);
            }
          }
          co = (osip_contact_t *)osip_list_get_next(&it);
        }
      }
      
      return OSIP_SUCCESS;
    }
    
    static int
    tcp_tl_send_message (struct eXosip_t *excontext, osip_transaction_t * tr, osip_message_t * sip, char *host, int port, int out_socket)
    {
      struct eXtltcp *reserved = (struct eXtltcp *) excontext->eXtltcp_reserved;
      size_t length = 0;
      char *message = NULL;
      int i;
      int pos = -1;
      osip_naptr_t *naptr_record = NULL;
      
      if (reserved == NULL) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
        return OSIP_WRONG_STATE;
      }
      
      if (host == NULL) {
        host = sip->req_uri->host;
        if (sip->req_uri->port != NULL)
          port = osip_atoi (sip->req_uri->port);
        else
          port = 5060;
      }
      
      i = -1;
      if (tr == NULL) {
        _eXosip_srv_lookup (excontext, sip, &naptr_record);
        
        if (naptr_record != NULL) {
          eXosip_dnsutils_dns_process (naptr_record, 1);
          if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NAPTRDONE || naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVINPROGRESS)
            eXosip_dnsutils_dns_process (naptr_record, 1);
        }
        
        if (naptr_record != NULL && naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVDONE) {
          /* 4: check if we have the one we want... */
          if (naptr_record->siptcp_record.name[0] != '\0' && naptr_record->siptcp_record.srventry[naptr_record->siptcp_record.index].srv[0] != '\0') {
            /* always choose the first here.
             if a network error occur, remove first entry and
             replace with next entries.
             */
            osip_srv_entry_t *srv;
            
            srv = &naptr_record->siptcp_record.srventry[naptr_record->siptcp_record.index];
            if (srv->ipaddress[0]) {
              host = srv->ipaddress;
              port = srv->port;
            }
            else {
              host = srv->srv;
              port = srv->port;
            }
          }
        }
        
        if (naptr_record != NULL && naptr_record->keep_in_cache == 0)
          osip_free (naptr_record);
        naptr_record = NULL;
      }
      else {
        naptr_record = tr->naptr_record;
      }
      
      
      if (naptr_record != NULL) {
        /* 1: make sure there is no pending DNS */
        eXosip_dnsutils_dns_process (naptr_record, 0);
        if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NAPTRDONE || naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVINPROGRESS)
          eXosip_dnsutils_dns_process (naptr_record, 0);
        
        if (naptr_record->naptr_state == OSIP_NAPTR_STATE_UNKNOWN) {
          /* fallback to DNS A */
          if (naptr_record->keep_in_cache == 0)
            osip_free (naptr_record);
          naptr_record = NULL;
          if (tr != NULL)
            tr->naptr_record = NULL;
          /* must never happen? */
        }
        else if (naptr_record->naptr_state == OSIP_NAPTR_STATE_INPROGRESS) {
          /* 2: keep waiting (naptr answer not received) */
          return OSIP_SUCCESS + 1;
        }
        else if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NAPTRDONE) {
          /* 3: keep waiting (naptr answer received/no srv answer received) */
          return OSIP_SUCCESS + 1;
        }
        else if (naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVINPROGRESS) {
          /* 3: keep waiting (naptr answer received/no srv answer received) */
          return OSIP_SUCCESS + 1;
        }
        else if (naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVDONE) {
          /* 4: check if we have the one we want... */
          if (naptr_record->siptcp_record.name[0] != '\0' && naptr_record->siptcp_record.srventry[naptr_record->siptcp_record.index].srv[0] != '\0') {
            /* always choose the first here.
             if a network error occur, remove first entry and
             replace with next entries.
             */
            osip_srv_entry_t *srv;
            
            if (MSG_IS_REGISTER (sip) || MSG_IS_OPTIONS (sip)) {
              /* activate the failover capability: for no answer OR 503 */
              if (naptr_record->siptcp_record.srventry[naptr_record->siptcp_record.index].srv_is_broken.tv_sec>0) {
                naptr_record->siptcp_record.srventry[naptr_record->siptcp_record.index].srv_is_broken.tv_sec=0;
                naptr_record->siptcp_record.srventry[naptr_record->siptcp_record.index].srv_is_broken.tv_usec=0;
                if (eXosip_dnsutils_rotate_srv (&naptr_record->siptcp_record) > 0) {
                  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL,
                                          "Doing TCP failover: %s:%i->%s:%i\n", host, port, naptr_record->siptcp_record.srventry[naptr_record->siptcp_record.index].srv, naptr_record->siptcp_record.srventry[naptr_record->siptcp_record.index].port));
                }
              }
            }
            srv = &naptr_record->siptcp_record.srventry[naptr_record->siptcp_record.index];
            if (srv->ipaddress[0]) {
              host = srv->ipaddress;
              port = srv->port;
            }
            else {
              host = srv->srv;
              port = srv->port;
            }
          }
        }
        else if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NOTSUPPORTED || naptr_record->naptr_state == OSIP_NAPTR_STATE_RETRYLATER) {
          /* 5: fallback to DNS A */
          if (naptr_record->keep_in_cache == 0)
            osip_free (naptr_record);
          naptr_record = NULL;
          if (tr != NULL)
            tr->naptr_record = NULL;
        }
      }
      
      /* verify all current connections */
      _tcp_tl_check_connected (excontext);
      
      if (out_socket > 0) {
        for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
          if (reserved->socket_tab[pos].socket != 0) {
            if (reserved->socket_tab[pos].socket == out_socket) {
              OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "reusing REQUEST connection (to dest=%s:%i)\n", reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port));
              break;
            }
          }
        }
        if (pos == EXOSIP_MAX_SOCKETS)
          out_socket = 0;
        
        if (out_socket > 0) {
          int pos2;
          
          /* If we have SEVERAL sockets to same destination with different port
           number, we search for the one with "SAME port" number.
           The specification is not clear about re-using the existing transaction
           in that use-case...
           Such test, will help mainly with server having 2 sockets: one for
           incoming transaction and one for outgoing transaction?
           */
          pos2 = _tcp_tl_find_socket (excontext, host, port);
          if (pos2 >= 0) {
            out_socket = reserved->socket_tab[pos2].socket;
            pos = pos2;
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "reusing connection --with exact port--: (to dest=%s:%i)\n", reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port));
          }
        }
      }
      
      /* Step 1: find existing socket to send message */
      if (out_socket <= 0) {
        pos = _tcp_tl_find_socket (excontext, host, port);
        if (pos >= 0) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "reusing connection (to dest=%s:%i)\n", reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port));
        }
        
        /* Step 2: create new socket with host:port */
        if (pos < 0) {
          pos = _tcp_tl_connect_socket (excontext, host, port);
        }
        if (pos >= 0) {
          out_socket = reserved->socket_tab[pos].socket;
        }
      }
      
      if (out_socket <= 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "no socket can be found or created\n"));
        if (naptr_record != NULL && (MSG_IS_REGISTER (sip) || MSG_IS_OPTIONS (sip))) {
          if (eXosip_dnsutils_rotate_srv (&naptr_record->siptcp_record) > 0) {
            _eXosip_mark_registration_expired (excontext, sip->call_id->number);
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL,
                                    "Doing TCP failover: %s:%i->%s:%i\n", host, port, naptr_record->siptcp_record.srventry[naptr_record->siptcp_record.index].srv, naptr_record->siptcp_record.srventry[naptr_record->siptcp_record.index].port));
          }
        }
        return -1;
      }
      
      if (MSG_IS_REGISTER (sip)) {
        /* this value is saved: when a connection breaks, we will ask to retry the registration */
        snprintf(reserved->socket_tab[pos].reg_call_id, sizeof(reserved->socket_tab[pos].reg_call_id), "%s", sip->call_id->number);
      }
      
      i = _tcp_tl_is_connected (out_socket);
      if (i > 0) {
        time_t now;
        if (tr!=NULL) {
          now = osip_getsystemtime (NULL);
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "socket node:%s, socket %d [pos=%d], in progress\n", host, out_socket, pos));
          if (tr != NULL && now - tr->birth_time > 10) {
            if (naptr_record != NULL && (MSG_IS_REGISTER (sip) || MSG_IS_OPTIONS (sip))) {
              if (eXosip_dnsutils_rotate_srv (&naptr_record->siptcp_record) > 0) {
                _eXosip_mark_registration_expired (excontext, reserved->socket_tab[pos].reg_call_id);
                if (pos >= 0) _tcp_tl_close_sockinfo (&reserved->socket_tab[pos]);
                OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL,
                                        "Doing TCP failover: %s:%i->%s:%i\n", host, port, naptr_record->siptcp_record.srventry[naptr_record->siptcp_record.index].srv, naptr_record->siptcp_record.srventry[naptr_record->siptcp_record.index].port));
              }
            }
            return -1;
          }
        }
        return 1;
      }
      else if (i == 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "socket node:%s , socket %d [pos=%d], connected\n", host, out_socket, pos));
        reserved->socket_tab[pos].tcp_inprogress_max_timeout=0;
      }
      else {
        if (naptr_record != NULL && (MSG_IS_REGISTER (sip) || MSG_IS_OPTIONS (sip))) {
          if (eXosip_dnsutils_rotate_srv (&naptr_record->siptcp_record) > 0) {
            _eXosip_mark_registration_expired (excontext, reserved->socket_tab[pos].reg_call_id);
          }
        }
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "socket node:%s, socket %d [pos=%d], socket error\n", host, out_socket, pos));
        _tcp_tl_close_sockinfo (&reserved->socket_tab[pos]);
        return -1;
      }
      
      
#ifdef MULTITASKING_ENABLED
      
      if (pos >= 0 && reserved->socket_tab[pos].readStream == NULL) {
        reserved->socket_tab[pos].readStream = NULL;
        reserved->socket_tab[pos].writeStream = NULL;
        CFStreamCreatePairWithSocket (kCFAllocatorDefault, out_socket, &reserved->socket_tab[pos].readStream, &reserved->socket_tab[pos].writeStream);
        if (reserved->socket_tab[pos].readStream != NULL)
          CFReadStreamSetProperty (reserved->socket_tab[pos].readStream, kCFStreamNetworkServiceType, kCFStreamNetworkServiceTypeVoIP);
        if (reserved->socket_tab[pos].writeStream != NULL)
          CFWriteStreamSetProperty (reserved->socket_tab[pos].writeStream, kCFStreamNetworkServiceType, kCFStreamNetworkServiceTypeVoIP);
        if (CFReadStreamOpen (reserved->socket_tab[pos].readStream)) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "CFReadStreamOpen Succeeded!\n"));
        }
        
        CFWriteStreamOpen (reserved->socket_tab[pos].writeStream);
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "socket node:%s:%i , socket %d [pos=%d], family:?, connected\n", reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port, reserved->socket_tab[pos].socket, pos));
      }
#endif
      
      _eXosip_request_viamanager(excontext, tr, sip, IPPROTO_TCP, NULL, reserved->socket_tab[pos].ephemeral_port, reserved->socket_tab[pos].socket, host);
      if (excontext->use_ephemeral_port==1)
        _eXosip_message_contactmanager(excontext, tr, sip, IPPROTO_TCP, NULL, reserved->socket_tab[pos].ephemeral_port, reserved->socket_tab[pos].socket, host);
      else
        _eXosip_message_contactmanager(excontext, tr, sip, IPPROTO_TCP, NULL, excontext->eXtl_transport.proto_local_port, reserved->socket_tab[pos].socket, host);
      if (excontext->tcp_firewall_ip[0] != '\0' || excontext->auto_masquerade_contact > 0)
        _tcp_tl_update_contact (excontext, sip, reserved->socket_tab[pos].natted_ip, reserved->socket_tab[pos].natted_port);
      
      /* remove preloaded route if there is no tag in the To header
       */
      {
        osip_route_t *route = NULL;
        osip_generic_param_t *tag = NULL;
        
        if (excontext->remove_prerouteset>0) {
          osip_message_get_route (sip, 0, &route);
          osip_to_get_tag (sip->to, &tag);
          if (tag == NULL && route != NULL && route->url != NULL) {
            osip_list_remove (&sip->routes, 0);
            osip_message_force_update(sip);
          }
        }
        i = osip_message_to_str (sip, &message, &length);
        if (tag == NULL && route != NULL && route->url != NULL) {
          osip_list_add (&sip->routes, route, 0);
        }
      }
      
      if (i != 0 || length <= 0) {
        osip_free (message);
        return -1;
      }
      
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Message sent: (to dest=%s:%i) \n%s\n", host, port, message));
      
      if (pos >= 0 && excontext->enable_dns_cache == 1 && osip_strcasecmp (host, reserved->socket_tab[pos].remote_ip) != 0 && MSG_IS_REQUEST (sip)) {
        if (MSG_IS_REGISTER (sip)) {
          struct eXosip_dns_cache entry;
          
          memset (&entry, 0, sizeof (struct eXosip_dns_cache));
          snprintf (entry.host, sizeof (entry.host), "%s", host);
          snprintf (entry.ip, sizeof (entry.ip), "%s", reserved->socket_tab[pos].remote_ip);
          eXosip_set_option (excontext, EXOSIP_OPT_ADD_DNS_CACHE, (void *) &entry);
        }
      }
      
      i = _tcp_tl_send (excontext, out_socket, (const void *) message, (int) length);
      if (i<0) {
        if (pos >= 0) _tcp_tl_close_sockinfo (&reserved->socket_tab[pos]);
      }
      
      if (i==0 && tr!=NULL && MSG_IS_REGISTER(sip) && pos>=0) {
        /* start a timeout to destroy connection if no answer */
        reserved->socket_tab[pos].tcp_max_timeout = osip_getsystemtime (NULL) + 32;
      }
      
      osip_free (message);
      return i;
    }
    
#ifdef ENABLE_KEEP_ALIVE_OPTIONS_METHOD
    static int
    _tcp_tl_get_socket_info (int socket, char *host, int hostsize, int *port)
    {
      struct sockaddr addr;
      int nameLen = sizeof (addr);
      int ret;
      
      if (socket <= 0 || host == NULL || hostsize <= 0 || port == NULL)
        return OSIP_BADPARAMETER;
      ret = getsockname (socket, &addr, &nameLen);
      if (ret != 0) {
        /* ret = ex_errno; */
        return OSIP_UNDEFINED_ERROR;
      }
      else {
        ret = getnameinfo ((struct sockaddr *) &addr, nameLen, host, hostsize, NULL, 0, NI_NUMERICHOST);
        if (ret != 0)
          return OSIP_UNDEFINED_ERROR;
        
        if (addr.sa_family == AF_INET)
          (*port) = ntohs (((struct sockaddr_in *) &addr)->sin_port);
        else
          (*port) = ntohs (((struct sockaddr_in6 *) &addr)->sin6_port);
      }
      return OSIP_SUCCESS;
    }
#endif
    
    static int
    tcp_tl_keepalive (struct eXosip_t *excontext)
    {
      struct eXtltcp *reserved = (struct eXtltcp *) excontext->eXtltcp_reserved;
      char buf[5] = "\r\n\r\n";
      int pos;
      int i;
      
      if (reserved == NULL) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
        return OSIP_WRONG_STATE;
      }
      
      if (reserved->tcp_socket <= 0)
        return OSIP_UNDEFINED_ERROR;
      
      for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
        if (reserved->socket_tab[pos].socket > 0) {
          i = _tcp_tl_is_connected (reserved->socket_tab[pos].socket);
          if (i > 0) {
            OSIP_TRACE (osip_trace
                        (__FILE__, __LINE__, OSIP_INFO2, NULL, "tcp_tl_keepalive socket node:%s:%i, socket %d [pos=%d], in progress\n", reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port, reserved->socket_tab[pos].socket, pos));
            continue;
          }
          else if (i == 0) {
            OSIP_TRACE (osip_trace
                        (__FILE__, __LINE__, OSIP_INFO2, NULL, "tcp_tl_keepalive socket node:%s:%i , socket %d [pos=%d], connected\n", reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port, reserved->socket_tab[pos].socket, pos));
            reserved->socket_tab[pos].tcp_inprogress_max_timeout=0;
          }
          else {
            OSIP_TRACE (osip_trace
                        (__FILE__, __LINE__, OSIP_ERROR, NULL, "tcp_tl_keepalive socket node:%s:%i, socket %d [pos=%d], socket error\n", reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port, reserved->socket_tab[pos].socket, pos));
#if TARGET_OS_IPHONE
            _eXosip_mark_registration_expired (excontext, reserved->socket_tab[pos].reg_call_id);
#endif
            _tcp_tl_close_sockinfo (&reserved->socket_tab[pos]);
            continue;
          }
          if (excontext->ka_interval > 0) {
#ifdef ENABLE_KEEP_ALIVE_OPTIONS_METHOD
            if (excontext->ka_options != 0) {
              osip_message_t *options;
              char from[NI_MAXHOST];
              char to[NI_MAXHOST];
              char locip[NI_MAXHOST];
              int locport;
              char *message;
              size_t length;
              
              options = NULL;
              memset (to, '\0', sizeof (to));
              memset (from, '\0', sizeof (from));
              memset (locip, '\0', sizeof (locip));
              locport = 0;
              
              snprintf (to, sizeof (to), "<sip:%s:%d>", reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port);
              _tcp_tl_get_socket_info (reserved->socket_tab[pos].socket, locip, sizeof (locip), &locport);
              if (locip[0] == '\0') {
                OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "tcp_tl_keepalive socket node:%s , socket %d [pos=%d], failed to create sip options message\n", reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].socket, pos));
                continue;
              }
              
              snprintf (from, sizeof (from), "<sip:%s:%d>", locip, locport);
              
              eXosip_lock (excontext);
              /* Generate an options message */
              if (eXosip_options_build_request (excontext, &options, to, from, NULL) == OSIP_SUCCESS) {
                message = NULL;
                length = 0;
                /* Convert message to str for direct sending over correct socket */
                if (osip_message_to_str (options, &message, &length) == OSIP_SUCCESS) {
                  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "tcp_tl_keepalive socket node:%s , socket %d [pos=%d], sending sip options\n\r%s", reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].socket, pos, message));
                  i = send (reserved->socket_tab[pos].socket, (const void *) message, length, 0);
                  osip_free (message);
                  if (i > 0) {
                    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "eXosip: Keep Alive sent on TCP!\n"));
                  }
                }
                else {
                  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "tcp_tl_keepalive socket node:%s , socket %d [pos=%d], failed to convert sip options message\n", reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].socket, pos));
                }
              }
              else {
                OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "tcp_tl_keepalive socket node:%s , socket %d [pos=%d], failed to create sip options message\n", reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].socket, pos));
              }
              eXosip_unlock (excontext);
              continue;
            }
#endif
            i = (int) send (reserved->socket_tab[pos].socket, (const void *) buf, 4, 0);
          }
        }
      }
      return OSIP_SUCCESS;
    }
    
    static int
    tcp_tl_set_socket (struct eXosip_t *excontext, int socket)
    {
      struct eXtltcp *reserved = (struct eXtltcp *) excontext->eXtltcp_reserved;
      
      if (reserved == NULL) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
        return OSIP_WRONG_STATE;
      }
      
      reserved->tcp_socket = socket;
      
      return OSIP_SUCCESS;
    }
    
    static int
    tcp_tl_masquerade_contact (struct eXosip_t *excontext, const char *public_address, int port)
    {
      if (public_address == NULL || public_address[0] == '\0') {
        memset (excontext->tcp_firewall_ip, '\0', sizeof (excontext->tcp_firewall_ip));
        memset (excontext->tcp_firewall_port, '\0', sizeof (excontext->tcp_firewall_port));
        return OSIP_SUCCESS;
      }
      snprintf (excontext->tcp_firewall_ip, sizeof (excontext->tcp_firewall_ip), "%s", public_address);
      if (port > 0) {
        snprintf (excontext->tcp_firewall_port, sizeof (excontext->tcp_firewall_port), "%i", port);
      }
      return OSIP_SUCCESS;
    }
    
    static int
    tcp_tl_get_masquerade_contact (struct eXosip_t *excontext, char *ip, int ip_size, char *port, int port_size)
    {
      struct eXtltcp *reserved = (struct eXtltcp *) excontext->eXtltcp_reserved;
      
      memset (ip, 0, ip_size);
      memset (port, 0, port_size);
      
      if (reserved == NULL) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
        return OSIP_WRONG_STATE;
      }
      
      if (excontext->tcp_firewall_ip[0] != '\0')
        snprintf (ip, ip_size, "%s", excontext->tcp_firewall_ip);
      
      if (excontext->tcp_firewall_port[0] != '\0')
        snprintf (port, port_size, "%s", excontext->tcp_firewall_port);
      return OSIP_SUCCESS;
    }
    
    static int
    tcp_tl_update_contact (struct eXosip_t *excontext, osip_message_t * req)
    {
      req->application_data = (void*) 0x1; /* request for masquerading */
      return OSIP_SUCCESS;
    }
    
    static int
    tcp_tl_check_connection (struct eXosip_t *excontext)
    {
      struct eXtltcp *reserved = (struct eXtltcp *) excontext->eXtltcp_reserved;
      int pos;
      int i;
      
      if (reserved == NULL) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
        return OSIP_WRONG_STATE;
      }
      
      if (reserved->tcp_socket <= 0)
        return OSIP_UNDEFINED_ERROR;
      
      for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
        if (reserved->socket_tab[pos].socket > 0) {
          i = _tcp_tl_is_connected (reserved->socket_tab[pos].socket);
          if (i > 0) {
            if (reserved->socket_tab[pos].tcp_inprogress_max_timeout>0) {
              time_t now = osip_getsystemtime (NULL);
              if (now > reserved->socket_tab[pos].tcp_inprogress_max_timeout) {
                OSIP_TRACE (osip_trace
                            (__FILE__, __LINE__, OSIP_INFO2, NULL, "tcp_tl_check_connection socket is in progress since 32 seconds / close socket\n"));
                reserved->socket_tab[pos].tcp_inprogress_max_timeout=0;
                _eXosip_mark_registration_expired (excontext, reserved->socket_tab[pos].reg_call_id);
                _tcp_tl_close_sockinfo (&reserved->socket_tab[pos]);
                continue;
              }
            }
            OSIP_TRACE (osip_trace
                        (__FILE__, __LINE__, OSIP_INFO2, NULL, "tcp_tl_check_connection socket node:%s:%i, socket %d [pos=%d], in progress\n", reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port, reserved->socket_tab[pos].socket, pos));
            continue;
          }
          else if (i == 0) {
            reserved->socket_tab[pos].tcp_inprogress_max_timeout=0;
            
            OSIP_TRACE (osip_trace
                        (__FILE__, __LINE__, OSIP_INFO2, NULL, "tcp_tl_check_connection socket node:%s:%i , socket %d [pos=%d], connected\n", reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port, reserved->socket_tab[pos].socket, pos));
            if (reserved->socket_tab[pos].tcp_max_timeout>0) {
              time_t now = osip_getsystemtime (NULL);
              if (now > reserved->socket_tab[pos].tcp_max_timeout) {
                OSIP_TRACE (osip_trace
                            (__FILE__, __LINE__, OSIP_INFO2, NULL, "tcp_tl_check_connection we excepted a reply on established sockets / close socket\n"));
                reserved->socket_tab[pos].tcp_max_timeout=0;
                _eXosip_mark_registration_expired (excontext, reserved->socket_tab[pos].reg_call_id);
                _tcp_tl_close_sockinfo (&reserved->socket_tab[pos]);
                continue;
              }
            }
          }
          else {
            OSIP_TRACE (osip_trace
                        (__FILE__, __LINE__, OSIP_ERROR, NULL, "tcp_tl_check_connection socket node:%s:%i, socket %d [pos=%d], socket error\n", reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port, reserved->socket_tab[pos].socket, pos));
#if TARGET_OS_IPHONE
            _eXosip_mark_registration_expired (excontext, reserved->socket_tab[pos].reg_call_id);
#endif
            _tcp_tl_close_sockinfo (&reserved->socket_tab[pos]);
            continue;
          }
        }
      }
      return OSIP_SUCCESS;
    }
    
    
    static struct eXtl_protocol eXtl_tcp = {
      1,
      5060,
      "TCP",
      "0.0.0.0",
      IPPROTO_TCP,
      AF_INET,
      0,
      0,
      0,
      
      &tcp_tl_init,
      &tcp_tl_free,
      &tcp_tl_open,
      &tcp_tl_set_fdset,
      &tcp_tl_read_message,
      &tcp_tl_send_message,
      &tcp_tl_keepalive,
      &tcp_tl_set_socket,
      &tcp_tl_masquerade_contact,
      &tcp_tl_get_masquerade_contact,
      &tcp_tl_update_contact,
      &tcp_tl_reset,
      &tcp_tl_check_connection
    };
    
    void
    eXosip_transport_tcp_init (struct eXosip_t *excontext)
    {
      memcpy (&excontext->eXtl_transport, &eXtl_tcp, sizeof (struct eXtl_protocol));
    }
