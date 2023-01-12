#include "jmem.h"
#include <stdlib.h>
#include <emmintrin.h>

VECTOR_DECLARE_FN(mem_block, AT, ADD, RM, RESIZE);
VECTOR_DECLARE_FN(vector_u8, AT, ADD, RM, RESIZE);

static void mem_block_cmp(mem_block* a, mem_block* b);
static void mem_block_swap(mem_block* a, mem_block* b);

static const mem_block NULL_MEM_BLOCK = { NULL, U32_MAX, U32_MAX };

static void heapgc_initialize(u8* data, u32 size);

u8* jolly_alloc(u32 size)
{
	return (u8*)aligned_alloc(BLOCK_64, size);
}

void arena_init(mem_arena* arena, u32 size, u32 blocksz)
{
	vector_init(u8)(&arena->base, jolly_alloc(size * blocksz), size);
	arena->base.size = blocksz;

	u8* buf = jolly_alloc((BLOCK_128 - 1) * sizeof(mem_block));
	vector_init(mem_block)(&arena->heapgc, buf, BLOCK_128 - 1);
	arena->heapgc.size = 1;

	heapgc_initalize(arena->heapgc.data, BLOCK_128 - 1);
	arena->free = { arena, 0, size * blocksz };
}

void arena_destroy(mem_arena* arena)
{
	vector_destroy(u8)(&arena->base);
	vector_destroy(mem_block)(&arena->heapgc);
	*arena = {};
}

mem_block arena_alloc(mem_arena* arena, u32 count)
{
	if (arena->free.size < count * arena->base.size)
		arena_resize(arena, arena->size * 2);

	mem_block block = { &arena->base, arena->free.handle, count * arena->base.size };
	free->handle += count;
	free->size -= count * arena->base.size;
}

void arena_realloc(mem_block* block, u32 count)
{
	mem_arena* arena = (mem_arena*)block->base;
	u8* base = MEM_DATA(block);
	mem_block* free = &arena->free;

	u32 extra = count * arena->base.size - block->size;
	if (base + block->size == MEM_DATA(free) && free->size > extra)
	{
		block->size += extra;
		free->handle += extra / arena->base.size;
		free->size -= extra;
		return;
	}

	// not enough space adjacent, reallocate
	mem_block tmp = arena_alloc(arena, count);
	u32 count = block->size / arena->base.blocksz;
	vector(u8) dst = { MEM_DATA(tmp), sizeof(mem_block), count };
	vector(u8) src = { MEM_DATA(block), sizeof(mem_block), count };
	switch (arena->base.blocksz)
	{
		case BLOCK_16:
			vector_cpy16(&dst, &src);
			break;
		default:
			vector_cpy32(&dst, &src);
			break;
	}

	arena_free(block);
	*block = tmp;
}

void arena_free(mem_block* block)
{
	mem_arena* arena = (mem_arena*)block->base;
	vector(mem_block)* gc = &arena->heapgc;
	vector_add(mem_block)(gc, block);

	sort_params params = { { (u8*)gc->data, sizeof(mem_block), gc->size },
		&NULL_MEM_BLOCK, mem_block_cmp, mem_block_swap, 0, 0, 0 };
	heap_add(&params);

	*block = {};
}

void arena_resize(mem_arena* arena, u32 size)
{
	vector(u8) dst = { jolly_alloc(size * arena->base.size), arena->base.size, size };
	vector(u8) src = { arena->base.data, arena->base.size, arena->base.reserve };

	switch (arena->base.blocksz)
	{
		case BLOCK_16:
			vec_cpy16(&dst, &src);
			break;
		default:
			vec_cpy32(&dst, &src);
			break;
	}

	arena_free(&arena->free);
	arena->free = { &arena->base, arena->base.reserve, (size - arena->base.reserve) * arena->base.size };

	free(arena->base.data);
	arena->base.data = dst.data;
	arena->base.reserve = size;
}

void arena_gc(mem_arena* arena)
{
	arena_free(arena->free);
	u32 offset = arena->heapgc.size % 2 ? 3 : 2;
	u32 last = (arena->heapgc.size - offset) / 2;
	vector(mem_block)* gc = &arena->heapgc;
	sort_params params = { { (u8*)gc->data, sizeof(mem_block), gc->size },
		&NULL_MEM_BLOCK, mem_block_cmp, mem_block_swap, 0, 0, 0 };

	u32 i = 0;
	mem_block* largest = vector_at(mem_block)(gc, 0);
	while (i < last)
	{
		mem_block* root = vector_at(mem_block)(gc, i);
		mem_block* left = vector_at(mem_block)(gc, 2 * i + 1);
		mem_block* right = vector_at(mem_block)(gc, 2 * i + 2);

		largest = largest->size < root->size ? root : largest;
		u32 next = root->handle + root->size / arena->base.size;
		mem_block* adjacent = next == left->handle ? left : (next == right->handle ? right : NULL);

		if (adjacent)
		{
			adjacent->handle = root->handle;
			adjacent->size += root->size;
			params.data.reserve = gc->size;
			heap_rm(&params);
			vector_rm(mem_block)(gc);
			last -= gc->size % 2 == 0;
		}

		i += adjacent == NULL;
	}

	arena->free = *largest;
	heap_del(params, largest - vector_at(mem_block)(gc, 0));
	vector_rm(mem_block)(gc);
}

