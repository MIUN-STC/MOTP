#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "debug.h"


#define BUFQ_ERROR 1 << 0
#define BUFQ_FULL 1 << 1
#define BUFQ_MALLOC 1 << 2

#define BUFQ_LOOP1(q,t,e) for (t * (e) = (q).base; (e) != NULL; (e) = bufq_next(&(q)))
#define BUFQ_LOOP2(q,t,e) for (t * (e) = (q).base; (e) <= (t *) bufq_last (&(q)); ++(e))

struct BufQ
{
	uint32_t type;
	uint32_t flags;
	uint32_t capacity;
	uint32_t esize8;
	uint32_t count;
	void * base;
};

void bufq_init (struct BufQ * q)
{
	q->base = malloc (q->esize8 * q->capacity);
	q->count = 0;
}


void * bufq_at (struct BufQ * q, size_t i)
{
	uintptr_t b = (uintptr_t) q->base;
	b += q->esize8 * i;
	return (void *) b;
}


void * bufq_next (struct BufQ * q)
{
	if (q->count == q->capacity)
	{
		q->flags |= BUFQ_FULL;
		return NULL;
	}
	ASSERT (q->count < q->capacity);
	q->count ++;
	return bufq_at (q, q->count - 1);
}




void * bufq_last (struct BufQ * q)
{
	return bufq_at (q, q->count - 1);
}



