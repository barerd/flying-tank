// car.c
#include "car.h"
#include "entity.h"

bool car_load(Entity* car, SDL_Renderer* renderer) {
    SDL_Surface* surface = SDL_CreateRGBSurface(0, 25, 50, 32,
                                                0x00FF0000,
                                                0x0000FF00,
                                                0x000000FF,
                                                0xFF000000);
    if (!surface) return false;

    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 0, 0));  // Red

    car->texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!car->texture) return false;

    car->x = 100;
    car->y = 650;
    car->angle = 0;
    car->speed = 0;
    car->accel = 0.2f;
    car->max_speed = 3.0f;
    car->friction = 0.03f;

    return true;
}

void car_update(Entity* car, const Uint8* keystate) {
    if (keystate[SDL_SCANCODE_A])
        entity_turn(car, -5.0f);
    if (keystate[SDL_SCANCODE_D])
        entity_turn(car, +5.0f);
    if (keystate[SDL_SCANCODE_W])
        entity_thrust(car, +car->accel);
    if (keystate[SDL_SCANCODE_S])
        entity_thrust(car, -car->accel);

    entity_update(car);
}

void car_render(SDL_Renderer* renderer, Entity* car) {
    SDL_Rect dst = { (int)car->x, (int)car->y, 25, 50 };
    SDL_RenderCopyEx(renderer, car->texture, NULL, &dst, car->angle, NULL, SDL_FLIP_NONE);
}

void car_unload(Entity* car) {
    entity_unload(car);
}
