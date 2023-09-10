#include "queue.h"
#include "memory.h"
#include <stdatomic.h>

void queue_create_(queue_* q, u32 size) {
	memptr data = alloc256(size * sizeof(u64));
	q->data = (u64*)data.data;
	q->reserve = size;
	atomic_store_explicit(&q->size, 0, memory_order_relaxed);
	atomic_store_explicit(&q->head, 0, memory_order_relaxed);
	atomic_store_explicit(&q->head, 0, memory_order_relaxed);
}

void queue_destroy_(queue_* q) {
	free256(q->data);
}

u32 queue_size_(queue_* q) {
	u32 size = atomic_load_explicit(&q->size, memory_order_acquire);
	return size;
}

void queue_add_(queue_* q, void* data) {
	u32 idx = atomic_load_explicit(&q->tail, memory_order_relaxed);

	u32 next = (idx + 1) % q->reserve;
	while(!atomic_compare_exchange_weak_explicit(&q->tail, &idx, next,
				memory_order_release, memory_order_relaxed)) {
		next = (idx + 1) % q->reserve;
	}

	q->data[idx] = (u64)data;
	atomic_fetch_add_explicit(&q->size, 1, memory_order_release);
}


void* queue_del_(queue_* q) {
	u32 size = atomic_load_explicit(&q->size, memory_order_relaxed);
	if (size == 0) return NULL;
	while(!atomic_compare_exchange_weak_explicit(&q->size, &size, size - 1, memory_order_release, memory_order_relaxed)) {
		if (size == 0) return NULL;
	}

	u32 idx = atomic_load_explicit(&q->head, memory_order_relaxed);
	u32 next = (idx + 1) % q->reserve;
	while (!atomic_compare_exchange_weak_explicit(&q->head, &idx, next, memory_order_release, memory_order_relaxed)) {
		next = (idx + 1) % q->reserve;
	}

	void* tmp = (void*)q->data[idx];
	q->data[idx] = 0;
	return tmp;
}

void queue_clear_(queue_* q) {
	atomic_store_explicit(&q->size, 0, memory_order_release);
}
