#ifndef CONFIG_H
#define CONFIG_H

/* ------------------------------------------------------------------
 * Konfigurationsdateien
 *
 *  game.conf      Einstellungen und Balancing-Faktoren (name = wert).
 *                 Einstellungen mit sinnvollem Bereich haben zusätzlich
 *                 name_min / name_max.
 *  strength.conf  Stärke-Tabellen für Waffen/Rüstung, Klassenwerte
 *  tiers.conf     Werte pro Stufe (Normal ... Legendär)
 *  monsters.conf  Monstertypen
 *  maps.dat       Kapitel, Karten, Tempel, Ruinen
 *
 * Alle Dateien: "# Kommentar", "name = wert", "[abschnitt]".
 * ------------------------------------------------------------------ */

/* ---- generischer Parser ---- */

typedef struct {
    int   sec;          /* laufende Nummer des Abschnitts (0 = vor dem ersten) */
    int   line;
    char  key[48];
    char *val;
} ConfEntry;

typedef struct {
    char       path[128];
    ConfEntry *e;
    int        n, cap;
    int        nsec;                 /* Anzahl Abschnitte (ohne Abschnitt 0)  */
    char       secName[256][24];     /* Name je Abschnitt (Index = sec)       */
} ConfDoc;

extern int confErrors;               /* Anzahl gemeldeter Fehler               */

int         ConfLoad(ConfDoc *d, const char *path);       /* 0 = Datei fehlt  */
void        ConfFree(ConfDoc *d);
int         ConfSectionCount(const ConfDoc *d, const char *name);
int         ConfSection(const ConfDoc *d, const char *name, int k);   /* sec-ID des k-ten Abschnitts */
void        ConfError(const ConfDoc *d, int line, const char *fmt, ...);
const char *ConfStr(ConfDoc *d, int sec, const char *key, int required);
double      ConfNum(ConfDoc *d, int sec, const char *key);            /* Pflichtwert */
double      ConfNumOpt(ConfDoc *d, int sec, const char *key, double def);
int         ConfNums(ConfDoc *d, int sec, const char *key, double *out, int max, int required);
int         ConfNext(const ConfDoc *d, int sec, const char *key, int *iter, const char **val, int *line);
int         ConfParseNums(const char *s, double *out, int max);

/* ---- game.conf ---- */

/* X(name, default, min, max, ranged, gruppe, beschreibung)
 * ranged = 1: Einstellung für Spieler mit sinnvollem Bereich (name_min / name_max). */
