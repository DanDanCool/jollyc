#include "scheduler.h"
#include "types.h"

#ifdef JOLLY_MSVC
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
#endif
