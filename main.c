#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

// This fixes whatever SDL tries to do on Windows with main
#ifdef _WIN32
#undef main
#endif

typedef struct Buffer {
	char *dat;
	size_t size;
	size_t cap;
} Buffer;

void buffer_append(Buffer *buffer, const char *dat, size_t len) {
	if (buffer->size + len >= buffer->cap) {
		buffer->cap += 50 + len;
		char* new_dat = malloc(buffer->cap);
		memcpy(new_dat, buffer->dat, buffer->size);
		free(buffer->dat);
		buffer->dat = new_dat;
	}

	memcpy(buffer->dat + buffer->size, dat, len);
	buffer->size += len;
}

void buffer_pop(Buffer *buffer) {
	if (buffer->size > 0)
		buffer->size--;
	// TODO: smart memory deallocation
}

// Only supports 1 bit per pixel bmp files
SDL_Texture *load_1bpp_bitmap(const char *path, SDL_Renderer *render) {
	FILE *bmp = NULL;
	SDL_Texture *tex = NULL;
	uint8_t *data = NULL;
	uint8_t *tex_data = NULL;

	int c_error = 0;
	int sdl_error = 0;
	do {
		bmp = fopen(path, "rb");
		if (!bmp) {
			c_error = 1;
			break;
		}

		uint8_t header[14];
		if (fread(&header, 1, 14, bmp) != 14) {
			fputs("load_1bpp_bitmap(): unexpected EOF", stderr);
			break;
		}

		// Offset of the pixel array
		uint32_t pixel_offset = *(uint32_t*)(header + 10);
		// Going to use this to allocate a full buffer for the file
		uint32_t size = *(uint32_t*)(header + 2);

		// Read the whole file
		rewind(bmp);
		data = malloc(size);
		if (fread(data, 1, size, bmp) != size) {
			fputs("load_1bpp_bitmap(): malformed bmp", stderr);
			break;
		}

		fclose(bmp);
		bmp = NULL;

		uint16_t bpp = *(uint16_t*)(data + 28);
		if (bpp != 1) {
			fputs("load_1bpp_bitmap(): bmp expected to be 1 bit per pixel", stderr);
			break;
		}
		int32_t w = *(int32_t*)(data + 18);
		int32_t h = *(int32_t*)(data + 22);
		if (w < 0 || h < 0) {
			fputs("load_1bpp_bitmap(): bmp dimensions are negative", stderr);
			break;
		}
		if (w % 8 || h % 8) {
			fputs("load_1bpp_bitmap(): dimensions must be multiples of 8", stderr);
			break;
		}

		// Check if the pixels for the width and height will be in bounds
		if (pixel_offset + h * (w / 8) > size) {
			fputs("load_1bpp_bitmap(): malformed bmp", stderr);
			break;
		}

		tex_data = malloc((size_t)w * (size_t)h * 3ull); // Multiply by 3 for RGB

		for (uint32_t y = 0; y < h; y++) {
			for (uint32_t x = 0; x < w; x++) {
				size_t bmp_index = (size_t)pixel_offset + (size_t)y
					* (w / 8) + x / 8; /* Accounting for it being
										packed into a byte */
				size_t tex_index = (size_t)(h - y - 1) * w * 3 + (size_t)x * 3;

				if (data[bmp_index] & (128 >> (x % 8))) {
					tex_data[tex_index + 0] = 255;
					tex_data[tex_index + 1] = 255;
					tex_data[tex_index + 2] = 255;
				}
				else {
					tex_data[tex_index + 0] = 0;
					tex_data[tex_index + 1] = 0;
					tex_data[tex_index + 2] = 0;
				}
			}
		}

		tex = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, w, h);
		if (!tex) {
			sdl_error = 1;
			break;
		}

		if (SDL_UpdateTexture(tex, NULL, tex_data, w * 3)) {
			sdl_error = 1;
			break;
		}

		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_ADD);
	} while (0);

	if (c_error) {
		perror("load_1bpp_bitmap()");
	}
	if (sdl_error) {
		fputs(SDL_GetError(), stderr);
	}

	if (bmp) fclose(bmp);
	free(data);
	free(tex_data);
	return tex;
}

