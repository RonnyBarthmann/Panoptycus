#include "game.h"
#include "sdl_wrapper.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "core.h"

#define DEBUG 1
#define CONFIG_FILE "game.conf"
#define SAVE_FILE   "savegame.dat"
#define SAVE_MAGIC  0x50414E34u   /* "PAN4" */

#define MOUSEBUTTON_Left 1
#define MOUSEBUTTON_Right 2
#define MOUSEBUTTON_Middle 3
#define MOUSEBUTTON_Any 0

static int renderWidth;
static int renderHeight;

/* Die Module werden direkt eingebunden (wie schon maps.c vorher). */
#include "ui.c"
#include "maps.c"
#include "data.c"
#include "sim.c"
#include "render.c"
#include "screens.c"

/* ================================================================== */
/* Speichern / Laden                                                  */
/* ================================================================== */

typedef struct { unsigned int magic; int stageCount; int heroSize, playerSize; } SaveHeader;

static void SaveGame(void) {
    FILE *f = fopen(SAVE_FILE, "wb");
    if (!f) return;
    SaveHeader h = { SAVE_MAGIC, StageCount, (int)sizeof heroes, (int)sizeof player };
    fwrite(&h, sizeof h, 1, f);
    fwrite(&player, sizeof player, 1, f);
    fwrite(heroes, sizeof heroes, 1, f);
    fclose(f);
}

/* liest nur die Rohdaten (player, heroes); 0 bei fehlender/unpassender Datei */
static int LoadSaveRaw(void) {
    FILE *f = fopen(SAVE_FILE, "rb");
    if (!f) return 0;
    SaveHeader h;
    int ok = fread(&h, sizeof h, 1, f) == 1 && h.magic == SAVE_MAGIC &&
             h.heroSize == (int)sizeof heroes && h.playerSize == (int)sizeof player &&
             fread(&player, sizeof player, 1, f) == 1 && fread(heroes, sizeof heroes, 1, f) == 1;
    fclose(f);
    return ok;
}

static int LoadGame(void) {
    if (!LoadSaveRaw()) return 0;
    int stageOf[MAX_HEROES], partnerOf[MAX_HEROES];
    for (int i = 0; i < MAX_HEROES; i++) { stageOf[i] = heroes[i].stage; partnerOf[i] = heroes[i].partner; heroes[i].inst = -1; heroes[i].stage = -1; heroes[i].partner = -1; }
    for (int i = 0; i < MAX_HEROES; i++) {
        if (!heroes[i].isHero || stageOf[i] < 0 || heroes[i].inst >= 0) continue;
        int p = partnerOf[i];
        if (p < 0 || !heroes[p].isHero || stageOf[p] != stageOf[i] || heroes[p].inst >= 0) p = -1;
        double hp = heroes[i].hp; (void)hp;
        StartStage(i, stageOf[i], p);
    }
    for (int i = 0; i < MAX_HEROES; i++) if (heroes[i].isHero) RecalcHero(i);
    return 1;
}

static void NewGame(void) {
    memset(&player, 0, sizeof player);
    for (int i = 0; i < MAX_HEROES; i++) { memset(&instances[i], 0, sizeof instances[i]); ClearHero(i); }
    player.level = 1; player.gold = cfg.start_gold;
    for (int l = 0; l < LAYOUT_COUNT; l++) for (int c = 0; c < 4; c++) player.layouts[l][c] = -1;
    Item m;
    for (int k = 0; k < MAT_COUNT; k++) { m = ItemMakeMaterial(TIER_NORMAL, k, (int)cfg.start_material); InvAdd(&m); }
    ShopRestock();
    int h = CreateHero(CLASS_Knight);
    StartStage(h, 0, -1);
    player.layouts[0][0] = h;
}