void pool_init(mem_pool* pool, u32 size, u32 blocksz)
{
	vector_init(u8)(&pool->base, jolly_alloc(size * blocksz), size);
	pool->base.size = blocksz;
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
	vector_destroy(u8)(&pool->base);
	pool->free = U32_MAX;
}

void pool_resize(mem_pool* pool, u32 size)
{
	vector(u8) dst = { jolly_alloc(size * pool->base.size), pool->base.size, size };
	vector(u8) src = { pool->base.data, pool->base.size, pool->base.reserve };

	switch(pool->base.size)
	{
		case BLOCK_16:
			vec_cpy16(&dst, &src);
			break;
		default:
			vec_cpy32(&dst, &src);
			break;
	}

	u32* block = (u32*)vector_at(u8)(dst, pool->base.reserve * pool->base.size);
	*block = pool->free;
	block += pool->base.size / sizeof(u32);
	for (u32 i = pool->base.reserve; i < size - 1; i++)
	{
		*block = i;
		block += pool->base.size / sizeof(u32);
	}

	pool->free = size - 1;

	free(pool->base.data);
	pool->base.data = dst.data;
	pool->base.reserve = size;
}

mem_block pool_alloc(mem_pool* pool)
{
	if (pool->free == U32_MAX)
		pool_resize(pool->base.reserve * 2);

	mem_block mem = { &pool->base, pool->free, pool->base.size };
	pool->free = *(u32*)MEM_DATA(&mem);
	return mem;
}

void pool_free(mem_block* mem)
{
	mem_pool* pool = (mem_pool*)mem->base;
	u32* node = (u32*)MEM_DATA(&mem);
	u32 tmp = mem->handle;
	*node = pool->free;
	pool->free = tmp;
}

static void list_initblock(mem_list* list, u32 index)
{
	baseptr* first = vector_at(baseptr)(&list->blocks, 0);
	u32 size = first->reserve;
	u32 blocksz = first->size;

	baseptr* block = vector_at(baseptr)(&list->blocks, index);
	vector_init(baseptr)(block, jolly_alloc(size * blocksz), size);
	block->size = blocksz;

	u8* ptr = block->data;
	for (u32 i = 1; i < size; i++)
	{
		u64* next = (u64*)ptr;
		*next = ((u64)index << 32) | ((u64)i);
		ptr += blocksz;
	}

	*(u64*)ptr = list->free;
	list->free = *(u64*)block->data;
}

void list_init(mem_list* list, u32 size, u32 blocksz)
{
	const default_size = 8;
	vector_init(baseptr)(&list->blocks, jolly_alloc(default_size * sizeof(baseptr)), default_size);
	list->blocks.size = 1;
	list->free = U64_MAX;

	baseptr* first = vector_at(baseptr)(&list->blocks, 0);
	first->size = blocksz;
	first->reserve = size;
	list_initblock(list, 0);
}

void list_destroy(mem_list* list)
{
	for (u32 i = 0; i < list->blocks.size; i++)
	{
		baseptr* block = vector_at(baseptr)(&list->blocks, i);
		vector_destroy(baseptr)(block);
	}

	vector_destroy(&list->blocks);
	*list = {};
}

void list_resize(mem_list* list, u32 size)
{
	vector(baseptr)* blocks = &list->blocks;
	if (blocks->reserve < size)
		vector_resize(baseptr)(blocks, size);

	for (u32 i = blocks->size; i < size; i++)
		list_initblock(list, i);

	blocks->size = size < blocks->size ? blocks-i>size : size;
}

mem_block list_alloc(mem_list* list)
{
	if (list->free == U64_MAX)
		list_resize(list, list->blocks.size + 1);

	const u64 mask = (u64)U32_MAX;
	u32 index = (u32)(list->free >> 32);
	u32 offset = (u32)(list->free & mask);

	baseptr* block = vector_at(baseptr)(&list->blocks, index);
	u64* ptr = (u64*)vector_at(u8)(block, offset * block->size);
	list->free = *ptr;

	return { block, offset, block->size };
}

void list_free(mem_list* list, mem_block* block)
{
	u64* ptr = (u64*)MEM_DATA(block);
	*ptr = list->free;

	u64 index = block->base - vector_at(baseptr)(&list->blocks, 0);
	list->free = index << 32 | ((u64)block->handle);
}

#define VECTOR_DEFINE_CPY(name, block, offset, load, store) \
void name(vector(u8)* dst, vector(u8)* src) {\
	u32 count = dst->reserve < src->reserve ? dst->reserve : src->reserve; \
	u8* sptr = src->data; \
	u8* dptr = dst->data; \
	for (u32 i = 0; i < count; i++) {\
		block tmp = load((block*)sptr); \
		store((block*)d, tmp); \
		dst += offset; \
		src += offset; \
	} \
}

