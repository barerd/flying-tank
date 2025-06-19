#ifndef ENTITY_SPAWN_ANIMATED_H
#define ENTITY_SPAWN_ANIMATED_H

#include "entity.h"
#include <SDL.h>

AnimatedEntity* spawn_animated_entity(const char* id, SDL_Renderer* renderer, const char* base_path, int frame_count, float x, float y);

#endif
