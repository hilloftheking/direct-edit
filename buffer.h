#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct Buffer {
	char *dat;
	size_t size;
	size_t cap;
} Buffer;

static void buffer_append(Buffer *buffer, const char *dat, size_t len) {
	if (buffer->size + len >= buffer->cap) {
		buffer->cap += 50 + len;
		char* new_dat = (char*)malloc(buffer->cap);
		memcpy(new_dat, buffer->dat, buffer->size);
		free(buffer->dat);
		buffer->dat = new_dat;
	}

	memcpy(buffer->dat + buffer->size, dat, len);
	buffer->size += len;
}

static void buffer_pop(Buffer *buffer) {
	if (buffer->size > 0)
		buffer->size--;
	// TODO: smart memory deallocation
}