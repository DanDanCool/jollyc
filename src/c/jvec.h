#pragma once

#include "jtype.h"
#include "jmem.h"
#include "jsimd.h"

// note: do to the nature of memory, it will be up to the user to allocate and free memory
typedef struct
{
	mem_block data;
	u32 blocksz;
	u32 size;
} vector;

typedef int (*pfn_cmp)(u8* a, u8* b);
typedef void (*pfn_swap)(u8* a, u8* b);

inline u8* vec_at(vector* v, u32 index)
{
	u8* data = MEM_DATA(&v->data);
	return data + index * v->blocksz;
}

void vec_add(vector* v, u8* data);
u8* vec_rm(vector* v);
void vec_resize(vector* v, u32 size);
void vec_sort(vector* v, pfn_cmp cmp, pfn_swap swap);
