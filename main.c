#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "entity.h"
#include "mount_system.h"
#include "entity_spawn_animated.h"
#include "entity_render_helpers.h"
#include "behavior_helpers.h"
#include "sdl_helpers.h"
#include "mount_helpers.h"
#include "bullet.h"

#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 750
#define FIXED_DT (1.0f / 60.0f)
bool space_pressed = false;
float shoot_cooldown = 0.0f;
const float SHOOT_COOLDOWN_TIME = 0.2f; // 200ms between shots

int main() {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    if (!init_sdl(&window, &renderer, WINDOW_WIDTH, WINDOW_HEIGHT)) return 1;

    Entity* entities[4];
    int entity_count = 0;
    bullet_system_init();

    // ---- Tank Setup ----
    REGISTER_ENTITY(tank, spawn_entity("tank1", renderer, "assets/tank.png", 100, 100));
    
    // Initialize mount system for tank
    mount_system_init(tank, 4);
    
    // ---- Turret Setup ----
    REGISTER_ENTITY(turret, spawn_entity("turret1", renderer, "assets/turret.png", 0, 0));
    turret->is_animated = false;  // Make sure this flag exists
    register_mount_and_attach(tank, "main_weapon", -1, 40, 50, 0.0f, true, 0.0f, turret);

    // Exhaust Flame Component
    REGISTER_ANIMATED_ENTITY(flame, spawn_animated_entity("flame1", renderer, "assets/exhaust-flame", 4, 0, 0));
    register_mount_and_attach(tank, "exhaust_flame", -1, -90, 0, 0.0f, true, 0.0f, flame);

    // ---- Main Loop ----
    bool running = true;
    bool turret_mounted = true;
    bool turret_toggle_pressed = false;
    float turret_remount_cooldown = 0.5f;  // in seconds
    Uint32 last_time = SDL_GetTicks();

    while (running) {
        SDL_Event e;
	Uint32 now = SDL_GetTicks();
        float delta_ms = now - last_time;
        last_time = now;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT ||
               (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
                running = false;
            }
        }

        const Uint8* keystate = SDL_GetKeyboardState(NULL);
	apply_thrust_turn(tank, keystate, SDL_SCANCODE_UP, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, tank->accel, 180, FIXED_DT);

        bool moving = keystate[SDL_SCANCODE_UP] || 
                      keystate[SDL_SCANCODE_LEFT] || 
                      keystate[SDL_SCANCODE_RIGHT];

        flame->active = moving;

	toggle_mount_with_key(turret, tank, "main_weapon", SDL_SCANCODE_T, &turret_mounted, &turret_toggle_pressed, &turret_remount_cooldown, 0.5f, FIXED_DT);

	rotate_within_limits(turret, tank, keystate, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT,
                     -90, 90, 180.0f, FIXED_DT, "main_weapon");

	// bullet shoot
	shoot_cooldown -= FIXED_DT;
        if (shoot_cooldown < 0.0f) shoot_cooldown = 0.0f;

        if (keystate[SDL_SCANCODE_SPACE] && !space_pressed && shoot_cooldown <= 0.0f && turret_mounted) {
            space_pressed = true;
    
            // Get turret world position and angle
            float turret_x, turret_y, turret_angle;
            mount_get_world_position(tank, "main_weapon", &turret_x, &turret_y, &turret_angle);
    
            // Calculate bullet spawn position (slightly in front of turret)
            float spawn_distance = 30.0f; // Distance in front of turret
            float angle_rad = turret_angle * (M_PI / 180.0f);
            float bullet_x = turret_x + cosf(angle_rad) * spawn_distance;
            float bullet_y = turret_y + sinf(angle_rad) * spawn_distance;
    
            // Spawn bullet
            spawn_bullet(renderer, bullet_x, bullet_y, turret_angle, 400.0f);
    
            shoot_cooldown = SHOOT_COOLDOWN_TIME;
        }

        if (!keystate[SDL_SCANCODE_SPACE]) {
            space_pressed = false;
        }

        // ---- Physics ----
        if (turret_remount_cooldown > 0.0f) {
            turret_remount_cooldown -= FIXED_DT;
            if (turret_remount_cooldown < 0.0f)
                turret_remount_cooldown = 0.0f;
        }
	
	entity_update(tank, keystate, FIXED_DT);
	mount_update_all(tank, FIXED_DT);
	update_all_bullets(FIXED_DT);

        // ---- Rendering ----
        SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
        SDL_RenderClear(renderer);

	entity_render(renderer, tank, tank->width, tank->height);
	mount_render_all(renderer, tank);
	render_all_bullets(renderer);
	if (flame->active)
            render_animated_entity(renderer, flame, delta_ms);
	
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    // ---- Cleanup ----
    mount_system_cleanup(tank);
    cleanup_bullet_system();
    shutdown_game(window, renderer, entities, entity_count);
    return 0;
}
