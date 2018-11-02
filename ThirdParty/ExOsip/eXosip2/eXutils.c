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

#if defined(HAVE_STDARG_H)
#include <stdarg.h>
#define VA_START(a, f)  va_start(a, f)
#else
#if defined(HAVE_VARARGS_H)
#include <varargs.h>
#define VA_START(a, f) va_start(a)
#else
#include <stdarg.h>
#define VA_START(a, f)  va_start(a, f)
#endif
#endif

#include <osipparser2/osip_port.h>

#if defined(WIN32) && !defined(_WIN32_WCE)
#define HAVE_WINDNS_H
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
#undef HAVE_WINDNS_H
#endif
#endif

#if defined(WIN32)
#define HAVE_IPHLPAPI_H
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
#undef HAVE_IPHLPAPI_H
#endif
#endif

#if defined(WIN32)
#if defined(HAVE_WINDNS_H)
#include <windns.h>
#include <malloc.h>
#endif
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#ifdef HAVE_ARPA_NAMESER_H
#include <arpa/nameser.h>
#endif

#ifdef HAVE_NAMESER8_COMPAT_H
#include <nameser8_compat.h>
#include <resolv8_compat.h>
#elif defined(HAVE_RESOLV_H) || defined(OpenBSD) || defined(FreeBSD) || defined(NetBSD)
#ifdef HAVE_ARPA_NAMESER_COMPAT_H
#include <arpa/nameser_compat.h>
#endif
#include <resolv.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#endif

#ifdef TSC_SUPPORT
#include "tsc_socket_api.h"
#include "tsc_control_api.h"
#endif

#if defined(__arc__)
#define USE_GETHOSTBYNAME
#endif

int
_eXosip_closesocket(SOCKET_TYPE sock) {
#if	!defined(_WIN32) && !defined(_WIN32_WCE)
  return close (sock);
#else
  return closesocket(sock);
#endif
}

#if defined(USE_GETHOSTBYNAME)

void
_eXosip_freeaddrinfo (struct addrinfo *ai)
{
  struct addrinfo *next;

  while (ai) {
    next = ai->ai_next;
    free (ai);
    ai = next;
  }
}

struct namebuf {
  struct hostent hostentry;
  char *h_addr_list[2];
  struct in_addr addrentry;
  char h_name[16];              /* 123.123.123.123 = 15 letters is maximum */
};

static struct addrinfo *
osip_he2ai (struct hostent *he, int port, int protocol)
{
  struct addrinfo *ai;

  struct addrinfo *prevai = NULL;

  struct addrinfo *firstai = NULL;

  struct sockaddr_in *addr;

  int i;

  struct in_addr *curr;

  if (!he)
    /* no input == no output! */
    return NULL;

  for (i = 0; (curr = (struct in_addr *) he->h_addr_list[i]); i++) {

    ai = calloc (1, sizeof (struct addrinfo) + sizeof (struct sockaddr_in));

    if (!ai)
      break;

    if (!firstai)
      /* store the pointer we want to return from this function */
      firstai = ai;

    if (prevai)
      /* make the previous entry point to this */
      prevai->ai_next = ai;

    ai->ai_family = AF_INET;    /* we only support this */
    if (protocol == IPPROTO_UDP)
      ai->ai_socktype = SOCK_DGRAM;
    else
      ai->ai_socktype = SOCK_STREAM;
    ai->ai_addrlen = sizeof (struct sockaddr_in);
    /* make the ai_addr point to the address immediately following this struct
       and use that area to store the address */
    ai->ai_addr = (struct sockaddr *) ((char *) ai + sizeof (struct addrinfo));

    /* leave the rest of the struct filled with zero */

    addr = (struct sockaddr_in *) ai->ai_addr;  /* storage area for this info */

    memcpy ((char *) &(addr->sin_addr), curr, sizeof (struct in_addr));
    addr->sin_family = he->h_addrtype;
    addr->sin_port = htons ((unsigned short) port);

    prevai = ai;
  }
  return firstai;
}

/*
 * osip_ip2addr() takes a 32bit ipv4 internet address as input parameter
 * together with a pointer to the string version of the address, and it
 * returns a struct addrinfo chain filled in correctly with information for this
 * address/host.
 *
 * The input parameters ARE NOT checked for validity but they are expected
 * to have been checked already when this is called.
 */
static struct addrinfo *
osip_ip2addr (in_addr_t num, const char *hostname, int port, int protocol)
{
  struct addrinfo *ai;
  struct hostent *h;
  struct in_addr *addrentry;
  struct namebuf buffer;
  struct namebuf *buf = &buffer;

  h = &buf->hostentry;
  h->h_addr_list = &buf->h_addr_list[0];
  addrentry = &buf->addrentry;
  addrentry->s_addr = num;
  h->h_addr_list[0] = (char *) addrentry;
  h->h_addr_list[1] = NULL;
  h->h_addrtype = AF_INET;
  h->h_length = sizeof (*addrentry);
  h->h_name = &buf->h_name[0];
  h->h_aliases = NULL;

  /* Now store the dotted version of the address */
  snprintf ((char *) h->h_name, 16, "%s", hostname);

  ai = osip_he2ai (h, port, protocol);
  return ai;
}

static int
eXosip_inet_pton (int family, const char *src, void *dst)
{
  if (strchr (src, ':'))        /* possible IPv6 address */
    return OSIP_UNDEFINED_ERROR;        /* (inet_pton(AF_INET6, src, dst)); */
  else if (strchr (src, '.')) { /* possible IPv4 address */
    struct in_addr *tmp = dst;

    tmp->s_addr = inet_addr (src);      /* already in N. byte order */
    if (tmp->s_addr == INADDR_NONE)
      return 0;

    return 1;                   /* (inet_pton(AF_INET, src, dst)); */
  }
  else                          /* Impossibly a valid ip address */
    return INADDR_NONE;
}

/*
 * osip_getaddrinfo() - the ipv4 synchronous version.
 *
 * The original code to this function was from the Dancer source code, written
 * by Bjorn Reese, it has since been patched and modified considerably.
 *
 * gethostbyname_r() is the thread-safe version of the gethostbyname()
 * function. When we build for plain IPv4, we attempt to use this
 * function. There are _three_ different gethostbyname_r() versions, and we
 * detect which one this platform supports in the configure script and set up
 * the HAVE_GETHOSTBYNAME_R_3, HAVE_GETHOSTBYNAME_R_5 or
 * HAVE_GETHOSTBYNAME_R_6 defines accordingly. Note that HAVE_GETADDRBYNAME
 * has the corresponding rules. This is primarily on *nix. Note that some unix
 * flavours have thread-safe versions of the plain gethostbyname() etc.
 *
 */
int
_eXosip_get_addrinfo (struct eXosip_t *excontext, struct addrinfo **addrinfo, const char *hostname, int port, int protocol)
{
  struct hostent *h = NULL;

  in_addr_t in;

  struct hostent *buf = NULL;

  char portbuf[10];

  *addrinfo = NULL;             /* default return */

  if (port < 0)                 /* -1 for SRV record */
    return OSIP_BADPARAMETER;

  snprintf (portbuf, sizeof (portbuf), "%i", port);

  if (1 == eXosip_inet_pton (AF_INET, hostname, &in))
    /* This is a dotted IP address 123.123.123.123-style */
  {
    *addrinfo = osip_ip2addr (in, hostname, port, protocol);
    return OSIP_SUCCESS;
  }
#if defined(HAVE_GETHOSTBYNAME_R)
  /*
   * gethostbyname_r() is the preferred resolve function for many platforms.
   * Since there are three different versions of it, the following code is
   * somewhat #ifdef-ridden.
   */
  else {
    int h_errnop;

    int res = ERANGE;

    buf = (struct hostent *) calloc (CURL_HOSTENT_SIZE, 1);
    if (!buf)
      return NULL;              /* major failure */
    /*
     * The clearing of the buffer is a workaround for a gethostbyname_r bug in
     * qnx nto and it is also _required_ for some of these functions on some
     * platforms.
     */

#ifdef HAVE_GETHOSTBYNAME_R_5
    /* Solaris, IRIX and more */
    (void) res;                 /* prevent compiler warning */
    h = gethostbyname_r (hostname, (struct hostent *) buf, (char *) buf + sizeof (struct hostent), CURL_HOSTENT_SIZE - sizeof (struct hostent), &h_errnop);

    /* If the buffer is too small, it returns NULL and sets errno to
     * ERANGE. The errno is thread safe if this is compiled with
     * -D_REENTRANT as then the 'errno' variable is a macro defined to get
     * used properly for threads.
     */

    if (h) {
      ;
    }
    else
#endif /* HAVE_GETHOSTBYNAME_R_5 */
#ifdef HAVE_GETHOSTBYNAME_R_6
      /* Linux */

      res = gethostbyname_r (hostname, (struct hostent *) buf, (char *) buf + sizeof (struct hostent), CURL_HOSTENT_SIZE - sizeof (struct hostent), &h, /* DIFFERENCE */
                             &h_errnop);
    /* Redhat 8, using glibc 2.2.93 changed the behavior. Now all of a
     * sudden this function returns EAGAIN if the given buffer size is too
     * small. Previous versions are known to return ERANGE for the same
     * problem.
     *
     * This wouldn't be such a big problem if older versions wouldn't
     * sometimes return EAGAIN on a common failure case. Alas, we can't
     * assume that EAGAIN *or* ERANGE means ERANGE for any given version of
     * glibc.
     *
     * For now, we do that and thus we may call the function repeatedly and
     * fail for older glibc versions that return EAGAIN, until we run out of
     * buffer size (step_size grows beyond CURL_HOSTENT_SIZE).
     *
     * If anyone has a better fix, please tell us!
     *
     * -------------------------------------------------------------------
     *
     * On October 23rd 2003, Dan C dug up more details on the mysteries of
     * gethostbyname_r() in glibc:
     *
     * In glibc 2.2.5 the interface is different (this has also been
     * discovered in glibc 2.1.1-6 as shipped by Redhat 6). What I can't
     * explain, is that tests performed on glibc 2.2.4-34 and 2.2.4-32
     * (shipped/upgraded by Redhat 7.2) don't show this behavior!
     *
     * In this "buggy" version, the return code is -1 on error and 'errno'
     * is set to the ERANGE or EAGAIN code. Note that 'errno' is not a
     * thread-safe variable.
     */

    if (!h)                     /* failure */
#endif /* HAVE_GETHOSTBYNAME_R_6 */
#ifdef HAVE_GETHOSTBYNAME_R_3
      /* AIX, Digital Unix/Tru64, HPUX 10, more? */

      /* For AIX 4.3 or later, we don't use gethostbyname_r() at all, because of
       * the plain fact that it does not return unique full buffers on each
       * call, but instead several of the pointers in the hostent structs will
       * point to the same actual data! This have the unfortunate down-side that
       * our caching system breaks down horribly. Luckily for us though, AIX 4.3
       * and more recent versions have a "completely thread-safe"[*] libc where
       * all the data is stored in thread-specific memory areas making calls to
       * the plain old gethostbyname() work fine even for multi-threaded
       * programs.
       *
       * This AIX 4.3 or later detection is all made in the configure script.
       *
       * Troels Walsted Hansen helped us work this out on March 3rd, 2003.
       *
       * [*] = much later we've found out that it isn't at all "completely
       * thread-safe", but at least the gethostbyname() function is.
       */

      if (CURL_HOSTENT_SIZE >= (sizeof (struct hostent) + sizeof (struct hostent_data))) {

        /* August 22nd, 2000: Albert Chin-A-Young brought an updated version
         * that should work! September 20: Richard Prescott worked on the buffer
         * size dilemma.
         */

        res = gethostbyname_r (hostname, (struct hostent *) buf, (struct hostent_data *) ((char *) buf + sizeof (struct hostent)));
        h_errnop = errno;       /* we don't deal with this, but set it anyway */
      }
      else
        res = -1;               /* failure, too smallish buffer size */

    if (!res) {                 /* success */

      h = buf;                  /* result expected in h */

      /* This is the worst kind of the different gethostbyname_r() interfaces.
       * Since we don't know how big buffer this particular lookup required,
       * we can't realloc down the huge alloc without doing closer analysis of
       * the returned data. Thus, we always use CURL_HOSTENT_SIZE for every
       * name lookup. Fixing this would require an extra malloc() and then
       * calling struct addrinfo_copy() that subsequent realloc()s down the new
       * memory area to the actually used amount.
       */
    }
    else
#endif /* HAVE_GETHOSTBYNAME_R_3 */
    {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "gethostbyname failure. %s:%s (%s)\n", hostname, port));
      h = NULL;                 /* set return code to NULL */
      free (buf);
    }
#else /* HAVE_GETHOSTBYNAME_R */
  /*
   * Here is code for platforms that don't have gethostbyname_r() or for
   * which the gethostbyname() is the preferred() function.
   */
  else {
    h = NULL;
#if !defined(__arc__)
    h = gethostbyname (hostname);
#endif
    if (!h) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "gethostbyname failure. %s:%s (%s)\n", hostname, port));
    }
#endif /*HAVE_GETHOSTBYNAME_R */
  }

  if (h) {
    *addrinfo = osip_he2ai (h, port, protocol);

    if (buf)                    /* used a *_r() function */
      free (buf);
  }

  return OSIP_SUCCESS;
}

#endif

int _eXosip_getport(const struct sockaddr *sa, socklen_t salen)
{
  if (sa->sa_family == AF_INET)
    return ntohs (((struct sockaddr_in *) sa)->sin_port);

  return ntohs (((struct sockaddr_in6 *) sa)->sin6_port);
}

#if defined(__arc__)

int _eXosip_getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags)
{
  struct sockaddr_in *fromsa = (struct sockaddr_in *) sa;
  char *tmp;

  tmp = inet_ntoa (fromsa->sin_addr);
  if (tmp == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "_eXosip_getnameinfo failure\n"));
    snprintf (host, hostlen, "127.0.0.1");
    return OSIP_UNDEFINED_ERROR;
  }

  snprintf (host, hostlen, "%s", tmp);
  return OSIP_SUCCESS;
}

#else
int _eXosip_getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags)
{
  int err;

  err = getnameinfo ((struct sockaddr *) sa, salen, host, hostlen, serv, servlen, flags);

  if (err != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "_eXosip_getnameinfo failure\n"));
    snprintf (host, hostlen, "127.0.0.1");
    return OSIP_UNDEFINED_ERROR;
  }
  return OSIP_SUCCESS;
}
#endif

int
_eXosip_guess_ip_for_via (struct eXosip_t *excontext, int family, char *address, int size)
{
  if (family == AF_INET)
    return _eXosip_guess_ip_for_destination (excontext, family, excontext->ipv4_for_gateway, address, size);

  return _eXosip_guess_ip_for_destination (excontext, family, excontext->ipv6_for_gateway, address, size);
}

#if defined(WIN32) || defined(_WIN32_WCE)

