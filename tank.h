#ifndef TANK_H
#define TANK_H

#include "entity.h"
#include <stdbool.h>

typedef struct {
    Entity base;
    MountPoint exhaust_mount;
  // More mounts can go here: e.g., turret_mount, missile_mount, etc.
} Tank;

bool tank_load(SDL_Renderer* renderer, Tank* tank);
void tank_update(Tank* tank, const Uint8* keystate);
void tank_render(SDL_Renderer* renderer, Tank* tank, const Uint8* keystate);
void tank_unload(Tank* tank);

#endif
