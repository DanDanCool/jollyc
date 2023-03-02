#pragma once

#include "jtype.h"
#include "jvec.h"

INTERFACE(mem)
{
	void (*vfree)(u8* ptr);
	u8* (*vresize)(u8* ptr);
};

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
typedef struct mem_arena mem_arena;
typedef struct mem_pool mem_pool;
typedef struct mem_list mem_list;
typedef struct mem_header mem_header;

struct mem_header
{
	baseptr* base;
	vmem* vtable;
	u32 size;
	u32 offset;
};

struct mem_block
{
	baseptr* base;
	u32 handle;
	u32 size;
};

VECTOR_DECLARE(mem_block);
VECTOR_DEFINE(mem_block);

// supports dynamically sized allocations
// continuous buffer of memory
struct mem_arena
{
	baseptr base;
	vector(mem_block) heapgc;
	mem_block free;
};

// fixed size allocations
// continuous buffer of memory
struct mem_pool
{
	baseptr base;
	u32 free;
};

VECTOR_DECLARE(baseptr);
VECTOR_DEFINE(baseptr);

// fixed sized allocations, eight byte minimum allocations
// memory not continuous
struct mem_list
{
	vector(baseptr) blocks;
	u64 free;
};

inline u8* MEM_DATA(mem_block* block)
{
	baseptr* base = block->base;
	return vector_at(u8)(base, block->handle * base->size)
}

// GCC language extension
#define MEM_GUARD(TYPE, dtor) TYPE __attribute__((cleanup(dtor)))

// force multiple of 64 allocation size
u8* jolly_alloc(size);

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

void list_init(mem_list* list, u32 size, u32 blocksz);
void list_destroy(mem_list* list);
void list_resize(mem_list* list, u32 size);

mem_block list_alloc(mem_list* list);
void list_free(mem_list* list, mem_block* block);

void vector_cpy16(vector(u8)* dst, vector(u8)* src);
void vector_cpy32(vector(u8)* dst, vector(u8)* src);

void vector_set16(vector(u8)* dst, u32 val);
void vector_set32(vector(u8)* dst, u32 val);

int vector_cmp16(vector(u8)* dst, vector(u8)* src);
int vector_cmp32(vector(u8)* dst, vector(u8)* src);

// unaligned versions
void vector_cpy16u(vector(u8)* dst, vector(u8)* src);
void vector_cpy32u(vector(u8)* dst, vector(u8)* src);

int vector_cmp16u(vector(u8)* dst, vector(u8)* src);
int vector_cmp32u(vector(u8)* dst, vector(u8)* src);