int
_eXosip_guess_ip_for_destination (struct eXosip_t *excontext, int family, char *destination, char *address, int size)
{
  SOCKET sock;

  SOCKADDR_STORAGE local_addr;

  int local_addr_len;

  struct addrinfo *addrf = NULL;

  address[0] = '\0';

  if (destination==NULL && family == AF_INET)
    destination = excontext->ipv4_for_gateway;
  if (destination==NULL && family == AF_INET6)
    destination = excontext->ipv6_for_gateway;

#ifdef TSC_SUPPORT
  if (excontext->tunnel_handle) {
    tsc_config config;

    tsc_get_config (excontext->tunnel_handle, &config);
    tsc_ip_address_to_str (&(config.internal_address), address, TSC_ADDR_STR_LEN);
    return 0;
  }
#endif

  sock = socket (family, SOCK_DGRAM, 0);

  if (family == AF_INET) {
    _eXosip_get_addrinfo (excontext, &addrf, destination, 0, IPPROTO_UDP);
  }
  else if (family == AF_INET6) {
    _eXosip_get_addrinfo (excontext, &addrf, destination, 0, IPPROTO_UDP);
  }

  if (addrf == NULL) {
    if (family == AF_INET) {
      _eXosip_get_addrinfo (excontext, &addrf, "217.12.3.11", 0, IPPROTO_UDP);
    }
    else if (family == AF_INET6) {
      _eXosip_get_addrinfo (excontext, &addrf, "2001:638:500:101:2e0:81ff:fe24:37c6", 0, IPPROTO_UDP);
    }
  }

  if (addrf == NULL) {
    _eXosip_closesocket (sock);
    snprintf (address, size, (family == AF_INET) ? "127.0.0.1" : "::1");
    return OSIP_NO_NETWORK;
  }

  if (WSAIoctl (sock, SIO_ROUTING_INTERFACE_QUERY, addrf->ai_addr, (DWORD)addrf->ai_addrlen, &local_addr, sizeof (local_addr), &local_addr_len, NULL, NULL) != 0) {
    _eXosip_closesocket (sock);
    _eXosip_freeaddrinfo (addrf);
    snprintf (address, size, (family == AF_INET) ? "127.0.0.1" : "::1");
    return OSIP_NO_NETWORK;
  }

  _eXosip_closesocket (sock);
  _eXosip_freeaddrinfo (addrf);

  if (getnameinfo ((const struct sockaddr *) &local_addr, local_addr_len, address, size, NULL, 0, NI_NUMERICHOST)) {
    snprintf (address, size, (family == AF_INET) ? "127.0.0.1" : "::1");
    return OSIP_NO_NETWORK;
  }

  return OSIP_SUCCESS;
}

int
_eXosip_guess_ip_for_destinationsock (struct eXosip_t *excontext, int family, int proto, struct sockaddr_storage *udp_local_bind, int sock, char *destination, char *address, int size)
{
  SOCKADDR_STORAGE local_addr;

  DWORD local_addr_len;
  
  struct addrinfo *addrf = NULL;

  address[0] = '\0';

  if (destination==NULL && family == AF_INET)
    destination = excontext->ipv4_for_gateway;
  if (destination==NULL && family == AF_INET6)
    destination = excontext->ipv6_for_gateway;

#ifdef TSC_SUPPORT
  if (excontext->tunnel_handle) {
    tsc_config config;

    tsc_get_config (excontext->tunnel_handle, &config);
    tsc_ip_address_to_str (&(config.internal_address), address, TSC_ADDR_STR_LEN);
    return 0;
  }
#endif

  if (family == AF_INET) {
    _eXosip_get_addrinfo (excontext, &addrf, destination, 0, proto);
  }
  else if (family == AF_INET6) {
    _eXosip_get_addrinfo (excontext, &addrf, destination, 0, proto);
  }

  if (addrf == NULL) {
    if (family == AF_INET) {
      _eXosip_get_addrinfo (excontext, &addrf, "217.12.3.11", 0, proto);
    }
    else if (family == AF_INET6) {
      _eXosip_get_addrinfo (excontext, &addrf, "2001:638:500:101:2e0:81ff:fe24:37c6", 0, proto);
    }
  }

  if (addrf == NULL) {
    snprintf (address, size, (family == AF_INET) ? "127.0.0.1" : "::1");
    return OSIP_NO_NETWORK;
  }

  if (WSAIoctl (sock, SIO_ROUTING_INTERFACE_QUERY, addrf->ai_addr, (DWORD)addrf->ai_addrlen, &local_addr, sizeof (local_addr), &local_addr_len, NULL, NULL) != 0) {
    _eXosip_freeaddrinfo (addrf);
    snprintf (address, size, (family == AF_INET) ? "127.0.0.1" : "::1");
    return OSIP_NO_NETWORK;
  }

  _eXosip_freeaddrinfo (addrf);

  if (getnameinfo ((const struct sockaddr *) &local_addr, (socklen_t)local_addr_len, address, size, NULL, 0, NI_NUMERICHOST)) {
    snprintf (address, size, (family == AF_INET) ? "127.0.0.1" : "::1");
    return OSIP_NO_NETWORK;
  }

  return OSIP_SUCCESS;
}

#else /* sun, *BSD, linux, and other? */


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <sys/param.h>

#include <stdio.h>

#ifdef HAVE_GETIFADDRS

#include <ifaddrs.h>
static int
_eXosip_default_gateway_with_getifaddrs (int type, char *address, int size)
{
  struct ifaddrs *ifp;

  struct ifaddrs *ifpstart;

  int ret = -1;

  if (getifaddrs (&ifpstart) < 0) {
    return OSIP_NO_NETWORK;
  }

  for (ifp = ifpstart; ifp != NULL; ifp = ifp->ifa_next) {
    if (ifp->ifa_addr && ifp->ifa_addr->sa_family == type && (ifp->ifa_flags & IFF_RUNNING) && !(ifp->ifa_flags & IFF_LOOPBACK)) {
      getnameinfo (ifp->ifa_addr, (type == AF_INET6) ? sizeof (struct sockaddr_in6) : sizeof (struct sockaddr_in), address, size, NULL, 0, NI_NUMERICHOST);
      if (strchr (address, '%') == NULL) {      /*avoid ipv6 link-local addresses */
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "_eXosip_default_gateway_with_getifaddrs(): found %s\n", address));
        ret = 0;
        break;
      }
    }
  }
  freeifaddrs (ifpstart);
  return ret;
}
#endif

/* This is a portable way to find the default gateway.
 * The ip of the default interface is returned.
 */
static int
  _eXosip_default_gateway_ipv4 (struct eXosip_t *excontext, char *destination, char *address, int size)
{
  socklen_t len;
  int sock_rt, on = 1;

  struct sockaddr_in iface_out;

  struct sockaddr_in remote;

  memset (&remote, 0, sizeof (struct sockaddr_in));

  remote.sin_family = AF_INET;
  remote.sin_addr.s_addr = inet_addr (destination);
  remote.sin_port = htons (11111);

  memset (&iface_out, 0, sizeof (iface_out));
  sock_rt = socket (AF_INET, SOCK_DGRAM, 0);

  if (setsockopt (sock_rt, SOL_SOCKET, SO_BROADCAST, &on, sizeof (on)) == -1) {
    _eXosip_closesocket (sock_rt);
    snprintf (address, size, "127.0.0.1");
    return OSIP_NO_NETWORK;
  }

  if (connect (sock_rt, (struct sockaddr *) &remote, sizeof (struct sockaddr_in)) == -1) {
    _eXosip_closesocket (sock_rt);
    snprintf (address, size, "127.0.0.1");
    return OSIP_NO_NETWORK;
  }

  len = sizeof (iface_out);
  if (getsockname (sock_rt, (struct sockaddr *) &iface_out, &len) == -1) {
    _eXosip_closesocket (sock_rt);
    snprintf (address, size, "127.0.0.1");
    return OSIP_NO_NETWORK;
  }

  _eXosip_closesocket (sock_rt);
  if (iface_out.sin_addr.s_addr == 0) { /* what is this case?? */
    snprintf (address, size, "127.0.0.1");
    return OSIP_NO_NETWORK;
  }
  osip_strncpy (address, inet_ntoa (iface_out.sin_addr), size - 1);
  return OSIP_SUCCESS;
}


/* This is a portable way to find the default gateway.
 * The ip of the default interface is returned.
 */
static int
_eXosip_default_gateway_ipv6 (struct eXosip_t *excontext, char *destination, char *address, int size)
{
  socklen_t len;
  int sock_rt, on = 1;

  struct sockaddr_in6 iface_out;

  struct sockaddr_in6 remote;

  memset (&remote, 0, sizeof (struct sockaddr_in6));

  remote.sin6_family = AF_INET6;
  inet_pton (AF_INET6, destination, &remote.sin6_addr);
  remote.sin6_port = htons (11111);

  memset (&iface_out, 0, sizeof (iface_out));
  sock_rt = socket (AF_INET6, SOCK_DGRAM, 0);
  /*default to ipv6 local loopback in case something goes wrong: */
  snprintf (address, size, "::1");
  if (setsockopt (sock_rt, SOL_SOCKET, SO_BROADCAST, &on, sizeof (on)) == -1) {
    _eXosip_closesocket (sock_rt);
    return OSIP_NO_NETWORK;
  }

  if (connect (sock_rt, (struct sockaddr *) &remote, sizeof (struct sockaddr_in6)) == -1) {
    _eXosip_closesocket (sock_rt);
    return OSIP_NO_NETWORK;
  }

  len = sizeof (iface_out);
  if (getsockname (sock_rt, (struct sockaddr *) &iface_out, &len) == -1) {
    _eXosip_closesocket (sock_rt);
    return OSIP_NO_NETWORK;
  }
  _eXosip_closesocket (sock_rt);

  inet_ntop (AF_INET6, (const void *) &iface_out.sin6_addr, address, size - 1);
  return OSIP_SUCCESS;
}

int
_eXosip_guess_ip_for_destination (struct eXosip_t *excontext, int family, char *destination, char *address, int size)
{
  int err;

  if (family == AF_INET6) {
    err = _eXosip_default_gateway_ipv6 (excontext, destination, address, size);
  }
  else {
    err = _eXosip_default_gateway_ipv4 (excontext, destination, address, size);
  }
#ifdef HAVE_GETIFADDRS
  if (err < 0)
    err = _eXosip_default_gateway_with_getifaddrs (family, address, size);
#endif
  return err;
}

/* This is a portable way to find the default gateway.
 * The ip of the default interface is returned.
 */
static int
  _eXosip_default_gateway_ipv4sock (struct eXosip_t *excontext, int proto, struct sockaddr_storage *udp_local_bind, int sock, char *destination, char *address, int size)
{
  socklen_t len;
  struct sockaddr_in iface_out;

  snprintf (address, size, "127.0.0.1");
  if (udp_local_bind!=NULL) {
    struct sockaddr_in remote;
    /* for udp, we use an independant socket with similar binding, because we can't connect the socket */
    
    memset (&remote, 0, sizeof (struct sockaddr_in));
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = inet_addr (destination);
    remote.sin_port = htons (11111);
    
    memcpy (&iface_out, udp_local_bind, sizeof (iface_out));
    len = sizeof (iface_out);
    iface_out.sin_port = htons (0);
    
    sock = socket (AF_INET, SOCK_DGRAM, proto);
    if (bind (sock, (struct sockaddr*)&iface_out, len)<0) {
      _eXosip_closesocket (sock);
      return OSIP_NO_NETWORK;
    }
    if (connect (sock, (struct sockaddr *) &remote, sizeof (struct sockaddr_in)) == -1) {
      _eXosip_closesocket (sock);
      return OSIP_NO_NETWORK;
    }
    len = sizeof (iface_out);
    if (getsockname (sock, (struct sockaddr *) &iface_out, &len) == -1) {
      _eXosip_closesocket (sock);
      return OSIP_NO_NETWORK;
    }
    
    _eXosip_closesocket (sock);
    if (iface_out.sin_addr.s_addr == 0) { /* what is this case?? */
      return OSIP_NO_NETWORK;
    }
    osip_strncpy (address, inet_ntoa (iface_out.sin_addr), size - 1);
    return OSIP_SUCCESS;
  }
  
  memset (&iface_out, 0, sizeof (iface_out));
  len = sizeof (iface_out);
  if (getsockname (sock, (struct sockaddr *) &iface_out, &len) == -1) {
    return OSIP_NO_NETWORK;
  }

  if (iface_out.sin_addr.s_addr == 0) { /* what is this case?? */
    return OSIP_NO_NETWORK;
  }
  osip_strncpy (address, inet_ntoa (iface_out.sin_addr), size - 1);
  return OSIP_SUCCESS;
}


/* This is a portable way to find the default gateway.
 * The ip of the default interface is returned.
 */
static int
_eXosip_default_gateway_ipv6sock (struct eXosip_t *excontext, int proto, struct sockaddr_storage *udp_local_bind, int sock, char *destination, char *address, int size)
{
  socklen_t len;
  struct sockaddr_in6 iface_out;

  snprintf (address, size, "::1");
  if (udp_local_bind!=NULL) {
    /* for udp, we use an independant socket with similar binding, because we can't connect the socket */
    struct sockaddr_in6 remote;

    memset (&remote, 0, sizeof (struct sockaddr_in6));
    remote.sin6_family = AF_INET6;
    inet_pton (AF_INET6, destination, &remote.sin6_addr);
    remote.sin6_port = htons (11111);

    memcpy (&iface_out, udp_local_bind, sizeof (iface_out));
    len = sizeof (iface_out);
    iface_out.sin6_port = htons (0);
    
    sock = socket (AF_INET6, SOCK_DGRAM, proto);
    if (bind(sock, (struct sockaddr*)&iface_out, len)<0) {
      _eXosip_closesocket (sock);
      return OSIP_NO_NETWORK;
    }
    
    if (connect (sock, (struct sockaddr *) &remote, sizeof (struct sockaddr_in6)) == -1) {
      _eXosip_closesocket (sock);
      return OSIP_NO_NETWORK;
    }
    
    len = sizeof (iface_out);
    if (getsockname (sock, (struct sockaddr *) &iface_out, &len) == -1) {
      _eXosip_closesocket (sock);
      return OSIP_NO_NETWORK;
    }
    _eXosip_closesocket (sock);
    
    inet_ntop (AF_INET6, (const void *) &iface_out.sin6_addr, address, size - 1);
    return OSIP_SUCCESS;
  }

  memset (&iface_out, 0, sizeof (iface_out));
  len = sizeof (iface_out);
  if (getsockname (sock, (struct sockaddr *) &iface_out, &len) == -1) {
    return OSIP_NO_NETWORK;
  }

  inet_ntop (AF_INET6, (const void *) &iface_out.sin6_addr, address, size - 1);
  return OSIP_SUCCESS;
}

int
_eXosip_guess_ip_for_destinationsock (struct eXosip_t *excontext, int family, int proto, struct sockaddr_storage *udp_local_bind, int sock, char *destination, char *address, int size)
{
  int err;

  if (family == AF_INET6) {
    err = _eXosip_default_gateway_ipv6sock (excontext, proto, udp_local_bind, sock, destination, address, size);
  }
  else {
    err = _eXosip_default_gateway_ipv4sock (excontext, proto, udp_local_bind, sock, destination, address, size);
  }
#ifdef HAVE_GETIFADDRS
  if (err < 0)
    err = _eXosip_default_gateway_with_getifaddrs (family, address, size);
#endif
  return err;
}

#endif

char *
_eXosip_strdup_printf (const char *fmt, ...)
{
  /* Guess we need no more than 100 bytes. */
  int n, size = 100;

  char *p;

  va_list ap;

  if ((p = osip_malloc (size)) == NULL)
    return NULL;
  while (1) {
    /* Try to print in the allocated space. */
    VA_START (ap, fmt);
#ifdef WIN32
    n = _vsnprintf (p, size, fmt, ap);
#else
    n = vsnprintf (p, size, fmt, ap);
#endif
    va_end (ap);
    /* If that worked, return the string. */
    if (n > -1 && n < size)
      return p;
    /* Else try again with more space. */
    if (n > -1)                 /* glibc 2.1 */
      size = n + 1;             /* precisely what is needed */
    else                        /* glibc 2.0 */
      size *= 2;                /* twice the old size */
    if ((p = osip_realloc (p, size)) == NULL)
      return NULL;
  }
}

