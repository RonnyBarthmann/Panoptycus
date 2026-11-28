#ifndef GAME_H
#define GAME_H

/* Enthält die komplette Spiellogik inkl. Hauptschleife.
 * Kehrt zurück, sobald das Spiel beendet werden soll. */
void GameMain(void);

/* 1 = Savegame-Editor statt Spiel starten (Kommandozeile: --editor) */
void GameSetEditor(int on);

#endif /* GAME_H */
