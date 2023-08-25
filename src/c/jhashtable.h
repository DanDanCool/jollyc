#pragma once
#include "jvec.h"

const u32 PRIME_ARRAY[] = {
	187091u,     1289u,       28802401u,   149u,        15173u,      2320627u,    357502601u,  53u,
	409u,        4349u,       53201u,      658753u,     8175383u,    101473717u,  1259520799u, 19u,
	89u,         241u,        709u,        2357u,       8123u,       28411u,      99733u,      351061u,
	1236397u,    4355707u,    15345007u,   54061849u,   190465427u,  671030513u,  2364114217u, 7u,
	37u,         71u,         113u,        193u,        313u,        541u,        953u,        1741u,
	3209u,       5953u,       11113u,      20753u,      38873u,      72817u,      136607u,     256279u,
	480881u,     902483u,     1693859u,    3179303u,    5967347u,    11200489u,   21023161u,   39460231u,
	74066549u,   139022417u,  260944219u,  489790921u,  919334987u,  1725587117u, 3238918481u, 3u,
	13u,         29u,         43u,         61u,         79u,         103u,        137u,        167u,
	211u,        277u,        359u,        467u,        619u,        823u,        1109u,       1493u,
	2029u,       2753u,       3739u,       5087u,       6949u,       9497u,       12983u,      17749u,
	24281u,      33223u,      45481u,      62233u,      85229u,      116731u,     159871u,     218971u,
	299951u,     410857u,     562841u,     771049u,     1056323u,    1447153u,    1982627u,    2716249u,
	3721303u,    5098259u,    6984629u,    9569143u,    13109983u,   17961079u,   24607243u,   33712729u,
	46187573u,   63278561u,   86693767u,   118773397u,  162723577u,  222936881u,  305431229u,  418451333u,
	573292817u,  785430967u,  1076067617u, 1474249943u, 2019773507u, 2767159799u, 3791104843u, 2u,
	5u,          11u,         17u,         23u,         31u,         41u,         47u,         59u,
	67u,         73u,         83u,         97u,         109u,        127u,        139u,        157u,
	179u,        199u,        227u,        257u,        293u,        337u,        383u,        439u,
	503u,        577u,        661u,        761u,        887u,        1031u,       1193u,       1381u,
	1613u,       1879u,       2179u,       2549u,       2971u,       3469u,       4027u,       4703u,
	5503u,       6427u,       7517u,       8783u,       10273u,      12011u,      14033u,      16411u,
	19183u,      22447u,      26267u,      30727u,      35933u,      42043u,      49201u,      57557u,
	67307u,      78779u,      92203u,      107897u,     126271u,     147793u,     172933u,     202409u,
	236897u,     277261u,     324503u,     379787u,     444487u,     520241u,     608903u,     712697u,
	834181u,     976369u,     1142821u,    1337629u,    1565659u,    1832561u,    2144977u,    2510653u,
	2938679u,    3439651u,    4026031u,    4712381u,    5515729u,    6456007u,    7556579u,    8844859u,
	10352717u,   12117689u,   14183539u,   16601593u,   19431899u,   22744717u,   26622317u,   31160981u,
	36473443u,   42691603u,   49969847u,   58488943u,   68460391u,   80131819u,   93793069u,   109783337u,
	128499677u,  150406843u,  176048909u,  206062531u,  241193053u,  282312799u,  330442829u,  386778277u,
	452718089u,  529899637u,  620239453u,  725980837u,  849749479u,  994618837u,  1164186217u, 1362662261u,
	1594975441u, 1866894511u, 2185171673u, 2557710269u, 2993761039u, 3504151727u, 4101556399u
};

u32 hash_value(u8* key, int len);
u32 hash_size(u32 size);

#define compare(TYPE) compare_##TYPE
#define COMPARE_DECLARE(TYPE) \
int compare(TYPE)(u8* a, u8* b)

#define hash_table(K, V) hash_table_##K_##V
#define key_item(K) key_item_##K
#define table_init(K, V) table_init_##K_##V
#define table_destroy(K, V) table_destroy_##K_##V
#define table_add(K, V) table_add_##K_##V
#define table_find(K, V) table_find_##K_##V
#define table_resize(K, V) table_resize_##K_##V

#define HASH_TABLE_DECLARE(K, V) \
typedef struct hash_table(K, V) hash_table(K, V); \
typedef struct key_item(K) key_item(K)

#define HASH_TABLE_DECLARE_INIT(K, V) \
void table_init(K, V)(hash_table(K, V)* table, u32 size)

#define HASH_TABLE_DECLARE_DESTROY(K, V) \
void table_destroy(K, V)(hash_table(K, V)* table)

#define HASH_TABLE_DECLARE_ADD(K, V) \
void table_add(K, V)(hash_table(K, V)* table, K* key, V* value)

#define HASH_TABLE_DECLARE_FIND(K, V) \
V* table_find(K, V)(hash_table(K, V)* table, K* key)

#define HASH_TABLE_DECLARE_RESIZE(K, V) \
void table_resize(K, V)(hash_table(K, V)* table, u32 size)

