#include "jmem.h"
#include <stdlib.h>
#include <emmintrin.h>

#define ALIGN_SIZE(sz) ((((size) - 1) / BLOCK_32 + 1) * BLOCK_32)

VECTOR_DECLARE_FN(mem_block, AT, ADD, RM, RESIZE);
VECTOR_DECLARE_FN(vector_u8, AT, ADD, RM, RESIZE);

static void mem_block_cmp(mem_block* a, mem_block* b);
static void mem_block_swap(mem_block* a, mem_block* b);

static const mem_block NULL_MEM_BLOCK = { NULL, U32_MAX, U32_MAX };

static void heapgc_initialize(u8* data, u32 size);

INTERFACE_IMPL(mem, block) {
	mem_free,
	mem_resize
};

u8* mem_alloc(u32 size) {
	size = ALIGN_SIZE(SZ);
	mem_header* header = (mem_header*)aligned_alloc(DEFAULT_ALIGNMENT, size);
	header->base = (void*)header;
	header->vtable = interface_impl(mem, block);
	header->offset = sizeof(mem_header);
	header->size = size;
	return (u8*)header + sizeof(mem_header);
}

u8* mem_resize(u8* mem, u32 size) {
	mem_header* header = mem_getheader(mem);

	// TODO: investigae whether realloc reliably returns the same address in this case
	if (size < header->size)
		return mem;

	u8* tmp = mem_alloc(size);
	mem_cpy(tmp, mem);
	mem_free(mem);
	return tmp;
}

void mem_free(u8* mem) {
	mem_header* header = mem_getheader(mem);
	free(header->base);
}

u64 mem_size(u8* mem) {
	mem_header* header = mem_getheader(mem);
	return header->size;
}

void mem_cpy(u8* dst, u8* src) {
	mem_header* dstheader = mem_getheader(dst);
	mem_header* srcheader = mem_getheader(src);
	u32 count = (dstheader->size < srcheader->size ? dstheader->size : srcheader->size) / BLOCK_32;

	for (u32 i = 0; i < count; i++) {
		__m256i tmp = _mm256_load_si256((__m256i*)src);
		_mm256_store_si256((__m256i*)dst, tmp);
		dst += BLOCK_32;
		src += BLOCK_32;
	}
}

void mem_set(u8* dst, u32 val) {
	mem_header* header = mem_getheader(dst);
	u32 count = header->size / BLOCK_32;
	const __m256i tmp = _mm256_set1_epi32(val);

	for (u32 i = 0; i < count; i++) {
		_mm256_store_si256((__m256i*)dst, tmp);
		dst += BLOCK_32;
	}
}

mem_header* mem_getheader(u8* mem) {
	return (mem_header*)(mem - sizeof(mem_header));
}

u8* mem_vresize(u8* mem, u32 size) {
	mem_header* header = mem_getheader(mem);
	return header->vdispatch(vresize)(mem, size);
}

u8* mem_vfree(u8* mem) {
	mem_header* header = mem_getheader(mem);
	header->vdispatch(vfree)(mem);
}

void arena_init(mem_arena* arena) {
	mem_header* root = mem_alloc(BLOCK_4096);
	*root = { (u8*)root, interface_impl(mem, arena), sizeof(mem_header), BLOCK_4096 - sizeof(mem_header) };
	u32* magic = (u32*)((u8*)root + sizeof(mem_header));
	*magic = MAGIC;
	u32 default_size = 8;
	vector_init(u8*)(&arena->blocks, mem_alloc(default_size * sizeof(u8*)));
	vector_add(u8*)(&arena->blocks, root->base);
	rbtree_init(mem_header)(&arena->free, &root);
}

void arena_destroy(mem_arena* arena) {
	rbtree_destroy(&arena->free);
	vector_destroy(&arena->blocks);
	*arena = {};
}

u8* arena_alloc(mem_arena* arena, u32 size) {
	size = ALIGN_SIZE(size + sizeof(mem_header));
	assert(size < BLOCK_4096); // maximum size limit
	mem_header cmp = { NULL, NULL, 0, size };
	u32 h = rbtree_best(mem_header*)(&arena->free, &cmp);
	mem_header* free = *rbtree_at(mem_header)(&arena->free, h);

	if (free->size < size) {
		arena_resize(arena, arena->blocks.size + 1);
		h = rbtree_best(mem_header)(&arena->free, &cmp);
		free = rbtree_at(mem_header)(&arena->free, h);
	}

	rbtree_delete(&arena->free, h);
	if (size + sizeof(mem_header) < free->size) {
		mem_header* tmp = (u8*)free + size;
		*tmp = { free->base, interface_impl(mem, arena), free->offset + size, free->size - size - sizeof(mem_header) };
		rbtree_add(mem_header*)(&arena->free, &tmp);
		free->size -= tmp->size;
	}

	u32* block = free + sizeof(mem_header);
	*block = 0;
	return block;
}

