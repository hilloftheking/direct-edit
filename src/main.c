#include <stdio.h>

#include <SDL.h>

#include "program.h"

// This fixes whatever SDL tries to do on Windows with main
#ifdef _WIN32
#undef main
#endif

int main(int argc, char *argv[]) {
  if (program_init(argc, argv)) {
    fputs("init failed", stderr);
  }

  int should_quit = 0;
  while (!should_quit) {
    if (program_render())
      break;
    should_quit = program_process();
  }

  program_quit();
}