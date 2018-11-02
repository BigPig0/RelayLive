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

#include <osipparser2/internal.h>

#include <osipparser2/osip_port.h>
#include <osipparser2/osip_list.h>

int
osip_list_init (osip_list_t * li)
{
  if (li == NULL)
    return OSIP_BADPARAMETER;
  memset (li, 0, sizeof (osip_list_t));
  return OSIP_SUCCESS;          /* ok */
}

int
osip_list_clone (const osip_list_t * src, osip_list_t * dst, int (*clone_func) (void *, void **))
{
  void *data;
  void *data2;
  int i;
  osip_list_iterator_t iterator;

  for (data = osip_list_get_first ((osip_list_t *) src, &iterator); osip_list_iterator_has_elem (iterator); data = osip_list_get_next (&iterator)) {
    i = clone_func (data, &data2);
    if (i != 0)
      return i;
    osip_list_add (dst, data2, -1);
  }
  return OSIP_SUCCESS;
}

void
osip_list_special_free (osip_list_t * li, void (*free_func) (void *))
{
  void *element;

  if (li == NULL)
    return;
  while (!osip_list_eol (li, 0)) {
    element = (void *) osip_list_get (li, 0);
    osip_list_remove (li, 0);
    if (free_func != NULL)
      free_func (element);
  }
}

void
osip_list_ofchar_free (osip_list_t * li)
{
  char *chain;

  if (li == NULL)
    return;
  while (!osip_list_eol (li, 0)) {
    chain = (char *) osip_list_get (li, 0);
    osip_list_remove (li, 0);
    osip_free (chain);
  }
}

int
osip_list_size (const osip_list_t * li)
{
  if (li == NULL)
    return OSIP_BADPARAMETER;

  return li->nb_elt;
}

int
osip_list_eol (const osip_list_t * li, int i)
{
  if (li == NULL)
    return OSIP_BADPARAMETER;
  if (i < li->nb_elt)
    return OSIP_SUCCESS;        /* not end of list */
  return 1;                     /* end of list */
}

/* index starts from 0; */
int
osip_list_add (osip_list_t * li, void *el, int pos)
{
  __node_t *ntmp;
  int i = 0;

  if (li == NULL)
    return OSIP_BADPARAMETER;

  if (li->nb_elt == 0) {

    li->node = (__node_t *) osip_malloc (sizeof (__node_t));
    if (li->node == NULL)
      return OSIP_NOMEM;
    li->node->element = el;
    li->node->next = NULL;
    li->nb_elt++;
    return li->nb_elt;
  }

  if (pos == -1 || pos >= li->nb_elt) { /* insert at the end  */
    pos = li->nb_elt;
  }

  ntmp = li->node;              /* exist because nb_elt>0  */

  if (pos == 0) {               /* pos = 0 insert before first elt  */
    li->node = (__node_t *) osip_malloc (sizeof (__node_t));
    if (li->node == NULL) {
      /* leave the list unchanged */
      li->node = ntmp;
      return OSIP_NOMEM;
    }
    li->node->element = el;
    li->node->next = ntmp;
    li->nb_elt++;
    return li->nb_elt;
  }


  while (pos > i + 1) {
    i++;
    /* when pos>i next node exist  */
    ntmp = ntmp->next;
  }

  /* if pos==nb_elt next node does not exist  */
  if (pos == li->nb_elt) {
    ntmp->next = osip_malloc (sizeof (__node_t));
    if (ntmp->next == NULL)
      return OSIP_NOMEM;        /* leave the list unchanged */
    ntmp = ntmp->next;
    ntmp->element = el;
    ntmp->next = NULL;
    li->nb_elt++;
    return li->nb_elt;
  }

  /* here pos==i so next node is where we want to insert new node */
  {
    __node_t *nextnode = ntmp->next;

    ntmp->next = osip_malloc (sizeof (__node_t));
    if (ntmp->next == NULL) {
      /* leave the list unchanged */
      ntmp->next = nextnode;
      return OSIP_NOMEM;
    }
    ntmp = ntmp->next;
    ntmp->element = el;
    ntmp->next = nextnode;
    li->nb_elt++;
  }
  return li->nb_elt;
}

/* index starts from 0 */
void *
osip_list_get (const osip_list_t * li, int pos)
{
  __node_t *ntmp;
  int i = 0;

  if (li == NULL)
    return NULL;

  if (pos < 0 || pos >= li->nb_elt)
    /* element does not exist */
    return NULL;


  ntmp = li->node;              /* exist because nb_elt>0 */

  while (pos > i) {
    i++;
    ntmp = ntmp->next;
  }
  return ntmp->element;
}

/* added by bennewit@cs.tu-berlin.de */
void *
osip_list_get_first (const osip_list_t * li, osip_list_iterator_t * iterator)
{
  if (li==NULL || 0 >= li->nb_elt) {
    iterator->actual = 0;
    return OSIP_SUCCESS;
  }

  iterator->actual = li->node;
  iterator->prev = (__node_t**)&li->node;
  iterator->li = (osip_list_t*)li;
  iterator->pos = 0;

  return li->node->element;
}

/* added by bennewit@cs.tu-berlin.de */
void *
osip_list_get_next (osip_list_iterator_t * iterator)
{
  if (iterator->actual == NULL) {
    return OSIP_SUCCESS;
  }

  iterator->prev = &(iterator->actual->next);
  iterator->actual = iterator->actual->next;
  ++(iterator->pos);

  if (osip_list_iterator_has_elem (*iterator)) {
    return iterator->actual->element;
  }

  iterator->actual = 0;
  return OSIP_SUCCESS;
}

/* added by bennewit@cs.tu-berlin.de */
void *
osip_list_iterator_remove (osip_list_iterator_t * iterator)
{
  if (osip_list_iterator_has_elem (*iterator)) {
    --(iterator->li->nb_elt);

    *(iterator->prev) = iterator->actual->next;

    osip_free (iterator->actual);
    iterator->actual = *(iterator->prev);
  }

  if (osip_list_iterator_has_elem (*iterator)) {
    return iterator->actual->element;
  }

  return OSIP_SUCCESS;
}

/* return -1 if failed */
int
osip_list_remove (osip_list_t * li, int pos)
{

  __node_t *ntmp;
  int i = 0;

  if (li == NULL)
    return OSIP_BADPARAMETER;

  if (pos < 0 || pos >= li->nb_elt)
    /* element does not exist */
    return OSIP_UNDEFINED_ERROR;

  ntmp = li->node;              /* exist because nb_elt>0 */

  if (pos == 0) {               /* special case  */
    li->node = ntmp->next;
    li->nb_elt--;
    osip_free (ntmp);
    return li->nb_elt;
  }

  while (pos > i + 1) {
    i++;
    ntmp = ntmp->next;
  }

  /* insert new node */
  {
    __node_t *remnode;

    remnode = ntmp->next;
    ntmp->next = (ntmp->next)->next;
    osip_free (remnode);
    li->nb_elt--;
  }
  return li->nb_elt;
}
