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
- 10 Kapitel à 5 Karten, dazu "Tempel" und "Ruinen" (jeweils auf der 5. Originalkarte)
- Ansichten: Vollbild pro Held, 2x2-Raster mit 4 speicherbaren Anordnungen
- Balancing aus `Strength.ods` (Y3:AU27), logarithmisch

## Tasten

- `ESC` Menü/zurück, `9` Raster, `m` Menü, `g` alle Kisten einsammeln, `f` Kisten des Helden
- `y x c v b` Held 1-5, `.` / `,` nächster/voriger Held
- `1-3` Tränke, `q w e r` Skills, `a` Autopilot
- Maus: Klick = laufen (halten = weiterlaufen), Klick auf Monster = angreifen
- Debug (`PANOPTYCUS_DEBUGKEYS=uk`): `0` Neustart, `u` Spielerlevel, `k` Gold/Material, `l` Level

Spielstand: `savegame.dat` (wird automatisch geschrieben).

---------------------------------------------------------------

### Author:

Ronny Barthmann (Germany/Schulzendorf)

### License:

GNU General Public License v3.0 or later
