#pragma once


//The name "COUNTOF" is like the name "sizeof" but with COUNT instead.
#define COUNTOF(x) (sizeof(x) / sizeof(*(x)))

//TODO add CROP
#ifndef MIN
#define MIN(a, b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a, b) (((a)>(b))?(a):(b))
#endif

//typeof is a GNU extension
#define SWAP(x, y) do { typeof(x) SWAP = x; x = y; y = SWAP; } while (0)


//Loop n times
//Loop permutate n times
//These macros might be removed.
#define LOOP(t, i, n) for (t (i) = 0; (i) < (n); (i) = (i) + 1)
#define LOOP_AB(t, i, a, b) for (t (i) = (a); (i) < (b); (i) = (i) + 1)
#define LOOPN(t, i, n) for (t (i) = 0; (i) < (n); (i) = (i) + 1)
#define LOOPP(t, i1, i2, n) \
for (t (i1) = 0;      (i1) < (n); (i1) = (i1) + 1) \
for (t (i2) = i1 + 1; (i2) < (n); (i2) = (i2) + 1)


//https://stackoverflow.com/questions/9907160/how-to-convert-enum-names-to-string-in-c
#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,
