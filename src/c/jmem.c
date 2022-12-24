#include "jmem.h"
#include <stdlib.h>
#include <emmintrin.h>

static void gc_resize(mem_gc* gc, u32 size);
static void gc_sort(mem_block* beg, mem_block* end);
static mem_block* gc_partition(mem_block* beg, mem_block* end);
static void gc_insertion(mem_block* beg, mem_block* end);

// swap if memory adress in a is greater than b
static void swapgt(mem_block* a, mem_block* b);
static void swap(mem_block* a, mem_block* b);

inline mem_block* arena_freeblock(mem_arena* arena)
{
	return arena->gc.data + arena->gc.free;
}

void arena_init(mem_arena* arena, u32 size, u32 blocksz)
{
	arena->base.reserve = size;
	arena->base.size = blocksz;
	arena->base.data = (u8*)aligned_alloc(64, size * blocksz);

	arena->gc.data = (mem_block*)aligned_alloc(64, BLOCK_128 * sizeof(mem_block));
	arena->gc.size = 1;
	arena->gc.cap = BLOCK_128;
	arena->gc.new = 0;
	arena->gc.data[0] = { arena, 0, size * blocksz };
	arena->gc.free = 0;
}

void arena_destroy(mem_arena* arena)
{
	free(arena->data);
	arena->base = {};

	free(arena->gc.data);
	arena->gc = {};
	arena->gc.free = U32_MAX;
}

mem_block arena_alloc(mem_arena* arena, u32 count)
{
	mem_block* free = arena_freeblock(arena);

	if (free->size < count * arena->base.blocksz)
	{
		arena_resize(arena, arena->size * 2);
		free = arena_freeblock(arena);
	}

	mem_block block = { &arena->base, free->handle, count * arena->base.blocksz };
	free->handle += count;
	free->size -= count * arena->base.blocksz;
}

mem_block arena_realloc(mem_block* block, u32 count)
{
	mem_arena* arena = (mem_arena*)block->owner;
	u8* base = MEM_DATA(block);
	mem_block* free = arena_freeblock(arena);

	u32 extra = count * arena->base.blocksz - block->size;
	if (base + block->size == MEM_DATA(free) && free->size > extra)
	{
		block->size += extra;
		free->handle += extra / arena->base.blocksz;
		free->size -= extra;
		return;
	}

	// not enough space adjacent, reallocate
	mem_block tmp = arena_alloc(arena, count);
	switch (arena->base.blocksz)
	{
		case BLOCK_16:
		{
			u32 count = block->size / BLOCK_16;
			mem_cpy16(tmp.data, block->data, count);
			break;
		}
		default:
		{
			u32 count = block->size / BLOCK_32;
			mem_cpy32(tmp.data, block-.data, count);
			break;
		}
	}

	*block = tmp;
}

void arena_free(mem_block* block)
{
	mem_arena* arena = (mem_arena*)block->owner;
	mem_gc* gc = &arena->gc;
	if (gc->cap <= gc->size)
		gc_resize(gc, gc->cap * 2);

	gc->data[gc->size++] = *block;
	gc->new++;

	// should only do this in debug
	*block = {};
}

void arena_resize(mem_arena* arena, u32 size)
{
	u8* tmp = (u8*)aligned_alloc(64, size * arena->base.blocksz);
	u32 copy = arena->base.size > size ? size : arena->base.size;

	switch (arena->base.blocksz)
	{
		case BLOCK_16:
		{
			mem_cpy16(tmp, arena->base.data, copy);
			break;
		}
		default:
		{
			mem_cpy32(tmp, arena->base.data, copy * arena->base.blocksz / BLOCK_32);
			break;
		}
	}

	free(arena->base.data);
	arena->base.data = tmp;
	arena->base.size = size;
}

void arena_gc(mem_arena* arena)
{
	mem_gc* gc = arena->gc;
	mem_block* unsorted = gc->data + gc->size - gc->new - 1;
	gc_sort(unsorted, unsorted + gc->new);

	if (gc->size + gc->new * 2 > gc->cap)
		gc_resize(gc, gc->cap * 2);

	// merge sorted and newly sorted regions
	mem_cpy32u(unsorted + gc->new, unsorted, gc->new);
	mem_block* arr1 = gc->data + gc->size - gc->new - 1;
	mem_block* arr2 = gc->data + gc->size + gc->new - 1;
	mem_block* dst = gc->data + gc->size - 1;

	for (u32 i = 0; i < gc->size; i++)
	{
		__m128i x = _mm_load_si128((__m128i*)arr1);
		__m128i y = _mm_load_si128((__m128i*)arr2);

		int mask = arr1->data > arr2->data ? 0x0 : 0xF;
		__m128i res = _mm_blend_epi32(x, y, mask);
		_mm_store_si128((__m128i*)dst, res);

		arr1 -= arr1->data > arr2->data;
		arr2 -= !(arr1->data > arr2->data);
		dst--;
	}

	// collect contiguous memory blocks into single memory block
	mem_block* cur = gc->data;
	mem_block* it = gc->data + 1;
	for (u32 i = 0; i < gc->size; i++)
	{
		int next = MEM_DATA(cur) + cur->size == MEM_DATA(it);
		cur->size = next ? cur->size + it->size : cur->size;
		it->handle = next ? U32_MAX : it->handle;
		cur = next ? cur : it;
		it++;
	}

	// fill in holes in the array
	int distance = 0;
	int size = 0;
	mem_block* best = gc->data;
	cur = gc->data;
	for (u32 i = 0; i < gc->size; i++)
	{
		mem_cpy16(cur - distance, cur, 1);
		best = best->size < cur->size ? cur : best;
		distance += cur->handle == U32_MAX;
		size += cur->handle != U32_MAX;
	}

	gc->free = best - gc->data;
	gc->size = size;
	gc->new = 0;
}

