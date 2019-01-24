#pragma once

#include "map.h"
#include "v.h"
#include "c.h"


void gen_pixmap_sample (uint8_t * data, size_t w, size_t h, size_t d)
{
	for (size_t y = 0; y < h; ++ y)
	for (size_t x = 0; x < w; ++ x)
	//for (size_t c = 0; c < d; ++ c)
	{
		//size_t i = d*w*y + d*x + c;
		size_t i = d*w*y + d*x;
		uint8_t c = x*y;
		data [i + 0] = c;
		data [i + 1] = c;
		data [i + 2] = c;
		data [i + 3] = 255;
	}
}


void gen_mandelbrot_pallete (uint8_t * o, size_t count, size_t depth)
{
	ASSERT (depth == 4);
	float s [3] = {0};
	s [0] = 360.0f / (float)(depth * count);
	//s [1] = 1.0f / (float)(depth * count);
	s [2] = 0.5f / (float)(depth * count);
	float t [3] = {0};
	t [0] = 0.0f;
	t [1] = 1.0f;
	t [2] = 0.5f;

	for (size_t i = 0; i < (depth * count); i += depth)
	{
		float hsv [3];
		float rgb [3];
		hsv [0] = (float)i * s [0] + t [0];
		hsv [1] = (float)i * s [1] + t [1];
		hsv [2] = (float)i * s [2] + t [2];
		rgb_from_hsv (rgb, hsv);
		//TRACE_F ("%f %f %f %f", hsv [0], rgb [0], rgb [1], rgb [2]);
		o [i + 0] = rgb [0] * 255;
		o [i + 1] = rgb [1] * 255;
		o [i + 2] = rgb [2] * 255;
		o [i + 3] = 255;
	}
}


void gen_mandelbrot_pixmap 
(uint8_t * o, uint8_t const * pallete, size_t w, size_t h, size_t d, size_t max_iteration)
{
	ASSERT (d == 4);
	//For each pixel (ix, iy) on the screen, do:
	for (size_t ix = 0; ix < w; ++ ix)
	for (size_t iy = 0; iy < h; ++ iy)
	{
		size_t const i = (d * w * iy) + (d * ix);
		//Scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.5, 1))
		//Scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1, 1))
		float x0 = map_lin_float (ix, 0.0f, (float) w, -2.5f, 1.0f);
		float y0 = map_lin_float (iy, 0.0f, (float) h, -1.0f, 1.0f);
		float x = 0.0f;
		float y = 0.0f;
		size_t j;
		for (j = 0; j < max_iteration-1; ++ j)
		{
			if (x*x + y*y >= 2*2) {break;}
			float xt = (x * x) - (y * y) + x0;
			y = (2.0f * x * y) + y0;
			x = xt;
		}
		ASSERT (j < d*max_iteration);
		memcpy (o + i, pallete + (j*d), d);
		//TRACE_F ("%i", i);
	}
}


void gen_economic_curve (uint8_t * o, size_t w, size_t h, size_t d)
{
	memset (o, 0, w * h * d);
	float * f = malloc (w);
	for (size_t i = 0; i < 100; ++ i)
	{
		f [i] = cos ((float)i / 40.0f);
	}
	
	for (size_t ix = 0; ix < w; ++ ix)
	{
		size_t iy = f [ix];
		size_t const i = (d * w * iy) + (d * ix);
		o [i+0] = 1.0f;
		o [i+1] = 1.0f;
		o [i+2] = 1.0f;
		o [i+3] = 1.0f;
	}
	free (f);
}


void gen4x6_square_pos (float v [24], float x, float y, float w, float h)
{
	float v0 [] =
	{
		x,     y + h,   0.0f, 0.0f,
		x,     y,       0.0f, 0.0f,
		x + w, y,       0.0f, 0.0f,
		x,     y + h,   0.0f, 0.0f,
		x + w, y,       0.0f, 0.0f,
		x + w, y + h,   0.0f, 0.0f       
	};
	memcpy (v, v0, sizeof (v0));
}


void gen4x6_square_pos_gridcell (float v [24], uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
	float dx = 2.0f / (float) w;
	float dy = 2.0f / (float) h;
	float xx = (float) x * dx - 1.0f;
	float yy = (float) y * dy - 1.0f;
	gen4x6_square_pos (v, xx, yy, dx, dy);
}


