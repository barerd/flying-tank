#include "entity.h"
#include <math.h>
#include <SDL_image.h>

int entity_load_texture(SDL_Renderer* renderer, Entity* e, const char* filepath) {
    SDL_Surface* surf = IMG_Load(filepath);
    if (!surf) return 0;

    e->texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);

    return e->texture != NULL;
}

void entity_unload(Entity* e) {
    if (e->texture) SDL_DestroyTexture(e->texture);
    e->texture = NULL;
}

void entity_turn(Entity* e, float angle_delta) {
    e->angle += angle_delta;
}

void entity_thrust(Entity* e, float amount) {
    e->speed += amount;
    if (e->speed > e->max_speed) e->speed = e->max_speed;
    if (e->speed < -e->max_speed) e->speed = -e->max_speed;
}

void entity_update(Entity* e) {
    float angle_rad = (e->angle - 90.0f) * (float)(M_PI / 180.0f);
    e->x += cosf(angle_rad) * e->speed;
    e->y += sinf(angle_rad) * e->speed;

    if (e->speed > 0) {
        e->speed -= e->friction;
        if (e->speed < 0) e->speed = 0;
    } else if (e->speed < 0) {
        e->speed += e->friction;
        if (e->speed > 0) e->speed = 0;
    }
}

void entity_render(SDL_Renderer* renderer, const Entity* e, int width, int height) {
    SDL_Rect dst = { (int)e->x, (int)e->y, width, height };
    SDL_RenderCopyEx(renderer, e->texture, NULL, &dst, e->angle, NULL, SDL_FLIP_NONE);
}

bool entity_check_collision(Entity* a, Entity* b, int w_a, int h_a, int w_b, int h_b) {
    SDL_Rect rect_a = { (int)a->x, (int)a->y, w_a, h_a };
    SDL_Rect rect_b = { (int)b->x, (int)b->y, w_b, h_b };
    return SDL_HasIntersection(&rect_a, &rect_b);
}
