#include "jfile.h"

file file_open(cstr fname, int mode)
{
	file f = {};
	f.buffer = mem_alloc(BUFFER_SIZE);
	f.buffer.size = 0;

	cstr mode = mode == FILE_READ ? "rb" : "wb";
	f.handle = fopen(fname, mode);
	return f;
}

void file_read(file* f)
{
	u32 bytes = fread(FBUF_DATA(f), 1, BUFFER_SIZE, f->handle);
	f->buffer.size = bytes;
}

void file_write(file* f)
{
	u32 bytes = fwrite(FBUF_DATA(f), 1, FBUF_SIZE(f), f->handle);
	f->buffer.size = 0;
}

void file_flush(file* f)
{
	fflush(f->handle);
}

void file_close(file* f)
{
	fclose(f->handle);
	mem_free(f->buffer);
}