void gen4x6_square_col (float v [24], float r, float g, float b, float a)
{
	float v0 [] =
	{
		r, g, b, a,
		r, g, b, a,
		r, g, b, a,
		r, g, b, a,
		r, g, b, a,
		r, g, b, a
	};
	memcpy (v, v0, sizeof (v0));
}


void gen_pos_line_xy (float v [8], float x0, float y0, float x1, float y1)
{
	float v0 [] =
	{
		x0,    y0,   0.0f, 0.0f,
		x1,    y1,   0.0f, 0.0f,
	};
	memcpy (v, v0, sizeof (v0));
}


void gen_col (uint32_t vn, float v [], float r, float g, float b, float a)
{
	for (uint32_t i = 0; i < vn; i += 4)
	{
		v [i+0] = r;
		v [i+1] = g;
		v [i+2] = b;
		v [i+3] = a;
	}
}


void gen_grid 
(
	float v [], 
	uint32_t r, 
	uint32_t c, 
	float x, 
	float y,
	float w,
	float h
)
{
	float const dx = w / (float)c;
	float const dy = h / (float)r;
	while (c--)
	{
		v [0] = x + dx * c;
		v [1] = y;
		v [2] = 0;
		v [3] = 0;
		v [4] = x + dx * c;
		v [5] = y + h;
		v [6] = 0;
		v [7] = 0;
		v += 8;
	}
	while (r--)
	{
		v [0] = x;
		v [1] = y + dy * r;
		v [2] = 0;
		v [3] = 0;
		v [4] = x + w;
		v [5] = y + dy * r;
		v [6] = 0;
		v [7] = 0;
		v += 8;
	}
}


void gen_randcurve 
(
	float v [],
	uint32_t n,
	float ymin,
	float ymax,
	float x,
	float w
)
{
	float dy = ymax - ymin;
	float dx = w / (float) n;
	while (n--)
	{
		float rnd = (float) rand () / (float) RAND_MAX;
		v [0] = x + n * dx;
		v [1] = rnd * dy + ymin;
		//TRACE_F ("%f %f", (double)v [0], (double)v [1]);
		v [2] = 0.0f;
		v [3] = 0.0f;
		v += 4;
	}
}


void gen_curve 
(
	float v [],
	uint32_t * n,
	FILE * f,
	float ymin,
	float ymax,
	float x,
	float w
)
{
	float * V;
	//float dy = ymax - ymin;
	uint32_t i = 0;
	V = v;
	for (i = 0; i < *n; ++ i)
	{
		int r = fscanf (f, "%f", V + 1);
		//TRACE_F ("%f %f", (double)v [0], (double)v [1]);
		V [2] = 0.0f;
		V [3] = 0.0f;
		V += 4;
		if (r == EOF) {break;}
	}
	*n = i;
	TRACE_F ("%i", *n);
	
	float dx = w / (float) *n;
	V = v;
	for (i = 0; i < *n; ++ i)
	{
		V [0] = x + i * dx;
		V += 4;
	}
}


void gen4x6_square_tex1 (float v [24], float w, float h, float l)
{
	float v0 [24] =
	{
		0.0f, 0.0f, l, 0.0f,
		0.0f, h   , l, 0.0f,
		w   , h   , l, 0.0f,
		0.0f, 0.0f, l, 0.0f,
		w   , h   , l, 0.0f,
		w   , 0.0f, l, 0.0f       
	};
	memcpy (v, v0, sizeof (v0));
}


void gen4x6_square_tex (uint32_t n, float v [24], uint8_t const l [])
{
	for (uint32_t i = 0; i < n; ++ i)
	{
		gen4x6_square_tex1 (v, 1.0f, 1.0f, (float)l [i]);
		v += 24;
	}
}


void gen4x6_grid_pos 
(
	float v [],
	uint32_t w,
	uint32_t h
)
{
	float dx = 2.0f / (float) w;
	float dy = 2.0f / (float) h;
	for (uint32_t y = 0; y < w; ++y)
	for (uint32_t x = 0; x < h; ++x)
	{
		float xx = (float)x * dx - 1.0f;
		float yy = (float)y * dy - 1.0f;
		gen4x6_square_pos (v, xx, yy, dx, dy);
		v += 24;
	}
}