u8* arena_realloc(mem_arena* arena, u8* block, u32 size) {
	// we could probably see if there's space adjacent but it involves a tree lookup which might be slower
	u8* new = arena_alloc(arena, size);
	mem_cpy(new, block);
	rbtree_add(mem_header*)(&arena->free, &mem_getheader(block));
	return new;
}

void arena_free(mem_arena* arena, u8* block) {
	*block = MAGIC;
	rbtree_add(mem_header*)(&arena->free, &mem_getheader(block));
}

void arena_resize(mem_arena* arena, u32 size) {
	for (u32 i = arena->blocks.size; i < size; i++) {
		mem_header* block = mem_alloc(BLOCK_4096);
		*block = { (u8*)root, interface_impl(mem, arena), sizeof(mem_header), BLOCK_4096 - sizeof(mem_header) };
		u32* magic = (u32*)((u8*)block + sizeof(mem_header));
		*magic = MAGIC;
		vector_add(u8*)(&arena->blocks, block->base);
		rbtree_add(mem_header*)(&arena->free, &block);
	}
}

void arena_gc(mem_arena* arena) {
	const u32 MAX_POS = 64;
	const MM256 INDICES = { 0 * 32, 1 * 32, 2 * 32, 3 * 32, 4 * 32, 5 * 32, 6 * 32, 7 * 32 };

	u64 mem[MAX_POS];
	vector(u64) pos = { mem, 0, 64 };
	u32 magic = MAGIC;

	for (u32 i = 0; i < arena->blocks.size; i++) {
		u8* block = vector_at(&arena->blocks, i);

		u8* chunk = block;
		__m256i cmp = _mm256_set1_epi32(magic);
		__m256i indices = _mm256_load_si256((__m256i*)INDICES);
		for (u32 j = 0; j < BLOCK_4096/BLOCK_256; j++) {
			__m256i x = _mm256_load_si256((__m256i*)chunk);
			__m256i mask = _mm256_cmpeq_epi32(x, cmp);

			__m256i y = _mm256_set1_epi32(j * BLOCK_256);
			y = _mm256_add_epi32(y, indices);

			u32 position[8] = {};
			_mm256_maskstore_epi32(&position, mask, y);
			for (u32 k = 0; k < 8; k++) {
				if (position[k] == 0) continue;
				u32 index = i << 32 | position[k];
				vector_add(u64)(&pos, &index);
			}
		}

		u32 block = (u32)(*vector_at(u64)(&pos, 0) >> 32);
		u32 offset = (u32)(*vector_at(u64)(&pos, 0) & U32_MAX);
		mem_header* header = mem_getheader(*vector_at(u8*)(&arena->blocks, block) + offset);
		u32 flush = false;
		for (u32 i = 1; i < pos.size; i++) {
			block = (u32)(*vector_at(u64)(&pos, i) >> 32);
			offset = (u32)(*vector_at(u64)(&pos, i) & U32_MAX);
			mem_header* tmp = mem_getheader(*vector_at(u8*)(&arena->blocks, block) + offset);

			if (header->offset + header->size == tmp->offset) {
				*(u32*)((u8*)tmp + sizeof(mem_header)) = 0;
				h = rbtree_find(mem_header*)(&arena->free, &tmp);
				rbtree_del(&arena->free, h);
				header->size += tmp->size;
				flush = true;
			} else {
				rbtree_add(mem_header*)(&arena->free, &header);
				header = tmp;
				flush = false;
			}
		}

		if (flush) rbtree_add(mem_header*)(&arena->free, &header);
		pos.size = 0;
	}
}

void pool_init(mem_pool* pool, u32 size, u32 blocksz) {
	vector_init(u8)(&pool->base, jolly_alloc(size * blocksz));
	pool->blocksz = blocksz;
	pool->free = 0;

	u32* block = (u32*)vector_at(u8)(pool->data, 0);
	for (u32 i = 1; i < size; i++) {
		*block = i;
		block += blocksz / sizeof(u32);
	}

	*block = U32_MAX;
}

