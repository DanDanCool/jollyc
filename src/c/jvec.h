#pragma once
#include <stdlib.h>

#include "jtype.h"
#include "jmem.h"
#include "jsimd.h"

#define VEC(type) vec_##type

#define VEC_DEFINE(type) \
	typedef struct { \
		type* data; \
		mem_block mem; \
		uint32 size; \
	} vec_##type

#define VEC_ALLOC(vec, size) \
	vec.size = 0; \
	vec.mem = mem_alloc(sizeof(vec.data[0] * size)); \
	vec.data = (type*)vec.mem.data

#define VEC_FREE(vec) \
	free(vec.data); \
	vec.mem = {}; \
	vec{}

#define VEC_ADD_FAST(vec, data) \
	memcpy(&vec.data[vec.size++], data, sizeof[vec[0]]);

#define VEC_ADD_SAFE(vec, data) \
	if (!(vec.size < vec.mem.size)) \
		VEC_ALLOC(vec, vec.cap * 2); \
	memcpy(&vec.data[vec.size++], data, sizeof[vec[0]]);

#define VEC_ADD(vec, data) VEC_ADD_FAST(vec, data)

#define VEC_RM(vec) \
	vec.data[--vec.size]

#define VEC_AT(vec, index) \
	vec.data[index]

#define VEC_SORT_DEFINE(type, cmp, swap) \
	enum { \
		INSERTION_SORT_THRESH_##type = 24; \
	} \
	static void insertion_sort_##type(type* beg, type* end) { \
		for (int i = 1; i < end - beg; i++) { \
			int j = i; \
			while (j > 0 && cmp(&beg[j - 1], &beg[j]) > 0) { \
				swap(&beg[j - 1], &beg[j]) \
				j--; \
			} \
		} \
	} \
	static type* partion_##type(type* beg, type* end) { \
		type* mid = beg + (end - beg) / 2; \
		if (cmp(beg, mid) > 0) swap(beg, mid); \
		if (cmp(mid, end) > 0) swap(mid, end); \
		while (true) { \
			while (cmp(beg, mid) < 1) beg++; \
			while (cmp(mid, end) < 1) end--; \
			if (beg >= end) break; \
			swap(beg, end); \
		} \
		return mid; \
	} \
	void vec_sort_##type(type* beg, type* end) { \
		if (end - beg < INSERTION_SORT_THRESH_##type) { \
			insertion_sort_##type(beg, end); \
			return; \
		} \
		type* mid = partition_##type(beg, end); \
		vec_sort_##type(beg, mid); \
		vec_sort_##type(mid, end); \
	}

void vec_sort(uint8* data, uint32 size, uint32 blocksz, int (*cmp)(uint8*, uint8*));
