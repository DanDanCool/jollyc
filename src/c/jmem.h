#pragma once

#include "jtype.h"
#include "jvec.h"

INTERFACE(mem) {
	void (*vfree)(u8* ptr);
	u8* (*vresize)(u8* ptr, u32 size);
};

INTERFACE_DECLARE(mem, block);
INTERFACE_DECLARE(mem, arena);
INTERFACE_DECLARE(mem, list);

enum {
	BLOCK_32 = 32,
	BLOCK_64 = 64,
	BLOCK_128 = 128,
	BLOCK_256 = 256,
	BLOCK_512 = 512,
	BLOCK_1024 = 1024,
	BLOCK_2048 = 2048,
	BLOCK_4096 = 4096,
	BLOCK_8192 = 8192,

	DEFAULT_ALIGNMENT = BLOCK_32,
};

enum
{
	ALLOC_VTABLE = 1 << 0,
};

typedef vector(u8) baseptr;
typedef struct mem_arena mem_arena;
typedef struct mem_pool mem_pool;
typedef struct mem_list mem_list;
typedef struct mem_header mem_header;

struct mem_header {
	void* base;
	vmem* vtable;
	u64 offset;
	u64 size;
};

// 32 byte alignment
u8* mem_alloc(u32 size);
u8* mem_resize(u8* mem, u32 size);
void mem_free(u8* mem);

u64 mem_size(u8* mem);
void mem_cpy(u8* dst, u8* src);
void mem_set(u8* dst, u32 val)
mem_header* mem_getheader(u8* mem);

u8* mem_vresize(u8* mem, u32 size);
void mem_vfree(u8* mem);

// fixed size allocations
// continuous buffer of memory
struct mem_pool {
	baseptr base;
	u32 blocksz;
	u32 free;
};

// fixed sized allocations, eight byte minimum allocations
// memory not continuous
struct mem_list {
	vector(baseptr) blocks;
	u32 free;
	u32 blocksz;
};

// supports dynamically sized allocations
struct mem_arena {
	rbtree(mem_header*) free;
	vector(u8*) blocks;
};

// GCC language extension
#define MEM_GUARD(TYPE, dtor) TYPE __attribute__((cleanup(dtor)))

void arena_init(mem_arena* arena);
void arena_destroy(mem_arena* arena);
void arena_resize(mem_arena* arena, u32 size);
void arena_gc(mem_arena* arena);

u8* arena_alloc(mem_arena* arena, u32 size);
u8* arena_realloc(mem_arena* arena, u8* block, u32 size);
void arena_free(mem_arena* arena, u8* block);

void pool_init(mem_pool* pool, u32 size, u32 blocksz);
void pool_destroy(mem_pool* pool);
void pool_resize(mem_pool* pool, u32 size);

u8* pool_at(mem_pool* pool, u32 handle);
u32 pool_alloc(mem_pool* pool);
void pool_free(mem_pool* pool, u32 handle);

void list_init(mem_list* list, u32 blocksz, u32 flags);
void list_destroy(mem_list* list);
void list_resize(mem_list* list, u32 size);

u8* list_alloc(mem_list* list);
void list_free(mem_list* list, u8* block);

u32 list_halloc(mem_list* list);
void list_hfree(mem_list*, u32 handle);
u8* list_at(mem_list* list, u32 handle);