#if !defined(USE_GETHOSTBYNAME)

static int
  _exosip_isipv4addr(const char *ip)
{
  int i;

  for(i = 0; i < 4 && *ip != '\0'; i++) {
    char c=*ip;
    while(*ip != '\0' && (c>='0') && (c<='9'))
      ip++;

    if(*ip != '\0') {
      if(*ip == '.' && i < 3)
        ip++;
      else
        break;
    }
  }

  if(i == 4 && *ip == '\0')
    return 1;

  return 0;
}

int
_eXosip_get_addrinfo (struct eXosip_t *excontext, struct addrinfo **addrinfo, const char *hostname, int service, int protocol)
{
  struct addrinfo hints;
  char portbuf[10];
  int error;
  int i;

  if (hostname == NULL)
    return OSIP_BADPARAMETER;

  if (service == -1) {          /* -1 for SRV record */
    /* obsolete code: make an SRV record? */
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "_eXosip_get_addrinfo: obsolete code?\n"));
    return -1;
  }

  if (excontext != NULL) {
    for (i = 0; i < MAX_EXOSIP_DNS_ENTRY; i++) {
      if (excontext->dns_entries[i].host[0] != '\0' && 0 == osip_strcasecmp (excontext->dns_entries[i].host, hostname)) {
        /* update entry */
        if (excontext->dns_entries[i].ip[0] != '\0') {
          hostname = excontext->dns_entries[i].ip;
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "eXosip option set: dns cache used:%s -> %s\n", excontext->dns_entries[i].host, excontext->dns_entries[i].ip));
        }
      }
    }
  }

  snprintf (portbuf, sizeof (portbuf), "%i", service);

  memset (&hints, 0, sizeof (hints));

  hints.ai_flags = 0;

  if (excontext->ipv6_enable)
    hints.ai_family = PF_INET6;
  else
    hints.ai_family = PF_INET;  /* ipv4 only support */

  if (strchr(hostname, ':')!=NULL) /* it's an IPv6 address... */
    hints.ai_family = PF_INET6;
  else if (_exosip_isipv4addr(hostname))
    hints.ai_family = PF_INET;  /* it's an IPv4 address... */

  if (protocol == IPPROTO_UDP)
    hints.ai_socktype = SOCK_DGRAM;
  else
    hints.ai_socktype = SOCK_STREAM;

  hints.ai_protocol = protocol; /* IPPROTO_UDP or IPPROTO_TCP */
  error = getaddrinfo (hostname, portbuf, &hints, addrinfo);
  if (osip_strcasecmp (hostname, "0.0.0.0") != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "DNS resolution with %s:%i\n", hostname, service));
  }

  if (error || *addrinfo == NULL) {
#if defined(HAVE_RESOLV_H)
    /* When a DNS server has changed after roaming to a new network. The
       new one should be automatically used. However, a few system are not
       doing this automatically so, when the DNS server is not accessible,
       we force getaddrinfo to use the new one after the first failure. */
    if (error == EAI_AGAIN)
      res_init ();
#endif
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "getaddrinfo failure. %s:%s (%d)\n", hostname, portbuf, error));
    return OSIP_UNKNOWN_HOST;
  }
  else {
    struct addrinfo *elem;

    char tmp[INET6_ADDRSTRLEN];

    char porttmp[10];

    for (elem = *addrinfo; elem != NULL; elem = elem->ai_next) {
      getnameinfo (elem->ai_addr, (socklen_t)elem->ai_addrlen, tmp, sizeof (tmp), porttmp, sizeof (porttmp), NI_NUMERICHOST | NI_NUMERICSERV);
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "getaddrinfo returned: %s port %s\n", tmp, porttmp));
    }
  }

  /* only one address (?) */
  if (excontext->tunnel_handle) {
    (*addrinfo)->ai_next = 0;

    return 0;
  }

  return OSIP_SUCCESS;
}
#endif

static void
osip_srv_record_sort (struct osip_srv_record *rec, int n)
{
  int i;
  int permuts;
  struct osip_srv_entry swap;

  do {
    struct osip_srv_entry *s1, *s2;

    permuts = 0;
    for (i = 0; i < n - 1; ++i) {
      s1 = &rec->srventry[i];
      s2 = &rec->srventry[i + 1];
      if (s1->priority > s2->priority) {
        memcpy (&swap, s1, sizeof (swap));
        memcpy (s1, s2, sizeof (swap));
        memcpy (s2, &swap, sizeof (swap));
        permuts++;
      }
    }
  } while (permuts != 0);
}


int
_eXosip_srv_lookup (struct eXosip_t *excontext, osip_message_t * sip, osip_naptr_t ** naptr_record)
{
  int use_srv = 1;
  char *host;
  osip_via_t *via;

  via = (osip_via_t *) osip_list_get (&sip->vias, 0);
  if (via == NULL || via->protocol == NULL)
    return OSIP_BADPARAMETER;

  if (MSG_IS_REQUEST (sip)) {
    osip_route_t *route;

    if (sip->sip_method == NULL)
      return OSIP_BADPARAMETER;

    osip_message_get_route (sip, 0, &route);
    if (route != NULL) {
      osip_uri_param_t *lr_param = NULL;

      osip_uri_uparam_get_byname (route->url, "lr", &lr_param);
      if (lr_param == NULL)
        route = NULL;
    }

    if (route != NULL) {
      if (route->url->port != NULL) {
        use_srv = 0;
      }
      host = route->url->host;
    }
    else {
      /* search for maddr parameter */
      osip_uri_param_t *maddr_param = NULL;

      osip_uri_uparam_get_byname (sip->req_uri, "maddr", &maddr_param);
      host = NULL;
      if (maddr_param != NULL && maddr_param->gvalue != NULL)
        host = maddr_param->gvalue;

      if (sip->req_uri->port != NULL) {
        use_srv = 0;
      }

      if (host == NULL)
        host = sip->req_uri->host;
    }
  }
  else {
    osip_generic_param_t *maddr;

    osip_generic_param_t *received;

    osip_generic_param_t *rport;

    osip_via_param_get_byname (via, "maddr", &maddr);
    osip_via_param_get_byname (via, "received", &received);
    osip_via_param_get_byname (via, "rport", &rport);
    if (maddr != NULL)
      host = maddr->gvalue;
    else if (received != NULL)
      host = received->gvalue;
    else
      host = via->host;

    if (via->port == NULL)
      use_srv = 0;
  }

  if (host == NULL) {
    return OSIP_UNKNOWN_HOST;
  }

  /* check if we have an IPv4 or IPv6 address */
  if (strchr (host, ':') || (INADDR_NONE != inet_addr (host))) {
    return OSIP_UNDEFINED_ERROR;
  }

  if (use_srv == 1) {
    int keep_in_cache = MSG_IS_REGISTER (sip) ? 1 : 0;

    /* Section 16.6 Request Forwarding (4. Record-Route)
       The URI placed in the Record-Route header field MUST resolve to
       the element inserting it (or a suitable stand-in) when the
       server location procedures of [4] are applied to it, so that
       subsequent requests reach the same SIP element. 

       Note: When the above doesn't appear to be true, check at least cache. */

    osip_generic_param_t *tag = NULL;

    osip_to_get_tag (sip->to, &tag);
    if (tag != NULL)            /* check cache only */
      *naptr_record = eXosip_dnsutils_naptr (excontext, host, "sip", via->protocol, -1);
    else
      *naptr_record = eXosip_dnsutils_naptr (excontext, host, "sip", via->protocol, keep_in_cache);

    return OSIP_SUCCESS;
  }
  return OSIP_UNDEFINED_ERROR;
}

int
eXosip_dnsutils_rotate_srv (osip_srv_record_t * srv_record)
{
  int n;

  if (srv_record->srventry[0].srv[0] == '\0')
    return -1;
  srv_record->index++;
  if (srv_record->srventry[srv_record->index].srv[0] == '\0')
    srv_record->index = 0;

  for (n = 1; n < 10 && srv_record->srventry[n].srv[0] != '\0'; n++) {
  }
  return n - 1;
}

#ifdef SRV_RECORD

static osip_list_t *dnsutils_list = NULL;

#if defined(HAVE_CARES_H) || defined(HAVE_ARES_H)

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_ARPA_NAMESER_H
#include <arpa/nameser.h>
#else
#include "nameser.h"
#endif
#ifdef HAVE_ARPA_NAMESER_COMPAT_H
#include <arpa/nameser_compat.h>
#endif

#ifndef HFIXEDSZ
#define HFIXEDSZ 12
#endif
#ifndef QFIXEDSZ
#define QFIXEDSZ 4
#endif
#ifndef RRFIXEDSZ
#define RRFIXEDSZ 10
#endif

#ifndef T_A
#define T_A 1
#endif

#ifndef T_AAAA
#define T_AAAA 28
#endif

#ifndef T_SRV
#define T_SRV 33
#endif

#ifndef T_NAPTR
#define T_NAPTR 35
#endif

#ifndef C_IN
#define C_IN 1
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <ares.h>
#include <ares_dns.h>

#ifdef _WIN32_WCE
#include "inet_ntop.h"
#elif WIN32
#include "inet_ntop.h"
#endif

#ifdef USE_WINSOCK
#define SOCKERRNO         ((int)WSAGetLastError())
#define SET_SOCKERRNO(x)  (WSASetLastError((int)(x)))
#else
#define SOCKERRNO         (errno)
#define SET_SOCKERRNO(x)  (errno = (x))
#endif

static const unsigned char *
skip_question (const unsigned char *aptr, const unsigned char *abuf, int alen)
{
  char *name;
  int status;
  long len;

  status = ares_expand_name (aptr, abuf, alen, &name, &len);
  if (status != ARES_SUCCESS)
    return NULL;
  aptr += len;

  ares_free_string (name);
  if (aptr + QFIXEDSZ > abuf + alen) {
    return NULL;
  }

  aptr += QFIXEDSZ;
  return aptr;
}

static const unsigned char *
save_A (osip_naptr_t * output_record, const unsigned char *aptr, const unsigned char *abuf, int alen)
{
  char rr_name[512];

  /* int dnsclass, ttl; */
  int type, dlen, status;
  long len;
  char addr[46];
  union {
    unsigned char *as_uchar;
    char *as_char;
  } name;

  /* Parse the RR name. */
  status = ares_expand_name (aptr, abuf, alen, &name.as_char, &len);
  if (status != ARES_SUCCESS)
    return NULL;
  aptr += len;

  if (aptr + RRFIXEDSZ > abuf + alen) {
    ares_free_string (name.as_char);
    return NULL;
  }

  type = DNS_RR_TYPE (aptr);
  /* dnsclass = DNS_RR_CLASS(aptr); */
  /* ttl = DNS_RR_TTL(aptr); */
  dlen = DNS_RR_LEN (aptr);
  aptr += RRFIXEDSZ;
  if (aptr + dlen > abuf + alen) {
    ares_free_string (name.as_char);
    return NULL;
  }

  snprintf (rr_name, sizeof (rr_name), "%s", name.as_char);
  ares_free_string (name.as_char);

  switch (type) {
  case T_A:
    /* The RR data is a four-byte Internet address. */
    {
      int n;
      osip_srv_entry_t *srventry;

      if (dlen != 4)
        return NULL;

      inet_ntop (AF_INET, aptr, addr, sizeof (addr));
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "save_A: A record %s -> %s\n", rr_name, addr));

      for (n = 0; n < 10; n++) {
        if (osip_strcasecmp (rr_name, output_record->sipudp_record.srventry[n].srv) == 0) {
          srventry = &output_record->sipudp_record.srventry[n];
          snprintf (srventry->ipaddress, sizeof (srventry->ipaddress), "%s", addr);
        }
        if (osip_strcasecmp (rr_name, output_record->siptcp_record.srventry[n].srv) == 0) {
          srventry = &output_record->siptcp_record.srventry[n];
          snprintf (srventry->ipaddress, sizeof (srventry->ipaddress), "%s", addr);
        }
        if (osip_strcasecmp (rr_name, output_record->siptls_record.srventry[n].srv) == 0) {
          srventry = &output_record->siptls_record.srventry[n];
          snprintf (srventry->ipaddress, sizeof (srventry->ipaddress), "%s", addr);
        }
        if (osip_strcasecmp (rr_name, output_record->sipdtls_record.srventry[n].srv) == 0) {
          srventry = &output_record->sipdtls_record.srventry[n];
          snprintf (srventry->ipaddress, sizeof (srventry->ipaddress), "%s", addr);
        }
        if (osip_strcasecmp (rr_name, output_record->sipsctp_record.srventry[n].srv) == 0) {
          srventry = &output_record->sipsctp_record.srventry[n];
          snprintf (srventry->ipaddress, sizeof (srventry->ipaddress), "%s", addr);
        }
      }
    }

    break;

  case T_AAAA:
    /* The RR data is a 16-byte IPv6 address. */
    if (dlen != 16)
      return NULL;

    inet_ntop (AF_INET6, aptr, addr, sizeof (addr));
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "save_AAAA: A record %s -> %s\n", rr_name, addr));
    break;

  default:
    break;
  }

  return aptr + dlen;
}

static const unsigned char *
save_SRV (osip_naptr_t * output_record, const unsigned char *aptr, const unsigned char *abuf, int alen)
{
  char rr_name[512];

  /* int dnsclass, ttl; */
  int type, dlen, status;
  long len;
  union {
    unsigned char *as_uchar;
    char *as_char;
  } name;

  status = ares_expand_name (aptr, abuf, alen, &name.as_char, &len);
  if (status != ARES_SUCCESS)
    return NULL;
  aptr += len;

  if (aptr + RRFIXEDSZ > abuf + alen) {
    ares_free_string (name.as_char);
    return NULL;
  }

  type = DNS_RR_TYPE (aptr);
  /* dnsclass = DNS_RR_CLASS(aptr); */
  /* ttl = DNS_RR_TTL(aptr); */
  dlen = DNS_RR_LEN (aptr);
  aptr += RRFIXEDSZ;
  if (aptr + dlen > abuf + alen) {
    ares_free_string (name.as_char);
    return NULL;
  }

  snprintf (rr_name, sizeof (rr_name), "%s", name.as_char);
  ares_free_string (name.as_char);

  switch (type) {
  case T_SRV:
    /* The RR data is three two-byte numbers representing the
     * priority, weight, and port, followed by a domain name.
     */
    {
      osip_srv_record_t *srvrecord = NULL;
      osip_srv_entry_t *srventry = NULL;
      int n;

      if (osip_strcasecmp (rr_name, output_record->sipudp_record.name) == 0)
        srvrecord = &output_record->sipudp_record;
      else if (osip_strcasecmp (rr_name, output_record->siptcp_record.name) == 0)
        srvrecord = &output_record->siptcp_record;
      else if (osip_strcasecmp (rr_name, output_record->siptls_record.name) == 0)
        srvrecord = &output_record->siptls_record;
      else if (osip_strcasecmp (rr_name, output_record->sipdtls_record.name) == 0)
        srvrecord = &output_record->sipdtls_record;
      else if (osip_strcasecmp (rr_name, output_record->sipsctp_record.name) == 0)
        srvrecord = &output_record->sipsctp_record;
      else
        break;

      n = 0;
      while (n < 10 && srvrecord->srventry[n].srv[0] != '\0')
        n++;
      if (n == 10)
        break;                  /* skip... */
      srventry = &srvrecord->srventry[n];

      srventry->priority = DNS__16BIT (aptr);
      srventry->weight = DNS__16BIT (aptr + 2);
      srventry->port = DNS__16BIT (aptr + 4);

      if (srventry->weight)
        srventry->rweight = 1 + rand () % (10000 * srventry->weight);
      else
        srventry->rweight = 0;

      status = ares_expand_name (aptr + 6, abuf, alen, &name.as_char, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      snprintf (srventry->srv, sizeof (srventry->srv), "%s", name.as_char);

      srvrecord->srv_state = OSIP_SRV_STATE_COMPLETED;

      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "save_SRV: SRV record %s IN SRV -> %s:%i/%i/%i/%i\n", rr_name, srventry->srv, srventry->port, srventry->priority, srventry->weight, srventry->rweight));

      osip_srv_record_sort (srvrecord, n + 1);
      ares_free_string (name.as_char);
    }
    break;
  default:
    break;
  }

  return aptr + dlen;
}

