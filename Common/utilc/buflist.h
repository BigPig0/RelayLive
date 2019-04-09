#include <stdint.h>

typedef struct _buflist_ buflist_t;

/**
 * buflist_append_segment(): add buffer to buflist at head
 *
 * \param head: list head
 * \param buf: buffer to stash
 * \param len: length of buffer to stash
 *
 * Returns -1 on OOM, 1 if this was the first segment on the list, and 0 if
 * it was a subsequent segment.
 */
int buflist_append_segment(buflist_t **head, const uint8_t *buf, size_t len);

/**
 * buflist_next_segment_len(): number of bytes left in current segment
 *
 * \param head: list head
 * \param buf: if non-NULL, *buf is written with the address of the start of
 *		the remaining data in the segment
 *
 * Returns the number of bytes left in the current segment.  0 indicates
 * that the buflist is empty (there are no segments on the buflist).
 */
size_t buflist_next_segment_len(buflist_t **head, uint8_t **buf);

/**
 * buflist_use_segment(): remove len bytes from the current segment
 *
 * \param head: list head
 * \param len: number of bytes to mark as used
 *
 * If len is less than the remaining length of the current segment, the position
 * in the current segment is simply advanced and it returns.
 *
 * If len uses up the remaining length of the current segment, then the segment
 * is deleted and the list head moves to the next segment if any.
 *
 * Returns the number of bytes left in the current segment.  0 indicates
 * that the buflist is empty (there are no segments on the buflist).
 */
int buflist_use_segment(buflist_t **head, size_t len);

/**
 * buflist_destroy_all_segments(): free all segments on the list
 *
 * \param head: list head
 *
 * This frees everything on the list unconditionally.  *head is always
 * NULL after this.
 */
void buflist_destroy_all_segments(buflist_t **head);

void buflist_describe(buflist_t **head, void *id);
