#pragma once

#include "jtype.h"
#include "jmacro.h"
#include <stdatomic>

enum
{
	ALLOC_NEW = 1 << 31,

	ATOMIC_MUTEX = 1 << 0,
	ATOMIC_TRANSFER = 1 << 1,
	ATOMIC_RESIZE = 1 <<2,
};

int spinlock_wait(_Atomic(u32)* lock, u32 target);
int spinlock_trywait(_Atomic(u32)* lock, u32 target);
int spinlock_nolock(_Atomic(u32)* lock, u32 target);
void spinlock_signal(_Atomic(u32)* lock, u32 target);

#define atomic(TYPE) atomic_##TYPE

#define atomic_vector(TYPE) atomic_vector_##TYPE
#define atomic_vector_init(TYPE) atomic_vector_init_##TYPE
#define atomic_vector_destroy(TYPE) atomic_vector_destroy_##TYPE
#define atomic_vector_load(TYPE) atomic_vector_load_##TYPE
#define atomic_vector_store(TYPE) atomic_vector_store_##TYPE
#define atomic_vector_add(TYPE) atomic_vector_add_##TYPE
#define atomic_vector_rm(TYPE) atomic_vector_rm_##TYPE
#define atomic_vector_resize(TYPE) atomic_vector_resize_##TYPE

#define atomic_queue(TYPE) atomic_queue_##TYPE
#define atomic_queue_init(TYPE) atomic_queue_init_##TYPE
#define atomic_queue_destroy(TYPE) atomic_queue_destroy_##TYPE
#define atomic_queue_load(TYPE) atomic_queue_load_##TYPE
#define atomic_queue_store(TYPE) atomic_queue_store_##TYPE
#define atomic_queue_push(TYPE) atomic_queue_push_##TYPE
#define atomic_queue_pop(TYPE) atomic_queue_pop_##TYPE
#define atomic_queue_trypop(TYPE) atomic_queue_pop_##TYPE

#define ATOMIC_VECTOR_DECLARE(TYPE)\
typedef struct atomic_vector(TYPE) atomic_vector(TYPE)

#define ATOMIC_VECTOR_DECLARE_INIT(TYPE) \
void atomic_vector_init(TYPE)(atomic_vector(TYPE)* v, u8* data, u32 size)

#define ATOMIC_VECTOR_DECLARE_DESTROY(TYPE) \
void atomic_vector_destroy(TYPE)(atomic_vector(TYPE)* v)

#define ATOMIC_VECTOR_DECLARE_LOAD(TYPE) \
void atomic_vector_load(TYPE)(atomic_vector(TYPE)* v, u32 index, TYPE* out)

#define ATOMIC_VECTOR_DECLARE_STORE(TYPE) \
void atomic_vector_store(TYPE)(atomic_vector(TYPE)* v, u32 index, TYPE* in)

#define ATOMIC_VECTOR_DECLARE_ADD(TYPE) \
void atomic_vector_add(TYPE)(atomic_vector(TYPE)* v, TYPE* data)

#define ATOMIC_VECTOR_DECLARE_RM(TYPE) \
u32 atomic_vector_rm(TYPE)(atomic_vector(TYPE)* v)

#define ATOMIC_VECTOR_DECLARE_RESIZE(TYPE) \
void atomic_vector_resize(TYPE)(atomic_vector(TYPE)* v, u32 size)

#define ATOMIC_VECTOR_DECLARE_FN__() ATOMIC_VECTOR_DECLARE_FN_

#define ATOMIC_VECTOR_DECLARE_FN_(TYPE, arg, ...) \
ATOMIC_VECTOR_DECLARE_##arg(TYPE); \
__VA_OPT__(ATOMIC_VECTOR_DECLARE_FN__ PAREN (TYPE, __VA_ARGS__))

#define ATOMIC_VECTOR_DECLARE_FN(TYPE, ...) \
__VA_OPT__(EXPAND(ATOMIC_VECTOR_DECLARE_FN_(TYPE, __VA_ARGS__)))

#define ATOMIC_QUEUE_DECLARE(TYPE) \
typedef struct atomic_queue(TYPE) atomic_queue(TYPE)

#define ATOMIC_QUEUE_DECLARE_INIT(TYPE) \
void atomic_queue_init(TYPE)(atomic_queue(TYPE)* q, u8* data, u32 size)

#define ATOMIC_QUEUE_DECLARE_DESTROY(TYPE) \
void atomic_queue_destroy(TYPE)(atomic_queue(TYPE)* q)

