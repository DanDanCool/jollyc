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

typedef struct
{
	uint8* data;
	uint32 size;
} mem_block;

#define MEM_DATA(mem) mem.data
#define MEM_SIZE(mem) mem.size

typedef struct
{
	uint8* data;

	struct
	{
		mem_block* data;
		uint32 size;
		uint32 cap;
	} free;

	uint32 size;
	uint32 blocksz;
} mem_arena;

void arena_init(mem_arena* mem, uint32 size, uint32 blocksz);
void arena_destroy(mem_arena* mem);

mem_block arena_alloc(mem_arena* mem, uint32 count);
void arena_realloc(mem_arena* mem, mem_block* block, uint32 count);
void arena_free(mem_arena* mem, mem_block* block);
void arena_gc(mem_arena* mem);

mem_block mem_alloc(uint32 size);
void mem_realloc(mem_block* block, uint32 size);

void mem_cpy16(uint8* dst, uint8* src, uint32 count);
void mem_cpy32(uint8* dst, uint8* src, uint32 count);

void mem_set16(uint8* dst, uint32 val, uint32 count);
void mem_set32(uint8* dst, uint32 val, uint32 count);

int mem_cmp16(uint8* a, uint8* b, uint32 count);
int mem_cmp32(uint8* a, uint8* b, uint32 count);

// unaligned versions
void mem_cpy16u(uint8* dst, uint8* src, uint32 count);
void mem_cpy32u(uint8* dst, uint8* src, uint32 count);

int mem_cmp16u(uint8* a, uint8* b, uint32 count);
int mem_cmp32u(uint8* a, uint8* b, uint32 count);