#define HASH_TABLE_DECLARE_FN__() HASH_TABLE_DECLARE_FN_

#define HASH_TABLE_DECLARE_FN_(K, V, arg, ...) \
HASH_TABLE_DECLARE_##arg(K, V); \
__VA_OPT__(HASH_TABLE_DECLARE_FN__ PAREN (TYPE, __VA_ARGS__))

#define HASH_TABLE_DECLARE_FN(K, V, arg, ...) \
__VA_OPT__(EXPAND(HASH_TABLE_DECLARE_FN_(K, V, __VA_ARGS__)))

struct hash_table {
	vector hash;
	vector keys;
	vector items;
	u32 max_probe;
	u32 probe;
}

#define HASH_TABLE_DEFINE_INIT(K, V) \
void table_init(K, V)(hash_table* table, u32 size) { \
	size = hash_size(size); \
	u8* hash = mem_alloc(size * sizeof(u32)); \
	u8* key = mem_alloc(size * sizeof(K)); \
	u8* value = mem_alloc(size * sizeof(V)); \
	vector_init(u32)(&table->hash; hash); \
	vector_init(K)(&table->keys, key); \
	vector_init(V)(&table->items, value); \
	table->max_probe = 24; \
	table->probe = 0; \
}

#define HASH_TABLE_DEFINE_ADD(K, V) \
void table_add(K, V)(hash_table(K, V)* table, K* key, V* value) { \
	if (table->max_probe < table->probe || table->keys.reserve <= table->keys.size) \
		table_resize(K, V)(table, table->keys.size * 2); \
	u32 reserve = table->keys.reserve; \
	u32 hash = hash_value((u8*)key, sizeof(K)); \
	u32 index = hash % reserve; u32 probe = 0; \
	key_item(K)* item = vector_at(key_item(K))(&table->keys, index); \
	key_item(K) tmpitem; V tmpval; \
	while (true) { \
		if (!item->hash) { \
			item->hash = hash; \
			item->key = *key; \
			vector_store(V)(&table->items, index, value); \
			table->keys.size++; \
			table->items.size++; \
			break; \
		} \
		u32 dist = ABS(item->hash % reserve - index); \
		if (dist < probe) { \
			tmpitem = *item; \
			item->hash = hash; \
			item->key = *key; \
			hash = tmpitem.hash; \
			key = &tmpitem.key; \
			vector_load(V)(&table->items, index, &tmpval); \
			vector_store(V)(&table->items, index, value); \
			value = &tmpval; \
			probe = dist; \
		} \
		probe++; \
		index = (index + 1) < reserve ? index + 1 : 0; \
		item = vector_at(key_item(K))(&table->keys, index); \
	} \
	table->probe = table->probe < probe ? probe : table->probe; \
}

#define HASH_TABLE_DEFINE_FIND(K, V) \
V* table_find(K, V)(hash_table(K, V)* table, K* key) { \
	u32 hash = hash_value((u8*)key, sizeof(K)); \
	u32 reserve = table->keys.reserve; \
	u32 index = hash % eserve; \
	key_item(K)* item = vector_at(key_item(K))(&table->keys, index); \
	for (u32 i = 0; i <= table->probe; i++) { \
		if (hash == item->hash && compare((u8*)key, (u8*)&item->key)) \
			return vector_at(V)(&table->items, index); \
		index = (index + 1) < reserve ? index + 1 : 0; \
	} \
	return NULL; \
}

#define HASH_TABLE_DEFINE_RESIZE(K, V) \
void table_resize(K, V)(hash_table(K, V)* table, u32 size) { \
	size = hash_size(size); \
	vector(key_item(K)) key = { jolly_alloc(size * sizeof(key_item(K))), 0, size }; \
	vector(V) val = { jolly_alloc(size * sizeof(V)), 0, size }; \
	table->probe = 0; \
	for (u32 i = 0; i < table->keys.reserve; i++) { \
		key_item(K)* item = vector_at(key_item(K))(&table->keys, i); \
		V* value = vector_at(V)(&table->items, i); \
		u32 index = item->hash % size; u32 probe = 0; \
		key_item(K)* nitem = vector_at(key_item(K))(&key, index); \
		key_item(K) tmpitem; V tmpval; \
		while (true) { \
			if (!nitem->hash) { \
				nitem->hash = item->hash; \
				nitem->key = item->key; \
				vector_store(V)(&val, index, value); \
				break; \
			} \
			u32 dist = ABS(nitem->hash % size - index); \
			if (dist < probe) { \
				tmpitem = *nitem; \
				nitem->hash = item->hash; \
				nitem->key = item->key; \
				*item = tmpitem; \
				vector_load(V)(&table->items, index, &tmpval); \
				vector_store(V)(&table->items, index, value); \
				value = &tmpval; \
				probe = dist; \
			} \
			probe++; \
			index = (index + 1) < size ? index + 1 : 0; \
			nitem = vector_at(key_item(K))(&table->keys, index); \
		} \
		table->probe = table->probe < probe ? probe : table->probe; \
	} \
	free(table->keys.data); \
	free(table->items.data); \
	table->keys.data = key.data; \
	table->items.data = val.data; \
}