#define ATOMIC_QUEUE_DECLARE_LOAD(TYPE) \
void atomic_queue_load(TYPE)(atomic_queue(TYPE)* q, u32 index, TYPE* out)

#define ATOMIC_QUEUE_DECLARE_STORE(TYPE) \
void atomic_queue_store(TYPE)(atomic_queue(TYPE)* q, u32 index, TYPE* in)

#define ATOMIC_QUEUE_DECLARE_PUSH(TYPE) \
void atomic_queue_push(TYPE)(atomic_queue(TYPE)* q, TYPE* data)

#define ATOMIC_QUEUE_DECLARE_POP(TYPE) \
int atomic_queue_pop(TYPE)(atomic_queue(TYPE)* q, TYPE* out)

#define ATOMIC_QUEUE_DECLARE_TRYPOP(TYPE) \
void atomic_queue_trypop(TYPE)(atomic_queue(TYPE)* q, TYPE* out)

#define ATOMIC_QUEUE_DECLARE_FN__() ATOMIC_QUEUE_DECLARE_FN_

#define ATOMIC_QUEUE_DECLARE_FN_(TYPE, arg, ...) \
ATOMIC_QUEUE_DECLARE_##arg(TYPE); \
__VA_OPT__(ATOMIC_QUEUE_DECLARE_FN__ PAREN (TYPE, __VA_ARGS__))

#define ATOMIC_QUEUE_DECLARE_FN(TYPE, ...) \
__VA_OPT__(EXPAND(ATOMIC_QUEUE_DECLARE_FN_(TYPE, __VA_ARGS__)))

#define ATOMIC_VECTOR_DEFINE(TYPE) \
struct atomic_vector_##TYPE {    \
    _Atomic(TYPE*) data;  \
    _Atomic(u32) size;    \
    _Atomic(u32) reserve; \
	_Atomic(u32) flags;
}

#define VECTOR_DEFINE_INIT(TYPE) \
void atomic_vector_init(TYPE)(atomic_vector(TYPE)* v, u8* data, u32 size) { \
	atomic_init(&v->data, data); \
	atomic_init(&v->size, 0); \
	atomic_init(&v->reserve, size); \
	atomic_init(&v->flags, 0);\
}

// assumes that only the calling thread has access to the vector, and all other operations are completed
#define ATOMIC_VECTOR_DEFINE_DESTROY(TYPE) \
void atomic_vector_destroy(TYPE)(atomic_vector(TYPE)* v) { \
	atomic_store_explicit(&v->size, 0, memory_order_relaxed); \
	atomic_store_explicit(&v->reserve, 0, memory_order_relaxed); \
	atomic_store_explicit(&v->flags, 0, memory_order_relaxed); \
	TYPE* buf = atomic_exchange_explicit(&v->data, NULL, memory_order_relaxed); \
	free(buf); \
}

// need VEC_MEMCOPY
#define ATOMIC_VECTOR_DEFINE_LOAD(TYPE) \
void atomic_vector_load(TYPE)(atomic_vector(TYPE)* v, u32 index, TYPE* out) { \
	TYPE* buf = atomic_load_explicit(&v->data, memory_order_acquire); \
	VEC_MEMCPY((u8*)out, (u8*)(buf + index), sizeof(TYPE)); \
}

#define ATOMIC_VECTOR_DEFINE_STORE(TYPE) \
void atomic_vector_store(TYPE)(atomic_vector(TYPE)* v, u32 index, TYPE* in) { \
	spinlock_nolock(&v->flags, ATOMIC_RESIZE); \
	TYPE* buf = atomic_load_explicit(&v->data, memory_order_acquire); \
	VEC_MEMCPY((u8*)(buf + index), (u8*)in, sizeof(TYPE)); \
}

// require definition of VEC_MEMCPY, vector_resiz
#define ATOMIC_VECTOR_DEFINE_ADD(TYPE) \
void atomic_vector_add(TYPE)(vector(TYPE)* v, TYPE* data) {\
	u32 size = atomic_load_explicit(&v->size, memory_order_relaxed); \
	while (!atomic_compare_exchange_weak_explicit(&v->size, &size, size + 1, memory_order_release, memory_order_relaxed)); \
	u32 reserve = atomic_load_explicit(&v->reserve, memory_order_acquire); \
    if (reserve < size) \
        atomic_vector_resize(TYPE)(v, v->size * 2); \
	atomic_vector_store(TYPE)(v, size, data); \
}

