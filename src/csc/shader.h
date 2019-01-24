#pragma once
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>
#include "misc.h"
#include "debug_gl.h"


void gl_program_free_shaders (GLuint program)
{
	ASSERT (glIsProgram (program));

	GLsizei n = 10;
	GLuint shaders [10];
	glGetAttachedShaders (program, n, &n, shaders);
	
	for (GLsizei i = 0; i < n; ++ i)
	{
		glDetachShader (program, shaders [i]);
		glDeleteShader (shaders [i]);
	}
}


//Get the shader type depending on filename extension.
//These extensions are not standard but it makes it easy differentiate between different shader types.
GLuint gl_shader_fileext (char const * filename)
{
	if (0) {}
	else if (strstr (filename, ".glvs"))
	{
		return GL_VERTEX_SHADER;
	}
	else if (strstr (filename, ".glfs"))
	{
		return GL_FRAGMENT_SHADER;
	}
	ASSERT_F (0, "%s", filename);
	return 0;
}



//Load a shader from a file by filename.
//Returns a compiled shader on success.
//Returns a non compiled shader on error.
GLuint gl_shader_from_filename (char const * filename, GLenum kind)
{
	ASSERT (kind != 0);
	GLuint shader = glCreateShader (kind);
	ASSERT (shader != 0);
	//Set the shader source code:
	char * buffer = app_malloc_file (filename);
	glShaderSource (shader, 1, (const GLchar **) &buffer, NULL);
	free (buffer);
	glCompileShader (shader);
	return shader;
}


//Load program by filename.
//The filename must consist of multiple filenames that are refering to shader files.
//e.g. "mydir/myshader.glfs;mydir/myshader.glvs"
GLuint gl_program_from_filename (char const * filename)
{
	GLuint program = glCreateProgram ();
	GL_CHECK_ERROR;
	ASSERT (program > 0);
	struct str_ab s;
	s.a = filename;
	char token [100];
	while (1)
	{
		str_cpytok (100, token, &s, ";");
		TRACE_F ("Token: %s", token);
		GLuint shader = gl_shader_from_filename (token, gl_shader_fileext (token));
		glAttachShader (program, shader);
		GL_CHECK_ERROR;
		if (s.b == NULL) {break;}
	}
	glLinkProgram (program);
	GL_CHECK_ERROR;
	return program;
}


void gl_programs_from_filenames (size_t n, GLuint programs [], char const * filenames [])
{
	for (size_t i = 0; i < n; ++i)
	{
		programs [i] = gl_program_from_filename (filenames [i]);
	}
}


//Convert shader type to string.
char const * gl_str_shader_type (GLenum type)
{
	switch (type)
	{
		case GL_VERTEX_SHADER:
		return "GL_VERTEX_SHADER";
		case GL_FRAGMENT_SHADER:
		return "GL_FRAGMENT_SHADER";
	}
	return "";
}


//Convert GL boolean to string.
char const * gl_str_boolean (GLint value)
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


GLint gl_is_linked (GLuint program)
{
	GLint status;
	glGetProgramiv (program, GL_LINK_STATUS, &status);
	return status;
}


GLint gl_is_compiled (GLuint shader)
{
	GLint status;
	glGetShaderiv (shader, GL_COMPILE_STATUS, &status);
	return status;
}


struct GL_Program_State
{
	GLint delete_status;
	GLint link_status;
	GLint validate_status;
	GLint infolog_length;
	GLint attached_shaders;
	GLint active_atomic_counter_buffers;
	GLint active_attributes;
	GLint active_attribute_max_length;
	GLint active_uniforms;
	GLint active_uniform_max_length;
	GLint binary_length;
	GLint compute_work_group_size;
	GLint transform_feedback_buffer_mode;
	GLint transform_feedback_varyings;
	GLint transform_feedback_varying_max_length;
	GLint geometry_vertices_out;
	GLint geometry_input_type;
	GLint geometry_output_type;
};


