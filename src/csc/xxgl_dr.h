#pragma once

#include <csc/xxgl.h>


enum xxgl_dr_vbo
{
	VBO_POS,
	VBO_COL,
	VBO_TEX,
	VBO_N
};


struct xxgl_dr
{
	uint32_t n;
	uint32_t * offset;
	uint32_t * length;
	uint32_t * capacity;
	GLenum * primitive;
	
	GLuint vbo [3];
};


float * xxgl_dr_vf32_map (struct xxgl_dr * dr, uint32_t idr, uint32_t ivbo, uint32_t vdim)
{
	ASSERT (dr);
	GLenum const vtarget = GL_ARRAY_BUFFER;
	GLbitfield const vaccess = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
	uint32_t const vsize = sizeof (float) * vdim;
	GLintptr const voffset8 = dr->offset [idr] * vsize;
	GLsizeiptr const vlength8 = dr->length [idr] * vsize;
	glBindBuffer (vtarget, dr->vbo [ivbo]);
	float * v = glMapBufferRange (vtarget, voffset8, vlength8, vaccess);
	ASSERT (v);
	return v;
}


void xxgl_dr_unmap (struct xxgl_dr * dr)
{
	ASSERT (dr);
	GLenum const vtarget = GL_ARRAY_BUFFER;
	glUnmapBuffer (vtarget);
}


void xxgl_dr_calloc (struct xxgl_dr * dr)
{
	dr->offset = calloc (dr->n, sizeof (uint32_t));
	dr->length = calloc (dr->n, sizeof (uint32_t));
	dr->capacity = calloc (dr->n, sizeof (uint32_t));
	dr->primitive = calloc (dr->n, sizeof (GLenum));
	ASSERT (dr->offset);
	ASSERT (dr->length);
	ASSERT (dr->capacity);
	ASSERT (dr->primitive);
	glGenBuffers (3, dr->vbo);
	//ASSERT (glIsBuffer (dr->vbo [0]));
	//ASSERT (glIsBuffer (dr->vbo [1]));
	//ASSERT (glIsBuffer (dr->vbo [2]));
}


uint32_t xxgl_dr_cap (struct xxgl_dr * dr)
{
	return dr->offset [dr->n-1] + dr->capacity [dr->n-1];
}


//first: Specifies the starting index in the enabled arrays.
//count: Specifies the number of indices to be rendered.
void xxgl_dr_draw (struct xxgl_dr * dr, uint32_t i)
{
	//Prevent OpenGL GL_INVALID_OPERATION error when length is zero.
	if (dr->length [i] == 0) {return;}
	glDrawArrays (dr->primitive [i], (GLint)dr->offset [i], (GLsizei)dr->length [i]);
	//glDrawArrays (GL_LINES, (GLint)first [i], (GLsizei)count [i]);
}


void xxgl_dr_allocate (struct xxgl_dr * dr)
{
	uint32_t i = 3;
	while (i--)
	{
		GLenum const flags = GL_MAP_WRITE_BIT;
		GLenum const target = GL_ARRAY_BUFFER;
		GLvoid const * data = NULL;
		GLsizeiptr const size = sizeof (float) * 4 * xxgl_dr_cap (dr);
		glBindBuffer (target, dr->vbo [i]);
		glBufferStorage (target, size, data, flags);
		GL_CHECK_ERROR;
	}
}




void xxgl_dr_v4f32_repeat4
(
	struct xxgl_dr * dr,
	uint32_t ivbo,
	uint32_t idr,
	float x0,
	float x1,
	float x2,
	float x3
)
{
	float * v = xxgl_dr_vf32_map (dr, idr, ivbo, 4);
	v4f32_repeat4 (dr->length [idr], v, x0, x1, x2, x3);
	xxgl_dr_unmap (dr);
}


void xxgl_dr_v4f32_grid
(
	struct xxgl_dr * dr,
	uint32_t ivbo,
	uint32_t idr,
	uint32_t w,
	uint32_t h
)
{
	float * v = xxgl_dr_vf32_map (dr, idr, ivbo, 4);
	gen4x6_grid_pos (v, w, h);
	xxgl_dr_unmap (dr);
}


void xxgl_dr_v4f32_randomcolor 
(
	struct xxgl_dr * dr,
	uint32_t ivbo,
	uint32_t idr,
	uint32_t n
)
{
	float * v = xxgl_dr_vf32_map (dr, idr, ivbo, 4);
	while (n--)
	{
		float r = (float)rand () / (float)RAND_MAX;
		float g = (float)rand () / (float)RAND_MAX;
		float b = (float)rand () / (float)RAND_MAX;
		float a = 1.0f;
		v4f32_repeat4 (6, v, r, g, b, a);
		v += 4 * 6;
	}
	xxgl_dr_unmap (dr);
}



void xxgl_dr_v4f32_squaretex_countup 
(
	struct xxgl_dr * dr,
	uint32_t ivbo,
	uint32_t idr,
	uint32_t n,
	uint32_t a
)
{
	float * v = xxgl_dr_vf32_map (dr, idr, ivbo, 4);
	while (n--)
	{
		gen4x6_square_tex1 (v, 1.0f, 1.0f, a);
		a ++;
		v += 4 * 6;
	}
	xxgl_dr_unmap (dr);
}


