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


#ifndef _LIST_H_
#define _LIST_H_

#ifdef ENABLE_MPATROL
#include <mpatrol.h>
#endif

/**
 * @file osip_list.h
 * @brief oSIP list Routines
 *
 * This is a simple implementation of a linked list.
 */

/**
 * @defgroup oSIP_LIST oSIP list Handling
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN
/**
 * Structure for referencing a node in a osip_list_t element.
 * @var __node_t
 */
  typedef struct __node __node_t;

/**
 * Structure for referencing a node in a osip_list_t element.
 * @struct __node
 */
  struct __node {
    __node_t *next;         /**< next __node_t containing element */
    void *element;          /**< element in Current node */
  };
#endif

/**
 * Structure for referencing a list of elements.
 * @var osip_list_t
 */
  typedef struct osip_list osip_list_t;

/**
 * Structure used to iterate list.
 * @var osip_list_iterator_t
 */
  typedef struct {
    __node_t *actual; /**< actual */
    __node_t **prev;  /**< prev */
    osip_list_t *li;  /**< li */
    int pos;          /**< pos */
  } osip_list_iterator_t;

/**
 * Structure for referencing a list of elements.
 * @struct osip_list
 */
  struct osip_list {

    int nb_elt;                 /**< Number of element in the list */
    __node_t *node;             /**< Next node containing element  */

  };

/**
 * Initialise a osip_list_t element.
 * NOTE: this element MUST be previously allocated with
 * osip_malloc(). The osip_free() call on the list is
 * still automatically done by osip_list_free(). This
 * also means you can't use a static osip_list_t variable
 * if you want to use osip_list_free().
 * @param li The element to initialise.
 */
  int osip_list_init (osip_list_t * li);
/**
 * Free a list of element.
 * Each element will be free with the method given as the second parameter.
 * @param li The element to work on.
 * @param free_func The method that is able to release one element of the list.
 */
  void osip_list_special_free (osip_list_t * li, void (*free_func) (void *));
/**
 * Clone a list of element.
 * Each element will be cloned with the method given as the second parameter.
 * @param src The element to work on.
 * @param dst The element to work on.
 * @param clone_func The method that is able to release one element of the list.
 */
  int osip_list_clone (const osip_list_t * src, osip_list_t * dst, int (*clone_func) (void *, void **));
/**
 * Free a list of element where elements are pointer to 'char'.
 * @param li The element to work on.
 */
  void osip_list_ofchar_free (osip_list_t * li);
/**
 * Get the size of a list of element.
 * @param li The element to work on.
 */
  int osip_list_size (const osip_list_t * li);
/**
 * Check if the end of list is detected .
 * @param li The element to work on.
 * @param pos The index of the possible element.
 */
  int osip_list_eol (const osip_list_t * li, int pos);
/**
 * Add an element in a list.
 * @param li The element to work on.
 * @param element The pointer on the element to add.
 * @param pos the index of the element to add. (or -1 to append the element at the end)
 */
  int osip_list_add (osip_list_t * li, void *element, int pos);
/**
 * Get an element from a list.
 * @param li The element to work on.
 * @param pos the index of the element to get.
 */
  void *osip_list_get (const osip_list_t * li, int pos);
/**
 * Remove an element from a list.
 * @param li The element to work on.
 * @param pos the index of the element to remove.
 */
  int osip_list_remove (osip_list_t * li, int pos);

/**
 * Check current iterator state.
 * @param it The element to work on.
 */
#define osip_list_iterator_has_elem( it ) ( 0 != (it).actual && (it).pos < (it).li->nb_elt )
/**
 * Get first iterator from list.
 * @param li The element to work on.
 * @param it The iterator.
 */
  void *osip_list_get_first (const osip_list_t * li, osip_list_iterator_t * it);
/**
 * GEt next iterator.
 * @param it The element to work on.
 */
  void *osip_list_get_next (osip_list_iterator_t * it);
/**
 * Remove current iterator.
 * @param it The element to work on.
 */
  void *osip_list_iterator_remove (osip_list_iterator_t * it);

#ifdef __cplusplus
}
#endif
/** @} */
#endif
