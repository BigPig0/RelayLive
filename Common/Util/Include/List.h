#pragma once

#define ListForEach(pos, head)        for (pos = (head)->next; pos != (head); pos = pos->next)
#define ListForEachPrev(pos, head)   for (pos = (head)->prev; pos != (head); pos = pos->prev)

#define ListForEachSafe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)
#define ListForEachPrevSafe(pos, n, head) \
    for (pos = (head)->prev, n = pos->prev; pos != (head); pos = n, n = pos->prev)

template<typename _T>
struct List
{
    explicit List() : prev(this), next(this), size(0)
    {}

    explicit List(_T payload) : value(payload)
    {}

    bool empty()
    {
        return next == this;
    }

    void push_front(List *node)
    {
        node->next = next;
        node->prev = this;
        next->prev = node;
        next = node;
        size++;
    }

    void push_back(List *node)
    {
        node->next = this;
        node->prev = prev;
        prev->next = node;
        prev = node;
        size++;
    }

    void pop_back()
    {
        erase(prev);
    }

    template<typename Free>
    void pop_back(Free fn)
    {
        erase(prev, fn);
    }

    void pop_front()
    {
        erase(next);
    }

    template<typename Free>
    void pop_front(Free fn)
    {
        erase(next, fn);
    }

    _T front()
    {
        return next->value;
    }

    _T back()
    {
        return prev->value;
    }

    void erase(List *pos)
    {
        pos->next->prev = pos->prev;
        pos->prev->next = pos->next;
        delete pos;
        --size;
    }

    template<typename Free>
    void erase(List *pos, Free fn)
    {
        pos->next->prev = pos->prev;
        pos->prev->next = pos->next;
        fn(pos->value);
        delete pos;
        --size;
    }

    bool isHead(List *pos)
    {
        return pos->prev == this;
    }

    bool isTail(List *pos)
    {
        return pos->next == this;
    }

    _T value;
    List *prev, *next;
    int size;
};
