#pragma once

#include "jtype.h"

enum
{
	BLOCK_16 = 16;
	BLOCK_32 = 32;
	BLOCK_64 = 64;
	BLOCK_128 = 128;
	BLOCK_256 = 256;
	BLOCK_512 = 512;
	BLOCK_1024 = 1024;
	BLOCK_2048 = 2048;
	BLOCK_4096 = 4096;
	BLOCK_8192 = 8192;
};

struct mem_arena_t;
typedef mem_arena_t mem_arena;

typedef struct
{
	mem_arena* arena;
	u32 handle;
	u32 size;
} mem_block;

inline u8* MEM_DATA(mem_block* block)
{
	// if handle is equal to U32_MAX then the data the block points to is stored in arena
	mem_arena* arena = block->arena;
	u8* data = arena->data + block->handle * arena->blocksz;
	return block->handle == U32_MAX ? (u8*)arena : data;
}

inline u32 MEM_SIZE(mem_block* block)
{
	return block->size;
}

typedef struct
{
	mem_block* data;
	u32 free;
	u32 size;
	u32 cap;
	u32 new;
} mem_gc;

struct mem_arena_t
{
	u8* data;
	u32 blocksz;
	u32 size;
	mem_gc gc;
};

void arena_init(mem_arena* arena, u32 size, u32 blocksz);
void arena_destroy(mem_arena* arena);
void arena_resize(mem_arena* arena, u32 size);
void arena_gc(mem_arena* arena);

mem_block arena_alloc(mem_arena* arena, u32 count);
void arena_realloc(mem_block* block, u32 count);
void arena_free(mem_block* block);

mem_block mem_alloc(u32 size);
void mem_realloc(mem_block* block, u32 size);
void mem_free(mem_block* block);

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