void pool_init(mem_pool* pool, u32 size, u32 blocksz)
{
	pool->base.data = (u8*)aligned_alloc(64, size * blocksz);
	pool->base.blocksz = blocksz;
	pool->base.size = size;
	pool->free = 0;

	u32* block = (u32*)pool->base.data;
	for (u32 i = 1; i < size; i++)
	{
		*block = i;
		block += blocksz;
	}

	*block = U32_MAX;
}

void pool_destroy(mem_pool* pool)
{
	free(pool->base.data);
	pool->base = {};
	pool->free = U32_MAX;
}

void pool_resize(mem_pool* pool, u32 size)
{
	u8* tmp = (u8*)aligned_alloc(64, size * pool->base.blocksz);
	u32 copy = pool->base.size > size ? size : pool->base.size;

	switch(pool->base.blocksz)
	{
		case BLOCK_16:
		{
			mem_cpy16(tmp, arena->base.data, copy);
			break;
		}
		default:
		{
			mem_cpy32(tmp, pool->base.data, copy * pool->base.blocksz / BLOCK32);
			break;
		}
	}

	if (size > pool->base.size)
	{
		u32* block = tmp + pool->base.blocksz * size;
		u32 node = pool->free;

		while (node != U32_MAX)
		{
			block = tmp + pool->base.blocksz * last;
			node = *block;
		}

		for (u32 i = pool->base.size; i < size; i++)
		{
			*block = i;
			block += pool->base.blocksz;
		}

		*block = U32_MAX;
		pool->free = pool->free == U32_MAX ? pool->base.size : pool->free;
	}
	else
	{
		u32* block;
		u32 node = pool->free;

		while (node != U32_MAX)
		{
			block = pool->base.data + pool->base.blocksz * node;
			u32 next = *block;

			while (next > size && next != U32_MAX)
			{
				block = pool->base.data + pool->base.blocksz * next;
				next = *block;
			}

			block = tmp + pool->base.blocksz * node;
			*block = next;
			node = next;
		}
	}

	free(pool->base.data);
	pool->base.data = tmp;
	pool->base.size = size;
}

mem_block pool_alloc(mem_pool* pool)
{
	if (pool->free == U32_MAX)
		pool_resize(pool->base.size * 2);

	mem_block mem = { &pool->base, pool->free, pool->base.blocksz };
	u32* node = (u32*)(pool->base.data + pool->free * pool->base.blocksz);
	pool->free = *node;
	return mem;
}

void pool_free(mem_block* mem)
{
	mem_pool* pool = (mem_pool*)mem->owner;
	u32* node = (u32*)(pool->base.data + mem->handle * pool->base.blocksz);
	u32 tmp = mem->handle;
	*node = pool->free;
	pool->free = tmp;
}

mem_ptr mem_alloc(u32 size)
{
	mem_ptr mem = {};
	mem.data = (u8*)aligned_alloc(64, size);
	mem.size = size;
	return mem;
}

void mem_realloc(mem_ptr* block, u32 size)
{
	mem_ptr tmp = mem_alloc(size);

	// assume that the original size is a multiple of 32
	mem_cpy32(tmp.data, block->data, block->size / 32);

	free(block->data);
	*block = tmp;
}

void mem_free(mem_ptr* block)
{
	// assert that this block does not belong to an arena
	free(block->data);
	block->size = 0;
}

void mem_cpy16(u8* dst, u8* src, u32 count)
{
	for (int i = 0; i < count; i++)
	{
		__m128i tmp = _mm_load_si128((__m128i*)src);
		_mm_store_si128((__m128i*)dst, tmp);

		dst += BLOCK_16;
		src += BLOCK_16;
	}
}

void mem_cpy32(u8* dst, u8* src, u32 count)
{
	for (u32 i = 0; i < count; i++)
	{
		__m256i tmp = _mm256_load_si256((__m256i*)src);
		_mm256_store_si256((__m256i*)dst, tmp);

		dst += BLOCK_32;
		src += BLOCK_32;
	}
}

void mem_cpy16u(u8* dst, u8* src, u32 count)
{
	for (u32 i = 0; i < count; i++)
	{
		__m128i tmp = _mm_loadu_si128((__m128i*)src);
		_mm_storeu_si128((__m128i*)dst, tmp);

		dst += BLOCK_16;
		src += BLOCK_16;
	}
}

