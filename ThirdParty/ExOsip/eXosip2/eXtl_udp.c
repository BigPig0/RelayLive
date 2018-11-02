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

#ifdef _WIN32_WCE
#include "inet_ntop.h"
#elif WIN32
#include "inet_ntop.h"
#endif

#if defined(_WIN32_WCE)
#define strerror(X) "-1"
#endif

#if !defined(_WIN32_WCE)
#include <errno.h>
#endif

#ifdef __APPLE_CC__
#include "TargetConditionals.h"
#endif

#ifdef TSC_SUPPORT
#include "tsc_socket_api.h"
#include "tsc_control_api.h"
#endif

#if (_WIN32_WINNT >= 0x0600)
#define ENABLE_SIP_QOS
#if (_MSC_VER >= 1700) && !defined(_USING_V110_SDK71_)
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
#undef ENABLE_SIP_QOS
#endif
#endif
#endif


#ifdef ENABLE_SIP_QOS
#include <delayimp.h>
#undef ExternC
#include <QOS2.h>
#endif

struct _udp_stream {
  char remote_host[256];
  char remote_ip[64];
  int remote_port;
  int out_socket;
};

#ifndef EXOSIP_MAX_SOCKETS
#define EXOSIP_MAX_SOCKETS 200
#endif

/* recv on long message returns -1 with errno=0 */
#if !defined(WIN32) && !defined(_WIN32_WCE)
#define SOCKET_OPTION_VALUE	void *
static size_t udp_message_max_length = SIP_MESSAGE_MAX_LENGTH;
#else
static int udp_message_max_length = SIP_MESSAGE_MAX_LENGTH;
#define SOCKET_OPTION_VALUE char *
#endif

struct eXtludp {
  int udp_socket;
  struct sockaddr_storage ai_addr;
  char *buf;
  void *QoSHandle;
  unsigned long QoSFlowID;
  
  int udp_socket_oc;
  struct sockaddr_storage ai_addr_oc;
  
  struct _udp_stream socket_tab[EXOSIP_MAX_SOCKETS];
};

static int
udp_tl_init (struct eXosip_t *excontext)
{
  struct eXtludp *reserved = (struct eXtludp *) osip_malloc (sizeof (struct eXtludp));

  if (reserved == NULL)
    return OSIP_NOMEM;
  memset (reserved, 0, sizeof (struct eXtludp));

  excontext->eXtludp_reserved = reserved;
  return OSIP_SUCCESS;
}

static int
udp_tl_free (struct eXosip_t *excontext)
{
  struct eXtludp *reserved = (struct eXtludp *) excontext->eXtludp_reserved;

  if (reserved == NULL)
    return OSIP_SUCCESS;

#ifdef ENABLE_SIP_QOS
  if (reserved->QoSFlowID != 0)
  {
    OSVERSIONINFOEX ovi;
    memset(&ovi, 0, sizeof(ovi));
    ovi.dwOSVersionInfoSize = sizeof(ovi);
    GetVersionEx((LPOSVERSIONINFO) & ovi);

    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "QOS: check OS support for qwave.lib: %i %i %i\n",
      ovi.dwMajorVersion, ovi.dwMinorVersion, ovi.dwBuildNumber));
    if (ovi.dwMajorVersion > 5) {
      HRESULT hr = E_FAIL;
      __try {
        hr = __HrLoadAllImportsForDll("qwave.dll");
      }
      __except(EXCEPTION_EXECUTE_HANDLER) {
        hr = E_FAIL;
      }
      if (! SUCCEEDED(hr)) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "QOS: Failed to load qwave.dll: no QoS available\n"));
      }
      else
      {
        BOOL QoSResult;
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "QOS: QoS API detected\n"));
        QoSResult = QOSRemoveSocketFromFlow(reserved->QoSHandle, 
          0, 
          reserved->QoSFlowID, 
          0);

        if (QoSResult != TRUE){
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "QOS: QOSRemoveSocketFromFlow failed to end a flow with error\n"));
        }
        reserved->QoSFlowID=0;
      }
    }
  }

  if (reserved->QoSHandle != NULL)
  {
    QOSCloseHandle(reserved->QoSHandle);
    reserved->QoSHandle=NULL;
  }
#endif

  memset (&reserved->ai_addr, 0, sizeof (struct sockaddr_storage));
#ifdef TSC_SUPPORT
  if (reserved->udp_socket > 0) {
    if (excontext->tunnel_handle) {
      tsc_close (reserved->udp_socket);
      reserved->udp_socket = 0;
    }
  }
#endif
  if (reserved->udp_socket > 0)
    _eXosip_closesocket (reserved->udp_socket);
  if (reserved->udp_socket_oc > 0)
    _eXosip_closesocket (reserved->udp_socket_oc);

  if (reserved->buf != NULL)
    osip_free(reserved->buf);

  osip_free (reserved);
  excontext->eXtludp_reserved = NULL;
  return OSIP_SUCCESS;
}

