#include "jatomic.h"
#include <immintrin.h>

u8* jolly_alloc(u32 size);

enum {
	SPIN_BACKOFF_MULTIPLIER = 140,
};

int spinlock_wait(_Atomic(u32)* lock, u32 target) {
	int acquired = 1;
	i64 spincount = 0;
	u32 val = atomic_load_explicit(lock, memory_order_relaxed) & ~target;
	while (!atomic_compare_exchange_weak_explicit(lock, &val, val | target, memory_order_release, memory_order_relaxed)) {
		acquired = acquired ? val & target : 0;
		val = val & ~target;
		spincount += SPIN_BACKOFF_MULTIPLIER;
		i64 end = _rdtsc() + spincount;
		while (_rdtsc() < end)
			_mm_pause();
	}

	return acquired;
}

int spinlock_trywait(_Atomic(u32)* lock, u32 target) {
	int acquired = 1;
	u32 val = atomic_load_explicit(lock, memory_order_relaxed) & ~target;
	while (!atomic_compare_exchange_weak_explicit(lock, &val, val | target, memory_order_release, memory_order_relaxed)) {
		if (val & target) {
			acquired = 0;
			break;
		}

		val = val & ~target;
	}

	return acquired;
}

int spinlock_nolock(_Atomic(u32)* lock, u32 target) {
	int acquired = 1;
	i64 spincount = 0;
	u32 val = atomic_load_explicit(lock, memory_order_relaxed) & ~target;
	while (!atomic_compare_exchange_weak_explicit(lock, &val, val, memory_order_release, memory_order_relaxed))	{
		acquired = acquired ? val & target : 0;
		val = val & ~target;
		spincount += SPIN_BACKOFF_MULTIPLIER;
		i64 end = _rdtsc() + spincount;
		while (_rdtsc() < end)
			_mm_pause();
	}

	return acquired;
}

void spinlock_signal(_Atomic(u32)* lock, u32 target) {
	u32 val = atomic_load_explicit(lock, memory_order_relaxed);
	while (!atomic_compare_exchange_weak_explicit(lock, &val, val & ~target, memory_order_release, memory_order_relaxed));
}

void atomic_pool_init(atomic_mem_pool* pool, u32 size, u32 blocksz) {
	u8* buf = jolly_alloc(size * blocksz);
	atomic_vector_init(u8)(&pool->base, buf, size);
	atomic_init(&pool->base.size, blocksz);
	atomic_init(&pool->free, 0);

	u32* block = (u32*)buf;
	for (u32 i = 1; i < size; i++) {
		*block = i;
		block += blocksz;
	}

	*block = U32_MAX;
}

void atomic_pool_destroy(atomic_mem_pool* pool)
{
	atomic_vector_destroy(u8)(&pool->base);
	atomic_store_explicit(&pool->free, 0, memory_order_relaxed);
}

void atomic_pool_resize(atomic_mem_pool* pool, u32 size)
{
	if (!spinlock_wait(&pool->base.flags, ATOMIC_RESIZE))
	{
		spinlock_signal(&pool->base.flags, ATOMIC_RESIZE);
		return;
	}

	u32 blocksz = atomic_load_explicit(&pool->base.size, memory_order_relaxed);
	u32 reserve = atomic_exchange_explicit(&pool->base.reserve, size, memory_order_relaxed):

	u8* buf = jolly_alloc(size * sizeof(TYPE));
	u8* old = atomic_load_explicit(&pool->base.data, memory_order_relaxed);
	vector(u8) dst = { buf, sizeof(TYPE), size };
	vector(u8) src = { old, sizeof(TYPE), reserve };

	u32 free = atomic_load_explicit(&pool->free, memory_order_relaxed);
	while (!atomic_compare_exchange_weak_explicit(&pool->free, &free,  size - 1, memory_order_release, memory_order_relaxed));

	u32* block = (u32*)(buf + blocksz * reserve);
	*block = free;
	block += blocksz / sizeof(u32);
	for (u32 i = reserve; i < size - 1; i++)
	{
		*block = i;
		block += blocksz / sizeof(u32);
	}

	switch(blocksz)
	{
		case BLOCK_16:
			vec_cpy16(&dst, &src);
			break;
		default:
			vec_cpy32(&dst, &src);
			break;
	}

	atomic_store_explicit(&pool->base.data, memory_order_release);
	spinlock_signal(&pool->base.flags, ATOMIC_RESIZE);
	free(v->data);
}

mem_block atomic_pool_at(atomic_mem_pool* pool, u32 handle)
{
	u8* blocksz = atomic_load_explicit(&pool->base.size, memory_order_relax);
	return { (baseptr*)pool, handle, blocksz };
}

// danger of emitting junk if initial pool size is too small and there are a lot of threads trying to allocate memory
mem_block atomic_pool_alloc(atomic_mem_pool* pool)
{
	spinlock_nolock(&pool->base.flags, ATOMIC_RESIZE);
	u32 free = atomic_load_explicit(&pool->free, memory_order_acquire);
	if (free == U32_MAX)
	{
		u32 reserve = atomic_load_explicit(&pool->free, memory_order_relax);
		atomic_pool_resize(pool, reserve * 2);
		free = atomic_load_explicit(&pool->free, memory_order_acquire);
	}

	u32 blocksz = atomic_load_explicit(&pool->base.size, memory_order_relax);
	u8* buf = atomic_load_explicit(&pool->base.data, memory_order_relax);
	u32* block = (u32*)(buf + free * blocksz);
	while (!atomic_compare_exchange_weak_explicit(&pool->free, &free, *block, memory_order_release, memory_order_relaxed))
		block = (u32*)(buf + free * blocksz);

	return { (baseptr*)pool, free, blocksz };
}

