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

#ifndef OSIP_MONOTHREAD

#include <osipparser2/osip_port.h>
#include <osip2/osip_mt.h>
#include <osip2/osip_condv.h>


#if !defined(__rtems__) && !defined(__VXWORKS_OS__) && !defined(__PSOS__) && \
	!defined(WIN32) && !defined(_WIN32_WCE) && !defined(HAVE_PTHREAD_WIN32) && \
    !defined(HAVE_PTHREAD) && !defined(HAVE_PTH_PTHREAD_H)
#error No thread implementation found!
#endif

#if defined(HAVE_PTHREAD) || defined(HAVE_PTH_PTHREAD_H) || defined(HAVE_PTHREAD_WIN32)
/*
  #include <sys/types.h>
  #include <sys/timeb.h>
*/
#include <pthread.h>

struct osip_cond *
osip_cond_init ()
{
  osip_cond_t *cond = (osip_cond_t *) osip_malloc (sizeof (osip_cond_t));

  if (cond && (pthread_cond_init (&cond->cv, NULL) == 0)) {
    return (struct osip_cond *) (cond);
  }
  osip_free (cond);

  return NULL;
}

int
osip_cond_destroy (struct osip_cond *_cond)
{
  int ret;

  if (!_cond)
    return OSIP_BADPARAMETER;
  ret = pthread_cond_destroy (&_cond->cv);
  osip_free (_cond);
  return ret;
}

int
osip_cond_signal (struct osip_cond *_cond)
{
  if (!_cond)
    return OSIP_BADPARAMETER;
  return pthread_cond_signal (&_cond->cv);
}


int
osip_cond_wait (struct osip_cond *_cond, struct osip_mutex *_mut)
{
  if (!_cond)
    return OSIP_BADPARAMETER;
  return pthread_cond_wait (&_cond->cv, (pthread_mutex_t *) _mut);
}


int
osip_cond_timedwait (struct osip_cond *_cond, struct osip_mutex *_mut, const struct timespec *abstime)
{
  if (!_cond)
    return OSIP_BADPARAMETER;
  return pthread_cond_timedwait (&_cond->cv, (pthread_mutex_t *) _mut, (const struct timespec *) abstime);
}

#endif


#if defined(_WIN32_WCE)

#endif

#if defined(WIN32) && !defined(_WIN32_WCE) && !defined(HAVE_PTHREAD_WIN32)

#include <sys/types.h>
#include <sys/timeb.h>

struct osip_cond *
osip_cond_init ()
{
  osip_cond_t *cond = (osip_cond_t *) osip_malloc (sizeof (osip_cond_t));

  if (cond && (cond->mut = osip_mutex_init ()) != NULL) {
    cond->sem = osip_sem_init (0);      /* initially locked */
    return (struct osip_cond *) (cond);
  }
  osip_free (cond);

  return NULL;
}

int
osip_cond_destroy (struct osip_cond *_cond)
{
  if (!_cond)
    return OSIP_SUCCESS;
  if (_cond->sem == NULL)
    return OSIP_SUCCESS;

  osip_sem_destroy (_cond->sem);

  if (_cond->mut == NULL)
    return OSIP_SUCCESS;

  osip_mutex_destroy (_cond->mut);
  osip_free (_cond);
  return (0);
}

int
osip_cond_signal (struct osip_cond *_cond)
{
  if (!_cond)
    return OSIP_BADPARAMETER;
  return osip_sem_post (_cond->sem);
}


int
osip_cond_wait (struct osip_cond *_cond, struct osip_mutex *_mut)
{
  int ret1 = 0, ret2 = 0, ret3 = 0;
  int i;

  if (!_cond)
    return OSIP_BADPARAMETER;

  i = osip_mutex_lock (_cond->mut);
  if (i != 0)
    return i;

  i = osip_mutex_unlock (_mut);
  if (i != 0)
    return i;

  ret1 = osip_sem_wait (_cond->sem);

  ret2 = osip_mutex_lock (_mut);

  ret3 = osip_mutex_unlock (_cond->mut);

  if (ret1)
    return ret1;
  if (ret2)
    return ret2;
  if (ret3)
    return ret3;
  return OSIP_SUCCESS;
}

#define OSIP_CLOCK_REALTIME 4002

int
__osip_clock_gettime (unsigned int clock_id, struct timespec *tp)
{
  struct _timeb time_val;

  if (clock_id != OSIP_CLOCK_REALTIME)
    return OSIP_BADPARAMETER;

  if (tp == NULL)
    return OSIP_BADPARAMETER;

  _ftime (&time_val);
  tp->tv_sec = (long) time_val.time;
  tp->tv_nsec = time_val.millitm * 1000000;
  return OSIP_SUCCESS;
}

