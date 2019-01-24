#pragma once

#include "global.h"

//sqrt
#include <math.h>

//rand
#include <stdlib.h>

//FLT_MAX
#include <float.h>


#include <stdarg.h> 



void vf32_cpy (uint32_t n, float des [], float const src [])
{
	while (n--)
	{
		des [n] = src [n];
	}
}


// r := a . b
float vf32_dot (uint32_t n, float const a [], float const b [])
{
	float r = 0;
	while (n--)
	{
		r += a [n] * b [n];
	}
	return r;
}


// r := a + b
void vf32_add (uint32_t n, float r [], float const a [], float const b [])
{
	while (n--)
	{
		r [n] = a [n] + b [n];
	}
}


// r := a + b
void vf32_add1 (uint32_t n, float r [], float const a [], float const b)
{
	while (n--)
	{
		r [n] = a [n] + b;
	}
}


// r := a + b
void vf32_add1max (uint32_t n, float r [], float const a [], float const b, float max)
{
	while (n--)
	{
		if (r [n] < max) 
		{	
			r [n] = a [n] + b;
		}
	}
}


// r := a - b
void vf32_sub (uint32_t n, float r [], float const a [], float const b [])
{
	while (n--)
	{
		r [n] = a [n] - b [n];
	}
}


// r := a - b
void vf32_sub1 (uint32_t n, float r [], float const a [], float b)
{
	while (n--)
	{
		r [n] = a [n] - b;
	}
}


// r := a < b
void vf32_lt (uint32_t n, float r [], float const a [], float const b [])
{
	while (n--)
	{
		r [n] = a [n] < b [n];
	}
}


// r := a > b
void vf32_gt (uint32_t n, float r [], float const a [], float const b [])
{
	while (n--)
	{
		r [n] = a [n] > b [n];
	}
}


// ret a > b
int vf32_gt_all (uint32_t n, float const a [], float const b [])
{
	while (n--)
	{
		if (a [n] <= b [n]) {return 0;}
	}
	return 1;
}


// ret a < b
int vf32_lt_all (uint32_t n, float const a [], float const b [])
{
	while (n--)
	{
		if (a [n] <= b [n]) {return 0;}
	}
	return 1;
}


// Set all element (x) of r to b
// r := {x | x = ?}
void vf32_random (uint32_t n, float r [])
{
	while (n--)
	{
		r [n] = (float)rand () / (float)RAND_MAX;
		//r [i] = 1.0f;
	}
}


// Set all element (x) of r to b
// r := {x | x = b}
void vf32_set1 (uint32_t n, float r [], float const b)
{
	while (n--)
	{
		r [n] = b;
	}
}


// r := a * b
void vf32_mul (uint32_t n, float r [], float const a [], float const b [])
{
	while (n--)
	{
		r [n] = a [n] * b [n];
	}
}


// r := a * b
void vf32_mus (uint32_t n, float r [], float const a [], float const b)
{
	while (n--)
	{
		r [n] = a [n] * b;
	}
}


// ret |a|^2
float vf32_norm2 (uint32_t n, float const a [])
{
	return vf32_dot (n, a, a);
}


// ret |a|
float vf32_norm (uint32_t n, float const a [])
{
	return sqrtf (vf32_norm2 (n, a));
}


// r := a / |a|
void vf32_normalize (uint32_t n, float r [], float const a [])
{
	float const l = vf32_norm (n, a);
	float const s = l > 0.0f ? 1.0f / l : 0.0f;
	vf32_mus (n, r, a, s);
}


void vf32_linespace (uint32_t n, float x [], float x1, float x2)
{
	float const d = (x2 - x1) / n;
	float a = x1;
	while (n--)
	{
		x [n] = a;
		a += d;
	}
}