void mem_cpy32u(u8* dst, u8* src, u32 count)
{
	for (u32 i = 0; i < count; i++)
	{
		__m256i tmp = _mm256_loadu_si256((__m256i*)src);
		_mm_storeu_si256((__m256i*)dst, tmp);

		dst += BLOCK_32;
		src += BLOCK_32;
	}
}

void mem_set16(u8* dst, u32 val, u32 count)
{
	__m128i tmp = _mm_set1_epi32(val);
	for (u32 i = 0; i < count; i++)
	{
		_mm_store_si128((__m128i*)dst, tmp);
		dst += BLOCK_16;
	}
}

void mem_set32(u8* dst, u32 val, u32 count)
{
	__m256i tmp = _mm256_set1_epi32(val);
	for (u32 i = 0; i < count; i++)
	{
		_mm_store_si256((__m256i*)dst, tmp);
		dst += BLOCK_32;
	}
}

int mem_cmp16(u8* a, u8* b, u32 count)
{
	__m128i one = _mm_set1_epi8(0xFF);
	for (u32 i = 0; i < count; i++)
	{
		__m128i x = _mm_load_si128((__m128i*)a);
		__m128i y = _mm_load_si128((__m128i*)b);
		__m128i xor = _mm_xor_si128(x, y);
		if (!_mm_testz_si128(xor, one))
			return 0;
	}

	return 1;
}

int mem_cmp32(u8* a, u8* b, u32 count)
{
	__m256i one = _mm256_set1_epi8(0xFF);
	for (u32 i = 0; i < count; i++)
	{
		__m256i x = _mm256_load_si256((__m256i*)a);
		__m256i y = _mm256_load_si256((__m256i*)b);
		__m256i xor = _mm256_xor_si256(x, y);
		if (!_mm256_testz_si256(xor, one))
			return 0;
	}

	return 1;
}

int mem_cmp16u(u8* a, u8* b, u32 count)
{
	__m128i one = _mmset1_epi8(0xFF);
	for (u32 i = 0; i < count; i++)
	{
		__m128i x = _mm_loadu_si128((__m128i*)a);
		__m128i y = _mm_loadu_si128((__m128i*)b);
		__m128i xor = _mm_xor_si128(x, y);
		if (!_mm_testz_si128(xor, one))
			return 0;
	}

	return 1;
}

int mem_cmp32u(u8* a, u8* b, u32 count)
{
	__m256i one = _mm256_set1_epi8(0xFF);
	for (u32 i = 0; i < count; i++)
	{
		__m256i x = _mm256_loadu_si256((__m256i*)a);
		__m256i y = _mm256_loadu_si256((__m256i*)b);
		__m256i xor = _mm256_xor_si256(xor, one);
		if (!_mm256_testz_si256(xor, one))
			return 0;
	}

	return 1;
}

enum
{
	INSERTION_THRESHOLD = 24,
}

static void swap(mem_block* a, mem_block* b)
{
	__m128i x = _mm_load_si128((__m128i*)a);
	__m128i y = _mm_load_si128((__m128i*)b);

	_mm_store_si128((__m128i*)a, y);
	_mm_store_si128((__m128i*)b, x);
}

// swap if memory adress in a is greater than b
static void swapgt(mem_block* a, mem_block* b)
{
	__m128i x = _mm_load_si128((__m128i*)a);
	__m128i y = _mm_load_si128((__m128i*)b);

	int mask = a->data > b->data ? 0xF : 0x0;
	__m128i dsta = _mm_blend_epi32(x, y, mask);
	__m128i dstb = _mm_blend_epi32(y, x, mask);

	_mm_store_si128((__m128i*)a, dsta);
	_mm_store_si128((__m128i*)b, dstb);
}

static void gc_sort(mem_block* beg, mem_block* end)
{
	if ((end - beg) < INSERTION_THRESHOLD)
	{
		gc_insertion(beg, end);
		return;
	}

	mem_block* mid = gc_partition(beg, end);
	gc_sort(beg, mid);
	gc_sort(mid + 1, end);
}

static mem_block* gc_partition(mem_block* beg, mem_block* end)
{
	// pivot is the median of the beginning, middle and end
	mem_block* mid = beg + (end - beg) / 2;
	swapgt(beg, mid);
	swapgt(mid, end);

	while (true)
	{
		while (beg->data < mid->data) beg++;
		while (end->data > mid->data) end--;
		if (beg >= end) break;
		swap(beg, end);
	}

	return mid;
}

static void gc_insertion(mem_block* beg, mem_block* end)
{
	for (int i = 1; i < end - beg; i++)
	{
		int j = i;
		while (beg[j - 1] < beg[j] && j > 0)
		{
			swap(&beg[j - 1], &beg[j]);
			j--;
		}
	}
}

static void gc_resize(mem_gc* gc, u32 size)
{
	mem_block* tmp = (mem_block*)aligned_alloc(64, size * sizeof(mem_block));
	u32 copy = gc->cap > size ? size : gc->cap;
	mem_cpy32(tmp, gc->data, copy);
	free(gc->data);
	gc->data = tmp;
	gc->cap = size;
}
