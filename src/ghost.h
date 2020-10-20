#ifndef GHOST_H
#define GHOST_H

#include "SDL2/SDL.h"

#include "game.h"

#define GHOST_SPEED 2.0f
#define PATH_UPDATE_FREQ 1000

struct Ghost;
typedef struct Ghost Ghost;

Ghost *create_ghost(SDL_Renderer *renderer);
void destroy_ghost(Ghost *ghost);
void update_ghost(int delta_time, Ghost *ghost, const SDL_FPoint *player_pos, Map *map);
void draw_ghost(SDL_Renderer *renderer, const Ghost *ghost, const SDL_Point *camera_offset);

#endif