/* Gruppiert Helden ohne Platz in der aktuellen Anordnung automatisch ein */
static void SyncLayouts(void) {
    /* neue Helden werden einmalig in ein freies Feld der Anordnung 1 gesetzt; bewusst geleerte Felder bleiben leer */
    static unsigned seen = 0; static int init = 0;
    int *lay = player.layouts[0];
    for (int c = 0; c < 4; c++) if (lay[c] >= 0 && !heroes[lay[c]].isHero) lay[c] = -1;
    for (int h = 0; h < MAX_HEROES; h++) {
        unsigned bit = 1u << h;
        if (!heroes[h].isHero) { seen &= ~bit; continue; }
        if (seen & bit) continue;
        seen |= bit;
        if (!init) continue;
        int shown = 0; for (int c = 0; c < 4; c++) if (lay[c] == h) shown = 1;
        if (shown) continue;
        for (int c = 0; c < 4; c++) if (lay[c] < 0) { lay[c] = h; break; }
    }
    init = 1;
}

/* ================================================================== */
/* Einstellungen                                                      */
/* ================================================================== */

/* Lädt alle Konfigurationsdateien. Rückgabe: Anzahl Fehler (0 = alles in Ordnung). */
int GameLoadConfigs(void) {
    CfgInit();
    if (!CfgLoad(CONFIG_FILE)) CfgSave(CONFIG_FILE);          /* fehlt game.conf: mit Standardwerten anlegen */
    int errors = LoadItemConfigs(".");
    errors += LoadMonsterConf(".");
    errors += LoadMapsDat(".");
    return errors;
}

static void ApplyDisplaySettings(int force) {
    static int lastVsync = -1, lastFull = -1, lastFps = -1;
    int vs = cfg.vsync != 0, fu = cfg.fullscreen != 0, fp = (int)cfg.max_fps;
    if (force || vs != lastVsync) { SetVSync(vs); lastVsync = vs; }
    if (force || fu != lastFull)  { if (!(force && !fu)) SetFullscreen(fu); lastFull = fu; }
    if (force || fp != lastFps)   { SetFrameLimit(fp); lastFps = fp; }
    MapRenderDistance = (int)cfg.render_distance;
}

static void HandleKey(int key, int *quit) {
    if (key == 0) return;
    Hero *A = heroes[activeHero].isHero ? &heroes[activeHero] : NULL;
    if (key == 27) {                          /* ESC: erst zurück zum Raster, dann beenden */
        if (screen == SCR_GRID) *quit = 1; else { screen = SCR_GRID; pickClass = 0; }
        return;
    }
    if (key == '9') screen = SCR_GRID;
    if (key == 'm') screen = (screen == SCR_MENU) ? SCR_GRID : SCR_MENU;
    if (key == 'g') CollectAllChests();
    int hk = (key == 'y') ? 0 : (key == 'x') ? 1 : (key == 'c') ? 2 : (key == 'v') ? 3 : (key == 'b') ? 4 : -1;
    if (hk >= 0 && heroes[hk].isHero) { activeHero = hk; screen = SCR_HERO; }
    if (screen == SCR_HERO && A) {
        if (key == '.') activeHero = NextHeroAfter(activeHero, 1);
        if (key == ',') activeHero = NextHeroAfter(activeHero, -1);
        if (key >= '1' && key <= '3') HeroUsePotion(activeHero, key - '1');
        if (key == 'q') HeroUseSkill(activeHero, 0);
        if (key == 'w') HeroUseSkill(activeHero, 1);
        if (key == 'e') HeroUseSkill(activeHero, 2);
        if (key == 'r') HeroUseSkill(activeHero, 3);
        if (key == 'a') { A->autopilotEnabled = !A->autopilotEnabled; A->manualTimer = 0; }
        if (key == 'f') { int n; CollectChests(activeHero, &n); Toast("Kisten von %s eingesammelt", A->name); }
    }
#if DEBUG
    if (key == '0') for (int i = 0; i < MAX_HEROES; i++) if (instances[i].used) RestartInstance(i);
    if (key == 'u') { for (int s = 0; s < StageCount; s++) player.cleared[s] = 1; Toast("DEBUG: alle Karten freigeschaltet"); }
    if (key == 'k') {
        player.gold += 1e6; player.level += 5; if (player.level > 100) player.level = 100;
        for (int t = 0; t < TIER_COUNT; t++) for (int k = 0; k < MAT_COUNT; k++) { Item m = ItemMakeMaterial(t, k, 200); InvAdd(&m); }
        Toast("DEBUG: Gold, Material, Spieler-Level");
    }
    if (key == 'l' && A) { HeroGainXp(activeHero, XpNeeded(A->level)); Toast("DEBUG: Level-Up"); }
#endif
}

