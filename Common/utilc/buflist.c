#include "buflist.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#ifndef __func__
#define __func__ __FUNCTION__
#endif

typedef struct _buflist_ {
    struct _buflist_ *next;

    size_t len;
    size_t pos;

    uint8_t buf[1]; /* true length of this is set by the oversize malloc */
} buflist_t;

int buflist_append_segment(buflist_t **head, const uint8_t *buf, size_t len)
{
    buflist_t *nbuf;
    int first = !*head;
    void *p = *head;
    int sanity = 1024;

    assert(buf);
    assert(len);

    /* append at the tail */
    while (*head) {
        if (!--sanity || head == &((*head)->next)) {
            printf("%s: corrupt list points to self\n", __func__);
            return -1;
        }
        head = &((*head)->next);
    }

    printf("%s: len %u first %d %p\n", __func__, (uint32_t)len, first, p);

    nbuf = (buflist_t*)malloc(sizeof(**head) + len);
    if (!nbuf) {
        printf("%s: OOM\n", __func__);
        return -1;
    }

    nbuf->len = len;
    nbuf->pos = 0;
    nbuf->next = NULL;

    p = (void *)nbuf->buf;
    memcpy(p, buf, len);

    *head = nbuf;

    return first; /* returns 1 if first segment just created */
}

static int buflist_destroy_segment(buflist_t **head)
{
    buflist_t *old = *head;

    assert(*head);
    *head = (*head)->next;
    old->next = NULL;
    free(old);

    return !*head; /* returns 1 if last segment just destroyed */
}

void buflist_destroy_all_segments(buflist_t **head)
{
    buflist_t *p = *head, *p1;

    while (p) {
        p1 = p->next;
        p->next = NULL;
        free(p);
        p = p1;
    }

    *head = NULL;
}

size_t buflist_next_segment_len(buflist_t **head, uint8_t **buf)
{
    if (!*head) {
        if (buf)
            *buf = NULL;

        return 0;
    }

    if (!(*head)->len && (*head)->next)
        buflist_destroy_segment(head);

    if (!*head) {
        if (buf)
            *buf = NULL;

        return 0;
    }

    assert((*head)->pos < (*head)->len);

    if (buf)
        *buf = (*head)->buf + (*head)->pos;

    return (*head)->len - (*head)->pos;
}

int buflist_use_segment(buflist_t **head, size_t len)
{
    assert(*head);
    assert(len);
    assert((*head)->pos + len <= (*head)->len);

    (*head)->pos += len;
    if ((*head)->pos == (*head)->len)
        buflist_destroy_segment(head);

    if (!*head)
        return 0;

    return (int)((*head)->len - (*head)->pos);
}

void buflist_describe(buflist_t **head, void *id)
{
    buflist_t *old;
    int n = 0;

    if (*head == NULL)
        printf("%p: buflist empty\n", id);

    while (*head) {
        printf("%p: %d: %llu / %llu (%llu left)\n", id, n,
            (unsigned long long)(*head)->pos,
            (unsigned long long)(*head)->len,
            (unsigned long long)(*head)->len - (*head)->pos);
        old = *head;
        head = &((*head)->next);
        if (*head == old) {
            printf("%s: next points to self\n", __func__);
            break;
        }
        n++;
    }
}
