#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

// Only supports 1 bit per pixel bmp files
static SDL_Texture *load_1bpp_bitmap(const char *path, SDL_Renderer *renderer) {
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
    uint32_t pixel_offset = *(uint32_t *)(header + 10);
    // Going to use this to allocate a full buffer for the file
    uint32_t size = *(uint32_t *)(header + 2);

    // Read the whole file
    rewind(bmp);
    data = malloc(size);
    if (fread(data, 1, size, bmp) != size) {
      fputs("load_1bpp_bitmap(): malformed bmp", stderr);
      break;
    }

    fclose(bmp);
    bmp = NULL;

    uint16_t bpp = *(uint16_t *)(data + 28);
    if (bpp != 1) {
      fputs("load_1bpp_bitmap(): bmp expected to be 1 bit per pixel", stderr);
      break;
    }
    int32_t w = *(int32_t *)(data + 18);
    int32_t h = *(int32_t *)(data + 22);
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
	size_t bmp_index = (size_t)pixel_offset + (size_t)y * (w / 8) +
			   x / 8; /* Accounting for it being
						       packed into a byte */
	size_t tex_index = (size_t)(h - y - 1) * w * 3 + (size_t)x * 3;

	if (data[bmp_index] & (128 >> (x % 8))) {
	  tex_data[tex_index + 0] = 255;
	  tex_data[tex_index + 1] = 255;
	  tex_data[tex_index + 2] = 255;
	} else {
	  tex_data[tex_index + 0] = 0;
	  tex_data[tex_index + 1] = 0;
	  tex_data[tex_index + 2] = 0;
	}
      }
    }

    tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
			    SDL_TEXTUREACCESS_STATIC, w, h);
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

  if (bmp)
    fclose(bmp);
  free(data);
  free(tex_data);
  return tex;
}