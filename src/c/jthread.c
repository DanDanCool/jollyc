#include "jthread.h"
#include "jsystem.h"

#include <stdlib.h>

typedef struct task_t task_t;
typedef struct dep_t dep_t;
typedef struct tasklock_t tasklock_t;

struct task_t
{
	pfn_task task;
	void* data;
	u64 lock;
	u32 in; // dependency refcount
	u32 out; // dependencies to signal
};

struct dep_t
{
	u32 task;
	u32 next;
};

struct tasklock_t
{
	u64 out; // associated dependency
	semaphore lock;
};

static void task_function(thread* t)
{
	while (true)
	{
		while (true)
		{
			thread_trykill(t);
			thread_trypause(t);

			if (true)
				break;
		}

		task_t* task = (task_t*)t->data;
		task->task(task->data);
	}
}

void scheduler_init(scheduler* s)
{
	const u32 default_size = BLOCK_32;
	pool_init(&s->tasks, default_size, sizeof(task_t));
	pool_init(&s->deps, default_size, sizeof(dep_t));
	list_init(&s->locks, default_size, sizeof(tasklock_t));

	system_info info;
	get_system_info(&info);
	vector_init(thread)(&s->threads, jolly_alloc(sizeof(thread) * info.threads), info.threads);
	s->threads.size = info.threads;

	for (u32 i = 0; i < info.threads; i++)
	{
		thread* t = &s->threads.data[i];
		thread_create(t, task_function, THREAD_PAUSE);
		thread_pause(t);
		thread_start(t);
	}

	queue_init(task_t)(&s->taskq, jolly_alloc(sizeof(u32) * default_size), default_size);
}

void scheduler_destroy(scheduler* s)
{
	schedular_waitall(s);
	vector_destroy(thread)(&s->threads);
	queue_destroy(u32)(&s->taskq);
	pool_destroy(s->tasks);
	pool_destroy(s->deps);
	list_destroy(s->locks);
}

u32 scheduler_taskcreate(scheduler* s, pfn_task, void* data)
{
	mem_block mem = pool_alloc(&s->tasks);
	task_t* task = (task_t*)MEM_DATA(&mem);
	task->task = task;
	task->data = data;
	task->signal = U32_MAX;
	task->dependencies = U32_MAX;
	return mem->handle;
}

u32 scheduler_dependencycreate(scheduler* s, u32 task)
{

}

void thread_create(thread* t, pfn_task task, u32 flags)
{
	if ((flags & THREAD_PAUSE) == THREAD_PAUSE)
	{
		semaphore_create(&t->lock);
		flags = flags & ~THREAD_PAUSE;
	}

	t->task = task;
	atomic_init(&t->flags, flags);
}

#ifdef JOLLY_LINUX
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>

void thread_start(thread* t)
{
	u32 flags = atomic_load_explicit(&t->flags, memory_order_acquire);
	flags = flags | THREAD_RUNNING;
	atomic_store_explicit(&t->flags, flags, memory_order_release);

	pthread_t id;
	pthread_create(&id, NULL, t->task, t);
	t->handle = (u64)id;

	if ((flags & THREAD_DETACH) == THREAD_DETACH)
		pthread_detach(id);
}

void thread_join(thread* t)
{
	pthread_join((pthread_t)t->handle, NULL);
}

int thread_tryjoin(thread* t)
{
	return pthread_tryjoin_np((pthread_t)t->handle, NULL);
}

int thread_waitjoin(thread* t, u32 ns)
{
	struct timespec ts = {};
	ts.tv_nsec = ns;
	return pthread_timedjoin_np((pthread_t)t->handle, NULL, &ts);
}

void thread_kill(thread* t)
{
	u32 flags = atomic_load_explicit(&t->flags, memory_order_relaxed);
	u32 new = flags | THREAD_KILL;

	while (!atomic_compare_exchange_weak_explicit(&t->flags, &flags, new, memory_order_release, memory_order_relaxed))
		new = flags | THREAD_KILL;
}

void thread_pause(thread* t)
{
	semaphore_trywait(&t->lock);
	u32 flags = atomic_load_explicit(&t->flags, memory_order_relaxed);
	u32 new = flags | THREAD_PAUSE;

	while (!atomic_compare_exchange_weak_explicit(&t->flags, &flags, new, memory_order_release, memory_order_relaxed))
		new = flags | THREAD_PAUSE;
}

void thread_unpause(thread* t)
{
	u32 flags = atomic_load_explicit(&t->flags, memory_order_relaxed);
	u32 new = flags & ~THREAD_PAUSE;

	while (!atomic_compare_exchange_weak_explicit(&t->flags, &flags, new, memory_order_release, memory_order_relaxed))
		new = flags & ~THREAD_PAUSE;

	semaphore_signal(&t->lock);
}

void thread_trykill(thread* t)
{
	u32 flags = atomic_load_explicit(&t->flags, memory_order_acquire);
	if ((flags & THREAD_KILL) != THREAD_KILL)
		return;

	thread_exit(t);
}

void thread_trypause(thread* t)
{
	u32 flags = atomic_load_explicit(&t->flags, memory_order_acquire);
	if ((flags & THREAD_PAUSE) != THREAD_PAUSE)
		return;

	semaphore_wait(&t->lock);
	semaphore_signal(&t->lock);
}

void thread_exit(thread* t)
{
	pthread_exit(NULL);
}

void thread_yield(thread* t)
{
	sched_yield();
}

void semaphore_create(semaphore* sem)
{
	sem_t* id = (sem_t*)&sem->handle;
	sem_init(id, 0, 1);
}

void semaphore_destroy(semaphore* sem)
{
	sem_t* id = (sem_t*)&sem->handle;
	sem_destroy(id);
	sem->handle = U64_MAX;
}

void semaphore_wait(semaphore* sem)
{
	sem_t* id = (sem_t*)&sem->handle;
	sem_wait(id);
}

int semaphore_trywait(semaphore* sem)
{
	sem_t* id = (sem_t*)&sem->handle;
	return sem_trywait(id);
}

void semaphore_signal(semaphore* sem)
{
	sem_t* id = (sem_t*)&sem->handle;
	sem_post(id);
}

int semaphore_value(semaphore* sem)
{
	sem_t* id = (semi_t*)&sem->handle;
	int val;
	sem_getvalue(id, &val);
	return val;
}
#endif

#ifdef JOLLY_WIN32
#endif
