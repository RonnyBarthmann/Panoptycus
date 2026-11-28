# ATTENTION - this fork from IdleManager is HEAVLY AI-Coded (vibe-coded based on a non-vibe-coded base)

# Panoptycus – Idle-RPG-Manager (SDL2, C)

Idle-Spiel im Stil von "Panoptyca": mehrere Helden laufen autonom (Autopilot) durch Karten, besiegen Monster, sammeln Kisten und werden durch Ausrüstung, Handwerk und Tränke stärker.

Bauen: `make` (benötigt SDL2, SDL2_image). Tests: `make -C tests` bzw. `tests/test_items.c`.

## Inhalt

- 7 Klassen (Krieger, Magier, Bogenschütze, Assassine, Ritter, Schamane, Priester), je 6 Skills (4 belegbar), je 24 Waffen; 96 Rüstungsteile
- 4 Heldenplätze, wachsend bis 16 mit dem Spielerlevel; Helden- und Spieler-XP
- 6 Seltenheitsstufen mit durchgehendem Farbschema (grau, blau, gelb, orange, rot, rosa)
- Kisten pro Kill (1/10 … 1/1M), Tragekapazität 10 + bis 60 durch Beutel/Brustrüstung
- 14 Ausrüstungsplätze, Tränke (dauerhaft, laden sich bei Kills auf), Ringe, Amulette, Flügel
- Handwerk, Item-Verbesserung (Stufe 0–21, 19→21 per Zufallssprung), Shop (nur Material)
- Kapitel und Karten aus `maps.dat` (Standard: 10 Kapitel à 5 Karten), dazu "Tempel" und "Ruinen" (jeweils auf der 5. Originalkarte)
- Ansichten: Vollbild pro Held, 2x2-Raster mit 4 speicherbaren Anordnungen
- Balancing aus `Strength.ods` (Y3:AU27), logarithmisch

## Tasten

- `ESC` Menü/zurück, `9` Raster, `m` Menü, `g` alle Kisten einsammeln, `f` Kisten des Helden
- `y x c v b` Held 1-5, `.` / `,` nächster/voriger Held
- `1-3` Tränke, `q w e r` Skills, `a` Autopilot
- Maus: Klick = laufen (halten = weiterlaufen), Klick auf Monster = angreifen
- Debug (`PANOPTYCUS_DEBUGKEYS=uk`): `0` Neustart, `u` Spielerlevel, `k` Gold/Material, `l` Level

## Konfiguration

Alle Balancing-Werte liegen in lesbaren Textdateien neben dem Spiel (`#` = Kommentar, `key = wert`). Fehlerhafte Pflichtdateien werden beim Start als Liste gemeldet.

- `game.conf` – Fenster, Bildrate, VSync, Spielgeschwindigkeit (auch <1 = Zeitlupe), Fortschritt, Monster-/Helden-Wachstum, XP, Gold, Tempel/Ruinen, Kisten, Shop. Fehlt die Datei, wird sie mit Standardwerten angelegt. Einstellungen mit `name_min`/`name_max` werden im Spiel auf diesen Bereich begrenzt; Werte außerhalb aus der Datei bleiben bis zur ersten Änderung in der Ansicht "Einstellungen" erhalten.
- `strength.conf` – Rechenwerte für Waffen-/Rüstungsstärke.
- `tiers.conf` – Stärke je Stufe (normal … legendär), welche Waffenlevel es je Stufe gibt, stufenabhängige Monstervariablen. Die Zahl der Stufen ist fest.
- `monsters.conf` – Monstertypen.
- `maps.dat` – Kartenvorlagen, Kapitel (Level erste/letzte Karte, linear skaliert), Tempel und Ruinen. Anzahl der Karten, Kapitel, Tempel und Ruinen ist frei wählbar. Achtung: Ändert sich die Kartenzahl, verschieben sich Karten-Indizes alter Spielstände.

Bedienung: Zahlenfelder per Klick + Tippen; `+`/`-` und Mausrad ändern den Wert sofort. Namen von Helden sind im Spiel editierbar. Lange Listen haben Scrollbalken und Mausrad-Scrolling. Ansicht "Einstellungen" im Menü für Anzeige und Spielgeschwindigkeit; Änderung von `render_height` wirkt nach Neustart.

## Savegame-Editor

Start mit `./game --editor`. Der Editor bearbeitet `savegame.dat` (vor dem ersten Speichern entsteht `savegame.dat.bak`). Das Spiel sollte dabei nicht laufen, es überschreibt den Spielstand regelmäßig.

- **Helden:** alle 16 Heldenplätze mit ihrer Ausrüstung. Klick auf das Klassensymbol öffnet die Held-Daten (Level, EP, Name, Karte, Kisten, Skills, ...).
- **Spieler:** das Inventar mit 168 Slots. Das Symbol "Spieler-Daten" öffnet Gold, Level, EP, Anordnungen, geschaffte Karten, Shop-Vorrat und Timer.
- **Editor-Slot** (rechts): Kategorie, Stufe, Art, Nummer, Item-Level bzw. Anzahl einstellen. Linke Maustaste halten und über Slots fahren setzt das Item in jeden Slot, der es annimmt (Helm nur in Helm-Slots). Rechtsklick auf einen Slot kopiert ihn in den Editor-Slot.
- Zahlenfelder: anklicken, tippen, Enter. `-`/`+` ändern um 1.

Spielstand: `savegame.dat` (wird automatisch geschrieben).

---------------------------------------------------------------

### Author:

Ronny Barthmann (Germany/Schulzendorf)

### License:

GNU General Public License v3.0 or later
