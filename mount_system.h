#ifndef MOUNT_SYSTEM_H
#define MOUNT_SYSTEM_H

#include <SDL.h>
#include <stdbool.h>

typedef struct Entity Entity;
typedef struct MountedComponent MountedComponent;

// Mount position lookup entry
typedef struct {
    float angle;      // Angle in degrees
    float offset_x;   // X offset from entity center
    float offset_y;   // Y offset from entity center
} MountOffset;

// Added by Claude AI
typedef struct EntityMount {
    char* name;
    MountOffset* offsets;
    int offset_count;
    bool inherit_rotation;
    float rotation_offset;
} EntityMount;

// Mount point definition
typedef struct {
    const char* name;           // e.g., "exhaust", "turret", "weapon"
    MountOffset* offsets;       // Array of angle->offset mappings
    int offset_count;           // Number of entries in offsets array
    bool inherit_rotation;      // Whether mounted component rotates with parent
    float rotation_offset;      // Additional rotation offset for this mount
} MountPoint;

// Component that can be mounted
typedef struct MountedComponent {
    const char* type;           // Component type identifier
    Entity* entity;             // The entity being mounted (can be NULL for effects)

    // Component-specific data (union for different types)
    union {
        struct {
            SDL_Texture** frames;
            int frame_count;
            int current_frame;
            Uint32 frame_delay;
            Uint32 last_update;
            int width, height;
        } animation;

        struct {
            SDL_Texture* texture;
            int width, height;
        } sprite;

      Entity* entity;
    } data;

    // Component lifecycle callbacks
    void (*update)(MountedComponent* comp, float dt);
    void (*render)(SDL_Renderer* renderer, MountedComponent* comp, float x, float y, float angle);
    void (*destroy)(MountedComponent* comp);

    // Mount state
    bool active;
    float local_angle;

    struct MountedComponent* next;  // Linked list for multiple components per mount
} MountedComponent;

// Mount system functions
void mount_system_init(Entity* entity, int mount_count);
void mount_system_cleanup(Entity* entity);  // ✅ Add missing prototype

void interpolate_mount_offset(MountPoint* mount, float angle, float* out_x, float* out_y);
int  mount_add_point(Entity* entity, const char* name, MountOffset* offsets, int offset_count,
                     bool inherit_rotation, float rotation_offset);

MountedComponent* mount_create_animation_component(const char* type, SDL_Texture** frames,
                                                   int frame_count, int width, int height, Uint32 frame_delay);

MountedComponent* mount_create_sprite_component(const char* type, SDL_Texture* texture,
                                                int width, int height);

bool mount_attach_component(Entity* entity, const char* mount_name, MountedComponent* component);
bool mount_detach_component(Entity* entity, const char* mount_name, const char* component_type);
bool mount_attach_entity(Entity* parent, const char* mount_name, Entity* child);
bool mount_detach_entity(Entity* parent, const char* mount_name);
void mount_get_world_position(const Entity* entity, const char* mount_name,
                              float* out_x, float* out_y, float* out_angle);

void mount_update_all(Entity* entity, float dt);
void mount_render_all(SDL_Renderer* renderer, const Entity* entity);

// Offset utilities
MountOffset* mount_create_offset_table(int count);
void         mount_set_offset(MountOffset* table, int index, float angle, float offset_x, float offset_y);

void mount_system_init_with_entities(Entity* entity, int component_mounts, int entity_mounts);
void entity_mount_update_all(Entity* entity, float dt);
void entity_mount_render_all(SDL_Renderer* renderer, const Entity* entity);
void mount_system_cleanup_entities(Entity* entity);

int entity_mount_add_point(Entity* entity, const char* name, MountOffset* offsets, int offset_count,
			   bool inherit_rotation, float rotation_offset);
void entity_mount_get_world_position(const Entity* entity, const char* mount_name, 
				     float* out_x, float* out_y, float* out_angle);
bool entity_mount_attach(Entity* parent, const char* mount_name, Entity* child);
bool entity_mount_detach(Entity* parent, const char* mount_name);

#endif
