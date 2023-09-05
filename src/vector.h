#pragma once

#include "macros.h"
#include "types.h"
#include "operations.h"

typedef struct vector vector_;
struct vector {
	u8* data;
	u32 reserve;
	u32 size;
};

void vector_create_(vector_* v, u32 bytes);
void vector_resize_(vector_* v, u32 bytes);
void vector_destroy_(vector_* v);

void heap_siftup_(vector_* v, u32 idx, pfn_swap swapfn, pfn_le lefn, u32 keysize);
void heap_siftdown_(vector_* v, u32 idx, pfn_swap swapfn, pfn_lt ltfn, u32 keysize);

#define vector(type) vector_
#define vector_create(TYPE) vector_create_##TYPE
#define vector_destroy(TYPE) vector_destroy_
#define vector_resize(TYPE) vector_resize_##TYPE
#define vector_at(TYPE) vector_at_##TYPE
#define vector_get(TYPE) vector_get_##TYPE
#define vector_set(TYPE) vector_set_##TYPE
#define vector_add(TYPE) vector_add_##TYPE
#define vector_del(TYPE) vector_rm_##TYPE

#define pair(a, b) pair_##a##_##b

#define PAIR_DECLARE(a, b) \
typedef struct pair(a, b) pair(a, b); \
struct pair(a, b) { \
	a first; \
	b second; \
}

// min heap
#define heap(type) heap_##type
#define heap_add(type) heap_add_##type
#define heap_del(type) heap_del_##type
#define heap_replace(type) heap_replace_##type

#define VECTOR_DECLARE_CREATE(TYPE) \
void vector_create(TYPE)(vector_* v, u32 size)

#define VECTOR_DECLARE_RESIZE(TYPE) \
void vector_resize(TYPE)(vector_* v, u32 size)

#define VECTOR_DECLARE_AT(TYPE) \
TYPE* vector_at(TYPE)(vector_* v, u32 idx)

#define VECTOR_DECLARE_GET(TYPE) \
void vector_get(TYPE)(vector_* v, TYPE* data, u32 idx)

#define VECTOR_DECLARE_SET(TYPE) \
void vector_set(TYPE)(vector_* v, TYPE* data, u32 idx)

#define VECTOR_DECLARE_ADD(TYPE) \
void vector_add(TYPE)(vector_* v, TYPE* data)

#define VECTOR_DECLARE_DEL(TYPE) \
TYPE* vector_del(TYPE)(vector_* v, u32 idx)

#define VECTOR_DECLARE(TYPE) \
EXPAND4(DEFER(VECTOR_DECLARE_CREATE)(TYPE); \
        DEFER(VECTOR_DECLARE_RESIZE)(TYPE); \
        DEFER(VECTOR_DECLARE_AT)(TYPE); \
        DEFER(VECTOR_DECLARE_GET)(TYPE); \
        DEFER(VECTOR_DECLARE_SET)(TYPE); \
        DEFER(VECTOR_DECLARE_ADD)(TYPE); \
        DEFER(VECTOR_DECLARE_DEL)(TYPE);)

#define VECTOR_DEFINE_CREATE(TYPE) \
void vector_create(TYPE)(vector_* v, u32 size) { \
	size = MAX(size, BLOCK_32); \
	vector_create_(v, size * sizeof(TYPE)); \
}

#define VECTOR_DEFINE_RESIZE(TYPE) \
void vector_resize(TYPE)(vector_* v, u32 size) { \
	vector_resize_(v, size * sizeof(TYPE)); \
}

#define VECTOR_DEFINE_AT(TYPE) \
TYPE* vector_at(TYPE)(vector_* v, u32 idx) { \
	TYPE* data = (TYPE*)v->data; \
	return data + idx; \
}

#define VECTOR_DEFINE_GET(TYPE) \
void vector_get(TYPE)(vector_* v, TYPE* data, u32 idx) { \
	copy(TYPE)(vector_at(TYPE)(v, idx), data); \
}

#define VECTOR_DEFINE_SET(TYPE) \
void vector_set(TYPE)(vector_* v, TYPE* data, u32 idx) { \
	copy(TYPE)(data, vector_at(TYPE)(v, idx)); \
}