#ifdef ENABLE_SIP_QOS
static int
_udp_tl_transport_set_dscp_qos (struct eXosip_t *excontext, struct sockaddr *rem_addr, int rem_addrlen)
{
  int res=0;
  QOS_TRAFFIC_TYPE tos;
  OSVERSIONINFOEX ovi;

  struct eXtludp *reserved = (struct eXtludp *) excontext->eXtludp_reserved;

  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "QOS: wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }

  if (excontext->tunnel_handle)
    return 0;

  if (excontext->dscp<=0)
    return 0;

  memset(&ovi, 0, sizeof(ovi));
  ovi.dwOSVersionInfoSize = sizeof(ovi);
  GetVersionEx((LPOSVERSIONINFO) & ovi);

  if (ovi.dwMajorVersion > 5) {
    HRESULT hr = E_FAIL;

    if (excontext->dscp<=0x8)
      tos=QOSTrafficTypeBackground;
    else if (excontext->dscp<=0x28)
      tos=QOSTrafficTypeAudioVideo;
    else if (excontext->dscp<=0x38)
      tos=QOSTrafficTypeVoice;
    else
      tos=QOSTrafficTypeExcellentEffort; /* 0x28 */

    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "QOS: Check OS support for qwave.lib: %i %i %i\n",
      ovi.dwMajorVersion, ovi.dwMinorVersion, ovi.dwBuildNumber));

    __try {
      hr = __HrLoadAllImportsForDll("qwave.dll");
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
      hr = E_FAIL;
    }
    if (! SUCCEEDED(hr)) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "QOS: Failed to load qwave.dll: no QoS available\n"));
		}
		else
		{
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "QoS API detected\n"));
			if (excontext->dscp==0)
				tos=QOSTrafficTypeBestEffort;
			else if (excontext->dscp<=0x8)
				tos=QOSTrafficTypeBackground;
			else if (excontext->dscp<=0x28)
				tos=QOSTrafficTypeAudioVideo;
			else if (excontext->dscp<=0x38)
				tos=QOSTrafficTypeVoice;
			else
				tos=QOSTrafficTypeExcellentEffort; /* 0x28 */

			if (reserved->QoSHandle==NULL) {
				QOS_VERSION version;
				BOOL QoSResult;

				version.MajorVersion = 1;
				version.MinorVersion = 0;

				QoSResult = QOSCreateHandle(&version, &reserved->QoSHandle);

				if (QoSResult != TRUE){
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "QOS: QOSCreateHandle failed to create handle with error\n"));
					res=-1;
				}
			}
			if (reserved->QoSHandle!=NULL && rem_addrlen>0) {
				BOOL QoSResult;
				QoSResult = QOSAddSocketToFlow(
					reserved->QoSHandle, 
					reserved->udp_socket,
					rem_addr,
					tos, 
					QOS_NON_ADAPTIVE_FLOW, 
					&reserved->QoSFlowID);

				if (QoSResult != TRUE){
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "QOS: QOSAddSocketToFlow failed to add a flow with error\n"));
					res=-1;
				}
			}
		}
	  if (res<0)
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "QOS: Failed to set DSCP value on socket\n"));
    return res;
	}
  return OSIP_SUCCESS;
}
#endif

int
_eXosip_transport_set_dscp (struct eXosip_t *excontext, int family, int sock)
{
  int res;

  if (excontext->tunnel_handle)
    return 0;

  if (family == AF_INET) {
    int tos = (excontext->dscp << 2) & 0xFC;

    res = setsockopt (sock, IPPROTO_IP, IP_TOS, (SOCKET_OPTION_VALUE) & tos, sizeof (tos));
  }
  else {
    int tos = (excontext->dscp << 2) & 0xFC;

#ifdef IPV6_TCLASS
    res = setsockopt (sock, IPPROTO_IPV6, IPV6_TCLASS, (SOCKET_OPTION_VALUE) & tos, sizeof (tos));
#else
    res = setsockopt (sock, IPPROTO_IPV6, IP_TOS, (SOCKET_OPTION_VALUE) & tos, sizeof (tos));
#endif
  }
  return res;
}

static int
_udp_tl_open (struct eXosip_t *excontext)
{
  struct eXtludp *reserved = (struct eXtludp *) excontext->eXtludp_reserved;
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
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO3, NULL, "eXosip: Skipping protocol %d\n", curinfo->ai_protocol));
      continue;
    }

#ifdef TSC_SUPPORT
    if (excontext->tunnel_handle) {
      sock = (int) tsc_socket (excontext->tunnel_handle, curinfo->ai_family, curinfo->ai_socktype, curinfo->ai_protocol);
    }
    else {
      sock = (int) socket (curinfo->ai_family, curinfo->ai_socktype, curinfo->ai_protocol);
    }
#else
    sock = (int) socket (curinfo->ai_family, curinfo->ai_socktype, curinfo->ai_protocol);
#endif
    if (sock < 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot create socket %s!\n", strerror (errno)));
      continue;
    }

    if (curinfo->ai_family == AF_INET6) {
#ifdef IPV6_V6ONLY
      if (setsockopt_ipv6only (sock)) {
        _eXosip_closesocket (sock);
        sock = -1;
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot set socket option %s!\n", strerror (errno)));
        continue;
      }
#endif /* IPV6_V6ONLY */
    }

    {
      int valopt = 1;

      setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (void *) &valopt, sizeof (valopt));
    }

#if SO_NOSIGPIPE
    {
      int val;

      val = 1;
      setsockopt (sock, SOL_SOCKET, SO_NOSIGPIPE, (void *) &val, sizeof (int));
    }
#endif

#ifdef TSC_SUPPORT
    if (excontext->tunnel_handle) {
      tsc_config config;
      struct sockaddr_in *addr;

      tsc_get_config (excontext->tunnel_handle, &config);

      addr = (struct sockaddr_in *) (curinfo->ai_addr);
      addr->sin_addr.s_addr = htonl (config.internal_address.address);
      res = tsc_bind (sock, curinfo->ai_addr, (int)curinfo->ai_addrlen);
    }
    else {
      res = bind (sock, curinfo->ai_addr, (socklen_t)curinfo->ai_addrlen);
    }
#else
    res = bind (sock, curinfo->ai_addr, (socklen_t)curinfo->ai_addrlen);