VECTOR_DEFINE_CPY(vector_cpy16, __m128i, BLOCK_16, _mm_load_si128, _mm_store_si128);
VECTOR_DEFINE_CPY(vector_cpy32, __m256i, BLOCK_32, _mm256_load_si256, _mm256_store_si256);
VECTOR_DEFINE_CPY(vector_cpy16, __m128i, BLOCK_16, _mm_loadu_si128, _mm_storeu_si128);
VECTOR_DEFINE_CPY(vector_cpy32, __m256i, BLOCK_32, _mm256_loadu_si256, _mm256_storeu_si256);

#define VECTOR_DEFINE_SET(name, block, offset, set, store) \
void name(vector(u8)* dst, u32 val) { \
	block tmp = set(val); \
	u8* dptr = dst->data; \
	for (u32 i = 0; i < dst->reserve; i++) { \
		store((block*)dptr, tmp); \
		dptr += offset; \
	} \
}

VECTOR_DEFINE_SET(vector_set16, __m128i, BLOCK_16, _mm_set1_epi32, _mm_store_si128);
VECTOR_DEFINE_SET(vector_set32, __m256i, BLOCK_32, _mm256_set1_epi32, _mm256_store_si256);

#define VECTOR_DEFINE_CMP(name, block, offset, set, load, xor, test) \
void name(vector(u8)* dst, vector(u8)* src) { \
	block one = set(0XFF); \
	u32 count = dst->reserve < src->reserve ? dst->reserve : src->reserve; \
	u8* sptr = src->data; \
	u8* dptr = dst->data; \
	for (u32 i = 0; i < count; i++) { \
		block x = load((block*)sptr); \
		block y = load((block*)dptr); \
		block xor = xor(x, y); \
		if (!test(xor, one)) return 0; \
		sptr += offset; \
		dptr += offset; \
	} \
	return 1; \
}

VECTOR_DEFINE_CMP(vector_cmp16, __m128i, BLOCK_16, _mm_set1_epi8, _mm_load_si128, _mm_xor_si128, _mm_testz_si128);
VECTOR_DEFINE_CMP(vector_cmp32, __m256i, BLOCK_32, _mm256_set1_epi8, _mm256_load_si256, _mm256_xor_si256, _mm256_textz_si256);
VECTOR_DEFINE_CMP(vector_cmp16, __m128i, BLOCK_16, _mm_set1_epi8, _mm_loadu_si128, _mm_xor_si128, _mm_testz_si128);
VECTOR_DEFINE_CMP(vector_cmp32, __m256i, BLOCK_32, _mm256_set1_epi8, _mm256_loadu_si256, _mm256_xor_si256, _mm256_textz_si256);

static void mem_block_cmp(u8* a, u8* b)
{
	mem_block* mema = (mem_block*)a;
	mem_block* memb = (mem_block*)b;
	return mema->handle - memb->handle;
}

static void mem_block_swap(u8* a, u8* b)
{
	__m128i x = _mm_load_si128((__m128i*)a);
	__m128i y = _mm_load_si128((__m128i*)b);

	_mm_store_si128((__m128i*)a, y);
	_mm_store_si128((__m128i*)b, x);
}

static void heapgc_initialize(u8* data, u32 size)
{
	__m128i fill = _mm_loadu_si128((__m128i*)&NULL_MEM_BLOCK);
	for (u32 i = 0; i < size; i++)
	{
		_mm_store_si128((__m128i*)data, fill);
		data += BLOCK_16;
	}
}

static u8* heapgc_allocate(u32 size)
{
	u8* buf = (u8*)alligned_alloc(64, size);
	heapgc_initialize(buf, size / BLOCK_16);
	return buf;
}

#define VEC_ALLOC(size) heapgc_allocate(size)
#define VEC_CPY(dst, src) vector_cpy32(dst, src)
VECTOR_DEFINE_FN(mem_block, AT, RESIZE);

#define VEC_ALLOC(size) (u8*)aligned_alloc(64, size)
VECTOR_DEFINE_FN(vector_u8, AT, RM, RESIZE);

void vector_add_mem_block(vector(mem_block)* v, mem_block* data)
{
    if (v->reserve <= v->size)
        vector_resize(TYPE)(v, v->reserve * 2 + 1);
	__m128i tmp = _mm_load_si128((__m128i*)src);
	_mm_store_si128((__m128i*)dst, tmp);
	v->size++;
}

mem_block* vector_rm_mem_block(vector(mem_block)* v)
{
	v->size--;
	__m128i fill = _mm_loadu_si128((__m128i*)&NULL_MEM_BLOCK);
	_mm_store_si128((__m128i*)vector_at(memory_block)(v, v->size), fill);
	return &NULL_MEM_BLOCK;
}