void GameWindowSize(int *w, int *h) { *w = (int)cfg.window_width; *h = (int)cfg.window_height; }

static int editorMode = 0;
void GameSetEditor(int on) { editorMode = on; }

#include "editor.c"

void GameMain(void) {
    engineStep = 0;
    InitMapTiles();
    renderHeight = (int)cfg.render_height;
    if (renderHeight < 200) renderHeight = 200;
    ApplyDisplaySettings(1);
    renderWidth = renderHeight * 16 / 9;
    scale = (double)renderHeight / 1080.0;

    int Canvas = CreateImage(renderWidth, renderHeight);
    int heroCanvas[4];
    if (Canvas == 0) return;
    for (int i = 0; i < 4; i++) { heroCanvas[i] = CreateImage(renderWidth, renderHeight); if (!heroCanvas[i]) return; }
    if (LoadTextures() == 0) return;

    srand((unsigned)time(NULL));
    if (editorMode) { EditorMain(Canvas); return; }
    if (!LoadGame()) NewGame();
    if (HeroCount() == 0) NewGame();
    SyncLayouts();
    activeHero = FirstHero();
    selHero = activeHero;
    screen = SCR_GRID;
    int quit = 0;
    double simAcc = 0;
    time_t lastSave = time(NULL);

    /* Test-Hooks (Umgebungsvariablen) */
    if (getenv("PANOPTYCUS_SIM")) {                       /* nur simulieren, dann Statistik ausgeben */
        long frames = atol(getenv("PANOPTYCUS_SIM"));
        if (getenv("PANOPTYCUS_SIM_HEROES")) {
            int n = atoi(getenv("PANOPTYCUS_SIM_HEROES"));
            player.level = 100;
            for (int c = 1; c < n && c < CLASS_COUNT; c++) { int h = CreateHero(c); if (h >= 0) StartStage(h, 0, -1); }
        }
        for (long f = 0; f < frames; f++) {
            SimStep(); engineStep++;
            if (f % 36000 == 35999 || f == frames - 1) {
                printf("-- %ld min --  Spieler Lv %d, Gold %.0f, Kills %ld, geschafft:", (f + 1) / 3600, player.level, player.gold, player.totalKills);
                int cl = 0; for (int s = 0; s < StageCount; s++) cl += player.cleared[s]; printf(" %d\n", cl);
                for (int h = 0; h < MAX_HEROES; h++) if (heroes[h].isHero)
                    printf("  %-14s Lv %3d  Karte %-24s Kisten %2d  Kills %ld  HP %.0f/%.0f\n", heroes[h].name, heroes[h].level,
                           heroes[h].stage >= 0 ? Stages[heroes[h].stage].name : "-", HeroChestTotal(h), heroes[h].kills, heroes[h].hp, heroes[h].hpMax);
                if (getenv("PANOPTYCUS_SIM_VERBOSE")) for (int h = 0; h < MAX_HEROES; h++) if (heroes[h].isHero && heroes[h].inst >= 0) {
                    int alive = 0; for (int m = 0; m < MAX_MONSTERS; m++) alive += instances[heroes[h].inst].mon[m].alive;
                    printf("    %s pos %.0f,%.0f wp %d/%d alive %d state %d walking %d retreat %d atk %d skill %d\n", heroes[h].name, heroes[h].x, heroes[h].y,
                           heroes[h].waypoint, Templates[Stages[heroes[h].stage].tmpl].nWay, alive, instances[heroes[h].inst].state, heroes[h].walking, heroes[h].retreat, heroes[h].attacking, heroes[h].skilling);
                }
            }
        }
        return;
    }
    if (getenv("PANOPTYCUS_SCREEN")) {
        const char *sc = getenv("PANOPTYCUS_SCREEN");
        if (!strcmp(sc, "hero")) screen = SCR_HERO;
        else if (sc[0] == 'm') { screen = SCR_MENU; menuTab = atoi(sc + 1); }
        if (getenv("PANOPTYCUS_DEBUGKEYS")) for (const char *k = getenv("PANOPTYCUS_DEBUGKEYS"); *k; k++) HandleKey(*k, &quit);
    }

    while (!ShouldQuit() && !quit) {
        /* ---- Eingabe (logische Koordinaten 1920x1080) ---- */
        int ww = WindowWidth(), wh = WindowHeight();
        double sx = (double)ww / ((double)wh * renderWidth / renderHeight);
        double sy = (double)wh / ((double)ww * renderHeight / renderWidth);
        if (sy < sx) sy = 1; else sx = 1;
        int rw = (int)((double)ww / sx), rh = (int)((double)wh / sy);
        int rx = (ww - rw) / 2, ry = (wh - rh) / 2;
        int mx = GetMouseX(), my = GetMouseY();
        uiMX = mx < 0 ? -1000 : (int)((double)(mx - rx) / rw * 1920);
        uiMY = my < 0 ? -1000 : (int)((double)(my - ry) / rh * 1080);
        uiClick = GetMousePressed(MOUSEBUTTON_Left) != 0;
        uiRClick = GetMousePressed(MOUSEBUTTON_Right) != 0;
        uiDown = GetMouseIsPressed(MOUSEBUTTON_Left) != 0;
        uiWheel = GetMouseWheel();
        uiClickUsed = 0;
        uiTipItem = NULL;
        int key = GetKey();
        UiFrameInput(&key);
        HandleKey(key, &quit);

        /* ---- Simulation ---- */
        ApplyDisplaySettings(0);
        double gs = cfg.gamespeed; if (gs < 0) gs = 0; if (gs > 16) gs = 16;
        simAcc += gs;
        for (int st = 0; simAcc >= 1.0 && st < 16; st++, simAcc -= 1.0) {
            SimStep();
            if (++player.shopTimer >= RestockFrames()) ShopRestock();
        }
        if (simAcc > 2.0) simAcc = 2.0;
        if (time(NULL) - lastSave >= (time_t)cfg.autosave_seconds) { SaveGame(); lastSave = time(NULL); }
        SyncLayouts();
        if (!heroes[activeHero].isHero) activeHero = FirstHero();

        /* ---- Zeichnen ---- */
        DrawOutput(Canvas);
        if (screen == SCR_HERO && heroes[activeHero].isHero) {
            DrawHeroView(Canvas, activeHero, 1.0);
            DrawOutput(Canvas);
            HeroScreenInput(activeHero);
        } else if (screen == SCR_GRID) {
            DrawImageScaled(CanvasBG, 0, 0, 1920, 1080);
            GridScreen(heroCanvas, Canvas);
            DrawNavBar(1);
        } else {
            MenuScreen();
            DrawNavBar(1);
        }
        DrawToast();
        UiDrawTooltip();

        /* ---- auf das Fenster skalieren ---- */
        DrawOutput(0);
        DrawImage(VoidBG, 0, 0, ww, wh);
        DrawImage(Canvas, rx, ry, rw, rh);
        if (getenv("PANOPTYCUS_SHOT") && engineStep == atoi(getenv("PANOPTYCUS_SHOT"))) {
            SaveScreenshot(getenv("PANOPTYCUS_SHOT_FILE") ? getenv("PANOPTYCUS_SHOT_FILE") : "shot.bmp");
        }
        FlipBuffers();
        engineStep++;
    }
    SaveGame();
}