#endif

    if (res < 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot bind socket node:%s family:%d %s\n", excontext->eXtl_transport.proto_ifs, curinfo->ai_family, strerror (errno)));
#ifdef TSC_SUPPORT
      if (excontext->tunnel_handle) {
        tsc_close (sock);
      }
      else {
        _eXosip_closesocket (sock);
      }
#else
      _eXosip_closesocket (sock);
#endif
      sock = -1;
      continue;
    }

#ifdef TSC_SUPPORT
    if (excontext->tunnel_handle) {
      len = sizeof (reserved->ai_addr);
      res = tsc_getsockname (sock, (struct sockaddr *) &reserved->ai_addr, &len);
      if (res != 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot get socket name (%s)\n", strerror (errno)));
        memcpy (&reserved->ai_addr, curinfo->ai_addr, curinfo->ai_addrlen);
      }
    }
    else {
      len = sizeof (reserved->ai_addr);
      res = getsockname (sock, (struct sockaddr *) &reserved->ai_addr, &len);
      if (res != 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot get socket name (%s)\n", strerror (errno)));
        memcpy (&reserved->ai_addr, curinfo->ai_addr, curinfo->ai_addrlen);
      }
    }
#else
    len = sizeof (reserved->ai_addr);
    res = getsockname (sock, (struct sockaddr *) &reserved->ai_addr, &len);
    if (res != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot get socket name (%s)\n", strerror (errno)));
      memcpy (&reserved->ai_addr, curinfo->ai_addr, curinfo->ai_addrlen);
    }
#endif
    if (excontext->eXtl_transport.proto_num != IPPROTO_UDP) {
      res = listen (sock, SOMAXCONN);
      if (res < 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot bind socket node:%s family:%d %s\n", excontext->eXtl_transport.proto_ifs, curinfo->ai_family, strerror (errno)));
        _eXosip_closesocket (sock);
        sock = -1;
        continue;
      }
    }

    break;
  }

  _eXosip_freeaddrinfo (addrinfo);

  if (sock < 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot bind on port: %i\n", excontext->eXtl_transport.proto_local_port));
    return -1;
  }

  reserved->udp_socket = sock;

  _eXosip_transport_set_dscp (excontext, excontext->eXtl_transport.proto_family, sock);

  if (excontext->eXtl_transport.proto_local_port == 0) {
    /* get port number from socket */
    if (excontext->eXtl_transport.proto_family == AF_INET)
      excontext->eXtl_transport.proto_local_port = ntohs (((struct sockaddr_in *) &reserved->ai_addr)->sin_port);
    else
      excontext->eXtl_transport.proto_local_port = ntohs (((struct sockaddr_in6 *) &reserved->ai_addr)->sin6_port);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "eXosip: Binding on port %i!\n", excontext->eXtl_transport.proto_local_port));
  }
  return OSIP_SUCCESS;
}

static int
_udp_tl_open_oc (struct eXosip_t *excontext)
{
  struct eXtludp *reserved = (struct eXtludp *) excontext->eXtludp_reserved;
  int res;
  struct addrinfo *addrinfo = NULL;
  struct addrinfo *curinfo;
  int sock = -1;
  
  if (excontext->oc_local_address[0]=='\0')
    return OSIP_SUCCESS;
    
  res = _eXosip_get_addrinfo (excontext, &addrinfo, excontext->oc_local_address, excontext->oc_local_port_range[0], excontext->eXtl_transport.proto_num);
  if (res)
    return -1;
  
  for (curinfo = addrinfo; curinfo; curinfo = curinfo->ai_next) {
    socklen_t len;
    
    if (curinfo->ai_protocol && curinfo->ai_protocol != excontext->eXtl_transport.proto_num) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO3, NULL, "eXosip: Skipping protocol %d\n", curinfo->ai_protocol));
      continue;
    }
    
#ifdef TSC_SUPPORT
    if (excontext->tunnel_handle) {
      sock = (int) tsc_socket (excontext->tunnel_handle, curinfo->ai_family, curinfo->ai_socktype, curinfo->ai_protocol);
    }
    else {
      sock = (int) socket (curinfo->ai_family, curinfo->ai_socktype, curinfo->ai_protocol);
    }
#else
    sock = (int) socket (curinfo->ai_family, curinfo->ai_socktype, curinfo->ai_protocol);
#endif
    if (sock < 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot create socket %s!\n", strerror (errno)));
      continue;
    }
    
    if (curinfo->ai_family == AF_INET6) {
#ifdef IPV6_V6ONLY
      if (setsockopt_ipv6only (sock)) {
        _eXosip_closesocket (sock);
        sock = -1;
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot set socket option %s!\n", strerror (errno)));
        continue;
      }
#endif /* IPV6_V6ONLY */
    }
    
    {
      int valopt = 1;
      
      setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (void *) &valopt, sizeof (valopt));
    }
    
#if SO_NOSIGPIPE
    {
      int val;
      
      val = 1;
      setsockopt (sock, SOL_SOCKET, SO_NOSIGPIPE, (void *) &val, sizeof (int));
    }
#endif
    
#ifdef TSC_SUPPORT
    if (excontext->tunnel_handle) {
      tsc_config config;
      struct sockaddr_in *addr;
      
      tsc_get_config (excontext->tunnel_handle, &config);
      
      addr = (struct sockaddr_in *) (curinfo->ai_addr);
      addr->sin_addr.s_addr = htonl (config.internal_address.address);
      res = tsc_bind (sock, curinfo->ai_addr, (int)curinfo->ai_addrlen);
    }
    else {
      res = bind (sock, curinfo->ai_addr, (socklen_t)curinfo->ai_addrlen);
    }
#else
    res = bind (sock, curinfo->ai_addr, (socklen_t)curinfo->ai_addrlen);
