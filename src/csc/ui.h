#pragma once

#include <SDL2/SDL.h>
#include "debug.h"
#include "v.h"
#include "q.h"


void ui_mouse (float x [2], SDL_Window * window, SDL_MouseButtonEvent const * m)
{
	int w;
	int h;
	SDL_GetWindowSize (window, &w, &h);
	x [0] = ((float)m->x/(float)w)*2.0f - 1.0f;
	x [1] = ((float)m->y/(float)w)*2.0f - 1.0f;
	x [1] *= -1.0f;
}


void ui_mouse_quaternion (SDL_MouseMotionEvent const * m, float q1 [4])
{
	float z1 [4];
	z1 [0] = m->yrel;
	z1 [1] = m->xrel;
	z1 [2] = 0;
	//Set angular speed in 4th dimension.
	z1 [3] = 0.001f;
	
	//smooth_mouse (app->dt, z1, a1);
	
	//q1 := convert (z1)
	qf32_axis_angle (q1, z1);
	//q1 := q1 / |q1|
	qf32_normalize  (q1, q1);
	
	
	//printf ("(%f %f %f %f) ", z1 [0], z1 [1], z1 [2], z1 [3]);
	//printf ("(%f %f %f %f) ", x1 [0], x1 [1], x1 [2], x1 [3]);
	//printf ("(%f %f %f %f)\n", q1 [0], q1 [1], q1 [2], q1 [3]);
	//printf ("(%i %i)\n", m->x, m->y);
	//fflush (stdout);
}


void ui_mouse_position1 (SDL_Window * window, float r [2])
{
	int w;
	int h;
	SDL_GetWindowSize (window, &w, &h);
	int x;
	int y;
	SDL_GetMouseState (&x, &y);
	r [0] = ((float)x/(float)w)*2.0f - 1.0f;
	r [1] = ((float)y/(float)h)*2.0f - 1.0f;
	r [1] = -r [1];
	//printf ("(%f %f)\n", (float)x/(float)w - 0.5f, (float)y/(float)h - 0.5f);
}


void sdl_get_rotation_vector (float a [4], uint8_t const * keyboard)
{
	float yaw_left   = keyboard [SDL_SCANCODE_LEFT];
	float yaw_right  = keyboard [SDL_SCANCODE_RIGHT];
	float pitch_down = keyboard [SDL_SCANCODE_DOWN];
	float pitch_up   = keyboard [SDL_SCANCODE_UP];
	float roll_left  = keyboard [SDL_SCANCODE_Q];
	float roll_right = keyboard [SDL_SCANCODE_E];
	
	a [0] = pitch_down - pitch_up;
	a [1] = yaw_right  - yaw_left;
	a [2] = roll_right - roll_left;
	a [3] = 0.01f;
	
	qf32_axis_angle (a, a);
	qf32_normalize (a, a);
}


void sdl_get_translation_vector (float t [4], uint8_t const * keyboard)
{
	float left     = keyboard [SDL_SCANCODE_A];
	float right    = keyboard [SDL_SCANCODE_D];
	float down     = keyboard [SDL_SCANCODE_LCTRL];
	float up       = keyboard [SDL_SCANCODE_SPACE];
	float forward  = keyboard [SDL_SCANCODE_W];
	float backward = keyboard [SDL_SCANCODE_S];
	float faster   = keyboard [SDL_SCANCODE_LSHIFT];
	
	t [0] = left    - right;
	t [1] = down    - up;
	t [2] = forward - backward;
	t [3] = 0;
	
	v4f32_normalize (t, t);
	v4f32_mul_scalar (t, t, 0.01f * (50.0f * faster +1));
	
	//printf ("(%f %f %f %f)\n", t [0], t [1], t [2], t [3]);
}
