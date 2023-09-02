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

void heap_siftup_(vector_* v, u32 idx, pfn_swap swapfn, pfn_le lefn, u32 keysize) {
	u32 offset = idx % 2 ? 1 : 2;
	u32 p = (idx - offset) / 2;
	u8* iptr = vector_at(u8)(v, idx * keysize);
	u8* pptr = vector_at(u8)(v, p * keysize);

	while (p >= 0 && lefn(iptr, pptr)) {
		swapfn(iptr, pptr);
		idx = p;
		offset = idx % 2 ? 1 : 2;
		p = (idx - offset) / 2;
		iptr = vector_at(u8)(v, idx * keysize);
		pptr = vector_at(u8)(v, p * keysize);
	}
}

void heap_siftdown_(vector_* v, u32 idx, pfn_swap swapfn, pfn_lt ltfn, u32 keysize) {
	u32 left = 2 * idx + 1;
	u32 right = 2 * idx + 2;
	while (left < v->size) {
		u8* iptr = vector_at(u8)(v, idx * keysize);
		u8* lptr = vector_at(u8)(v, left * keysize);
		u8* rptr = right < v->size ? vector_at(u8)(v, right * keysize) : NULL;

		idx = ltfn(rptr, lptr) ? right : left;
		u8* swap = vector_at(u8)(v, idx * keysize);
		if (ltfn(iptr, swap)) break;

		swapfn(iptr, swap);
		left = 2 * idx + 1;
		right = 2 * idx + 2;
	}
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
