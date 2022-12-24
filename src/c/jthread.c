#include "jthread.h"
#include "jsystem.h"

#include <stdlib.h>

typedef struct task_t task_t;
typedef struct dependency_t dependency_t;

struct task_t
{
	pfn_task task;
};

struct dependency_t
{
	u32 lock;
	u32 task;
	u32 next;
};

static void task_function(thread* t)
{
	while (true)
	{
		thread_trykill(t);
		thread_trypause(t);

		t->task(t);
	}
}

void scheduler_init(scheduler* s)
{
	arena_init(s->tasks, 32, sizeof(task_t));
	arena_init(s->dependencies, 32, sizeof(dependency_t));
	arena_init(s->semaphores, 32, sizeof(semaphore));

	system_info info;
	get_system_info(&info);
	s->threads.data = (thread*)aligned_alloc(64, sizeof(thread) * info.threads);
	s->threads.size = info.threads;

	for (u32 i = 0; i < info.threads; i++)
	{
		thread* t = &s->threads.data[i];
		thread_create(t, task_function, THREAD_PAUSE);
		thread_pause(t);
		thread_start(t);
	}
}

void scheduler_destroy(scheduler* s)
{
	arena_destroy(s->tasks);
	arena_destroy(s->dependencies);
	arena_destroy(s->semaphores);
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
