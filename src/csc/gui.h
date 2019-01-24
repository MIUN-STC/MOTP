#pragma once

#include "v.h"
#include "v2.h"
#include "gen.h"


__attribute__((packed))
struct GUI_Vertex
{
	float p [2];
	float c [4];
	float uv [2];
};


void gui_vertex_print (struct GUI_Vertex * v, size_t n)
{
	for (size_t i = 0; i < 8; ++ i)
	{
		printf ("%f %f : %f %f %f %f\n", v [i].p [0], v [i].p [1], v [i].c [0], v [i].c [1], v [i].c [2], v [i].c [3]);
	}
}


__attribute__((packed))
struct GUI_VertexRectangle
{
	struct GUI_Vertex sw;
	struct GUI_Vertex nw;
	struct GUI_Vertex ne;
	struct GUI_Vertex se;
};


void gui_vertex_layout 
(GLuint vao, GLuint vbo, GLuint ebo, size_t v8, size_t e8, void * v, void * e)
{
	/*
	struct Vertex_Format f;
	f.location = 0;
	f.size = 0;
	f.type = GL_FLOAT;
	f.normalized = GL_FALSE;
	f.stride = sizeof (struct GUI_Vertex);
	f.offset = 0;
	*/
	glBindVertexArray (vao);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glBufferData (GL_ARRAY_BUFFER, v8, v, GL_STATIC_DRAW);
	glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData (GL_ELEMENT_ARRAY_BUFFER, e8, e, GL_STATIC_DRAW);
	glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, sizeof (struct GUI_Vertex), (void*)0);
	glEnableVertexAttribArray (0);
	glVertexAttribPointer (1, 4, GL_FLOAT, GL_FALSE, sizeof (struct GUI_Vertex), (void*) (2 * sizeof (float)));
	glEnableVertexAttribArray (1);
	glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, sizeof (struct GUI_Vertex), (void*) (6 * sizeof (float)));
	glEnableVertexAttribArray (2);
}


