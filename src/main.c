#include <stdio.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

#include "Utils.h"

#include "game.c"

int main(int argc, char* args[]) {

    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window *window = NULL;
    window = SDL_CreateWindow("Pacman", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, 0);

    SDL_Renderer *renderer = NULL;
    renderer = SDL_CreateRenderer(window, -1, 0);

    run(renderer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();

    return 0;
}