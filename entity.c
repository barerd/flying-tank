#include "entity.h"
#include "mount_system.h"
#include <math.h>
#include <SDL_image.h>

int entity_load_texture(SDL_Renderer* renderer, Entity* e, const char* filepath) {
    SDL_Surface* surf = IMG_Load(filepath);
    if (!surf) return 0;
    e->texture = SDL_CreateTextureFromSurface(renderer, surf);
    if (!e->texture) {
        SDL_FreeSurface(surf);
        return 0;
    }
    e->width = surf->w;
    e->height = surf->h;
    
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
    // Convert center position to top-left for SDL rendering
    int render_x = (int)(e->x - width / 2.0f);
    int render_y = (int)(e->y - height / 2.0f);
    
    SDL_Rect dst = { render_x, render_y, width, height };
    
    // For center-based rotation, we don't need to specify a center point
    // SDL will rotate around the center of the destination rectangle
    SDL_RenderCopyEx(renderer, e->texture, NULL, &dst, e->angle, NULL, SDL_FLIP_NONE);
}

bool entity_check_collision(Entity* a, Entity* b, int w_a, int h_a, int w_b, int h_b) {
    // Convert center positions to top-left for collision detection
    int a_x = (int)(a->x - w_a / 2.0f);
    int a_y = (int)(a->y - h_a / 2.0f);
    int b_x = (int)(b->x - w_b / 2.0f);
    int b_y = (int)(b->y - h_b / 2.0f);
    
    SDL_Rect rect_a = { a_x, a_y, w_a, h_a };
    SDL_Rect rect_b = { b_x, b_y, w_b, h_b };
    return SDL_HasIntersection(&rect_a, &rect_b);
}

void mount_to_world_coords(Entity* parent, MountPoint* mount, float* out_x, float* out_y) {
    // Interpolate the offset from mount’s offset table
    float offset_x, offset_y;
    interpolate_mount_offset(mount, parent->angle, &offset_x, &offset_y);

    // Adjust angle to match sprite orientation - if your sprite points right at 0°, add 90°
    float angle_rad = (parent->angle + 90.0f) * (M_PI / 180.0f);

    float center_x = parent->x;
    float center_y = parent->y;

    float forward = -offset_x;  // forward = -X (assuming sprite points up)
    float right   =  offset_y;  // right = +Y

    float rotated_dx = cosf(angle_rad) * forward - sinf(angle_rad) * right;
    float rotated_dy = sinf(angle_rad) * forward + cosf(angle_rad) * right;

    *out_x = center_x + rotated_dx;
    *out_y = center_y + rotated_dy;
}
