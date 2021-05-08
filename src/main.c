#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_mixer.h"
#include "SDL2/SDL_ttf.h"

#include "debug.h"
#include "utils.h"

#include "game.h"

/*
	Fix the mem leaks
	Win game state (blink screen and stuff)
	Add multiple behaviours for ghosts
	More SFX (when empowered, eating ghosts,...)
	Improve ghost eating 
		SFX
		Freeze frame when eating ghosts
		Display score text when eating ghosts
		Eye srpite
*/

int main(int argc, char *args[]) {
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
	SDL_Window *window = NULL;
	window = SDL_CreateWindow("Pacman", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 16 * 28, 16 * 32, 0);
    
	SDL_Renderer *renderer = NULL;
	renderer = SDL_CreateRenderer(window, -1, 0);
    
	run(renderer, window);
    
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
    
	Mix_CloseAudio();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
    
	DBG_dump_memory_leaks();
	return 0;
}