#ifndef COLLISION_H
#define COLLISION_H

#include <SDL.h>
#include <stdbool.h>
#include <float.h>
#include "entity.h"

// Collider type enum
typedef enum {
    COLLIDER_NONE,
    COLLIDER_POLYGON,
    COLLIDER_CIRCLE
} ColliderType;

// Collider component
typedef struct {
    Entity* entity;
    ColliderType type;
    struct {
        SDL_Point* points;
        int point_count;
    } polygon;
    void (*on_collision)(Entity* self, Entity* other);
} ColliderComponent;

// API
void attach_polygon_collider(Entity* e, SDL_Point* points, int point_count);
void handle_all_collisions(float dt);
ColliderComponent* get_collider(Entity* entity);
void draw_all_collision_polygons(SDL_Renderer* renderer, Entity** entities, int count);
SDL_Point rotate_and_translate(SDL_Point p, float angle_deg, float cx, float cy);

bool check_entities_collision(Entity* e1, Entity* e2);
bool polygons_intersect(SDL_Point* poly1, int count1, SDL_Point* poly2, int count2);
void transform_polygon(SDL_Point* src, SDL_Point* dest, int count, Entity* entity);
void load_entity_hitbox(Entity* e, const char* json_filename);
void debug_collision_info(Entity* entity);

#endif