#endif
    
    if (res < 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot bind socket node:%s family:%d %s\n", excontext->eXtl_transport.proto_ifs, curinfo->ai_family, strerror (errno)));
#ifdef TSC_SUPPORT
      if (excontext->tunnel_handle) {
        tsc_close (sock);
      }
      else {
        _eXosip_closesocket (sock);
      }
#else
      _eXosip_closesocket (sock);
#endif
      sock = -1;
      continue;
    }
    
#ifdef TSC_SUPPORT
    if (excontext->tunnel_handle) {
      len = sizeof (reserved->ai_addr_oc);
      res = tsc_getsockname (sock, (struct sockaddr *) &reserved->ai_addr_oc, &len);
      if (res != 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot get socket name (%s)\n", strerror (errno)));
        memcpy (&reserved->ai_addr_oc, curinfo->ai_addr, curinfo->ai_addrlen);
      }
    }
    else {
      len = sizeof (reserved->ai_addr_oc);
      res = getsockname (sock, (struct sockaddr *) &reserved->ai_addr_oc, &len);
      if (res != 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot get socket name (%s)\n", strerror (errno)));
        memcpy (&reserved->ai_addr_oc, curinfo->ai_addr, curinfo->ai_addrlen);
      }
    }
#else
    len = sizeof (reserved->ai_addr_oc);
    res = getsockname (sock, (struct sockaddr *) &reserved->ai_addr_oc, &len);
    if (res != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot get socket name (%s)\n", strerror (errno)));
      memcpy (&reserved->ai_addr_oc, curinfo->ai_addr, curinfo->ai_addrlen);
    }
#endif
    if (excontext->eXtl_transport.proto_num != IPPROTO_UDP) {
      res = listen (sock, SOMAXCONN);
      if (res < 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot bind socket node:%s family:%d %s\n", excontext->eXtl_transport.proto_ifs, curinfo->ai_family, strerror (errno)));
        _eXosip_closesocket (sock);
        sock = -1;
        continue;
      }
    }
    
    break;
  }
  
  _eXosip_freeaddrinfo (addrinfo);
  
  if (sock < 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot bind on port: %i\n", excontext->oc_local_port_range[0]));
    return -1;
  }
  
  reserved->udp_socket_oc = sock;
  
  _eXosip_transport_set_dscp (excontext, excontext->eXtl_transport.proto_family, sock);
  
  return OSIP_SUCCESS;
}

static int
udp_tl_open (struct eXosip_t *excontext)
{
  struct eXtludp *reserved = (struct eXtludp *) excontext->eXtludp_reserved;
  int res;
  
  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }
  
  res = _udp_tl_open(excontext);
  _udp_tl_open_oc(excontext);
  return res;
}

static int
_udp_tl_reset (struct eXosip_t *excontext)
{
  struct eXtludp *reserved = (struct eXtludp *) excontext->eXtludp_reserved;

  if (reserved->udp_socket > 0)
    _eXosip_closesocket (reserved->udp_socket);
  reserved->udp_socket=0;
  return _udp_tl_open (excontext);
  }

static int
_udp_tl_reset_oc (struct eXosip_t *excontext)
{
  struct eXtludp *reserved = (struct eXtludp *) excontext->eXtludp_reserved;
  
  if (reserved->udp_socket_oc > 0)
    _eXosip_closesocket (reserved->udp_socket_oc);
  reserved->udp_socket_oc=0;
  return _udp_tl_open_oc (excontext);
}

static int
udp_tl_set_fdset (struct eXosip_t *excontext, fd_set * osip_fdset, fd_set * osip_wrset, int *fd_max)
{
  struct eXtludp *reserved = (struct eXtludp *) excontext->eXtludp_reserved;

  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }

  if (reserved->udp_socket <= 0)
    return -1;

#ifdef TSC_SUPPORT
  if (!excontext->tunnel_handle) {
    eXFD_SET (reserved->udp_socket, osip_fdset);
  }
#else
  eXFD_SET (reserved->udp_socket, osip_fdset);
#endif

  if (reserved->udp_socket > *fd_max)
    *fd_max = reserved->udp_socket;

  if (reserved->udp_socket_oc > 0)
  {
#ifdef TSC_SUPPORT
    if (!excontext->tunnel_handle) {
      eXFD_SET (reserved->udp_socket_oc, osip_fdset);
    }
#else
    eXFD_SET (reserved->udp_socket_oc, osip_fdset);
#endif
    
    if (reserved->udp_socket_oc > *fd_max)
      *fd_max = reserved->udp_socket_oc;
  }
  
  return OSIP_SUCCESS;
}

