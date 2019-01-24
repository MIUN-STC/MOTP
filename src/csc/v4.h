#pragma once

#include "v.h"

struct v4f32_XYZW
{
	float x;
	float y;
	float z;
	float w;
};


struct v4f32_XYZW v4f32_XYZW (float x, float y, float z, float w)
{
	struct v4f32_XYZW v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}


struct v4f32_rgba
{
	float r;
	float g;
	float b;
	float a;
};


struct v4f32_xywh
{
	float x;
	float y;
	float w;
	float h;
};


struct v4u32_xywh
{
	uint32_t x;
	uint32_t y;
	uint32_t w;
	uint32_t h;
};


struct v4f32_xyuv
{
	float x;
	float y;
	float u;
	float v;
};



//static_assert (sizeof (struct v4f32_rgba) == sizeof (float) * 4, "struct v4f32_rgba is not 4 x float size");
//static_assert (sizeof (struct v4f32_xywh) == sizeof (float) * 4, "struct v4f32_xywh is not 4 x float size");







void v4f32_cpy (float r [4], float const a [4])
{
	vf32_cpy (4, r, a);
}

void v4f32_add (float r [4], float const a [4], float const b [4])
{
	vf32_add (4, r, a, b);
}


void v4f32_sub (float r [4], float const a [4], float const b [4])
{
	vf32_sub (4, r, a, b);
}


void v4f32_set1 (float r [4], float const b)
{
	vf32_set1 (4, r, b);
}


void v4f32_mus (float r [4], float const a [4], float const b)
{
	vf32_mus (4, r, a, b);
}


void v4f32_normalize (float r [4], float const a [4])
{
	vf32_normalize (4, r, a);
}


float v4f32_norm2 (float const a [4])
{
	return vf32_dot (4, a, a);
}


//xyzw xyzw xyzw ... n times
void v4f32_repeat4 (uint32_t n, float r [], float x, float y, float z, float w)
{
	while (n--)
	{
		//TRACE_F ("%i", n);
		r [0] = x;
		r [1] = y;
		r [2] = z;
		r [3] = w;
		r += 4;
	}
}


void v4f32_repeat_random (uint32_t n, float r [])
{
	uint32_t const dim = 4;
	while (n--)
	{
		vf32_random (dim, r);
		r += dim;
	}
}


void v4f32_repeat_channel (uint32_t n, float r [], uint32_t channel, float a)
{
	while (n--)
	{
		r [channel] = a;
		r += 4;
	}
}

