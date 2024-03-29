#include "table.h"
#include "simd.h"

#include <assert.h>

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

const u32 MAX_PROBE = 24;

u32 hash_size(u32 size);

void table_create_(table_* t, u32 keysize, u32 itemsize, u32 size) {
	assert(keysize <= BLOCK_256);
    size = MAX(size, 100);
	size = hash_size(size);
	vector_create_(&t->hash, sizeof(hash_) * size);
	vector_create_(&t->keys, keysize * size);
	vector_create_(&t->items, itemsize * size);
	t->reserve = size;
	t->probe = MAX_PROBE;
}

void table_resize_(table_* t, u32 keysize, u32 size) {
    if (t->probe <= MAX_PROBE) return;
	size = hash_size(size);

	vector_ hash = t->hash;
	vector_ keys = t->keys;

	vector_create_(&t->hash, sizeof(hash_) * size);
	vector_create_(&t->keys, keysize * size);

    t->probe = MAX_PROBE;
	u32 count = t->reserve;
	t->reserve = size;
	for (u32 i = 0; i < count; i++) {
		hash_* h = (hash_*)vector_at(u64)(&hash, i);
		if (h->hash == 0) continue;
		memptr key = { keys.data + i * keysize, keysize };
		table_probe_(t, key, *h);
	}
}

void table_destroy_(table_* t) {
	vector_destroy_(&t->hash);
	vector_destroy_(&t->keys);
	vector_destroy_(&t->items);
    t->reserve = 0;
	t->probe = 0;
}

void table_clear_(table_* t) {
	vector_clear_(&t->hash);
	vector_clear_(&t->keys);
	vector_clear_(&t->items);
}

u32 hash_size(u32 size) {
	u32 best = U32_MAX;
	u32 cur  = 0;

	for (int i = 0; i < 7; i++)	{
		u32 prime = PRIME_ARRAY[cur];
		if (size < prime) {
			cur = 2 * cur + 1;
		} else {
			cur = 2 * cur + 2;
		}

		if (size <= prime && prime < best) {
			best = prime;
		}
	}

	return best;
}

void table_probe_(table_* t, memptr key, hash_ hash) {
	vec256i buf1_ = {0};

	copy8(key.data, (u8*)&buf1_, (u32)key.size);
	key.data = (u8*)&buf1_;

	u32 probe = 0;
	u32 idx = hash.hash % t->reserve;

	while (true) {
		hash_* item = (hash_*)vector_at(u64)(&t->hash, idx);
		if (item->hash == 0) {
			*item = hash;
			u8* ptr = t->keys.data + idx * key.size;
			copy8(key.data, ptr, (u32)key.size);
            break;
		}

		u32 dist = item->hash - idx;
		if (dist < probe) {
			vec256i buf = {0};

			hash_ tmp = *item;
			u8* ptr = t->keys.data + idx * key.size;
			copy8(ptr, (u8*)&buf, (u32)key.size);

			*item = hash;
			copy8(key.data, ptr, (u32)key.size);

			hash = tmp;
			copy8((u8*)&buf, key.data, (u32)key.size);
		}

		probe++;
		idx++;
		idx = idx == t->reserve ? 0 : idx;
	}

    t->probe = MAX(t->probe, probe);
}

u32 table_find_(table_* t, memptr key, hash_ hash, pfn_eq eq_fn) {
	for (u32 i = 0; i < t->probe; i++) {
		u32 idx = (hash.hash + i) % t->reserve;
		hash_* item = (hash_*)vector_at(u64)(&t->hash, idx);
		if (item->hash == hash.hash) {
			u8* k = t->keys.data + key.size * idx;
			if (eq_fn(k, key.data)) return idx;
		}
	}

	return U32_MAX;
}
