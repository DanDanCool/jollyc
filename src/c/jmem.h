#pragma once

#include "jtype.h"
#include "jvec.h"

enum
{
	BLOCK_16 = 16,
	BLOCK_32 = 32,
	BLOCK_64 = 64,
	BLOCK_128 = 128,
	BLOCK_256 = 256,
	BLOCK_512 = 512,
	BLOCK_1024 = 1024,
	BLOCK_2048 = 2048,
	BLOCK_4096 = 4096,
	BLOCK_8192 = 8192,
};

typedef vector(u8) baseptr;
typedef struct mem_block mem_block;
typedef struct mem_ptr mem_ptr;
typedef struct mem_arena mem_arena;
typedef struct mem_pool mem_pool;
typedef struct mem_gc mem_gc;

struct mem_block
{
	baseptr* base;
	u32 handle;
	u32 size;
};

VECTOR_DEFINE(mem_block);

struct mem_arena
{
	baseptr base;
	vector(mem_block) heapgc;
	mem_block free;
};

struct mem_pool
{
	baseptr base;
	u32 free;
};

inline u8* MEM_DATA(mem_block* block)
{
	return vector_at(u8)(owner, block->handle * owner->size)
}

inline u32 MEM_SIZE(mem_block* block)
{
	return block->size;
}

void arena_init(mem_arena* arena, u32 size, u32 blocksz);
void arena_destroy(mem_arena* arena);
void arena_resize(mem_arena* arena, u32 size);
void arena_gc(mem_arena* arena);

mem_block arena_alloc(mem_arena* arena, u32 count);
void arena_realloc(mem_block* block, u32 count);
void arena_free(mem_block* block);

void pool_init(mem_pool* pool, u32 size, u32 blocksz);
void pool_destroy(mem_pool* pool);
void pool_resize(mem_pool* pool, u32 size);

mem_block pool_alloc(mem_pool* pool);
void pool_free(mem_block* block);

mem_ptr mem_alloc(u32 size);
void mem_realloc(mem_ptr* ptr, u32 size);
void mem_free(mem_ptr* block);

void mem_cpy16(u8* dst, u8* src, u32 count);
void mem_cpy32(u8* dst, u8* src, u32 count);

void mem_set16(u8* dst, u32 val, u32 count);
void mem_set32(u8* dst, u32 val, u32 count);

int mem_cmp16(u8* a, u8* b, u32 count);
int mem_cmp32(u8* a, u8* b, u32 count);

// unaligned versions
void mem_cpy16u(u8* dst, u8* src, u32 count);
void mem_cpy32u(u8* dst, u8* src, u32 count);

int mem_cmp16u(u8* a, u8* b, u32 count);
int mem_cmp32u(u8* a, u8* b, u32 count);
