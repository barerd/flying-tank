#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mount_system.h"
#include "entity.h"
#include "entity_render_helpers.h"
#include <SDL_log.h>

MountOffset* mount_create_offset_table(int count) {
    return calloc(count, sizeof(MountOffset));
}

void mount_set_offset(MountOffset* offsets, int index, float angle, float offset_x, float offset_y) {
    offsets[index].angle = angle;
    offsets[index].offset_x = offset_x;
    offsets[index].offset_y = offset_y;
}

void mount_system_init(Entity* entity, int mount_count) {
    entity->mount_points = calloc(mount_count, sizeof(MountPoint));
    entity->mounted_entities = calloc(mount_count, sizeof(Entity*));  // <-- this line was missing
    entity->entity_mount_count = mount_count;
}

void mount_system_cleanup(Entity* entity) {    
    // Cleanup entity mounts (new)
    if (entity->mount_points) {
        for (int i = 0; i < entity->entity_mount_count; i++) {
            if (entity->mount_points[i].name) {
                free(entity->mount_points[i].name);
            }
            if (entity->mount_points[i].offsets) {
                free(entity->mount_points[i].offsets);
            }
        }
        free(entity->mount_points);
        free(entity->mounted_entities);
        entity->mount_points = NULL;
        entity->mounted_entities = NULL;
        entity->entity_mount_count = 0;
    }
}

int mount_add_point(Entity* entity, const char* name, MountOffset* offsets, int offset_count,
                    bool inherit_rotation, float rotation_offset, int slot_index) {
    if (!entity || !entity->mount_points || !entity->mounted_entities) return -1;

    // If -1, find a free slot
    if (slot_index == -1) {
        for (int i = 0; i < entity->entity_mount_count; i++) {
            if (!entity->mount_points[i].name) {
                slot_index = i;
                break;
            }
        }
    }

    if (slot_index < 0 || slot_index >= entity->entity_mount_count) {
        SDL_Log("FATAL: Invalid or no available mount slot!");
        return -1;
    }

    if (entity->mount_points[slot_index].name) {
        SDL_Log("FATAL: Slot %d already occupied!", slot_index);
        return -1;
    }

    entity->mount_points[slot_index].name = strdup(name);
    entity->mount_points[slot_index].offsets = offsets;
    entity->mount_points[slot_index].offset_count = offset_count;
    entity->mount_points[slot_index].inherit_rotation = inherit_rotation;
    entity->mount_points[slot_index].rotation_offset = rotation_offset;

    return slot_index;
}

void interpolate_mount_offset(MountPoint* mount, float angle, float* out_x, float* out_y) {
    if (mount->offset_count == 0) {
        *out_x = *out_y = 0;
        return;
    }
    
    // Normalize angle to 0-360
    while (angle < 0) angle += 360;
    while (angle >= 360) angle -= 360;
    
    // Find closest entries
    int lower = 0;
    float min_diff = 360;
    
    for (int i = 0; i < mount->offset_count; i++) {
        float diff = fabs(mount->offsets[i].angle - angle);
        if (diff < min_diff) {
            min_diff = diff;
            lower = i;
        }
    }
    
    // For now, just use closest match (could add interpolation later)
    *out_x = mount->offsets[lower].offset_x;
    *out_y = mount->offsets[lower].offset_y;
}

