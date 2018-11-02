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

#if !defined(__VXWORKS_OS__) && !defined(__PSOS__) && \
	!defined(WIN32) && !defined(_WIN32_WCE) && !defined(HAVE_PTHREAD_WIN32) && \
    !defined(HAVE_PTHREAD) && !defined(HAVE_PTH_PTHREAD_H) && !defined(__rtems__)
#error No thread implementation found!
#endif

#if defined(HAVE_PTHREAD) || defined(HAVE_PTH_PTHREAD_H) || defined(HAVE_PTHREAD_WIN32)

struct osip_mutex *
osip_mutex_init ()
{
  osip_mutex_t *mut = (osip_mutex_t *) osip_malloc (sizeof (osip_mutex_t));

  if (mut == NULL)
    return NULL;
  pthread_mutex_init (mut, NULL);
  return (struct osip_mutex *) mut;
}

void
osip_mutex_destroy (struct osip_mutex *_mut)
{
  osip_mutex_t *mut = (osip_mutex_t *) _mut;

  if (mut == NULL)
    return;
  pthread_mutex_destroy (mut);
  osip_free (mut);
}

int
osip_mutex_lock (struct osip_mutex *_mut)
{
  osip_mutex_t *mut = (osip_mutex_t *) _mut;

  if (mut == NULL)
    return OSIP_BADPARAMETER;
  return pthread_mutex_lock (mut);
}

int
osip_mutex_unlock (struct osip_mutex *_mut)
{
  osip_mutex_t *mut = (osip_mutex_t *) _mut;

  if (mut == NULL)
    return OSIP_BADPARAMETER;
  return pthread_mutex_unlock (mut);
}

#endif

#if defined(__arc__)

/* Counting Semaphore is initialized to value */
struct osip_sem *
osip_sem_init (unsigned int value)
{
  osip_sem_t *sem = (osip_sem_t *) osip_malloc (sizeof (osip_sem_t));

  if (sem == NULL)
    return NULL;

  sem->_sem_counter = 0;
  sem->_sem_mutex = osip_mutex_init ();
  if (sem->_sem_mutex != NULL)
    return (struct osip_sem *) sem;
  osip_free (sem);
  return NULL;
}

int
osip_sem_destroy (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_SUCCESS;
  osip_mutex_destroy (sem->_sem_mutex);
  osip_free (sem);
  return OSIP_SUCCESS;
}

int
osip_sem_post (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  osip_mutex_lock (sem->_sem_mutex);
  sem->_sem_counter++;
  osip_mutex_unlock (sem->_sem_mutex);
  return OSIP_SUCCESS;
}

int
osip_sem_wait (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;

  /* poor emulation... */
  while (1) {
    osip_mutex_lock (sem->_sem_mutex);
    if (sem->_sem_counter > 0) {
      sem->_sem_counter--;
      osip_mutex_unlock (sem->_sem_mutex);
      return OSIP_SUCCESS;
    }
    osip_mutex_unlock (sem->_sem_mutex);
    osip_usleep (1000);
  }
  return OSIP_UNDEFINED_ERROR;
}

int
osip_sem_trywait (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  osip_mutex_lock (sem->_sem_mutex);
  if (sem->_sem_counter > 0) {
    sem->_sem_counter--;
    osip_mutex_unlock (sem->_sem_mutex);
    return OSIP_SUCCESS;
  }
  osip_mutex_unlock (sem->_sem_mutex);
  return OSIP_UNDEFINED_ERROR;
}

#elif (defined(HAVE_SEMAPHORE_H) && !defined(__APPLE_CC__)) || defined(HAVE_PTHREAD_WIN32)

/* Counting Semaphore is initialized to value */
struct osip_sem *
osip_sem_init (unsigned int value)
{
  osip_sem_t *sem = (osip_sem_t *) osip_malloc (sizeof (osip_sem_t));

  if (sem == NULL)
    return NULL;

  if (sem_init (sem, 0, value) == 0)
    return (struct osip_sem *) sem;
  osip_free (sem);
  return NULL;
}

