#include "scheduler.h"

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
#endif
