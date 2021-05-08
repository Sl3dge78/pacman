@echo off
REM gcc src/*.c -std=c17 -g -Iinclude -Llib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -o bin/pacman.exe

@echo off
set project_name=pacman

set args= -GR- -EHa -nologo -Zi -experimental:external -external:anglebrackets -DDEBUG
set include_path=-external:I ..\include\ 

set linker_options=-link -SUBSYSTEM:WINDOWS -LIBPATH:..\lib 
set libs=SDL2main.lib SDL2.lib SDL2_ttf.lib SDL2_image.lib SDL2_mixer.lib Shell32.lib

pushd bin
cl %args% -Fe%project_name% %include_path% ../src/*.c %linker_options% %libs%
popd
echo Build completed!
