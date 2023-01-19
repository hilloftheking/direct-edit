#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

void buffer_insert(Buffer *buffer, const char *dat, size_t len) {
  // Destination might change if allocating more memory
  char *dest = buffer->dat;
  size_t needed_cap = buffer->size + len;

  if (needed_cap > buffer->cap) {
    puts("allocating more");
    buffer->cap += 50 + len;
    dest = malloc(buffer->cap);
    memcpy(dest, buffer->dat, buffer->pos);
  }

  // Move text starting at the cursor to cursor+len (out of the way of
  // the inserted text)
  memcpy(dest + buffer->pos + len, buffer->dat + buffer->pos,
         buffer->size - buffer->pos);
  // Copy the data to be inserted to the cursor position
  memcpy(dest + buffer->pos, dat, len);
  buffer->size += len;
  buffer->pos += len;

  if (dest != buffer->dat) {
    free(buffer->dat);
    buffer->dat = dest;
  }
}

void buffer_pop(Buffer *buffer) {
  if (buffer->size == 0 || buffer->pos == 0)
    return;

  memcpy(buffer->dat + buffer->pos - 1, buffer->dat + buffer->pos,
         buffer->size - buffer->pos);
  buffer->size--;
  if (buffer->pos != 0)
    buffer->pos--;
  // TODO: smart memory deallocation
}