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

#include <osipparser2/osip_port.h>
#include <osip2/osip_fifo.h>


/* always use this method to initiate osip_fifo_t.
*/
void
osip_fifo_init (osip_fifo_t * ff)
{
#ifndef OSIP_MONOTHREAD
  ff->qislocked = osip_mutex_init ();
  /*INIT SEMA TO BLOCK ON GET() WHEN QUEUE IS EMPTY */
  ff->qisempty = osip_sem_init (0);
#endif
  osip_list_init (&ff->queue);
  /* ff->nb_elt = 0; */
  ff->state = osip_empty;
}

int
osip_fifo_add (osip_fifo_t * ff, void *el)
{
#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (ff->qislocked);
#endif

  osip_list_add (&ff->queue, el, -1);   /* insert at end of queue */
  ff->state = osip_ok;

#ifndef OSIP_MONOTHREAD
  osip_sem_post (ff->qisempty);
  osip_mutex_unlock (ff->qislocked);
#endif
  return OSIP_SUCCESS;
}


int
osip_fifo_insert (osip_fifo_t * ff, void *el)
{
#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (ff->qislocked);
#endif

  osip_list_add (&ff->queue, el, 0);    /* insert at end of queue */
  ff->state = osip_ok;

#ifndef OSIP_MONOTHREAD
  osip_sem_post (ff->qisempty);
  osip_mutex_unlock (ff->qislocked);
#endif
  return OSIP_SUCCESS;
}


int
osip_fifo_size (osip_fifo_t * ff)
{
  int i;

#ifndef OSIP_MONOTHREAD
  osip_mutex_lock (ff->qislocked);
#endif

  i = osip_list_size (&ff->queue);
#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (ff->qislocked);
#endif
  return i;
}


void *
osip_fifo_get (osip_fifo_t * ff)
{
  void *el = NULL;

#ifndef OSIP_MONOTHREAD
  int i = osip_sem_wait (ff->qisempty);

  if (i != 0)
    return NULL;
  osip_mutex_lock (ff->qislocked);
#endif

  if (ff->state != osip_empty) {
    el = osip_list_get (&ff->queue, 0);
    osip_list_remove (&ff->queue, 0);
    /* ff->nb_elt--; */
  }
  else {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "no element in fifo.\n"));
#ifndef OSIP_MONOTHREAD
    osip_mutex_unlock (ff->qislocked);
#endif
    return OSIP_SUCCESS;        /* pile vide */
  }
  /* if (ff->nb_elt <= 0) */
  if (osip_list_size (&ff->queue) <= 0)
    ff->state = osip_empty;
  else
    ff->state = osip_ok;

#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (ff->qislocked);
#endif
  return el;
}

void *
osip_fifo_tryget (osip_fifo_t * ff)
{
  void *el = NULL;

#ifndef OSIP_MONOTHREAD
  if (0 != osip_sem_trywait (ff->qisempty)) {   /* no elements... */
    return NULL;
  }
  osip_mutex_lock (ff->qislocked);
#else
  if (ff->state == osip_empty)
    return NULL;
#endif

  if (ff->state != osip_empty) {
    el = osip_list_get (&ff->queue, 0);
    osip_list_remove (&ff->queue, 0);
    /* ff->nb_elt--; */
  }
#ifndef OSIP_MONOTHREAD
  else {                        /* this case MUST never happen... */
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO4, NULL, "no element in fifo.\n"));
    osip_mutex_unlock (ff->qislocked);
    return OSIP_SUCCESS;
  }
#endif

  /* if (ff->nb_elt <= 0) */
  if (osip_list_size (&ff->queue) <= 0)
    ff->state = osip_empty;
  else
    ff->state = osip_ok;

#ifndef OSIP_MONOTHREAD
  osip_mutex_unlock (ff->qislocked);
#endif
  return el;
}

void
osip_fifo_free (osip_fifo_t * ff)
{
  if (ff == NULL)
    return;
#ifndef OSIP_MONOTHREAD
  osip_mutex_destroy (ff->qislocked);
  /* seems that pthread_mutex_destroy does not free space by itself */
  osip_sem_destroy (ff->qisempty);
#endif
  osip_free (ff);
}
