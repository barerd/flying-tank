#ifndef MOUNT_SYSTEM_H
#define MOUNT_SYSTEM_H

#include <SDL.h>
#include <stdbool.h>

typedef struct Entity Entity;
typedef struct MountedComponent MountedComponent;

typedef struct {
    float angle;
    float offset_x;
    float offset_y;
} MountOffset;

// Added by Claude AI
typedef struct MountPoint {
    char* name;
    MountOffset* offsets;
    int offset_count;
    bool inherit_rotation;
    float rotation_offset;
} MountPoint;

MountOffset* mount_create_offset_table(int count);
void mount_set_offset(MountOffset* offsets, int index, float angle, float offset_x, float offset_y);

void mount_system_init(Entity* entity, int mount_count);
void mount_update_all(Entity* entity, float dt);
void mount_render_all(SDL_Renderer* renderer, const Entity* entity);
void mount_system_cleanup(Entity* entity);
void interpolate_mount_offset(MountPoint* mount, float angle, float* out_x, float* out_y);
int  mount_add_point(Entity* entity, const char* name, MountOffset* offsets, int offset_count,
		     bool inherit_rotation, float rotation_offset, int slot_index);

void mount_get_world_position(const Entity* entity, const char* mount_name, 
				     float* out_x, float* out_y, float* out_angle);
bool mount_attach(Entity* parent, const char* mount_name, Entity* child);
bool mount_detach(Entity* parent, const char* mount_name);

#endif
