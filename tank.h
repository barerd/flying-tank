#ifndef TANK_H
#define TANK_H

#include "entity.h"
#include "mount_system.h"
#include <SDL.h>
#include <stdbool.h>

typedef struct {
    Entity base;
    MountedComponent* exhaust_flame;
    bool thrusting;
} Tank;

bool tank_load(SDL_Renderer* renderer, Tank* tank);
void tank_update(Tank* tank, const Uint8* keystate, float dt);
void tank_render(SDL_Renderer* renderer, Tank* tank);
void tank_unload(Tank* tank);

#endif
