@echo off
gcc src/main.c -g  -Iinclude -Llib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -o bin/pacman.exe