void gl_program_get_state (GLuint program, struct GL_Program_State * s)
{
	ASSERT (glIsProgram (program));
	glGetProgramiv (program,                         GL_DELETE_STATUS, &s->delete_status);GL_CHECK_ERROR;
	glGetProgramiv (program,                           GL_LINK_STATUS, &s->link_status);GL_CHECK_ERROR;
	glGetProgramiv (program,                       GL_VALIDATE_STATUS, &s->validate_status);GL_CHECK_ERROR;
	glGetProgramiv (program,                       GL_INFO_LOG_LENGTH, &s->infolog_length);GL_CHECK_ERROR;
	glGetProgramiv (program,                      GL_ATTACHED_SHADERS, &s->attached_shaders);GL_CHECK_ERROR;
	glGetProgramiv (program,         GL_ACTIVE_ATOMIC_COUNTER_BUFFERS, &s->active_atomic_counter_buffers);GL_CHECK_ERROR;
	glGetProgramiv (program,                     GL_ACTIVE_ATTRIBUTES, &s->active_attributes);GL_CHECK_ERROR;
	glGetProgramiv (program,           GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &s->active_attribute_max_length);GL_CHECK_ERROR;
	glGetProgramiv (program,                       GL_ACTIVE_UNIFORMS, &s->active_uniforms);GL_CHECK_ERROR;
	glGetProgramiv (program,             GL_ACTIVE_UNIFORM_MAX_LENGTH, &s->active_uniform_max_length);GL_CHECK_ERROR;
	glGetProgramiv (program,                 GL_PROGRAM_BINARY_LENGTH, &s->binary_length);GL_CHECK_ERROR;
	//glGetProgramiv (program,               GL_COMPUTE_WORK_GROUP_SIZE, &s->compute_work_group_size);GL_CHECK_ERROR;
	glGetProgramiv (program,        GL_TRANSFORM_FEEDBACK_BUFFER_MODE, &s->transform_feedback_buffer_mode);GL_CHECK_ERROR;
	glGetProgramiv (program,           GL_TRANSFORM_FEEDBACK_VARYINGS, &s->transform_feedback_varyings);GL_CHECK_ERROR;
	glGetProgramiv (program, GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH, &s->transform_feedback_varying_max_length);GL_CHECK_ERROR;
	//glGetProgramiv (program,                 GL_GEOMETRY_VERTICES_OUT, &s->geometry_vertices_out);GL_CHECK_ERROR;
	//glGetProgramiv (program,                   GL_GEOMETRY_INPUT_TYPE, &s->geometry_input_type);GL_CHECK_ERROR;
	//glGetProgramiv (program,                  GL_GEOMETRY_OUTPUT_TYPE, &s->geometry_output_type);GL_CHECK_ERROR;

}


struct GL_Shader_State
{
	GLint type;
	GLint delete_status;
	GLint compile_status;
	GLint infolog_length;
	GLint source_length;
	char * log;
};


void gl_shader_get_state (GLuint shader, struct GL_Shader_State * s)
{
	glGetShaderiv (shader, GL_SHADER_TYPE, &s->type);
	glGetShaderiv (shader, GL_DELETE_STATUS, &s->delete_status);
	glGetShaderiv (shader, GL_COMPILE_STATUS, &s->compile_status);
	glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &s->infolog_length);
	glGetShaderiv (shader, GL_SHADER_SOURCE_LENGTH, &s->source_length);
	
	GLsizei l = s->infolog_length;
	s->log = (char *) malloc (sizeof (char) * l);
	glGetShaderInfoLog (shader, l, &l, s->log);
}


void gl_print_bool (FILE * f, int n, GLint x, char const * c0, char const * c1)
{
	char const * c [2];
	c [0] = c0;
	c [1] = c1;
	ASSERT (x == 0 || x == 1);
	fprintf (f, "%s%*s " TCOL_RESET, c [x], n, gl_str_boolean (x));
}

