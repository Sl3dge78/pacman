#ifndef UTILS_H
#define UTILS_H

#include "SDL2/SDL.h"

#define true 1
#define false 0
#define bool unsigned int

#define CLAMP(min, x, max) (x > max) ? (max) : ((x < min) ? min : x)

unsigned int SDL_Point_Distance(const SDL_Point *a, const SDL_Point *b);
float SDL_FPoint_Distance(const SDL_FPoint *a, const SDL_FPoint *b);

bool SDL_Point_Equals(const SDL_Point *a, const SDL_Point *b);

#endif
