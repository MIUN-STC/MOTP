#pragma once

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>

#include "debug.h"
#include "tcol.h"

#define GL_CHECK_ERROR gl_check_error (__COUNTER__, __RELATIVE_FILE__, __LINE__, __func__)


static char const * 
gl_getstr_source (GLenum source)
{
	switch (source)
	{
		case GL_DEBUG_SOURCE_API:             return "GL_DEBUG_SOURCE_API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "GL_DEBUG_SOURCE_WINDOW_SYSTEM";
		case GL_DEBUG_SOURCE_SHADER_COMPILER: return "GL_DEBUG_SOURCE_SHADER_COMPILER";
		case GL_DEBUG_SOURCE_THIRD_PARTY:     return "GL_DEBUG_SOURCE_THIRD_PARTY Party";
		case GL_DEBUG_SOURCE_APPLICATION:     return "GL_DEBUG_SOURCE_APPLICATION";
		case GL_DEBUG_SOURCE_OTHER:           return "GL_DEBUG_SOURCE_OTHER";
		default: return "UNKNOWN";
	}
}


static char const * 
gl_getstr_type (GLenum type)
{
	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:               return "GL_DEBUG_TYPE_ERROR";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR"; 
		case GL_DEBUG_TYPE_PORTABILITY:         return "GL_DEBUG_TYPE_PORTABILITY";
		case GL_DEBUG_TYPE_PERFORMANCE:         return "GL_DEBUG_TYPE_PERFORMANCE";
		case GL_DEBUG_TYPE_MARKER:              return "GL_DEBUG_TYPE_MARKER";
		case GL_DEBUG_TYPE_PUSH_GROUP:          return "GL_DEBUG_TYPE_PUSH_GROUP";
		case GL_DEBUG_TYPE_POP_GROUP:           return "GL_DEBUG_TYPE_POP_GROUP";
		case GL_DEBUG_TYPE_OTHER:               return "GL_DEBUG_TYPE_OTHER";
		default: return "UNKNOWN";
	}
}


static char const * 
gl_getstr_severity (GLenum severity)
{
	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:         return "GL_DEBUG_SEVERITY_HIGH";
		case GL_DEBUG_SEVERITY_MEDIUM:       return "GL_DEBUG_SEVERITY_MEDIUM";
		case GL_DEBUG_SEVERITY_LOW:          return "GL_DEBUG_SEVERITY_LOW";
		case GL_DEBUG_SEVERITY_NOTIFICATION: return "GL_DEBUG_SEVERITY_NOTIFICATION";
		default: return "UNKNOWN";
	}
}


static char const * 
gl_getstr_code (GLenum code)
{
	switch (code)
	{
		case GL_NO_ERROR:                      return "GL_NO_ERROR";
		case GL_INVALID_ENUM:                  return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:                 return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:             return "GL_INVALID_OPERATION";
		case GL_STACK_OVERFLOW:                return "GL_STACK_OVERFLOW";
		case GL_STACK_UNDERFLOW:               return "GL_STACK_UNDERFLOW";
		case GL_OUT_OF_MEMORY:                 return "GL_OUT_OF_MEMORY";
		case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
		default: return "Unknown error";
	}
}


static char const * 
gl_getcol_source (GLenum source)
{
	switch (source)
	{
		case GL_DEBUG_SOURCE_API:             return TCOL (TCOL_BOLD, TCOL_DEFAULT, TCOL_DEFAULT);
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return TCOL (TCOL_BOLD, TCOL_DEFAULT, TCOL_DEFAULT);
		case GL_DEBUG_SOURCE_SHADER_COMPILER: return TCOL (TCOL_BOLD, TCOL_DEFAULT, TCOL_DEFAULT);
		case GL_DEBUG_SOURCE_THIRD_PARTY:     return TCOL (TCOL_BOLD, TCOL_DEFAULT, TCOL_DEFAULT);
		case GL_DEBUG_SOURCE_APPLICATION:     return TCOL (TCOL_BOLD, TCOL_DEFAULT, TCOL_DEFAULT);
		case GL_DEBUG_SOURCE_OTHER:           return TCOL (TCOL_BOLD, TCOL_DEFAULT, TCOL_DEFAULT);
		default: return TCOL (TCOL_NORMAL, TCOL_DEFAULT, TCOL_DEFAULT);
	}
}