int
osip_sem_destroy (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_SUCCESS;
  sem_destroy (sem);
  osip_free (sem);
  return OSIP_SUCCESS;
}

int
osip_sem_post (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  return sem_post (sem);
}

int
osip_sem_wait (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  return sem_wait (sem);
}

int
osip_sem_trywait (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  return sem_trywait (sem);
}

#elif defined (__APPLE_CC__)
struct osip_sem *
osip_sem_init (unsigned int value)
{
  task_t task = mach_task_self ();
  int policy = SYNC_POLICY_FIFO;
  osip_sem_t *sem = (osip_sem_t *) osip_malloc (sizeof (osip_sem_t));

  if (sem == NULL)
    return NULL;

  if (semaphore_create (task, &sem->semid, policy, value) == KERN_SUCCESS)
    return (struct osip_sem *) sem;

  osip_free (sem);
  return NULL;
}

int
osip_sem_destroy (struct osip_sem *_sem)
{
  task_t task = mach_task_self ();
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_SUCCESS;
  if (semaphore_destroy (task, sem->semid) == KERN_SUCCESS) {
    osip_free (sem);
    return OSIP_SUCCESS;
  }

  osip_free (sem);
  return OSIP_SUCCESS;
}

int
osip_sem_post (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  if (semaphore_signal (sem->semid) == KERN_SUCCESS)
    return OSIP_SUCCESS;

  return OSIP_UNDEFINED_ERROR;
}

int
osip_sem_wait (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  if (semaphore_wait (sem->semid) == KERN_SUCCESS)
    return OSIP_SUCCESS;

  return OSIP_UNDEFINED_ERROR;
}

int
osip_sem_trywait (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  mach_timespec_t wait_time = { 0, 0 };
  if (semaphore_timedwait (sem->semid, wait_time) == KERN_SUCCESS)
    return OSIP_SUCCESS;

  return OSIP_UNDEFINED_ERROR;
}

#elif defined (HAVE_SYS_SEM_H)
/* support for semctl, semop, semget */

#define SEM_PERM 0600

struct osip_sem *
osip_sem_init (unsigned int value)
{
  union semun val;
  int i;
  osip_sem_t *sem = (osip_sem_t *) osip_malloc (sizeof (osip_sem_t));

  if (sem == NULL)
    return NULL;

  sem->semid = semget (IPC_PRIVATE, 1, IPC_CREAT | SEM_PERM);
  if (sem->semid == -1) {
    perror ("semget error");
    osip_free (sem);
    return NULL;
  }
  val.val = (int) value;
  i = semctl (sem->semid, 0, SETVAL, val);
  if (i != 0) {
    perror ("semctl error");
    osip_free (sem);
    return NULL;
  }
  return (struct osip_sem *) sem;
}

int
osip_sem_destroy (struct osip_sem *_sem)
{
  union semun val;
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_SUCCESS;
  val.val = 0;
  semctl (sem->semid, 0, IPC_RMID, val);
  osip_free (sem);
  return OSIP_SUCCESS;
}

int
osip_sem_post (struct osip_sem *_sem)
{
  struct sembuf sb;
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  sb.sem_num = 0;
  sb.sem_op = 1;
  sb.sem_flg = 0;
  return semop (sem->semid, &sb, 1);
}

int
osip_sem_wait (struct osip_sem *_sem)
{
  struct sembuf sb;
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  sb.sem_num = 0;
  sb.sem_op = -1;
  sb.sem_flg = 0;
  return semop (sem->semid, &sb, 1);
}

int
osip_sem_trywait (struct osip_sem *_sem)
{
  struct sembuf sb;
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  sb.sem_num = 0;
  sb.sem_op = -1;
  sb.sem_flg = IPC_NOWAIT;
  return semop (sem->semid, &sb, 1);
}

#endif


