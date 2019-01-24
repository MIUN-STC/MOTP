#pragma once

#include <math.h>

#include "m.h"


//Translation vector
//Independent from row/col major 
#define M4_TX 12
#define M4_T0 12
#define M4_TY 13
#define M4_T1 13
#define M4_TZ 14
#define M4_T2 14
#define M4_TW 15
#define M4_T3 15


//Column vectors
//Independent from row/col major 
#define M4_V0 0
#define M4_VX 0
#define M4_V1 4
#define M4_VY 4
#define M4_V2 8
#define M4_VZ 8
#define M4_V3 12
#define M4_VT 12


//Scale scalars
//Independent from row/col major 
#define M4_S0 0
#define M4_S1 5
#define M4_S2 10
#define M4_S3 15


void m4f32_print (float M [16], uint8_t flags, FILE * f)
{
	mf32_print (M, 4, 4, flags, f);
}



void m4f32_set (float M [16], float c)
{
	mf32_set (M, c, 16);
}



// 4x4 matrix identity
// M := I
void m4f32_identity (float M [16])
{
	m4f32_set (M, 0);
	M [M4_S0] = 1.0f;
	M [M4_S1] = 1.0f;
	M [M4_S2] = 1.0f;
	M [M4_S3] = 1.0f;
}


void m4f32_translation (float M [16], float const t [4])
{
	m4f32_identity (M);
	M [M4_T0 + 0] = t [0]; 
	M [M4_T0 + 1] = t [1]; 
	M [M4_T0 + 2] = t [2];
	//Translate 4th dimension?
	//m [M4_T0 + 3] = t [3];
}


// 4x4 only matrix multiplication
// R := A * B
void m4f32_mul (float R [16], float const A [16], float const B [16], uint8_t flags)
{
	//Temporary output is requried if R uses the same memory as A or B.
	float T [16];
	mf32_mulcpy (T, A, B, 4, 4, 4, flags);
	memcpy (R, T, sizeof (float) * 16);
}


// 4x4 * 4x1 multiplication
// r := A * b
void m4v4f32_mul (float r [4], float const A [16], float const b [4], uint8_t flags)
{
	//Temporary output is requried if R uses the same memory as A.
	float t [4];
	mf32_mulcpy (t, A, b, 4, 4, 1, flags);
	memcpy (r, t, sizeof (float) * 4);
}





void m4f32_frustum 
(float M [16], float l, float r, float b, float t, float n, float f, uint8_t flags)
{
	m4f32_set (M, 0);
	M [0] = (2 * n) / (r - l);
	M [5] = (2 * n) / (t - b);
	M [8] = (r + l) / (r - l);
	M [9] = (t + b) / (t - b);
	M [10] = (-f - n) / (f - n);
	M [11] = -1;
	M [14] = (-2 * f * n) / (f - n);
}


void m4f32_perspective 
(float M [16], float fov1, float aspect1, float near1, float far1, uint8_t flags)
{
	float tangent = tanf (((float)M_PI/180.0f) * (fov1 / 2.0f));
	float height = near1 * tangent;
	float width = height * aspect1;
	m4f32_frustum (M, -width, width, -height, height, near1, far1, flags);
}


void m4f32_ortho
()
{

}


