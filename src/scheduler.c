#include "scheduler.h"
#include "types.h"

typedef struct worker worker;
struct worker {
	threadid handle;
	semaphore lock;
	semaphore starve;
	scheduler* sched;
	_Atomic u32 run;
};

VECTOR_DECLARE(worker);

int worker_main(void* in) {
	worker* args = (worker*)in;
	scheduler* sched = args->sched;

	u32 run = atomic_load_explicit(&args->run, memory_order_relaxed);

	const u32 MAX_BACKOFF = 128;
	const u32 INITIAL_BACKOFF = 16;
	u32 backoff = INITIAL_BACKOFF;
	while (run) {
		semaphore_acquire(args->lock);
		taskinfo* task = queue_del(taskinfo)(&sched->wait);
		if (!task) {
			semaphore_release(args->starve);
			semaphore_release(args->lock);
			thread_sleep(backoff);
			backoff = MIN(MAX_BACKOFF, backoff * 2);
			continue;
		}

		backoff = INITIAL_BACKOFF;
		semaphore_tryacquire(args->starve);

		task->task(task->args);
		queue_add(taskinfo)(&sched->done, task);
		semaphore_release(args->lock);

		run = atomic_load_explicit(&args->run, memory_order_relaxed);
	}

	return 0;
}

const static u32 THREAD_COUNT = 4;
const static u32 TASK_COUNT = 128;

void scheduler_create(scheduler* s, u32 count) {
	vector_create(worker)(&s->threads, count);
	queue_create(taskinfo)(&s->wait, TASK_COUNT);
	queue_create(taskinfo)(&s->done, TASK_COUNT);

	s->threads.size = count;
	for (u32 i = 0; i < s->threads.size; i++) {
		worker* t = vector_at(worker)(&s->threads, i);
		t->lock = semaphore_create(1, 1);
		t->starve = semaphore_create(0, 1);
		t->sched = s;
		atomic_store_explicit(&t->run, 1, memory_order_release);
		t->handle = thread_create(worker_main, t);
	}
}

void scheduler_destroy(scheduler* s) {
	for (u32 i = 0; i < s->threads.size; i++) {
		worker* t = vector_at(worker)(&s->threads, i);
		atomic_store_explicit(&t->run, 0, memory_order_release);
		thread_join(t->handle);
		semaphore_destroy(t->lock);
		semaphore_destroy(t->starve);
	}

	vector_destroy(worker)(&s->threads);
	queue_destroy(taskinfo)(&s->wait);
	queue_destroy(taskinfo)(&s->done);
}

int scheduler_cansubmit(scheduler* s) {
	u32 size = queue_size(taskinfo)(&s->wait) + queue_size(taskinfo)(&s->done);
	return size < TASK_COUNT;
}

void scheduler_submit(scheduler* s, taskinfo* task) {
	queue_add(taskinfo)(&s->wait, task);
}

void scheduler_waitall(scheduler* s) {
	for (u32 i = 0; i < s->threads.size; i++) {
		worker* t = vector_at(worker)(&s->threads, i);
		semaphore_acquire(t->starve);
	}
}

void scheduler_run(scheduler* s) {
	for (u32 i = 0; i < s->threads.size; i++) {
		worker* t = vector_at(worker)(&s->threads, i);
		semaphore_release(t->lock);
	}
}

void scheduler_stop(scheduler* s) {
	for (u32 i = 0; i < s->threads.size; i++) {
		worker* t = vector_at(worker)(&s->threads, i);
		semaphore_acquire(t->lock);
	}
}

COPY_DEFINE(worker);
SWAP_DEFINE(worker);

QUEUE_DEFINE(taskinfo);
VECTOR_DEFINE(worker);

#ifdef JOLLY_WIN32
#include <windows.h>
#include <process.h>
threadid thread_create(pfn_thread_start fn, void* args) {
	threadid res = {0};
	res.handle = (void*)_beginthreadex(NULL, 0, fn, args, 0, NULL);
	return res;
}

void thread_yield() {
	(void)SwitchToThread();
}

void thread_sleep(int ms) {
	Sleep(ms);
}

void thread_exit(int res) {
	_endthreadex(res);
}

void thread_join(threadid id) {
	WaitForSingleObject((HANDLE)id.handle, INFINITE);
}

mutex mutex_create() {
	mutex mtx = {0};
	mtx.handle = (void*)CreateMutex(NULL, false, NULL);
	return mtx;
}

void mutex_destroy(mutex mtx) {
	CloseHandle(mtx.handle);
}

int mutex_tryacquire(mutex mtx) {
	DWORD res = WaitForSingleObject((HANDLE)mtx.handle, 0);
	return res == WAIT_TIMEOUT ? 0 : 1;
}

void mutex_acquire(mutex mtx) {
	WaitForSingleObject((HANDLE)mtx.handle, INFINITE);
}

void mutex_release(mutex mtx) {
	ReleaseMutex((HANDLE)mtx.handle);
}

semaphore semaphore_create(u32 count, u32 max_count) {
	HANDLE tmp = CreateSemaphoreA(NULL, count, max_count, NULL);
	semaphore sem = { (void*)tmp };
	return sem;
}

void semaphore_destroy(semaphore sem) {
	CloseHandle((HANDLE)sem.handle);
}

void semaphore_tryacquire(semaphore sem) {
	WaitForSingleObject((HANDLE)sem.handle, 0);
}

void semaphore_acquire(semaphore sem) {
	WaitForSingleObject((HANDLE)sem.handle, INFINITE);
}

void semaphore_release(semaphore sem) {
	LONG count = -1;
	ReleaseSemaphore((HANDLE)sem.handle, 1, &count);
}
#endif
