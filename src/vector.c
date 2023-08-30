#include "vector.h"
#include "memory.h"

void vector_create_(vector_* v, u32 bytes) {
	memptr ptr = alloc256(bytes);
	v->data = ptr.data;
	v->reserve = (u32)ptr.size;
	v->size = 0;
}

void vector_resize_(vector_* v, u32 bytes) {
	memptr ptr = alloc256(bytes);
	copy256(v->data, ptr.data, (u32)ptr.size);
	free256(v->data);
	v->data = ptr.data;
	v->reserve = (u32)ptr.size;
}

void vector_destroy_(vector_* v) {
	free256(v->data);
	v->data = NULL;
	v->reserve = 0;
	v->size = 0;
}

VECTOR_DEFINE(i8);
VECTOR_DEFINE(i16);
VECTOR_DEFINE(i32);
VECTOR_DEFINE(i64);

VECTOR_DEFINE(u8);
VECTOR_DEFINE(u16);
VECTOR_DEFINE(u32);
VECTOR_DEFINE(u64);

VECTOR_DEFINE(f32);
VECTOR_DEFINE(f64);
