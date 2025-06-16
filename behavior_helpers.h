// behavior_helpers.h
#pragma once

#include <SDL.h>
#include <stdbool.h>
#include "entity.h"

void apply_thrust_turn(
    Entity* e,
    const Uint8* keystate,
    SDL_Scancode forward_key,
    SDL_Scancode left_key,
    SDL_Scancode right_key,
    float thrust_accel,
    float turn_speed_deg_per_sec,
    float dt);

void apply_thrust_turn_bidirectional(
    Entity* e,
    const Uint8* keystate,
    SDL_Scancode forward_key,
    SDL_Scancode backward_key,
    SDL_Scancode left_key,
    SDL_Scancode right_key,
    float thrust_accel,
    float turn_speed_deg_per_sec,
    float dt);

void toggle_mount_with_key(
    Entity* child,
    Entity* parent,
    const char* mount_point_name,
    SDL_Scancode toggle_key,
    bool* mounted_flag,
    bool* toggle_pressed_flag,
    float* cooldown,
    float cooldown_sec,
    float dt);

void rotate_within_limits(
    Entity* child,
    Entity* parent,
    const Uint8* keystate,
    SDL_Scancode left_key,
    SDL_Scancode right_key,
    float min_rel_angle_deg,
    float max_rel_angle_deg,
    float rotation_speed_deg_per_sec,
    float dt,
    const char* mount_name
);

void rotate_infinite(
    Entity* child,
    Entity* parent,
    const Uint8* keystate,
    SDL_Scancode left_key,
    SDL_Scancode right_key,
    float rotation_speed_deg_per_sec,
    float dt,
    const char* mount_name
);