void setup_texture2d (GLuint tex, size_t w, size_t h)
{
	//GL_RGBA
	size_t d = 4;
	glBindTexture (GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	uint8_t * data = malloc (w * h * d);
	//gen_pixmap_sample (data, w, h, d);
	uint8_t pallete [100 * 4];
	gen_mandelbrot_pallete (pallete, 100, 4);
	gen_mandelbrot_pixmap (data, pallete, w, h, d, 1000);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap (GL_TEXTURE_2D);
	free (data);
}


void setup_texture3d (GLuint tex, size_t w, size_t h)
{
	//GL_RGBA
	size_t d = 4;
	glBindTexture (GL_TEXTURE_2D_ARRAY, tex);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	uint8_t * data = malloc (w * h * d * 2);
	//gen_pixmap_sample (data, w, h, d);
	uint8_t pallete [100 * 4];
	gen_mandelbrot_pallete (pallete, 100, 4);
	gen_mandelbrot_pixmap (data, pallete, w, h, d, 1000);
	glTexImage3D
	(
		GL_TEXTURE_2D_ARRAY,
		0,                // level
		GL_RGBA8,         // Internal format
		w, h, 2, // width,height,depth
		0,                // border?
		GL_RGBA,          // format
		GL_UNSIGNED_BYTE, // type
		data
	);
	glGenerateMipmap (GL_TEXTURE_2D_ARRAY);
	free (data);
}




__attribute__((packed))
struct GUI_RectangleSWNE
{
	float sw [2]; //Bottom left
	float ne [2]; //Top right
};

struct GUI_RectangleSWNE GUI_RectangleSWNE (float swx, float swy, float nex, float ney)
{
	struct GUI_RectangleSWNE r;
	r.sw [0] = swx;
	r.sw [1] = swy;
	r.ne [0] = nex;
	r.ne [1] = ney;
	return r;
}


__attribute__((packed))
struct GUI_ElementsRectangle
{
	unsigned int v [6];
};


void gui_gen_elements (struct GUI_ElementsRectangle e [], size_t n)
{
	for (size_t i = 0; i < n; ++ i)
	{
		e [i].v [0] = 0 + (i * 4);
		e [i].v [1] = 1 + (i * 4);
		e [i].v [2] = 3 + (i * 4);
		e [i].v [3] = 1 + (i * 4);
		e [i].v [4] = 2 + (i * 4);
		e [i].v [5] = 3 + (i * 4);
	}
}


void gui_gen_vertices 
(struct GUI_VertexRectangle v [], struct GUI_RectangleSWNE const x [], size_t xn)
{
	//Set random color for now. A temporary test.
	srand (0);
	for (size_t i = 0; i < xn; ++ i)
	{
		float * vsw = v [i].sw.c;
		float * vnw = v [i].nw.c;
		float * vne = v [i].ne.c;
		float * vse = v [i].se.c;
		vf32_random (vsw, 3);
		vf32_random (vnw, 3);
		vf32_random (vne, 3);
		vf32_random (vse, 3);
		vsw [3] = 1.0f;
		vnw [3] = 1.0f;
		vne [3] = 1.0f;
		vse [3] = 1.0f;
	}
	
	//Set the uv coordinates.
	//        (1)
	//         |
	// (-1)---(0)---(1)
	//         |
	//        (-1)
	for (size_t i = 0; i < xn; ++ i)
	{
		float * uvsw = v [i].sw.uv;
		float * uvnw = v [i].nw.uv;
		float * uvne = v [i].ne.uv;
		float * uvse = v [i].se.uv;
		uvsw [0] = 1.0; uvsw [1] = 1.0; // top right
		uvnw [0] = 1.0; uvnw [1] = 0.0; // bottom right
		uvne [0] = 0.0; uvne [1] = 0.0; // bottom left
		uvse [0] = 0.0; uvse [1] = 1.0; // top left 
	}

	//The rectangle is described as two corner points (nw,se).
	//Extract all corners (sw,nw,ne,se) from (nw,se) and copy them to vertices.
	//nw xx --> nw ne
	//xx se --> sw se
	for (size_t i = 0; i < xn; ++ i)
	{
		float const * rsw = x [i].sw;
		float const * rne = x [i].ne;
		float * vsw = v [i].sw.p;
		float * vnw = v [i].nw.p;
		float * vne = v [i].ne.p;
		float * vse = v [i].se.p;
		vf32_cpy (vsw, rsw, 2);
		vnw [0] = rsw [0];
		vnw [1] = rne [1];
		vf32_cpy (vne, rne, 2);
		vse [0] = rne [0];
		vse [1] = rsw [1];
	}
}


#define GUI_COLOR1 1 << 0

struct GUI_Object
{
	uint32_t flags;
	
};


struct GUI
{
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	
	struct GUI_VertexRectangle * vertices;
	struct GUI_RectangleSWNE * rectangles;
	struct GUI_Object * objects;
	size_t n;
};





void gui_init (struct GUI * g, size_t n)
{
	struct GUI_ElementsRectangle * elements;
	size_t const v8 = sizeof (struct GUI_VertexRectangle) * n;
	size_t const e8 = sizeof (struct GUI_ElementsRectangle) * n;
	g->n = n;
	//calloc allocates and set memory data to zero.
	g->rectangles = calloc (n, sizeof (struct GUI_RectangleSWNE));
	g->vertices = calloc (n, sizeof (struct GUI_VertexRectangle));
	g->objects = calloc (n, sizeof (struct GUI_Object));
	elements = malloc (e8);
	glGenVertexArrays (1, &g->vao);
	glGenBuffers (1, &g->vbo);
	glGenBuffers (1, &g->ebo);
	gui_gen_elements (elements, n);
	gui_vertex_layout (g->vao, g->vbo, g->ebo, v8, e8, NULL, elements);
	free (elements);
}


void gui_sync (struct GUI * g)
{
	gui_gen_vertices (g->vertices, g->rectangles, g->n);
	size_t v8 = sizeof (struct GUI_VertexRectangle) * g->n;
	glBufferSubData (GL_ARRAY_BUFFER, 0, v8, g->vertices);
}


void gui_draw (struct GUI * g, GLint uniform)
{
	//Number of rectangles (g->n).
	size_t const n = g->n;
	//Vertex array object.
	GLuint const vao = g->vao;
	
	struct GUI_Object const * o = g->objects;
	
	glBindVertexArray (vao);
	//glDrawElements (GL_TRIANGLES, n * 6, GL_UNSIGNED_INT, 0);
	
	for (size_t i = 0; i < n; ++ i)
	{
		if (o [i].flags & GUI_COLOR1)
		{
			glUniform1i (uniform, 1);
			//glUniform4f (uniform, 1.0, 1.0, 1.0, 1.0);
		}
		else
		{
			glUniform1i (uniform, 0);
			//glUniform4f (uniform, 0.0, 0.0, 0.0, 0.0);
		}
		size_t m = i * 6 * sizeof (unsigned int);
		glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)m);
	}
}




//Add or remove flags to a intersected object.
void gui_intersect_point 
(struct GUI * g, float p [2], int32_t flags_add, int32_t flags_remove)
{
	//Number of rectangles (g->n).
	size_t const n = g->n;
	//Rectangles ((SW1, NE1), (SW2, NE2), ...) <=> ((vec2, vec2), (vec2, vec2))
	struct GUI_RectangleSWNE * r = g->rectangles;
	for (size_t i = 0; i < n; ++ i)
	{
		float d [2];
		float const * sw = r [i].sw;
		float const * ne = r [i].ne;
		
		//00 p  --> Check if p is on the SW side
		//SW 00 --> 
		v2f32_sub (d, sw, p);
		if ((d [0] > 0.0f) || (d [1] > 0.0f)) {continue;}
		
		//00 NE --> Check if p is on the SW side
		//p  00 --> 
		v2f32_sub (d, ne, p);
		if ((d [0] < 0.0f) || (d [1] < 0.0f)) {continue;}
		
		//P should intersects the rectangle now.
		//Use bitfield operations to add or remove flags from the intersected object.
		//The user can set the flags_add, flags_remove.
		g->objects [i].flags |= flags_add;
		g->objects [i].flags &= ~flags_remove;
	}
}







