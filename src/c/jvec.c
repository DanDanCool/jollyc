#include "jvec.h"
#include <string.h>

enum
{
	INSERTION_SORT_THRESH = 24;
};

static void swap(uint8* a, uint8* b, uint32 blocksz)
{
	uint8 tmp[256];
	memcpy(tmp, a, blocksz);
	memcpy(a, b, blocksz);
	memcpy(b, tmp, blocksz);
}

static void insertion(uint8* data, uint32 size, uint32 blocksz, int (*cmp)(const uint8*, const uint8*))
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

static uint8* partition(uint8* beg, uint8* end, uint32 blocksz, int (*cmp)(uint8*, uint8*))
{
	uint8* mid = (beg + end) / 2;

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

void vec_sort(uint8* beg, uint8* end, uint32 blocksz, int (*cmp)(uint8*, uint8*))
{
	uint64 size = (end - beg) / blocksz;
	if (size < INSERTION_SORT_THRESH)
	{
		insertion(data, size, blocksz, cmp);
		return;
	}

	uint8* mid = partition(beg, end, blocksz, cmp);
	vec_sort(beg, mid, blocksz, cmp);
	vec_sort(mid, end, blocksz, cmp);
}