#define ATOMIC_VECTOR_DEFINE_RM(TYPE) \
u32 atomic_vector_rm(TYPE)(atomic_vector(TYPE)* v) { \
	u32 size = atomic_load_explicit(&v->size, memory_order_relaxed); \
	while (!atomic_compare_exchange_weak_explicit(&v->size, &size, size - 1, memory_order_release, memory_order_relaxed)); \
	return size - 1; \
}

// require definition of VEC_ALLOC and VEC_CPY
// always copies data
#define ATOMIC_VECTOR_DEFINE_RESIZE(TYPE) \
void atomic_vector_resize(TYPE)(atomic_vector(TYPE)* v, u32 size) {\
	if (!spinlock_wait(&v->flags, ATOMIC_RESIZE)) {\
		spinlock_signal(&v->flags, ATOMIC_RESIZE); \
		return; \
	} \
	u32 vsize = atomic_load_explicit(&v->size, memory_order_acquire); \
	TYPE* old = atomic_load_explicit(&v->data, memory_order_relax); \
	TYPE* buf = (TYPE*)VEC_ALLOC(size * sizeof(TYPE)); \
	vector(u8) dst = { buf, sizeof(TYPE), size }; \
	vector(u8) src = { old, sizeof(TYPE), vsize }; \
	VEC_CPY(&dst, &src); \
	atomic_store_explicit(&v->data, buf, memory_order_release); \
	atomic_store_explicit(&v->reserve, size, memory_order_release); \
	spinlock_signal(&v->flags, ATOMIC_RESIZE); \
	free(v->data); \
}

#define VECTOR_DECLARE_FN__() VECTOR_DECLARE_FN_

#define VECTOR_DECLARE_FN_(TYPE, arg, ...) \
VECTOR_DECLARE_##arg(TYPE); \
__VA_OPT__(VECTOR_DECLARE_FN__ PAREN (TYPE, __VA_ARGS__))

#define VECTOR_DECLARE_FN(TYPE, arg, ...) \
__VA_OPT__(EXPAND(VECTOR_DECLARE_FN_(TYPE, __VA_ARGS__)))

#define ATOMIC_QUEUE_DEFINE(TYPE) \
struct atomic_queue_##TYPE { \
	atomic_vector(TYPE) data; \
	_Atomic(u32) beg; \
	_Atomic(u32) end; \
}

#define ATOMIC_QUEUE_DEFINE_INIT(TYPE) \
void atomic_queue_init(TYPE)(atomic_queue(TYPE)* q, u8* data, u32 size) { \
	atomic_vector_init(TYPE)(&q->data, data, size); \
	atomic_init(&q->beg, 0); \
	atomic_init(&q->end, 0); \
}

#define ATOMIC_QUEUE_DEFINE_DESTROY(TYPE) \
void atomic_queue_destroy(TYPE)(atomic_queue(TYPE)* q) { \
	atomic_vector_destroy(TYPE)(&q->data); \
	atomic_store_explicit(&q->beg, 0, memory_order_relaxed); \
	atomic_store_explicit(&q->end, 0, memory_order_relaxed); \
}

#define ATOMIC_QUEUE_DEFINE_LOAD(TYPE) \
void atomic_queue_load(TYPE)(atomic_queue(TYPE)* q, u32 index, TYPE* out) { \
	atomic_vector_load(TYPE)(&q->data, index, out); \
}

#define ATOMIC_QUEUE_DEFINE_STORE(TYPE) \
void atomic_queue_store(TYPE)(atomic_queue(TYPE)* q, u32 index, TYPE* in) { \
	atomic_vector_store(TYPE)(&q->data, index, in); \
}

// see requirements for vector_add
#define ATOMIC_QUEUE_DEFINE_PUSH(TYPE) \
void atomic_queue_push(TYPE)(atomic_queue(TYPE)* q, TYPE* data) { \
	u32 size = atomic_load_explicit(&q->data.size, memory_order_relaxed); \
	u32 reserve = atomic_load_explicit(&q->data.reserve, memory_order_relaxed); \
	if (reserve <= size) \
		atomic_vector_resize(TYPE)(&q->data, size * 2); \
	while (!atomic_compare_exchange_weak_explicit(&q->data.size, &size, size + 1, memory_order_release, memory_order_relaxed)); \
	u32 end = atomic_load_explicit(&q->end, memory_order_relaxed); \
	while (!atomic_compare_exchange_weak_explicit(&q->end, &end, end + 1, memory_order_release, memory_order_relaxed)); \
	atomic_queue_store(q, end, data); \
}

