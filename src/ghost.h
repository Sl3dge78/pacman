#ifndef GHOST_H
#define GHOST_H

#include "SDL2/SDL.h"

#include "game.h"
#define PATH_UPDATE_FREQ 1000

struct Ghost;
typedef struct Ghost Ghost;

enum GhostState {
	WAITING,
	ATTACKING,
	FLEEING,
	DEAD
} typedef GhostState;

void ghost_reset(Ghost *ghost, const float speed);
SDL_FPoint *ghost_get_pos(Ghost *ghost);

Ghost *create_ghost(SDL_Renderer *renderer, const float x, const float y, const int wait_time, const int sprite_x, const int sprite_y, const float speed);
void destroy_ghost(Ghost *ghost);
void update_ghost(Ghost *ghost, int delta_time, const SDL_FPoint *player_pos, Map *map);
void draw_ghost(SDL_Renderer *renderer, const Ghost *ghost, const SDL_Point *camera_offset);
void dbg_draw_ghost(Ghost *ghost, SDL_Renderer *renderer, TTF_Font *font, const SDL_Point *camera_offset);
void ghost_kill(Ghost *ghost);
#endif