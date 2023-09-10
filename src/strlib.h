#pragma once

#include "types.h"
#include "memory.h"
#include "vector.h"
#include "table.h"
#include "operations.h"

typedef struct memptr string;

string string_create(cstr str);
char string_at(string str, int idx);
string string_cat(string s1, string s2);
string string_combine(vector_ strings);
vector_ string_split(string str, const char* delim);
string string_substr(string str, u32 beg, u32 end);
void string_destroy(string* str);

COPY_DECLARE(string);
SWAP_DECLARE(string);
HASH_DECLARE(string);
EQ_DECLARE(string);

VECTOR_DECLARE(string);

#define STRTABLE_DEFINE_DESTROY(V) \
void table_destroy(string, V)(table_* t) { \
	strtable_destroy_(t); \
}

#define STRTABLE_DEFINE_CLEAR(V) \
void table_clear(string, V)(table_* t) { \
	strtable_clear_(t); \
}

#define STRTABLE_DEFINE_SET(V) \
void table_set(string, V)(table_* t, string key, V* value) { \
    V* cur = table_get(string, V)(t, key); \
    if (cur) { \
        copy(V)(value, cur); \
        return; \
    } \
	string str = string_create(key.data); \
	strtable_set_(t, str); \
	vector_add(V)(&t->items, value); \
}

#define CSTRTABLE_DEFINE_SET(V) \
void table_set(cstr, V)(table_* t, cstr key, V* value) { \
	string str = string_create(key); \
	V* cur = table_get(string, V)(t, str); \
	if (cur) { \
		copy(V)(value, cur); \
		string_destroy(&str); \
		return; \
	} \
	strtable_set_(t, str); \
	vector_add(V)(&t->items, value); \
}

#define CSTRTABLE_DEFINE_GET(V) \
V* table_get(cstr, V)(table_* t, cstr key) { \
	string str = string_create(key); \
	V* val = table_get(string, V)(t, str); \
	string_destroy(&str); \
	return val; \
}

#define STRTABLE_DEFINE_DEL(V) \
void table_del(string, V)(table_* t, string key) { \
	u32 idx = strtable_del_(t, key); \
	if (idx == U32_MAX) return; \
	vector_del(V)(&t->items, idx); \
}

#define CSTRTABLE_DEFINE_DEL(V) \
void table_del(cstr, V)(table_* t, cstr key) { \
	string str = string_create(key); \
	table_del(string, V)(t, str); \
	string_destroy(&str); \
}

#define STRTABLE_DECLARE(V) \
void table_set(cstr, V)(table_* t, cstr key, V* value); \
V* table_get(cstr, V)(table_* t, cstr key); \
void table_del(cstr, V)(table_* t, cstr key); \
EXPAND4(DEFER(TABLE_DECLARE_CREATE)(string, V); \
        DEFER(TABLE_DECLARE_RESIZE)(string, V); \
        DEFER(TABLE_DECLARE_DESTROY)(string, V); \
        DEFER(TABLE_DECLARE_CLEAR)(string, V); \
        DEFER(TABLE_DECLARE_GET)(string, V); \
        DEFER(TABLE_DECLARE_SET)(string, V); \
        DEFER(TABLE_DECLARE_DEL)(string, V);)

#define STRTABLE_DEFINE(V) \
EXPAND4(DEFER(TABLE_DEFINE_CREATE)(string, V); \
        DEFER(TABLE_DEFINE_RESIZE)(string, V); \
        DEFER(STRTABLE_DEFINE_DESTROY)(V); \
        DEFER(STRTABLE_DEFINE_CLEAR)(V); \
        DEFER(TABLE_DEFINE_GET)(string, V); \
		DEFER(CSTRTABLE_DEFINE_SET)(V); \
		DEFER(CSTRTABLE_DEFINE_GET)(V); \
        DEFER(STRTABLE_DEFINE_SET)(V); \
        DEFER(STRTABLE_DEFINE_DEL)(V); \
		DEFER(CSTRTABLE_DEFINE_DEL)(V);)

void strtable_destroy_(table_* t);
void strtable_clear_(table_* t);
void strtable_set_(table_* t, string key);
u32 strtable_del_(table_* t, string key);
