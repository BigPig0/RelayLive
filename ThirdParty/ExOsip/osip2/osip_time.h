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

#ifndef _OSIP_TIME_H_
#define _OSIP_TIME_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Common time-related functions and data types */

#if defined(_WIN32_WCE)
  struct _timeb {
    time_t time;
    unsigned short millitm;
    short timezone;
    short dstflag;
  };

#endif

/* struct timeval, as defined in <sys/time.h>, <winsock.h> or <winsock2.h> */
  struct timeval;

/* Time manipulation functions */
  void add_gettimeofday (struct timeval *atv, int ms);
  void min_timercmp (struct timeval *tv1, struct timeval *tv2);

/* Operations on struct timeval */
#if !defined(timerisset)
#define osip_timerisset(tvp)         ((tvp)->tv_sec || (tvp)->tv_usec)
#else
#define osip_timerisset(tvp)            timerisset(tvp)
#endif

#if !defined(timercmp)
#define osip_timercmp(a, b, CMP)                          \
  (((a)->tv_sec == (b)->tv_sec) ?                          \
   ((a)->tv_usec CMP (b)->tv_usec) :                       \
   ((a)->tv_sec CMP (b)->tv_sec))
#else
#define osip_timercmp(tvp, uvp, cmp)    timercmp(tvp,uvp,cmp)
#endif

#if !defined(timerclear)
#define osip_timerclear(tvp)         (tvp)->tv_sec = (tvp)->tv_usec = 0
#else
#define osip_timerclear(tvp)            timerclear(tvp)
#endif

  int osip_gettimeofday (struct timeval *tp, void *tz);
  time_t osip_getsystemtime (time_t * t);
  void osip_compensatetime (void);

#ifdef __cplusplus
}
#endif
#endif
