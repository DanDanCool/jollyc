#pragma once

#include "memory.h"


typedef void (*pfn_copy)(u8* a, u8* b);
typedef void (*pfn_swap)(u8* a, u8* b);
typedef int (*pfn_eq)(u8* a, u8* b);
typedef int (*pfn_le)(u8* a, u8* b);
typedef int (*pfn_lt)(u8* a, u8* b);

#define copy(type) copy_##type
#define _copy(type) _copy_##type
#define swap(type) swap_##type
#define _swap(type) _swap_##type
#define hash(type) hash_##type
#define eq(type) eq_##type
#define _eq(type) _eq_##type
#define le(type) le_##type
#define _le(type) _le_##type
#define lt(type) lt_##type
#define _lt(type) _lt_##type
#define str(type) str_##type

#define COPY_DECLARE(type) \
void copy(type)(type* src, type* dst); \
void _copy(type)(u8* src, u8* dst)

#define COPY_DEFINE(type) \
void copy(type)(type* src, type* dst) { \
	*dst = *src; \
} \
void _copy(type)(u8* src, u8* dst) { \
	copy(type)((type*)src, (type*)dst); \
}

#define SWAP_DECLARE(type) \
void swap(type)(type* src, type* dst); \
void _swap(type)(u8* src, u8* dst)

#define SWAP_DEFINE(type) \
void swap(type)(type* src, type* dst) { \
	type tmp = {0}; \
	copy(type)(src, &tmp); \
	copy(type)(dst, src); \
	copy(type)(&tmp, dst); \
} \
void _swap(type)(u8* src, u8* dst) { \
	swap(type)((type*)src, (type*)dst); \
}

#define MIN(x, y) (x) < (y) ? (x) : (y)
#define MAX(x, y) (x) > (y) ? (x) : (y)

#define HASH_DECLARE(type) \
u32 hash(type)(type key)

#define HASH_DEFINE(type) \
u32 hash(type)(type key) { \
	return fnv1a((u8*)&key, sizeof(type)); \
}

#define EQ_DECLARE(type) \
int eq(type)(type* a, type* b); \
int _eq(type)(u8* a, u8* b)

#define EQ_DEFINE(type) \
int eq(type)(type* a, type* b) { \
	if (!a || !b) return false; \
	return *a == *b; \
} \
int _eq(type)(u8* a, u8* b) { \
	return eq(type)((type*)a, (type*)b); \
}

#define LE_DECLARE(type) \
int le(type)(type* a, type* b); \
int _le(type)(u8* a, u8* b)

#define LE_DEFINE(type) \
int le(type)(type* a, type* b) { \
	if (!a || !b) return false; \
	return *a <= *b; \
} \
int _le(type)(u8* a, u8* b) { \
	return le(type)((type*)a, (type*)b); \
}

#define LT_DECLARE(type) \
int lt(type)(type* a, type* b); \
int _lt(type)(u8* a, u8* b)

#define LT_DEFINE(type) \
int lt(type)(type* a, type* b) { \
	if (!a || !b) return false; \
	return *a < *b; \
} \
int _lt(type)(u8* a, u8* b) { \
	return lt(type)((type*)a, (type*)b); \
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

SWAP_DECLARE(u8);
SWAP_DECLARE(u16);
SWAP_DECLARE(u32);
SWAP_DECLARE(u64);
SWAP_DECLARE(i8);
SWAP_DECLARE(i16);
SWAP_DECLARE(i32);
SWAP_DECLARE(i64);
SWAP_DECLARE(f32);
SWAP_DECLARE(f64);

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

LE_DECLARE(u8);
LE_DECLARE(u16);
LE_DECLARE(u32);
LE_DECLARE(u64);
LE_DECLARE(i8);
LE_DECLARE(i16);
LE_DECLARE(i32);
LE_DECLARE(i64);
LE_DECLARE(f32);
LE_DECLARE(f64);

LT_DECLARE(u8);
LT_DECLARE(u16);
LT_DECLARE(u32);
LT_DECLARE(u64);
LT_DECLARE(i8);
LT_DECLARE(i16);
LT_DECLARE(i32);
LT_DECLARE(i64);
LT_DECLARE(f32);
LT_DECLARE(f64);
