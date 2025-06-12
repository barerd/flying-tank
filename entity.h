#ifndef ENTITY_H
#define ENTITY_H

#include <SDL.h>
#include <stdbool.h>

typedef struct {
    float x, y;           // Position
    float angle;          // Rotation angle in degrees
    float speed;          // Current speed
    float max_speed;      // Max speed
    float accel;          // Acceleration
    float friction;       // Friction slowdown
    int width;
    int height;
    SDL_Texture* texture; // Image
} Entity;

typedef struct {
    float offset_x;
    float offset_y;
} MountPoint;

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

// MountPoint
void mount_to_world_coords(Entity* parent, MountPoint* mount, float* out_x, float* out_y);

#endif
