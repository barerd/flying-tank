#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mount_system.h"
#include "entity.h"

void mount_system_init(Entity* entity, int mount_count) {
    entity->mount_points = calloc(mount_count, sizeof(MountPoint));
    entity->mounted_components = calloc(mount_count, sizeof(MountedComponent*));
    entity->mount_count = mount_count;
}

void mount_system_cleanup(Entity* entity) {
    if (!entity->mount_points) return;
    
    // Clean up all mounted components
    for (int i = 0; i < entity->mount_count; i++) {
        MountedComponent* comp = entity->mounted_components[i];
        while (comp) {
            MountedComponent* next = comp->next;
            if (comp->destroy) comp->destroy(comp);
            free(comp);
            comp = next;
        }
        
        // Free offset tables
        if (entity->mount_points[i].offsets) {
            free(entity->mount_points[i].offsets);
        }
    }
    
    free(entity->mount_points);
    free(entity->mounted_components);
    entity->mount_points = NULL;
    entity->mounted_components = NULL;
    entity->mount_count = 0;
}

int mount_add_point(Entity* entity, const char* name, MountOffset* offsets, int offset_count,
                   bool inherit_rotation, float rotation_offset) {
    
    // Find empty slot
    for (int i = 0; i < entity->mount_count; i++) {
        if (!entity->mount_points[i].name) {
            entity->mount_points[i].name = strdup(name);
            entity->mount_points[i].offsets = offsets;
            entity->mount_points[i].offset_count = offset_count;
            entity->mount_points[i].inherit_rotation = inherit_rotation;
            entity->mount_points[i].rotation_offset = rotation_offset;
            return i;
        }
    }
    return -1; // No free slots
}

// Helper to interpolate between offset entries based on angle
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

void mount_get_world_position(Entity* entity, const char* mount_name, float* out_x, float* out_y, float* out_angle) {
    // Find mount point
    MountPoint* mount = NULL;
    for (int i = 0; i < entity->mount_count; i++) {
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
    
    // Get offset for current angle
    float offset_x, offset_y;
    interpolate_mount_offset(mount, entity->angle, &offset_x, &offset_y);
    
    // Transform to world coordinates
    float angle_rad = entity->angle * (M_PI / 180.0f);
    float cos_a = cosf(angle_rad);
    float sin_a = sinf(angle_rad);
    
    *out_x = entity->x + (offset_x * cos_a - offset_y * sin_a);
    *out_y = entity->y + (offset_x * sin_a + offset_y * cos_a);
    
    if (mount->inherit_rotation) {
        *out_angle = entity->angle + mount->rotation_offset;
    } else {
        *out_angle = mount->rotation_offset;
    }
}

// --- Animation component methods ---

void animation_component_update(MountedComponent* comp, float dt) {
    (void)dt;
    Uint32 now = SDL_GetTicks();
    if (now - comp->data.animation.last_update >= comp->data.animation.frame_delay) {
        comp->data.animation.current_frame = (comp->data.animation.current_frame + 1) % comp->data.animation.frame_count;
        comp->data.animation.last_update = now;
    }
}

void animation_component_render(SDL_Renderer* renderer, MountedComponent* comp, float x, float y, float angle) {
    if (!comp->active) return;

    SDL_Texture* frame = comp->data.animation.frames[comp->data.animation.current_frame];
    if (!frame) return;

    SDL_Rect dst = {
        (int)(x - comp->data.animation.width / 2),
        (int)(y - comp->data.animation.height / 2),
        comp->data.animation.width,
        comp->data.animation.height
    };

    SDL_RenderCopyEx(renderer, frame, NULL, &dst, angle + comp->local_angle, NULL, SDL_FLIP_NONE);
}

void animation_component_destroy(MountedComponent* comp) {
    free((void*)comp->type);
}


// --- Sprite component methods ---

void sprite_component_update(MountedComponent* comp, float dt) {
    // Static sprite, nothing to update
    (void)comp;
    (void)dt;
}

void sprite_component_render(SDL_Renderer* renderer, MountedComponent* comp, float x, float y, float angle) {
    if (!comp->active || !comp->data.sprite.texture) return;

    SDL_Rect dst = {
        (int)(x - comp->data.sprite.width / 2),
        (int)(y - comp->data.sprite.height / 2),
        comp->data.sprite.width,
        comp->data.sprite.height
    };

    SDL_RenderCopyEx(renderer, comp->data.sprite.texture, NULL, &dst, angle + comp->local_angle, NULL, SDL_FLIP_NONE);
}

void sprite_component_destroy(MountedComponent* comp) {
    free((void*)comp->type);
}

// --- Component creation ---

MountedComponent* mount_create_animation_component(const char* type, SDL_Texture** frames,
                                                   int frame_count, int width, int height, Uint32 frame_delay) {
    MountedComponent* comp = calloc(1, sizeof(MountedComponent));
    comp->type = strdup(type);
    comp->data.animation.frames = frames;
    comp->data.animation.frame_count = frame_count;
    comp->data.animation.current_frame = 0;
    comp->data.animation.frame_delay = frame_delay;
    comp->data.animation.last_update = SDL_GetTicks();
    comp->data.animation.width = width;
    comp->data.animation.height = height;
    comp->active = true;
    comp->local_angle = 0;

    comp->update = animation_component_update;
    comp->render = animation_component_render;
    comp->destroy = animation_component_destroy;

    return comp;
}

MountedComponent* mount_create_sprite_component(const char* type, SDL_Texture* texture,
                                                int width, int height) {
    MountedComponent* comp = calloc(1, sizeof(MountedComponent));
    comp->type = strdup(type);
    comp->data.sprite.texture = texture;
    comp->data.sprite.width = width;
    comp->data.sprite.height = height;
    comp->active = true;
    comp->local_angle = 0;

    comp->update = sprite_component_update;
    comp->render = sprite_component_render;
    comp->destroy = sprite_component_destroy;

    return comp;
}

bool mount_attach_component(Entity* entity, const char* mount_name, MountedComponent* component) {
    int mount_index = -1;
    for (int i = 0; i < entity->mount_count; i++) {
        if (entity->mount_points[i].name && strcmp(entity->mount_points[i].name, mount_name) == 0) {
            mount_index = i;
            break;
        }
    }

    if (mount_index == -1) return false;

    component->next = entity->mounted_components[mount_index];
    entity->mounted_components[mount_index] = component;
    return true;
}

void mount_update_all(Entity* entity, float dt) {
    for (int i = 0; i < entity->mount_count; i++) {
        MountedComponent* comp = entity->mounted_components[i];
        while (comp) {
            if (comp->update && comp->active)
                comp->update(comp, dt);
            comp = comp->next;
        }
    }
}

void mount_render_all(SDL_Renderer* renderer, Entity* entity) {
    for (int i = 0; i < entity->mount_count; i++) {
        if (!entity->mount_points[i].name) continue;

        float x, y, angle;
        mount_get_world_position(entity, entity->mount_points[i].name, &x, &y, &angle);

        MountedComponent* comp = entity->mounted_components[i];
        while (comp) {
            if (comp->render && comp->active)
                comp->render(renderer, comp, x, y, angle);
            comp = comp->next;
        }
    }
}

// --- Offset utilities ---

MountOffset* mount_create_offset_table(int count) {
    return calloc(count, sizeof(MountOffset));
}

void mount_set_offset(MountOffset* table, int index, float angle, float offset_x, float offset_y) {
    table[index].angle = angle;
    table[index].offset_x = offset_x;
    table[index].offset_y = offset_y;
}
