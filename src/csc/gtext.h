#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

//Common simple c functions
#include <csc/SDLGL.h>
#include <csc/debug.h>
#include <csc/debug_gl.h>
#include <csc/gen.h>
#include <csc/xxgl.h>
#include <csc/xxgl_dr.h>


struct gtext_fdim
{
	uint32_t n;
	float * a;
	float * x;
	float * y;
	float * w;
	float * h;
	float * u;
	float * v;
};


void gtext_fdim_calloc (struct gtext_fdim * g)
{
	g->a = calloc (g->n, sizeof (float));
	g->x = calloc (g->n, sizeof (float));
	g->y = calloc (g->n, sizeof (float));
	g->w = calloc (g->n, sizeof (float));
	g->h = calloc (g->n, sizeof (float));
	g->u = calloc (g->n, sizeof (float));
	g->v = calloc (g->n, sizeof (float));
	ASSERT (g->a);
	ASSERT (g->x);
	ASSERT (g->y);
	ASSERT (g->w);
	ASSERT (g->h);
	ASSERT (g->u);
	ASSERT (g->v);
}


void gtext_maxsize (FT_Face face, uint32_t n, uint32_t * w, uint32_t * h)
{
	for (uint32_t c = 0; c < n; c++)
	{
		int r = FT_Load_Char (face, c, FT_LOAD_RENDER);
		ASSERT_F (r == 0, "ERROR::FREETYTPE %i: Failed to load Glyph %i", r, c);
		*w = MAX (*w, face->glyph->bitmap.width);
		*h = MAX (*h, face->glyph->bitmap.rows);
	}
}


void gtext_setup 
(
	GLuint tex, 
	FT_Face face, 
	struct gtext_fdim * g
)
{
	//TRACE_F ("%i", face->glyph->metrics.width);
	//TRACE_F ("%i", face->glyph->metrics.height);
	GLenum const target = GL_TEXTURE_2D_ARRAY;
	uint32_t width = 0;
	uint32_t height = 0;
	gtext_maxsize (face, g->n, &width, &height);
	TRACE_F ("FREETYPE: max size %i %i", width, height);
	uint32_t const layerCount = g->n;
	GLsizei const mipLevelCount = 1;
	GLenum const internalformat = GL_R8;
	glBindTexture (target, tex);
	glTexStorage3D 
	(
		target, 
		mipLevelCount, 
		internalformat, 
		(GLsizei)width, 
		(GLsizei)height, 
		(GLsizei)layerCount
	);
	GL_CHECK_ERROR;
	
	//IMPORTANT. Disable byte-alignment restriction.
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	
	for (uint32_t c = 0; c < g->n; c++)
	{
		{
			int r = FT_Load_Char (face, c, FT_LOAD_RENDER);
			ASSERT_F (r == 0, "ERROR::FREETYTPE: Failed to load Glyph %i", r);
			if (r) {continue;}
		}
		
		g->a [c] = (float) (face->glyph->advance.x >> 6);
		g->x [c] = (float)face->glyph->bitmap_left;
		g->y [c] = (float)face->glyph->bitmap_top - (float)face->glyph->bitmap.rows;
		g->w [c] = (float)face->glyph->bitmap.width;
		g->h [c] = (float)face->glyph->bitmap.rows;
		g->u [c] = (float)face->glyph->bitmap.width / (float)width;
		g->v [c] = (float)face->glyph->bitmap.rows / (float)height;
		
		uint32_t xoffset = 0;
		uint32_t yoffset = 0;
		uint32_t zoffset = c;
		
		//TRACE_F ("glyph %i %i %i %i", c, zoffset, face->glyph->bitmap.width, face->glyph->bitmap.rows);
		
		ASSERT (face->glyph->bitmap.width <= width);
		ASSERT (face->glyph->bitmap.rows <= height);
		ASSERT (zoffset < layerCount);
		
		glTexSubImage3D 
		(
			target, 
			0, 
			(GLint) xoffset, 
			(GLint) yoffset, 
			(GLint) zoffset, 
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			1, 
			GL_RED,
			GL_UNSIGNED_BYTE, 
			face->glyph->bitmap.buffer
		);
		GL_CHECK_ERROR;
	}
	
	// Always set reasonable texture parameters
	glTexParameteri (target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri (target, GL_TEXTURE_MAX_LEVEL, 0);
	//Single channel textures stores pixel values in the red channel.
	//It is better to store it in the alpha channel because 
	//then we can choose the color from the vertex color.
	//https://www.g-truc.net/post-0734.html
	//https://www.khronos.org/opengl/wiki/GLAPI/glTexParameter
	glTexParameteri (target, GL_TEXTURE_SWIZZLE_A, GL_RED);
	glTexParameteri (target, GL_TEXTURE_SWIZZLE_R, GL_ZERO);
}

















void gtext_draw 
(
	char const * text, 
	struct gtext_fdim * g,
	struct xxgl_dr * dr,
	uint32_t di,
	
	float ox,
	float oy,
	float sx,
	float sy,
	float sa
)
{
	//Vertex buffer for position, texpos, color.
	float * v;
	//Iterator and current character of the text.
	char const * c;
	
	v = xxgl_dr_vf32_map (dr, di, VBO_POS, 4);
	c = text;
	uint32_t i = 0;
	float ax = 0.0f;
	while (1)
	{
		if (c [0] == '\0') {break;}
		if (i >= dr->capacity [di]) {break;}
		uint8_t iglyph = *c;
		float x = sx * g->x [iglyph] + ox + ax;
		float y = sy * g->y [iglyph] + oy;
		float w = sx * g->w [iglyph];
		float h = sy * g->h [iglyph];
		gen4x6_square_pos (v, x, y, w, h);
		v += 4*6;
		i += 6;
		ax += g->a [iglyph] * sa;
		c ++;
	}
	xxgl_dr_unmap (dr);
	dr->length [di] = i;
	
	v = xxgl_dr_vf32_map (dr, di, VBO_TEX, 4);
	c = text;
	while (*c)
	{
		uint8_t iglyph = *c;
		gen4x6_square_tex1 (v, g->u [iglyph], g->v [iglyph], iglyph);
		v += 4*6;
		c ++;
	}
	xxgl_dr_unmap (dr);
}