void mount_get_world_position(const Entity* entity, const char* mount_name, 
                                   float* out_x, float* out_y, float* out_angle) {
    // Find entity mount point
    MountPoint* mount = NULL;
    for (int i = 0; i < entity->entity_mount_count; i++) {
        if (entity->mount_points[i].name && strcmp(entity->mount_points[i].name, mount_name) == 0) {
            mount = &entity->mount_points[i];
            break;
        }
    }
    
    if (!mount) {
        *out_x = entity->x;
        *out_y = entity->y;
        *out_angle = entity->angle;
        return;
    }
    
    // Get offset for current angle (reuse existing interpolation logic)
    float offset_x, offset_y;
    if (mount->offset_count == 0) {
        offset_x = offset_y = 0;
    } else {
        // Normalize angle to 0-360
        float angle = entity->angle;
        while (angle < 0) angle += 360;
        while (angle >= 360) angle -= 360;
        
        // Find closest match
        int closest = 0;
        float min_diff = 360;
        for (int i = 0; i < mount->offset_count; i++) {
            float diff = fabs(mount->offsets[i].angle - angle);
            if (diff < min_diff) {
                min_diff = diff;
                closest = i;
            }
        }
        offset_x = mount->offsets[closest].offset_x;
        offset_y = mount->offsets[closest].offset_y;
    }
    
    // Transform to world coordinates
    float angle_rad = entity->angle * (M_PI / 180.0f);
    float cos_a = cosf(angle_rad);
    float sin_a = sinf(angle_rad);
    
    *out_x = entity->x + (offset_x * cos_a - offset_y * sin_a);
    *out_y = entity->y + (offset_x * sin_a + offset_y * cos_a);
    
    /* if (mount->inherit_rotation) { */
    /*     *out_angle = entity->angle + mount->rotation_offset; */
    /* } else { */
    /*     *out_angle = mount->rotation_offset; */
    /* } */
    if (mount->inherit_rotation) {
        // Use the dynamic angle from offsets[0], not the static rotation_offset
        float dynamic_angle = (mount->offset_count > 0) ? mount->offsets[0].angle : 0.0f;
        *out_angle = entity->angle + mount->rotation_offset + dynamic_angle;
    } else {
        float dynamic_angle = (mount->offset_count > 0) ? mount->offsets[0].angle : 0.0f;
        *out_angle = mount->rotation_offset + dynamic_angle;
    }
}

bool mount_attach(Entity* parent, const char* mount_name, Entity* child) {
    int mount_index = -1;
    for (int i = 0; i < parent->entity_mount_count; i++) {
        if (parent->mount_points[i].name && strcmp(parent->mount_points[i].name, mount_name) == 0) {
            mount_index = i;
            break;
        }
    }
    if (mount_index == -1) return false;
    
    // For now, only support one entity per mount point (can be extended later)
    if (parent->mounted_entities[mount_index] != NULL) {
        return false; // Already occupied
    }
    
    parent->mounted_entities[mount_index] = child;
    return true;
}

bool mount_detach(Entity* parent, const char* mount_name) {
    for (int i = 0; i < parent->entity_mount_count; i++) {
        if (parent->mount_points[i].name && strcmp(parent->mount_points[i].name, mount_name) == 0) {
            parent->mounted_entities[i] = NULL;
            return true;
        }
    }
    return false;
}

void mount_update_all(Entity* entity, float dt) {
    for (int i = 0; i < entity->entity_mount_count; i++) {
        Entity* mounted = entity->mounted_entities[i];
        if (mounted && mounted->active) {
            // Update mounted entity position
            float x, y, angle;
            mount_get_world_position(entity, entity->mount_points[i].name, &x, &y, &angle);
            mounted->x = x;
            mounted->y = y;
            mounted->angle = angle;
            
            // Update the entity itself
            entity_update(mounted, NULL, dt);
            
            // Recursively update its mounts
            mount_update_all(mounted, dt);
        }
    }
}

void mount_render_all(SDL_Renderer* renderer, const Entity* entity) {
    for (int i = 0; i < entity->entity_mount_count; i++) {
        Entity* mounted = entity->mounted_entities[i];
        if (mounted && mounted->active) {
            if (mounted->type == ENTITY_ANIMATED) {
                AnimatedEntity* anim_entity = (AnimatedEntity*)mounted;
                static Uint32 last_time = 0;
                Uint32 now = SDL_GetTicks();
                float delta = (last_time == 0) ? 16.0f : (now - last_time);
                last_time = now;
                render_animated_entity(renderer, anim_entity, delta);  // Pass AnimatedEntity directly
            } else {
                entity_render(renderer, mounted, mounted->width, mounted->height);
            }
            
            mount_render_all(renderer, mounted);
        }
    }
}

// Returns a pointer to the MountPoint with the given name, or NULL if not found
MountPoint* mount_get(Entity* parent, const char* name) {
    for (int i = 0; i < parent->entity_mount_count; i++) {
        if (strcmp(parent->mount_points[i].name, name) == 0) {
            return &parent->mount_points[i];
        }
    }
    return NULL;
}