void pool_destroy(mem_pool* pool) {
	vector_destroy(&pool->base);
	pool->free = U32_MAX;
}

void pool_resize(mem_pool* pool, u32 size) {
	u64 reserve = mem_size(pool->base.data) & VECTOR_RESERVE_MASK;
	vector_resize(pool->base, size);

	u32* block = (u32*)pool_at(pool, reserve * pool->blocksz);
	*block = pool->free;
	block += pool->blocksz / sizeof(u32);
	for (u32 i = reserve; i < size - 1; i++) {
		*block = i;
		block += pool->blocksz / sizeof(u32);
	}

	pool->free = size - 1;
}

u8* pool_at(mem_pool* pool, u32 handle) {
	return vector_at(u8)(pool->base, handle * pool->blocksz);
}

u32 pool_alloc(mem_pool* pool) {
	if (pool->free == U32_MAX)
		pool_resize(pool->base.reserve * 2);

	u32 tmp = pool->free;
	pool->free = *(u32*)pool_at(tmp);
	return tmp;
}

void pool_free(mem_pool* pool, u32 handle) {
	*(u32*)pool_at(pool, handle) = pool->free;
	pool->free = handle;
}

static void list_initblock(mem_list* list, u32 index) {
	baseptr* first = vector_at(baseptr)(&list->blocks, 0);
	const u32 size = BLOCK_64;

	baseptr* block = vector_at(baseptr)(&list->blocks, index);
	vector_init(baseptr)(block, mem_alloc(size * blocksz));

	u8* ptr = block->data;
	for (u32 i = 1; i < size; i++) {
		u32* next = (u32*)ptr;
		*next = (index << 16) | i;
		ptr += list->blocksz;
	}

	*(u32*)ptr = list->free;
	list->free = *(u32*)block->data;
}

void list_init(mem_list* list, u32 blocksz, u32 flags) {
	const default_size = 8;
	blocksz += sizeof(mem_header) * (flags & ALLOC_VTABLE) == ALLOC_VTABLE;
	vector_init(baseptr)(&list->blocks, mem_alloc(default_size * sizeof(baseptr)));
	list->free = U32_MAX;
	list->blocksz = blocksz;

	list_initblock(list, 0);
}

void list_destroy(mem_list* list) {
	for (u32 i = 0; i < list->blocks.size; i++) {
		baseptr* block = vector_at(baseptr)(&list->blocks, i);
		vector_destroy(block);
	}

	vector_destroy(&list->blocks);
	*list = {};
}

void list_resize(mem_list* list, u32 size) {
	vector(baseptr)* blocks = &list->blocks;
	if (blocks->reserve < size)
		vector_resize(blocks, size);

	for (u32 i = blocks->size; i < size; i++)
		list_initblock(list, i);

	blocks->size = size < blocks->size ? blocks->size : size;
}

u8* list_alloc(mem_list* list) {
	if (list->free == U64_MAX)
		list_resize(list, list->blocks.size + 1);

	u32* ptr = (u32*)list_at(list, list->free);
	u32 tmp = list->free;
	list->free = *ptr;

	mem_header* header = (mem_header*)ptr;
	header->base = (void*)list;
	header->vtable = interface_impl(mem, list);
	header->offset = tmp;
	header->size = list->blocksz;

	return (u8*)header + sizeof(mem_header);
}

void list_free(mem_list* list, u8* block) {
	mem_header* header = mem_getheader(block);
	u64 tmp = header->offset;

	u32* ptr = (u32*)header;
	*ptr = list->free;
	list->free = tmp;
}

u64 list_halloc(mem_list* list) {
	if (list->free == U64_MAX)
		list_resize(list, list->blocks.size + 1);

	u32* ptr = (u32*)list_at(list, list->free);
	u64 tmp = list->free;
	list->free = *ptr;

	return tmp;
}

void list_hfree(mem_list* list, u64 handle) {
	u64* ptr = (u64*)list_at(list, handle);
	*ptr = list->free;
	list->free = handle;
}

u8* list_at(mem_list* list, u64 handle) {
	const u32 mask = U16_MAX;
	u32 index = handle >> 16;
	u32 offset = handle & mask;

	baseptr* block = vector_at(baseptr)(&list->blocks, index);
	return vector_at(u8)(block, offset * block->size);
}
