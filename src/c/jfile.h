#pragma once

#include <stdio.h>
#include "jtype.h"
#include "jmem.h"

enum
{
	FILE_READ,
	FILE_WRITE
};

enum
{
	BUFFER_SIZE = 4096
};

typedef struct
{
	FILE* handle;
	mem_block buffer;
} file;

inline u32 FBUF_SIZE(file* f)
{
	return f->buffer.size;
}

inline u8* FBUF_DATA(file* f)
{
	return (u8*)f->buffer.arena;
}

file file_open(cstr fname, int mode);
void file_read(file* f);
void file_write(file* f);
void file_flush(file* f);
void file_close(file* f);
