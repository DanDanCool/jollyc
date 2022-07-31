#pragma once

#include "jtype.h"
#include "jmem.h"

// recommended to use a block size of at least 32
typedef struct
{
	mem_block data;
	uint32 size;
} string;

// last argument should be null, 32 strings max
void str_cat(mem_arena* mem, string* str, ...);
u32 str_len(cstr str);
int str_cmp(string* s1, string* s2);
