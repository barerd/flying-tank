#ifndef MOUNT_HELPERS_H
#define MOUNT_HELPERS_H

#include "entity.h"

void mount_system_init(Entity* parent, int max_mounts);

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
);

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
);

void attach_mounted_entity(Entity* parent, const char* mount_name, Entity* child);
void detach_mounted_entity(Entity* parent, const char* mount_name);
Entity* get_mounted_entity(Entity* parent, const char* mount_name);

void register_and_attach_mount(
    Entity* parent,
    const char* mount_name,
    int slot_index,
    float offset_x,
    float offset_y,
    float offset_angle,
    bool relative_rotation,
    float default_rotation,
    Entity* child
);

void mount_update_all(Entity* parent, float dt);

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
);

#endif
