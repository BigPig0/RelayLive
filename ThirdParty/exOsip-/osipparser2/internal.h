/*
  The oSIP library implements the Session Initiation Protocol (SIP -rfc3261-)
  Copyright (C) 2001-2015 Aymeric MOIZARD amoizard@antisip.com
  
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

#ifndef _INTERNALOSIPPARSER_H_
#define _INTERNALOSIPPARSER_H_

#if defined (HAVE_CONFIG_H)
#include <osip-config.h>
#endif

#if defined(__PALMOS__) && (__PALMOS__ >= 0x06000000)
#define HAVE_CTYPE_H 1
#define HAVE_STRING_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_TIME_H 1
#define HAVE_STDARG_H 1

#elif defined(__VXWORKS_OS__) || defined(__rtems__)
#define HAVE_STRING_H 1
#define HAVE_TIME_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_STDARG_H 1

#elif defined _WIN32_WCE

#define HAVE_CTYPE_H 1
#define HAVE_STRING_H 1
#define HAVE_TIME_H 1
#define HAVE_STDARG_H 1

#define snprintf  _snprintf

#elif defined(WIN32)

#define HAVE_CTYPE_H 1
#define HAVE_STRING_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_TIME_H 1
#define HAVE_STDARG_H 1

#define snprintf _snprintf

#if (_MSC_VER >= 1700) && !defined(_USING_V110_SDK71_)
#include <winapifamily.h>
#endif

/* use win32 crypto routines for random number generation */
/* only use for vs .net (compiler v. 1300) or greater */
#if _MSC_VER >= 1300
#define WIN32_USE_CRYPTO 1
#if (_MSC_VER >= 1700) && !defined(_USING_V110_SDK71_)
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
#undef WIN32_USE_CRYPTO
#elif defined(WINAPI_FAMILY) && WINAPI_FAMILY_ONE_PARTITION( WINAPI_FAMILY, WINAPI_PARTITION_APP )
#undef WIN32_USE_CRYPTO
#endif
#endif
#endif

#endif

#if defined (HAVE_STRING_H)
#include <string.h>
#elif defined (HAVE_STRINGS_H)
#include <strings.h>
#else
#include <string.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#if defined (HAVE_SYS_TYPES_H)
#include <sys/types.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#if defined (HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif

#if defined(__arc__)
#include "includes_api.h"
#include "os_cfg_pub.h"
#include <posix_time_pub.h>
#endif

#ifdef __PSOS__
#define VA_START(a, f)  va_start(a, f)
#include "pna.h"
#include "stdlib.h"
#include "time.h"
#define timercmp(tvp, uvp, cmp) \
((tvp)->tv_sec cmp (uvp)->tv_sec || \
(tvp)->tv_sec == (uvp)->tv_sec && (tvp)->tv_usec cmp (uvp)->tv_usec)
#define snprintf  osip_snprintf
#ifndef INT_MAX
#define INT_MAX 0x7FFFFFFF
#endif
#endif

#endif /* #ifndef _INTERNAL_H_ */
