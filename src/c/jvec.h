#pragma once

#include "jtype.h"
#include "jmacro.h"

#define vector(TYPE) vector
#define vector_init(TYPE) vector_init_##TYPE
void vector_destroy(vector* v);
#define vector_at(TYPE) vector_at_##TYPE
#define vector_get(TYPE) vector_get_##TYPE
#define vector_set(TYPE) vector_set_##TYPE
#define vector_add(TYPE) vector_add_##TYPE
#define vector_del(TYPE) vector_rm_##TYPE
void vector_resize(vector* v, u32 size);

#define queue(TYPE) queue
#define queue_init(TYPE) queue_init_##TYPE
#define queue_destroy(TYPE) queue_destroy_##TYPE
#define queue_at(TYPE) queue_at_##TYPE
#define queue_add(TYPE) queue_add_##TYPE
#define queue_del(TYPE) queue_del_##TYPE

#define VECTOR_DECLARE_INIT(TYPE) \
void vector_init(TYPE)(vector* v, u8* data)

#define VECTOR_DECLARE_AT(TYPE) \
TYPE* vector_at(TYPE)(vector* v, u32 index)

#define VECTOR_DECLARE_GET(TYPE) \
void vector_get(TYPE)(vector* v, u32 index, TYPE* out)

#define VECTOR_DECLARE_SET(TYPE) \
void vector_set(TYPE)(vector* v, u32 index, TYPE* in)

#define VECTOR_DECLARE_ADD(TYPE) \
void vector_add(TYPE)(vector* v, TYPE* data)

#define VECTOR_DECLARE_DEL(TYPE) \
TYPE* vector_del(TYPE)(vector* v, u32 index)

#define VECTOR_DECLARE_FN(TYPE, ...) \
DECLARE_FN(VECTOR_DECLARE, TYPE, __VA_ARGS__)

#define QUEUE_DECLARE_INIT(TYPE) \
void queue_init(TYPE)(queue* q, u8* data, u32 size)

#define QUEUE_DECLARE_AT(TYPE) \
TYPE* queue_at(TYPE)(queue* q, TYPE* data)

#define QUEUE_DECLARE_ADD(TYPE) \
void queue_add(TYPE)(queue* q, TYPE* data)

#define QUEUE_DECLARE_DEL(TYPE) \
TYPE* queue_del(TYPE)(queue* q, TYPE* data)

#define QUEUE_DECLARE_FN(TYPE, ...) \
DECLARE_FN(QUEUE_DECLARE, TYPE, __VA_ARGS__)

typedef struct vector vector;
struct vector {
    u8* data;
	u32 size;
	u32 reserve;
}

#define VECTOR_DEFINE_INIT(TYPE) \
void vector_init(TYPE)(vector* v, u8* data) { \
	v->data = data; \
	mem_header* header = mem_getheader(data); \
	v->reserve = header->size / sizeof(TYPE); \
	v->size = 0; \
}

#define VECTOR_DEFINE_AT(TYPE) \
TYPE* vector_at(TYPE)(vector* v, u32 index) { \
	return (TYPE*)v->data + index; \
}

#define VECTOR_DEFINE_GET(TYPE) \
void vector_get(TYPE)(vector* v, u32 index, TYPE* out) { \
	copy(TYPE)(out, vector_at(TYPE)(v, index)); \
}

#define VECTOR_DEFINE_SET(TYPE) \
void vector_set(TYPE)(vector* v, u32 index, TYPE* in) { \
	copy(TYPE)(vector_at(TYPE)(v, index), out); \
}

#define VECTOR_DEFINE_ADD(TYPE) \
void vector_add(TYPE)(vector* v, TYPE* data) {\
    if (v->reserve <= v->size) \
        vector_resize(TYPE)(v, size * 2); \
	vector_set(TYPE)(v, v->size, data); \
	v->size++; \
}

#define VECTOR_DEFINE_DEL(TYPE) \
TYPE* vector_del(TYPE)(vector* v, u32 index) { \
	v->size--; \
	TYPE tmp; \
	vector_get(v, index, &tmp); \
	vector_set(v, index, vector_at(TYPE)(v, v->size)); \
	vector_set(v, v->size, &tmp); \
}

#define VECTOR_DEFINE_FN(TYPE, ...) \
DECLARE_FN(VECTOR_DEFINE, TYPE, __VA_ARGS__)

typedef struct queue queue;
struct queue
{
	vector data;
	u32 beg;
	u32 end;
}

#define QUEUE_DEFINE_INIT(TYPE) \
void queue_init(TYPE)(queue* q, u8* data) { \
	*q = {}; \
	vector_init(TYPE)(&q->data, data); \
}

#define QUEUE_DEFINE_AT(TYPE) \
TYPE* queue_at(TYPE)(queue* q, u32 index) { \
	return vector_at(TYPE)(&q->data, index); \
}

#define QUEUE_DEFINE_ADD(TYPE) \
void queue_add(TYPE)(queue* q, TYPE* data) { \
	if (q->data.reserve <= q->data.size) { \
		vector_resize(&q->data, q->data.size * 2); \
		q->end = q->data.size; \
	} \
	vector_set(TYPE)(&q->data, q->end, data); \
	q->data.size++; \
}

#define QUEUE_DEFINE_DEL(TYPE) \
TYPE* queue_del(TYPE)(queue* q) { \
	assert(q->begin <= q->end); \
	q->data.size--; \
	u32 tmp = q->begin++; \
	return queue_at(TYPE)(q, tmp); \
}

#define QUEUE_DEFINE_FN(TYPE, ...) \
DECLARE_FN(QUEUE_DEFINE, TYPE, __VA_ARGS__)

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

// rewrite, switching to macro based type templating
void vector_sort(sort_params* params);
void heap(sort_params* params);
void heap_add(sort_params* params); // assumes new data already appended to end of array
void heap_del(sort_params* params, u32 i);
void heap_rm(sort_params* params); // will set root to last element, remove with pop()
void heap_replace(sort_params* params);

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
