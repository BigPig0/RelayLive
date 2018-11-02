/*
  The oSIP library implements the Session Initiation Protocol (SIP -rfc3261-)
  Copyright (C) 2001-2012 Aymeric MOIZARD amoizard@antisip.com
  
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

#ifndef _INTERNAL_H_
#define _INTERNAL_H_

#if defined (HAVE_CONFIG_H)
#include <osip-config.h>
#endif

#if defined(__PALMOS__) && (__PALMOS__ >= 0x06000000)
#	define HAVE_CTYPE_H 1
#	define HAVE_STRING_H 1
#	define HAVE_SYS_TYPES_H 1
#	define HAVE_TIME_H 1
#	define HAVE_STDARG_H 1

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

/* use win32 crypto routines for random number generation */
/* only use for vs .net (compiler v. 1300) or greater */
#if _MSC_VER >= 1300
#define WIN32_USE_CRYPTO 1
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

#ifdef __BORLANDC__
#define _timeb timeb
#define _ftime ftime
#endif

#ifndef DOXYGEN

#if !( defined(__rtems__) || defined(__PALMOS__) || defined(HAVE_STRUCT_TIMEVAL) )
/* Struct timeval */
struct timeval {
  long tv_sec;                  /* seconds */
  long tv_usec;                 /* and microseconds */
};
#endif

#ifndef OSIP_MONOTHREAD

/* Thread abstraction layer definition */
#if defined(__rtems__)
#include <rtems.h>
#else

/* Is there any thread implementation available? */
/* HAVE_PTHREAD_H is not used any more! I keep it for a while... */
#if !defined(__VXWORKS_OS__) && !defined(__PSOS__) && \
	!defined(WIN32) && !defined(_WIN32_WCE) && !defined(HAVE_PTHREAD_WIN32) && \
    !defined(HAVE_PTHREAD) && !defined(HAVE_PTHREAD_H) && !defined(HAVE_PTH_PTHREAD_H)
#error No thread implementation found!
#endif

/* Pthreads support: */
/* - Unix: native Pthreads. */
/* - Win32: Pthreads for Win32 (http://sources.redhat.com/pthreads-win32). */
#if defined(HAVE_PTHREAD) || defined(HAVE_PTHREAD_H) || defined(HAVE_PTH_PTHREAD_H) || \
	defined(HAVE_PTHREAD_WIN32)
#if defined(__arc__)
#include <ucos_ii_api.h>
#endif


#include <pthread.h>
typedef pthread_t osip_thread_t;
#endif

#if (_MSC_VER >= 1700) && !defined(_USING_V110_SDK71_)
#include <winapifamily.h>
#endif

/* Windows without Pthreads for Win32 */
#if (_MSC_VER >= 1700) && !defined(_USING_V110_SDK71_)
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
#define HAVE_CPP11_THREAD
#elif defined(WINAPI_FAMILY) && WINAPI_FAMILY_ONE_PARTITION( WINAPI_FAMILY, WINAPI_PARTITION_APP )
#define HAVE_CPP11_THREAD
#endif
#endif

#if defined(HAVE_CPP11_THREAD)

typedef struct {
  void *h;
} osip_thread_t;

#elif (defined(WIN32) || defined(_WIN32_WCE)) && !defined(HAVE_PTHREAD_WIN32)

/* Prevent the inclusion of winsock.h */
#define _WINSOCKAPI_
#include <windows.h>
#undef _WINSOCKAPI_

typedef struct {
  HANDLE h;
  unsigned id;
} osip_thread_t;
#endif

#ifdef __VXWORKS_OS__
#include <taskLib.h>
typedef struct {
  int id;
} osip_thread_t;
#endif

#ifdef __PSOS__
#include <psos.h>
typedef struct {
  unsigned long tid;
} osip_thread_t;
#endif


/* Semaphore and Mutex abstraction layer definition */

