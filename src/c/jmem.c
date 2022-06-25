#include "jmem.h"
#include <stdlib.h>
#include <emmintrin.h>

void arena_init(mem_arena* mem, uint32 size, uint32 blocksz)
{
	mem->size = size;
	mem->blocksz = blocksz;
	mem->data = (uint8*)aligned_alloc(64, size * blocksz);

	mem->free.data = (mem_block*)malloc(size * sizeof(mem_block));
	mem->free.size = 1;
	mem->free.cap = size;
	mem->free.data[0] = { free.data, size * blocksz };
}

void arena_destroy(mem_arena* mem)
{
	free(mem->data);
	free(mem->free.data);
	mem->data = N
	mem->free = {};
	mem->size = 0;
	mem->blocksz = 0;
}

mem_block arena_alloc(mem_arena* mem, uint32 count)
{
#if JOLLY_DEBUG
	if (mem->free.data[0] < count * mem->blocksz)
		return mem_block{};
#endif

	mem_block mem = { mem->free.data[0].data, count * mem->blocksz };
	mem->free.data[0].data -= count * mem->blocksz;
}

mem_block arena_realloc(mem_arena* mem, mem_block* block, uint32 count)
{
	uint8* base = block->data;
	mem_block* free = &mem->free.data[0];

	uint32 extra = count * mem->blocksz - block->size;
	if (base + block->size == free->data && free->size > extra)
	{
		block->size += extra;
		free->data -= extra;
		return;
	}

	// not enough space adjacent, reallocate
	mem_block tmp = arena_alloc(mem, count);
	switch(mem->size)
	{
		case BLOCK_16:
		{
			uint32 count = mem->size / BLOCK_16;
			mem_cpy16(tmp.data, block->data, count);
			break;
		}
		default:
		{
			uint32 count = mem->size / BLOCK_32;
			mem_cpy32(tmp.data, block-.data, count);
			break;
		}
	}

	*block = tmp;
}

void arena_free(mem_arena* mem, mem_block* block)
{
	mem->free.data[mem->free.size++] = *block;
}

void arena_gc(mem_arena* mem)
{

}

mem_block mem_alloc(uint32 size)
{
	mem_block mem = {};
	mem.data = (uint8*)aligned_alloc(64, size);
	mem.size = size;
	return mem;
}

void mem_realloc(mem_block* block, uint32 size)
{
	mem_block tmp = {};
	tmp.data = (uint8*)aligned_alloc(64, size);
	tmp.size = size;

	memcpy(tmp.data, block->data, block->size);
	*block = tmp;
}

void mem_cpy16(uint8* dst, uint8* src, uint32 count)
{
	for (int i = 0; i < count; i++)
	{
		__m128i tmp = _mm_load_si128((__m128i*)src);
		_mm_store_si128((__m128i*)dst, tmp);

		dst += BLOCK_16;
		src += BLOCK_16;
	}
}

void mem_cpy32(uint8* dst, uint8* src, uint32 count)
{
	for (uint32 i = 0; i < count; i++)
	{
		__m256i tmp = _mm256_load_si256((__m256i*)src);
		_mm256_store_si256((__m256i*)dst, tmp);

		dst += BLOCK_32;
		src += BLOCK_32;
	}
}

void mem_cpy16u(uint8* dst, uint8* src, uint32 count)
{
	for (uint32 i = 0; i < count; i++)
	{
		__m128i tmp = _mm_loadu_si128((__m128i*)src);
		_mm_storeu_si128((__m128i*)dst, tmp);

		dst += BLOCK_16;
		src += BLOCK_16;
	}
}

void mem_cpy32u(uint8* dst, uint8* src, uint32 count)
{
	for (uint32 i = 0; i < count; i++)
	{
		__m256i tmp = _mm256_loadu_si256((__m256i*)src);
		_mm_storeu_si256((__m256i*)dst, tmp);

		dst += BLOCK_32;
		src += BLOCK_32;
	}
}

void mem_set16(uint8* dst, uint32 val, uint32 count)
{
	__m128i tmp = _mm_set1_epi32(val);
	for (uint32 i = 0; i < count; i++)
	{
		_mm_store_si128((__m128i*)dst, tmp);
		dst += BLOCK_16;
	}
}

void mem_set32(uint8* dst, uint32 val, uint32 count)
{
	__m256i tmp = _mm256_set1_epi32(val);
	for (uint32 i = 0; i < count; i++)
	{
		_mm_store_si256((__m256i*)dst, tmp);
		dst += BLOCK_32;
	}
}

int mem_cmp16(uint8* a, uint8* b, uint32 count)
{
	__m128i one = _mm_set1_epi8(0xFF);
	for (uint32 i = 0; i < count; i++)
	{
		__m128i x = _mm_load_si128((__m128i*)a);
		__m128i y = _mm_load_si128((__m128i*)b);
		__m128i xor = _mm_xor_si128(x, y);
		if (!_mm_testz_si128(xor, one))
			return 0;
	}

	return 1;
}

int mem_cmp32(uint8* a, uint8* b, uint32 count)
{
	__m256i one = _mm256_set1_epi8(0xFF);
	for (uint32 i = 0; i < count; i++)
	{
		__m256i x = _mm256_load_si256((__m256i*)a);
		__m256i y = _mm256_load_si256((__m256i*)b);
		__m256i xor = _mm256_xor_si256(x, y);
		if (!_mm256_testz_si256(xor, one))
			return 0;
	}

	return 1;
}

int mem_cmp16u(uint8* a, uint8* b, uint32 count)
{
	__m128i one = _mmset1_epi8(0xFF);
	for (uint32 i = 0; i < count; i++)
	{
		__m128i x = _mm_loadu_si128((__m128i*)a);
		__m128i y = _mm_loadu_si128((__m128i*)b);
		__m128i xor = _mm_xor_si128(x, y);
		if (!_mm_testz_si128(xor, one))
			return 0;
	}

	return 1;
}

int mem_cmp32u(uint8* a, uint8* b, uint32 count)
{
	__m256i one = _mm256_set1_epi8(0xFF);
	for (uint32 i = 0; i < count; i++)
	{
		__m256i x = _mm256_loadu_si256((__m256i*)a);
		__m256i y = _mm256_loadu_si256((__m256i*)b);
		__m256i xor = _mm256_xor_si256(xor, one);
		if (!_mm256_testz_si256(xor, one))
			return 0;
	}

	return 1;
}
