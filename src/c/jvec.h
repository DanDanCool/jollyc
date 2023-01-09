#pragma once

#include "jtype.h"
#include "jmacro.h"

#define vector(TYPE) vector_##TYPE
#define vector_init(TYPE) vector_init_##TYPE
#define vector_destroy(TYPE) vector_destroy_##TYPE
#define vector_at(TYPE) vector_at_##TYPE
#define vector_add(TYPE) vector_add_##TYPE
#define vector_rm(TYPE) vector_rm_##TYPE
#define vector_resize(TYPE) vector_resize_##TYPE

#define queue(TYPE) queue_##TYPE
#define queue_init(TYPE) queue_init_##TYPE
#define queue_destroy(TYPE) queue_destroy_##TYPE
#define queue_at(TYPE) queue_at_##TYPE
#define queue_push(TYPE) queue_push_##TYPE
#define queue_pop(TYPE) queue_pop_##TYPE

#define VECTOR_DECLARE(TYPE)\
typedef struct vector(TYPE) vector(TYPE)

#define VECTOR_DECLARE_INIT(TYPE) \
void vector_init(TYPE)(vector(TYPE)* v, u8* data, u32 size)

#define VECTOR_DECLARE_DESTROY(TYPE) \
void vector_destroy(TYPE)(vector(TYPE)* v)

#define VECTOR_DECLARE_AT(TYPE) \
TYPE* vector_at(TYPE)(vector(TYPE)* v, u32 index)

#define VECTOR_DECLARE_ADD(TYPE) \
void vector_add(TYPE)(vector(TYPE)* v, TYPE* data)

#define VECTOR_DECLARE_RM(TYPE) \
TYPE* vector_rm(TYPE)(vector(TYPE)* v, TYPE* data)

#define VECTOR_DECLARE_RESIZE(TYPE) \
void vector_resize(TYPE)(vector(TYPE)* v, u32 size)

#define VECTOR_DECLARE_FN__() VECTOR_DECLARE_FN_

#define VECTOR_DECLARE_FN_(TYPE, arg, ...) \
VECTOR_DECLARE_##arg(TYPE); \
__VA_OPT__(VECTOR_DECLARE_FN__ PAREN (TYPE, __VA_ARGS__))

#define VECTOR_DECLARE_FN(TYPE, ...) \
__VA_OPT__(EXPAND(VECTOR_DECLARE_FN_(TYPE, __VA_ARGS__)))

#define QUEUE_DECLARE(TYPE) \
typedef struct queue(TYPE) queue(TYPE)

#define QUEUE_DECLARE_INIT(TYPE) \
void queue_init(TYPE)(queue(TYPE)* q, u8* data, u32 size)

#define QUEUE_DECLARE_DESTROY(TYPE) \
void queue_destroy(TYPE)(queue(TYPE)* q)

#define QUEUE_DECLARE_AT(TYPE) \
TYPE* queue_at(TYPE)(queue(TYPE)* q, TYPE* data)

#define QUEUE_DECLARE_PUSH(TYPE) \
void queue_push(TYPE)(queue(TYPE)* q, TYPE* data)

#define QUEUE_DECLARE_POP(TYPE) \
TYPE* queue_pop(TYPE)(queue(TYPE)* q, TYPE* data)

#define QUEUE_DECLARE_FN__() QUEUE_DECLARE_FN_

#define QUEUE_DECLARE_FN_(TYPE, arg, ...) \
QUEUE_DECLARE_##arg(TYPE); \
__VA_OPT__(QUEUE_DECLARE_FN__ PAREN (TYPE, __VA_ARGS__))

#define QUEUE_DECLARE_FN(TYPE, ...) \
__VA_OPT__(EXPAND(QUEUE_DECLARE_FN_(TYPE, __VA_ARGS__)))

#define VECTOR_DEFINE(TYPE) \
struct vector_##TYPE {    \
    TYPE* data;  \
    u32 size;    \
    u32 reserve; \
}