/* use VxWorks implementation */
#ifdef __VXWORKS_OS__

struct osip_mutex *
osip_mutex_init ()
{
  return (struct osip_mutex *) semMCreate (SEM_Q_FIFO | SEM_DELETE_SAFE);
}

void
osip_mutex_destroy (struct osip_mutex *_mut)
{
  osip_mutex_t *mut = (osip_mutex_t *) _mut;

  if (mut == NULL)
    return;
  semDelete (mut);
}

int
osip_mutex_lock (struct osip_mutex *_mut)
{
  osip_mutex_t *mut = (osip_mutex_t *) _mut;

  if (mut == NULL)
    return OSIP_BADPARAMETER;
  return semTake (mut, WAIT_FOREVER);
}

int
osip_mutex_unlock (struct osip_mutex *_mut)
{
  osip_mutex_t *mut = (osip_mutex_t *) _mut;

  if (mut == NULL)
    return OSIP_BADPARAMETER;
  return semGive (mut);
}

struct osip_sem *
osip_sem_init (unsigned int value)
{
  SEM_ID initsem;
  osip_sem_t *x;

  x = (osip_sem_t *) osip_malloc (sizeof (osip_sem_t));
  if (x == NULL)
    return NULL;
  initsem = semCCreate (SEM_Q_FIFO, value);
  x->semId = initsem;
  x->refCnt = value;
  x->sem_name = NULL;
  return (struct osip_sem *) x;
}

int
osip_sem_destroy (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_SUCCESS;
  semDelete (sem->semId);
  osip_free (sem);
  return OSIP_SUCCESS;
}

int
osip_sem_post (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  return semGive (sem->semId);
}

int
osip_sem_wait (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  return semTake (sem->semId, WAIT_FOREVER);
}

int
osip_sem_trywait (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  return semTake (sem->semId, NO_WAIT);
}

#endif


#if (defined(WIN32) || defined(_WIN32_WCE)) && !defined(HAVE_PTHREAD_WIN32)
#include <limits.h>

#if (_WIN32_WINNT >= 0x0403) && !defined(_WIN32_WCE)
struct osip_mutex *
osip_mutex_init ()
{
  osip_mutex_t *mut = (osip_mutex_t *) osip_malloc (sizeof (osip_mutex_t));

  if (mut == NULL)
    return NULL;
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
  if (InitializeCriticalSectionEx (&mut->h, OSIP_CRITICALSECTION_SPIN, CRITICAL_SECTION_NO_DEBUG_INFO) != 0) {
    return (struct osip_mutex *) (mut);
  }
#else
  if (InitializeCriticalSectionAndSpinCount (&mut->h, OSIP_CRITICALSECTION_SPIN) != 0)
    return (struct osip_mutex *) (mut);
#endif
  osip_free (mut);
  return (NULL);
}

void
osip_mutex_destroy (struct osip_mutex *_mut)
{
  osip_mutex_t *mut = (osip_mutex_t *) _mut;

  if (mut == NULL)
    return;
  DeleteCriticalSection (&mut->h);
  osip_free (mut);
}

int
osip_mutex_lock (struct osip_mutex *_mut)
{
  osip_mutex_t *mut = (osip_mutex_t *) _mut;

  if (mut == NULL)
    return OSIP_BADPARAMETER;
  EnterCriticalSection (&mut->h);

  return (0);
}

int
osip_mutex_unlock (struct osip_mutex *_mut)
{
  osip_mutex_t *mut = (osip_mutex_t *) _mut;

  if (mut == NULL)
    return OSIP_BADPARAMETER;
  LeaveCriticalSection (&mut->h);
  return (0);
}
#else

struct osip_mutex *
osip_mutex_init ()
{
  osip_mutex_t *mut = (osip_mutex_t *) osip_malloc (sizeof (osip_mutex_t));

  if (mut == NULL)
    return NULL;
  if ((mut->h = CreateMutex (NULL, FALSE, NULL)) != NULL)
    return (struct osip_mutex *) (mut);
  osip_free (mut);
  return (NULL);
}