struct {
	int scale;
	int width;
	int height;
	int x_padding;
	int y_padding;
} font_settings = {
		2, // Scale
		5, // Width
		7, // Height
		1, // Horizontal Padding
		2  // Vertial Padding
};

void draw_text(SDL_Renderer *render, SDL_Texture *font_tex, const Buffer *text) {
	// dst_rect is the rectangle of the render target that will change
	SDL_Rect dst_rect;
	dst_rect.x = 0;
	dst_rect.y = 0;
	dst_rect.w = font_settings.width * font_settings.scale;
	dst_rect.h = font_settings.height * font_settings.scale;

	for (size_t i = 0; i < text->size; i++) {
		char c = text->dat[i];
		int index;
		if (c >= ' ' && c <= '~') {
			index = c - ' ';
		}
		else if (c == '\n') {
			dst_rect.y += 8 * font_settings.scale; // 7px + 1px spacing
			dst_rect.x = 0;
			continue;
		}
		else {
			index = '?' - ' ';
		}

		// The rectangle of font_tex to copy
		SDL_Rect src_rect;
		src_rect.x = 1 + (index % 18) * 5 + (index % 18) * 2;
		src_rect.y = 1 + (index / 18) * 7 + (index / 18) * 2;
		src_rect.w = 5;
		src_rect.h = 7;

		SDL_RenderCopy(render, font_tex, &src_rect, &dst_rect);

		dst_rect.x += (font_settings.width + font_settings.x_padding)
			* font_settings.scale; // 5px + 1px spacing
	}
}

int main(int argc, char *argv[]) {
	static const char starting_str[] = "hello world";

	int sdl_error = 0;
	do {
		if (SDL_Init(SDL_INIT_VIDEO)) {
			sdl_error = 1;
			break;
		}

		SDL_Window* wnd = SDL_CreateWindow(
			"Text Editor", 
			SDL_WINDOWPOS_UNDEFINED, // X position
			SDL_WINDOWPOS_UNDEFINED, // Y position
			(font_settings.width + font_settings.x_padding)
				* font_settings.scale * 80, // Width (80 characters)
			(font_settings.height + font_settings.y_padding)
				* font_settings.scale * 24, // Height (24 characters)
			SDL_WINDOW_RESIZABLE);
		if (!wnd) {
			sdl_error = 1;
			break;
		}
		SDL_Renderer* render = SDL_CreateRenderer(wnd, -1, 0);
		if (!render) {
			sdl_error = 1;
			break;
		}

		SDL_RenderSetVSync(render, 1);
		SDL_SetRenderDrawColor(render, 12, 25, 48, 255);

		SDL_Texture* font_tex = load_1bpp_bitmap("font.bmp", render);
		if (!font_tex) {
			fputs("failed to load font.bmp", stderr);
			break;
		}

		Buffer buffer = {
			malloc(50),
			0, // length
			50 // capacity
		};

		buffer_append(&buffer, starting_str, sizeof starting_str - 1);
		buffer_append(&buffer, "\n", 1);

		for (int i = 0; i < 78; i++) {
			buffer_append(&buffer, " ", 1);
		}
		buffer_append(&buffer, "80\nnew line", 11);

		int running = 1;
		SDL_StartTextInput();
		while (running) {
			SDL_RenderClear(render);
			draw_text(render, font_tex, &buffer);
			SDL_RenderPresent(render);

			SDL_Event event;
			SDL_PollEvent(&event);

			switch (event.type) {
			case SDL_QUIT:
				running = 0;
				break;
			case SDL_TEXTINPUT:
				char *input = event.text.text;
				size_t len = strlen(input);
				buffer_append(&buffer, input, len);
				
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.scancode) {
				case SDL_SCANCODE_RETURN:
					buffer_append(&buffer, "\n", 1);
					break;
				case SDL_SCANCODE_BACKSPACE:
					buffer_pop(&buffer);
					break;
				}

				break;
			}
		}
	} while (0);

	SDL_StopTextInput();

	if (sdl_error)
		fputs(SDL_GetError(), stderr);

	SDL_Quit();
}