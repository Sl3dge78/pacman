#ifndef GAME_H
#define GAME_H

#include "SDL2/SDL.h"

#define FPS 60
#define FRAME_TIME 1.0f / (float)FPS

#define MAP_WIDTH 28
#define MAP_HEIGHT 31
#define MAP_SIZE MAP_WIDTH *MAP_HEIGHT

#define POWERUP_MAX_TIME 10000
#define PAC_AMOUNT 240

#define PTS_FOR_NEW_LIFE 100000
#define STARTING_LIVES 2

#define GHOST_AMT 1

void run(SDL_Renderer *renderer);

struct Map;
typedef struct Map Map;

enum Tile {
	PAC = -2,
	POWERUP = -3,
	EMPTY = -1,
	TURN_RIGHT = 0,
	TURN_DOWN = 1,
	STRAIGHT_HOR = 2,
	TURN_UP = 3,
	TURN_LEFT = 4,
	STRAIGHT_VER = 5,
} typedef Tile;

Tile get_tile_at_pos(const int x, const int y, const Map *map);

#endif