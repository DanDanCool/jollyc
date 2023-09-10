#include "strlib.h"
#include "simd.h"

#include <assert.h>

string string_create(cstr str) {
	u64 bytes = 0;
	while (str[bytes]) {
		bytes++;
	}

	memptr data = alloc256((u32)bytes + 1);
	copy8((u8*)str, data.data, (u32)bytes);
	data.size = bytes;
	return data;
}

char string_at(string str, int idx) {
	return (char)str.data[idx];
}

string string_cat(string s1, string s2) {
	u64 bytes = s1.size + s2.size;
	memptr data = alloc256((u32)bytes + 1);

	u32 count = align_size256((u32)s1.size);
	copy256(s1.data, data.data, count);
	u8* ptr = s2.data;
	for (u64 i = s1.size; i < bytes; i++) {
		data.data[i] = *ptr;
		ptr++;
	}

	return data;
}

string string_combine(vector_ strings) {
	u64 bytes = 0;
	for (u32 i = 0; i < strings.size; i++) {
		string* str = vector_at(string)(&strings, i);
		bytes += str->size;
	}

	memptr data = alloc256((u32)bytes + 1);
	u8* ptr = data.data;
	for (u32 i = 0; i < strings.size; i++) {
		string* str = vector_at(string)(&strings, i);
		for (u32 j = 0; j < str->size; j++) {
			*ptr = str->data[j];
			ptr++;
		}
	}

	data.size = bytes;
	return data;
}

vector_ string_split(string str, const char* delim) {
	vector_ res;
	vector_create(string)(&res, 0);

	u32 beg = 0;
	for (u32 i = 0; i < str.size; i++) {
		const char* ptr = delim;
		while (*ptr) {
			if (string_at(str, i) == *ptr) {
				string substr = string_substr(str, beg, i);
				vector_add(string)(&res, &substr);
				beg = i + 1;
			}
			ptr++;
		}
	}
	string substr = string_substr(str, beg, (u32)str.size);
	vector_add(string)(&res, &substr);

	return res;
}

string string_substr(string str, u32 beg, u32 end) {
	if (end <= beg) return string_create("");

	memptr data = alloc256(end - beg);
	u8* ptr = data.data;

	for (u32 i = beg; i < end; i++) {
		*ptr = str.data[i];
		ptr++;
	}

	return data;
}

void string_destroy(string* str) {
	free256(str->data);
	str->data = NULL;
	str->size = 0;
}

COPY_DEFINE(string);
SWAP_DEFINE(string);

u32 hash(string)(string key) {
	return fnv1a(key.data, (u32)key.size);
}

int eq(string)(string* s1, string* s2) {
	if (s1->size != s2->size) return false;

	u32 count = align_size256((u32)s1->size) / BLOCK_32;
	u8* a = s1->data;
	u8* b = s2->data;
	for (u32 i = 0; i < count; i++) {
		__m256i x = _mm256_load_si256((__m256i*)a);
		__m256i y = _mm256_load_si256((__m256i*)b);

		__m256i z = _mm256_cmpeq_epi8(x, y);
		int equal = _mm256_testc_si256(z, z);
		if (!equal) return false;

        a += BLOCK_32;
        b += BLOCK_32;
	}

	return true;
}

int _eq(string)(u8* s1, u8* s2) {
    return eq(string)((string*)s1, (string*)s2);
}

VECTOR_DEFINE(string);

void strtable_destroy_(table_* t) {
	for (u32 i = 0; i < t->reserve; i++) {
		hash_* h = (hash_*)vector_at(u64)(&t->hash, i);
		if (h->hash == 0) continue;
		string* s = vector_at(string)(&t->keys, i);
		string_destroy(s);
	}
	table_destroy_(t);
}

void strtable_clear_(table_* t) {
	for (u32 i = 0; i < t->reserve; i++) {
		hash_* h = (hash_*)vector_at(u64)(&t->hash, i);
		if (h->hash == 0) continue;
		string* s = vector_at(string)(&t->keys, i);
		string_destroy(s);
	}
	table_clear_(t);
}

void strtable_set_(table_* t, string key) {
	u32 idx = t->items.size;
	hash_ hash = { hash(string)(key), idx };
	memptr k = { (u8*)&key, sizeof(string) };
	table_probe_(t, k, hash);
    table_resize_(t, sizeof(string), t->reserve * 2);
}

u32 strtable_del_(table_* t, string key) {
	hash_ hash = { hash(string)(key), 0 };
	memptr k = { (u8*)&key, sizeof(string) };
	u32 idx = table_find_(t, k, hash, _eq(string));
	if (idx == U32_MAX) return U32_MAX;
	hash_* h = (hash_*)vector_at(u64)(&t->hash, idx);
	string* str = vector_at(string)(&t->keys, idx);
	string_destroy(str);
	idx = h->index;
    zero8((u8*)h, sizeof(hash_));
	zero8((u8*)vector_at(string)(&t->keys, idx), sizeof(string));
	return idx;
}
