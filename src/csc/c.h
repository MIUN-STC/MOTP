#pragma once


struct v4f32_RGBA
{
	float r;
	float g;
	float b;
	float a;
};

struct v4f32_RGBA v4f32_RGBA (float r, float g, float b, float a)
{
	struct v4f32_RGBA c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;
	return c;
}


void rgb_from_hsv (float rgb [3], float const hsv [3])
{
	float H = hsv [0];
	float S = hsv [1];
	float V = hsv [2];
	float P;
	float Q;
	float T;
	float fract;

	if (H == 360.0f) {H = 0;}
	else             {H /= 60.0f;}
    fract = H - floorf (H);
    P = V * (1.0f - S);
    Q = V * (1.0f - S * fract);
    T = V * (1.0f - S * (1.0f - fract));

	if (0) {}
    else if (0.0f <= H && H < 1.0f) {rgb [0] = V; rgb [1] = T; rgb [2] = P;}
    else if (1.0f <= H && H < 2.0f) {rgb [0] = Q; rgb [1] = V; rgb [2] = P;}
    else if (2.0f <= H && H < 3.0f) {rgb [0] = P; rgb [1] = V; rgb [2] = T;}
    else if (3.0f <= H && H < 4.0f) {rgb [0] = P; rgb [1] = Q; rgb [2] = V;}
    else if (4.0f <= H && H < 5.0f) {rgb [0] = T; rgb [1] = P; rgb [2] = V;}
    else if (5.0f <= H && H < 6.0f) {rgb [0] = V; rgb [1] = P; rgb [2] = Q;}
    else {rgb [0] = 0.0f; rgb [1] = 0.0f; rgb [2] = 0.0f;}
}


void rgb_random (float c [3], unsigned int seed)
{
	//TRACE_F ("%li", seed);
	srand (seed);
	//TRACE_F ("%li %f %f %f", seed, rand (), rand (), rand ());
	c [0] = (float)rand () / (float)RAND_MAX;
	c [1] = (float)rand () / (float)RAND_MAX;
	c [2] = (float)rand () / (float)RAND_MAX;
}
