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

#if defined(__rtems__)
#include <rtems.h>
#endif

#if !defined(__rtems__)
#if (defined(WIN32) || defined(_WIN32_WCE)) && !defined(HAVE_PTHREAD_WIN32)
#if defined _WIN32_WCE
#define _beginthreadex	CreateThread
#define	_endthreadex	ExitThread
#elif defined WIN32
#include <process.h>
#endif
#endif

/* stack size is only needed on VxWorks. */

#ifndef __VXWORKS_OS__
#if defined(HAVE_PTHREAD) || defined(HAVE_PTH_PTHREAD_H) || defined(HAVE_PTHREAD_WIN32)

struct osip_thread *
osip_thread_create (int stacksize, void *(*func) (void *), void *arg)
{
  int i;
  osip_thread_t *thread = (osip_thread_t *) osip_malloc (sizeof (osip_thread_t));

  if (thread == NULL)
    return NULL;

  i = pthread_create (thread, NULL, func, (void *) arg);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Error while creating a new thread\n"));
    osip_free (thread);
    return NULL;
  }
  return (struct osip_thread *) thread;
}

int
osip_thread_set_priority (struct osip_thread *thread, int priority)
{
  return OSIP_SUCCESS;
}

int
osip_thread_join (struct osip_thread *_thread)
{
  osip_thread_t *thread = (osip_thread_t *) _thread;

  if (thread == NULL)
    return OSIP_BADPARAMETER;
  return pthread_join (*thread, NULL);
}

void
osip_thread_exit ()
{
#if !defined(__arc__)
  pthread_exit (NULL);
#endif
}

#endif
#endif


#if (defined(WIN32) || defined(_WIN32_WCE)) && !defined(HAVE_PTHREAD_WIN32)

#if defined(HAVE_CPP11_THREAD)
#include <iostream>
#include <thread>

struct osip_thread *
osip_thread_create (int stacksize, void *(*func) (void *), void *arg)
{
  osip_thread_t *thread = (osip_thread_t *) osip_malloc (sizeof (osip_thread_t));

  if (thread == NULL)
    return NULL;
  thread->h = new std::thread (func, arg);
  if (thread->h == 0) {
    osip_free (thread);
    return NULL;
  }
  return (struct osip_thread *) thread;
}

int
osip_thread_join (struct osip_thread *_thread)
{
  osip_thread_t *thread = (osip_thread_t *) _thread;

  std::thread * th;
  if (thread == NULL)
    return OSIP_BADPARAMETER;
  th = (std::thread *) thread->h;
  th->join ();
  delete th;

  thread->h = NULL;
  return (0);
}

void
osip_thread_exit ()
{
}

int
osip_thread_set_priority (struct osip_thread *thread, int priority)
{
  return OSIP_SUCCESS;
}

#else

struct osip_thread *
osip_thread_create (int stacksize, void *(*func) (void *), void *arg)
{
  osip_thread_t *thread = (osip_thread_t *) osip_malloc (sizeof (osip_thread_t));

  if (thread == NULL)
    return NULL;
  thread->h = (HANDLE) _beginthreadex (NULL,    /* default security attr */
                                       0,       /* use default one */
                                       (unsigned (__stdcall *) (void *)) func, arg, 0, &(thread->id));
  if (thread->h == 0) {
    osip_free (thread);
    return NULL;
  }
  return (struct osip_thread *) thread;
}

int
osip_thread_join (struct osip_thread *_thread)
{
  int i;
  osip_thread_t *thread = (osip_thread_t *) _thread;

  if (thread == NULL)
    return OSIP_BADPARAMETER;
  i = WaitForSingleObject (thread->h, INFINITE);
  if (i == WAIT_OBJECT_0) {
    /* fprintf (stdout, "thread joined!\n"); */
  }
  else {
    /* fprintf (stdout, "ERROR!! thread joined ERROR!!\n"); */
    return OSIP_UNDEFINED_ERROR;
  }
  CloseHandle (thread->h);
  return (0);
}

void
osip_thread_exit ()
{
  /* ExitThread(0); */
  _endthreadex (0);
}

int
osip_thread_set_priority (struct osip_thread *thread, int priority)
{
  return OSIP_SUCCESS;
}

