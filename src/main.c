#include <stdio.h>

#include <SDL.h>

#include "bitmap.h"
#include "buffer.h"
#include "program.h"

// This fixes whatever SDL tries to do on Windows with main
#ifdef _WIN32
#undef main
#endif

int main(int argc, char *argv[]) {
  int init_err = program_init(argc, argv);
  if (init_err == 1) {
    printf("init failed: %s\n", SDL_GetError());
    return -1;
  } else if (init_err == -1) {
    perror("init failed");
    return -1;
  }

  int sdl_error = 0;
  int should_quit = 0;
  while (!should_quit) {
    sdl_error = program_render();
    if (sdl_error) {
      fputs(SDL_GetError(), stderr);
      break;
    }
    should_quit = program_process();
  }

  program_quit();
}