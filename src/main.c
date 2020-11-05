#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"

#include "debug.h"
#include "utils.h"

#include "game.h"

/*

	Debug
	Implement fleeing pathfinding

*/

int main(int argc, char *args[]) {
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();

	SDL_Window *window = NULL;
	window = SDL_CreateWindow("Pacman", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 16 * 28, 16 * 32, 0);

	SDL_Renderer *renderer = NULL;
	renderer = SDL_CreateRenderer(window, -1, 0);

	run(renderer);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();

	DBG_dump_memory_leaks();
	return 0;
}