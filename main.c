#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "bitmap.h"
#include "buffer.h"
#include "program.h"

// This fixes whatever SDL tries to do on Windows with main
#ifdef _WIN32
#undef main
#endif

int main(int argc, char *argv[]) {
  static const char window_title[] = "Direct Edit";

  int sdl_error = 0;
  do {
    if (SDL_Init(SDL_INIT_VIDEO)) {
      sdl_error = 1;
      break;
    }

    program.window =
        SDL_CreateWindow(window_title,
                         SDL_WINDOWPOS_UNDEFINED, // X position
                         SDL_WINDOWPOS_UNDEFINED, // Y position
                         (font_settings.width + font_settings.x_padding) *
                             font_settings.scale * 80, // Width (80 characters)
                         (font_settings.height + font_settings.y_padding) *
                             font_settings.scale * 24, // Height (24 characters)
                         SDL_WINDOW_RESIZABLE);
    if (!program.window) {
      sdl_error = 1;
      break;
    }
    program.renderer = SDL_CreateRenderer(program.window, -1, 0);
    if (!program.renderer) {
      sdl_error = 1;
      break;
    }

    SDL_RenderSetVSync(program.renderer, 1);
    SDL_SetRenderDrawBlendMode(program.renderer, SDL_BLENDMODE_BLEND);

    program.font_tex = load_1bpp_bitmap("font.bmp", program.renderer);
    if (!program.font_tex) {
      fputs("failed to load font.bmp", stderr);
      break;
    }

    // Put the window title into the program's text buffer
    Buffer *buf = &program.text_buf;
    buf->dat = malloc(50);
    buf->size = 0;
    buf->cap = 50;
    buffer_insert(buf, window_title, sizeof window_title - 1);

    // This is used to attempt to synchronize the renderer with vsync so
    // that events are handled ASAP
    Uint64 ms_render_interval = 0;
    Uint64 last_present_tick = 0;
    SDL_DisplayMode disp_mode = {0};

    int should_quit = 0;
    SDL_StartTextInput();
    while (!should_quit) {
      should_quit = program_process();

      // Only attempt to render a little before the present should be
      // due. This prevents blocking the event handling
      if (SDL_GetTicks64() - last_present_tick > ms_render_interval) {
        SDL_SetRenderDrawColor(program.renderer, 12, 25, 48, 255);
        if (SDL_RenderClear(program.renderer)) {
          sdl_error = 1;
          break;
        }

        SDL_Point cursor_point = program_draw_text();
        SDL_Rect cursor_rect = {cursor_point.x, cursor_point.y,
                                font_settings.width * font_settings.scale,
                                font_settings.height * font_settings.scale};
        SDL_SetRenderDrawColor(program.renderer, 255, 255, 255, 100);
        SDL_RenderFillRect(program.renderer, &cursor_rect);

        SDL_RenderPresent(program.renderer);
        last_present_tick = SDL_GetTicks64();

        // Change the interval at which presenting is attempted
        // since the refresh rate may have changed
        if (SDL_GetCurrentDisplayMode(SDL_GetWindowDisplayIndex(program.window),
                                      &disp_mode)) {
          sdl_error = 1;
          break;
        }
        // Try to present 1ms early so vsync is beaten
        ms_render_interval = (1000 / disp_mode.refresh_rate) - 1;
        if (ms_render_interval == (Uint64)-1) {
          // Future proofing for 1000hz+
          ms_render_interval = 0;
        }
      }
    }
  } while (0);

  SDL_StopTextInput();

  if (sdl_error)
    fputs(SDL_GetError(), stderr);

  SDL_Quit();
}