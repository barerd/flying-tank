#ifndef BULLET_H
#define BULLET_H

#include "entity.h"
#include <SDL.h>

#define MAX_BULLETS 50

typedef struct {
    Entity* entity;
    float lifetime;
    bool active;
} Bullet;

extern Bullet bullets[MAX_BULLETS];
extern int bullet_count;

// Initialize bullet system
void bullet_system_init();

// Spawn a bullet at given position and angle
Bullet* spawn_bullet(SDL_Renderer* renderer, float x, float y, float angle, float speed);

// Update all bullets
void update_all_bullets(float dt);

// Render all bullets
void render_all_bullets(SDL_Renderer* renderer);

// Cleanup bullet system
void cleanup_bullet_system();

#endif
