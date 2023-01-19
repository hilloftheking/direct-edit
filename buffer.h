#pragma once

#include <stddef.h>

// A buffer of text
typedef struct Buffer {
  char *dat;
  size_t size;
  size_t cap;
  size_t pos; // cursor position
} Buffer;

// Appends at the cursor's position
void buffer_insert(Buffer *buffer, const char *dat, size_t len);

// Pops character behind cursor
void buffer_pop(Buffer *buffer);