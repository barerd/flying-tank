#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdbool.h>

#include "entity.h"
#include "tank.h"
#include "car.h"

#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 750

int main() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("IMG_Init Error: %s\n", IMG_GetError());
        SDL_Quit(); return 1;
    }

    SDL_Window* win = SDL_CreateWindow("Flying Tank", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    
    if (!win) {
        printf("CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit(); return 1;
    }

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) {
        SDL_DestroyWindow(win);
        SDL_Quit(); return 1;
    }

    Tank tank;
    if (!tank_load(ren, &tank)) {
        printf("Failed to load tank.\n");
        SDL_DestroyRenderer(ren); SDL_DestroyWindow(win);
        SDL_Quit(); return 1;
    }

    Entity car;
    if (!car_load(&car, ren)) {
        printf("Failed to load car.\n");
        tank_unload(&tank);
        SDL_DestroyRenderer(ren); SDL_DestroyWindow(win); SDL_Quit(); return 1;
    }

    bool running = true;
    while (running) {
        SDL_Event e;
	while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
                running = false;
        }

	if (entity_check_collision(&tank.base, &car, 208, 208, 25, 50)) {
            printf("Tank collided with Car!\n");

            // Game logic decides the effect
            tank.base.speed = 0;
            car.speed = 0;

            // Or call an object-specific effect
            // tank_on_collision(&tank, &car);
            // car_on_collision(&car, &tank);
        }
	
	const Uint8* keystate = SDL_GetKeyboardState(NULL);
        tank_update(&tank, keystate);  // Controlled by player
        car_update(&car, keystate);    // Same controls for now

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        tank_render(ren, &tank, keystate);
	car_render(ren, &car);

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    tank_unload(&tank);
    car_unload(&car);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
