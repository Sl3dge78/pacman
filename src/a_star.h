#ifndef A_STAR_H
#define A_STAR_H

#include "SDL2/SDL.h"
#include "game.h"
#include "map.h"

void a_star(const Map *map, const SDL_Point *start, const SDL_Point *end, SDL_Point **path, int *length);
void reverse_a_star(const Map *map, const SDL_Point *start, const SDL_Point *place_to_flee, const int max_distance, SDL_Point **path, int *length);
void dbg_draw_a_star(SDL_Renderer *renderer, const SDL_Point *path, const int length, SDL_Point cam_offset);

#endif