#define VECTOR_DEFINE_INIT(TYPE) \
void vector_init(TYPE)(vector(TYPE)* v, u8* data, u32 size) { \
	*v = {}; \
	v->data = (TYPE*)data; \
	v->reserve = size; \
}

#define VECTOR_DEFINE_DESTROY(TYPE) \
void vector_destroy(TYPE)(vector(TYPE)* v) { \
	free(v->data); \
	*v = {}; \
}

#define VECTOR_DEFINE_AT(TYPE) \
TYPE* vector_at(TYPE)(vector(TYPE)* v, u32 index) { \
	return v->data + index; \
}

// require definition of VEC_MEMCOPY, vector_resize
#define VECTOR_DEFINE_ADD(TYPE) \
void vector_add(TYPE)(vector(TYPE)* v, TYPE* data) {\
    if (v->reserve <= v->size) \
        vector_resize(TYPE)(v, v->size * 2); \
	VEC_MEMCPY((u8*)vector_at(TYPE)(v, v->size), (u8*)data, sizeof(TYPE)); \
	v->size++; \
}

#define VECTOR_DEFINE_RM(TYPE) \
TYPE* vector_rm(TYPE)(vector(TYPE)* v) { \
	v->size--; \
	return vec_at(TYPE)(v, v->size); \
}

// require definition of VEC_ALLOC and VEC_CPY
// always copies data
#define VECTOR_DEFINE_RESIZE(TYPE) \
void vector_resize(TYPE)(vector(TYPE)* v, u32 size) {\
	vector(u8) dst = { VEC_ALLOC(size * sizeof(TYPE)), sizeof(TYPE), size }; \
	vector(u8) src = { (u8*)v->data, sizeof(mem_block), v->size }; \
	VEC_CPY(&dst, &src); \
	free(v->data); \
	v->data = (TYPE*)dst.data; \
	v->reserve = size; \
}

#define QUEUE_DEFINE(TYPE) \
struct queue_##TYPE { \
	vector(TYPE) data; \
	u32 beg; \
	u32 end; \
}

#define QUEUE_DEFINE_INIT(TYPE) \
void queue_init(TYPE)(queue(TYPE)* q, u8* data, u32 size) { \
	*q = {}; \
	q->data.data = (TYPE*)data; \
	q->data.reserve = size; \
}

#define QUEUE_DEFINE_DESTROY(TYPE) \
void queue_destroy(TYPE)(queue(TYPE)* q) { \
	vector_destroy(TYPE)(&q->data); \
	*q = {}; \
}

#define QUEUE_DEFINE_AT(TYPE) \
TYPE* queue_at(TYPE)(queue(TYPE)* q, u32 index) { \
	return vector_at(TYPE)(&q->data, index); \
}

// see requirements for vector_add
#define QUEUE_DEFINE_PUSH(TYPE) \
void queue_push(TYPE)(queue(TYPE)* q, TYPE* data) { \
	if (q->data.reserve <= q->data.size) \
		vector_resize(TYPE)(&q->data, q->data.size * 2); \
	VEC_MEMCPY((u8*)queue_at(TYPE)(q, q->end), (u8*)data, sizeof(TYPE)); \
	q->data.size++; \
	q->end++; \
}

// see requirements for vector_rm
#define QUEUE_DEFINE_POP(TYPE) \
TYPE* queue_pop(TYPE)(queue(TYPE)* q) { \
	assert(q->begin <= q->end); \
	u32 tmp = q->begin++; \
	return queue_at(TYPE)(q, tmp); \
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

typedef struct sort_params sort_params;

// data.size: size of each data block
// data.reserve: number of elements to sort
// actual reserved space should always be odd so heap operations do not access oob memory
// comparisons with null value should be valid
struct sort_params
{
	vector(u8) data;
	u8* null_value;
	pfn_cmp cmp;
	pfn_swap swap;
	u16 max_depth
	u16 insertion_thresh;
	u16 depth;
};

void vector_sort(sort_params* params);
void heap(sort_params* params);
void heap_add(sort_params* params); // assumes new data already appended to end of array
void heap_del(sort_params* params, u32 i);
void heap_rm(sort_params* params); // will set root to last element, remove with pop()

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