#define CFG_LIST(X) \
 X(window_width,        1280, 320, 7680, 0, "Fenster und Anzeige", "Startbreite des Fensters in Pixeln") \
 X(window_height,        720, 200, 4320, 0, "Fenster und Anzeige", "Starthöhe des Fensters in Pixeln") \
 X(fullscreen,             0,   0,    1, 1, "Fenster und Anzeige", "Vollbild (0 = Fenster, 1 = Vollbild)") \
 X(render_height,       1080, 360, 2160, 1, "Fenster und Anzeige", "Interne Render-Auflösung (Höhe in Pixeln, Breite = 16:9)") \
 X(render_distance,     1024, 256, 4096, 1, "Fenster und Anzeige", "Sichtweite der Kartenkacheln") \
 X(vsync,                  1,   0,    1, 1, "Fenster und Anzeige", "Vertikale Synchronisation (0 = aus, 1 = an)") \
 X(max_fps,               60,   0,  360, 1, "Fenster und Anzeige", "Maximale Bildrate (0 = unbegrenzt)") \
 X(floating_text,          1,   0,    1, 1, "Fenster und Anzeige", "Schadenszahlen anzeigen (0 = aus, 1 = an)") \
 X(gamespeed,              1, 0.1,    4, 1, "Spielablauf", "Spielgeschwindigkeit (1 = normal, kleiner als 1 = Zeitlupe)") \
 X(autosave_seconds,      30,   5,  600, 1, "Spielablauf", "Automatisches Speichern alle x Sekunden") \
 X(start_gold,           150,   0, 1e12, 0, "Spielablauf", "Gold bei einem neuen Spiel") \
 X(start_material,        20,   0, 9999, 0, "Spielablauf", "Baumaterial jeder Art (Stufe Normal) bei einem neuen Spiel") \
 X(hero_slots_start,       4,   1,   16, 0, "Spielablauf", "Heldenplätze bei Spieler-Level 1") \
 X(hero_slots_max,        16,   1,   16, 0, "Spielablauf", "Heldenplätze bei Spieler-Level 100 (höchstens 16)") \
 X(power_base,             5,   0, 1e12, 0, "Progression", "Grundwert der Stärke-Kurve bei Level 0 (PowerCurve = base * 10^(exp * Level / 100))") \
 X(power_exp,        5.69897,   0,   20, 0, "Progression", "Steilheit der Stärke-Kurve (5.7 = 5 bei Level 0, 2.5 Mio bei Level 100)") \
 X(monster_hp_factor,      6,   0, 1e12, 0, "Progression", "Monster-Leben = Faktor * Stärke-Kurve(Map-Level)") \
 X(monster_atk_factor,   0.5,   0, 1e12, 0, "Progression", "Monster-Angriff = Faktor * Stärke-Kurve(Map-Level)") \
 X(monster_armor_factor,   4,   0, 1e12, 0, "Progression", "Monster-Rüstung = Faktor * Stärke-Kurve(Map-Level)") \
 X(monster_count_per_level, 0.01, 0, 10, 0, "Progression", "Zusätzliche Monster je Spawnpunkt und Map-Level (0.01 = bei Level 100 doppelt so viele)") \
 X(gold_per_kill_factor, 0.6,   0, 1e12, 0, "Progression", "Gold pro Kill = Faktor * Stärke-Kurve(Monster-Level) * Gold-Faktor der Monsterstufe") \
 X(stage_bonus_first,     60,   0, 1e9, 0, "Progression", "Gold-Bonus beim ersten Abschluss einer Karte (Vielfaches des Kill-Golds)") \
 X(stage_bonus_repeat,    15,   0, 1e9, 0, "Progression", "Gold-Bonus bei jedem weiteren Abschluss einer Karte") \
 X(hero_xp_base,         100,   0, 1e12, 0, "Progression", "Erfahrung für Held-Level 1: base * 10^(growth * Level + curve * (Level/100)^2)") \
 X(hero_xp_growth,      0.05,   0,    1, 0, "Progression", "Lineares Wachstum der Heldenerfahrung (Exponent je Level)") \
 X(hero_xp_curve,        3.3,   0,   20, 0, "Progression", "Zusätzliche Verteuerung hoher Held-Level (quadratischer Exponent)") \
 X(kill_xp_base,           4,   0, 1e12, 0, "Progression", "Erfahrung je Kill = base * 10^(growth * Monster-Level)") \
 X(kill_xp_growth,      0.05,   0,    1, 0, "Progression", "Wachstum der Kill-Erfahrung je Monster-Level") \
 X(player_xp_base,       300,   0, 1e12, 0, "Progression", "Erfahrung für Spieler-Level 1: base * 10^(growth * Level)") \
 X(player_xp_growth,    0.05,   0,    1, 0, "Progression", "Wachstum der Spieler-Erfahrung je Level") \
 X(player_xp_share,        1,   0,  100, 0, "Progression", "Anteil der Kill-Erfahrung, den der Spieler zusätzlich erhält (1 = gleich viel wie der Held)") \
 X(hero_hp_base,         150,   0, 1e12, 0, "Helden", "Leben eines Helden bei Level 0 (mal Klassenfaktor)") \
 X(hero_hp_growth,       4.6,   0,   20, 0, "Helden", "Lebenswachstum: 10^(growth * Level / 100)") \
 X(hero_mana_base,       100,   0, 1e12, 0, "Helden", "Mana bei Level 0 (mal Klassenfaktor)") \
 X(hero_mana_growth,     1.2,   0,   20, 0, "Helden", "Manawachstum: 10^(growth * Level / 100)") \
 X(hero_atk_base,          3,   0, 1e12, 0, "Helden", "Angriff bei Level 0 (mal Klassenfaktor)") \
 X(hero_atk_growth,        3,   0,   20, 0, "Helden", "Angriffswachstum: 10^(growth * Level / 100)") \
 X(hero_aggro_range,     720,   0, 1e5, 0, "Helden", "Entfernung, ab der der Autopilot Gegner angreift") \
 X(hero_retreat_hp,     0.22,   0,    1, 0, "Helden", "Autopilot zieht sich zurück, wenn das Leben darunter fällt (Anteil) und kein Heiltrank da ist") \
 X(hero_retreat_resume_hp, 0.6, 0,    1, 0, "Helden", "Autopilot kämpft wieder, wenn das Leben darüber liegt (Anteil)") \
 X(party_hp_mul,         1.6,   0, 1e6, 0, "Karten", "Monster-Leben-Faktor auf Karten für zwei Helden") \
 X(temple_hp_mul,          6,   0, 1e6, 0, "Karten", "Tempel: Monster-Leben-Faktor (zähe Gegner)") \
 X(temple_atk_mul,       0.5,   0, 1e6, 0, "Karten", "Tempel: Monster-Angriff-Faktor") \
 X(temple_xp_mul,        1.6,   0, 1e6, 0, "Karten", "Tempel: Erfahrungs-Faktor") \
 X(ruins_hp_mul,         0.6,   0, 1e6, 0, "Karten", "Ruinen: Monster-Leben-Faktor") \
 X(ruins_atk_mul,          5,   0, 1e6, 0, "Karten", "Ruinen: Monster-Angriff-Faktor (harte Gegner)") \
 X(ruins_xp_mul,         1.6,   0, 1e6, 0, "Karten", "Ruinen: Erfahrungs-Faktor") \
 X(miniboss_tier,          2,   0,    5, 0, "Monster", "Stufe der Zwischenbosse (0 = Normal ... 5 = Legendär)") \
 X(boss_tier,              4,   0,    5, 0, "Monster", "Stufe der Endbosse") \
 X(miniboss_size,        1.3,   0,   10, 0, "Monster", "Größenfaktor Zwischenboss") \
 X(boss_size,            1.7,   0,   10, 0, "Monster", "Größenfaktor Endboss") \
 X(boss_speed_mul,       0.9,   0,   10, 0, "Monster", "Tempofaktor Endboss") \
 X(monster_aggro,        900,   0, 1e5, 0, "Monster", "Aggro-Reichweite normaler Monster") \
 X(boss_aggro,          1200,   0, 1e5, 0, "Monster", "Aggro-Reichweite von Bossen und Zwischenbossen") \
 X(chest_base_capacity,   10,   0, 1e6, 0, "Kisten und Items", "Kistenkapazität eines Helden ohne Ausrüstung") \
 X(chest_item_percent,    15,   0,  100, 0, "Kisten und Items", "Wahrscheinlichkeit (%), dass eine Kiste ein Item statt Material enthält") \
 X(chest_wings_den,      400,   1, 1e9, 0, "Kisten und Items", "Flügel: 1 von x Kisten (extrem selten)") \
 X(level_skip_chance,     20,   0,  100, 0, "Kisten und Items", "Chance (%) beim Verbessern von Level 19 direkt auf 21 zu springen") \
 X(sell_factor,         0.35,   0,  100, 0, "Kisten und Items", "Verkaufswert = Basiswert * Faktor") \
 X(craft_gold_factor,    0.5,   0,  100, 0, "Kisten und Items", "Gold-Kosten beim Herstellen (Anteil des Basiswerts)") \
 X(upgrade_gold_factor,  0.5,   0,  100, 0, "Kisten und Items", "Gold-Kosten beim Verbessern (Anteil des Basiswerts)") \
 X(upgrade_gold_growth, 1.35,   1,  100, 0, "Kisten und Items", "Verteuerung je Item-Level beim Verbessern") \
 X(loot_w_weapon,         20,   0, 1e6, 0, "Kisten und Items", "Gewicht Waffen bei Item-Drops") \
 X(loot_w_offhand,         8,   0, 1e6, 0, "Kisten und Items", "Gewicht Sekundärwaffen") \
 X(loot_w_helm,            8,   0, 1e6, 0, "Kisten und Items", "Gewicht Helme") \
 X(loot_w_chest,           8,   0, 1e6, 0, "Kisten und Items", "Gewicht Brustpanzer") \
 X(loot_w_gloves,          8,   0, 1e6, 0, "Kisten und Items", "Gewicht Handschuhe") \
 X(loot_w_boots,           8,   0, 1e6, 0, "Kisten und Items", "Gewicht Stiefel") \
 X(loot_w_bag,             6,   0, 1e6, 0, "Kisten und Items", "Gewicht Tragetaschen") \
 X(loot_w_potion,         14,   0, 1e6, 0, "Kisten und Items", "Gewicht Tränke") \
 X(loot_w_ring,           10,   0, 1e6, 0, "Kisten und Items", "Gewicht Ringe") \
 X(loot_w_necklace,        5,   0, 1e6, 0, "Kisten und Items", "Gewicht Halsketten (halb so oft wie Ringe)") \
 X(shop_price_factor,     40,   0, 1e6, 0, "Shop", "Einkaufspreis Material = Verkaufswert * Faktor * Tier-Wachstum^Stufe") \
 X(shop_price_tier_growth, 2,   1,  100, 0, "Shop", "Preisanstieg je Stufe im Shop") \
 X(death_cooldown_seconds, 300, 0, 1e6, 0, "Helden", "Sekunden Wartezeit nach dem Tod eines Helden (bzw. der Gruppe), bis die Karte neu startet") \
 X(shop_restock_seconds, 600,   1, 1e6, 0, "Shop", "Sekunden bis zum nächsten Nachschub")

typedef struct {
#define X(n, d, mi, ma, r, g, desc) double n;
    CFG_LIST(X)
#undef X
} Cfg;

typedef struct {
    const char *name; double *val; double def, min, max; int ranged; const char *group, *desc;
    double userMin, userMax;      /* name_min / name_max (nur ranged) */
} CfgDef;

extern Cfg    cfg;
extern CfgDef cfgDefs[];
extern int    cfgCount;

void CfgInit(void);                                  /* Standardwerte */
int  CfgLoad(const char *path);                      /* 0 = Datei fehlt (Standardwerte bleiben) */
int  CfgSave(const char *path);                      /* schreibt game.conf mit Kommentaren */
double CfgClampUser(const CfgDef *d, double v);      /* auf name_min..name_max begrenzen */

#endif /* CONFIG_H */
