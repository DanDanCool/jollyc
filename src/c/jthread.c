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
	_Atomic(u32) in; // dependency refcount
	u32 out; // dependencies to signal, FUTURE: store this as a vector
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
	scheduler* s = (scheduler*)t->data;
	task_t task;
	while (true)
	{
		while (true)
		{
			thread_trykill(t);
			thread_trypause(t);

			u32 handle;
			int acquired = atomic_queue_trypop(u32)(&s->taskq, &handle);
			if (acquired)
			{
				mem_block mem = atomic_pool_at(&s->tasks, handle);
				atomic_mem_load(task_t)(&mem, &task);
				break;
			}
		}

		task.task(task.data);

		tasklock_t* lock = (tasklock_t*)atomic_list_at(task.lock);
		semaphore_signal(&lock.lock);

		u32 handle = task.out;
		while (handle != U32_MAX)
		{
			dep_t dep;
			mem_block mem = atomic_pool_at(&s->deps, handle);
			atomic_mem_load(dep_t)(&mem, &dep);
			task_signal(dep.task);
			handle = dep.next;
			atomic_pool_free(&mem);
		}
	}
}

void scheduler_init(scheduler* s)
{
	const u32 default_size = BLOCK_64;
	atomic_queue_init(task_t)(&s->taskq, jolly_alloc(sizeof(u32) * default_size), default_size);
	atomic_pool_init(&s->tasks, default_size, sizeof(task_t));
	atomic_pool_init(&s->deps, default_size, sizeof(dep_t));
	atomic_list_init(&s->locks, default_size, sizeof(tasklock_t));

	system_info info;
	get_system_info(&info);
	vector_init(thread)(&s->threads, jolly_alloc(sizeof(thread) * info.threads), info.threads);
	s->threads.size = info.threads;

	for (u32 i = 0; i < info.threads; i++)
	{
		thread* t = &s->threads.data[i];
		thread_create(t, task_function, THREAD_PAUSE);
		t->data = (void*)s;
		thread_pause(t);
		thread_start(t);
	}
}

void scheduler_destroy(scheduler* s)
{
	schedular_waitall(s);
	vector_destroy(thread)(&s->threads);
	atomic_queue_destroy(u32)(&s->taskq);
	pool_destroy(s->tasks);
	pool_destroy(s->deps);
	list_destroy(s->locks);
}

static u64 lock_create(scheduler* s)
{
	u64 handle = atomic_list_alloc(&s->locks);
	if (handle & ALLOC_NEW)
	{
		tasklock_t* lock = (tasklock_t*)atomic_list_at(&s->locks, handle);
		semaphore_init(&lock->lock);
	}

	return handle;
}

u32 task_create(scheduler* s, pfn_task, void* data)
{
	mem_block mem = atomic_pool_alloc(&s->tasks);
	task_t task;
	task->task = task;
	task->data = data;
	task->lock = lock_create(s);
	task->in = 0
	task->out = U32_MAX;
	atomic_mem_store(task_t)(&mem, &task);
	return mem->handle;
}

u32 dependency_create(scheduler* s, u32 task)
{
	mem_block tmem = atomic_pool_at(&s->tasks, task);
	task_t tcpy;
	atomic_mem_load(task_t)(&tmem, &tcpy);

	mem_block mem = atomic_pool_alloc(&s->deps);
	dep_t dep = { U32_MAX, tcpy.out };
	tcpy.out = mem->handle;
	atomic_mem_store(task_t)(&tmem, &task);
	atomic_mem_store(dep_t)(&mem, &dep);
}

void dependency_add(scheduler* s, u32 task, u32 dependency)
{
	mem_block mem = atomic_pool_at(&s->deps, dependency);
	dep_t dep;
	atomic_mem_load(dep_t)(&mem, &dep);
	dep.task = task;
	atomic_mem_store(dep_t)(&mem, &dep);
	task_wait(s, task);
}

void task_wait(scheduler* s, u32 task)
{
	u8* buf = atomic_load_explicit(&s->tasks.base.data, memory_order_relaxed);
	task_t* t = buf + task * sizeof(task_t);
	atomic_fetch_add_explicit(&t->in, 1, memory_order_relaxed);
}

void task_signal(scheduler* s, u32 task)
{
	u8* buf = atomic_load_explicit(&s->tasks.base.data, memory_order_relaxed);
	task_t* t = buf + task * sizeof(task_t);
	atomic_fetch_sub_explicit(&t->in, 1, memory_order_release);
	u32 val = atomic_load_explicit(&t->in, memory_order_acquire);
	if (!val)
		atomic_queue_push(&s->taskq, &task);
}

void scheduler_submit(scheduler* s, u32* tasks, u32 count)
{
	u8* buf = atomic_load_explicit(&s->tasks.base.data, memory_order_relaxed);
	for (u32 i = 0; i < count; i++)
	{
		atomic_queue_push(u32)(&s->taskq, tasks + i);
		task_t* t = buf + task[i] * sizeof(task_t);
		tasklock_t* lock = (tasklock_t*)atomic_list_at(&s->locks, t->lock);
		semaphore_trywait(&lock->lock);
	}

	// wake up threads
}

void scheduler_waittask(scheduler* s, u32* tasks, u32 count)
{
	for (u32 i = 0; i < count; i++)
	{
		task_t* t = buf + task[i] * sizeof(task_t);
		tasklock_t* lock = (tasklock_t*)atomic_list_at(&s->locks, t->lock);
		semaphore_wait(&lock->lock);
	}
}

void scheduler_waitall(scheduler* s)
{
	for (u32 i = 0; i < s->threads.size; i++)
	{
		thread* t = vector_at(thread)(&s->threads, i);
		semaphore_wait(&t->lock);
	}
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
	u32 flags = atomic_load_explicit(&t->flags, memory_order_relaxed);
	while (!atomic_compare_exchange_weak_explicit(&t->flags, &flags, flags | THREAD_RUNNING, memory_order_release, memory_order_relaxed));

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
	while (!atomic_compare_exchange_weak_explicit(&t->flags, &flags, flags | THREAD_KILL, memory_order_release, memory_order_relaxed));
}

void thread_pause(thread* t)
{
	semaphore_trywait(&t->lock);
	u32 flags = atomic_load_explicit(&t->flags, memory_order_relaxed);
	while (!atomic_compare_exchange_weak_explicit(&t->flags, &flags, flags | THREAD_PAUSE, memory_order_release, memory_order_relaxed));
}

void thread_unpause(thread* t)
{
	u32 flags = atomic_load_explicit(&t->flags, memory_order_relaxed);
	while (!atomic_compare_exchange_weak_explicit(&t->flags, &flags, flags & ~THREAD_PAUSE, memory_order_release, memory_order_relaxed));
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
