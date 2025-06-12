#include "tank.h"
#include "exhaust_flame.h"
#include "entity.h"
#include <SDL.h>

static Exhaust_Flame exhaust_flame;

bool tank_load(SDL_Renderer* renderer, Tank* tank) {
    if (!entity_load_texture(renderer, &tank->base, "assets/tank.png"))
        return false;

    if (!flame_load(renderer, &exhaust_flame, "assets/exhaust-flame", 4))
        return false;
	
    tank->base.x = 500 - 104;
    tank->base.y = 375 - 104;
    tank->base.angle = 0;
    tank->base.speed = 0;
    tank->base.max_speed = 5.0f;
    tank->base.accel = 0.3f;
    tank->base.friction = 0.05f;

    tank->exhaust_mount.offset_x = -54;
    tank->exhaust_mount.offset_y = 0;

    return true;
}

void tank_update(Tank* tank, const Uint8* keystate) {
    if (keystate[SDL_SCANCODE_LEFT])
        entity_turn(&tank->base, -5.0f);
    if (keystate[SDL_SCANCODE_RIGHT])
        entity_turn(&tank->base, +5.0f);
    if (keystate[SDL_SCANCODE_UP])
        entity_thrust(&tank->base, tank->base.accel);
    /* if (keystate[SDL_SCANCODE_DOWN]) */
    /*     entity_thrust(tank, -tank->accel) */;

    entity_update(&tank->base);
}

void tank_render(SDL_Renderer* renderer, Tank* tank, const Uint8* keystate) {
    entity_render(renderer, &tank->base, 208, 208);

    if (keystate[SDL_SCANCODE_LEFT] || keystate[SDL_SCANCODE_RIGHT] || keystate[SDL_SCANCODE_UP]) {
        float fx, fy;
        mount_to_world_coords(&tank->base, &tank->exhaust_mount, &fx, &fy);
        flame_update(&exhaust_flame);
        flame_render(renderer, &exhaust_flame, fx, fy, tank->base.angle);
    }
}

void tank_unload(Tank* tank) {
    entity_unload(&tank->base);
    flame_unload(&exhaust_flame);
}