static const unsigned char *
save_NAPTR (osip_naptr_t * output_record, const unsigned char *aptr, const unsigned char *abuf, int alen)
{
  char rr_name[512];
  const unsigned char *p;

  /* int dnsclass, ttl; */
  int type, dlen, status;
  long len;
  union {
    unsigned char *as_uchar;
    char *as_char;
  } name;

  status = ares_expand_name (aptr, abuf, alen, &name.as_char, &len);
  if (status != ARES_SUCCESS)
    return NULL;
  aptr += len;

  if (aptr + RRFIXEDSZ > abuf + alen) {
    ares_free_string (name.as_char);
    return NULL;
  }

  type = DNS_RR_TYPE (aptr);
  /* dnsclass = DNS_RR_CLASS(aptr); */
  /* ttl = DNS_RR_TTL(aptr); */
  dlen = DNS_RR_LEN (aptr);
  aptr += RRFIXEDSZ;
  if (aptr + dlen > abuf + alen) {
    ares_free_string (name.as_char);
    return NULL;
  }

  snprintf (rr_name, sizeof (rr_name), "%s", name.as_char);
  ares_free_string (name.as_char);

  switch (type) {
  case T_NAPTR:
    {
      osip_srv_record_t srvrecord;

      memset (&srvrecord, 0, sizeof (osip_srv_record_t));

      srvrecord.order = DNS__16BIT (aptr);
      srvrecord.preference = DNS__16BIT (aptr + 2);

      p = aptr + 4;
      status = ares_expand_string (p, abuf, alen, &name.as_uchar, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      ares_free_string (name.as_char);
      p += len;

      status = ares_expand_string (p, abuf, alen, &name.as_uchar, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      snprintf (srvrecord.protocol, sizeof (srvrecord.protocol), "%s", name.as_char);
      ares_free_string (name.as_char);
      p += len;

      status = ares_expand_string (p, abuf, alen, &name.as_uchar, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      ares_free_string (name.as_char);
      p += len;

      status = ares_expand_name (p, abuf, alen, &name.as_char, &len);
      if (status != ARES_SUCCESS)
        return NULL;

      snprintf (srvrecord.name, sizeof (srvrecord.name), "%s", name.as_char);

      srvrecord.srv_state = OSIP_SRV_STATE_UNKNOWN;
      if (osip_strncasecmp(srvrecord.name, "_sip._udp.", 10)==0 || osip_strncasecmp (srvrecord.protocol, "SIP+D2U", 8) == 0) {   /* udp */
        memcpy (&output_record->sipudp_record, &srvrecord, sizeof (osip_srv_record_t));
      }
      else if (osip_strncasecmp(srvrecord.name, "_sip._tcp.", 10)==0 || osip_strncasecmp (srvrecord.protocol, "SIP+D2T", 8) == 0) {      /* tcp */
        memcpy (&output_record->siptcp_record, &srvrecord, sizeof (osip_srv_record_t));
      }
      else if (osip_strncasecmp (srvrecord.protocol, "SIPS+D2T", 9) == 0) {     /* tls */
        memcpy (&output_record->siptls_record, &srvrecord, sizeof (osip_srv_record_t));
      }
      else if (osip_strncasecmp (srvrecord.protocol, "SIPS+D2U", 9) == 0) {     /* dtls-udp */
        memcpy (&output_record->sipdtls_record, &srvrecord, sizeof (osip_srv_record_t));
      }
      else if (osip_strncasecmp (srvrecord.protocol, "SIP+D2S", 8) == 0) {      /* sctp */
        memcpy (&output_record->sipsctp_record, &srvrecord, sizeof (osip_srv_record_t));
      }

      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "save_NAPTR: NAPTR %s ->%i/%i/%s\n", rr_name, srvrecord.order, srvrecord.preference, srvrecord.name));

      ares_free_string (name.as_char);
    }
    break;

  default:
    break;
  }

  return aptr + dlen;
}

static void
_store_A (void *arg, int status, int timeouts, unsigned char *abuf, int alen, int verbose)
{
  osip_naptr_t *output_record = (osip_naptr_t *) arg;

#if 0
  int qr, aa, tc, rd, ra, opcode, rcode;        /* , id; */
#endif
  unsigned int qdcount, ancount, nscount, arcount, i;
  const unsigned char *aptr;

  (void) timeouts;

  if (status != ARES_SUCCESS) {
    if (verbose) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "DNS A: %s %s\n", output_record->domain, ares_strerror (status)));
    }
    if (!abuf)
      return;
  }

  if (alen < HFIXEDSZ)
    return;

#if 0
  /* id = DNS_HEADER_QID(abuf); */
  qr = DNS_HEADER_QR (abuf);
  opcode = DNS_HEADER_OPCODE (abuf);
  aa = DNS_HEADER_AA (abuf);
  tc = DNS_HEADER_TC (abuf);
  rd = DNS_HEADER_RD (abuf);
  ra = DNS_HEADER_RA (abuf);
  rcode = DNS_HEADER_RCODE (abuf);
#endif
  qdcount = DNS_HEADER_QDCOUNT (abuf);
  ancount = DNS_HEADER_ANCOUNT (abuf);
  nscount = DNS_HEADER_NSCOUNT (abuf);
  arcount = DNS_HEADER_ARCOUNT (abuf);

  /* the questions. */
  aptr = abuf + HFIXEDSZ;
  for (i = 0; i < qdcount; i++) {
    aptr = skip_question (aptr, abuf, alen);
    if (aptr == NULL)
      return;
  }

  /* the answers. */
  for (i = 0; i < ancount; i++) {
    aptr = save_A (output_record, aptr, abuf, alen);
    if (aptr == NULL)
      return;
  }

  /* the NS records. */
  for (i = 0; i < nscount; i++) {
    aptr = save_A (output_record, aptr, abuf, alen);
    if (aptr == NULL)
      return;
  }

  /* the additional records. */
  for (i = 0; i < arcount; i++) {
    aptr = save_A (output_record, aptr, abuf, alen);
    if (aptr == NULL)
      return;
  }
}

static void
_store_srv (void *arg, int status, int timeouts, unsigned char *abuf, int alen, int verbose)
{
  osip_naptr_t *output_record = (osip_naptr_t *) arg;

#if 0
  int qr, aa, tc, rd, ra, opcode, rcode;        /* , id; */
#endif  
  unsigned int qdcount, ancount, nscount, arcount, i;
  const unsigned char *aptr;

  (void) timeouts;

  if (status != ARES_SUCCESS) {
    if (verbose) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "DNS SRV: %s %s\n", output_record->domain, ares_strerror (status)));
    }
    if (!abuf)
      return;
  }

  if (alen < HFIXEDSZ)
    return;

#if 0
  /* id = DNS_HEADER_QID(abuf); */
  qr = DNS_HEADER_QR (abuf);
  opcode = DNS_HEADER_OPCODE (abuf);
  aa = DNS_HEADER_AA (abuf);
  tc = DNS_HEADER_TC (abuf);
  rd = DNS_HEADER_RD (abuf);
  ra = DNS_HEADER_RA (abuf);
  rcode = DNS_HEADER_RCODE (abuf);
#endif
  qdcount = DNS_HEADER_QDCOUNT (abuf);
  ancount = DNS_HEADER_ANCOUNT (abuf);
  nscount = DNS_HEADER_NSCOUNT (abuf);
  arcount = DNS_HEADER_ARCOUNT (abuf);

  /* the questions. */
  aptr = abuf + HFIXEDSZ;
  for (i = 0; i < qdcount; i++) {
    aptr = skip_question (aptr, abuf, alen);
    if (aptr == NULL)
      return;
  }

  /* the answers. */
  for (i = 0; i < ancount; i++) {
    aptr = save_SRV (output_record, aptr, abuf, alen);
    if (aptr == NULL)
      return;
  }

  /* the NS records. */
  for (i = 0; i < nscount; i++) {
    aptr = save_SRV (output_record, aptr, abuf, alen);
    if (aptr == NULL)
      return;
  }

  /* the additional records. */
  for (i = 0; i < arcount; i++) {
    aptr = save_SRV (output_record, aptr, abuf, alen);
    if (aptr == NULL)
      return;
  }
}

static void
_store_naptr (void *arg, int status, int timeouts, unsigned char *abuf, int alen, int verbose)
{
  osip_naptr_t *output_record = (osip_naptr_t *) arg;

#if 0
  int qr, aa, tc, rd, ra, opcode, rcode;        /* , id; */
#endif
  unsigned int qdcount, ancount, nscount, arcount, i;
  const unsigned char *aptr;

  (void) timeouts;

  if (status != ARES_SUCCESS) {
    if (verbose) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "DNS NAPTR: %s %s\n", output_record->domain, ares_strerror (status)));
    }
    if (!abuf)
      return;
  }

  if (alen < HFIXEDSZ)
    return;

#if 0
  /* id = DNS_HEADER_QID(abuf); */
  qr = DNS_HEADER_QR (abuf);
  opcode = DNS_HEADER_OPCODE (abuf);
  aa = DNS_HEADER_AA (abuf);
  tc = DNS_HEADER_TC (abuf);
  rd = DNS_HEADER_RD (abuf);
  ra = DNS_HEADER_RA (abuf);
  rcode = DNS_HEADER_RCODE (abuf);
#endif
  qdcount = DNS_HEADER_QDCOUNT (abuf);
  ancount = DNS_HEADER_ANCOUNT (abuf);
  nscount = DNS_HEADER_NSCOUNT (abuf);
  arcount = DNS_HEADER_ARCOUNT (abuf);

  /* the questions. */
  aptr = abuf + HFIXEDSZ;
  for (i = 0; i < qdcount; i++) {
    aptr = skip_question (aptr, abuf, alen);
    if (aptr == NULL)
      return;
  }

  /* the answers. */
  for (i = 0; i < ancount; i++) {
    aptr = save_NAPTR (output_record, aptr, abuf, alen);
    if (aptr == NULL)
      return;
  }

  /* the NS records. */
  for (i = 0; i < nscount; i++) {
    aptr = save_NAPTR (output_record, aptr, abuf, alen);
    if (aptr == NULL)
      return;
  }

  /* the additional records. */
  for (i = 0; i < arcount; i++) {
    aptr = save_NAPTR (output_record, aptr, abuf, alen);
    if (aptr == NULL)
      return;
  }
}

static void
_srv_callback (void *arg, int status, int timeouts, unsigned char *abuf, int alen)
{
  _store_srv (arg, status, timeouts, abuf, alen, 1);
  _store_A (arg, status, timeouts, abuf, alen, 0);
}

static void
_naptr_callback (void *arg, int status, int timeouts, unsigned char *abuf, int alen)
{
  osip_naptr_t *output_record = (osip_naptr_t *) arg;

  if (status != ARES_SUCCESS) {
    if (status == ARES_ENODATA || status == ARES_ENOTFOUND) {       /* no NAPTR record for this domain */
      osip_srv_record_t srvrecord;

      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "_naptr_callback: %s %s\n", output_record->domain, ares_strerror (status)));
      /* pre-set all SRV record to unsupported? */
      output_record->naptr_state = OSIP_NAPTR_STATE_NAPTRDONE;

      output_record->sipudp_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;
      output_record->siptcp_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;
      output_record->siptls_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;
      output_record->sipdtls_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;
      output_record->sipsctp_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;

      memset (&srvrecord, 0, sizeof (osip_srv_record_t));

      srvrecord.order = 49;
      srvrecord.preference = 49;
      srvrecord.srv_state = OSIP_SRV_STATE_UNKNOWN;

      snprintf (srvrecord.protocol, sizeof (srvrecord.protocol), "%s", "SIP+D2U");
      snprintf (srvrecord.name, sizeof (srvrecord.name), "_sip._udp.%s", output_record->domain);
      memcpy (&output_record->sipudp_record, &srvrecord, sizeof (osip_srv_record_t));

      snprintf (srvrecord.protocol, sizeof (srvrecord.protocol), "%s", "SIP+D2T");
      snprintf (srvrecord.name, sizeof (srvrecord.name), "_sip._tcp.%s", output_record->domain);
      memcpy (&output_record->siptcp_record, &srvrecord, sizeof (osip_srv_record_t));

      snprintf (srvrecord.protocol, sizeof (srvrecord.protocol), "%s", "SIPS+D2T");
      snprintf (srvrecord.name, sizeof (srvrecord.name), "_sips._tcp.%s", output_record->domain);
      memcpy (&output_record->siptls_record, &srvrecord, sizeof (osip_srv_record_t));

      snprintf (srvrecord.protocol, sizeof (srvrecord.protocol), "%s", "SIPS+D2U");
      snprintf (srvrecord.name, sizeof (srvrecord.name), "_sips._udp.%s", output_record->domain);
      memcpy (&output_record->sipdtls_record, &srvrecord, sizeof (osip_srv_record_t));

      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "_naptr_callback: NO NAPTR DNS // SRV record created manually ->%i/%i/%s\n", srvrecord.order, srvrecord.preference, srvrecord.name));

      return;
    }
  }

  if (status != ARES_SUCCESS) {
    if (status == ARES_ENODATA) /* no NAPTR record for this domain */
      output_record->naptr_state = OSIP_NAPTR_STATE_NOTSUPPORTED;
    else if (status == ARES_ENOTFOUND)  /* domain does not exist */
      output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;
    else if (status == ARES_ETIMEOUT)
      output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;
    else if (status == ARES_ESERVFAIL)
      output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;
    else if (status == ARES_ENOTIMP)
      output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;
    else if (status == ARES_EREFUSED)
      output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;
    else                        /* ... */
      output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;

    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "_naptr_callback: %s %s\n", output_record->domain, ares_strerror (status)));
    /*if (!abuf) */
    return;
  }

  /* pre-set all SRV record to unsupported? */
  output_record->sipudp_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;
  output_record->siptcp_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;
  output_record->siptls_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;
  output_record->sipdtls_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;
  output_record->sipsctp_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;

  _store_naptr (arg, status, timeouts, abuf, alen, 1);
  _store_srv (arg, status, timeouts, abuf, alen, 0);
  _store_A (arg, status, timeouts, abuf, alen, 0);
  output_record->naptr_state = OSIP_NAPTR_STATE_NAPTRDONE;
}