#define GL_TCOL_SUCCESS TCOL (TCOL_NORMAL, TCOL_GREEN, TCOL_DEFAULT)
#define GL_TCOL_ERROR TCOL (TCOL_NORMAL, TCOL_RED, TCOL_DEFAULT)



void gl_shader_debug (size_t n, GLuint shaders [])
{
	fprintf (stderr, "\u251C");
	fprintf (stderr, "%8s %20s %10s %10s %10s %10s\n", "SHADER", "TYPE", "DELETE", "COMPILE", "LOGLEN", "SRCLEN");
	for (size_t i = 0; i < n; ++ i)
	{
		struct GL_Shader_State s;
		gl_shader_get_state (shaders [i], &s);
		fprintf (stderr, "\u251C");
		fprintf (stderr, "%8i ", (int) shaders [i]);
		fprintf (stderr, "%20s " , gl_str_shader_type (s.type));
		gl_print_bool (stderr, 10, s.delete_status, TCOL_RESET, TCOL_RESET);
		gl_print_bool (stderr, 10, s.compile_status, GL_TCOL_ERROR, GL_TCOL_SUCCESS);
		fprintf (stderr, "%10i ", (int) s.infolog_length);
		fprintf (stderr, "%10i ", (int) s.source_length);
		fprintf (stderr, "%s ", s.log);
		fprintf (stderr, "\n");
	}
	fflush (stderr);
}


void gl_shader_debug1 (GLuint program)
{
	GLsizei n = 10;
	GLuint shaders [10];
	glGetAttachedShaders (program, n, &n, shaders);
		
	fprintf (stderr, "\u250C");
	fprintf (stderr, "%8s %8s %20s %10s %10s %10s %10s\n", "PROGRAM", "SHADER", "TYPE", "DELETE", "COMPILE", "LOGLEN", "SRCLEN");
	for (GLsizei i = 0; i < n; ++ i)
	{
		struct GL_Shader_State s;
		gl_shader_get_state (shaders [i], &s);
		fprintf (stderr, "\u251C");
		fprintf (stderr, "%8i ", (int) program);
		fprintf (stderr, "%8i ", (int) shaders [i]);
		fprintf (stderr, "%20s " , gl_str_shader_type (s.type));
		gl_print_bool (stderr, 10, s.delete_status, TCOL_RESET, TCOL_RESET);
		gl_print_bool (stderr, 10, s.compile_status, GL_TCOL_ERROR, GL_TCOL_SUCCESS);
		fprintf (stderr, "%10i ", (int) s.infolog_length);
		fprintf (stderr, "%10i ", (int) s.source_length);
		fprintf (stderr, "%.*s", s.infolog_length, s.log);
		fprintf (stderr, "\n");
	}
	fflush (stderr);
}



//Show extra information about a GL program.
void gl_program_debug (size_t n, GLuint programs [])
{
	fprintf (stderr, "\u250C");
	fprintf (stderr, "%8s %10s %10s\n", "PROGRAM", "DEL", "LINK");
	for (size_t i = 0; i < n; ++ i)
	{
		if (!glIsProgram (programs [i])) {continue;}
		struct GL_Program_State s;
		gl_program_get_state (programs [i], &s);
		fprintf (stderr, "\u251C");
		fprintf (stderr, "%8i ", (int) programs [i]);
		gl_print_bool (stderr, 10, s.delete_status, TCOL_RESET, TCOL_RESET);
		gl_print_bool (stderr, 10, s.link_status, GL_TCOL_ERROR, GL_TCOL_SUCCESS);
		fprintf (stderr, "\n");
	}
	fprintf (stderr, "\u2514\n");
	
	for (size_t i = 0; i < n; ++ i)
	{
		if (!glIsProgram (programs [i])) {continue;}
		gl_shader_debug1 (programs [i]);
		fprintf (stderr, "\u2514\n");
	}
}