static char const * 
gl_getcol_type (GLenum type)
{
	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:               return TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT);
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return TCOL (TCOL_BOLD, TCOL_DEFAULT, TCOL_DEFAULT);
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return TCOL (TCOL_BOLD, TCOL_DEFAULT, TCOL_DEFAULT);
		case GL_DEBUG_TYPE_PORTABILITY:         return TCOL (TCOL_BOLD, TCOL_DEFAULT, TCOL_DEFAULT);
		case GL_DEBUG_TYPE_PERFORMANCE:         return TCOL (TCOL_BOLD, TCOL_DEFAULT, TCOL_DEFAULT);
		case GL_DEBUG_TYPE_MARKER:              return TCOL (TCOL_BOLD, TCOL_DEFAULT, TCOL_DEFAULT);
		case GL_DEBUG_TYPE_PUSH_GROUP:          return TCOL (TCOL_BOLD, TCOL_DEFAULT, TCOL_DEFAULT);
		case GL_DEBUG_TYPE_POP_GROUP:           return TCOL (TCOL_BOLD, TCOL_DEFAULT, TCOL_DEFAULT);
		case GL_DEBUG_TYPE_OTHER:               return TCOL (TCOL_BOLD, TCOL_DEFAULT, TCOL_DEFAULT);
		default: return TCOL (TCOL_NORMAL, TCOL_DEFAULT, TCOL_DEFAULT);
	}
}


static char const * 
gl_getcol_severity (GLenum severity)
{
	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:         return TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT);
		case GL_DEBUG_SEVERITY_MEDIUM:       return TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT);
		case GL_DEBUG_SEVERITY_LOW:          return TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT);
		case GL_DEBUG_SEVERITY_NOTIFICATION: return TCOL (TCOL_NORMAL, TCOL_YELLOW, TCOL_DEFAULT);
		default: return TCOL (TCOL_NORMAL, TCOL_DEFAULT, TCOL_DEFAULT);
	}
}


static char const * 
gl_getcol_code (GLenum code)
{
	switch (code)
	{
		case GL_NO_ERROR:                      return TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT);
		case GL_INVALID_ENUM:                  return TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT);
		case GL_INVALID_VALUE:                 return TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT);
		case GL_INVALID_OPERATION:             return TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT);
		case GL_STACK_OVERFLOW:                return TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT);
		case GL_STACK_UNDERFLOW:               return TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT);
		case GL_OUT_OF_MEMORY:                 return TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT);
		case GL_INVALID_FRAMEBUFFER_OPERATION: return TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT);
		default: return TCOL (TCOL_NORMAL, TCOL_DEFAULT, TCOL_DEFAULT);
	}
}


__attribute__ ((__unused__)) static void gl_check_error 
(int id, char const * file, int line, char const * fn)
{
	GLenum code = glGetError();
	if (code == GL_NO_ERROR) {return;}
	while (1)
	{
		fprintf	
		(
			stderr,
			TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT) "GL_CHECK_ERROR" TCOL_RESET " "
			"%04i "
			TCOL (TCOL_BOLD, TCOL_BLUE, TCOL_DEFAULT) "%s" TCOL_RESET ":"
			TCOL (TCOL_BOLD, TCOL_BLUE, TCOL_DEFAULT) "%04i" TCOL_RESET " in "
			TCOL (TCOL_NORMAL, TCOL_YELLOW , TCOL_DEFAULT) "%s" TCOL_RESET " () " 
			TCOL (TCOL_BOLD, TCOL_RED , TCOL_DEFAULT) "%s" TCOL_RESET " (%04i)",
			id, 
			file, 
			line, 
			fn, 
			gl_getstr_code (code),
			code
		);
		fprintf (stderr, "\n");
		code = glGetError();
		if (code == GL_NO_ERROR) {break;}
	}
	abort ();
}


void GLAPIENTRY glDebugOutput
(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, void const * arg)
{
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {return;}
	//ignore non-significant error/warning codes
	//if(id == 131169 || id == 131185 || id == 131218 || id == 131204) {return;}
	fprintf (stderr, "%i | ", id);
	fprintf (stderr, "%s%s" TCOL_RESET " | ", gl_getcol_source (source), gl_getstr_source (source));
	fprintf (stderr, "%s%s" TCOL_RESET " | ", gl_getcol_type (type), gl_getstr_type (type));
	fprintf (stderr, "%s%s" TCOL_RESET " | ", gl_getcol_severity (severity), gl_getstr_severity (severity));
	fprintf (stderr, "%s", message);
	fprintf (stderr, "\n");
	fflush (stderr);
}