static int
eXosip_dnsutils_srv_lookup (struct osip_naptr *output_record)
{
  ares_channel channel = NULL;
  struct ares_options options;
  int i;

  if (output_record->naptr_state == OSIP_NAPTR_STATE_SRVINPROGRESS) {
    /* continue searching if channel exist */
    if (output_record->arg == NULL)
      return OSIP_SUCCESS;

    channel = output_record->arg;
    {
      fd_set read_fds, write_fds;
      struct timeval *tvp, tv;
      int nfds;
      int count;

      FD_ZERO (&read_fds);
      FD_ZERO (&write_fds);
      nfds = ares_fds (channel, &read_fds, &write_fds);
      if (nfds != 0) {
        tvp = ares_timeout (channel, NULL, &tv);
        tvp->tv_sec = 0;
        tvp->tv_usec = 0;
        count = select (nfds, &read_fds, &write_fds, NULL, tvp);
        if (count < 0 && SOCKERRNO != EINVAL) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_srv_lookup: select failed ('%s SRV')\n", output_record->domain));
          output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;
          output_record->arg = NULL;
          ares_destroy (channel);
          return OSIP_UNDEFINED_ERROR;
        }
        ares_process (channel, &read_fds, &write_fds);

        FD_ZERO (&read_fds);
        FD_ZERO (&write_fds);
        nfds = ares_fds (channel, &read_fds, &write_fds);
      }

      if (nfds == 0) {
        /* SRVs finished */
        if (output_record->sipudp_record.srv_state == OSIP_SRV_STATE_COMPLETED)
          output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
        else if (output_record->siptcp_record.srv_state == OSIP_SRV_STATE_COMPLETED)
          output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
        else if (output_record->siptls_record.srv_state == OSIP_SRV_STATE_COMPLETED)
          output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
        else if (output_record->sipdtls_record.srv_state == OSIP_SRV_STATE_COMPLETED)
          output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
        else if (output_record->sipsctp_record.srv_state == OSIP_SRV_STATE_COMPLETED)
          output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
        else {
          if (output_record->sipudp_record.order == 49 && output_record->sipudp_record.preference == 49)
            output_record->naptr_state = OSIP_NAPTR_STATE_NOTSUPPORTED;
          else
            output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;
        }

        output_record->arg = NULL;
        ares_destroy (channel);
        return OSIP_SUCCESS;
      }
    }

    return OSIP_SUCCESS;
  }

  if (output_record->naptr_state != OSIP_NAPTR_STATE_NAPTRDONE)
    return OSIP_SUCCESS;

  if (output_record->sipudp_record.name[0] == '\0' && output_record->siptcp_record.name[0] == '\0' && output_record->siptls_record.name[0] == '\0' && output_record->sipdtls_record.name[0] == '\0' && output_record->sipsctp_record.name[0] == '\0') {
    output_record->naptr_state = OSIP_NAPTR_STATE_NOTSUPPORTED;
    if (output_record->arg != NULL) {
      output_record->arg = NULL;
      ares_destroy (channel);
    }
    return OSIP_SUCCESS;
  }

  if (output_record->arg == NULL) {
    options.timeout = 3000;
    options.tries = 1;
    options.flags = ARES_FLAG_NOALIASES;
    i = ares_init_options (&channel, &options, ARES_OPT_TIMEOUTMS | ARES_OPT_TRIES | ARES_OPT_FLAGS);
    if (i != ARES_SUCCESS) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_srv_lookup: ares_init_options failed ('%s SRV')\n", output_record->domain));
      output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;
      return OSIP_BADPARAMETER;
    }
    output_record->arg = channel;
  }
  else {
    channel = output_record->arg;
  }

  output_record->naptr_state = OSIP_NAPTR_STATE_SRVINPROGRESS;

  if (output_record->sipudp_record.name[0] != '\0' && output_record->sipudp_record.srv_state != OSIP_SRV_STATE_COMPLETED) {
    ares_query (channel, output_record->sipudp_record.name, C_IN, T_SRV, _srv_callback, (void *) output_record);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_srv_lookup: About to ask for '%s SRV'\n", output_record->sipudp_record.name));
  }

  if (output_record->siptcp_record.name[0] != '\0' && output_record->siptcp_record.srv_state != OSIP_SRV_STATE_COMPLETED) {
    ares_query (channel, output_record->siptcp_record.name, C_IN, T_SRV, _srv_callback, (void *) output_record);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_srv_lookup: About to ask for '%s SRV'\n", output_record->siptcp_record.name));
  }

  if (output_record->siptls_record.name[0] != '\0' && output_record->siptls_record.srv_state != OSIP_SRV_STATE_COMPLETED) {
    ares_query (channel, output_record->siptls_record.name, C_IN, T_SRV, _srv_callback, (void *) output_record);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_srv_lookup: About to ask for '%s SRV'\n", output_record->siptls_record.name));
  }

  if (output_record->sipdtls_record.name[0] != '\0' && output_record->sipdtls_record.srv_state != OSIP_SRV_STATE_COMPLETED) {
    ares_query (channel, output_record->sipdtls_record.name, C_IN, T_SRV, _srv_callback, (void *) output_record);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_srv_lookup: About to ask for '%s SRV'\n", output_record->sipdtls_record.name));
  }

  {
    fd_set read_fds, write_fds;
    struct timeval *tvp, tv;
    int nfds;
    int count;

    FD_ZERO (&read_fds);
    FD_ZERO (&write_fds);
    nfds = ares_fds (channel, &read_fds, &write_fds);
    if (nfds != 0) {
      tvp = ares_timeout (channel, NULL, &tv);
      tvp->tv_sec = 0;
      tvp->tv_usec = 0;
      count = select (nfds, &read_fds, &write_fds, NULL, tvp);
      if (count < 0 && SOCKERRNO != EINVAL) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip_dnsutils_srv_lookup: select failed ('%s SRV')\n", output_record->domain));
        output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;
        output_record->arg = NULL;
        ares_destroy (channel);
        return OSIP_UNDEFINED_ERROR;
      }
      ares_process (channel, &read_fds, &write_fds);

      FD_ZERO (&read_fds);
      FD_ZERO (&write_fds);
      nfds = ares_fds (channel, &read_fds, &write_fds);
    }

    if (nfds == 0) {
      /* SRVs finished: we assume that one is enough */
      if (output_record->sipudp_record.srv_state == OSIP_SRV_STATE_COMPLETED)
        output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
      else if (output_record->siptcp_record.srv_state == OSIP_SRV_STATE_COMPLETED)
        output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
      else if (output_record->siptls_record.srv_state == OSIP_SRV_STATE_COMPLETED)
        output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
      else if (output_record->sipdtls_record.srv_state == OSIP_SRV_STATE_COMPLETED)
        output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
      else if (output_record->sipsctp_record.srv_state == OSIP_SRV_STATE_COMPLETED)
        output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
      else {
        if (output_record->sipudp_record.order == 49 && output_record->sipudp_record.preference == 49)
          output_record->naptr_state = OSIP_NAPTR_STATE_NOTSUPPORTED;
        else
          output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;
      }
      output_record->arg = NULL;
      ares_destroy (channel);
      return OSIP_SUCCESS;
    }
  }

  return OSIP_SUCCESS;
}

static int
eXosip_dnsutils_naptr_lookup (osip_naptr_t * output_record, const char *domain)
{
  ares_channel channel = NULL;
  struct ares_options options;
  int i;

  if (output_record->arg != NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip_dnsutils_naptr_lookup: wrong code path ('%s NAPTR')\n", domain));
    return OSIP_UNDEFINED_ERROR;
  }

  output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;
  if (domain == NULL)
    return OSIP_BADPARAMETER;

  if (strlen (domain) > 512)
    return OSIP_BADPARAMETER;

  snprintf (output_record->domain, sizeof (output_record->domain), "%s", domain);

  options.timeout = 3000;
  options.tries = 1;
  options.flags = ARES_FLAG_NOALIASES;
  i = ares_init_options (&channel, &options, ARES_OPT_TIMEOUTMS | ARES_OPT_TRIES | ARES_OPT_FLAGS);
  if (i != ARES_SUCCESS) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip_dnsutils_naptr_lookup: ares_init_options failed ('%s NAPTR')\n", domain));
    return OSIP_BADPARAMETER;
  }
  output_record->arg = channel;
  output_record->naptr_state = OSIP_NAPTR_STATE_INPROGRESS;
  ares_query (channel, domain, C_IN, T_NAPTR, _naptr_callback, (void *) output_record);

  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_naptr_lookup: About to ask for '%s NAPTR'\n", domain));

  {
    fd_set read_fds, write_fds;
    struct timeval *tvp, tv;
    int nfds;
    int count;

    FD_ZERO (&read_fds);
    FD_ZERO (&write_fds);
    nfds = ares_fds (channel, &read_fds, &write_fds);
    if (nfds != 0) {
      tvp = ares_timeout (channel, NULL, &tv);
      tvp->tv_sec = 0;
      tvp->tv_usec = 0;
      count = select (nfds, &read_fds, &write_fds, NULL, tvp);
      if (count < 0 && SOCKERRNO != EINVAL) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip_dnsutils_naptr_lookup: select failed ('%s NAPTR')\n", domain));
        output_record->arg = NULL;
        ares_destroy (channel);
        output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;
        return OSIP_UNDEFINED_ERROR;
      }
      ares_process (channel, &read_fds, &write_fds);

      FD_ZERO (&read_fds);
      FD_ZERO (&write_fds);
      nfds = ares_fds (channel, &read_fds, &write_fds);
    }
    if (nfds == 0) {
      if (output_record->naptr_state != OSIP_NAPTR_STATE_NAPTRDONE) {
        /* don't need channel any more */
        ares_destroy (channel);
        output_record->arg = NULL;
      }
      return OSIP_SUCCESS;
    }
  }

  return OSIP_SUCCESS;
}

struct osip_naptr *
eXosip_dnsutils_naptr (struct eXosip_t *excontext, const char *domain, const char *protocol, const char *transport, int keep_in_cache)
{
  osip_list_iterator_t it;
  struct osip_naptr *naptr_record;
  int i;

#if defined(HAVE_WINDNS_H)
  DNS_STATUS err;
  DWORD buf_length = 0;
  IP4_ARRAY *dns_servers;
#endif
  int not_in_list = 0;

  if (excontext->dns_capabilities <= 0)
    return NULL;

  if (dnsutils_list == NULL) {
    dnsutils_list = (osip_list_t *) osip_malloc (sizeof (osip_list_t));
    osip_list_init (dnsutils_list);

    i = ares_library_init (ARES_LIB_INIT_ALL);
    if (i != ARES_SUCCESS) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip_dnsutils_naptr: ares cannot be initialized\n"));
      return NULL;
    }
  }

  if (keep_in_cache < 0) {
    naptr_record = (osip_naptr_t *)osip_list_get_first(dnsutils_list, &it);
    while (naptr_record != NULL) {
      if (osip_strcasecmp (domain, naptr_record->domain) == 0) {
        if (naptr_record->naptr_state == OSIP_NAPTR_STATE_RETRYLATER)
          break;
        if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NOTSUPPORTED)
          break;

        return naptr_record;
      }

      naptr_record = NULL;
      if (it.pos == 9)
        break;
      naptr_record = (osip_naptr_t *)osip_list_get_next(&it);
    }
    return NULL;
  }

#if defined(HAVE_WINDNS_H)
  err = DnsQueryConfig (DnsConfigDnsServerList, 0, NULL, NULL, NULL, &buf_length);
  if (err == DNS_ERROR_NO_DNS_SERVERS) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip_dnsutils_naptr: no DNS server configured\n"));
    return NULL;
  }

  if (err != ERROR_MORE_DATA && err != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip_dnsutils_naptr: error with DnsQueryConfig / DnsConfigDnsServerList\n"));
    return NULL;
  }

  dns_servers = osip_malloc (buf_length);
  err = DnsQueryConfig (DnsConfigDnsServerList, 0, NULL, NULL, dns_servers, &buf_length);
  if (err != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip_dnsutils_naptr: error with DnsQueryConfig / DnsConfigDnsServerList\n"));
    osip_free (dns_servers);
    return NULL;
  }

  if (dns_servers->AddrCount == 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_naptr: no DNS server configured\n"));
    osip_free (dns_servers);
    return NULL;
  }

  for (i = 0; (DWORD) i < dns_servers->AddrCount; i++) {
    char ipaddress[512];
    DWORD val = dns_servers->AddrArray[i];

    snprintf (ipaddress, sizeof (ipaddress), "%d.%d.%d.%d", (val >> 0) & 0x000000FF, (val >> 8) & 0x000000FF, (val >> 16) & 0x000000FF, (val >> 24) & 0x000000FF);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_naptr: DNS server (%i): %s\n", i, ipaddress));
  }
  osip_free (dns_servers);
#endif

  naptr_record = (osip_naptr_t *)osip_list_get_first(dnsutils_list, &it);
  while (naptr_record != NULL) {

    /* process all */
    if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NAPTRDONE || naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVINPROGRESS)
      eXosip_dnsutils_srv_lookup (naptr_record);

    naptr_record = NULL;
    if (it.pos == 9)
      break;
    naptr_record = (osip_naptr_t *)osip_list_get_next(&it);
  }

  if (domain == NULL)
    return NULL;

  it.pos=0;
  naptr_record = (osip_naptr_t *)osip_list_get_first(dnsutils_list, &it);
  while (naptr_record != NULL) {
    if (osip_strcasecmp (domain, naptr_record->domain) == 0) {
      if (naptr_record->naptr_state == OSIP_NAPTR_STATE_RETRYLATER)
        break;

      if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NAPTRDONE || naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVINPROGRESS)
        eXosip_dnsutils_srv_lookup (naptr_record);

      return naptr_record;
    }

    naptr_record = NULL;
    if (it.pos == 9)
      break;
    naptr_record = (osip_naptr_t *)osip_list_get_next(&it);
  }

  if (it.pos == 9 && keep_in_cache > 0) {
    /* no NAPTR found within the last 10 NAPTR : refuse to keep in cache... */
    /* If we were adding unlimited NAPTR record into the cache, the program
       would infinitly increase memory usage. If you reach there, then you
       most probably don't use the API correctly: Only NAPTR related to
       registration will end up in the cache. So in theory, 10 NAPTR will be
       cached for 10 accounts, which seems enough for a softphone. Then all
       SIP message should go through the same proxy by using a pre-route set.
       Correctly using the API will definitly send out-of-dialog SIP message
       to the same proxy as the REGISTER. If you have issue with in-dialog
       Request or Response and NAPTR, that means your proxy is not following
       guidelines from the rfc3261 and rfc3263 where proxy should specify
       port numbers when they want you to resolve the host as a A record
       -and then avoid NAPTR-.
     */
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_naptr: Time will tell how much you go there (%s) - it's wrong code path, fix it\n", domain));

    keep_in_cache = 0;

    naptr_record = (osip_naptr_t *) osip_malloc (sizeof (osip_naptr_t));
    memset (naptr_record, 0, sizeof (osip_naptr_t));
    naptr_record->keep_in_cache = keep_in_cache;
  }
  else if (naptr_record == NULL) {
    naptr_record = (osip_naptr_t *) osip_malloc (sizeof (osip_naptr_t));
    memset (naptr_record, 0, sizeof (osip_naptr_t));
    naptr_record->keep_in_cache = keep_in_cache;
    not_in_list = 1;
  }
  else {
    /* it was found, so it WAS in cache before, but we were in "retry" state */
    memset (naptr_record, 0, sizeof (osip_naptr_t));
    naptr_record->keep_in_cache = 1;
  }
  i = eXosip_dnsutils_naptr_lookup (naptr_record, domain);
  if (i < 0) {
    if (keep_in_cache <= 0) {
      return naptr_record;
    }
    if (not_in_list == 1) {
      osip_list_add (dnsutils_list, naptr_record, -1);
    }
    return naptr_record;
  }

  if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NAPTRDONE || naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVINPROGRESS)
    eXosip_dnsutils_srv_lookup (naptr_record);

  if (keep_in_cache <= 0) {
    return naptr_record;
  }
  if (not_in_list == 1)
    osip_list_add (dnsutils_list, naptr_record, -1);
  return naptr_record;
}