// 4x4 matrix inversion (glu version)
// R := M^(-1)
float m4f32_glu_inv (float R [16], float const M [16])
{
	float I [16];

	I[0] = 
	M[5]  * M[10] * M[15] - 
	M[5]  * M[11] * M[14] - 
	M[9]  * M[6]  * M[15] + 
	M[9]  * M[7]  * M[14] +
	M[13] * M[6]  * M[11] - 
	M[13] * M[7]  * M[10];

	I[4] = 
	-M[4]  * M[10] * M[15] + 
	M[4]  * M[11] * M[14] + 
	M[8]  * M[6]  * M[15] - 
	M[8]  * M[7]  * M[14] - 
	M[12] * M[6]  * M[11] + 
	M[12] * M[7]  * M[10];

	I[8] = 
	M[4]  * M[9] * M[15] - 
	M[4]  * M[11] * M[13] - 
	M[8]  * M[5] * M[15] + 
	M[8]  * M[7] * M[13] + 
	M[12] * M[5] * M[11] - 
	M[12] * M[7] * M[9];

	I[12] = 
	-M[4]  * M[9] * M[14] + 
	M[4]  * M[10] * M[13] +
	M[8]  * M[5] * M[14] - 
	M[8]  * M[6] * M[13] - 
	M[12] * M[5] * M[10] + 
	M[12] * M[6] * M[9];

	I[1] = 
	-M[1]  * M[10] * M[15] + 
	M[1]  * M[11] * M[14] + 
	M[9]  * M[2] * M[15] - 
	M[9]  * M[3] * M[14] - 
	M[13] * M[2] * M[11] + 
	M[13] * M[3] * M[10];

	I[5] = 
	M[0]  * M[10] * M[15] - 
	M[0]  * M[11] * M[14] - 
	M[8]  * M[2] * M[15] + 
	M[8]  * M[3] * M[14] + 
	M[12] * M[2] * M[11] - 
	M[12] * M[3] * M[10];

	I[9] = 
	-M[0]  * M[9] * M[15] + 
	M[0]  * M[11] * M[13] + 
	M[8]  * M[1] * M[15] - 
	M[8]  * M[3] * M[13] - 
	M[12] * M[1] * M[11] + 
	M[12] * M[3] * M[9];

	I[13] = 
	M[0]  * M[9] * M[14] - 
	M[0]  * M[10] * M[13] - 
	M[8]  * M[1] * M[14] + 
	M[8]  * M[2] * M[13] + 
	M[12] * M[1] * M[10] - 
	M[12] * M[2] * M[9];

	I[2] = 
	M[1]  * M[6] * M[15] - 
	M[1]  * M[7] * M[14] - 
	M[5]  * M[2] * M[15] + 
	M[5]  * M[3] * M[14] + 
	M[13] * M[2] * M[7] - 
	M[13] * M[3] * M[6];

	I[6] = 
	-M[0]  * M[6] * M[15] + 
	M[0]  * M[7] * M[14] + 
	M[4]  * M[2] * M[15] - 
	M[4]  * M[3] * M[14] - 
	M[12] * M[2] * M[7] + 
	M[12] * M[3] * M[6];

	I[10] = 
	M[0]  * M[5] * M[15] - 
	M[0]  * M[7] * M[13] - 
	M[4]  * M[1] * M[15] + 
	M[4]  * M[3] * M[13] + 
	M[12] * M[1] * M[7] - 
	M[12] * M[3] * M[5];

	I[14] = 
	-M[0]  * M[5] * M[14] + 
	M[0]  * M[6] * M[13] + 
	M[4]  * M[1] * M[14] - 
	M[4]  * M[2] * M[13] - 
	M[12] * M[1] * M[6] + 
	M[12] * M[2] * M[5];

	I[3] = 
	-M[1] * M[6] * M[11] + 
	M[1] * M[7] * M[10] + 
	M[5] * M[2] * M[11] - 
	M[5] * M[3] * M[10] - 
	M[9] * M[2] * M[7] + 
	M[9] * M[3] * M[6];

	I[7] = 
	M[0] * M[6] * M[11] - 
	M[0] * M[7] * M[10] - 
	M[4] * M[2] * M[11] + 
	M[4] * M[3] * M[10] + 
	M[8] * M[2] * M[7] - 
	M[8] * M[3] * M[6];

	I[11] = 
	-M[0] * M[5] * M[11] + 
	M[0] * M[7] * M[9] + 
	M[4] * M[1] * M[11] - 
	M[4] * M[3] * M[9] - 
	M[8] * M[1] * M[7] + 
	M[8] * M[3] * M[5];

	I[15] = 
	M[0] * M[5] * M[10] - 
	M[0] * M[6] * M[9] - 
	M[4] * M[1] * M[10] + 
	M[4] * M[2] * M[9] + 
	M[8] * M[1] * M[6] - 
	M[8] * M[2] * M[5];

	float det = 
	M[0] * I[0] + 
	M[1] * I[4] + 
	M[2] * I[8] + 
	M[3] * I[12];

	if (det == 0) {return det;}

	det = 1.0f / det;

	for (size_t i = 0; i < 16; ++ i)
	{
		R [i] = I [i] * det;
	}
	
	return det;
}

