#include "entity.h"
#include <SDL.h>

void render_animated_entity(SDL_Renderer* renderer, AnimatedEntity* ae, float delta_ms) {
    if (!ae || !ae->base.active) return;
    
    // Check animation-specific fields
    if (!ae->frames || ae->frame_count == 0) return;
    
    // Update animation timing
    ae->frame_timer += delta_ms;
    if (ae->frame_timer >= ae->frame_delay_ms) {
        ae->frame_timer = 0;
        ae->current_frame = (ae->current_frame + 1) % ae->frame_count;
    }
    
    // Render using base entity position/size and animated texture
    SDL_Rect dst = {
        (int)(ae->base.x - ae->base.width / 2),
        (int)(ae->base.y - ae->base.height / 2),
        ae->base.width,
        ae->base.height
    };
    
    SDL_RenderCopyEx(renderer, ae->frames[ae->current_frame], NULL, &dst, ae->base.angle, NULL, SDL_FLIP_NONE);
}