int
eXosip_dnsutils_dns_process (osip_naptr_t * naptr_record, int force)
{
  ares_channel channel = NULL;

  if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NAPTRDONE || naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVINPROGRESS)
    eXosip_dnsutils_srv_lookup (naptr_record);

  if (naptr_record->arg != NULL)
    channel = naptr_record->arg;

  if (channel == NULL)
    return OSIP_SUCCESS;

  /* in "keep_in_cache" use-case (REGISTER), we delayed completion. */
  for (;;) {
    fd_set read_fds, write_fds;
    struct timeval *tvp, tv;
    int nfds;
    int count;

    FD_ZERO (&read_fds);
    FD_ZERO (&write_fds);
    nfds = ares_fds (channel, &read_fds, &write_fds);
    if (nfds != 0) {
      tvp = ares_timeout (channel, NULL, &tv);
      tvp->tv_sec = 0;
      tvp->tv_usec = 0;
      count = select (nfds, &read_fds, &write_fds, NULL, tvp);
      if (count < 0 && SOCKERRNO != EINVAL) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_dns_process: select failed ('%s')\n", naptr_record->domain));
        ares_destroy (channel);
        naptr_record->arg = NULL;
        return OSIP_UNDEFINED_ERROR;
      }
      ares_process (channel, &read_fds, &write_fds);

      FD_ZERO (&read_fds);
      FD_ZERO (&write_fds);
      nfds = ares_fds (channel, &read_fds, &write_fds);
    }
    if (nfds == 0) {
      if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NAPTRDONE || naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVINPROGRESS) {
        /* missing SRV */
        eXosip_dnsutils_srv_lookup (naptr_record);
      }
      else if (naptr_record->naptr_state == OSIP_NAPTR_STATE_INPROGRESS) {
        if (naptr_record->sipudp_record.srv_state == OSIP_SRV_STATE_COMPLETED)
          naptr_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
        else if (naptr_record->siptcp_record.srv_state == OSIP_SRV_STATE_COMPLETED)
          naptr_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
        else if (naptr_record->siptls_record.srv_state == OSIP_SRV_STATE_COMPLETED)
          naptr_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
        else if (naptr_record->sipdtls_record.srv_state == OSIP_SRV_STATE_COMPLETED)
          naptr_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
        else if (naptr_record->sipsctp_record.srv_state == OSIP_SRV_STATE_COMPLETED)
          naptr_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
        else
          naptr_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;
        /* no need any more */
        naptr_record->arg = NULL;
        ares_destroy (channel);
        return OSIP_SUCCESS;
      }
      else {
        /* no need any more */
        naptr_record->arg = NULL;
        ares_destroy (channel);
        return OSIP_SUCCESS;
      }
    }
    if (force <= 0)
      break;
  }
  return OSIP_SUCCESS;
}

void
_eXosip_dnsutils_release (osip_naptr_t * naptr_record)
{
  ares_channel channel;

  if (naptr_record == NULL)
    return;
  if (naptr_record->keep_in_cache > 0)
    return;
  if (naptr_record->arg != NULL) {
    channel = naptr_record->arg;
    ares_destroy (channel);
    naptr_record->arg = NULL;
  }
  osip_free (naptr_record);
}

#elif defined(WIN32) && !defined(_WIN32_WCE)

int
_eXosip_dnsutils_srv_lookup (struct osip_srv_record *output_srv)
{
  PDNS_RECORD answer, tmp;      /* answer buffer from nameserver */
  int n;

  if (output_srv->name[0] == '\0') {
    return OSIP_SUCCESS;
  }

  if (output_srv->srventry[0].srv[0] != '\0') {
    /* if we received the SRV inside the NAPTR answer, are we sure we received
       all SRV entries? */
    return OSIP_SUCCESS;
  }

  if (DnsQuery (output_srv->name, DNS_TYPE_SRV, DNS_QUERY_STANDARD, NULL, &answer, NULL) != 0) {
    return OSIP_UNKNOWN_HOST;
  }

  n = 0;
  for (tmp = answer; tmp != NULL; tmp = tmp->pNext) {
    struct osip_srv_entry *srventry;

    DNS_SRV_DATA *data;

    if (tmp->wType != DNS_TYPE_SRV)
      continue;
    srventry = &output_srv->srventry[n];
    data = &tmp->Data.SRV;
    snprintf (srventry->srv, sizeof (srventry->srv), "%s", data->pNameTarget);

    srventry->priority = data->wPriority;
    srventry->weight = data->wWeight;
    if (srventry->weight)
      srventry->rweight = 1 + rand () % (10000 * srventry->weight);
    else
      srventry->rweight = 0;
    srventry->port = data->wPort;

    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "SRV record %s IN SRV -> %s:%i/%i/%i/%i\n", output_srv->name, srventry->srv, srventry->port, srventry->priority, srventry->weight, srventry->rweight));

    output_srv->srv_state = OSIP_SRV_STATE_COMPLETED;

    n++;
    if (n == 10)
      break;
  }

  DnsRecordListFree (answer, DnsFreeRecordList);

  if (n == 0)
    return OSIP_UNKNOWN_HOST;
  osip_srv_record_sort (output_srv, n);
  return OSIP_SUCCESS;
}

int
eXosip_dnsutils_srv_lookup (struct osip_naptr *output_record)
{
  if (output_record->naptr_state == OSIP_NAPTR_STATE_SRVDONE)
    return OSIP_SUCCESS;

  output_record->sipudp_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;
  output_record->siptcp_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;
  output_record->siptls_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;
  output_record->sipdtls_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;
  output_record->sipsctp_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;

  _eXosip_dnsutils_srv_lookup (&output_record->sipudp_record);
  _eXosip_dnsutils_srv_lookup (&output_record->siptcp_record);
  _eXosip_dnsutils_srv_lookup (&output_record->siptls_record);
  _eXosip_dnsutils_srv_lookup (&output_record->sipdtls_record);
  /* _eXosip_dnsutils_srv_lookup(&output_record->sipsctp_record); */

  if (output_record->sipudp_record.srv_state == OSIP_SRV_STATE_COMPLETED)
    output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
  else if (output_record->siptcp_record.srv_state == OSIP_SRV_STATE_COMPLETED)
    output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
  else if (output_record->siptls_record.srv_state == OSIP_SRV_STATE_COMPLETED)
    output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
  else if (output_record->sipdtls_record.srv_state == OSIP_SRV_STATE_COMPLETED)
    output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
  else if (output_record->sipsctp_record.srv_state == OSIP_SRV_STATE_COMPLETED)
    output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
  else
    output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;

  return 0;
}

int
_eX_dn_expand (unsigned char *msg, unsigned char *eomorig, unsigned char *comp_dn, unsigned char *exp_dn, int length)
{
  unsigned char *cp;
  unsigned char *dest;
  unsigned char *eom;
  int len = -1;
  int len_copied = 0;

  dest = exp_dn;
  cp = comp_dn;
  eom = exp_dn + length;

  while (*cp != '\0') {
    int w = *cp;
    int comp = w & 0xc0;

    cp++;
    if (comp == 0) {
      if (dest != exp_dn) {
        if (dest >= eom)
          return -1;
        *dest = '.';
        dest++;
      }
      if (dest + w >= eom)
        return -1;
      len_copied++;
      len_copied = len_copied + w;
      w--;
      for (; w >= 0; w--, cp++) {
        if ((*cp == '.') || (*cp == '\\')) {
          if (dest + w + 2 >= eom)
            return -1;
          *dest = '\\';
          dest++;
        }
        *dest = *cp;
        dest++;
        if (cp >= eomorig)
          return -1;
      }
    }
    else if (comp == 0xc0) {
      if (len < 0)
        len = cp - comp_dn + 1;
      cp = msg + (((w & 0x3f) << 8) | (*cp & 0xff));
      if (cp < msg || cp >= eomorig)
        return -1;
      len_copied = len_copied + 2;
      if (len_copied >= eomorig - msg)
        return -1;
    }
    else
      return -1;
  }

  *dest = '\0';

  if (len < 0)
    len = cp - comp_dn;
  return len;
}

int
eXosip_dnsutils_naptr_lookup (osip_naptr_t * output_record, const char *domain)
{
  PDNS_RECORD answer, tmp;      /* answer buffer from nameserver */
  DNS_STATUS ret;

  if (domain == NULL)
    return OSIP_BADPARAMETER;

  if (strlen (domain) > 512)
    return OSIP_BADPARAMETER;

  snprintf (output_record->domain, sizeof (output_record->domain), "%s", domain);

  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_naptr_lookup: About to ask for '%s NAPTR'\n", domain));

  ret = DnsQuery (domain, DNS_TYPE_NAPTR, DNS_QUERY_STANDARD, NULL, &answer, NULL);
  if (ret == DNS_ERROR_NO_DNS_SERVERS)
    return OSIP_NO_NETWORK;
  if (ret == ERROR_TIMEOUT)
    return OSIP_TIMEOUT;
  if (ret == DNS_INFO_NO_RECORDS)
    return OSIP_UNKNOWN_HOST;
  if (ret == DNS_ERROR_RCODE_SERVER_FAILURE)
    return OSIP_NOTFOUND;       /* no such domain? */

  if (ret != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_naptr_lookup: DnsQuery Failed for %s NAPTR with %d\n", domain, ret));
    return OSIP_UNDEFINED_ERROR;
  }

  for (tmp = answer; tmp != NULL; tmp = tmp->pNext) {
    char *buf = (char *) &tmp->Data;

    int len;
    OSVERSIONINFOEX ovi;
    typedef struct {
      unsigned short order;
      unsigned short pref;
      char flag[256];
      char service[1024];
      char regexp[1024];
      char replacement[1024];
    } osip_naptr_t;

    osip_naptr_t anaptr;

    if (tmp->wType != DNS_TYPE_NAPTR)
      continue;

    memset (&anaptr, 0, sizeof (osip_naptr_t));
    memset (&ovi, 0, sizeof (ovi));
    ovi.dwOSVersionInfoSize = sizeof (ovi);
    GetVersionEx ((LPOSVERSIONINFO) & ovi);

    /* Minimum: client: Windows 2000 Professional */
    /* Minimum: server: Windows 2000 Server */
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_naptr_lookup: check OS support for NAPTR: %i %i %i\n", ovi.dwMajorVersion, ovi.dwMinorVersion, ovi.dwBuildNumber));
    if (ovi.dwMajorVersion > 5) {

#if (_WIN32_WINNT >= 0x0600)
      /* RUN only on Vista? */
      /* compile starting from SDK 6.0A? even on XP... */
      anaptr.order = tmp->Data.NAPTR.wOrder;
      anaptr.pref = tmp->Data.NAPTR.wPreference;
      strncpy (anaptr.flag, tmp->Data.NAPTR.pFlags, sizeof (anaptr.flag) - 1);
      strncpy (anaptr.service, tmp->Data.NAPTR.pService, sizeof (anaptr.service) - 1);
      strncpy (anaptr.regexp, tmp->Data.NAPTR.pRegularExpression, sizeof (anaptr.regexp) - 1);
      strncpy (anaptr.replacement, tmp->Data.NAPTR.pReplacement, sizeof (anaptr.replacement) - 1);

#endif
    }

    else {
      memcpy ((void *) &anaptr.order, buf, 2);
      anaptr.order = ntohs (anaptr.order);      /* ((unsigned short)buf[0] << 8) | ((unsigned short)buf[1]); */
      buf += sizeof (unsigned short);
      memcpy ((void *) &anaptr.pref, buf, 2);
      anaptr.pref = ntohs (anaptr.pref);        /* ((unsigned short)buf[0] << 8) | ((unsigned short)buf[1]); */
      buf += sizeof (unsigned short);

      len = *buf;
      if (len < 0 || len > 255)
        break;
      buf++;
      strncpy (anaptr.flag, buf, len);
      anaptr.flag[len] = '\0';
      buf += len;

      len = *buf;
      if (len < 0 || len > 1023)
        break;
      buf++;
      strncpy (anaptr.service, buf, len);
      anaptr.service[len] = '\0';
      buf += len;

      len = *buf;
      if (len < 0 || len > 1023)
        break;
      buf++;
      strncpy (anaptr.regexp, buf, len);
      anaptr.regexp[len] = '\0';
      buf += len;

      len = _eX_dn_expand ((char *) &tmp->Data, ((char *) &tmp->Data) + tmp->wDataLength, buf, anaptr.replacement, 1024 - 1);

      if (len < 0)
        break;
      buf += len;
    }

    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_naptr_lookup: NAPTR %s ->%i/%i/%s/%s/%s/%s\n", domain, anaptr.order, anaptr.pref, anaptr.flag, anaptr.service, anaptr.regexp, anaptr.replacement));

    if (osip_strncasecmp (anaptr.service, "SIP+D2U", 8) == 0) { /* udp */
      snprintf (output_record->sipudp_record.name, sizeof (output_record->sipudp_record.name), "%s", anaptr.replacement);
      output_record->naptr_state = OSIP_NAPTR_STATE_NAPTRDONE;
    }
    else if (osip_strncasecmp (anaptr.service, "SIP+D2T", 8) == 0) {    /* tcp */
      snprintf (output_record->siptcp_record.name, sizeof (output_record->siptcp_record.name), "%s", anaptr.replacement);
      output_record->naptr_state = OSIP_NAPTR_STATE_NAPTRDONE;
    }
    else if (osip_strncasecmp (anaptr.service, "SIPS+D2T", 9) == 0) {   /* tls */
      snprintf (output_record->siptls_record.name, sizeof (output_record->siptls_record.name), "%s", anaptr.replacement);
      output_record->naptr_state = OSIP_NAPTR_STATE_NAPTRDONE;
    }
    else if (osip_strncasecmp (anaptr.service, "SIPS+D2U", 9) == 0) {   /* dtls-udp */
      snprintf (output_record->sipdtls_record.name, sizeof (output_record->sipdtls_record.name), "%s", anaptr.replacement);
      output_record->naptr_state = OSIP_NAPTR_STATE_NAPTRDONE;
    }
    else if (osip_strncasecmp (anaptr.service, "SIP+D2S", 8) == 0) {    /* sctp */
      snprintf (output_record->sipsctp_record.name, sizeof (output_record->sipsctp_record.name), "%s", anaptr.replacement);
    }
  }

  for (tmp = answer; tmp != NULL; tmp = tmp->pNext) {
    struct osip_srv_entry *srventry;
    struct osip_srv_record *srvrecord;
    DNS_SRV_DATA *data;
    int n;

    if (tmp->wType != DNS_TYPE_SRV)
      continue;

    if (osip_strcasecmp (tmp->pName, output_record->sipudp_record.name) == 0)
      srvrecord = &output_record->sipudp_record;
    else if (osip_strcasecmp (tmp->pName, output_record->siptcp_record.name) == 0)
      srvrecord = &output_record->siptcp_record;
    else if (osip_strcasecmp (tmp->pName, output_record->siptls_record.name) == 0)
      srvrecord = &output_record->siptls_record;
    else if (osip_strcasecmp (tmp->pName, output_record->sipdtls_record.name) == 0)
      srvrecord = &output_record->sipdtls_record;
    else if (osip_strcasecmp (tmp->pName, output_record->sipsctp_record.name) == 0)
      srvrecord = &output_record->sipsctp_record;
    else
      continue;

    n = 0;
    while (n < 10 && srvrecord->srventry[n].srv[0] != '\0')
      n++;
    if (n == 10)
      continue;                 /* skip... */
    srventry = &srvrecord->srventry[n];

    data = &tmp->Data.SRV;
    snprintf (srventry->srv, sizeof (srventry->srv), "%s", data->pNameTarget);

    srventry->priority = data->wPriority;
    srventry->weight = data->wWeight;
    if (srventry->weight)
      srventry->rweight = 1 + rand () % (10000 * srventry->weight);
    else
      srventry->rweight = 0;
    srventry->port = data->wPort;

    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "SRV record %s IN SRV -> %s:%i/%i/%i/%i\n", tmp->pName, srventry->srv, srventry->port, srventry->priority, srventry->weight, srventry->rweight));
    osip_srv_record_sort (srvrecord, n + 1);

    output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
  }

  for (tmp = answer; tmp != NULL; tmp = tmp->pNext) {
    struct osip_srv_entry *srventry;
    DNS_A_DATA *data;
    DWORD val;
    int n;

    if (tmp->wType != DNS_TYPE_A)
      continue;

    data = &tmp->Data.A;

    val = data->IpAddress;

    /* update all SRV bound to this A record. */
    for (n = 0; n < 10; n++) {
      if (osip_strcasecmp (tmp->pName, output_record->sipudp_record.srventry[n].srv) == 0)
        srventry = &output_record->sipudp_record.srventry[n];
      else if (osip_strcasecmp (tmp->pName, output_record->siptcp_record.srventry[n].srv) == 0)
        srventry = &output_record->siptcp_record.srventry[n];
      else if (osip_strcasecmp (tmp->pName, output_record->siptls_record.srventry[n].srv) == 0)
        srventry = &output_record->siptls_record.srventry[n];
      else if (osip_strcasecmp (tmp->pName, output_record->sipdtls_record.srventry[n].srv) == 0)
        srventry = &output_record->sipdtls_record.srventry[n];
      else if (osip_strcasecmp (tmp->pName, output_record->sipsctp_record.srventry[n].srv) == 0)
        srventry = &output_record->sipsctp_record.srventry[n];
      else
        continue;
      snprintf (srventry->ipaddress, sizeof (srventry->ipaddress), "%d.%d.%d.%d", (val >> 0) & 0x000000FF, (val >> 8) & 0x000000FF, (val >> 16) & 0x000000FF, (val >> 24) & 0x000000FF);
    }

    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "A record %s -> %d.%d.%d.%d\n", tmp->pName, (val >> 0) & 0x000000FF, (val >> 8) & 0x000000FF, (val >> 16) & 0x000000FF, (val >> 24) & 0x000000FF));

  }

  DnsRecordListFree (answer, DnsFreeRecordList);

  return OSIP_SUCCESS;
}

