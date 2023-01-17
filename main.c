#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
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

		program.window = SDL_CreateWindow(
			window_title, 
			SDL_WINDOWPOS_UNDEFINED, // X position
			SDL_WINDOWPOS_UNDEFINED, // Y position
			(font_settings.width + font_settings.x_padding)
				* font_settings.scale * 80, // Width (80 characters)
			(font_settings.height + font_settings.y_padding)
				* font_settings.scale * 24, // Height (24 characters)
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
		SDL_SetRenderDrawColor(program.renderer, 12, 25, 48, 255);

		program.font_tex = load_1bpp_bitmap("font.bmp", program.renderer);
		if (!program.font_tex) {
			fputs("failed to load font.bmp", stderr);
			break;
		}

		Buffer *buf = &program.text_buf;
		buf->dat = malloc(50);
		buf->size = 0;
		buf->cap = 50;
		buffer_append(buf, window_title, sizeof window_title - 1);

		int should_quit = 0;
		SDL_StartTextInput();
		while (!should_quit) {
			should_quit = program_process();

			SDL_RenderClear(program.renderer);
			program_draw_text();
			SDL_RenderPresent(program.renderer);
		}
	} while (0);

	SDL_StopTextInput();

	if (sdl_error)
		fputs(SDL_GetError(), stderr);

	SDL_Quit();
}