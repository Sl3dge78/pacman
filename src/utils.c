#include "utils.h"

unsigned int SDL_POINT_Distance(const SDL_Point *a, const SDL_Point *b) {
	int x = a->x - b->x;
	int y = a->y - b->y;
	unsigned int ux = x < 0 ? x * -1 : x;
	unsigned int uy = y < 0 ? y * -1 : y;
	return ux + uy;
}