static int
udp_tl_read_message (struct eXosip_t *excontext, fd_set * osip_fdset, fd_set * osip_wrset)
{
  struct eXtludp *reserved = (struct eXtludp *) excontext->eXtludp_reserved;
  socklen_t slen;
  int i;

  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }

  if (reserved->udp_socket <= 0)
    return -1;

  if (excontext->eXtl_transport.proto_family == AF_INET)
    slen = sizeof (struct sockaddr_in);
  else
    slen = sizeof (struct sockaddr_in6);

  if (FD_ISSET (reserved->udp_socket, osip_fdset)) {
    struct sockaddr_storage sa;

    if (reserved->buf == NULL)
      reserved->buf = (char *) osip_malloc (udp_message_max_length * sizeof (char) + 1);
    if (reserved->buf == NULL)
      return OSIP_NOMEM;

#ifdef TSC_SUPPORT
    if (excontext->tunnel_handle) {
      i = tsc_recvfrom (reserved->udp_socket, reserved->buf, udp_message_max_length, 0, (struct sockaddr *) &sa, &slen);
    }
    else {
      i = recvfrom (reserved->udp_socket, reserved->buf, udp_message_max_length, 0, (struct sockaddr *) &sa, &slen);
    }
#else
    i = (int) recvfrom (reserved->udp_socket, reserved->buf, udp_message_max_length, 0, (struct sockaddr *) &sa, &slen);
#endif

    if (i > 32) {
      char src6host[NI_MAXHOST];
      int recvport = 0;

      reserved->buf[i] = '\0';

      memset (src6host, 0, NI_MAXHOST);
      recvport = _eXosip_getport((struct sockaddr *) &sa, slen);
      _eXosip_getnameinfo((struct sockaddr *) &sa, slen, src6host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Message received from: %s:%i\n", src6host, recvport));

      _eXosip_handle_incoming_message (excontext, reserved->buf, i, reserved->udp_socket, src6host, recvport, NULL, NULL);

      /* if we have a second socket for outbound connection, save information about inbound traffic initiated by receiving data on udp_socket */
      if (reserved->udp_socket_oc > 0)
      {
        int pos;
        for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
          /* does the entry already exist? */
          if (reserved->socket_tab[pos].remote_port == recvport && osip_strcasecmp(reserved->socket_tab[pos].remote_ip, src6host)==0) {
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "inbound traffic/connection already in table\n"));
            break;
          }
        }
        if (pos == EXOSIP_MAX_SOCKETS) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "inbound traffic/new connection detected (%s:%i\n", src6host, recvport));
          for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
            if (reserved->socket_tab[pos].out_socket ==0) {
              reserved->socket_tab[pos].out_socket=reserved->udp_socket;
              snprintf(reserved->socket_tab[pos].remote_ip, sizeof(reserved->socket_tab[pos].remote_ip), "%s", src6host);
              reserved->socket_tab[pos].remote_port=recvport;
              OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "inbound traffic/new connection added in table\n"));
              break;
            }
          }
        }
      }
      
    }
    else if (i < 0) {
#ifdef _WIN32_WCE
      int my_errno = 0;
#else
      int my_errno = errno;
#endif
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Could not read socket (%i) (%i) (%s)\n", i, my_errno, strerror (my_errno)));
      if (errno==0 || errno==34) {
        udp_message_max_length = udp_message_max_length*2;
        osip_free(reserved->buf);
        reserved->buf = (char *) osip_malloc (udp_message_max_length * sizeof (char) + 1);
      }
      if (my_errno == 57) {
        _udp_tl_reset (excontext);
      }
    }
    else {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Dummy SIP message received\n"));
    }
  }

  if (reserved->udp_socket_oc > 0 && FD_ISSET (reserved->udp_socket_oc, osip_fdset)) {
    struct sockaddr_storage sa;
    
    if (reserved->buf == NULL)
      reserved->buf = (char *) osip_malloc (udp_message_max_length * sizeof (char) + 1);
    if (reserved->buf == NULL)
      return OSIP_NOMEM;
    
#ifdef TSC_SUPPORT
    if (excontext->tunnel_handle) {
      i = tsc_recvfrom (reserved->udp_socket_oc, reserved->buf, udp_message_max_length, 0, (struct sockaddr *) &sa, &slen);
    }
    else {
      i = recvfrom (reserved->udp_socket_oc, reserved->buf, udp_message_max_length, 0, (struct sockaddr *) &sa, &slen);
	}
#else
    i = (int) recvfrom (reserved->udp_socket_oc, reserved->buf, udp_message_max_length, 0, (struct sockaddr *) &sa, &slen);
#endif
    
    if (i > 32) {
      char src6host[NI_MAXHOST];
      int recvport = 0;
      
      reserved->buf[i] = '\0';
      
      memset (src6host, 0, NI_MAXHOST);
      recvport = _eXosip_getport((struct sockaddr *) &sa, slen);
      _eXosip_getnameinfo((struct sockaddr *) &sa, slen, src6host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Message received from: %s:%i\n", src6host, recvport));
      
      _eXosip_handle_incoming_message (excontext, reserved->buf, i, reserved->udp_socket_oc, src6host, recvport, NULL, NULL);
      
    }
    else if (i < 0) {
#ifdef _WIN32_WCE
      int my_errno = 0;
#else
      int my_errno = errno;
#endif
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Could not read socket (%i) (%i) (%s)\n", i, my_errno, strerror (my_errno)));
      if (errno==0 || errno==34) {
        udp_message_max_length = udp_message_max_length*2;
        osip_free(reserved->buf);
        reserved->buf = (char *) osip_malloc (udp_message_max_length * sizeof (char) + 1);
      }
      if (my_errno == 57) {
        _udp_tl_reset_oc (excontext);
      }
    }
    else {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Dummy SIP message received\n"));
    }
  }
  
  return OSIP_SUCCESS;
}

static int
udp_tl_update_contact (struct eXosip_t *excontext, osip_message_t * req)
{
  req->application_data = (void*) 0x1; /* request for masquerading */
  return OSIP_SUCCESS;
}

