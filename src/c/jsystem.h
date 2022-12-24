#pragma once

typedef struct
{
	u32 threads;
} system_info;

void get_system_info(system_info* info);
