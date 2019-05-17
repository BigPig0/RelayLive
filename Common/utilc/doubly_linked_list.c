#include "doubly_linked_list.h"
#include <stddef.h>

void dll_add_front(struct lws_dll *d, struct lws_dll *phead)
{
	if (d->prev)
		return;

	/* our next guy is current first guy */
	d->next = phead->next;
	/* if there is a next guy, set his prev ptr to our next ptr */
	if (d->next)
		d->next->prev = d;
	/* our prev ptr is first ptr */
	d->prev = phead;
	/* set the first guy to be us */
	phead->next = d;
}

/* situation is:
 *
 *  HEAD: struct lws_dll * = &entry1
 *
 *  Entry 1: struct lws_dll  .pprev = &HEAD , .next = Entry 2
 *  Entry 2: struct lws_dll  .pprev = &entry1 , .next = &entry2
 *  Entry 3: struct lws_dll  .pprev = &entry2 , .next = NULL
 *
 *  Delete Entry1:
 *
 *   - HEAD = &entry2
 *   - Entry2: .pprev = &HEAD, .next = &entry3
 *   - Entry3: .pprev = &entry2, .next = NULL
 *
 *  Delete Entry2:
 *
 *   - HEAD = &entry1
 *   - Entry1: .pprev = &HEAD, .next = &entry3
 *   - Entry3: .pprev = &entry1, .next = NULL
 *
 *  Delete Entry3:
 *
 *   - HEAD = &entry1
 *   - Entry1: .pprev = &HEAD, .next = &entry2
 *   - Entry2: .pprev = &entry1, .next = NULL
 *
 */

void dll_remove(struct lws_dll *d)
{
	if (!d->prev) /* ie, not part of the list */
		return;

	/*
	 *  remove us
	 *
	 *  USp <-> us <-> USn  -->  USp <-> USn
	 */

	/* if we have a next guy, set his prev to our prev */
	if (d->next)
		d->next->prev = d->prev;

	/* set our prev guy to our next guy instead of us */
	if (d->prev)
		d->prev->next = d->next;

	/* we're out of the list, we should not point anywhere any more */
	d->prev = NULL;
	d->next = NULL;
}