static int
_udp_tl_update_contact (struct eXosip_t *excontext, osip_message_t * req)
{
  struct eXosip_account_info *ainfo = NULL;
  char *proxy = NULL;
  int i;
  osip_via_t *via=NULL;

  if (req->application_data != (void*) 0x1)
    return OSIP_SUCCESS;
  req->application_data = (void*) 0x0; /* avoid doing twice */

  if (MSG_IS_REQUEST (req)) {
    if (req->from != NULL && req->from->url != NULL && req->from->url->host != NULL)
      proxy = req->from->url->host;
    osip_message_get_via (req, 0, &via);
  }
  else {
    if (req->to != NULL && req->to->url != NULL && req->to->url->host != NULL)
      proxy = req->to->url->host;
  }

  if (proxy != NULL) {
    for (i = 0; i < MAX_EXOSIP_ACCOUNT_INFO; i++) {
      if (excontext->account_entries[i].proxy[0] != '\0') {
        if (strstr (excontext->account_entries[i].proxy, proxy) != NULL || strstr (proxy, excontext->account_entries[i].proxy) != NULL) {
          /* use ainfo */
          if (excontext->account_entries[i].nat_ip[0] != '\0') {
            ainfo = &excontext->account_entries[i];
            break;
          }
        }
      }
    }
  }

  if (excontext->udp_firewall_ip[0] != '\0' || excontext->auto_masquerade_contact > 0) {
    osip_list_iterator_t it;
    osip_contact_t* co = (osip_contact_t *)osip_list_get_first(&req->contacts, &it);
    while (co != NULL) {
      if (co != NULL && co->url != NULL && co->url->host != NULL) {
        if (ainfo == NULL) {
          if (excontext->udp_firewall_port[0]=='\0') {
          } else if (co->url->port == NULL && 0 != osip_strcasecmp (excontext->udp_firewall_port, "5060")) {
            co->url->port = osip_strdup (excontext->udp_firewall_port);
            osip_message_force_update (req);
          }
          else if (co->url->port != NULL && 0 != osip_strcasecmp (excontext->udp_firewall_port, co->url->port)) {
            osip_free (co->url->port);
            co->url->port = osip_strdup (excontext->udp_firewall_port);
            osip_message_force_update (req);
          }
        }
        else {
          if (co->url->port == NULL && ainfo->nat_port != 5060) {
            co->url->port = osip_malloc (10);
            if (co->url->port == NULL)
              return OSIP_NOMEM;
            snprintf (co->url->port, 9, "%i", ainfo->nat_port);
            osip_message_force_update (req);
          }
          else if (co->url->port != NULL && ainfo->nat_port != atoi (co->url->port)) {
            osip_free (co->url->port);
            co->url->port = osip_malloc (10);
            if (co->url->port == NULL)
              return OSIP_NOMEM;
            snprintf (co->url->port, 9, "%i", ainfo->nat_port);
            osip_message_force_update (req);
          }
#if 1
          if (ainfo->nat_ip[0] != '\0') {
            osip_free (co->url->host);
            co->url->host = osip_strdup (ainfo->nat_ip);
            osip_message_force_update (req);
          }
#endif
        }
      }
      co = (osip_contact_t *)osip_list_get_next(&it);
    }
  }

  if (excontext->masquerade_via)
    if (via!=NULL) {
        if (ainfo == NULL) {
          if (excontext->udp_firewall_port[0]=='\0') {
          } else if (via->port == NULL && 0 != osip_strcasecmp (excontext->udp_firewall_port, "5060")) {
            via->port = osip_strdup (excontext->udp_firewall_port);
            osip_message_force_update (req);
          }
          else if (via->port != NULL && 0 != osip_strcasecmp (excontext->udp_firewall_port, via->port)) {
            osip_free (via->port);
            via->port = osip_strdup (excontext->udp_firewall_port);
            osip_message_force_update (req);
          }
        }
        else {
          if (via->port == NULL && ainfo->nat_port != 5060) {
            via->port = osip_malloc (10);
            if (via->port == NULL)
              return OSIP_NOMEM;
            snprintf (via->port, 9, "%i", ainfo->nat_port);
            osip_message_force_update (req);
          }
          else if (via->port != NULL && ainfo->nat_port != atoi (via->port)) {
            osip_free (via->port);
            via->port = osip_malloc (10);
            if (via->port == NULL)
              return OSIP_NOMEM;
            snprintf (via->port, 9, "%i", ainfo->nat_port);
            osip_message_force_update (req);
          }
#if 1
          if (ainfo->nat_ip[0] != '\0') {
            osip_free (via->host);
            via->host = osip_strdup (ainfo->nat_ip);
            osip_message_force_update (req);
          }
#endif
        }
    }
  return OSIP_SUCCESS;
}

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 65
#endif

