#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <math.h>
#include <SDL.h>
#include "entity.h"
#include "collision.h"

#define MAX_COLLIDERS 128
static ColliderComponent collider_registry[MAX_COLLIDERS];
static int collider_count = 0;

void attach_polygon_collider(Entity* e, SDL_Point* points, int point_count) {
    if (collider_count >= MAX_COLLIDERS) return;

    ColliderComponent* c = &collider_registry[collider_count++];
    c->entity = e;
    c->type = COLLIDER_POLYGON;
    c->polygon.points = points;
    c->polygon.point_count = point_count;
    c->on_collision = NULL;  // Optional: you can assign later
}

ColliderComponent* get_collider(Entity* e) {
    for (int i = 0; i < collider_count; i++) {
        if (collider_registry[i].entity == e)
            return &collider_registry[i];
    }
    return NULL;
}

void load_entity_hitbox(Entity* e, const char* image_filename) {
    // Derive JSON file path
    char json_filename[256];
    strncpy(json_filename, image_filename, sizeof(json_filename));
    char* ext = strrchr(json_filename, '.');
    if (ext) strcpy(ext, ".json");
    else strcat(json_filename, ".json");

    FILE* f = fopen(json_filename, "rb");
    if (!f) return;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* data = malloc(len + 1);
    fread(data, 1, len, f);
    data[len] = '\0';
    fclose(f);

    cJSON* root = cJSON_Parse(data);
    free(data);
    if (!root) return;

    cJSON* shapes = cJSON_GetObjectItem(root, "shapes");
    int shape_count = cJSON_GetArraySize(shapes);

    for (int i = 0; i < shape_count; i++) {
        cJSON* shape = cJSON_GetArrayItem(shapes, i);
        const char* label = cJSON_GetObjectItem(shape, "label")->valuestring;

        if (strcmp(label, e->id) != 0) continue;

        const char* shape_type = cJSON_GetObjectItem(shape, "shape_type")->valuestring;
        if (strcmp(shape_type, "polygon") != 0) continue;

	cJSON* points = cJSON_GetObjectItem(shape, "points");
        int count = cJSON_GetArraySize(points);
        if (count < 3) continue;

        SDL_Point* poly = malloc(sizeof(SDL_Point) * count);
        for (int j = 0; j < count; j++) {
            cJSON* pt = cJSON_GetArrayItem(points, j);
            poly[j].x = (int)cJSON_GetArrayItem(pt, 0)->valuedouble;
            poly[j].y = (int)cJSON_GetArrayItem(pt, 1)->valuedouble;
        }

        attach_polygon_collider(e, poly, count);
        break;  // Only first matching shape is attached
    }

    cJSON_Delete(root);
}

SDL_Point rotate_and_translate(SDL_Point p, float angle, float cx, float cy) {
    float s = sinf(angle);
    float c = cosf(angle);

    // translate point back to origin
    float x = p.x;
    float y = p.y;

    // rotate point
    float xnew = x * c - y * s;
    float ynew = x * s + y * c;

    // translate point back
    SDL_Point result = {
        (int)(xnew + cx),
        (int)(ynew + cy)
    };
    return result;
}

bool polygons_intersect(SDL_Point* poly1, int count1, SDL_Point* poly2, int count2) {
    // Check all edges of poly1
    for (int i = 0; i < count1; i++) {
        int j = (i + 1) % count1;
        
        // Get edge vector
        float edge_x = poly1[j].x - poly1[i].x;
        float edge_y = poly1[j].y - poly1[i].y;
        
        // Get perpendicular (normal) vector
        float normal_x = -edge_y;
        float normal_y = edge_x;
        
        // Normalize
        float length = sqrtf(normal_x * normal_x + normal_y * normal_y);
        if (length > 0) {
            normal_x /= length;
            normal_y /= length;
        }
        
        // Project both polygons onto this axis
        float min1 = FLT_MAX, max1 = -FLT_MAX;
        float min2 = FLT_MAX, max2 = -FLT_MAX;
        
        for (int k = 0; k < count1; k++) {
            float dot = poly1[k].x * normal_x + poly1[k].y * normal_y;
            if (dot < min1) min1 = dot;
            if (dot > max1) max1 = dot;
        }
        
        for (int k = 0; k < count2; k++) {
            float dot = poly2[k].x * normal_x + poly2[k].y * normal_y;
            if (dot < min2) min2 = dot;
            if (dot > max2) max2 = dot;
        }
        
        // Check for separation
        if (max1 < min2 || max2 < min1) {
            return false; // Separating axis found
        }
    }
    
    // Check all edges of poly2
    for (int i = 0; i < count2; i++) {
        int j = (i + 1) % count2;
        
        float edge_x = poly2[j].x - poly2[i].x;
        float edge_y = poly2[j].y - poly2[i].y;
        
        float normal_x = -edge_y;
        float normal_y = edge_x;
        
        float length = sqrtf(normal_x * normal_x + normal_y * normal_y);
        if (length > 0) {
            normal_x /= length;
            normal_y /= length;
        }
        
        float min1 = FLT_MAX, max1 = -FLT_MAX;
        float min2 = FLT_MAX, max2 = -FLT_MAX;
        
        for (int k = 0; k < count1; k++) {
            float dot = poly1[k].x * normal_x + poly1[k].y * normal_y;
            if (dot < min1) min1 = dot;
            if (dot > max1) max1 = dot;
        }
        
        for (int k = 0; k < count2; k++) {
            float dot = poly2[k].x * normal_x + poly2[k].y * normal_y;
            if (dot < min2) min2 = dot;
            if (dot > max2) max2 = dot;
        }
        
        if (max1 < min2 || max2 < min1) {
            return false;
        }
    }
    
    return true; // No separating axis found, polygons intersect
}

