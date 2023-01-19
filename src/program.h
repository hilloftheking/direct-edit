#pragma once

#include <string.h>

#include <SDL.h>

#include "buffer.h"

typedef struct Program {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *font_tex;
  Buffer text_buf;
} Program;

Program program;

struct {
  int scale;
  int width;
  int height;
  int x_padding;
  int y_padding;
} static font_settings = {
    2, // Scale
    5, // Width
    7, // Height
    1, // Horizontal Padding
    2  // Vertial Padding
};

// Draws each character in buffer and returns where the cursor should be
static SDL_Point program_draw_text() {
  // dst_rect is the rectangle of the render target that will change
  SDL_Rect dst_rect;
  dst_rect.x = 0;
  dst_rect.y = 0;
  dst_rect.w = font_settings.width * font_settings.scale;
  dst_rect.h = font_settings.height * font_settings.scale;

  SDL_Point ret_cursor_point;

  // Include the end of text_buf in case the cursor is there.
  for (size_t i = 0; i <= program.text_buf.size; i++) {
    if (i == program.text_buf.pos) {
      ret_cursor_point.x = dst_rect.x;
      ret_cursor_point.y = dst_rect.y;
    }
    if (i == program.text_buf.size)
      break;

    char c = program.text_buf.dat[i];
    int index;
    if (c >= ' ' && c <= '~') {
      index = c - ' ';
    } else if (c == '\n') {
      dst_rect.y += 8 * font_settings.scale; // 7px + 1px spacing
      dst_rect.x = 0;
      continue;
    } else {
      index = '?' - ' ';
    }

    // The rectangle of font_tex to copy
    SDL_Rect src_rect;
    src_rect.x = 1 + (index % 18) * 5 + (index % 18) * 2;
    src_rect.y = 1 + (index / 18) * 7 + (index / 18) * 2;
    src_rect.w = 5;
    src_rect.h = 7;

    SDL_RenderCopy(program.renderer, program.font_tex, &src_rect, &dst_rect);

    dst_rect.x += (font_settings.width + font_settings.x_padding) *
                  font_settings.scale; // 5px + 1px spacing
  }

  return ret_cursor_point;
}

// Returns 0 to continue, 1 to exit
static int program_process() {
  SDL_Event event;
  SDL_PollEvent(&event);

  int should_quit = 0;
  switch (event.type) {
  case SDL_QUIT:
    should_quit = 1;
    break;
  case SDL_TEXTINPUT: {
    char *input = event.text.text;
    size_t len = strlen(input);
    buffer_insert(&program.text_buf, input, len);
  }

  break;
  case SDL_KEYDOWN:
    switch (event.key.keysym.scancode) {
    case SDL_SCANCODE_RETURN:
      buffer_insert(&program.text_buf, "\n", 1);
      break;
    case SDL_SCANCODE_BACKSPACE:
      buffer_pop(&program.text_buf);
      break;
    case SDL_SCANCODE_LEFT:
      if (program.text_buf.pos != 0)
        program.text_buf.pos--;
      break;
    case SDL_SCANCODE_RIGHT:
      if (program.text_buf.pos != program.text_buf.size)
        program.text_buf.pos++;
      break;
    }

    break;
  }

  return should_quit;
}