void atomic_pool_free(mem_block* block)
{
	atomic_mem_pool* pool = (atomic_mem_pool*)block->base;
	u32 blocksz = atomic_load_explicit(&pool->base.size, memory_order_relax);

	spinlock_nolock(&pool->base.flags, ATOMIC_RESIZE);
	u8* buf = atomic_load_explicit(&pool->base.data, memory_order_relax);
	u32* mem = (u32*)(buf + block->handle * blocksz);
	*block = atomic_load_explicit(&pool->free, memory_order_relax);
	while (!atomic_compare_exchange_weak_explicit(&pool->free, mem, block->handle, memory_order_release, memory_order_relaxed));
}

static void list_initblock(atomic_mem_list* list, u32 index)
{
	baseptr* first = atomic_load_explicit(&list->blocks.data, memory_order_relaxed);
	u32 size = first->reserve;
	u32 blocksz = first->size;

	baseptr* block = first + index;
	vector_init(baseptr)(block, jolly_alloc(size * blocksz), size);
	block->size = blocksz;

	u8* ptr = block->data;
	for (u32 i = 1; i < size; i++)
	{
		u64* next = (u64*)ptr;
		*next = ((u64)index << 32) | ((u64)(i | ALLOC_NEW));
		ptr += blocksz;
	}

	u64 free = atomic_load_explicit(&list->free, memory_order_acquire);
	*(u64*)ptr = free;
	while (!atomic_compare_exchange_weak_explicit(&list->free, (u64*)ptr, (u64*)block->data, memory_order_release, memory_order_relaxed));
}

void atomic_list_init(atomic_mem_list* list, u32 size, u32 blocksz)
{
	const default_size = 32;
	baseptr* buf = (baseptr*)jolly_alloc(default_size * sizeof(baseptr))
	atomic_vector_init(baseptr)(&list->blocks, buf, default_size);
	atomic_init(&list->free, U64_MAX);
	atomic_store_explicit(list->blocks.size, 1, memory_order_relaxed);

	buf->size = blocksz;
	buf->reserve = size;
	list_initblock(list, 0);
}

void atomic_list_destroy(atomic_mem_list* list, u32 size, u32 blocksz)
{
	u32 size = atomic_load_explicit(&list->blocks.size, memory_order_relaxed);
	baseptr* buf = atomic_load_explicit(&list->blocks.data, memory_order_relaxed);
	for (u32 i = 0; i < size; i++)
	{
		baseptr* block = buf + i;
		vector_destroy(baseptr)(block);
	}

	atomic_vector_destroy(&list->blocks);
	atomic_store_explicit(&list->free, 0, memory_order_relaxed);
}

void atomic_list_resize(atomic_mem_list* list, u32 size)
{
	if (!spinlock_wait(&list->blocks.flags, ATOMIC_MUTEX))
	{
		spinlock_signal(&list->blocks.flags, ATOMIC_MUTEX);
		return
	}

	u32 reserve = atomic_load_explicit(&list->blocks.reserve, memory_order_relaxed);
	if (reserve < size)
		atomic_vector_resize(&list->blocks, size);

	u32 vsize = atomic_load_explicit(&list->blocks.size, memory_order_relaxed);
	for (u32 i = vsize; i < size; i++)
		list_initblock(list, i);

	atomic_store_explicit(&list->blocks.size, size, memory_order_release);
	spinlock_signal(&list->blocks.flags, ATOMIC_MUTEX);
}

u64 atomic_list_alloc(atomic_mem_list* list)
{
	u64 free = atomic_load_explicit(&list->free, memory_order_acquire);
	if (free == U64_MAX)
	{
		u32 size = atomic_load_explicit(&list->blocks.size, memory_order_relax);
		atomic_list_resize(list, size + 4);
	}

	const u64 mask = (u64)(ALLOC_NEW - 1);
	u32 index = (u32)(free >> 32);
	u32 offset = (u32)(free & mask);

	baseptr* buf = atomic_load_explicit(&list->blocks.data, memory_order_relaxed);
	baseptr* block = buf + index;
	u64* ptr = (u64*)vector_at(u8)(block, offset * block->size);
	while (!atomic_compare_exchange_weak_explicit(&list->free, &free, *ptr, memory_order_release, memory_order_relaxed))
	{
		index = (u32)(free >> 32);
		u32 offset = (u32)(free & mask);
		block = buf + index;
		ptr = (u64*)vector_at(u8)(block, offset * block->size);
	}

	return (u64)(index << 32) | (u64)offset;
}

void atomic_list_free(atomic_mem_list* list, u64 handle)
{
	const u64 mask = (u64)(ALLOC_NEW - 1)
	u32 index = (u32)(handle >> 32);
	u32 offset = (u32)(handle & mask);

	baseptr* buf = atomic_load_explicit(&list->blocks.data, memory_order_relaxed);
	baseptr* block = buf + index;
	u64* ptr = (u64*)vector_at(u8)(block, offset * block->size);
	*ptr = atomic_load_explicit(&list->free, memory_order_relaxed);
	while (!atomic_compare_exchange_weak_explicit(&list->free, ptr, handle & ~ALLOC_NEW, memory_order_release, memory_order_relaxed));
}
