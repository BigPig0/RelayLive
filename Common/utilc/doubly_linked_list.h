/*
 * doubly linked-list
 */
struct lws_dll { /* abstract */
	struct lws_dll *prev;
	struct lws_dll *next;
};

/*
 * these all point to the composed list objects... you have to use the
 * lws_container_of() helper to recover the start of the containing struct
 */

void dll_add_front(struct lws_dll *d, struct lws_dll *phead);

void lws_dll_remove(struct lws_dll *d);

#define dll_is_null(___dll) (!(___dll)->prev && !(___dll)->next)

/*
 * these are safe against the current container object getting deleted,
 * since the hold his next in a temp and go to that next.  ___tmp is
 * the temp.
 */

#define start_foreach_dll_safe(___type, ___it, ___tmp, ___start) \
{ \
	___type ___it = ___start; \
	while (___it) { \
		___type ___tmp = (___it)->next;

#define end_foreach_dll_safe(___it, ___tmp) \
		___it = ___tmp; \
	} \
}

#define start_foreach_dll(___type, ___it, ___start) \
{ \
	___type ___it = ___start; \
	while (___it) {

#define end_foreach_dll(___it) \
		___it = (___it)->next; \
	} \
}