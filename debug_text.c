#include "debug_text.h"
#include "tank.h"
#include <stdio.h>

bool debug_text_init(TTF_Font** font, const char* font_path, int size) {
    if (TTF_Init() == -1) return false;
    *font = TTF_OpenFont(font_path, size);
    return *font != NULL;
}

void debug_draw_mount_info(SDL_Renderer* renderer, TTF_Font* font, Tank* tank) {
    float fx, fy;
    mount_to_world_coords(&tank->base, &tank->exhaust_mount, &fx, &fy);

    char buffer[256];
    snprintf(buffer, sizeof(buffer),
        "Tank x=%.1f y=%.1f angle=%.1f\nMount ox=%.1f oy=%.1f\nFlame fx=%.1f fy=%.1f",
        tank->base.x, tank->base.y, tank->base.angle,
        tank->exhaust_mount.offset_x, tank->exhaust_mount.offset_y,
        fx, fy
    );

    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(font, buffer, white, 400);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect dst = {10, 10, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dst);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}
