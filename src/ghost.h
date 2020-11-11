#ifndef GHOST_H
#define GHOST_H

#include "SDL2/SDL.h"

#include "game.h"
#include "map.h"

#define PATH_UPDATE_FREQ 2000

struct Ghost;
typedef struct Ghost Ghost;

enum GhostState {
	WAITING,
	ATTACKING,
	FLEEING,
	DEAD
} typedef GhostState;

void ghost_reset(Ghost *ghost, const float speed);
void ghost_switch_state(Ghost *ghost, const GhostState state);
SDL_FPoint *ghost_get_pos(Ghost *ghost);

Ghost *create_ghost(SDL_Renderer *renderer, const float x, const float y, const int wait_time, const int sprite_x, const int sprite_y);
void destroy_ghost(Ghost *ghost);
void update_ghost(Ghost *ghost, int delta_time, const SDL_FPoint *player_pos, Map *map);
void draw_ghost(SDL_Renderer *renderer, const Ghost *ghost, const SDL_Point *camera_offset);
void dbg_draw_ghost(Ghost *ghost, SDL_Renderer *renderer, TTF_Font *font, const SDL_Point *camera_offset);
void ghost_kill(Ghost *ghost);
#endif