struct osip_naptr *
eXosip_dnsutils_naptr (struct eXosip_t *excontext, const char *domain, const char *protocol, const char *transport, int keep_in_cache)
{
  osip_list_iterator_t it;
  struct osip_naptr *naptr_record;
  int i;
  DNS_STATUS err;
  DWORD buf_length = 0;
  IP4_ARRAY *dns_servers;
  int not_in_list = 0;

  if (excontext->dns_capabilities <= 0)
    return NULL;

  if (dnsutils_list == NULL) {
    dnsutils_list = (osip_list_t *) osip_malloc (sizeof (osip_list_t));
    osip_list_init (dnsutils_list);
  }

  if (keep_in_cache < 0) {
    naptr_record = (osip_naptr_t *)osip_list_get_first(dnsutils_list, &it);
    while (naptr_record != NULL) {
      if (osip_strcasecmp (domain, naptr_record->domain) == 0) {
        if (naptr_record->naptr_state == OSIP_NAPTR_STATE_RETRYLATER)
          break;
        if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NOTSUPPORTED)
          break;

        return naptr_record;
      }

      naptr_record = NULL;
      if (it.pos == 9)
        break;
      naptr_record = (osip_naptr_t *)osip_list_get_next(&it);
    }
    return NULL;
  }

  err = DnsQueryConfig (DnsConfigDnsServerList, 0, NULL, NULL, NULL, &buf_length);
  if (err == DNS_ERROR_NO_DNS_SERVERS) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip_dnsutils_naptr: no DNS server configured\n"));
    return NULL;
  }

  if (err != ERROR_MORE_DATA && err != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip_dnsutils_naptr: error with DnsQueryConfig / DnsConfigDnsServerList\n"));
    return NULL;
  }

  dns_servers = osip_malloc (buf_length);
  err = DnsQueryConfig (DnsConfigDnsServerList, 0, NULL, NULL, dns_servers, &buf_length);
  if (err != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip_dnsutils_naptr: error with DnsQueryConfig / DnsConfigDnsServerList\n"));
    osip_free (dns_servers);
    return NULL;
  }

  if (dns_servers->AddrCount == 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_naptr: no DNS server configured\n"));
    osip_free (dns_servers);
    return NULL;
  }

  for (i = 0; (DWORD) i < dns_servers->AddrCount; i++) {
    char ipaddress[512];
    DWORD val = dns_servers->AddrArray[i];

    snprintf (ipaddress, sizeof (ipaddress), "%d.%d.%d.%d", (val >> 0) & 0x000000FF, (val >> 8) & 0x000000FF, (val >> 16) & 0x000000FF, (val >> 24) & 0x000000FF);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_naptr: DNS server (%i): %s\n", i, ipaddress));
  }


  naptr_record = (osip_naptr_t *)osip_list_get_first(dnsutils_list, &it);
  while (naptr_record != NULL) {

    /* process all */
    if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NAPTRDONE || naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVINPROGRESS)
      eXosip_dnsutils_srv_lookup (naptr_record);

    naptr_record = NULL;
    if (it.pos == 9)
      break;
    naptr_record = (osip_naptr_t *)osip_list_get_next(&it);
  }

  if (domain == NULL)
    return NULL;

  it.pos=0;
  naptr_record = (osip_naptr_t *)osip_list_get_first(dnsutils_list, &it);
  while (naptr_record != NULL) {
    if (osip_strcasecmp (domain, naptr_record->domain) == 0) {
      if (naptr_record->naptr_state == OSIP_NAPTR_STATE_RETRYLATER)
        break;

      if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NAPTRDONE || naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVINPROGRESS)
        eXosip_dnsutils_srv_lookup (naptr_record);

      return naptr_record;
    }

    naptr_record = NULL;
    if (it.pos == 9)
      break;
    naptr_record = (osip_naptr_t *)osip_list_get_next(&it);
  }

  if (it.pos == 9 && keep_in_cache > 0) {
    /* no NAPTR found within the last 10 NAPTR : refuse to keep in cache... */
    /* If we were adding unlimited NAPTR record into the cache, the program
       would infinitly increase memory usage. If you reach there, then you
       most probably don't use the API correctly: Only NAPTR related to
       registration will end up in the cache. So in theory, 10 NAPTR will be
       cached for 10 accounts, which seems enough for a softphone. Then all
       SIP message should go through the same proxy by using a pre-route set.
       Correctly using the API will definitly send out-of-dialog SIP message
       to the same proxy as the REGISTER. If you have issue with in-dialog
       Request or Response and NAPTR, that means your proxy is not following
       guidelines from the rfc3261 and rfc3263 where proxy should specify
       port numbers when they want you to resolve the host as a A record
       -and then avoid NAPTR-.
     */
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_naptr: Time will tell how much you go there (%s) - it's wrong code path, fix it\n", domain));

    keep_in_cache = 0;

    naptr_record = (osip_naptr_t *) osip_malloc (sizeof (osip_naptr_t));
    memset (naptr_record, 0, sizeof (osip_naptr_t));
    naptr_record->keep_in_cache = keep_in_cache;
  }
  else if (naptr_record == NULL) {
    naptr_record = (osip_naptr_t *) osip_malloc (sizeof (osip_naptr_t));
    memset (naptr_record, 0, sizeof (osip_naptr_t));
    naptr_record->keep_in_cache = keep_in_cache;
    not_in_list = 1;
  }
  else {
    /* it was found, so it WAS in cache before, but we were in "retry" state */
    memset (naptr_record, 0, sizeof (osip_naptr_t));
    naptr_record->keep_in_cache = 1;
  }

  i = eXosip_dnsutils_naptr_lookup (naptr_record, domain);
  if (i < 0) {
    if (keep_in_cache <= 0) {
      return naptr_record;
    }
    if (not_in_list == 1) {
      osip_list_add (dnsutils_list, naptr_record, -1);
    }
    return naptr_record;
  }

  if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NAPTRDONE || naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVINPROGRESS)
    eXosip_dnsutils_srv_lookup (naptr_record);

  if (keep_in_cache <= 0) {
    return naptr_record;
  }
  if (not_in_list == 1)
    osip_list_add (dnsutils_list, naptr_record, -1);
  return naptr_record;
}

int
eXosip_dnsutils_dns_process (osip_naptr_t * naptr_record, int force)
{
  return OSIP_SUCCESS;
}

void
_eXosip_dnsutils_release (osip_naptr_t * naptr_record)
{
  if (naptr_record == NULL)
    return;
  if (naptr_record->keep_in_cache > 0)
    return;
  osip_free (naptr_record);
}

#elif defined(__linux) || defined(__APPLE_CC__)

/* the biggest packet we'll send and receive */
#if PACKETSZ > 1024
#define	MAXPACKET PACKETSZ
#else
#define	MAXPACKET 1024
#endif

/* and what we send and receive */
typedef union {
  HEADER hdr;
  u_char buf[MAXPACKET];
} querybuf;

#ifndef T_SRV
#define T_SRV		33
#endif

#ifndef T_NAPTR
#define T_NAPTR	35
#endif

static int
_eXosip_dnsutils_srv_lookup (struct osip_srv_record *output_srv)
{
  querybuf answer;              /* answer buffer from nameserver */
  int n;
  int ancount, qdcount;         /* answer count and query count */
  HEADER *hp;                   /* answer buffer header */
  char hostbuf[256];
  unsigned char *msg, *eom, *cp;        /* answer buffer positions */
  int dlen, type, aclass, pref, weight, port;
  long ttl;
  int answerno;

  if (output_srv->name[0] == '\0') {
    return OSIP_SUCCESS;
  }

  if (output_srv->srventry[0].srv[0] != '\0') {
    /* if we received the SRV inside the NAPTR answer, are we sure we received
       all SRV entries? */
    return OSIP_SUCCESS;
  }

  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "About to ask for '%s IN SRV'\n", output_srv->name));

  n = res_query (output_srv->name, C_IN, T_SRV, (unsigned char *) &answer, sizeof (answer));

  if (n < (int) sizeof (HEADER)) {
    return OSIP_UNKNOWN_HOST;
  }

  /* browse message and search for DNS answers part */
  hp = (HEADER *) & answer;
  qdcount = ntohs (hp->qdcount);
  ancount = ntohs (hp->ancount);

  msg = (unsigned char *) (&answer);
  eom = (unsigned char *) (&answer) + n;
  cp = (unsigned char *) (&answer) + sizeof (HEADER);

  while (qdcount-- > 0 && cp < eom) {
    n = dn_expand (msg, eom, cp, (char *) hostbuf, 256);
    if (n < 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Invalid SRV record answer for '%s': bad format\n", output_srv->name));
      return OSIP_UNDEFINED_ERROR;
    }
    cp += n + QFIXEDSZ;
  }


  /* browse DNS answers */
  answerno = 0;

  /* loop through the answer buffer and extract SRV records */
  while (ancount-- > 0 && cp < eom) {
    struct osip_srv_entry *srventry;

    n = dn_expand (msg, eom, cp, (char *) hostbuf, 256);
    if (n < 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Invalid SRV record answer for '%s': bad format\n", output_srv->name));
      return OSIP_UNDEFINED_ERROR;
    }

    cp += n;


#if defined(__NetBSD__) || defined(__OpenBSD__) ||\
defined(OLD_NAMESER) || defined(__FreeBSD__)
    type = _get_short (cp);
    cp += sizeof (u_short);
#elif defined(__APPLE_CC__)
    GETSHORT (type, cp);
#else
    NS_GET16 (type, cp);
#endif

#if defined(__NetBSD__) || defined(__OpenBSD__) ||\
defined(OLD_NAMESER) || defined(__FreeBSD__)
    aclass = _get_short (cp);
    cp += sizeof (u_short);
#elif defined(__APPLE_CC__)
    aclass++;                   /* get rid of compiler warning... who cares */
    GETSHORT (aclass, cp);
#else
    aclass++;                   /* get rid of compiler warning... who cares */
    NS_GET16 (aclass, cp);
#endif

#if defined(__NetBSD__) || defined(__OpenBSD__) ||\
defined(OLD_NAMESER) || defined(__FreeBSD__)
    ttl = _get_long (cp);
    cp += sizeof (u_long);
#elif defined(__APPLE_CC__)
    ttl++;                      /* get rid of compiler warning... who cares */
    GETLONG (ttl, cp);
#else
    ttl++;                      /* get rid of compiler warning... who cares */
    NS_GET32 (ttl, cp);
#endif

#if defined(__NetBSD__) || defined(__OpenBSD__) ||\
defined(OLD_NAMESER) || defined(__FreeBSD__)
    dlen = _get_short (cp);
    cp += sizeof (u_short);
#elif defined(__APPLE_CC__)
    GETSHORT (dlen, cp);
#else
    NS_GET16 (dlen, cp);
#endif

    if (type != T_SRV) {
      cp += dlen;
      continue;
    }
#if defined(__NetBSD__) || defined(__OpenBSD__) ||\
defined(OLD_NAMESER) || defined(__FreeBSD__)
    pref = _get_short (cp);
    cp += sizeof (u_short);
#elif defined(__APPLE_CC__)
    GETSHORT (pref, cp);
#else
    NS_GET16 (pref, cp);
#endif

#if defined(__NetBSD__) || defined(__OpenBSD__) ||\
defined(OLD_NAMESER) || defined(__FreeBSD__)
    weight = _get_short (cp);
    cp += sizeof (u_short);
#elif defined(__APPLE_CC__)
    GETSHORT (weight, cp);
#else
    NS_GET16 (weight, cp);
#endif

#if defined(__NetBSD__) || defined(__OpenBSD__) ||\
defined(OLD_NAMESER) || defined(__FreeBSD__)
    port = _get_short (cp);
    cp += sizeof (u_short);
#elif defined(__APPLE_CC__)
    GETSHORT (port, cp);
#else
    NS_GET16 (port, cp);
#endif

    n = dn_expand (msg, eom, cp, (char *) hostbuf, 256);
    if (n < 0)
      break;
    cp += n;

    srventry = &output_srv->srventry[answerno];
    snprintf (srventry->srv, sizeof (srventry->srv), "%s", hostbuf);

    srventry->priority = pref;
    srventry->weight = weight;
    if (weight)
      srventry->rweight = (int) (1 + random () % (10000 * srventry->weight));
    else
      srventry->rweight = 0;
    srventry->port = port;

    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "SRV record %s IN SRV -> %s:%i/%i/%i/%i\n", output_srv->name, srventry->srv, srventry->port, srventry->priority, srventry->weight, srventry->rweight));

    output_srv->srv_state = OSIP_SRV_STATE_COMPLETED;

    answerno++;
    if (answerno == 10)
      break;
  }

  if (answerno == 0)
    return OSIP_UNKNOWN_HOST;

  osip_srv_record_sort (output_srv, answerno);
  return OSIP_SUCCESS;
}

