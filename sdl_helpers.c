#include "sdl_helpers.h"
#include <SDL_image.h>
#include <SDL.h>
#include "entity.h"

bool init_sdl(SDL_Window** window, SDL_Renderer** renderer, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        SDL_Log("IMG_Init Error: %s", IMG_GetError());
        SDL_Quit();
        return false;
    }

    *window = SDL_CreateWindow("Flying Tank Prototype",
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               width, height, 0);
    if (!*window) {
        SDL_Log("CreateWindow Error: %s", SDL_GetError());
        IMG_Quit(); SDL_Quit(); return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1,
                    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!*renderer) {
        SDL_Log("CreateRenderer Error: %s", SDL_GetError());
        SDL_DestroyWindow(*window);
        IMG_Quit(); SDL_Quit(); return false;
    }

    return true;
}

void shutdown_game(SDL_Window* window, SDL_Renderer* renderer, Entity** entities, int entity_count) {
    for (int i = 0; i < entity_count; i++) {
        if (entities[i]) {
            entity_unload(entities[i]);
            entity_destroy(entities[i]);
        }
    }

    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();
}
