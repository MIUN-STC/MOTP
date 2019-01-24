#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "debug.h"

#define BUFG_MALLOC 1 << 0
//#define BUFG_RING 1 << 1

/*
#define BUFG_UNKNOWN 0
#define BUFG_F32 1
#define BUFG_F64 2
#define BUFG_S8 3
#define BUFG_S16 4
#define BUFG_S32 5
#define BUFG_S64 6
#define BUFG_S128 7
#define BUFG_U8 3
#define BUFG_U16 4
#define BUFG_U32 5
#define BUFG_U64 6
#define BUFG_U128 7
*/

#define BUFG_LOOP(t, i, d) for (t (i) = 0; i < bufg_count ((d)); (i) = (i) + 1)
#define BUFG_LOOP_CAPACITY(t, i, d) for (t (i) = 0; i < bufg_capacity ((d)); (i) = (i) + 1)

struct BufG
{
	uint16_t flags;
	
	//Element size as 8 bit unit
	uint16_t element_size8;
	
	//Element count
	uint32_t count;
	
	//Capacity count
	uint32_t capacity;
};



void * bufg_init (void * data, uint32_t capacity, uint8_t element_size8, uint8_t flags)
{
	TRACE_F ("element_size8 %i", element_size8);
	struct BufG * g = data;
	if ((g == NULL) && (flags & BUFG_MALLOC))
	{
		size_t n = capacity * element_size8 + sizeof (struct BufG);
		g = malloc (n);
		ASSERT (g != NULL);
		//memset (g, 0, n);
	}
	ASSERT (g != NULL);
	g->count = 0;
	g->capacity = capacity;
	g->element_size8 = element_size8;
	g->flags = flags;
	return g + 1;
}


struct BufG * bufg_meta (void * data)
{
	struct BufG * g = data;
	g = g - 1;
	return g;
}

uint32_t bufg_count (void * data)
{
	struct BufG * g = bufg_meta (data);
	return g->count;
}


uint32_t bufg_capacity (void * data)
{
	struct BufG * g = bufg_meta (data);
	return g->capacity;
}

uint32_t bufg_cap8 (void * data)
{
	struct BufG * g = bufg_meta (data);
	return g->capacity * g->element_size8;
}



void * bufg_add (void * data)
{
	struct BufG * g = bufg_meta (data);
	if (g->count >= g->capacity) {return NULL;}
	g->count ++;
	uintptr_t d = (uintptr_t)data + g->element_size8 * (g->count - 1);
	return (void *)d;
}


void * bufg_get (void * data, size_t index)
{
	struct BufG * g = bufg_meta (data);
	if (index >= g->count) {return NULL;}
	uintptr_t d = (uintptr_t)data + g->element_size8 * index;
	return (void *)d;
}


/*
void bufg_print (struct BufG * g)
{
	for (size_t i = 0; i < n; i ++)
	{
		printf ("%f %f\n", x [i].x, x [i].y);
	}
}
* */




