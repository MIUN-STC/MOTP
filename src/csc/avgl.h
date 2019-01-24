#pragma once

#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include "SDLGL.h"
#include "debug.h"
#include "debug_gl.h"


void avgl_TextureArraySub (GLuint tex, uint32_t n, AVFrame * frame [])
{
	GLenum const target = GL_TEXTURE_2D_ARRAY;
	GLint const level = 0; //Level specifies the level-of-detail.
	GLint const xoffset = 0;
	GLint const yoffset = 0;
	GLsizei const depth = 1; //Depth means one frame at a time.
	GLenum const format = GL_RGB;
	GLenum const type = GL_UNSIGNED_BYTE;
	//TRACE_F ("glBindTexture %i", tex);
	glBindTexture (target, tex);
	GL_CHECK_ERROR;
	for (uint32_t i = 0; i < n; ++ i)
	{
		GLsizei const width = frame [i]->width;
		GLsizei const height = frame [i]->height;
		GLvoid const * pixels = frame [i]->data [0];
		GLint zoffset = i;
		glTexSubImage3D 
		(
			target, 
			level, 
			xoffset, 
			yoffset, 
			zoffset, 
			width, 
			height, 
			depth, 
			format, 
			type, 
			pixels
		);
		GL_CHECK_ERROR;
	}
}



void avgl_TextureArrayStorage (GLuint tex, uint32_t n, AVFrame * frame [])
{
	if (n <= 0) {return;}
	GLsizei const width = frame [0]->width;
	GLsizei const height = frame [0]->height;
	GLsizei const layerCount = n;
	GLsizei const mipLevelCount = 1;
	glBindTexture (GL_TEXTURE_2D_ARRAY, tex);
	//TRACE_F ("glTexStorage3D %i %i", width, height);
	glTexStorage3D (GL_TEXTURE_2D_ARRAY, mipLevelCount, GL_RGB8, width, height, layerCount);
	GL_CHECK_ERROR;
	//Always set reasonable texture parameters
	glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
