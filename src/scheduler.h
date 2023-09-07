#pragma once

#include <stdatomic.h>

typedef struct threadid threadid;
struct threadid {
	void* handle;
};

typedef int (*pfn_thread_start)(void* args);

threadid thread_create(pfn_thread_start fn, void* args);
void thread_join(threadid id);
void thread_yield();
void thread_exit(int res);
