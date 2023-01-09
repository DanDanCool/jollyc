#pragma once

#include "jtype.h"
#include "jmacro.h"
#include <stdatomic>

enum
{
	 ATOMIC_MUTEX = 1 << 0,
	 ATOMIC_TRANSFER = 1 << 1,
};

#define atomic(TYPE) atomic_##TYPE

#define ATOMIC_DEFINE(TYPE) typedef _Atomic TYPE atomic_##TYPE
ATOMIC_DEFINE(i8);
ATOMIC_DEFINE(i16);
ATOMIC_DEFINE(i32);
ATOMIC_DEFINE(i64);
ATOMIC_DEFINE(u8);
ATOMIC_DEFINE(u16);
ATOMIC_DEFINE(u32);
ATOMIC_DEFINE(u64);
ATOMIC_DEFINE(f32);
ATOMIC_DEFINE(f64);
#undef ATOMIC_DEFINE

#define atomic_vector(TYPE) atomic_vector_##TYPE
#define atomic_vector_init(TYPE) atomic_vector_init_##TYPE
#define atomic_vector_destroy(TYPE) atomic_vector_destroy_##TYPE
#define atomic_vector_at(TYPE) atomic_vector_at_##TYPE
#define atomic_vector_add(TYPE) atomic_vector_add_##TYPE
#define atomic_vector_rm(TYPE) atomic_vector_rm_##TYPE
#define atomic_vector_resize(TYPE) atomic_vector_resize_##TYPE

#define atomic_queue(TYPE) atomic_queue_##TYPE
#define atomic_queue_init(TYPE) atomic_queue_init_##TYPE
#define atomic_queue_destroy(TYPE) atomic_queue_destroy_##TYPE
#define atomic_queue_at(TYPE) atomic_queue_at_##TYPE
#define atomic_queue_push(TYPE) atomic_queue_push_##TYPE
#define atomic_queue_pop(TYPE) atomic_queue_pop_##TYPE

#define ATOMIC_VECTOR_DECLARE(TYPE)\
typedef struct atomic_vector(TYPE) atomic_vector(TYPE)

#define ATOMIC_VECTOR_DECLARE_INIT(TYPE) \
void atomic_vector_init(TYPE)(atomic_vector(TYPE)* v, u8* data, u32 size)

#define ATOMIC_VECTOR_DECLARE_DESTROY(TYPE) \
void atomic_vector_destroy(TYPE)(atomic_vector(TYPE)* v)

#define ATOMIC_VECTOR_DECLARE_AT(TYPE) \
TYPE* atomic_vector_at(TYPE)(atomic_vector(TYPE)* v, u32 index)

#define ATOMIC_VECTOR_DECLARE_ADD(TYPE) \
void vectora_add(TYPE)(atomic_vector(TYPE)* v, TYPE* data)

#define ATOMIC_VECTOR_DECLARE_RM(TYPE) \
TYPE* atomic_vector_rm(TYPE)(atomic_vector(TYPE)* v, TYPE* data)

#define ATOMIC_VECTOR_DECLARE_RESIZE(TYPE) \
void atomic_vector_resize(TYPE)(atomic_vector(TYPE)* v, u32 size)

#define ATOMIC_VECTOR_DECLARE_FN__() ATOMIC_VECTOR_DECLARE_FN_

#define ATOMIC_VECTOR_DECLARE_FN_(TYPE, arg, ...) \
ATOMIC_VECTOR_DECLARE_##arg(TYPE); \
__VA_OPT__(ATOMIC_VECTOR_DECLARE_FN__ PAREN (TYPE, __VA_ARGS__))

#define ATOMIC_VECTOR_DECLARE_FN(TYPE, ...) \
__VA_OPT__(EXPAND(ATOMIC_VECTOR_DECLARE_FN_(TYPE, __VA_ARGS__)))

#define ATOMIC_QUEUE_DECLARE(TYPE) \
typedef struct atomic_queue(TYPE) atomic_queue(TYPE)

#define ATOMIC_QUEUE_DECLARE_INIT(TYPE) \
void atomic_queue_init(TYPE)(atomic_queue(TYPE)* q, u8* data, u32 size)

#define ATOMIC_QUEUE_DECLARE_DESTROY(TYPE) \
void atomic_queue_destroy(TYPE)(atomic_queue(TYPE)* q)

#define ATOMIC_QUEUE_DECLARE_AT(TYPE) \
TYPE* atomic_queue_at(TYPE)(atomic_queue(TYPE)* q, TYPE* data)

#define ATOMIC_QUEUE_DECLARE_PUSH(TYPE) \
void atomic_queue_push(TYPE)(atomic_queue(TYPE)* q, TYPE* data)

#define ATOMIC_QUEUE_DECLARE_POP(TYPE) \
TYPE* atomic_queue_pop(TYPE)(atomic_queue(TYPE)* q, TYPE* data)

#define ATOMIC_QUEUE_DECLARE_FN__() ATOMIC_QUEUE_DECLARE_FN_

#define ATOMIC_QUEUE_DECLARE_FN_(TYPE, arg, ...) \
ATOMIC_QUEUE_DECLARE_##arg(TYPE); \
__VA_OPT__(ATOMIC_QUEUE_DECLARE_FN__ PAREN (TYPE, __VA_ARGS__))

#define ATOMIC_QUEUE_DECLARE_FN(TYPE, ...) \
__VA_OPT__(EXPAND(ATOMIC_QUEUE_DECLARE_FN_(TYPE, __VA_ARGS__)))

#define ATOMIC_VECTOR_DEFINE(TYPE) \
struct atomic_vector_##TYPE {    \
    TYPE* data;  \
    atomic(u32) size;    \
    atomic(u32) reserve; \
	atomic(u32) flags;
}

#define VECTOR_DEFINE_INIT(TYPE) \
void vectora_init(TYPE)(vectora(TYPE)* v, u8* data, u32 size) { \
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
