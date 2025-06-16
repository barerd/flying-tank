#ifndef ENTITY_RENDER_HELPERS_H
#define ENTITY_RENDER_HELPERS_H

#include "entity.h"
#include <SDL.h>

void render_animated_entity(SDL_Renderer* renderer, Entity* e, float delta_ms);

#endif
