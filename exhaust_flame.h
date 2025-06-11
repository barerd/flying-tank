#ifndef EXHAUST_FLAME_H
#define EXHAUST_FLAME_H

#include <SDL.h>
#include <stdbool.h>

#define MAX_FLAME_FRAMES 8

typedef struct {
    SDL_Texture* frames[MAX_FLAME_FRAMES];
    int frame_count;
    int frame_index;
    Uint32 frame_delay;  // in milliseconds
    Uint32 last_time;
} Exhaust_Flame;

bool flame_load(SDL_Renderer* ren, Exhaust_Flame* flame, const char* basepath, int frame_count);
void flame_update(Exhaust_Flame* flame);
void flame_render(SDL_Renderer* ren, Exhaust_Flame* flame, float x, float y, float angle);
void flame_unload(Exhaust_Flame* flame);

#endif
