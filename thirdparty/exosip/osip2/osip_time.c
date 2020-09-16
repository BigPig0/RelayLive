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

#include <osip2/internal.h>
#include <osip2/osip_time.h>
#include <osipparser2/osip_port.h>

#ifdef __APPLE__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

static struct timeval offset = { 0, 0 };

void
add_gettimeofday (struct timeval *atv, int ms)
{
  int m;

  if (ms >= 1000000) {
    atv->tv_usec = 0;
    m = ms / 1000;
  }
  else {
    atv->tv_usec += ms * 1000;
    m = atv->tv_usec / 1000000;
    atv->tv_usec = atv->tv_usec % 1000000;
  }
  atv->tv_sec += m;
}

void
min_timercmp (struct timeval *tv1, struct timeval *tv2)
{
  if (tv2->tv_sec == -1)
    return;
  if (osip_timercmp (tv1, tv2, >)) {
    /* replace tv1 with tv2 info */
    tv1->tv_sec = tv2->tv_sec;
    tv1->tv_usec = tv2->tv_usec;
  }
}

#if !defined(__PALMOS__) && defined(_WIN32_WCE)

#include <time.h>

int
osip_gettimeofday (struct timeval *tp, void *tz)
{
  DWORD timemillis = GetTickCount ();

  tp->tv_sec = (timemillis / 1000) + offset.tv_sec;
  tp->tv_usec = (timemillis - (tp->tv_sec * 1000)) * 1000;
  return 0;
}

static int
_osip_gettimeofday_realtime (struct timeval *tp, void *tz)
{
  tp->tv_sec = 0;
  tp->tv_usec = 0;
  return 0;
}

time_t
time (time_t * t)
{
  DWORD timemillis = GetTickCount ();

  if (timemillis > 0) {
    if (t != NULL)
      *t = timemillis / 1000;
  }
  return timemillis / 1000;
}

#elif !defined(__PALMOS__) && defined(WIN32)

#include <time.h>
#include <sys/timeb.h>

int
osip_gettimeofday (struct timeval *tp, void *tz)
{
  struct _timeb timebuffer;

  _ftime (&timebuffer);
  tp->tv_sec = (long) timebuffer.time + offset.tv_sec;
  tp->tv_usec = timebuffer.millitm * 1000;
  return 0;
}

static int
_osip_gettimeofday_realtime (struct timeval *tp, void *tz)
{
  FILETIME lSystemTimeAsFileTime;
  LONGLONG ll_now;

  GetSystemTimeAsFileTime (&lSystemTimeAsFileTime);
  ll_now = (LONGLONG) lSystemTimeAsFileTime.dwLowDateTime + ((LONGLONG) (lSystemTimeAsFileTime.dwHighDateTime) << 32LL);
  ll_now = ll_now / 10;         /* in us */
  tp->tv_sec = (long) ll_now / 1000000;
  tp->tv_usec = (long) ll_now % 1000000;
  return 0;
}

#elif defined(__linux) || defined(__linux__) || defined(HAVE_CLOCK_GETTIME_MONOTONIC)

int
osip_gettimeofday (struct timeval *tp, void *tz)
{
  struct timespec ts;

  if (clock_gettime (CLOCK_MONOTONIC, &ts) < 0) {
    gettimeofday (tp, tz);
    return 0;
  }
  tp->tv_sec = ts.tv_sec + offset.tv_sec;
  tp->tv_usec = ts.tv_nsec / 1000;
  return 0;
}

static int
_osip_gettimeofday_realtime (struct timeval *tp, void *tz)
{
  struct timespec ts;

  if (clock_gettime (CLOCK_REALTIME, &ts) < 0) {
    gettimeofday (tp, tz);
    return 0;
  }
  tp->tv_sec = ts.tv_sec;
  tp->tv_usec = ts.tv_nsec / 1000;
  return 0;
}

#elif defined(__APPLE__)

int
osip_gettimeofday (struct timeval *tp, void *tz)
{
  clock_serv_t cclock;
  mach_timespec_t mts;

  host_get_clock_service (mach_host_self (), SYSTEM_CLOCK, &cclock);
  clock_get_time (cclock, &mts);
  mach_port_deallocate (mach_task_self (), cclock);
  tp->tv_sec = mts.tv_sec + offset.tv_sec;
  tp->tv_usec = mts.tv_nsec / 1000;
  return 0;
}

static int
_osip_gettimeofday_realtime (struct timeval *tp, void *tz)
{
  /* TODO */
  tp->tv_sec = 0;
  tp->tv_usec = 0;
  return 0;
}

#else

/* Should never be compiled: missing monotonic clock */
int
osip_gettimeofday (struct timeval *tp, void *tz)
{
  gettimeofday (tp, tz);
  tp->tv_sec += offset.tv_sec;
  return 0;
}

static int
_osip_gettimeofday_realtime (struct timeval *tp, void *tz)
{
  tp->tv_sec = 0;
  tp->tv_usec = 0;
  return 0;
}

#endif

#if defined(__arc__)

time_t
time (time_t * t)
{
  struct timeval now;

  osip_gettimeofday (&now, NULL);

  if (t != NULL) {
    *t = now.tv_sec;
  }
  return now.tv_sec;
}

#endif

void
osip_compensatetime ()
{
  static struct timeval last_now_monotonic = { 0, 0 };
  static struct timeval last_now_real = { 0, 0 };
  struct timeval now_monotonic;
  struct timeval now_real;
  struct timeval diff_monotonic;
  struct timeval diff_real;

#ifndef ANDROID
  return;
#endif

  _osip_gettimeofday_realtime (&now_real, NULL);
  osip_gettimeofday (&now_monotonic, NULL);
  now_monotonic.tv_sec -= offset.tv_sec;

  if (now_real.tv_sec == 0)
    return;                     /* no compensation */

  /* monotonic clock may doesn't include deep sleep time */
  /* the goal is to compensate that time by looking at the real time */

  /* initial call: initialize */
  if (last_now_monotonic.tv_sec == 0) {
    _osip_gettimeofday_realtime (&last_now_real, NULL);
    osip_gettimeofday (&last_now_monotonic, NULL);
    last_now_monotonic.tv_sec -= offset.tv_sec;

    return;
  }

  diff_monotonic.tv_sec = now_monotonic.tv_sec - last_now_monotonic.tv_sec;
  diff_real.tv_sec = now_real.tv_sec - last_now_real.tv_sec;

  if (diff_real.tv_sec < 5)
    return;                     /* skip any "back in time" operation or small interval */
  if (diff_real.tv_sec > 3600)
    return;
  if (diff_real.tv_sec < diff_monotonic.tv_sec + 2)
    return;                     /* only large gap needs to be taken into accounts for this program... */

  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "adjusting exosip monotonic time (%i)!\n", diff_real.tv_sec - diff_monotonic.tv_sec));
  offset.tv_sec += diff_real.tv_sec - diff_monotonic.tv_sec;

  /* reset for later use */
  _osip_gettimeofday_realtime (&last_now_real, NULL);
  osip_gettimeofday (&last_now_monotonic, NULL);
  last_now_monotonic.tv_sec -= offset.tv_sec;
}

time_t
osip_getsystemtime (time_t * t)
{
  struct timeval now_monotonic;

#ifdef ANDROID
  osip_compensatetime ();
#endif

  osip_gettimeofday (&now_monotonic, NULL);
  if (t != NULL) {
    *t = now_monotonic.tv_sec;
  }

  return now_monotonic.tv_sec;
}