static int
_delta_time (const struct timespec *start, const struct timespec *end)
{
  int difx;

  if (start == NULL || end == NULL)
    return OSIP_SUCCESS;

  difx = ((end->tv_sec - start->tv_sec) * 1000) + ((end->tv_nsec - start->tv_nsec) / 1000000);

  return difx;
}


int
osip_cond_timedwait (struct osip_cond *_cond, struct osip_mutex *_mut, const struct timespec *abstime)
{
  DWORD dwRet;
  struct timespec now;
  int timeout_ms;
  HANDLE sem;
  int i;

  if (!_cond)
    return OSIP_BADPARAMETER;

  sem = *((HANDLE *) _cond->sem);

  if (sem == NULL)
    return OSIP_UNDEFINED_ERROR;

  if (abstime == NULL)
    return OSIP_BADPARAMETER;

  __osip_clock_gettime (OSIP_CLOCK_REALTIME, &now);

  timeout_ms = _delta_time (&now, abstime);
  if (timeout_ms <= 0)
    return 1;                   /* ETIMEDOUT; */

  i = osip_mutex_unlock (_mut);
  if (i != 0)
    return i;

  dwRet = WaitForSingleObjectEx (sem, timeout_ms, FALSE);

  i = osip_mutex_lock (_mut);
  if (i != 0)
    return i;

  switch (dwRet) {
  case WAIT_OBJECT_0:
    return OSIP_SUCCESS;
    break;
  case WAIT_TIMEOUT:
    return 1;                   /* ETIMEDOUT; */
    break;
  default:
    return OSIP_UNDEFINED_ERROR;
    break;
  }
}
#endif

#ifdef __PSOS__
 /*TODO*/
#endif
/* use VxWorks implementation */
#ifdef __VXWORKS_OS__
struct osip_cond *
osip_cond_init ()
{
  osip_cond_t *cond = (osip_cond_t *) osip_malloc (sizeof (osip_cond_t));

  if ((cond->sem = osip_sem_init (0)) != NULL) {
    return (struct osip_cond *) (cond);
  }
  osip_free (cond);

  return NULL;
}

int
osip_cond_destroy (struct osip_cond *_cond)
{
  if (_cond->sem == NULL)
    return OSIP_SUCCESS;

  osip_sem_destroy (_cond->sem);
  osip_free (_cond);
  return (0);
}

int
osip_cond_signal (struct osip_cond *_cond)
{
  return osip_sem_post (_cond->sem);
}

+static int
_cond_wait (struct osip_cond *_cond, struct osip_mutex *_mut, int ticks)
{
  int ret;

  if (_cond == NULL)
    return OSIP_BADPARAMETER;

  ret = osip_mutex_unlock (_mut);
  if (ret != 0) {
    return ret;
  }

  ret = semTake (((osip_sem_t *) _cond->sem)->semId, ticks);
  if (ret != OK) {
    switch (errno) {
    case S_objLib_OBJ_ID_ERROR:
      /* fall through */
    case S_objLib_OBJ_UNAVAILABLE:
      /* fall through */
#if 0
    case S_intLib_NOT_ISR_CALLABLE:
#endif
      ret = OSIP_UNDEFINED_ERROR;
      break;
    case S_objLib_OBJ_TIMEOUT:
      ret = 1;
      break;
    default:                   /* vxworks has bugs */
      ret = 1;
      break;
    }
  }

  i = osip_mutex_lock (_mut);
  if (i != 0) {
    ret = i;
  }
  return ret;
}

int
osip_cond_wait (struct osip_cond *_cond, struct osip_mutex *_mut)
{
  return _cond_wait (_cond, _mut, WAIT_FOREVER);
}

int
osip_cond_timedwait (struct osip_cond *_cond, struct osip_mutex *_mut, const struct timespec *abstime)
{
  int rate = sysClkRateGet ();
  struct timespec now;
  long sec, nsec;
  int ticks;
  SEM_ID sem;

  if (_cond == NULL)
    return OSIP_BADPARAMETER;

  sem = ((osip_sem_t *) _cond->sem)->semId;

  if (sem == NULL)
    return OSIP_UNDEFINED_ERROR;

  if (abstime == NULL)
    return OSIP_BADPARAMETER;
  clock_gettime (CLOCK_REALTIME, &now);

  sec = abstime->tv_sec - now.tv_sec;
  nsec = abstime->tv_nsec - now.tv_nsec;

  while ((sec > 0) && (nsec < 0)) {
    --sec;
    nsec += 1000000000;
  }
  if (nsec < 0)
    return 1;                   /*ETIMEDOUT; */
  ticks = (sec * rate) + (nsec / 1000 * rate / 1000000);

  return _cond_wait (_cond, _mut, ticks);
}

#endif

#endif /* #ifndef OSIP_MONOTHREAD */