#endif

#endif

#ifndef __VXWORKS_OS__
#ifdef __PSOS__
struct osip_thread *
osip_thread_create (int stacksize, void *(*func) (void *), void *arg)
{
  osip_thread_t *thread = (osip_thread_t *) osip_malloc (sizeof (osip_thread_t));

  if (thread == NULL)
    return (NULL);
  if (t_create ("sip", 150, stacksize, 0, 0, &thread->tid) != 0) {
    osip_free (thread);
    return (NULL);
  }

  if (t_start (thread->tid, T_PREEMPT | T_ISR, func, 0) != 0) {
    osip_free (thread);
    return (NULL);
  }

  return (struct osip_thread *) thread;
}

int
osip_thread_set_priority (struct osip_thread *_thread, int priority)
{
  unsigned long oldprio;
  osip_thread_t *thread = (osip_thread_t *) _thread;

  if (thread == NULL)
    return OSIP_BADPARAMETER;
  t_set_pri (thread->tid, priority, &oldprio);
  return OSIP_SUCCESS;
}

int
osip_thread_join (struct osip_thread *_thread)
{
  osip_thread_t *thread = (osip_thread_t *) _thread;

  if (thread == NULL)
    return OSIP_BADPARAMETER;
  t_delete (thread->tid);

  return (0);
}

void
osip_thread_exit ()
{
  t_delete (0);
}
#endif
#endif

#ifdef __VXWORKS_OS__
struct osip_thread *
osip_thread_create (int stacksize, void *(*func) (void *), void *arg)
{
  osip_thread_t *thread = (osip_thread_t *) osip_malloc (sizeof (osip_thread_t));

  if (thread == NULL)
    return NULL;
  thread->id = taskSpawn (NULL, 5, 0, stacksize, (FUNCPTR) func, (int) arg, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  if (thread->id < 0)
    osip_free (thread);
  return (struct osip_thread *) thread;
}

int
osip_thread_set_priority (struct osip_thread *_thread, int priority)
{
  osip_thread_t *thread = (osip_thread_t *) _thread;

  if (thread == NULL)
    return OSIP_BADPARAMETER;
  taskPrioritySet (thread->id, 1);
  return OSIP_SUCCESS;
}

int
osip_thread_join (struct osip_thread *_thread)
{
  osip_thread_t *thread = (osip_thread_t *) _thread;

  if (thread == NULL)
    return OSIP_BADPARAMETER;
  return taskDelete (thread->id);
}

void
osip_thread_exit ()
{
  /*?? */
}

#endif
#endif /* #ifndef __rtems__ */

#if defined(__rtems__)
struct osip_thread *
osip_thread_create (int stacksize, void *(*func) (void *), void *arg)
{
  rtems_status_code status;
  osip_thread_t *thread = (osip_thread_t *) osip_malloc (sizeof (osip_thread_t));

  if (thread == NULL)
    return NULL;

  status = rtems_task_create (rtems_build_name ('S', 'I', 'P', 'T'), 100, stacksize, RTEMS_DEFAULT_MODES, RTEMS_DEFAULT_ATTRIBUTES, &thread->tid);

  if (status == RTEMS_SUCCESSFUL) {
    status = rtems_task_start (thread->tid, (rtems_task_entry) func, (rtems_task_argument) arg);
  }

  if (status != RTEMS_SUCCESSFUL) {
    osip_free (thread);
    thread = NULL;
  }

  return (struct osip_thread *) thread;
}

int
osip_thread_set_priority (struct osip_thread *_thread, int priority)
{
  osip_thread_t *thread = (osip_thread_t *) _thread;

  rtems_task_priority old;

  if (thread != NULL) {
    return rtems_task_set_priority (thread->tid, priority, &old);
  }
  return OSIP_SUCCESS;
}


int
osip_thread_join (struct osip_thread *_thread)
{
  osip_thread_t *thread = (osip_thread_t *) _thread;

  if (thread == NULL) {
    return OSIP_BADPARAMETER;
  }
  return rtems_task_delete (thread->tid);
}

void
osip_thread_exit ()
{
  rtems_task_delete (RTEMS_SELF);
}


#endif

#endif /* #ifndef OSIP_MONOTHREAD */
