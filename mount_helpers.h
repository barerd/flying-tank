#ifndef MOUNT_HELPERS_H
#define MOUNT_HELPERS_H

#include "entity.h"

bool register_mount_and_attach(
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
