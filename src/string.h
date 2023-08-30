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
vector_ string_split(string str, char delim);
string string_substr(string str, u32 beg, u32 end);
void string_destroy(string* str);

COPY_DECLARE(string);
HASH_DECLARE(string);
EQ_DECLARE(string);

VECTOR_DECLARE(string);

#define STRTABLE_DEFINE_DESTROY(V) \
void table_destroy(string, V)(table_* t) { \
	strtable_destroy_(t); \
}

#define STRTABLE_DEFINE_SET(V) \
void table_set(string, V)(table_* t, string* key, V* value) { \
    V* cur = table_get(string, V)(t, key); \
    if (cur) { \
        copy(V)(value, cur); \
        return; \
    } \
	strtable_set_(t, key); \
	vector_add(V)(&t->items, value); \
}

#define STRTABLE_DEFINE_DEL(V) \
void table_del(string, V)(table_* t, string* key) { \
	u32 idx = strtable_del_(t, key); \
	if (idx == U32_MAX) return; \
	vector_del(V)(&t->items, idx); \
}

#define STRTABLE_DEFINE(V) \
EXPAND4(DEFER(TABLE_DEFINE_CREATE)(string, V); \
        DEFER(TABLE_DEFINE_RESIZE)(string, V); \
        DEFER(STRTABLE_DEFINE_DESTROY)(V); \
        DEFER(TABLE_DEFINE_GET)(string, V); \
        DEFER(STRTABLE_DEFINE_SET)(V); \
        DEFER(TABLE_DEFINE_ADD)(string, V); \
        DEFER(STRTABLE_DEFINE_DEL)(V);)

void strtable_destroy_(table_* t);
void strtable_set_(table_* t, string* key);
u32 strtable_del_(table_* t, string* key);
