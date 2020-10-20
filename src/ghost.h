#ifndef GHOST_H
#define GHOST_H

#include "SDL2/SDL.h"

#define GHOST_SPEED 5.0f

struct Ghost;
typedef struct Ghost Ghost;

Ghost *create_ghost(SDL_Renderer *renderer);
void destroy_ghost(Ghost *ghost);
void update_ghost(int delta_time, Ghost *ghost);
void draw_ghost(SDL_Renderer *renderer, const Ghost *ghost, const SDL_Point *camera_offset);

#endif