#include "mount_system.h"
#include "behavior_helpers.h"
#include <math.h>
#include <SDL.h>
#include <stdbool.h>

void apply_thrust_turn(
    Entity* e,
    const Uint8* keystate,
    SDL_Scancode forward_key,
    SDL_Scancode left_key,
    SDL_Scancode right_key,
    float thrust,
    float turn_speed_deg_per_sec,
    float dt
) {
    if (keystate[forward_key])
        entity_thrust(e, e->accel * thrust * dt);
    if (keystate[left_key])
        entity_turn(e, -turn_speed_deg_per_sec * thrust * dt);
    if (keystate[right_key])
        entity_turn(e, turn_speed_deg_per_sec * thrust * dt);
}

void apply_thrust_turn_bidirectional(
    Entity* e,
    const Uint8* keystate,
    SDL_Scancode forward_key,
    SDL_Scancode backward_key,
    SDL_Scancode left_key,
    SDL_Scancode right_key,
    float thrust,
    float turn_speed_deg_per_sec,
    float dt
) {
    if (keystate[forward_key])
        entity_thrust(e, e->accel * thrust * dt);
    if (keystate[backward_key])
        entity_thrust(e, -e->accel * thrust * dt);
    if (keystate[left_key])
        entity_turn(e, -turn_speed_deg_per_sec * thrust * dt);
    if (keystate[right_key])
        entity_turn(e, turn_speed_deg_per_sec * thrust * dt);
}

void apply_afterburner(
    Entity* e,
    const Uint8* keystate,
    SDL_Scancode key,
    float thrust,
    float dt
) {
    if (keystate[key])
        entity_thrust(e, e->accel * thrust * dt);
}

void toggle_mount_with_key(
    Entity* child,
    Entity* parent,
    const char* mount_name,
    SDL_Scancode toggle_key,
    bool* mounted_flag,
    bool* key_debounce,
    float* cooldown,
    float cooldown_duration,
    float dt
) {
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    *cooldown -= dt;
    if (*cooldown < 0.0f) *cooldown = 0.0f;

    if (keystate[toggle_key] && !*key_debounce) {
        *key_debounce = true;

        if (*mounted_flag) {
            entity_set_position(child, parent->x + 40, parent->y);
            child->active = true;
            mount_detach(parent, mount_name);
            *mounted_flag = false;
            *cooldown = cooldown_duration;
        } else if (*cooldown <= 0.0f) {
            float x, y, angle;
            mount_get_world_position(parent, mount_name, &x, &y, &angle);
            entity_set_position(child, x, y);
            mount_attach(parent, mount_name, child);
            child->active = true;
            *mounted_flag = true;
        }
    }

    if (!keystate[toggle_key]) {
        *key_debounce = false;
    }
}

void rotate_within_limits(
    Entity* parent,
    const char* mount_name,
    const Uint8* keystate,
    SDL_Scancode left_key,
    SDL_Scancode right_key,
    float min_angle,
    float max_angle,
    float rotation_speed_deg_per_sec,
    float dt
) {
    MountPoint* mp = mount_get(parent, mount_name);
    if (!mp) return;
    
    float rel_angle = mp->offsets[0].angle;
    if (keystate[left_key] && rel_angle > min_angle)
        rel_angle -= rotation_speed_deg_per_sec * dt;
    if (keystate[right_key] && rel_angle < max_angle)
        rel_angle += rotation_speed_deg_per_sec * dt;

    if (rel_angle < min_angle) rel_angle = min_angle;
    if (rel_angle > max_angle) rel_angle = max_angle;

    mp->offsets[0].angle = rel_angle;
}

void rotate_infinite(
    Entity* parent,
    const char* mount_name,
    const Uint8* keystate,
    SDL_Scancode left_key,
    SDL_Scancode right_key,
    float rotation_speed_deg_per_sec,
    float dt
) {
    MountPoint* mp = mount_get(parent, mount_name);
    if (!mp) return;
    
    if (keystate[left_key])
        mp->offsets[0].angle -= rotation_speed_deg_per_sec * dt;
    if (keystate[right_key])
        mp->offsets[0].angle += rotation_speed_deg_per_sec * dt;
}