#define VECTOR_DEFINE_ADD(TYPE) \
void vector_add(TYPE)(vector_* v, TYPE* data) {\
    if (v->reserve <= (v->size * sizeof(TYPE))) \
        vector_resize(TYPE)(v, v->size * 2); \
	vector_set(TYPE)(v, data, v->size); \
	v->size++; \
}

#define VECTOR_DEFINE_DEL(TYPE) \
TYPE* vector_del(TYPE)(vector_* v, u32 idx) { \
	v->size--; \
	TYPE* root = vector_at(TYPE)(v, idx); \
	TYPE* last = vector_at(TYPE)(v, v->size); \
	swap(TYPE)(root, last); \
	return vector_at(TYPE)(v, v->size); \
}

#define VECTOR_DEFINE(TYPE) \
EXPAND4(DEFER(VECTOR_DEFINE_CREATE)(TYPE); \
        DEFER(VECTOR_DEFINE_RESIZE)(TYPE); \
        DEFER(VECTOR_DEFINE_AT)(TYPE); \
        DEFER(VECTOR_DEFINE_GET)(TYPE); \
        DEFER(VECTOR_DEFINE_SET)(TYPE); \
        DEFER(VECTOR_DEFINE_ADD)(TYPE); \
        DEFER(VECTOR_DEFINE_DEL)(TYPE);)

#define HEAP_MAKE_DECLARE(TYPE) \
void heap(TYPE)(vector_* v)

#define HEAP_ADD_DECLARE(TYPE) \
void heap_add(TYPE)(vector_* v, TYPE* val)

#define HEAP_DEL_DECLARE(TYPE) \
TYPE* heap_del(TYPE)(vector_* v, u32 idx)

#define HEAP_REPLACE_DECLARE(TYPE) \
void heap_replace(TYPE)(vector_* v, TYPE* val)

#define HEAP_DECLARE(TYPE) \
EXPAND4(DEFER(HEAP_MAKE_DECLARE)(TYPE); \
		DEFER(HEAP_ADD_DECLARE)(TYPE); \
		DEFER(HEAP_DEL_DECLARE)(TYPE); \
		DEFER(HEAP_REPLACE_DECLARE)(TYPE); )

#define HEAP_MAKE_DEFINE(TYPE) \
void heap(TYPE)(vector_* v) { \
	u32 offset = v->size % 2 ? 3 : 2; \
	i32 i = (v->size - offset) / 2; \
	for (; i >= 0; i--) { \
		heap_siftdown_(v, i, _swap(TYPE), _lt(TYPE), sizeof(TYPE)); \
	} \
}

#define HEAP_ADD_DEFINE(TYPE) \
void heap_add(TYPE)(vector_* v, TYPE* val) { \
	vector_add(TYPE)(v, val); \
	heap_siftup_(v, v->size - 1, _swap(TYPE), _le(TYPE), sizeof(TYPE)); \
}

#define HEAP_DEL_DEFINE(TYPE) \
TYPE* heap_del(TYPE)(vector_* v, u32 idx) { \
	TYPE* val = vector_del(TYPE)(v, idx); \
	heap_siftdown_(v, idx, _swap(TYPE), _lt(TYPE), sizeof(TYPE)); \
	return val; \
}

#define HEAP_REPLACE_DEFINE(TYPE) \
void heap_replace(TYPE)(vector_* v, TYPE* val) { \
	TYPE* top = vector_at(TYPE)(v, 0); \
	if (lt(TYPE)(val, top)) return; \
	copy(TYPE)(val, top); \
	heap_siftdown_(v, 0, _swap(TYPE), _lt(TYPE), sizeof(TYPE)); \
}

#define HEAP_DEFINE(TYPE) \
EXPAND4(DEFER(HEAP_MAKE_DEFINE)(TYPE); \
		DEFER(HEAP_ADD_DEFINE)(TYPE); \
		DEFER(HEAP_DEL_DEFINE)(TYPE); \
		DEFER(HEAP_REPLACE_DEFINE)(TYPE); )

VECTOR_DECLARE(i8);
VECTOR_DECLARE(i16);
VECTOR_DECLARE(i32);
VECTOR_DECLARE(i64);

VECTOR_DECLARE(u8);
VECTOR_DECLARE(u16);
VECTOR_DECLARE(u32);
VECTOR_DECLARE(u64);

VECTOR_DECLARE(f32);
VECTOR_DECLARE(f64);
