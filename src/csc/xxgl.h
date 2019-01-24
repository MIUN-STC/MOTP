#pragma once

#include <csc/debug.h>
#include <csc/debug_gl.h>

void glVertexAttribPointer_byname 
(
	GLuint program,
	char const * name,
	uint32_t dim,
	GLenum type,
	GLboolean normalized,
	uint32_t stride,
	uint32_t offset
)
{
	GLint loc = glGetAttribLocation (program, name);
	ASSERT_F (loc >= 0, "glGetAttribLocation (%i, %s) no attribute found", (int) program, name);
	glVertexAttribPointer (loc, (GLint) dim, type, normalized, (GLsizei) stride, (const GLvoid *) (uintptr_t) offset);
	glEnableVertexAttribArray (loc);
}




char const * xxgl_primitive2str (GLenum primitive)
{
	switch (primitive)
	{
		case GL_POINTS:
		return "GL_POINTS";
		case GL_LINES:
		return "GL_LINES";
		case GL_TRIANGLES:
		return "GL_TRIANGLES";
		case GL_LINE_STRIP:
		return "GL_LINE_STRIP";
	}
	return "";
}


char const * xxgl_type2str (GLenum type)
{
	switch (type)
	{
		case GL_FLOAT:
		return "GL_FLOAT";
	}
	return "";
}


char const * xxgl_bool2str (GLboolean value)
{
	switch (value)
	{
		case GL_TRUE:
		return "GL_TRUE";
		case GL_FALSE:
		return "GL_FALSE";
	}
	return "";
}


void xxgl_layout 
(
	uint32_t n, 
	uint32_t  const index      [],
	uint32_t  const dim        [],
	GLenum    const type       [],
	GLboolean const normalized [],
	uint32_t  const stride     [],
	uint32_t  const offset     [],
	GLenum    const target     [],
	GLuint    const vbo        []
)
{
	for (uint32_t i = 0; i < n; ++ i)
	{
		glBindBuffer (target [i], vbo [i]);
		glEnableVertexAttribArray (index [i]);
		glVertexAttribPointer 
		(
			(GLuint)         index [i], 
			(uint32_t)       dim [i], 
			(GLenum)         type [i], 
			(GLboolean)      normalized [i], 
			(GLint)          stride [i], 
			(GLvoid const *) (uintptr_t) offset [i]
		);
		TRACE_F 
		(
			"index: %02i; dim: %01i; type: %10s; normalized: %8s; stride: %03i; offset: %04i;", 
			(int)index [i], 
			(int)dim [i], 
			xxgl_type2str (type [i]), 
			xxgl_bool2str (normalized [i]), 
			(int)stride [i], 
			(int)offset [i]
		);
		GL_CHECK_ERROR;
	}
}


void xxgl_layout11 
(
	uint32_t n, 
	uint32_t  const dim,
	GLenum    const type,
	GLboolean const normalized,
	GLenum    const target,
	GLuint    const vbo        []
)
{
	for (uint32_t i = 0; i < n; ++ i)
	{
		uint32_t  const index = i;
		uint32_t  const stride = 0;
		uint32_t  const offset = 0;
		
		glBindBuffer (target, vbo [i]);
		glEnableVertexAttribArray (index);
		glVertexAttribPointer 
		(
			(GLuint)         index, 
			(uint32_t)       dim, 
			(GLenum)         type, 
			(GLboolean)      normalized, 
			(GLint)          stride, 
			(GLvoid const *) (uintptr_t) offset
		);
		TRACE_F 
		(
			"index: %02i; dim: %01i; type: %10s; normalized: %8s; stride: %03i; offset: %04i;", 
			(int)index, 
			(int)dim, 
			xxgl_type2str (type), 
			xxgl_bool2str (normalized), 
			(int)stride, 
			(int)offset
		);
		GL_CHECK_ERROR;
	}
}


void xxgl_allocate 
(
	uint32_t n, 
	GLenum     const   target [],
	GLuint     const   vbo    [],
	uint32_t   const   stride [],
	void       const * data   [],
	GLbitfield const   flags  [],
	uint32_t vn
)
{
	for (uint32_t i = 0; i < n; ++ i)
	{
		GLvoid const * data0 = data ? data [i] : data;
		glBindBuffer (target [i], vbo [i]);
		glBufferStorage (target [i], stride [i] * vn, data0, flags [i]);
		GL_CHECK_ERROR;
	}
}







