#ifndef HITBOX_LOADER_H
#define HITBOX_LOADER_H

#include "entity.h"

// Loads and attaches all hitboxes to matching entities
void load_all_hitboxes(const char* hitbox_root, Entity** entities, int entity_count);

#endif
