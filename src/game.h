#ifndef GAME_H
#define GAME_H

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

#define FPS 60
#define FRAME_TIME 1.0f / (float)FPS

#define POWERUP_MAX_TIME 10000
#define PAC_AMOUNT 240

#define PTS_FOR_NEW_LIFE 100000
#define STARTING_LIVES 2

#define GHOST_AMT 4

void run(SDL_Renderer *renderer);

struct GraphMap;
typedef struct GraphMap GraphMap;

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
	NONE
} typedef Direction;

int draw_text(SDL_Renderer *renderer, TTF_Font *font, char *text, const SDL_Point *src, Alignement align);
static void next_level();

#endif