void
osip_mutex_destroy (struct osip_mutex *_mut)
{
  osip_mutex_t *mut = (osip_mutex_t *) _mut;

  if (mut == NULL)
    return;
  CloseHandle (mut->h);
  osip_free (mut);
}

int
osip_mutex_lock (struct osip_mutex *_mut)
{
  DWORD err;
  osip_mutex_t *mut = (osip_mutex_t *) _mut;

  if (mut == NULL)
    return OSIP_BADPARAMETER;
  if ((err = WaitForSingleObject (mut->h, INFINITE)) == WAIT_OBJECT_0)
    return (0);
  return (-1);
}

int
osip_mutex_unlock (struct osip_mutex *_mut)
{
  osip_mutex_t *mut = (osip_mutex_t *) _mut;

  if (mut == NULL)
    return OSIP_BADPARAMETER;
  ReleaseMutex (mut->h);
  return (0);
}
#endif

struct osip_sem *
osip_sem_init (unsigned int value)
{
  osip_sem_t *sem = (osip_sem_t *) osip_malloc (sizeof (osip_sem_t));

  if (sem == NULL)
    return NULL;

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
  if ((sem->h = CreateSemaphoreExW (NULL, value, LONG_MAX, NULL, 0, (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | SEMAPHORE_MODIFY_STATE))) != NULL)
    return (struct osip_sem *) (sem);
#else
  if ((sem->h = CreateSemaphore (NULL, value, LONG_MAX, NULL)) != NULL)
    return (struct osip_sem *) (sem);
#endif
  osip_free (sem);
  return (NULL);
}

int
osip_sem_destroy (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_SUCCESS;
  CloseHandle (sem->h);
  osip_free (sem);
  return (0);
}

int
osip_sem_post (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  ReleaseSemaphore (sem->h, 1, NULL);
  return (0);
}

int
osip_sem_wait (struct osip_sem *_sem)
{
  DWORD err;
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  if ((err = WaitForSingleObjectEx (sem->h, INFINITE, FALSE)) == WAIT_OBJECT_0)
    return (0);
  if (err == WAIT_TIMEOUT)
    return (-1);
  return (-1);
}

int
osip_sem_trywait (struct osip_sem *_sem)
{
  DWORD err;
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  if ((err = WaitForSingleObjectEx (sem->h, 0, FALSE)) == WAIT_OBJECT_0)
    return (0);
  return (-1);
}
#endif

#ifdef __PSOS__
struct osip_mutex *
osip_mutex_init ()
{
  osip_mutex_t *mut = (osip_mutex_t *) osip_malloc (sizeof (osip_mutex_t));

  if (sm_create ("mut", 1, 0, &mut->id) == 0)
    return (struct osip_mutex *) (mut);
  osip_free (mut);
  return (NULL);
}

void
osip_mutex_destroy (struct osip_mutex *_mut)
{
  osip_mutex_t *mut = (osip_mutex_t *) _mut;

  if (mut) {
    sm_delete (mut->id);
    osip_free (mut);
  }
}

int
osip_mutex_lock (struct osip_mutex *_mut)
{
  osip_mutex_t *mut = (osip_mutex_t *) _mut;

  if (mut) {
    if (sm_p (mut->id, SM_WAIT, 0) != 0)
      return OSIP_UNDEFINED_ERROR;
  }
  return (0);
}

int
osip_mutex_unlock (struct osip_mutex *_mut)
{
  osip_mutex_t *mut = (osip_mutex_t *) _mut;

  if (mut) {
    sm_v (mut->id);
  }

  return (0);
}

struct osip_sem *
osip_sem_init (unsigned int value)
{
  osip_sem_t *sem = (osip_sem_t *) osip_malloc (sizeof (osip_sem_t));

  if (sm_create ("sem", value, 0, &sem->id) == 0)
    return (struct osip_sem *) (sem);
  osip_free (sem);
  return (NULL);
}