// Debug function to print entity and polygon info
void debug_collision_info(Entity* entity) {
    ColliderComponent* c = get_collider(entity);
    if (!c) {
        printf("No collider for entity %s\n", entity->id);
        return;
    }
    
    printf("Entity %s: pos(%.1f, %.1f) angle(%.1f)\n", 
           entity->id, entity->x, entity->y, entity->angle);
    printf("  Polygon points (%d): ", c->polygon.point_count);
    for (int i = 0; i < c->polygon.point_count; i++) {
        printf("(%.1f,%.1f) ", (float)c->polygon.points[i].x, (float)c->polygon.points[i].y);
    }
    printf("\n");
}

// Transform polygon points to world coordinates
void transform_polygon(SDL_Point* src, SDL_Point* dest, int count, Entity* entity) {
    float angle_rad = entity->angle * M_PI / 180.0f;
    float cos_a = cosf(angle_rad);
    float sin_a = sinf(angle_rad);
    float half_width, half_height;
    
    half_width = entity->width / 2.0f;
    half_height = entity->height / 2.0f;
    
    for (int i = 0; i < count; i++) {
        // Convert polygon coordinates from image top-left to entity center
        float local_x = (float)src[i].x - half_width;
        float local_y = (float)src[i].y - half_height;
        
        // Rotate the point around the entity center
        float rotated_x = local_x * cos_a - local_y * sin_a;
        float rotated_y = local_x * sin_a + local_y * cos_a;
        
        // Translate to world position (entity->x, entity->y is the center)
        dest[i].x = (int)(rotated_x + entity->x);
        dest[i].y = (int)(rotated_y + entity->y);
    }
}

// Check collision between two entities using their polygon colliders
bool check_entities_collision(Entity* e1, Entity* e2) {
    ColliderComponent* c1 = get_collider(e1);
    ColliderComponent* c2 = get_collider(e2);
    
    if (!c1 || !c2) return false;
    if (c1->type != COLLIDER_POLYGON || c2->type != COLLIDER_POLYGON) return false;
    
    // Transform polygons to world coordinates
    SDL_Point* poly1 = malloc(sizeof(SDL_Point) * c1->polygon.point_count);
    SDL_Point* poly2 = malloc(sizeof(SDL_Point) * c2->polygon.point_count);
    
    transform_polygon(c1->polygon.points, poly1, c1->polygon.point_count, e1);
    transform_polygon(c2->polygon.points, poly2, c2->polygon.point_count, e2);
    
    bool collision = polygons_intersect(poly1, c1->polygon.point_count, 
                                       poly2, c2->polygon.point_count);
    
    free(poly1);
    free(poly2);
    
    return collision;
}

// Handle all collisions in the system
void handle_all_collisions(float dt) {
    (void)dt; // Suppress unused parameter warning
    
    for (int i = 0; i < collider_count; i++) {
        for (int j = i + 1; j < collider_count; j++) {
            ColliderComponent* c1 = &collider_registry[i];
            ColliderComponent* c2 = &collider_registry[j];
            
            if (check_entities_collision(c1->entity, c2->entity)) {
                // Call collision callbacks if they exist
                if (c1->on_collision) {
                    c1->on_collision(c1->entity, c2->entity);
                }
                if (c2->on_collision) {
                    c2->on_collision(c2->entity, c1->entity);
                }
            }
        }
    }
}

// Draw collision polygons for debugging
void draw_all_collision_polygons(SDL_Renderer* renderer, Entity** entities, int count) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128); // Red color
    
    for (int i = 0; i < count; i++) {
        ColliderComponent* c = get_collider(entities[i]);
        if (!c || c->type != COLLIDER_POLYGON) continue;
        
        // Transform to world coordinates
        SDL_Point* world_poly = malloc(sizeof(SDL_Point) * c->polygon.point_count);
        transform_polygon(c->polygon.points, world_poly, c->polygon.point_count, entities[i]);
        
        // Draw polygon edges
        for (int j = 0; j < c->polygon.point_count; j++) {
            int next = (j + 1) % c->polygon.point_count;
            SDL_RenderDrawLine(renderer, 
                             world_poly[j].x, world_poly[j].y,
                             world_poly[next].x, world_poly[next].y);
        }
        
        free(world_poly);
    }
}

