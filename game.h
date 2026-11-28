#ifndef GAME_H
#define GAME_H

/* Enthält die komplette Spiellogik inkl. Hauptschleife.
 * Kehrt zurück, sobald das Spiel beendet werden soll. */
void GameMain(void);

/* Lädt game.conf, strength.conf, tiers.conf, monsters.conf, maps.dat. Rückgabe: Anzahl Fehler. */
int GameLoadConfigs(void);
/* Fenstergröße aus game.conf (gültig nach GameLoadConfigs) */
void GameWindowSize(int *w, int *h);

/* 1 = Savegame-Editor statt Spiel starten (Kommandozeile: --editor) */
void GameSetEditor(int on);

#endif /* GAME_H */
