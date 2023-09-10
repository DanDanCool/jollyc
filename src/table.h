#pragma once

#include "macros.h"
#include "vector.h"

typedef struct table table_;
struct table {
	vector_ hash;
	vector_ keys;
	vector_ items;
	u32 reserve;
	u32 probe;
};

typedef struct hash_ hash_;
struct hash_ {
	u32 hash;
	u32 index;
};

#define table(k, v) table_
#define table_create(K, V) table_create_##K##_##V
#define table_resize(K, V) table_resize_##K##_##V
#define table_destroy(K, V) table_destroy_##K##_##V
#define table_clear(K, V) table_clear_##K##_##V
#define table_set(K, V) table_set_##K##_##V
#define table_get(K, V) table_get_##K##_##V
#define table_del(K, V) table_del_##K##_##V

#define TABLE_DECLARE_CREATE(K, V) \
void table_create(K, V)(table_* t, u32 size)

#define TABLE_DECLARE_RESIZE(K, V) \
void table_resize(K, V)(table_* t, u32 size)

#define TABLE_DECLARE_DESTROY(K, V) \
void table_destroy(K, V)(table_* t)

#define TABLE_DECLARE_CLEAR(K, V) \
void table_clear(K, V)(table_* t)

#define TABLE_DECLARE_SET(K, V) \
void table_set(K, V)(table_* t, K key, V* value)

#define TABLE_DECLARE_GET(K, V) \
V* table_get(K, V)(table_* t, K key)

#define TABLE_DECLARE_DEL(K, V) \
void table_del(K, V)(table_* t, K key)

#define TABLE_DECLARE(K, V) \
EXPAND4(DEFER(TABLE_DECLARE_CREATE)(K, V); \
        DEFER(TABLE_DECLARE_RESIZE)(K, V); \
        DEFER(TABLE_DECLARE_DESTROY)(K, V); \
        DEFER(TABLE_DECLARE_CLEAR)(K, V); \
        DEFER(TABLE_DECLARE_GET)(K, V); \
        DEFER(TABLE_DECLARE_SET)(K, V); \
        DEFER(TABLE_DECLARE_DEL)(K, V);)

#define TABLE_DEFINE_CREATE(K, V) \
void table_create(K, V)(table_* t, u32 size) { \
	table_create_(t, sizeof(K), sizeof(V), size); \
}

#define TABLE_DEFINE_RESIZE(K, V) \
void table_resize(K, V)(table_* t, u32 size) { \
	table_resize_(t, sizeof(K), size); \
}

#define TABLE_DEFINE_DESTROY(K, V) \
void table_destroy(K, V)(table_* t) { \
	table_destroy_(t); \
}

#define TABLE_DEFINE_SET(K, V) \
void table_set(K, V)(table_* t, K key, V* value) { \
    V* cur = table_get(K, V)(t, key); \
    if (cur) { \
        copy(V)(value, cur); \
        return; \
    } \
	u32 idx = t->items.size; \
	hash_ hash = { hash(K)(key), idx }; \
	memptr k = { (u8*)&key, sizeof(K) }; \
	table_probe_(t, k, hash); \
    table_resize_(t, sizeof(K), t->reserve * 2); \
	vector_add(V)(&t->items, value); \
}

#define TABLE_DEFINE_GET(K, V) \
V* table_get(K, V)(table_* t, K key) { \
	hash_ hash = { hash(K)(key), 0 }; \
	memptr k = { (u8*)&key, sizeof(K) }; \
	u32 idx = table_find_(t, k, hash, _eq(K)); \
	if (idx == U32_MAX) return NULL; \
	hash = *(hash_*)vector_at(u64)(&t->hash, idx); \
	return vector_at(V)(&t->items, hash.index); \
}

#define TABLE_DEFINE_DEL(K, V) \
void table_del(K, V)(table_* t, K key) { \
	hash_ hash = { hash(K)(key), 0 }; \
	memptr k = { (u8*)&key, sizeof(K) }; \
	u32 idx = table_find_(t, k, hash, _eq(K)); \
	if (idx == U32_MAX) return; \
	hash_* h = (hash_*)vector_at(u64)(&t->hash, idx); \
	vector_del(V)(&t->items, h->index); \
    zero8((u8*)h, sizeof(hash_)); \
	zero8((u8*)vector_at(K)(&t->keys, idx), sizeof(K)); \
}

#define TABLE_DEFINE(K, V) \
EXPAND4(DEFER(TABLE_DEFINE_CREATE)(K, V); \
        DEFER(TABLE_DEFINE_RESIZE)(K, V); \
        DEFER(TABLE_DEFINE_DESTROY)(K, V); \
        DEFER(TABLE_DEFINE_GET)(K, V); \
        DEFER(TABLE_DEFINE_SET)(K, V); \
        DEFER(TABLE_DEFINE_DEL)(K, V);)

void table_create_(table_* t, u32 keysize, u32 itemsize, u32 size);
void table_resize_(table_* t, u32 keysize, u32 size);
void table_destroy_(table_* t);
void table_clear_(table_* t);
void table_probe_(table_* t, memptr key, hash_ hash);
u32 table_find_(table_* t, memptr key, hash_ hash, pfn_eq eq_fn);
