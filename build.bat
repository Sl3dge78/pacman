@echo off
gcc src/*.c -std=c17 -g -Iinclude -Llib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf  -o bin/pacman.exe