int vf32_ray_sphere_intersect 
(float p [3], float d [3], float sc [3], float sr, float *t, float q [3]) 
{
	//Vector m = p - s.c;
	float m [3];
	vf32_sub (3, p, sc, m);
	//float b = Dot(m, d); 
	float b = vf32_dot (3, m, d);
	//float c = Dot(m, m) - s.r * s.r; 
	float c = vf32_dot (3, m, m) - (sr * sr);

	// Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0) 
	if (c > 0.0f && b > 0.0f) {return 0;}
	float discr = (b * b) - c; 

	// A negative discriminant corresponds to ray missing sphere 
	if (discr < 0.0f) {return 0;}

	// Ray now found to intersect sphere, compute smallest t value of intersection
	*t = -b - sqrtf (discr); 

	// If t is negative, ray started inside sphere so clamp t to zero 
	if (*t < 0.0f) {*t = 0.0f;}
	*t = MAX (*t, 0.0f);
	
	//q = p + dt;
	vf32_mus (3, q, d, *t);
	vf32_add (3, q, p, q);
	
	return 1;
}


void vf32_print (FILE * f, float const x [], size_t n, char const * format)
{
	fprintf (f, "(");
	for (size_t i = 0; i < n; ++ i)
	{
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wformat-nonliteral"
		fprintf (f, format, (double) x [i]);
		#pragma GCC diagnostic pop
	}
	fprintf (f, "\b)\n");
	fflush (f);
}


void vf32_print2 (FILE * f, float const x [], size_t n1, size_t n2, char const * format)
{
	for (size_t i = 0; i < n1; ++ i)
	{
		vf32_print (f, x + (i*n2), n2, format);
	}
}


// r := a - b
void vf32_sub2 (float r [], float const a [], float const b [], size_t an, size_t bn)
{
	for (size_t i = 0; i < an; ++ i)
	{
		vf32_sub (bn, r + (i*bn), a + (i*bn), b);
	}
}


void vf32_setl (float r [], uint32_t n, ...)
{
	va_list ap;
	va_start (ap, n);
	for (uint32_t i = 0; i < n; ++i)
	{
		r [i] = va_arg (ap, double);
	}
	va_end (ap);
}


void vu32_setl (uint32_t r [], uint32_t n, ...)
{
	va_list ap;
	va_start (ap, n);
	for (uint32_t i = 0; i < n; ++i)
	{
		r [i] = va_arg (ap, uint32_t);
	}
	va_end (ap);
}


void vu32_set1 (uint32_t n, uint32_t r [], uint32_t v)
{
	while (n--)
	{
		r [n] = v;
	}
}


void vu32_and1 (uint32_t n, uint32_t r [], uint32_t a [], uint32_t b)
{
	while (n--)
	{
		r [n] = a [n] & b;
	}
}


// r := a + b
void vu32_add1max (uint32_t n, uint32_t r [], uint32_t const a [], uint32_t b, uint32_t max)
{
	while (n--)
	{
		if (r [n] < max) 
		{	
			r [n] = a [n] + b;
		}
	}
}


// r := a + b
void vu32_add1min (uint32_t n, uint32_t r [], uint32_t const a [], uint32_t b, uint32_t min)
{
	while (n--)
	{
		if (r [n] > min) 
		{	
			r [n] = a [n] + b;
		}
	}
}



void vu32_ladder (uint32_t n, uint32_t x [], uint32_t const d [])
{
	uint32_t h = 0;
	//Must be forward iteration
	for (uint32_t i = 0; i < n; ++ i)
	{
		x [i] = h;
		h += d [i];
	}
}


void vu32_linespace (uint32_t n, uint32_t x [], uint32_t x1, uint32_t x2)
{
	float d = ((float)x2 - (float)x1) / n;
	float a = (float)x1;
	while (n--)
	{
		x [n] = (uint32_t)a;
		a += d;
	}
}


void vf32_repeat (uint32_t n, float v [], float value, uint32_t offset, uint32_t stride)
{
	v += offset;
	while (n--)
	{
		v [0] = value;
		v += stride;
	}
}


void vf32_weight_ab (uint32_t n, float y [], float a [], float b [], float k)
{
	float A = k;
	float B = 1.0f - k;
	while (n--)
	{
		y [n] = A * a [n] + B * b [n];
	}
}