int
osip_sem_destroy (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_SUCCESS;
  sm_delete (sem->id);
  osip_free (sem);
  return (0);
}

int
osip_sem_post (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  return (sm_v (sem->id));
}

int
osip_sem_wait (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  if (sm_p (sem->id, SM_WAIT, 0) != 0)
    return (-1);
  return (0);
}

int
osip_sem_trywait (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL)
    return OSIP_BADPARAMETER;
  if (sm_p (sem->id, SM_NOWAIT, 0) != 0)
    return OSIP_UNDEFINED_ERROR;
  return (0);
}

#endif



#if defined(__rtems__)

struct osip_mutex *
osip_mutex_init ()
{
  rtems_status_code status;
  osip_mutex_t *mut = (osip_mutex_t *) osip_malloc (sizeof (osip_mutex_t));

  status = rtems_semaphore_create (rtems_build_name ('s', 'i', 'p', 'M'), 1,    /* Count */
                                   RTEMS_SIMPLE_BINARY_SEMAPHORE, 0, &mut->id);
  if (status == RTEMS_SUCCESSFUL) {
    return (struct osip_mutex *) (mut);
  }
  osip_free (mut);
  return (NULL);
}

void
osip_mutex_destroy (struct osip_mutex *_mut)
{
  osip_mutex_t *mut = (osip_mutex_t *) _mut;

  if (mut != NULL) {
    rtems_semaphore_delete (mut->id);
    osip_free (mut);
  }
}

int
osip_mutex_lock (struct osip_mutex *_mut)
{
  rtems_status_code status;
  osip_mutex_t *mut = (osip_mutex_t *) _mut;

  if (mut != NULL) {
    status = rtems_semaphore_obtain (mut->id, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    if (status != RTEMS_SUCCESSFUL) {
      return OSIP_UNDEFINED_ERROR;
    }
  }
  return OSIP_SUCCESS;
}

int
osip_mutex_unlock (struct osip_mutex *_mut)
{
  osip_mutex_t *mut = (osip_mutex_t *) _mut;

  if (mut != NULL) {
    (void) rtems_semaphore_release (mut->id);
  }

  return (0);
}

struct osip_sem *
osip_sem_init (unsigned int value)
{
  rtems_status_code status;

  osip_sem_t *sem = (osip_sem_t *) osip_malloc (sizeof (osip_sem_t));

  status = rtems_semaphore_create (rtems_build_name ('s', 'i', 'p', 'S'), value, RTEMS_COUNTING_SEMAPHORE, 0, &sem->id);

  if (status == RTEMS_SUCCESSFUL) {
    return (struct osip_sem *) (sem);
  }
  osip_free (sem);
  return (NULL);
}

int
osip_sem_destroy (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL) {
    return OSIP_SUCCESS;
  }
  rtems_semaphore_delete (sem->id);
  osip_free (sem);
  return (0);
}

int
osip_sem_post (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL) {
    return OSIP_UNDEFINED_ERROR;
  }
  return rtems_semaphore_release (sem->id);
}

int
osip_sem_wait (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL) {
    return OSIP_BADPARAMETER;
  }
  if (rtems_semaphore_obtain (sem->id, RTEMS_WAIT, RTEMS_NO_TIMEOUT) != RTEMS_SUCCESSFUL) {
    return OSIP_UNDEFINED_ERROR;
  }
  return (0);
}

int
osip_sem_trywait (struct osip_sem *_sem)
{
  osip_sem_t *sem = (osip_sem_t *) _sem;

  if (sem == NULL) {
    return OSIP_BADPARAMETER;
  }
  if (rtems_semaphore_obtain (sem->id, RTEMS_NO_WAIT, 0) != RTEMS_SUCCESSFUL) {
    return OSIP_UNDEFINED_ERROR;
  }
  return (0);
}


#endif
#endif /* #ifndef OSIP_MONOTHREAD */