static int
eXosip_dnsutils_srv_lookup (struct osip_naptr *output_record)
{
  if (output_record->naptr_state == OSIP_NAPTR_STATE_SRVDONE)
    return OSIP_SUCCESS;

  output_record->sipudp_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;
  output_record->siptcp_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;
  output_record->siptls_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;
  output_record->sipdtls_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;
  output_record->sipsctp_record.srv_state = OSIP_SRV_STATE_NOTSUPPORTED;

  _eXosip_dnsutils_srv_lookup (&output_record->sipudp_record);
  _eXosip_dnsutils_srv_lookup (&output_record->siptcp_record);
  _eXosip_dnsutils_srv_lookup (&output_record->siptls_record);
  _eXosip_dnsutils_srv_lookup (&output_record->sipdtls_record);
  /* _eXosip_dnsutils_srv_lookup(&output_record->sipsctp_record); */

  if (output_record->sipudp_record.srv_state == OSIP_SRV_STATE_COMPLETED)
    output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
  else if (output_record->siptcp_record.srv_state == OSIP_SRV_STATE_COMPLETED)
    output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
  else if (output_record->siptls_record.srv_state == OSIP_SRV_STATE_COMPLETED)
    output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
  else if (output_record->sipdtls_record.srv_state == OSIP_SRV_STATE_COMPLETED)
    output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
  else if (output_record->sipsctp_record.srv_state == OSIP_SRV_STATE_COMPLETED)
    output_record->naptr_state = OSIP_NAPTR_STATE_SRVDONE;
  else
    output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;

  return 0;
}

static int
eXosip_dnsutils_naptr_lookup (osip_naptr_t * output_record, const char *domain)
{
  querybuf answer;              /* answer buffer from nameserver */
  int n;
  int ancount, qdcount;         /* answer count and query count */

  /* int nscount, arcount;         ns count and ar count */
  HEADER *hp;                   /* answer buffer header */
  char hostbuf[256];
  unsigned char *msg, *eom, *cp;        /* answer buffer positions */
  int dlen, type, aclass;
  long ttl;
  int answerno;

  output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;
  if (domain == NULL)
    return OSIP_BADPARAMETER;

  if (strlen (domain) > 512)
    return OSIP_BADPARAMETER;

  snprintf (output_record->domain, sizeof (output_record->domain), "%s", domain);

  output_record->naptr_state = OSIP_NAPTR_STATE_INPROGRESS;

  n = res_query (domain, C_IN, T_NAPTR, (unsigned char *) &answer, sizeof (answer));

  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_naptr_lookup: About to ask for '%s NAPTR'\n", domain));

  if (n < (int) sizeof (HEADER)) {
    int hstatus = h_errno;

    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip_dnsutils_naptr_lookup: res_query failed ('%s NAPTR')\n", domain));
    if (hstatus == NO_DATA)
      output_record->naptr_state = OSIP_NAPTR_STATE_NOTSUPPORTED;
    else if (hstatus == HOST_NOT_FOUND)
      output_record->naptr_state = OSIP_NAPTR_STATE_NOTSUPPORTED;
    else
      output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;
    return OSIP_UNDEFINED_ERROR;
  }

  /* browse message and search for DNS answers part */
  hp = (HEADER *) & answer;
  qdcount = ntohs (hp->qdcount);
  ancount = ntohs (hp->ancount);
  /* nscount = ntohs (hp->ancount); */
  /* arcount = ntohs (hp->arcount); */

  msg = (unsigned char *) (&answer);
  eom = (unsigned char *) (&answer) + n;
  cp = (unsigned char *) (&answer) + sizeof (HEADER);

  while (qdcount-- > 0 && cp < eom) {
    n = dn_expand (msg, eom, cp, (char *) hostbuf, 256);
    if (n < 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Invalid SRV record answer for '%s': bad format\n", domain));
      output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;
      return OSIP_UNDEFINED_ERROR;
    }
    cp += n + QFIXEDSZ;
  }


  /* browse DNS answers */
  answerno = 0;

  /* loop through the answer buffer and extract SRV records */
  while (ancount-- > 0 && cp < eom) {
    int len;

    typedef struct {
      unsigned short order;
      unsigned short pref;
      char flag[256];
      char service[1024];
      char regexp[1024];
      char replacement[1024];
    } osip_naptr_t;

    osip_naptr_t anaptr;

    n = dn_expand (msg, eom, cp, (char *) hostbuf, 256);
    if (n < 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Invalid NAPTR answer for '%s': bad format\n", domain));
      output_record->naptr_state = OSIP_NAPTR_STATE_RETRYLATER;
      return OSIP_UNDEFINED_ERROR;
    }

    cp += n;

#if defined(__NetBSD__) || defined(__OpenBSD__) ||\
defined(OLD_NAMESER) || defined(__FreeBSD__)
    type = _get_short (cp);
    cp += sizeof (u_short);
#elif defined(__APPLE_CC__)
    GETSHORT (type, cp);
#else
    NS_GET16 (type, cp);
#endif

#if defined(__NetBSD__) || defined(__OpenBSD__) ||\
defined(OLD_NAMESER) || defined(__FreeBSD__)
    aclass = _get_short (cp);
    cp += sizeof (u_short);
#elif defined(__APPLE_CC__)
    aclass++;                   /* get rid of compiler warning... who cares */
    GETSHORT (aclass, cp);
#else
    aclass++;                   /* get rid of compiler warning... who cares */
    NS_GET16 (aclass, cp);
#endif

#if defined(__NetBSD__) || defined(__OpenBSD__) ||\
defined(OLD_NAMESER) || defined(__FreeBSD__)
    ttl = _get_long (cp);
    cp += sizeof (u_long);
#elif defined(__APPLE_CC__)
    ttl++;                      /* get rid of compiler warning... who cares */
    GETLONG (ttl, cp);
#else
    ttl++;                      /* get rid of compiler warning... who cares */
    NS_GET32 (ttl, cp);
#endif

#if defined(__NetBSD__) || defined(__OpenBSD__) ||\
defined(OLD_NAMESER) || defined(__FreeBSD__)
    dlen = _get_short (cp);
    cp += sizeof (u_short);
#elif defined(__APPLE_CC__)
    GETSHORT (dlen, cp);
#else
    NS_GET16 (dlen, cp);
#endif

    if (type != T_NAPTR) {
      cp += dlen;
      continue;
    }

    memset (&anaptr, 0, sizeof (osip_naptr_t));

    memcpy ((void *) &anaptr.order, cp, 2);
    anaptr.order = ntohs (anaptr.order);        /*((unsigned short)cp[0] << 8) | ((unsigned short)cp[1]); */
    cp += sizeof (unsigned short);
    memcpy ((void *) &anaptr.pref, cp, 2);
    anaptr.pref = ntohs (anaptr.pref);  /* ((unsigned short)cp[0] << 8) | ((unsigned short)cp[1]); */
    cp += sizeof (unsigned short);

    len = *cp;
    cp++;
    strncpy (anaptr.flag, (char *) cp, len);
    anaptr.flag[len] = '\0';
    cp += len;

    len = *cp;
    cp++;
    strncpy (anaptr.service, (char *) cp, len);
    anaptr.service[len] = '\0';
    cp += len;

    len = *cp;
    cp++;
    strncpy (anaptr.regexp, (char *) cp, len);
    anaptr.regexp[len] = '\0';
    cp += len;

    n = dn_expand (msg, eom, cp, anaptr.replacement, 1024 - 1);

    if (n < 0)
      break;
    cp += n;

    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "NAPTR %s ->%i/%i/%s/%s/%s/%s\n", domain, anaptr.order, anaptr.pref, anaptr.flag, anaptr.service, anaptr.regexp, anaptr.replacement));

    if (osip_strncasecmp (anaptr.service, "SIP+D2U", 8) == 0) {
      snprintf (output_record->sipudp_record.name, sizeof (output_record->sipudp_record.name), "%s", anaptr.replacement);
      output_record->naptr_state = OSIP_NAPTR_STATE_NAPTRDONE;
    }
    else if (osip_strncasecmp (anaptr.service, "SIP+D2T", 8) == 0) {
      snprintf (output_record->siptcp_record.name, sizeof (output_record->siptcp_record.name), "%s", anaptr.replacement);
      output_record->naptr_state = OSIP_NAPTR_STATE_NAPTRDONE;
    }
    else if (osip_strncasecmp (anaptr.service, "SIPS+D2U", 9) == 0) {
      snprintf (output_record->sipdtls_record.name, sizeof (output_record->sipdtls_record.name), "%s", anaptr.replacement);
      output_record->naptr_state = OSIP_NAPTR_STATE_NAPTRDONE;
    }
    else if (osip_strncasecmp (anaptr.service, "SIPS+D2T", 9) == 0) {
      snprintf (output_record->siptls_record.name, sizeof (output_record->siptls_record.name), "%s", anaptr.replacement);
      output_record->naptr_state = OSIP_NAPTR_STATE_NAPTRDONE;
    }
    else if (osip_strncasecmp (anaptr.service, "SIP+D2S", 8) == 0) {
      snprintf (output_record->sipsctp_record.name, sizeof (output_record->sipsctp_record.name), "%s", anaptr.replacement);
      output_record->naptr_state = OSIP_NAPTR_STATE_NAPTRDONE;
    }

    answerno++;
  }

  if (answerno == 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "No NAPTR for SIP for domain %s\n", domain));
    output_record->naptr_state = OSIP_NAPTR_STATE_NOTSUPPORTED;
    return OSIP_SUCCESS;
  }

  if (output_record->naptr_state != OSIP_NAPTR_STATE_NAPTRDONE)
    output_record->naptr_state = OSIP_NAPTR_STATE_NOTSUPPORTED;

  return OSIP_SUCCESS;
}

struct osip_naptr *
eXosip_dnsutils_naptr (struct eXosip_t *excontext, const char *domain, const char *protocol, const char *transport, int keep_in_cache)
{
  osip_list_iterator_t it;
  struct osip_naptr *naptr_record;
  int i;
  int not_in_list = 0;

  if (excontext->dns_capabilities <= 0)
    return NULL;

  if (dnsutils_list == NULL) {
    dnsutils_list = (osip_list_t *) osip_malloc (sizeof (osip_list_t));
    osip_list_init (dnsutils_list);
  }

  if (keep_in_cache < 0) {
    naptr_record = (osip_naptr_t *)osip_list_get_first(dnsutils_list, &it);
    while (naptr_record != NULL) {
      if (osip_strcasecmp (domain, naptr_record->domain) == 0) {
        if (naptr_record->naptr_state == OSIP_NAPTR_STATE_RETRYLATER)
          break;
        if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NOTSUPPORTED)
          break;

        return naptr_record;
      }

      naptr_record = NULL;
      if (it.pos == 9)
        break;
      naptr_record = (osip_naptr_t *)osip_list_get_next(&it);
    }
    return NULL;
  }

  naptr_record = (osip_naptr_t *)osip_list_get_first(dnsutils_list, &it);
  while (naptr_record != NULL) {

    /* process all */
    if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NAPTRDONE || naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVINPROGRESS)
      eXosip_dnsutils_srv_lookup (naptr_record);

    naptr_record = NULL;
    if (it.pos == 9)
      break;
    naptr_record = (osip_naptr_t *)osip_list_get_next(&it);
  }

  if (domain == NULL)
    return NULL;

  it.pos=0;
  naptr_record = (osip_naptr_t *)osip_list_get_first(dnsutils_list, &it);
  while (naptr_record != NULL) {
    if (osip_strcasecmp (domain, naptr_record->domain) == 0) {
      if (naptr_record->naptr_state == OSIP_NAPTR_STATE_RETRYLATER)
        break;

      if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NAPTRDONE || naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVINPROGRESS)
        eXosip_dnsutils_srv_lookup (naptr_record);

      return naptr_record;
    }

    naptr_record = NULL;
    if (it.pos == 9)
      break;
    naptr_record = (osip_naptr_t *)osip_list_get_next(&it);
  }

  if (it.pos == 9 && keep_in_cache > 0) {
    /* no NAPTR found within the last 10 NAPTR : refuse to keep in cache... */
    /* If we were adding unlimited NAPTR record into the cache, the program
       would infinitly increase memory usage. If you reach there, then you
       most probably don't use the API correctly: Only NAPTR related to
       registration will end up in the cache. So in theory, 10 NAPTR will be
       cached for 10 accounts, which seems enough for a softphone. Then all
       SIP message should go through the same proxy by using a pre-route set.
       Correctly using the API will definitly send out-of-dialog SIP message
       to the same proxy as the REGISTER. If you have issue with in-dialog
       Request or Response and NAPTR, that means your proxy is not following
       guidelines from the rfc3261 and rfc3263 where proxy should specify
       port numbers when they want you to resolve the host as a A record
       -and then avoid NAPTR-.
     */
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip_dnsutils_naptr: Time will tell how much you go there (%s) - it's wrong code path, fix it\n", domain));

    keep_in_cache = 0;

    naptr_record = (osip_naptr_t *) osip_malloc (sizeof (osip_naptr_t));
    memset (naptr_record, 0, sizeof (osip_naptr_t));
    naptr_record->keep_in_cache = keep_in_cache;
  }
  else if (naptr_record == NULL) {
    naptr_record = (osip_naptr_t *) osip_malloc (sizeof (osip_naptr_t));
    memset (naptr_record, 0, sizeof (osip_naptr_t));
    naptr_record->keep_in_cache = keep_in_cache;
    not_in_list = 1;
  }
  else {
    /* it was found, so it WAS in cache before, but we were in "retry" state */
    memset (naptr_record, 0, sizeof (osip_naptr_t));
    naptr_record->keep_in_cache = 1;
  }

  i = eXosip_dnsutils_naptr_lookup (naptr_record, domain);
  if (i < 0) {
    if (keep_in_cache <= 0) {
      return naptr_record;
    }
    if (not_in_list == 1) {
      osip_list_add (dnsutils_list, naptr_record, -1);
    }
    return naptr_record;
  }

  if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NAPTRDONE || naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVINPROGRESS)
    eXosip_dnsutils_srv_lookup (naptr_record);

  if (keep_in_cache <= 0) {
    return naptr_record;
  }
  if (not_in_list == 1)
    osip_list_add (dnsutils_list, naptr_record, -1);
  return naptr_record;
}

int
eXosip_dnsutils_dns_process (osip_naptr_t * naptr_record, int force)
{
  return OSIP_SUCCESS;
}

void
_eXosip_dnsutils_release (osip_naptr_t * naptr_record)
{
  if (naptr_record == NULL)
    return;
  if (naptr_record->keep_in_cache > 0)
    return;
  osip_free (naptr_record);
}

#else

struct osip_naptr *
eXosip_dnsutils_naptr (struct eXosip_t *excontext, const char *domain, const char *protocol, const char *transport, int keep_in_cache)
{
  return NULL;
}

int
eXosip_dnsutils_dns_process (osip_naptr_t * naptr_record, int force)
{
  return OSIP_UNDEFINED_ERROR;
}

void
_eXosip_dnsutils_release (osip_naptr_t * naptr_record)
{
  return;
}

#endif

#else

struct osip_naptr *
eXosip_dnsutils_naptr (struct eXosip_t *excontext, const char *domain, const char *protocol, const char *transport, int keep_in_cache)
{
  return NULL;
}

int
eXosip_dnsutils_dns_process (osip_naptr_t * naptr_record, int force)
{
  return OSIP_UNDEFINED_ERROR;
}

void
_eXosip_dnsutils_release (osip_naptr_t * naptr_record)
{
  return;
}

#endif
