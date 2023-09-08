#pragma once

#include "types.h"

typedef struct queue_ queue_;

// fixed size queue
struct queue_ {
	u64* data;
	u32 reserve;
	_Atomic u32 size;
	_Atomic u32 head;
	_Atomic u32 tail;
};

#define queue(type) queue_
#define queue_create(type) queue_create_
#define queue_destroy(type) queue_destroy_
#define queue_size(type) queue_size_
#define queue_add(type) queue_add_##type
#define queue_del(type) queue_del_##type
#define queue_clear(type) queue_clear_

#define QUEUE_DECLARE(type) \
void queue_add(type)(queue_* q, type* data); \
type* queue_del(type)(queue_* q); \

#define QUEUE_DEFINE(type) \
void queue_add(type)(queue_* q, type* data) { \
	queue_add_(q, (void*)data); \
} \
type* queue_del(type)(queue_* q) { \
	return (type*)queue_del_(q); \
}

void queue_create_(queue_* q, u32 size);
void queue_destroy_(queue_* q);
u32 queue_size_(queue_* q);
void queue_add_(queue_* q, void* data);
void* queue_del_(queue_* q);
void queue_clear_(queue_* q);
