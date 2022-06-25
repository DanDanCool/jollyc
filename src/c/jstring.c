#include "jstring.h"
#include <stdarg.h>
#include <emmintrin.h>

void str_cat(mem_arena* mem, string* str, ...)
{
	va_list args;
	va_start(args, str);

	string* strings[32];
	uint32 index = 0;
	uint32 size = str->size;

	string* s = va_arg(args, string*);
	while (s)
	{
		strings[index++] = s;
		size += s ? s->size : 0;
		s = va_arg(args, string*);
	}

	va_end(args);

	if (str->data.size < size)
	{
		uint32 count = mem->blocksz / size;
		count += mem->blocksz % size ? 1 : 0;
		arena_realloc(mem, &str->data, count);
	}

	for (int i = 0; i < index; i++)
	{
		s = &strings[i];
		uint32 count = s->size / BLOCK_32;
		count += s->size % BLOCK_32 ? 1 : 0;
		mem_cpy32u(MEM_DATA(str->mem) + MEM_SIZE(str->mem), MEM_DATA(s->mem), count);
		str->size += s->size;
	}
}

uint32 str_len(const char* str)
{
	__m256i nul = _mm256_set1_epi8('\0');
	__m256i one = _mm256_set1_epi8(0xFF);
	uint32 size = 0;
	while (true)
	{
		__m256i tmp = _mm256_loadu_si256((__m256i*)str);
		__m256i cmp = _mm256_cmpeq_epi8(tmp, nul);

		if (!_mm256_testz_si256(res, one))
		{
			alignas(32) uint32 res[8];
			_mm256_store_si256((__m256i*)res, cmp);

			int found = 0;
			for (int i = 0; i < 8; i++)
			{
				// seems like index is set to 32 if there are no bits set
				int index = _bit_scan_forward(res[i]);
				size += found ? 0 : index < 32 ? index / 8 : 4;
				found = found ? found : index < 32;
			}

			break;
		}

		str += BLOCK_32;
		size += BLOCK_32;
	}

	return size;
}

int str_cmp(string* s1, string* s2)
{
	uint32 minsize = MEM_SIZE(s1->data) > MEM_SIZE(s2->data) ? MEM_SIZE(s2->data) : MEM_SIZE(s1->data);
	return mem_cmp32(MEM_DATA(s1->data), MEM_DATA(s2->data), minsize / BLOCK_32);
}
