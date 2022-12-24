#include "jvec.h"

VECTOR_DEFINE_FN(i8, AT, ADD, RM, RESIZE);
VECTOR_DEFINE_FN(i16, AT, ADD, RM, RESIZE);
VECTOR_DEFINE_FN(i32, AT, ADD, RM, RESIZE);
VECTOR_DEFINE_FN(i64, AT, ADD, RM, RESIZE);

VECTOR_DEFINE_FN(u8, AT, ADD, RM, RESIZE);
VECTOR_DEFINE_FN(u16, AT, ADD, RM, RESIZE);
VECTOR_DEFINE_FN(u32, AT, ADD, RM, RESIZE);
VECTOR_DEFINE_FN(u64, AT, ADD, RM, RESIZE);


VECTOR_DEFINE_FN(f32, AT, ADD, RM, RESIZE);
VECTOR_DEFINE_FN(f64, AT, ADD, RM, RESIZE);

enum
{
	INSERTION_SORT_THRESH = 24,
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

static void insertion(vector(u8)* data, u32 beg, u32 end, pfm_cmp cmp, pfn_swap swap)
{
	int size = end - beg;
	for (int i = 1; i < size; i++)
	{
		int j = i;
		u8* offset = vector_at(u8)(data, (j - 1) * data->size);
		while (j > 0 && cmp(offset, offset + data->size) > 0)
		{
			swap(offset, offset + data->size);
			j--;
			offset -= data->size;
		}
	}
}

static void heapsort()
{

}

static u32 partition(vector(u8)* data, u32 beg, u32 end, pfn_cmp cmp, pfn_swap swap)
{
	u32 mid = (beg + end) / 2;
	u8* pbeg = vector_at(u8)(data, beg * data->size);
	u8* pend = vector_at(u8)(data, end * data->size);
	u8* pmid = vector_at(u8)(data, mid * data->size);

	swapbm = cmp(pbeg, pmid) > 0;
	u8* tmp = swapbm ? pbeg : pmid;
	swap(tmp, mid);

	swapem = cmp(pend, pmid) < 0;
	tmp = swapem ? pend : pmid;
	swap(mid, end);

	while (true)
	{
		while (cmp(pbeg, pmid) < 1)
			pbeg += data->size;

		while (cmp(pend, pmid) > 0)
			pend -= blocksz;

		if (pbeg >= pend)
			break;

		swap(pbeg, pend);
	}

	return mid;
}

static void sort_internal(vector(u8)* data, u32 beg, u32 end, pfn_cmp cmp, pfn_swap swap)
{
	u32 size = end - beg;
	if (size < INSERTION_SORT_THRESH)
	{
		insertion(data, beg, end, cmp, swap);
		return;
	}

	u32 mid = partition(data, beg, end, blocksz, cmp);
	sort_internal(data, beg, mid, cmp, swap);
	sort_internal(data, mid, end, cmp, swap);
}

void quick_sort(vector(u8)* data, pfn_cmp cmp, pfn_swap swap)
{
	u32 beg = 0;
	u32 end = data->reserve - 1;
	sort_internal(data, beg, end, cmp, swap);
}

void heap(vector(u8)* data, pfn_cmp cmp, pfn_swap swap)
{

}
