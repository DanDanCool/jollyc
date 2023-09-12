#pragma once

#include <stdatomic.h>
#include "vector.h"
#include "queue.h"

typedef struct threadid threadid;
struct threadid {
	void* handle;
};

typedef struct mutex mutex;
struct mutex {
	void* handle;
};

typedef struct semaphore semaphore;
struct semaphore {
	void* handle;
};

typedef struct taskinfo taskinfo;
typedef void (*pfn_task)(taskinfo* args, u32 gen);
struct taskinfo {
	pfn_task task;
	void* args;
	u32 gen;
};

QUEUE_DECLARE(taskinfo);

typedef struct scheduler scheduler;
struct scheduler {
	vector_ threads;
	queue_ wait;
	queue_ done;
	_Atomic u32 gen;
};

void scheduler_create(scheduler* s, u32 count);
void scheduler_destroy(scheduler* s);

int scheduler_cansubmit(scheduler* s);
void scheduler_submit(scheduler* s, taskinfo* task);

void scheduler_invalidate(scheduler* s);
void scheduler_waitall(scheduler* s);

void scheduler_run(scheduler* s);
void scheduler_stop(scheduler* s);

typedef int (*pfn_thread_start)(void* args);

threadid thread_create(pfn_thread_start fn, void* args);
void thread_join(threadid id);
void thread_yield();
void thread_sleep(int ms);
void thread_exit(int res);

mutex mutex_create();
void mutex_destroy(mutex mtx);
int mutex_tryacquire(mutex mtx);
void mutex_acquire(mutex mtx);
void mutex_release(mutex mtx);

semaphore semaphore_create(u32 count, u32 max_count);
void semaphore_destroy(semaphore sem);

void semaphore_tryacquire(semaphore sem);
void semaphore_acquire(semaphore sem);
void semaphore_release(semaphore sem);
