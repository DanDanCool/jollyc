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

#define vector(type) vector_
#define vector_create(TYPE) vector_create_##TYPE
#define vector_destroy(TYPE) vector_destroy_
#define vector_resize(TYPE) vector_resize_##TYPE
#define vector_at(TYPE) vector_at_##TYPE
#define vector_get(TYPE) vector_get_##TYPE
#define vector_set(TYPE) vector_set_##TYPE
#define vector_add(TYPE) vector_add_##TYPE
#define vector_del(TYPE) vector_rm_##TYPE

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
	TYPE tmp; \
	vector_get(TYPE)(v, &tmp, idx); \
	vector_set(TYPE)(v, vector_at(TYPE)(v, v->size), idx); \
	vector_set(TYPE)(v, &tmp, v->size + 1); \
	return vector_at(TYPE)(v, v->size + 1); \
}

#define VECTOR_DEFINE(TYPE) \
EXPAND4(DEFER(VECTOR_DEFINE_CREATE)(TYPE); \
        DEFER(VECTOR_DEFINE_RESIZE)(TYPE); \
        DEFER(VECTOR_DEFINE_AT)(TYPE); \
        DEFER(VECTOR_DEFINE_GET)(TYPE); \
        DEFER(VECTOR_DEFINE_SET)(TYPE); \
        DEFER(VECTOR_DEFINE_ADD)(TYPE); \
        DEFER(VECTOR_DEFINE_DEL)(TYPE);)

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
