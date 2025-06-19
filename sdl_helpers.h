#ifndef SDL_HELPERS_H
#define SDL_HELPERS_H

#include <SDL.h>
#include <stdbool.h>
#include "entity.h"

bool init_sdl(SDL_Window** window, SDL_Renderer** renderer, int width, int height);
void shutdown_game(SDL_Window* window, SDL_Renderer* renderer, Entity** entities, int entity_count);

#define REGISTER_ENTITY(varname, spawn_call)             \
    Entity* varname = spawn_call;                         \
    if (!varname) {                                       \
        SDL_Log("Failed to spawn entity: " #varname);     \
        shutdown_game(window, renderer, entities, entity_count); \
        return 1;                                         \
    }                                                     \
    entities[entity_count++] = varname;

#define REGISTER_ANIMATED_ENTITY(varname, spawn_call)     \
    AnimatedEntity* varname = spawn_call;                          \
    if (!varname) {                                        \
        SDL_Log("Failed to spawn animated entity: " #varname); \
        shutdown_game(window, renderer, entities, entity_count); \
        return 1;                                          \
    }                                                      \
    entities[entity_count++] = (Entity*)varname;

#endif
