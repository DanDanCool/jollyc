#pragma once

#include "types.h"

#ifndef NULL
#define NULL (void*)0
#endif

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

typedef struct memptr memptr;
struct memptr {
	u8* data;
	u64 size;
};

u32 align_size256(u32 size);
memptr alloc256(u32 size);
memptr alloc8(u32 size);
void free256(void* ptr);
void free8(void* ptr);
void copy256(u8* src, u8* dst, u32 bytes);
void zero256(u8* dst, u32 bytes);
void copy8(u8* src, u8* dst, u32 bytes);
void zero8(u8* dst, u32 bytes);
