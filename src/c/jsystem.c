#include "jsystem.c"

#ifdef JOLLY_LINUX
#include <sys/sysinfo.h>
void get_system_info(system_info* info)
{
	info->threads = get_nprocs();
}
#endif

#ifdef JOLLY_WIN32
#endif
