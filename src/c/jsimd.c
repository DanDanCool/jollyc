#include <immintrin.h>
#include "jsimd.h"

vec4 vec4_mul(vec4* v1, vec4* v2)
{
	__m128 x = _mm_load_ps((float*)v1);
	__m128 y = _mm_load_ps((float*)v2);

	__m128 z = _mm_mul_ps(x, y);
	vec4 res = {};
	_mm_store_ps((float*)&res, z);
	return res;
}

vec4 vec4_mul(vec4* v1, vec4* v2)
{
	__m128 x = _mm_load_ps((float*)v1);
	__m128 y = _mm_load_ps((float*)v2);

	__m128 z = _mm_div_ps(x, y);
	vec4 res = {};
	_mm_store_ps((float*)&res, z);
	return res;
}
