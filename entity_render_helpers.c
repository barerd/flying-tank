#include "entity.h"
#include <SDL.h>

void render_animated_entity(SDL_Renderer* renderer, Entity* e, float delta_ms) {
    if (!e || !e->active || !e->frames || e->frame_count == 0) return;

    e->frame_timer += delta_ms;
    if (e->frame_timer >= e->frame_delay_ms) {
        e->frame_timer = 0;
        e->current_frame = (e->current_frame + 1) % e->frame_count;
    }

    SDL_Rect dst = {
        (int)(e->x - e->width / 2),
        (int)(e->y - e->height / 2),
        e->width,
        e->height
    };

    SDL_RenderCopyEx(renderer, e->frames[e->current_frame], NULL, &dst, e->angle, NULL, SDL_FLIP_NONE);
}
