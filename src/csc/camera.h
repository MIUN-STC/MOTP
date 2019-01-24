#pragma once

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>

#include "m4.h"
#include "v4.h"
#include "q.h"


void gl_update_projection (SDL_Window * window, float P [16])
{
	int w;
	int h;
	SDL_GetWindowSize (window, &w, &h);
	m4f32_perspective (P, 45.0f, (float)w/(float)h, 0.1f, 100.0f, M_COLMAJ);
}








struct Cam
{
	//Position (p)
	float p [4];
	
	//Looking direction (q)
	float q [4];
	
	//Projection (P) matrix
	float P [16];
	
	//Projection (P) View (V) matrix
	float PV [16];
	
	//This is used for calculating 3d coordinate from 2d coordinate.
	//Can be used for casting ray from camera pos and mouse coordinate.
	//e.t. near_xyz1 := PVinv * mouse_xy01
	//e.t. far_xyz1 := PVinv * mouse_xy11
	float PVinv [16];
	
	float PVM [16];
};


void cam_init (struct Cam * cam, SDL_Window * win)
{
	vf32_set_scalar (cam->p, 0, 4);
	m4f32_identity (cam->P);
	qf32_unit (cam->q);
	gl_update_projection (win, cam->P);
}


void cam_update (struct Cam * cam, float const p1 [4], float const q1 [4])
{
	//### Positional components.
	//Position velocity (p1) is used for moving the camera.
	//Position absolute (p) is used for setting the camera position.
	float * p = cam->p;
	
	//### Rotational components.
	//Quaternion velocity (q1) is used for rotating the camera.
	//Quaternion absolute (q) is used for settin the camera angle.
	float * q = cam->q;
	//printf ("(%f %f %f %f)\n", q1 [0], q1 [1], q1 [2], q1 [3]);
	
	//### Transformational components.
	//Translation matrix (T).
	//Rotation matrix (R).
	//Camera projection (P).
	//Camera perspective view (PV).
	float T [16];
	float R [16];
	float * P = cam->P; 
	float * PV = cam->PV;
	
	//### Rotatate the camera.
	//Quaternion (q1) is used to rotate the camera quaternion (q).
	//Rotation matrix (R) is calculated from quaternion (q).
	qf32_mul        (q, q1, q);
	qf32_normalize  (q, q);
	qf32_m4         (R, q, M_COLMAJ);
	
	//### Move camera from user input
	//1. Use coordinate system from matrix (R)
	//2. Move camera position (p) by (p1_rot) amount.
	//3. Convert camera position (p) to a translation matrix (T).
	//1. p1_rot := R^T * p1
	//2. p := p + p1_rot;
	//3. T := convert (t);
	float p1_rot [4];
	m4v4f32_mul       (p1_rot, R, p1, M_COLMAJ | M_TLEFT);
	v4f32_add         (p, p1_rot, p);
	m4f32_translation (T, p);
	
	//### Build PV matrix.
	m4f32_identity (PV);
	m4f32_mul (PV, T, PV, M_COLMAJ); //Apply translation to view matrix
	m4f32_mul (PV, R, PV, M_COLMAJ); //Apply rotation to view matrix
	m4f32_mul (PV, P, PV, M_COLMAJ); //Apply projection to view matrix
}


void cam_mvp_update (struct Cam * cam, float M [16])
{
	float * PVM = cam->PVM;
	//float PVM [16]; // ProjecitonMatrix * ViewMatrix * ModelMatrix
	float * PV = cam->PV;
	// Apply model matrix to projection*view matrix
	// PVM := PV * M;
	m4f32_mul (PVM, PV, M, M_COLMAJ);
	//m4f32_print (PVM, M_COLMAJ, stdout);
}


void cam_cast_ray (struct Cam * cam, float p [4], float n [4], float f [4])
{
	float * PV = cam->PV;
	float PVinv [16];
	m4f32_glu_inv (PVinv, PV);
	p [3] = 1.0f;
	
	p [2] = 0.0f;
	m4v4f32_mul (n, PVinv, p, M_COLMAJ);
	p [2] = 1.0f;
	m4v4f32_mul (f, PVinv, p, M_COLMAJ);
}


