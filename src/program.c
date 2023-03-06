#include <stddef.h>
#include <stdio.h>

#include <SDL.h>

#include "bitmap.h"
#include "buffer.h"
#include "program.h"

Program program;

// This changes when the window is clicked somewhere
struct {
  int changed;
  SDL_Point pos;
} clicked;

int program_init(int argc, char *argv[]) {
  Buffer *buf = &program.text_buf;

  if (argc > 1) {
    FILE *file = fopen(argv[1], "r");
    if (!file) {
      perror("fopen()");
      return 1;
    }

    // Load the file into the program's buffer
    char file_content[64];
    size_t size_read;
    do {
      size_read = fread(file_content, 1, sizeof file_content, file);
      buffer_insert(buf, file_content, size_read);
    } while (size_read == sizeof file_content);
  } else {
    buffer_insert(buf, window_title, sizeof window_title - 1);
  }

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

    SDL_StartTextInput();
  } while (0);

  if (sdl_error)
    fputs(SDL_GetError(), stderr);

  return sdl_error;
}

void program_quit() {
  SDL_StopTextInput();
  SDL_Quit();
}

// Draws text (and sets cursor position)
static SDL_Point program_draw_text() {
  /* TODO: Further optimize scrolling:
      - maybe a list of line start positions? this would make it easier to
      set the cursor position and to tell what text needs to be rendered */

  SDL_Point *top_left = &program.text_buf.top_left;

  // The visible area where text should be drawn
  SDL_Rect visible = {top_left->x, top_left->y, 0, 0};
  SDL_GetWindowSize(program.window, &visible.w, &visible.h);

  // dst_rect is the rectangle of the render target that will change
  SDL_Rect dst_rect = {0, 0, font_settings.width * font_settings.scale,
                       font_settings.height * font_settings.scale};

  SDL_Point ret_cursor_point;

  // Include the end of text_buf in case the cursor is there.
  for (size_t i = 0; i <= program.text_buf.size; i++) {
    if (i == program.text_buf.pos) {
      ret_cursor_point.x = dst_rect.x - top_left->x;
      ret_cursor_point.y = dst_rect.y - top_left->y;
    }
    if (i == program.text_buf.size)
      break;

    char c = program.text_buf.dat[i];

    if (clicked.changed) {
      SDL_Rect click_rect = dst_rect;
      click_rect.w += font_settings.x_padding * font_settings.scale;
      click_rect.h += font_settings.y_padding * font_settings.scale;
      // If click y pos is within this line
      if (clicked.pos.y >= click_rect.y &&
          clicked.pos.y < click_rect.y + click_rect.h) {
        // If click x pos is within this character
        if (clicked.pos.x >= click_rect.x &&
            clicked.pos.x < click_rect.x + click_rect.w) {
          program.text_buf.pos = i;
          clicked.changed = 0;
        } else if (c == '\n' || i == program.text_buf.size - 1) {
          // If this characer is the end of the line, just put the cursor here
          program.text_buf.pos = i;
          clicked.changed = 0;
        }
      }
    }

    if (c == '\n') {
      dst_rect.y += (font_settings.height + font_settings.y_padding) *
                    font_settings.scale;
      dst_rect.x = 0;
      continue;
    }

    if (SDL_PointInRect((SDL_Point *)&dst_rect, &visible)) {
      int index;
      if (c >= ' ' && c <= '~') {
        index = c - ' ';
      } else {
        index = '?' - ' ';
      }

      // The rectangle of font_tex to copy
      SDL_Rect src_rect;
      src_rect.x = 1 + (index % 18) * 5 + (index % 18) * 2;
      src_rect.y = 1 + (index / 18) * 7 + (index / 18) * 2;
      src_rect.w = 5;
      src_rect.h = 7;

      // Convert dst_rect to window coordinates
      dst_rect.x -= top_left->x;
      dst_rect.y -= top_left->y;

      SDL_RenderCopy(program.renderer, program.font_tex, &src_rect, &dst_rect);

      // Revert dst_rect back to actual coordinates
      dst_rect.x += top_left->x;
      dst_rect.y += top_left->y;
    }

    dst_rect.x += (font_settings.width + font_settings.x_padding) *
                  font_settings.scale; // 5px + 1px spacing
  }

  clicked.changed = 0;
  return ret_cursor_point;
}

int program_render() {
#define EXPAND_COLOR(sdl_col) sdl_col.r, sdl_col.g, sdl_col.b

  int sdl_error = 0;
  do {
    SDL_SetRenderDrawColor(program.renderer, EXPAND_COLOR(colors.bg), 255);
    if (SDL_RenderClear(program.renderer)) {
      sdl_error = 1;
      break;
    }

    if (SDL_SetTextureColorMod(program.font_tex, EXPAND_COLOR(colors.fg))) {
      sdl_error = 1;
      break;
    }
    SDL_Point cursor_point = program_draw_text();
    SDL_Rect cursor_rect = {cursor_point.x, cursor_point.y,
                            font_settings.width * font_settings.scale,
                            font_settings.height * font_settings.scale};
    SDL_SetRenderDrawColor(program.renderer, 0, 0, 0, 100);
    SDL_RenderFillRect(program.renderer, &cursor_rect);

    SDL_RenderPresent(program.renderer);
  } while (0);

  if (sdl_error)
    fputs(SDL_GetError(), stderr);

  return sdl_error;

#undef EXPAND_COLOR
}

static void handle_key(SDL_Scancode scancode) {
  switch (scancode) {
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

  case SDL_SCANCODE_HOME:
    while (program.text_buf.pos != 0) {
      program.text_buf.pos--;
      if (program.text_buf.dat[program.text_buf.pos] == '\n') {
        program.text_buf.pos++;
        break;
      }
    }
    break;

  case SDL_SCANCODE_END:
    while (program.text_buf.pos != program.text_buf.size) {
      if (program.text_buf.dat[program.text_buf.pos] == '\n') {
        break;
      }
      program.text_buf.pos++;
    }
    break;

  default:
    break;
  }
}

int program_process() {
  SDL_Event event;
  int should_quit = 0;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      should_quit = 1;
      break;
    case SDL_TEXTINPUT: {
      char *input = event.text.text;
      size_t len = strlen(input);
      buffer_insert(&program.text_buf, input, len);

      break;
    }
    case SDL_KEYDOWN:
      handle_key(event.key.keysym.scancode);
      break;
    case SDL_MOUSEWHEEL:
      program.text_buf.top_left.y -=
          event.wheel.y * (font_settings.height + font_settings.y_padding) *
          font_settings.scale * 2;
      break;
    case SDL_MOUSEBUTTONDOWN:
      clicked.changed = 1;
      clicked.pos.x = event.button.x + program.text_buf.top_left.x;
      clicked.pos.y = event.button.y + program.text_buf.top_left.y;
      break;
    default:
      break;
    }
  }

  return should_quit;
}
