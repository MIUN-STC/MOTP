#pragma once


void * memdup (const void * mem, size_t size8)
{ 
	void * out = malloc (size8);
	if (out != NULL)
	{
		memcpy (out, mem, size8);
	}
	return out;
}
