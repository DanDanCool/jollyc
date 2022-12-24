#pragma once

#include <stdatomic.h>

#include "jtype.h"
#include "jmem.h"

#define atomic(x) _Atomic x

typedef struct thread thread;
typedef struct semaphore semaphore;
typedef struct scheduler scheduler;

typedef void (*pfn_task)(thread* t);

enum
{
	THREAD_INVALID = 0,
	THREAD_RUNNING = 1 << 0,
	THREAD_KILL = 1 << 1,
	THREAD_PAUSE = 1 << 2,
	THREAD_DETACH = 1 << 3,
};

struct semaphore
{
	u64 handle;
};

// sem_t is 4 bytes while pthread_t is 8 bytes
// user responsible for providing thread memory
struct thread
{
	u64 handle;
	semaphore lock;
	pfn_task task;
	mem_block data;
	atomic(u32) flags;
};

struct scheduler;
{
	struct
	{
		thread* data;
		u32 size;
	} threads;

	mem_arena tasks;
	mem_arena dependencies;
	mem_arena semaphores;
};

void thread_create(thread* t, u32 flags);
void thread_start(thread* t);

void thread_join(thread* t);
int thread_tryjoin(thread* t);

void thread_kill(thread* t);

// should only be called with threads created with the THREAD_PAUSE flag
void thread_pause(thread* t);
void thread_unpause(thread* t);

void thread_priority(thread* t, int priority);

// check if the thread should be killed/paused
void thread_trykill(thread* t);
void thread_trypause(thread* t);

// call when the thread has finished execution
void thread_exit(thread* t);

// yield remaining scheduler time
void thread_yield(thread* t);

void semaphore_create(semaphore* sem);
void semaphore_destroy(semaphore* sem);

void semaphore_wait(semaphore* sem);
int semaphore_trywait(semaphore* sem);

void semaphore_signal(semaphore* sem);
int semaphore_value(semaphore* sem);

void scheduler_init(scheduler* s);
void scheduler_destroy(scheduler* s);
u32 scheduler_addtask(scheduler* s, pfn_task task, u32 dependency);
void scheduler_waittask(scheduler* s, u32* tasks);
void scheduler_waitall(scheduler* s);
