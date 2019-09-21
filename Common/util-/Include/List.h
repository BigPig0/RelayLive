#pragma once

#define list_for_each(pos, head)            for (pos = (head)->next; pos != (head); pos = pos->next)
#define list_for_each_prev(pos, head)       for (pos = (head)->prev; pos != (head); pos = pos->prev)

#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)
#define list_for_each_prev_safe(pos, n, head) \
    for (pos = (head)->prev, n = pos->prev; pos != (head); pos = n, n = pos->prev)

////////////////////////////////////////////////////////////////////////////////
template<typename Type>
struct List {
	explicit List() : prev(this), next(this)
	{}

	explicit List(Type v) : value(v)
	{}

	bool empty()
	{
		return next == this;
	}

	List *push_front(Type v)
	{
		List *pos = new List(v);
		pos->next = next;
		pos->prev = this;
		next->prev = pos;
		next = pos;
		return pos;
	}

	List *push_back(Type v)
	{
		List *pos = new List(v);
		pos->next = this;
		pos->prev = prev;
		prev->next = pos;
		prev = pos;
		return pos;
	}

	void pop_back()
	{
		erase(prev);
	}

	template<typename FreeFunc>
	void pop_back(FreeFunc func)
	{
		erase(prev, func);
	}

	void pop_front()
	{
		erase(next);
	}

	template<typename FreeFunc>
	void pop_front(FreeFunc func)
	{
		erase(next, func);
	}

	Type front()
	{
		return next->value;
	}

	Type back()
	{
		return prev->value;
	}

	void erase(List *pos)
	{
		pos->next->prev = pos->prev;
		pos->prev->next = pos->next;
		delete pos;
	}

	template<typename FreeFunc>
	void erase(List *pos, FreeFunc func)
	{
		pos->next->prev = pos->prev;
		pos->prev->next = pos->next;
		func(pos->value);
		delete pos;
	}

	bool is_head(List *pos)
	{
		return pos->prev == this;
	}

	bool is_tail(List *pos)
	{
		return pos->next == this;
	}

	Type value;
	List *prev, *next;
};

////////////////////////////////////////////////////////////////////////////////
template<typename Type>
struct Queue {
	void produce(Type v)
	{
		cs.lock();
		list.push_back(v);
		size++;
		cs.unlock();
	}

	Type consume()
	{
		Type v;
		cs.lock();
		v = list.empty() ? 0 : list.front();
		if (v) {
			list.pop_front();
			--size;
		}
		cs.unlock();
		return v;
	}

	CriticalSection cs;
	List<Type> list;
	uint volatile size;
    Queue():size(0){}
};

////////////////////////////////////////////////////////////////////////////////
template<typename Type>
struct LockedList {
	void add(Type v, List<Type> **pos)
	{
		cs.lock();
		*pos = list.push_back(v);
		size++;
		cs.unlock();
	}

	void del(List<Type> *pos)
	{
		cs.lock();
		list.erase(pos);
		--size;
		cs.unlock();
	}

	template<typename FreeFunc>
	void del(List<Type> *pos, FreeFunc func)
	{
		cs.lock();
		list.erase(pos, func);
		--size;
		cs.unlock();
	}

	void free()
	{
		if (size > 0) {
			List<Type> *pos, *candidate;
			list_for_each_safe(pos, candidate, &list) {
				delete pos->value;
				delete pos;
			}
		}
	}

	CriticalSection cs;
	List<Type> list;
	uint volatile size;
    LockedList():size(0){}
};
