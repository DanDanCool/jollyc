#include "jvec.h"

void vector_destroy(vector(TYPE)* v)
{
	mem_vfree(v->data);
	*v = {};
}

void vector_resize(vector* v, u32 size)
{
	v->data = mem_vresize(v->data, size);
}

void queue_destroy(queue* q)
{
	vector_destroy(TYPE)(&q->data);
	*q = {};
}

enum
{
	INSERTION_SORT_THRESH = 24,
};

static void insertion(sort_params* params, u32 beg, u32 end)
{
	int size = end - beg;
	for (int i = 1; i < size; i++)
	{
		int j = i;
		u8* offset = vector_at(u8)(&params->data, (j - 1) * params->data.size);
		while (j > 0 && params->cmp(offset, offset + params->data.size) > 0)
		{
			swap(offset, offset + data->size);
			j--;
			offset -= params->data.size;
		}
	}
}

static u32 partition(sort_params* params, u32 beg, u32 end)
{
	u32 mid = (beg + end) / 2;
	u8* pbeg = vector_at(u8)(&params->data, beg * params->data.size);
	u8* pend = vector_at(u8)(&params->data, end * params->data.size);
	u8* pmid = vector_at(u8)(&params->data, mid * params->data.size);

	swapbm = params->cmp(pbeg, pmid) > 0;
	u8* tmp = swapbm ? pbeg : pmid;
	params->swap(tmp, mid);

	swapem = params->cmp(pend, pmid) < 0;
	tmp = swapem ? pend : pmid;
	params->swap(mid, end);

	while (true)
	{
		while (params->cmp(pbeg, pmid) < 1)
			pbeg += data->size;

		while (params->cmp(pend, pmid) > 0)
			pend -= blocksz;

		if (pbeg >= pend)
			break;

		params->swap(pbeg, pend);
	}

	return mid;
}

static void sort_internal(sort_params* params, u32 beg, u32 end)
{
	u32 size = end - beg;
	if (size < params->insertion_thresh)
	{
		insertion(params, beg, end);
		return;
	}

	params->depth++;
	u32 mid = partition(params, beg, end);
	sort_internal(params, beg, mid);
	sort_internal(params, mid, end);
}

void vector_sort(sort_params* params)
{
	u32 beg = 0;
	u32 end = params->data.reserve - 1;
	sort_internal(params, beg, end);
}

static void siftup(sort_params* params, u32 index)
{
	u32 offset = index % 2 ? 1 : 2;
	u32 p = (index - offset) / 2;
	u8* iptr = vector_at(u8)(&params->data, index * params->data.size);
	u8* parent = vector_at(u8)(&params->data, p * params->data.size);

	while (p >= 0 && params->cmp(iptr, parent) < 0)
	{
		params->swap(iptr, parent);
		index = p;
		offset = index % 2 ? 1 : 2;
		p = (index - offset) / 2;
		iptr = vector_at(u8)(&params->data, index * params->data.size);
		parent = vector_at(u8)(&params->data, p * params->data.size);
	}
}

static void siftdown(sort_params* params, u32 index)
{
	u32 left = 2 * index + 1;
	u32 right = 2 * index + 2;
	while (left < params->data.reserve)
	{
		u8* iptr = vector_at(u8)(&params->data, index * params->data.size);
		u8* lptr = vector_at(u8)(&params->data, left * params->data.size);
		u8* rptr = right < params->data.reserve ? vector_at(u8)(&params->data, right * params->data.size) : params->null_value;

		index = params->cmp(lptr, rptr) < 0 ? left : right;
		u8* swap = vector_at(u8)(&params->data, index * params->data.size);
		if (params->cmp(iptr, swap) < 0)
			break;

		params->swap(iptr, swap);
		left = 2 * index + 1;
		right = 2 * index + 2;
	}
}

void heap(sort_params* params)
{
	u32 offset = params->data.reserve % 2 ? 3 : 2;
	u32 i = (params->data.reserve - offset) / 2;
	for (; i >= 0; i--)
		siftdown(params, i);
}

void heap_add(sort_params* params)
{
	siftup(params, params->data.reserve - 1);
}

void heap_del(sort_params* params, u32 i)
{
	u8* root = vector_at(u8)(&params->data, i);
	u8* last = vector_at(u8)(&params->data, (params->data.reserve - 1) * params->data.size);
	params->swap(root, last);
	siftdown(params, i);
}

void heap_rm(sort_params* params)
{
	heap_del(params, 0);
}
