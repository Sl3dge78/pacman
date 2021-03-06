#include "utils.h"

#include <math.h>

unsigned int SDL_Point_Distance(const SDL_Point *a, const SDL_Point *b) {
	int x = a->x - b->x;
	int y = a->y - b->y;
	unsigned int ux = x < 0 ? x * -1 : x;
	unsigned int uy = y < 0 ? y * -1 : y;
	return ux + uy;
}

float SDL_2FPoint_Distance(const SDL_FPoint *a, const SDL_FPoint *b) {
	float x = a->x - b->x;
	float y = a->y - b->y;
	float ux = x < 0 ? x * -1 : x;
	float uy = y < 0 ? y * -1 : y;
	return ux + uy;
}

float SDL_1FPoint_Distance(const SDL_Point *a, const SDL_FPoint *b) {
	float x = a->x - b->x;
	float y = a->y - b->y;
	float ux = x < 0 ? x * -1 : x;
	float uy = y < 0 ? y * -1 : y;
	return ux + uy;
}

void SDL_FPoint_Normalize(SDL_FPoint *vec) {
	float length = sqrt(vec->x * vec->x + vec->y * vec->y);
	if (length == 0)
		return;
	vec->x /= length;
	vec->y /= length;
}

bool SDL_Point_Equals(const SDL_Point *a, const SDL_Point *b) {
	return (a->x == b->x && a->y == b->y);
}

bool SDL_FPoint_Equals(const SDL_FPoint *a, const SDL_FPoint *b) {
	return abs(a->x - b->x) < 0.001f && abs(a->y - b->y) < 0.001f;
}