static int
udp_tl_send_message (struct eXosip_t *excontext, osip_transaction_t * tr, osip_message_t * sip, char *host, int port, int out_socket)
{
  struct eXtludp *reserved = (struct eXtludp *) excontext->eXtludp_reserved;
  socklen_t len = 0;
  size_t length = 0;
  struct addrinfo *addrinfo;
  struct __eXosip_sockaddr addr;
  char *message = NULL;

  char ipbuf[INET6_ADDRSTRLEN];
  int i;
  osip_naptr_t *naptr_record = NULL;
  int sock;
  struct sockaddr_storage *local_ai_addr;
  int local_port;

  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }

  if (reserved->udp_socket <= 0)
    return -1;

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
      if (naptr_record->sipudp_record.name[0] != '\0' && naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index].srv[0] != '\0') {
        /* always choose the first here.
           if a network error occur, remove first entry and
           replace with next entries.
         */
        osip_srv_entry_t *srv;
        int n = 0;

        for (srv = &naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index];
             n < 10 && naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index].srv[0]; srv = &naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index]) {
          if (srv->ipaddress[0])
            i = _eXosip_get_addrinfo (excontext, &addrinfo, srv->ipaddress, srv->port, IPPROTO_UDP);
          else
            i = _eXosip_get_addrinfo (excontext, &addrinfo, srv->srv, srv->port, IPPROTO_UDP);
          if (i == 0) {
            host = srv->srv;
            port = srv->port;
            break;
          }

          i = eXosip_dnsutils_rotate_srv (&naptr_record->sipudp_record);
          if (i <= 0) {
            return -1;
          }
          if (i >= n) {
            return -1;
          }
          i = -1;
          /* copy next element */
          n++;
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
      if (naptr_record->sipudp_record.name[0] != '\0' && naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index].srv[0] != '\0') {
        /* always choose the first here.
           if a network error occur, remove first entry and
           replace with next entries.
         */
        osip_srv_entry_t *srv;
        int n = 0;

        if (MSG_IS_REGISTER (sip) || MSG_IS_OPTIONS (sip)) {
          /* activate the failover capability: for no answer OR 503 */
          if (naptr_record->sipudp_record.srventry[tr->naptr_record->sipudp_record.index].srv_is_broken.tv_sec>0) {
            naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index].srv_is_broken.tv_sec=0;
            naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index].srv_is_broken.tv_usec=0;
            if (eXosip_dnsutils_rotate_srv (&naptr_record->sipudp_record) > 0) {
              OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL,
                                      "Doing UDP failover: ->%s:%i\n", naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index].srv, naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index].port));
            }
          }
        }
        
        for (srv = &naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index];
             n < 10 && naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index].srv[0]; srv = &naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index]) {
          if (srv->ipaddress[0])
            i = _eXosip_get_addrinfo (excontext, &addrinfo, srv->ipaddress, srv->port, IPPROTO_UDP);
          else
            i = _eXosip_get_addrinfo (excontext, &addrinfo, srv->srv, srv->port, IPPROTO_UDP);
          if (i == 0) {
            host = srv->srv;
            port = srv->port;
            break;
          }

          i = eXosip_dnsutils_rotate_srv (&naptr_record->sipudp_record);
          if (i <= 0) {
            return -1;
          }
          if (i >= n) {
            return -1;
          }
          i = -1;
          /* copy next element */
          n++;
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL,
                                  "Doing UDP failover: ->%s:%i\n", naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index].srv, naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index].port));
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

  /* if SRV was used, destination may be already found */
  if (i != 0) {
    i = _eXosip_get_addrinfo (excontext, &addrinfo, host, port, IPPROTO_UDP);
  }

  if (i != 0) {
    if (naptr_record != NULL) {
      /* rotate on failure! */
      if (MSG_IS_REGISTER (sip)) {
        if (eXosip_dnsutils_rotate_srv (&naptr_record->sipudp_record) > 0) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL,
                                  "Doing UDP failover: %s:%i->%s:%i\n", host, port, naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index].srv, naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index].port));
          _eXosip_mark_registration_expired(excontext, sip->call_id->number);
        }
      }
    }
    return -1;
  }

  memcpy (&addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
  len = (socklen_t)addrinfo->ai_addrlen;

  _eXosip_freeaddrinfo (addrinfo);

  /* default socket used */
  sock = reserved->udp_socket;
  local_port = excontext->eXtl_transport.proto_local_port;
  local_ai_addr = &reserved->ai_addr;

  /* if we have a second socket for outbound connection, re-use the incoming socket (udp_socket) for any message sent there */
  if (reserved->udp_socket_oc > 0)
  {
    int pos;
    
    sock = reserved->udp_socket_oc;
    local_port = excontext->oc_local_port_range[0];
    local_ai_addr = &reserved->ai_addr_oc;
    
    for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
      if (reserved->socket_tab[pos].out_socket == 0)
        continue;
      /* we insert in table ONLY incoming transaction that we want to remember: so any entry in the table refer to incoming udp_socket */
      if (reserved->socket_tab[pos].remote_port == port && osip_strcasecmp(reserved->socket_tab[pos].remote_ip, ipbuf)==0) {
        sock = reserved->socket_tab[pos].out_socket;
        local_port = excontext->eXtl_transport.proto_local_port;
        local_ai_addr = &reserved->ai_addr;
        break;
      }
    }
  }

  switch (((struct sockaddr *) &addr)->sa_family) {
    case AF_INET:
      inet_ntop (((struct sockaddr *) &addr)->sa_family, &(((struct sockaddr_in *) &addr)->sin_addr), ipbuf, sizeof (ipbuf));
      break;
    case AF_INET6:
      inet_ntop (((struct sockaddr *) &addr)->sa_family, &(((struct sockaddr_in6 *) &addr)->sin6_addr), ipbuf, sizeof (ipbuf));
      break;
    default:
      strncpy (ipbuf, "(unknown)", sizeof (ipbuf));
      break;
  }
  
  _eXosip_request_viamanager(excontext, tr, sip, IPPROTO_UDP, local_ai_addr, local_port, sock, ipbuf);
  _eXosip_message_contactmanager(excontext, tr, sip, IPPROTO_UDP, local_ai_addr, local_port, sock, ipbuf);
  _udp_tl_update_contact(excontext, sip);

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


  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Message sent: (to dest=%s:%i)\n%s\n", ipbuf, port, message));
  
  if (excontext->enable_dns_cache == 1 && osip_strcasecmp (host, ipbuf) != 0 && MSG_IS_REQUEST (sip)) {
    if (MSG_IS_REGISTER (sip)) {
      struct eXosip_dns_cache entry;

      memset (&entry, 0, sizeof (struct eXosip_dns_cache));
      snprintf (entry.host, sizeof (entry.host), "%s", host);
      snprintf (entry.ip, sizeof (entry.ip), "%s", ipbuf);
      eXosip_set_option (excontext, EXOSIP_OPT_ADD_DNS_CACHE, (void *) &entry);
    }
  }

  if (tr != NULL) {
    if (tr->ict_context != NULL)
      osip_ict_set_destination (tr->ict_context, osip_strdup (ipbuf), port);
    if (tr->nict_context != NULL)
      osip_nict_set_destination (tr->nict_context, osip_strdup (ipbuf), port);
  }

