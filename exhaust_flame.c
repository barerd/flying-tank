#include "exhaust_flame.h"
#include <SDL_image.h>
#include <stdio.h>
#include <string.h>

bool flame_load(SDL_Renderer* ren, Exhaust_Flame* flame, const char* basepath, int frame_count) {
    if (frame_count > MAX_FLAME_FRAMES) {
        fprintf(stderr, "Too many frames: %d (max %d)\n", frame_count, MAX_FLAME_FRAMES);
        return false;
    }

    flame->frame_count = frame_count;
    flame->frame_index = 0;
    flame->frame_delay = 100;
    flame->last_time = SDL_GetTicks();

    for (int i = 0; i < frame_count; ++i) {
        char filename[128];
        snprintf(filename, sizeof(filename), "%s%d.png", basepath, i);
        SDL_Surface* surf = IMG_Load(filename);
        if (!surf) {
            fprintf(stderr, "Failed to load %s: %s\n", filename, IMG_GetError());
            return false;
        }
        flame->frames[i] = SDL_CreateTextureFromSurface(ren, surf);
        SDL_FreeSurface(surf);
        if (!flame->frames[i]) {
            fprintf(stderr, "Failed to create texture from %s: %s\n", filename, SDL_GetError());
            return false;
        }
    }
    return true;
}

void flame_update(Exhaust_Flame* flame) {
    Uint32 now = SDL_GetTicks();
    if (now - flame->last_time > flame->frame_delay) {
        flame->frame_index = (flame->frame_index + 1) % flame->frame_count;
        flame->last_time = now;
    }
}

void flame_render(SDL_Renderer* ren, Exhaust_Flame* flame, float x, float y, float angle) {
    if (!flame || flame->frame_count == 0 || !flame->frames[flame->frame_index]) {
        SDL_Log("⚠️ flame_render skipped: not initialized or invalid frame.");
        return;
    }

    SDL_Texture* current = flame->frames[flame->frame_index];
    SDL_Rect dst = { (int)x, (int)y, 32, 32 };
    SDL_RenderCopyEx(ren, current, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
}

void flame_unload(Exhaust_Flame* flame) {
    for (int i = 0; i < flame->frame_count; ++i) {
        if (flame->frames[i]) {
            SDL_DestroyTexture(flame->frames[i]);
            flame->frames[i] = NULL;
        }
    }
}
