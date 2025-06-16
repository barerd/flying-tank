#include "mount_helpers.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void register_mount_and_attach(
    Entity* parent,
    const char* mount_name,
    int slot_index,
    float offset_x,
    float offset_y,
    float offset_angle,
    bool rotate_with_parent,
    float default_rotation,
    Entity* child
) {
    register_mount(parent, mount_name, slot_index, offset_x, offset_y, offset_angle, rotate_with_parent, default_rotation, child);
    bool ok = mount_attach_entity(parent, mount_name, child);
    if (!ok) {
        SDL_Log("ERROR: mount_attach_entity failed for %s", mount_name);
    }
}

void register_mount(
    Entity* parent,
    const char* mount_name,
    int slot_index,
    float offset_x,
    float offset_y,
    float offset_angle,
    bool rotate_with_parent,
    float default_rotation,
    Entity* child
) {
    MountOffset* offsets = mount_create_offset_table(1);
    mount_set_offset(offsets, 0, offset_angle, offset_x, offset_y);
    mount_add_point(parent, mount_name, offsets, slot_index, rotate_with_parent, default_rotation);
    float world_x, world_y, world_angle;
    mount_get_world_position(parent, mount_name, &world_x, &world_y, &world_angle);
    entity_set_position(child, world_x, world_y);
    child->angle = world_angle;
}

void attach_mounted_entity(Entity* parent, const char* mount_name, Entity* child) {
    float x, y, angle;
    mount_get_world_position(parent, mount_name, &x, &y, &angle);
    entity_set_position(child, x, y);
    mount_attach_entity(parent, mount_name, child);
}

void register_and_attach_mount(Entity* parent, const char* mount_name, int frame_index,
                               float x, float y, float angle_offset, bool sync_rotation,
                               float default_angle, Entity* child) {
    register_mount(parent, mount_name, frame_index, x, y, angle_offset, sync_rotation, default_angle, child);
    attach_mounted_entity(parent, mount_name, child);
}

Entity* get_mounted_entity(Entity* parent, const char* mount_name) {
    for (int i = 0; i < parent->mount_count; ++i) {
        if (strcmp(parent->mount_points[i].name, mount_name) == 0) {
            return parent->mounted_components[i] ? parent->mounted_components[i]->entity : NULL;
        }
    }
    return NULL;
}

void detach_mounted_entity(Entity* parent, const char* mount_name) {
    mount_detach_entity(parent, mount_name);
}

// Added by Claude AI
void register_entity_mount_and_attach(
    Entity* parent,
    const char* mount_name,
    int slot_index,
    float offset_x,
    float offset_y,
    float offset_angle,
    bool rotate_with_parent,
    float default_rotation,
    Entity* child
) {
    // Create offset table
    MountOffset* offsets = mount_create_offset_table(1);
    mount_set_offset(offsets, slot_index, offset_angle, offset_x, offset_y);  // Use slot_index here
    
    // Add entity mount point
    entity_mount_add_point(parent, mount_name, offsets, 1, rotate_with_parent, default_rotation);
    
    // Position child at mount point
    float world_x, world_y, world_angle;
    entity_mount_get_world_position(parent, mount_name, &world_x, &world_y, &world_angle);
    entity_set_position(child, world_x, world_y);
    child->angle = world_angle;
    
    // Attach child
    bool ok = entity_mount_attach(parent, mount_name, child);
    if (!ok) {
        SDL_Log("ERROR: entity_mount_attach failed for %s", mount_name);
    }
}

Entity* get_entity_mounted(Entity* parent, const char* mount_name) {
    for (int i = 0; i < parent->entity_mount_count; i++) {
        if (parent->entity_mounts[i].name && strcmp(parent->entity_mounts[i].name, mount_name) == 0) {
            return parent->mounted_entities[i];
        }
    }
    return NULL;
}

void detach_entity_mounted(Entity* parent, const char* mount_name) {
    entity_mount_detach(parent, mount_name);
}
