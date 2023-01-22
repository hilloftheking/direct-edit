#pragma once

#include <stddef.h>

#include <SDL.h>

// A buffer of text
typedef struct Buffer {
  char *dat;
  size_t size;
  size_t cap;
  
  size_t pos; // Cursor position
  SDL_Point top_left; // For scrolling
} Buffer;

// Appends at the cursor's position
void buffer_insert(Buffer *buffer, const char *dat, size_t len);

// Pops character behind cursor
void buffer_pop(Buffer *buffer);