#pragma once

#include "memory.h"

typedef int (*pfn_eq)(u8* a, u8* b);

#define copy(type) copy_##type
#define hash(type) hash_##type
#define eq(type) eq_##type

#define COPY_DECLARE(type) \
void copy(type)(type* src, type* dst)

#define COPY_DEFINE(type) \
void copy(type)(type* src, type* dst) { \
	*dst = *src; \
}

#define MIN(x, y) (x) < (y) ? (x) : (y)
#define MAX(x, y) (x) > (y) ? (x) : (y)

#define HASH_DECLARE(type) \
u32 hash(type)(type* key)

#define HASH_DEFINE(type) \
u32 hash(type)(type* key) { \
	return fnv1a((u8*)key, sizeof(type)); \
}

#define EQ_DECLARE(type) \
int eq(type)(u8* a, u8* b)

#define EQ_DEFINE(type) \
int eq(type)(u8* a, u8* b) { \
	type* a1 = (type*)a; \
	type* b1 = (type*)b; \
	return *a1 == *b1; \
}

u32 fnv1a(u8* key, u32 bytes);

COPY_DECLARE(u8);
COPY_DECLARE(u16);
COPY_DECLARE(u32);
COPY_DECLARE(u64);
COPY_DECLARE(i8);
COPY_DECLARE(i16);
COPY_DECLARE(i32);
COPY_DECLARE(i64);
COPY_DECLARE(f32);
COPY_DECLARE(f64);

HASH_DECLARE(u8);
HASH_DECLARE(u16);
HASH_DECLARE(u32);
HASH_DECLARE(u64);
HASH_DECLARE(i8);
HASH_DECLARE(i16);
HASH_DECLARE(i32);
HASH_DECLARE(i64);
HASH_DECLARE(f32);
HASH_DECLARE(f64);

EQ_DECLARE(u8);
EQ_DECLARE(u16);
EQ_DECLARE(u32);
EQ_DECLARE(u64);
EQ_DECLARE(i8);
EQ_DECLARE(i16);
EQ_DECLARE(i32);
EQ_DECLARE(i64);
EQ_DECLARE(f32);
EQ_DECLARE(f64);
