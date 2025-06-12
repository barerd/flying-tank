#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "tank.h"

bool debug_text_init(TTF_Font** font, const char* font_path, int size);
void debug_text_render(SDL_Renderer* renderer, TTF_Font* font, int x, int y, const char* text);
void debug_draw_mount_info(SDL_Renderer* renderer, TTF_Font* font, Tank* tank);
