#pragma once

#include "v.h"

struct v2f32_xy
{
	float v [2];
};


// ret a > b
int v2f32_gt_all (float const a [2], float const b [2])
{
	return vf32_gt_all (2, a, b);
}


// ret a < b
int v2f32_lt_all (float const a [2], float const b [2])
{
	return vf32_lt_all (2, a, b);
}


// r := a > b
void v2f32_gt (float r [2], float const a [2], float const b [2])
{
	vf32_gt (2, r, a, b);
}


// r := a < b
void v2f32_lt (float r [2], float const a [2], float const b [2])
{
	vf32_lt (2, r, a, b);
}


// r := a - b
void v2f32_sub (float r [2], float const a [2], float const b [2])
{
	vf32_sub (2, r, a, b);
}


// r := a - b
void v2f32_add (float r [2], float const a [2], float const b [2])
{
	vf32_add (2, r, a, b);
}


// r := a - b
void v2f32_mus (float r [2], float const a [2], float const b)
{
	vf32_mus (2, r, a, b);
}


float v2f32_dot (float const a [2], float const b [2])
{
	return vf32_dot (2, a, b);
}


float v2f32_norm2 (float const a [2])
{
	return vf32_dot (2, a, a);
}


float v2f32_dist2 (float const a [2], float const b [2])
{
	float d [2];
	v2f32_sub (d, a, b);
	return v2f32_norm2 (d);
}


void v2f32_random_wh (uint32_t n, float p [], float w, float h)
{
	while (n--)
	{
		//Normalized random between 0.0 .. 1.0.
		p [0] = (float) rand () / (float) RAND_MAX;
		p [1] = (float) rand () / (float) RAND_MAX;
		//Sclaed random between (0.0 .. w, 0.0 .. h)
		p [0] *= w;
		p [1] *= h;
		p += 2; //Position has dim=2.
	}
}


void v2f32_random (uint32_t n, float p [])
{
	while (n--)
	{
		//Normalized random between 0.0 .. 1.0.
		p [0] = (float) rand () * 2 / (float) RAND_MAX;
		p [1] = (float) rand () * 2 / (float) RAND_MAX;
		//Sclaed random between (-1.0 .. 1.0, -1.0 .. 1.0)
		p [0] -= 1;
		p [1] -= 1;
		p += 2; //Position has dim=2.
	}
}
