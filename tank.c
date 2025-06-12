#include "tank.h"
#include "exhaust_flame.h"
#include "entity.h"
#include <SDL_image.h>
#include <stdio.h>

static bool load_flame_textures(SDL_Renderer* renderer, SDL_Texture** frames, const char* basepath, int frame_count) {
    for (int i = 0; i < frame_count; i++) {
        char filename[256];
        snprintf(filename, sizeof(filename), "%s%d.png", basepath, i);
        
        SDL_Surface* surf = IMG_Load(filename);
        if (!surf) {
            fprintf(stderr, "Failed to load %s: %s\n", filename, IMG_GetError());
            // Clean up on failure
            for (int j = 0; j < i; j++) {
                if (frames[j]) SDL_DestroyTexture(frames[j]);
            }
            return false;
        }
        
        frames[i] = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        
        if (!frames[i]) {
            fprintf(stderr, "Failed to create texture from %s: %s\n", filename, SDL_GetError());
            for (int j = 0; j < i; j++) {
                if (frames[j]) SDL_DestroyTexture(frames[j]);
            }
            return false;
        }
    }
    return true;
}

bool tank_load(SDL_Renderer* renderer, Tank* tank) {
    SDL_Surface* surf = IMG_Load("assets/tank.png");
    if (!surf) return false;
    
    tank->base.texture = SDL_CreateTextureFromSurface(renderer, surf);
    tank->base.width = surf->w;
    tank->base.height = surf->h;
    SDL_FreeSurface(surf);
    
    if (!tank->base.texture) return false;
    
    // 1. Initialize entity
    tank->base.x = 500;
    tank->base.y = 375;
    tank->base.angle = 0;
    tank->base.speed = 0;
    tank->base.max_speed = 5.0f;
    tank->base.accel = 0.3f;
    tank->base.friction = 0.05f;
    
    // 2. Initialize mount system
    mount_system_init(&tank->base, 4); // Room for exhaust and potential turret
    
    // 3. Create exhaust mount with lookup table
    MountOffset* exhaust_offsets = mount_create_offset_table(8);
    mount_set_offset(exhaust_offsets, 0,   0.0f, -54,   0); // 0° - pointing up, exhaust below
    mount_set_offset(exhaust_offsets, 1,  45.0f, -38,  38); // 45° - exhaust bottom-right
    mount_set_offset(exhaust_offsets, 2,  90.0f,   0,  54); // 90° - pointing right, exhaust left
    mount_set_offset(exhaust_offsets, 3, 135.0f,  38,  38); // 135°
    mount_set_offset(exhaust_offsets, 4, 180.0f,  54,   0); // 180° - pointing down, exhaust above
    mount_set_offset(exhaust_offsets, 5, 225.0f,  38, -38); // 225°
    mount_set_offset(exhaust_offsets, 6, 270.0f,   0, -54); // 270° - pointing left, exhaust right
    mount_set_offset(exhaust_offsets, 7, 315.0f, -38, -38); // 315°
    
    mount_add_point(&tank->base, "exhaust", exhaust_offsets, 8, true, 0.0f);
    
    // Create exhaust flame component
    SDL_Texture** flame_frames = calloc(4, sizeof(SDL_Texture*));
    if (!load_flame_textures(renderer, flame_frames, "assets/exhaust-flame", 4)) {
        free(flame_frames);
        return false;
    }
    
    tank->exhaust_flame = mount_create_animation_component("exhaust_flame", flame_frames, 4, 32, 32, 100);
    mount_attach_component(&tank->base, "exhaust", tank->exhaust_flame);

    // 4. Create mount point for turret (static, no angle-dependent offset needed)
    MountOffset* turret_offsets = mount_create_offset_table(1);
    mount_set_offset(turret_offsets, 0, 0.0f, 46, -27);  // angle, offset_x, offset_y

    // Add the mount
    mount_add_point(&tank->base, "turret", turret_offsets, 1, true, 0.0f); // true: cisme göre sabit durur, false: pencereye göre sabit durud

    // Load turret texture
    SDL_Surface* turret_surf = IMG_Load("assets/turret.png");
    if (!turret_surf) {
        fprintf(stderr, "Failed to load turret.png: %s\n", IMG_GetError());
        return false;
    }
    SDL_Texture* turret_texture = SDL_CreateTextureFromSurface(renderer, turret_surf);
    SDL_SetTextureBlendMode(turret_texture, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(turret_surf);

    // Create and attach sprite component
    MountedComponent* turret_sprite = mount_create_sprite_component("turret", turret_texture, 10, 40);
    mount_attach_component(&tank->base, "turret", turret_sprite);
    
    tank->thrusting = false;
    return true;
}

void tank_update(Tank* tank, const Uint8* keystate, float dt) {
    // Handle input
    if (keystate[SDL_SCANCODE_LEFT])
        tank->base.angle -= 5.0f * dt * 60.0f; // Normalize for 60 FPS
    if (keystate[SDL_SCANCODE_RIGHT])
        tank->base.angle += 5.0f * dt * 60.0f;
    
    tank->thrusting = keystate[SDL_SCANCODE_UP];
    if (tank->thrusting)
        tank->base.speed += tank->base.accel;
    
    // Clamp speed
    if (tank->base.speed > tank->base.max_speed) tank->base.speed = tank->base.max_speed;
    if (tank->base.speed < -tank->base.max_speed) tank->base.speed = -tank->base.max_speed;
    
    // Update position
    float angle_rad = (tank->base.angle - 90.0f) * (M_PI / 180.0f);
    tank->base.x += cosf(angle_rad) * tank->base.speed;
    tank->base.y += sinf(angle_rad) * tank->base.speed;
    
    // Apply friction
    if (tank->base.speed > 0) {
        tank->base.speed -= tank->base.friction;
        if (tank->base.speed < 0) tank->base.speed = 0;
    } else if (tank->base.speed < 0) {
        tank->base.speed += tank->base.friction;
        if (tank->base.speed > 0) tank->base.speed = 0;
    }
    
    // Control exhaust flame visibility (matches your original behavior)
    tank->exhaust_flame->active = (keystate[SDL_SCANCODE_LEFT] || 
                                   keystate[SDL_SCANCODE_RIGHT] || 
                                   keystate[SDL_SCANCODE_UP]);
    
    // Update mounted components
    mount_update_all(&tank->base, dt);
}

void tank_render(SDL_Renderer* renderer, Tank* tank) {
    // Render main tank
    SDL_Rect dst = {
        (int)(tank->base.x - tank->base.width / 2),
        (int)(tank->base.y - tank->base.height / 2),
        tank->base.width,
        tank->base.height
    };
 
    SDL_RenderCopyEx(renderer, tank->base.texture, NULL, &dst, tank->base.angle, NULL, SDL_FLIP_NONE);
    
    // Render all mounted components
    mount_render_all(renderer, &tank->base);
}

void tank_unload(Tank* tank) {
    if (tank->base.texture) {
        SDL_DestroyTexture(tank->base.texture);
        tank->base.texture = NULL;
    }
    
    mount_system_cleanup(&tank->base);
}
