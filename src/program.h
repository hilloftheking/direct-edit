#pragma once

#include <SDL.h>

#include "buffer.h"

typedef struct Program {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *font_tex;
  Buffer text_buf;
} Program;

extern Program program;

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

static const char window_title[] = "Direct Edit";

// Initializes the program, returns 1 on SDL error, -1 on errno
int program_init(int argc, char *argv[]);

// Quits the program, duh
void program_quit();

// Renders the program if vsync won't block it for too long, returns 1 on error
int program_render();

// Processes events and keys, returns 1 to exit
int program_process();