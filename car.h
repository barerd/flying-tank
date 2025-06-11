// car.h
#ifndef CAR_H
#define CAR_H

#include "entity.h"
#include <SDL.h>
#include <stdbool.h>

bool car_load(Entity* car, SDL_Renderer* renderer);
void car_update(Entity* car, const Uint8* keystate);
void car_render(SDL_Renderer* renderer, Entity* car);
void car_unload(Entity* car);

#endif
