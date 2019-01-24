#pragma once

#include "v.h"

struct v3f32_XYZ
{
	float x;
	float y;
	float z;
};

struct v3f32_XYZ v3f32_XYZ (float x, float y, float z)
{
	struct v3f32_XYZ v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}


void v3f32_cpy (float r [3], float const a [3])
{
	vf32_cpy (r, a, 3);
}


void v3f32_add (float r [3], float const a [3], float const b [3])
{
	vf32_add (r, a, b, 3);
}


void v3f32_mul (float r [3], float const a [3], float const b [3])
{
	vf32_mul (r, a, b, 3);
}


void v3f32_sub (float r [3], float const a [3], float const b [3])
{
	vf32_sub (r, a, b, 3);
}


void v3f32_set (float r [3], float const b)
{
	vf32_set_scalar (r, b, 3);
}


void v3f32_mul_scalar (float r [3], float const a [3], float const b)
{
	vf32_mul_scalar (r, a, b, 3);
}


void v3f32_mus (float r [3], float const a [3], float const b)
{
	vf32_mul_scalar (r, a, b, 3);
}


void v3f32_normalize (float r [3], float const a [3])
{
	vf32_normalize (r, a, 3);
}


float v3f32_dot (float const a [3], float const b [3])
{
	return vf32_dot (a, b, 3);
}


float v3f32_norm2 (float const a [3])
{
	return vf32_dot (a, a, 3);
}


float v3f32_dist2 (float const a [4], float const b [4])
{
	float d [3];
	v3f32_sub (d, a, b);
	return v3f32_norm2 (d);
}
