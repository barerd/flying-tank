#ifndef ENTITY_H
#define ENTITY_H

#include <SDL.h>
#include <stdbool.h>
#include "mount_system.h"

typedef struct Entity {
    char* id;
    float x, y;
    float angle;
    float vx, vy;
    float speed, max_speed, accel, friction;      
    int width, height;
    bool active;
    SDL_Texture* texture;
    SDL_Texture** frames;
    int frame_count;            // Number of animation frames
    int current_frame;          // Current animation frame
    float frame_timer;          // Milliseconds elapsed in current frame
    float frame_delay_ms;       // Delay per frame in ms
    bool is_animated;

    // Optional per-entity logic (e.g., AI)
    void (*update)(struct Entity*, float dt);

    // Added by Claude AI
    MountPoint* mount_points;
    Entity** mounted_entities;
    int entity_mount_count;
} Entity;

// Lifecycle
extern Entity entities[];
Entity* entity_create(float x, float y, int width, int height);
Entity* spawn_entity(const char* id, SDL_Renderer* renderer, const char* texture_path, float x, float y);
Entity* find_entity(const char* name);
void    entity_destroy(Entity* e);
int     entity_load_texture(SDL_Renderer* renderer, Entity* e, const char* filepath);
void    entity_unload(Entity* e);

// Logic
void entity_update(Entity* e, const Uint8* keystate, float dt);
void entity_turn(Entity* e, float angle_delta);
void entity_thrust(Entity* e, float amount);
bool entity_check_collision(Entity* a, Entity* b, int w_a, int h_a, int w_b, int h_b);

// Render
void entity_render(SDL_Renderer* renderer, const Entity* e, int width, int height);

// Mounting utilities
void mount_to_world_coords(Entity* parent, MountPoint* mount, float* out_x, float* out_y);
void entity_set_position(Entity* e, float x, float y);

#endif
