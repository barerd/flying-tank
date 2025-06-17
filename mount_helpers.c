#include "mount_helpers.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

bool register_mount_and_attach(Entity* parent, const char* mount_name, int slot_index,
                               float offset_x, float offset_y, float offset_angle,
                               bool rotate_with_parent, float default_rotation,
                               Entity* child) {
    if (!parent || !child) return false;

    MountOffset* offsets = mount_create_offset_table(1);
    mount_set_offset(offsets, 0, offset_angle, offset_x, offset_y);

    int assigned_slot = mount_add_point(parent, mount_name, offsets, 1, rotate_with_parent, default_rotation, slot_index);
    if (assigned_slot == -1) {
        SDL_Log("Failed to add mount point: %s", mount_name);
        return false;
    }

    float world_x, world_y, world_angle;
    mount_get_world_position(parent, mount_name, &world_x, &world_y, &world_angle);
    entity_set_position(child, world_x, world_y);
    child->angle = world_angle;

    bool ok = mount_attach(parent, mount_name, child);
    if (!ok) {
        SDL_Log("ERROR: mount_attach failed for %s", mount_name);
    }

    return ok;
}

/* // Added by Claude AI */
/* void register_mount_and_attach( */
/*     Entity* parent, */
/*     const char* mount_name, */
/*     int slot_index, */
/*     float offset_x, */
/*     float offset_y, */
/*     float offset_angle, */
/*     bool rotate_with_parent, */
/*     float default_rotation, */
/*     Entity* child */
/* ) { */
/*     // Create offset table */
/*     MountOffset* offsets = mount_create_offset_table(1); */
/*     mount_set_offset(offsets, slot_index, offset_angle, offset_x, offset_y); */
    
/*     // Add entity mount point */
/*     mount_add_point(parent, mount_name, offsets, 1, rotate_with_parent, default_rotation); */
    
/*     // Position child at mount point */
/*     float world_x, world_y, world_angle; */
/*     mount_get_world_position(parent, mount_name, &world_x, &world_y, &world_angle); */
/*     entity_set_position(child, world_x, world_y); */
/*     child->angle = world_angle; */
    
/*     // Attach child */
/*     bool ok = mount_attach(parent, mount_name, child); */
/*     if (!ok) { */
/*         SDL_Log("ERROR: mount_point_attach failed for %s", mount_name); */
/*     } */
/* } */

Entity* get_mounted_entity(Entity* parent, const char* mount_name) {
    for (int i = 0; i < parent->entity_mount_count; i++) {
        if (parent->mount_points[i].name && strcmp(parent->mount_points[i].name, mount_name) == 0) {
            return parent->mounted_entities[i];
        }
    }
    return NULL;
}

void detach_mounted_entity(Entity* parent, const char* mount_name) {
    mount_detach(parent, mount_name);
}
