#ifndef GAME_H
#define GAME_H

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

#define FPS 60
#define FRAME_TIME 1.0f / (float)FPS

#define MAP_WIDTH 28
#define MAP_HEIGHT 31
#define MAP_SIZE MAP_WIDTH *MAP_HEIGHT

#define POWERUP_MAX_TIME 10000
#define PAC_AMOUNT 240

#define PTS_FOR_NEW_LIFE 100000
#define STARTING_LIVES 2

#define GHOST_AMT 4

void run(SDL_Renderer *renderer);

struct Map;
typedef struct Map Map;

enum Tile {
	DOOR = -4,
	POWERUP = -3,
	PAC = -2,
	EMPTY = -1,
	TURN_RIGHT = 0,
	TURN_DOWN = 1,
	STRAIGHT_HOR = 2,
	TURN_UP = 3,
	TURN_LEFT = 4,
	STRAIGHT_VER = 5,
	OUTMAP = 6
} typedef Tile;

enum Alignement {
	ALIGN_LEFT = 0,
	ALIGN_CENTERED,
	ALIGN_RIGHT
} typedef Alignement;

enum Direction {
	EAST = 0,
	SOUTH = 1,
	NORTH = 2,
	WEST = 3,
} typedef Direction;

Tile get_tile_at_pos(const int x, const int y, const Map *map);

int draw_text(SDL_Renderer *renderer, TTF_Font *font, char *text, const SDL_Point *src, Alignement align);

#endif