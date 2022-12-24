#pragma once

#include "jtype.h"
#include "jmacro.h"

#define vector(TYPE) vector_##TYPE
#define vector_at(TYPE) vector_at_##TYPE
#define vector_add(TYPE) vector_add_##TYPE
#define vector_rm(TYPE) vector_add_##TYPE
#define vector_resize(TYPE) vector_resize_##TYPE

#define VECTOR_DECLARE(TYPE)\
typedef struct vector(TYPE) vector(TYPE)

#define VECTOR_DECLARE_AT(TYPE) \
TYPE* vector_at(TYPE)(vector(TYPE)* v, u32 index)

#define VECTOR_DECLARE_ADD(TYPE) \
void vector_add(TYPE)(vector(TYPE)*, TYPE* data)

#define VECTOR_DECLARE_RM(TYPE) \
TYPE* vector_rm(TYPE)(vector(TYPE)*, TYPE* data)

#define VECTOR_DECLARE_RESIZE(TYPE) \
void vector_resize(TYPE)(vector(TYPE), u32 size)

#define VECTOR_DECLARE_FN__() VECTOR_DECLARE_FN_

#define VECTOR_DECLARE_FN_(TYPE, arg, ...) \
VECTOR_DECLARE_##arg(TYPE); \
__VA_OPT__(VECTOR_DECLARE_FN__ PAREN (TYPE, __VA_ARGS__))

#define VECTOR_DECLARE_FN(TYPE, ...) \
__VA_OPT__(EXPAND(VECTOR_DECLARE_FN_(TYPE, __VA_ARGS__)))

#define VECTOR_DEFINE(TYPE) \
struct vector_##TYPE \
{    \
    TYPE* data;  \
    u32 size;    \
    u32 reserve; \
}

#define VECTOR_DEFINE_AT(TYPE) \
TYPE* vector_at(TYPE)(vector(TYPE)* v, u32 index) \
{ \
	return v->data + index; \
}

// require definition of VEC_MEMCOPY
#define VECTOR_DEFINE_ADD(TYPE) \
void vector_add(TYPE)(vector(TYPE)* v, TYPE* data) \
{\
    if (v->size < v->reserve) \
        vector_resize_(TYPE)(v, v->size * 2); \
	VEC_MEMCOPY(vector_at(TYPE)(v, v->size), data, sizeof(TYPE)); \
	v->size++; \
}

#define VECTOR_DEFINE_RM(TYPE) \
TYPE* vector_rm(TYPE)(vector(TYPE)* v) \
{ \
	v->size--; \
	return vec_at(TYPE)(v, v->size); \
}

// require definition of VEC_REALLOC
// assume copying of data done in VEC_REALLOC as not always necessary, e.g. shrinking
#define VECTOR_DEFINE_RESIZE(TYPE) \
void vector_add(TYPE)(vector(TYPE)* v, u32 size) \
{\
	v->data = VEC_REALLOC(v->data, size); \
	v->reserve = size; \
}

#define VECTOR_DEFINE_FN__() VECTOR_DEFINE_FN_

#define VECTOR_DEFINE_FN_(TYPE, arg, ...) \
VECTOR_DEFINE_##arg(TYPE); \
__VA_OPT__(VECTOR_DEFINE_FN__ PAREN (TYPE, __VA_ARGS__))

#define VECTOR_DEFINE_FN(TYPE, ...) \
__VA_OPT__(EXPAND(VECTOR_DEFINE_FN_(TYPE, __VA_ARGS__)))

#define VECTOR_DECLARE_FN__() VECTOR_DECLARE_FN_

#define VECTOR_DECLARE_FN_(TYPE, arg, ...) \
VECTOR_DECLARE_##arg(TYPE); \
__VA_OPT__(VECTOR_DECLARE_FN__ PAREN (TYPE, __VA_ARGS__))

#define VECTOR_DECLARE_FN(TYPE, arg, ...) \
__VA_OPT__(EXPAND(VECTOR_DECLARE_FN_(TYPE, __VA_ARGS__)))

// -1 if a < b
// +1 if a > b
// 0 if a == b
typedef int (*pfn_cmp)(u8* a, u8* b);
typedef void (*pfn_swap)(u8* a, u8* b);

// data->size: size of each data block
// data->reserve: number of elements to sort
void vec_sort(vector(u8)* data, pfn_cmp cmp, pfn_swap swap);
void heap(vector(u8)* data, pfn_cmp cmp, pfn_swap swap);
void heap_add(vector(u8)* data, pfn_cmp cmp, pfn_swap swap);
void heap_rm(vector(u8)* data, u8* out, pfn_cmp cmp, pfn_swap swap);

VECTOR_DEFINE(i8);
VECTOR_DECLARE_FN(i8, AT, ADD, RM, RESIZE);
VECTOR_DEFINE(i16);
VECTOR_DECLARE_FN(i16, AT, ADD, RM, RESIZE);
VECTOR_DEFINE(i32);
VECTOR_DECLARE_FN(i32, AT, ADD, RM, RESIZE);
VECTOR_DEFINE(i64);
VECTOR_DECLARE_FN(i64, AT, ADD, RM, RESIZE);

VECTOR_DEFINE(u8);
VECTOR_DECLARE_FN(u8, AT, ADD, RM, RESIZE);
VECTOR_DEFINE(u16);
VECTOR_DECLARE(u16, AT, ADD, RM, RESIZE);
VECTOR_DEFINE(u32);
VECTOR_DECLARE_FN(u32, AT, ADD, RM, RESIZE);
VECTOR_DEFINE(u64);
VECTOR_DECLARE_FN(u64, AT, ADD, RM, RESIZE);

VECTOR_DEFINE(f32);
VECTOR_DECLARE_FN(f32, AT, ADD, RM, RESIZE);
VECTOR_DEFINE(f64);
VECTOR_DECLARE_FN(f64, AT, ADD, RM, RESIZE);
