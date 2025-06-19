#include "entity.h"
#include "mount_system.h"
#include <math.h>
#include <SDL_image.h>

#define MAX_ENTITIES 128

Entity entities[MAX_ENTITIES];
static const char* entity_names[MAX_ENTITIES];
static int entity_count = 0;

Entity* find_entity(const char* name) {
    for (int i = 0; i < entity_count; i++) {
        if (strcmp(entity_names[i], name) == 0)
            return &entities[i];
    }
    return NULL;
}

Entity* spawn_entity(const char* id, SDL_Renderer* renderer, const char* texture_path, float x, float y) {
    if (entity_count >= MAX_ENTITIES) {
        SDL_Log("Entity limit reached.");
        return NULL;
    }

    SDL_Surface* surface = IMG_Load(texture_path);
    if (!surface) {
        SDL_Log("IMG_Load failed: %s", IMG_GetError());
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        return NULL;
    }

    Entity* e = &entities[entity_count];
    memset(e, 0, sizeof(Entity));

    e->texture = texture;
    e->x = x;
    e->y = y;
    e->angle = 0;
    e->accel = 100;
    e->friction = 60;
    e->max_speed = 300;
    e->active = true;
    e->id = strdup(id);

    SDL_QueryTexture(texture, NULL, NULL, &e->width, &e->height);

    entity_names[entity_count] = strdup(id);  // optional external tracking
    entity_count++;

    return e;
}

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

void entity_update(Entity* e, const Uint8* keystate, float dt) {
    (void)keystate;  // Currently unused

    if (!e) return;

    // Defensive: avoid updating unloaded or malformed entities
    if (e->update) {
        e->update(e, dt);
    }

    // Skip physics for destroyed or non-movable entities
    if (e->friction == 0 && e->max_speed == 0 && e->speed == 0) return;

    // Apply friction
    if (e->speed > 0) {
        e->speed -= e->friction * dt;
        if (e->speed < 0) e->speed = 0;
    } else if (e->speed < 0) {
        e->speed += e->friction * dt;
        if (e->speed > 0) e->speed = 0;
    }

    // Clamp to max speed limits
    if (e->speed > e->max_speed)
        e->speed = e->max_speed;
    if (e->speed < -e->max_speed)
        e->speed = -e->max_speed;

    // Integrate position using angle and speed
    float rad = e->angle * (float)(M_PI / 180.0f);
    e->x += cosf(rad) * e->speed * dt;
    e->y += sinf(rad) * e->speed * dt;
}

void entity_render(SDL_Renderer* renderer, const Entity* e, int width, int height) {
    if (!e->active) return;

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

bool entity_check_collision_simple(Entity* a, Entity* b) {
    return entity_check_collision(a, b, a->width, a->height, b->width, b->height);
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

Entity* entity_create(float x, float y, int width, int height) {
    Entity* e = calloc(1, sizeof(Entity));
    if (!e) return NULL;

    e->x = x;
    e->y = y;
    e->width = width;
    e->height = height;

    e->angle = 0.0f;
    e->speed = 0.0f;
    e->max_speed = 10.0f;
    e->accel = 1.0f;
    e->friction = 0.9f;
    e->active = true;

    e->texture = NULL;
    e->mount_points = NULL;
    e->mounted_entities = NULL;
    e->entity_mount_count = 0;

    e->update = NULL;  // Optional logic

    return e;
}

void entity_destroy(Entity* e) {
    if (!e) return;

    if (e >= entities && e < entities + MAX_ENTITIES) {
        entity_unload(e);
        return;
    }
    
    // Clean up entity mounting system
    if (e->mount_points) {
        for (int i = 0; i < e->entity_mount_count; i++) {
            if (e->mount_points[i].name) {
                free(e->mount_points[i].name);
            }
            if (e->mount_points[i].offsets) {
                free(e->mount_points[i].offsets);
            }
        }
        free(e->mount_points);
        free(e->mounted_entities);
        e->mount_points = NULL;
        e->mounted_entities = NULL;
        e->entity_mount_count = 0;
    }

    // Clean up other resources
    if (e->texture) {
        SDL_DestroyTexture(e->texture);
    }
    if (e->id) {
        free(e->id);
    }
    
    free(e);
}

void entity_set_position(Entity* e, float x, float y) {
    e->x = x;
    e->y = y;
}

