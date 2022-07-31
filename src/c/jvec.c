#include "jvec.h"

#include <string.h>
#include <stdlib.h>

enum
{
	INSERTION_SORT_THRESH = 24;
};

void vec_add(vector* v, u8* data)
{
	if (v->size * v->blocksz >= v->data.size)1
		vec_resize(v, v->size * 2);

	// size of the type is not known ahead of time hence this
	memcpy(vec_at(v, v->size), data, v->blocksz);
	v->size++;
}

u8* vec_rm(vector* v)
{
	v->size--;
	return vec_at(v, v->size);
}

void vec_resize(vector* v, u32 size)
{
	if (v->data.handle == U32_MAX)
		mem_realloc(&v->data, size * v->blocksz);
	else
		arena_realloc(&v->data, size);
}

static void insertion(u8* data, u32 size, u32 blocksz, pfm_cmp cmp, pfn_swap swap)
{
	for (int i = 0; i < size; i++)
	{
		int j = i;
		while (j > 0 && cmp(data + (j - 1) * blocksz, data + j * blocksz) > 0)
		{
			swap(data + (j - 1) * blocksz, data + j * blocksz, blocksz);
			j--;
		}
	}
}

static void heapsort()
{

}

static u8* partition(u8* beg, u8* end, u32 blocksz, pfn_cmp cmp, pfn_swap swap)
{
	u8* mid = (beg + end) / 2;

	if (cmp(beg, mid) > 0)
		swap(beg, mid);

	if (cmp(mid, end) > 0)
		swap(mid, end);

	while (true)
	{
		while (cmp(beg, mid) < 1)
			beg += blocksz;

		while (cmp(mid, end) < 1)
			end -= blocksz;

		if (beg >= end)
			break;

		swap(beg, end);
	}

	return mid;
}

void sort_internal(u8* beg, u8* end, u32 blocksz, pfn_cmp cmp, pfn_swap swap)
{
	u64 size = (end - beg) / blocksz;
	if (size < INSERTION_SORT_THRESH)
	{
		insertion(data, size, blocksz, cmp);
		return;
	}

	u8* mid = partition(beg, end, blocksz, cmp);
	sort_internal(beg, mid, blocksz, cmp);
	sort_internal(mid, end, blocksz, cmp);
}

void vec_sort(vector* v, pfn_cmp cmp, pfn_swap swap)
{
	u8* beg = vec_at(v, 0);
	u8* end = vec_at(v, v->size - 1);
	sort_internal(beg, end, v->blocksz, cmp, swap);
}
