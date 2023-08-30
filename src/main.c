#include "vector.h"
#include "table.h"
#include "string.h"

#include <stdio.h>

TABLE_DECLARE(u32, u32);
TABLE_DECLARE(string, i32);

void test_vector() {
	vector(i32) v;
	vector_create(i32)(&v, 0);
	for (i32 i = 0; i < 100; i++) {
		vector_add(i32)(&v, &i);
	}

	i32 a = *vector_at(i32)(&v, 3);
	printf("hello world %i\n", a);
	vector_set(i32)(&v, &a, 0);
	vector_del(i32)(&v, 4);
	vector_resize(i32)(&v, 50);

	vector_get(i32)(&v, &a, 9);
	printf("hello world %i\n", a);
	vector_destroy(int)(&v);
	printf("hello world %i\n", v.reserve);
}

void test_table() {
    table(u32, u32) t;
    table_create(u32, u32)(&t, 5);

    for (u32 i = 0; i < 111; i++) {
        u32 val = i * 2;
        table_set(u32, u32)(&t, &i, &val);
    }

    u32 key = 5;
    u32 a = 0;
    u32* tmp = table_get(u32, u32)(&t, &key);
    if (tmp) a = *tmp;
	table_del(u32, u32)(&t, &key);
    tmp = table_get(u32, u32)(&t, &key);
    if (!tmp) printf("%i\n", a);

    table_destroy(u32, u32)(&t);
}

void test_strtable() {
	table(string, i32) t;
	table_create(string, i32)(&t, 0);

	string key = string_create("hello");
	i32 val = -69;
	table_set(string, i32)(&t, &key, &val);
	string_destroy(&key);

	key = string_create("world");
	val = 420;
	table_set(string, i32)(&t, &key, &val);
	string_destroy(&key);

	key = string_create("food");
	val = 2343420;
	table_set(string, i32)(&t, &key, &val);
	string_destroy(&key);

	key = string_create("baz");
	val = -420;
	table_set(string, i32)(&t, &key, &val);
	string_destroy(&key);

	key = string_create("bar");
	val = 420234;
	table_set(string, i32)(&t, &key, &val);
	string_destroy(&key);

	key = string_create("world");
	table_del(string, i32)(&t, &key);
	string_destroy(&key);

	key = string_create("hello");
	val = *table_get(string, i32)(&t, &key);
	string_destroy(&key);

	key = string_create("world");
	i32* tmp = table_get(string, i32)(&t, &key);
	if (!tmp) printf("hello strtable %i\n", val);
	string_destroy(&key);

	table_destroy(string, i32)(&t);
}

int main() {
	test_table();
	test_strtable();
	test_vector();
}

TABLE_DEFINE(u32, u32);
STRTABLE_DEFINE(i32);
