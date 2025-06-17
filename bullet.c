#include "bullet.h"
#include <math.h>
#include <string.h>

Bullet bullets[MAX_BULLETS];
int bullet_count = 0;

void bullet_system_init() {
    memset(bullets, 0, sizeof(bullets));
    bullet_count = 0;
}

Bullet* spawn_bullet(SDL_Renderer* renderer, float x, float y, float angle, float speed) {
    // Find free bullet slot
    int free_slot = -1;
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            free_slot = i;
            break;
        }
    }
    
    if (free_slot == -1) {
        return NULL; // No free slots
    }
    
    // Create bullet entity
    Entity* bullet_entity = spawn_entity("bullet", renderer, "assets/bullet.png", x, y);
    if (!bullet_entity) {
        return NULL;
    }
    
    // Set bullet properties
    bullet_entity->angle = angle;
    bullet_entity->speed = speed;
    bullet_entity->max_speed = speed;
    bullet_entity->friction = 0; // No friction for bullets
    bullet_entity->active = true;
    
    // Initialize bullet
    bullets[free_slot].entity = bullet_entity;
    bullets[free_slot].lifetime = 3.0f; // 3 seconds lifetime
    bullets[free_slot].active = true;
    
    if (free_slot >= bullet_count) {
        bullet_count = free_slot + 1;
    }
    
    return &bullets[free_slot];
}

void update_all_bullets(float dt) {
    for (int i = 0; i < bullet_count; i++) {
        if (!bullets[i].active) continue;
        
        // Update lifetime
        bullets[i].lifetime -= dt;
        if (bullets[i].lifetime <= 0) {
            // Properly destroy the entity
            entity_destroy(bullets[i].entity);
            bullets[i].entity = NULL;
            bullets[i].active = false;
            continue;
        }
        
        // Update bullet physics
        entity_update(bullets[i].entity, NULL, dt);
        
        // Check if bullet is off-screen and destroy it
        if (bullets[i].entity->x < -50 || bullets[i].entity->x > 1050 ||
            bullets[i].entity->y < -50 || bullets[i].entity->y > 800) {
            // Properly destroy the entity
            entity_destroy(bullets[i].entity);
            bullets[i].entity = NULL;
            bullets[i].active = false;
        }
    }
}

void render_all_bullets(SDL_Renderer* renderer) {
    for (int i = 0; i < bullet_count; i++) {
        if (bullets[i].active && bullets[i].entity->active) {
            entity_render(renderer, bullets[i].entity, 
                         bullets[i].entity->width, bullets[i].entity->height);
        }
    }
}

void cleanup_bullet_system() {
    for (int i = 0; i < bullet_count; i++) {
        if (bullets[i].entity) {
            entity_destroy(bullets[i].entity);
            bullets[i].entity = NULL;
        }
        bullets[i].active = false;
    }
    bullet_count = 0;
}
