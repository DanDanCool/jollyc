#pragma once

#include "jtype.h"

#define ALIGN(x) __attribute__ ((aligned(x)))

typedef struct
{
	float x;
	float y;
	float z;
	float w;
} vec4 ALIGN(16);

typedef struct
{
	uint32 x;
	uint32 y;
	uint32 z;
	uint32 w;
} vec4i ALIGN(16);

vec4 vec4_mul(vec4* v1, vec4* v2);
vec4 vec4_div(vec4* v1, vec4* v2);