/* Is there any semaphore implementation available? */
#if !defined(HAVE_SEMAPHORE_H) && !defined(HAVE_SYS_SEM_H) && \
    !defined(WIN32) && !defined(_WIN32_WCE) && !defined(HAVE_PTHREAD_WIN32) && \
    !defined(__PSOS__) && !defined(__VXWORKS_OS__) && !defined(__arc__)
#error No semaphore implementation found
#endif

/* Pthreads */
#if defined(HAVE_PTHREAD) || defined(HAVE_PTHREAD_H) || defined(HAVE_PTH_PTHREAD_H) || \
	defined(HAVE_PTHREAD_WIN32)
typedef pthread_mutex_t osip_mutex_t;
#endif

#ifdef __sun__
#include <semaphore.h>
#undef getdate
#include <synch.h>
#endif

#if defined(__arc__)

typedef struct {
  int _sem_counter;
  struct osip_mutex *_sem_mutex;
} sem_t;

typedef sem_t osip_sem_t;

#elif (defined(HAVE_SEMAPHORE_H) && !defined(__APPLE_CC__)) || defined(HAVE_PTHREAD_WIN32)
#include <semaphore.h>
#ifdef __sun__
#undef getdate
#include <synch.h>
#endif
/**
 * Structure for referencing a semaphore element.
 * @var osip_sem_t
 */
typedef sem_t osip_sem_t;

#elif defined (__APPLE_CC__)
#include <mach/task.h>
#include <mach/semaphore.h>
#include <mach/mach_init.h>
typedef struct {
  semaphore_t semid;
} osip_sem_t;
#elif defined(HAVE_SYS_SEM_H)
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
typedef struct {
  int semid;
} osip_sem_t;
#endif

/* Windows without Pthreads for Win32 */
#if (defined(WIN32) || defined(_WIN32_WCE)) && !defined(HAVE_PTHREAD_WIN32)
/* Prevent the inclusion of winsock.h */
#define _WINSOCKAPI_
#include <windows.h>
#undef _WINSOCKAPI_

#if (_WIN32_WINNT >= 0x0403) && (!defined(_WIN32_WCE))

#define OSIP_CRITICALSECTION_SPIN  4000
typedef struct {
  CRITICAL_SECTION h;
} osip_mutex_t;
#else

typedef struct {
  HANDLE h;
} osip_mutex_t;
#endif

typedef struct {
  HANDLE h;
} osip_sem_t;
#endif

#ifdef __VXWORKS_OS__
#include <semaphore.h>
#include <semLib.h>
typedef struct semaphore osip_mutex_t;
typedef sem_t osip_sem_t;
#endif

#ifdef __PSOS__
#include <Types.h>
#include <os.h>
typedef struct {
  UInt32 id;
} osip_mutex_t;
typedef struct {
  UInt32 id;
} osip_sem_t;
#endif


/* Condition variable abstraction layer definition */

/**
 * Structure for referencing a condition variable element.
 * @var osip_cond_t
 */
#if defined(HAVE_PTHREAD) || defined(HAVE_PTH_PTHREAD_H) || defined(HAVE_PTHREAD_WIN32)
typedef struct osip_cond {
  pthread_cond_t cv;
} osip_cond_t;
#endif

#if (defined(WIN32) || defined(_WIN32_WCE)) && !defined(HAVE_PTHREAD_WIN32)
typedef struct osip_cond {
  struct osip_mutex *mut;
  struct osip_sem *sem;
} osip_cond_t;
#endif

#if defined(__PSOS__) || defined(__VXWORKS_OS__)
typedef struct osip_cond {
  struct osip_sem *sem;
} osip_cond_t;
#endif

#endif

#if defined(__rtems__)
typedef struct {
  rtems_id tid;
} osip_thread_t;

typedef struct {
  rtems_id id;
} osip_sem_t;

typedef struct {
  rtems_id id;
} osip_mutex_t;
#endif


#endif /* #ifndef OSIP_MONOTHREAD */

#endif /* #ifndef DOXYGEN */

#endif /* #ifndef _INTERNAL_H_ */