#ifdef ENABLE_SIP_QOS
  _udp_tl_transport_set_dscp_qos(excontext, (struct sockaddr *) &addr, len);
#endif

#ifdef WIN32
#define CAST_RECV_LEN(L) ((int)(L))
#else
#define CAST_RECV_LEN(L) L
#endif

#ifdef TSC_SUPPORT
  if (excontext->tunnel_handle)
    i = tsc_sendto (reserved->udp_socket, message, CAST_RECV_LEN(length), 0, (struct sockaddr *) &addr, len);
  else
    i = sendto (sock, (const void *) message, CAST_RECV_LEN(length), 0, (struct sockaddr *) &addr, len);
#else
  i = sendto (sock, (const void *) message, CAST_RECV_LEN(length), 0, (struct sockaddr *) &addr, len);
#endif
  if (0 > i) {
    if (naptr_record != NULL) {
      /* rotate on failure! */
      if (MSG_IS_REGISTER (sip)) {
        if (eXosip_dnsutils_rotate_srv (&naptr_record->sipudp_record) > 0) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL,
                                  "Doing UDP failover: %s:%i->%s:%i\n", host, port, naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index].srv, naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index].port));
          _eXosip_mark_registration_expired(excontext, sip->call_id->number);
        }
      }
    }
    /* SIP_NETWORK_ERROR; */
    osip_free (message);
    return -1;
  }

  if (excontext->ka_interval > 0) {
    if (MSG_IS_REGISTER (sip)) {
      eXosip_reg_t *reg = NULL;

      if (_eXosip_reg_find (excontext, &reg, tr) == 0) {
        memcpy (&(reg->addr), &addr, len);
        reg->len = len;
      }
    }
  }

  if (naptr_record != NULL) {
    if (tr != NULL && MSG_IS_REGISTER (sip) && tr->last_response == NULL) {
      /* failover for outgoing transaction */
      time_t now;

      now = osip_getsystemtime (NULL);
      if (tr != NULL && now - tr->birth_time > 10) {
        /* avoid doing this twice... */
        if (eXosip_dnsutils_rotate_srv (&naptr_record->sipudp_record) > 0) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL,
                                  "Doing UDP failover: %s:%i->%s:%i\n", host, port, naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index].srv, naptr_record->sipudp_record.srventry[naptr_record->sipudp_record.index].port));
          osip_free (message);
          _eXosip_mark_registration_expired(excontext, sip->call_id->number);
          return -1;
        }
      }
    }
  }

  osip_free (message);
  return OSIP_SUCCESS;
}

static int
udp_tl_keepalive (struct eXosip_t *excontext)
{
  struct eXtludp *reserved = (struct eXtludp *) excontext->eXtludp_reserved;
  eXosip_reg_t *jr;

  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }

  if (excontext->ka_interval <= 0) {
    return 0;
  }

  if (reserved->udp_socket <= 0)
    return OSIP_UNDEFINED_ERROR;

  for (jr = excontext->j_reg; jr != NULL; jr = jr->next) {
    if (jr->len > 0) {
      if (sendto (reserved->udp_socket, (const void *) excontext->ka_crlf, 4, 0, (struct sockaddr *) &(jr->addr), jr->len) > 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "eXosip: Keep Alive sent on UDP!\n"));
      }
    }
  }
  return OSIP_SUCCESS;
}

static int
udp_tl_set_socket (struct eXosip_t *excontext, int socket)
{
  struct eXtludp *reserved = (struct eXtludp *) excontext->eXtludp_reserved;

  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }

  reserved->udp_socket = socket;

  return OSIP_SUCCESS;
}

static int
udp_tl_masquerade_contact (struct eXosip_t *excontext, const char *public_address, int port)
{
  if (public_address == NULL || public_address[0] == '\0') {
    memset (excontext->udp_firewall_ip, '\0', sizeof (excontext->udp_firewall_ip));
    memset (excontext->udp_firewall_port, '\0', sizeof (excontext->udp_firewall_port));
    return OSIP_SUCCESS;
  }
  snprintf (excontext->udp_firewall_ip, sizeof (excontext->udp_firewall_ip), "%s", public_address);
  if (port > 0) {
    snprintf (excontext->udp_firewall_port, sizeof (excontext->udp_firewall_port), "%i", port);
  }
  return OSIP_SUCCESS;
}

static int
udp_tl_get_masquerade_contact (struct eXosip_t *excontext, char *ip, int ip_size, char *port, int port_size)
{
  struct eXtludp *reserved = (struct eXtludp *) excontext->eXtludp_reserved;

  memset (ip, 0, ip_size);
  memset (port, 0, port_size);

  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }

  if (excontext->udp_firewall_ip[0] != '\0')
    snprintf (ip, ip_size, "%s", excontext->udp_firewall_ip);

  if (excontext->udp_firewall_port[0] != '\0')
    snprintf (port, port_size, "%s", excontext->udp_firewall_port);
  return OSIP_SUCCESS;
}

static struct eXtl_protocol eXtl_udp = {
  1,
  5060,
  "UDP",
  "0.0.0.0",
  IPPROTO_UDP,
  AF_INET,
  0,
  0,
  0,

  &udp_tl_init,
  &udp_tl_free,
  &udp_tl_open,
  &udp_tl_set_fdset,
  &udp_tl_read_message,
  &udp_tl_send_message,
  &udp_tl_keepalive,
  &udp_tl_set_socket,
  &udp_tl_masquerade_contact,
  &udp_tl_get_masquerade_contact,
  &udp_tl_update_contact,
  NULL,
  NULL
};

void
eXosip_transport_udp_init (struct eXosip_t *excontext)
{
  memcpy (&excontext->eXtl_transport, &eXtl_udp, sizeof (struct eXtl_protocol));
}