// see requirements for vector_rm
#define ATOMIC_QUEUE_DEFINE_POP(TYPE) \
void atomic_queue_pop(TYPE)(atomic_queue(TYPE)* q, TYPE* out) { \
	u32 beg = atomic_load_explicit(&q->beg, memory_order_relaxed); \
	while (!atomic_compare_exchange_weak_explicit(&q->beg, &beg, beg + 1, memory_order_release, memory_order_relaxed)); \
	atomic_queue_load(TYPE)(q, beg, out); \
	atomic_fetch_sub_explicit(&q->data.size, 1, memory_order_relaxed); \
}

// see requirements for vector_rm
#define ATOMIC_QUEUE_DEFINE_TRYPOP(TYPE) \
int atomic_queue_trypop(TYPE)(atomic_queue(TYPE)* q, TYPE* out) { \
	u32 beg = atomic_load_explicit(&q->beg, memory_order_acquire); \
	u32 end = atomic_load_explicit(&q->end, memory_order_acquire); \
	if (end <= beg) return 0; \
	while (!atomic_compare_exchange_weak_explicit(&q->beg, &beg, beg + 1, memory_order_release, memory_order_relaxed)); \
	atomic_queue_load(TYPE)(q, beg, out); \
	atomic_fetch_sub_explicit(&q->data.size, 1, memory_order_relaxed); \
	return 1; \
}

#define ATOMIC_QUEUE_DEFINE_FN__() ATOMIC_QUEUE_DEFINE_FN_

#define ATOMIC_QUEUE_DEFINE_FN_(TYPE, arg, ...) \
ATOMIC_QUEUE_DEFINE_##arg(TYPE); \
__VA_OPT__(ATOMIC_QUEUE_DEFINE_FN__ PAREN (TYPE, __VA_ARGS__))

#define ATOMIC_QUEUE_DEFINE_FN(TYPE, ...) \
__VA_OPT__(EXPAND(ATOMIC_QUEUE_DEFINE_FN_(TYPE, __VA_ARGS__)))

typedef atomic_vector(u8) atomic_baseptr;
typedef vector(u8) baseptr;
typedef struct mem_block mem_block;
typedef struct atomic_mem_pool atomic_mem_pool;
typedef struct atomic_mem_list atomic_mem_list;

struct mem_block
{
	baseptr* base;
	u32 handle;
	u32 size;
};

struct atomic_mem_pool
{
	atomic_baseptr base;
	_Atomic(u32) free;
};

ATOMIC_VECTOR_DECLARE(atomic_baseptr);
ATOMIC_VECTOR_DEFINE(atomic_baseptr);

struct atomic_mem_list
{
	atomic_vector(baseptr) blocks;
	_Atomic(u64) free;
};

#define atomic_mem_load(TYPE) atomic_mem_load_##TYPE
#define atomic_mem_store(TYPE) atomic_mem_store##TYPE

// require VEC_MEMCPY
#define ATOMIC_MEM_LOAD_DEFINE(TYPE) \
void atomic_mem_load(TYPE)(mem_block* block, TYPE* out) { \
	atomic_baseptr* base = (atomic_baseptr*)block->base; \
	TYPE* buf = (TYPE*)atomic_load_explicit(&base->data, memory_order_relaxed); \
	VEC_MEMCPY((u8*)out, (u8*)(buf + block->handle), sizeof(TYPE)); \
}

#define ATOMIC_MEM_STORE_DEFINE(TYPE) \
void atomic_mem_store(TYPE)(mem_block* block, TYPE* in) { \
	atomic_baseptr* base = (atomic_baseptr*)block->base; \
	spinlock_nolock(&base->flags, ATOMIC_RESIZE); \
	TYPE* buf = (TYPE*)atomic_load_explicit(&base->data, memory_order_acquire); \
	VEC_MEMCPY((u8*)(buf + block->handle), (u8*)in, sizeof(TYPE)); \
}

void atomic_pool_init(atomic_mem_pool* pool, u32 size, u32 blocksz);
void atomic_pool_destroy(atomic_mem_pool* pool);
void atomic_pool_resize(atomic_mem_pool* pool, u32 size);

mem_block atomic_pool_at(atomic_mem_pool* pool, u32 handle);
mem_block atomic_pool_alloc(atomic_mem_pool* pool);
void atomic_pool_free(mem_block* block);

void atomic_list_init(atomic_mem_list* list, u32 size, u32 blocksz);
void atomic_list_destroy(atomic_mem_list* list);
void atomic_list_resize(atomic_mem_list* list, u32 size);

u8* atomic_list_at(atomic_mem_list* list, u64 handle);
u64 atomic_list_alloc(atomic_mem_list* list);
void atomic_list_free(atomic_mem_list* list, u64 handle);
