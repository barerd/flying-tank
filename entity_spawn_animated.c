#include "entity.h"
#include <SDL_image.h>
#include <stdlib.h>
#include <string.h>

/* Entity* spawn_animated_entity(const char* id, SDL_Renderer* renderer, const char* base_path, int frame_count, float x, float y) { */
/*     Entity* e = malloc(sizeof(Entity)); */
/*     if (!e) return NULL; */
/*     memset(e, 0, sizeof(Entity)); */

/*     e->id = strdup(id); */
/*     e->x = x; */
/*     e->y = y; */
/*     e->angle = 0.0f; */
/*     e->active = true; */
/*     e->frame_count = frame_count; */
/*     e->current_frame = 0; */
/*     e->frame_timer = 0; */
/*     e->frame_delay_ms = 80; */

/*     e->frames = malloc(sizeof(SDL_Texture*) * frame_count); */
/*     if (!e->frames) { */
/*         free(e); */
/*         return NULL; */
/*     } */

/*     char path[128]; */
/*     for (int i = 0; i < frame_count; ++i) { */
/*         snprintf(path, sizeof(path), "%s%d.png", base_path, i); */
/*         SDL_Surface* surf = IMG_Load(path); */
/*         if (!surf) { */
/*             SDL_Log("Failed to load frame %d for %s: %s", i, id, IMG_GetError()); */
/*             for (int j = 0; j < i; ++j) */
/*                 SDL_DestroyTexture(e->frames[j]); */
/*             free(e->frames); */
/*             free(e); */
/*             return NULL; */
/*         } */
/*         e->frames[i] = SDL_CreateTextureFromSurface(renderer, surf); */
/*         SDL_FreeSurface(surf); */
/*     } */

/*     SDL_QueryTexture(e->frames[0], NULL, NULL, &e->width, &e->height); */

/*     return e; */
/* } */

AnimatedEntity* spawn_animated_entity(const char* id, SDL_Renderer* renderer, const char* base_path, int frame_count, float x, float y) {
    AnimatedEntity* ae = malloc(sizeof(AnimatedEntity));
    if (!ae) return NULL;
    memset(ae, 0, sizeof(AnimatedEntity));

    // Set entity type
    ae->base.type = ENTITY_ANIMATED;
    // Set base entity properties
    ae->base.x = x;
    ae->base.y = y;
    ae->base.angle = 0.0f;
    ae->base.active = true;
    
    // Set animated-specific properties
    ae->base.id = strdup(id);
    ae->frame_count = frame_count;
    ae->current_frame = 0;
    ae->frame_timer = 0;
    ae->frame_delay_ms = 80;
    ae->is_animated = true;
    
    ae->frames = malloc(sizeof(SDL_Texture*) * frame_count);
    if (!ae->frames) {
        free(ae->base.id);
        free(ae);
        return NULL;
    }
    
    char path[128];
    for (int i = 0; i < frame_count; ++i) {
        snprintf(path, sizeof(path), "%s%d.png", base_path, i);
        SDL_Surface* surf = IMG_Load(path);
        if (!surf) {
            SDL_Log("Failed to load frame %d for %s: %s", i, id, IMG_GetError());
            // Cleanup on failure
            for (int j = 0; j < i; ++j)
                SDL_DestroyTexture(ae->frames[j]);
            free(ae->frames);
            free(ae->base.id);
            free(ae);
            return NULL;
        }
        ae->frames[i] = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }
    
    SDL_QueryTexture(ae->frames[0], NULL, NULL, &ae->base.width, &ae->base.height);
    
    // Return as Entity* (safe because base is first member)
    return ae;
}
