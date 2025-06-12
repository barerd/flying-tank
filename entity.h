#ifndef ENTITY_H
#define ENTITY_H

#include <SDL.h>
#include <stdbool.h>
#include "mount_system.h"

typedef struct Entity {
    float x, y;
    float angle;
    float speed, max_speed, accel, friction;      
    int width, height;
    SDL_Texture* texture;

    // Mount system
    MountPoint* mount_points;   // Array of mount points
    int mount_count;            // Number of mount points
    MountedComponent** mounted_components;  // Array of component lists (one per mount)
} Entity;

// Lifecycle
int  entity_load_texture(SDL_Renderer* renderer, Entity* e, const char* filepath);
void entity_unload(Entity* e);

// Logic
void entity_update(Entity* e);
void entity_turn(Entity* e, float angle_delta);
void entity_thrust(Entity* e, float amount);
bool entity_check_collision(Entity* a, Entity* b, int w_a, int h_a, int w_b, int h_b);

// Render
void entity_render(SDL_Renderer* renderer, const Entity* e, int width, int height);

#endif
