#include <stdio.h>
#include <string.h>

#include "sdl_wrapper.h"
#include "game.h"
#include "config.h"

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE  "Idle Heroes Manager"

int main(int argc, char **argv)
{
    int editor = 0;
    for (int i = 1; i < argc; i++) if (!strcmp(argv[i], "--editor")) editor = 1;

    int cfgErrors = GameLoadConfigs();
    if (cfgErrors > 0) {
        fprintf(stderr, "\n%d Fehler in den Konfigurationsdateien - Spiel wird nicht gestartet.\n", cfgErrors);
        return 1;
    }
    int winW, winH;
    GameWindowSize(&winW, &winH);
    if (winW < 320) winW = 320;
    if (winH < 200) winH = 200;

    if (SDL_Wrapper_Init(winW, winH, editor ? "Panoptycus - Savegame-Editor" : WINDOW_TITLE) != 0) {
        fprintf(stderr, "Fehler: SDL konnte nicht initialisiert werden.\n");
        return 1;
    }

    GameSetEditor(editor);
    GameMain();

    SDL_Wrapper_Quit();

    return 0;
}
