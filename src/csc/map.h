#pragma once

#include <float.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>
#include "debug.h"


void find_range_u16 
(uint16_t x, uint16_t * min, uint16_t * max)
{
	if (x > *max) {*max = x;}
	if (x < *min) {*min = x;}
}


void find_range_u16v 
(uint16_t const * x, size_t count, uint16_t * min, uint16_t * max)
{
	for (size_t i = 0; i < count; i = i + 1) 
	{
		ASSERT (x != NULL);
		find_range_u16 (x [i], min, max);
	}
	ASSERT (*max >= *min);
}


//Linearly map (x) value from (A0 .. A1) to (B0 .. B1)
//Input X has to be 32 bit because it can be scaled up temporarily.
int map_lin_u16 (uint32_t X, uint16_t A0, uint16_t A1, uint16_t B0, uint16_t B1)
{
	ASSERT (X <= UINT16_MAX);
	//Crop
	if (X < A0) {return B0;}
	if (X > A1) {return B1;}
	int DA;
	int DB;
	//Delta
	DA = A1 - A0;
	DB = B1 - B0;
	//Accurate integer division.
	//Round up or down
	if (DA > DB) {DA = DA + 1; DB = DB + 1;}
	//X = (X-A0) * ((DB + K) / (DA + K)) + B0
	//Move to zero.
	X = X - A0;
	//Scale up before scaling down is important for integers.
	ASSERT ((uint64_t) (X * DB) <= UINT32_MAX);
	X = X * DB;
	//Zero division protection.				
	if (DA == 0) {return B1;};
	//Scale down
	X = X / DA;
	//Apply offset 
	X = X + B0;
	ASSERT (X <= B1);
	ASSERT (X >= B0);
	return X;
}


void map_lin_u16v (uint16_t const * src, uint16_t * des, size_t count, uint16_t A0, uint16_t A1, uint16_t B0, uint16_t B1)
{
	for (size_t i = 0; i < count; i = i + 1) 
	{
		des [i] = map_lin_u16 (src [i], A0, A1, B0, B1);
	}
}


//Linearly map (x) value from (A0 .. A1) to (B0 .. B1)
//Input X can get Integer overflow.
int map_lin_int (int X, int A0, int A1, int B0, int B1)
{
	//Crop
	if (X < A0) {return B0;}
	if (X > A1) {return B1;}
	int DA;
	int DB;
	//Delta
	DA = A1 - A0;
	DB = B1 - B0;
	//Accurate integer division.
	//Round up or down
	if (DA > DB) {DA = DA + 1; DB = DB + 1;}
	//X = (X-A0) * ((DB + K) / (DA + K)) + B0
	//Move to zero.
	X = X - A0;
	//Scale up before scaling down is important for integers.
	X = X * DB;
	//Zero division protection.				
	if (DA == 0) {return B1;};
	//Scale down
	X = X / DA;
	//Apply offset 
	X = X + B0;
	ASSERT (X <= B1);
	ASSERT (X >= B0);
	return X;
}


//Linearly map (x) value from (A0 .. A1) to (B0 .. B1)
float map_lin_float (float X, float A0, float A1, float B0, float B1)
{
	//cropping
	if (X < A0) {return B0;}
	if (X > A1) {return B1;}
	//calculate delta
	float DA;
	float DB;
	DA = A1 - A0;
	DB = B1 - B0;
	//move to zero
	X = X - A0;
	//zero division protection
	if (DA == 0) {return B1;};
	X = X / DA;
	//new scale
	X = X * DB;
	//new offset
	X = X + B0;
	return X;
}


//Histogram
void count_u8_u16 (uint8_t * src, size_t src_count, uint16_t * des, size_t des_count)
{
	memset (des, 0, des_count * sizeof (uint16_t));
	for (size_t I = 0; I < src_count; I = I + 1)
	{
		ASSERT (src [I] < des_count);
		if (des [src [I]] == UINT16_MAX) {continue;}
		des [src [I]] ++;
	}	
}
