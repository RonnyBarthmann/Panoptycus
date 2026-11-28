#include <stdio.h>
#include <string.h>

#include "sdl_wrapper.h"
#include "game.h"

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE  "Idle Heroes Manager"

int main(int argc, char **argv)
{
    int editor = 0;
    for (int i = 1; i < argc; i++) if (!strcmp(argv[i], "--editor")) editor = 1;

    if (SDL_Wrapper_Init(WINDOW_WIDTH, WINDOW_HEIGHT, editor ? "Panoptycus - Savegame-Editor" : WINDOW_TITLE) != 0) {
        fprintf(stderr, "Fehler: SDL konnte nicht initialisiert werden.\n");
        return 1;
    }

    GameSetEditor(editor);
    GameMain();

    SDL_Wrapper_Quit();

    return 0;
}
