/* Spieldaten: Skills, Monstertypen, Kartenvorlagen, Kapitel/Tempel/Ruinen.
 * (wird von game.c eingebunden) */

/* ================================================================== */
/* Skills                                                             */
/* ================================================================== */
/*  name, desc, cls, unlock, type, cd, n, visual, dur, buff, manaPct, mult, radius, angle, speed, pct */
#define SK_BUFF(nm, ds, cl, ul, cd, dur, bf, mana, pct) \
    { nm, ds, cl, ul, SKT_BUFF, cd, 0, 0, dur, bf, mana, 0, 0, 0, 0, pct }

static const SkillDef SkillDefs[CLASS_COUNT * SKILLS_PER_CLASS] = {
    /* ---- Ritter ---- */
    { "Schildhieb",    "Breiter Hieb vor dir",                 CLASS_Knight, 1,  SKT_CONE,   90, 0, 0, 0, 0,          4, 2.0, 160, 110, 0, 0 },
    { "Wirbelwind",    "Schlag in alle Richtungen",            CLASS_Knight, 5,  SKT_NOVA,  150, 0, 0, 0, 0,          8, 1.8, 190, 360, 0, 0 },
    { "Sturmangriff",  "Stürmt nach vorn und trifft alles",    CLASS_Knight, 12, SKT_DASH,  240, 0, 0, 0, 0,         10, 2.5, 400, 0, 0, 0 },
    SK_BUFF("Eiserne Haut", "+100% Rüstung", CLASS_Knight, 20, 900, 480, BUFF_ARMOR, 12, 100),
    SK_BUFF("Kriegsruf", "+50% Angriff", CLASS_Knight, 35, 1200, 600, BUFF_ATTACK, 15, 50),
    { "Erdbeben",      "Gewaltige Erschütterung",              CLASS_Knight, 55, SKT_NOVA,  600, 0, 0, 0, 0,         25, 4.0, 340, 360, 0, 0 },
    /* ---- Magier ---- (die ersten vier entsprechen den bisherigen Q/W/E/R-Skills) */
    { "Arkansalve",    "5 Geschosse im Fächer",                CLASS_Mage, 1,  SKT_FAN,     30, 5, PROJ_ARCANE, 0, 0, 10, 0.8, 0, 40, 10, 0 },
    { "Arkanring",     "36 Geschosse in alle Richtungen",      CLASS_Mage, 5,  SKT_CIRCLE,  60, 36, PROJ_ARCANE, 0, 0, 30, 0.6, 0, 360, 10, 0 },
    { "Arkanstrom",    "Dauerfeuer mit Streuung",              CLASS_Mage, 12, SKT_CHANNEL, 120, 20, PROJ_ARCANE, 60, 0, 15, 0.5, 0, 20, 10, 0 },
    { "Schrotflinte",  "49 Geschosse im weiten Fächer",        CLASS_Mage, 20, SKT_FAN,     90, 49, PROJ_ARCANE, 0, 0, 40, 0.5, 0, 48, 10, 0 },
    { "Blitzkette",    "Blitz springt auf 6 Gegner",           CLASS_Mage, 35, SKT_CHAIN,  180, 6, 0, 0, 0,          20, 3.0, 450, 0, 0, 0 },
    { "Meteorregen",   "Meteore schlagen im Umkreis ein",      CLASS_Mage, 55, SKT_RAIN,   480, 12, 0, 0, 0,         40, 4.0, 450, 0, 0, 0 },
    /* ---- Bogenschütze ---- */
    { "Pfeilfächer",   "5 Pfeile im Fächer",                   CLASS_Archer, 1,  SKT_FAN,     30, 5, PROJ_ARROW, 0, 0, 8, 0.8, 0, 40, 20, 0 },
    { "Pfeilkreis",    "36 Pfeile in alle Richtungen",         CLASS_Archer, 5,  SKT_CIRCLE,  60, 36, PROJ_ARROW, 0, 0, 25, 0.6, 0, 360, 20, 0 },
    { "Pfeilhagel",    "Dauerfeuer mit Streuung",              CLASS_Archer, 12, SKT_CHANNEL, 120, 20, PROJ_ARROW, 60, 0, 15, 0.5, 0, 20, 20, 0 },
    { "Schrotpfeil",   "49 Pfeile im weiten Fächer",           CLASS_Archer, 20, SKT_FAN,     90, 49, PROJ_ARROW, 0, 0, 40, 0.5, 0, 48, 20, 0 },
    { "Giftpfeile",    "3 Giftpfeile, Gift wirkt über Zeit",   CLASS_Archer, 35, SKT_FAN,    150, 3, PROJ_VENOM, 0, 0, 15, 1.0, 0, 16, 18, 60 },
    { "Pfeilregen",    "Pfeile prasseln im Umkreis nieder",    CLASS_Archer, 55, SKT_RAIN,   480, 14, 0, 0, 0,       40, 3.5, 480, 0, 0, 0 },
    /* ---- Krieger ---- */
    { "Wuchtschlag",   "Mächtiger Schlag vor dir",             CLASS_Warrior, 1,  SKT_CONE,  100, 0, 0, 0, 0,        5, 2.5, 180, 140, 0, 0 },
    { "Kreissäge",     "Wirbelt die Waffe im Kreis",           CLASS_Warrior, 5,  SKT_NOVA,  150, 0, 0, 0, 0,        8, 2.0, 210, 360, 0, 0 },
    { "Axtwurf",       "Durchschlagende Wurfaxt",              CLASS_Warrior, 12, SKT_FAN,   120, 1, PROJ_DAGGER, 0, 0, 8, 3.5, 0, 0, 16, 0 },
    SK_BUFF("Berserker", "+80% Angriff", CLASS_Warrior, 20, 900, 600, BUFF_ATTACK, 12, 80),
    { "Bodenstampfer", "Erschüttert alles in der Nähe",        CLASS_Warrior, 35, SKT_NOVA,  400, 0, 0, 0, 0,       20, 4.0, 320, 360, 0, 0 },
    SK_BUFF("Blutrausch", "+100% Angriffstempo", CLASS_Warrior, 55, 1200, 480, BUFF_ATKSPEED, 20, 100),
    /* ---- Assassine ---- */
    { "Dolchfächer",   "5 Wurfdolche im Fächer",               CLASS_Assassin, 1,  SKT_FAN,   40, 5, PROJ_DAGGER, 0, 0, 6, 0.9, 0, 30, 22, 0 },
    { "Schattenschritt","Blitzschneller Sprung durch Gegner",  CLASS_Assassin, 5,  SKT_DASH, 150, 0, 0, 0, 0,        8, 2.5, 340, 0, 0, 0 },
    { "Giftnebel",     "Vergiftet alle Gegner in der Nähe",    CLASS_Assassin, 12, SKT_POISON, 300, 0, 0, 0, 90,    12, 1.0, 240, 360, 0, 0 },
    { "Klingentanz",   "Schneller Rundumschlag",               CLASS_Assassin, 20, SKT_NOVA,   90, 0, 0, 0, 0,       6, 2.5, 130, 360, 0, 0 },
    SK_BUFF("Rauchbombe", "+60% Laufgeschwindigkeit", CLASS_Assassin, 35, 600, 400, BUFF_SPEED, 8, 60),
    { "Todesstoß",     "Gewaltiger Treffer auf ein Ziel",      CLASS_Assassin, 55, SKT_CHAIN, 240, 1, 0, 0, 0,      15, 8.0, 160, 0, 0, 0 },
    /* ---- Schamane ---- */
    { "Blitzschlag",   "Blitz auf bis zu 3 Gegner",            CLASS_Shaman, 1,  SKT_CHAIN,   80, 3, 0, 0, 0,        6, 1.5, 500, 0, 0, 0 },
    { "Giftwolke",     "Vergiftet Gegner im Umkreis",          CLASS_Shaman, 5,  SKT_POISON, 240, 0, 0, 0, 120,     12, 1.0, 270, 360, 0, 0 },
    { "Donnerruf",     "Blitze schlagen zufällig ein",         CLASS_Shaman, 12, SKT_RAIN,   300, 8, 0, 0, 0,       20, 2.5, 420, 0, 0, 0 },
    { "Geisterheilung","Heilt 25% des max. Lebens",            CLASS_Shaman, 20, SKT_HEAL,   600, 0, 0, 0, 0,       15, 0, 0, 0, 0, 25 },
    SK_BUFF("Sturmgeist", "+50% Angriffstempo", CLASS_Shaman, 35, 900, 500, BUFF_ATKSPEED, 15, 50),
    { "Gewitter",      "Dauerhafter Blitzhagel",               CLASS_Shaman, 55, SKT_RAIN,   600, 20, 0, 0, 0,      40, 2.5, 620, 0, 0, 0 },
    /* ---- Priester ---- */
    { "Heiliger Strahl","3 Lichtgeschosse",                    CLASS_Priest, 1,  SKT_FAN,     45, 3, PROJ_HOLY, 0, 0, 6, 1.2, 0, 20, 14, 0 },
    { "Heilung",       "Heilt 35% des max. Lebens",            CLASS_Priest, 5,  SKT_HEAL,   420, 0, 0, 0, 0,       12, 0, 0, 0, 0, 35 },
    { "Läuterung",     "Lichtnova um den Priester",            CLASS_Priest, 12, SKT_NOVA,   180, 0, 0, 0, 0,       12, 2.5, 260, 360, 0, 0 },
    SK_BUFF("Segen", "+80% Rüstung", CLASS_Priest, 20, 900, 600, BUFF_ARMOR, 12, 80),
    { "Himmelslicht",  "Lichtsäulen im Umkreis",               CLASS_Priest, 35, SKT_RAIN,   420, 6, 0, 0, 0,       25, 3.5, 420, 0, 0, 0 },
    { "Göttlicher Zorn","Gewaltige Lichtexplosion",            CLASS_Priest, 55, SKT_NOVA,   600, 0, 0, 0, 0,       35, 6.0, 420, 360, 0, 0 },
};

#define SKILL_ID(cls, i) ((cls) * SKILLS_PER_CLASS + (i))
#define SKILL_COUNT (CLASS_COUNT * SKILLS_PER_CLASS)

/* ================================================================== */
/* Monster-Typen (monsters.conf)                                      */
/* ================================================================== */

typedef struct { char name[24]; double hp, atk, speed, radius; int ranged; } MonsterInfo;
static MonsterInfo MonsterInfos[MONSTER_TYPES] = { { "-", 1, 1, 0, 32, 0 } };

static int LoadMonsterConf(const char *dir) {
    int before = confErrors;
    char path[256]; ConfDoc d;
    snprintf(path, sizeof path, "%s%smonsters.conf", dir, *dir ? "/" : "");
    if (!ConfLoad(&d, path)) { fprintf(stderr, "Fehler: %s nicht gefunden\n", path); return 1; }
    int n = ConfSectionCount(&d, "monster");
    if (n != MONSTER_TYPES - 1) ConfError(&d, 0, "es müssen genau %d [monster]-Abschnitte vorhanden sein (gefunden: %d)", MONSTER_TYPES - 1, n);
    for (int k = 0; k < MONSTER_TYPES - 1 && k < n; k++) {
        int s = ConfSection(&d, "monster", k);
        MonsterInfo *m = &MonsterInfos[k + 1];
        const char *nm = ConfStr(&d, s, "name", 1);
        snprintf(m->name, sizeof m->name, "%s", nm ? nm : "?");
        m->hp = ConfNum(&d, s, "hp"); m->atk = ConfNum(&d, s, "attack"); m->speed = ConfNum(&d, s, "speed");
        m->radius = ConfNum(&d, s, "radius"); m->ranged = (int)ConfNum(&d, s, "ranged");
    }
    ConfFree(&d);
    return confErrors - before;
}

/* ================================================================== */
/* Kartenvorlagen, Kapitel (maps.dat)                                 */
/* ================================================================== */

#define MAX_CHAPTERS 64

static Template Templates[MAX_TEMPLATES];
static int      TemplateCount = 0;           /* Karten je Kapitel */

static StageDef Stages[MAX_STAGES];
static int      StageCount = 0;

static int  StoryChapters = 0, StoryMapsPerChapter = 0, TempleCount = 0, RuinsCount = 0;
static char ChapterNames[MAX_CHAPTERS][40];
static char TempleNames[MAX_CHAPTERS][40];
static char RuinsNames[MAX_CHAPTERS][40];

static void TplSpawn(Template *t, double x, double y, int role) {
    if (t->nSpawn >= MAX_MONSTERS) return;
    t->spawn[t->nSpawn].x = (int)x; t->spawn[t->nSpawn].y = (int)y; t->spawn[t->nSpawn].role = role;
    t->nSpawn++;
}

/* Rolle am Ende einer Zeile (nach den n Zahlen); Standard normal */
static int ParseRole(ConfDoc *d, int line, const char *val, int nNums) {
    const char *p = val;
    for (int i = 0; i < nNums; i++) {
        while (*p == ' ' || *p == '\t') p++;
        while (*p && *p != ' ' && *p != '\t') p++;
    }
    while (*p == ' ' || *p == '\t') p++;
    if (!*p || !strcmp(p, "normal")) return ROLE_NORMAL;
    if (!strcmp(p, "miniboss")) return ROLE_MINIBOSS;
    if (!strcmp(p, "boss")) return ROLE_BOSS;
    ConfError(d, line, "unbekannte Rolle '%s' (normal, miniboss oder boss)", p);
    return ROLE_NORMAL;
}

static void ParseMap(ConfDoc *d, int sec, Template *t) {
    double a[8];
    int it = 0, line; const char *v;
    int nb = ConfNums(d, sec, "start", a, 3, 1);
    if (nb == 3) { t->startX = (int)a[0]; t->startY = (int)a[1]; t->angle = (int)a[2]; }
    else if (nb) ConfError(d, 0, "start braucht 3 Zahlen: x y winkel");
    /* spawn-Zeilen in Dateireihenfolge, alle Varianten */
    for (int i = 0; i < d->n; i++) {
        ConfEntry *e = &d->e[i];
        if (e->sec != sec) continue;
        int n;
        if (!strcmp(e->key, "spawn")) {
            n = ConfParseNums(e->val, a, 2);
            if (n != 2) { ConfError(d, e->line, "spawn: x y [rolle]"); continue; }
            TplSpawn(t, a[0], a[1], ParseRole(d, e->line, e->val, 2));
        } else if (!strcmp(e->key, "spawn_line")) {
            n = ConfParseNums(e->val, a, 5);
            if (n != 5 || a[4] <= 0) { ConfError(d, e->line, "spawn_line: x1 y1 x2 y2 abstand [rolle]"); continue; }
            int role = ParseRole(d, e->line, e->val, 5);
            double len = hypot(a[2] - a[0], a[3] - a[1]);
            int cnt = (int)(len / a[4] + 1e-6) + 1;
            for (int k = 0; k < cnt; k++) { double f = len > 0 ? k * a[4] / len : 0; TplSpawn(t, a[0] + (a[2] - a[0]) * f, a[1] + (a[3] - a[1]) * f, role); }
        } else if (!strcmp(e->key, "spawn_grid")) {
            n = ConfParseNums(e->val, a, 6);
            if (n != 6 || a[4] <= 0 || a[5] <= 0) { ConfError(d, e->line, "spawn_grid: x1 y1 x2 y2 abstand_x abstand_y [rolle]"); continue; }
            int role = ParseRole(d, e->line, e->val, 6);
            for (double y = a[1]; y <= a[3] + 1e-6; y += a[5])
                for (double x = a[0]; x <= a[2] + 1e-6; x += a[4]) TplSpawn(t, x, y, role);
        } else if (!strcmp(e->key, "waypoint")) {
            n = ConfParseNums(e->val, a, 2);
            if (n != 2) { ConfError(d, e->line, "waypoint: x y"); continue; }
            if (t->nWay >= MAX_WAYPOINTS) { ConfError(d, e->line, "zu viele Wegpunkte (max. %d)", MAX_WAYPOINTS); continue; }
            t->way[t->nWay].x = (int)a[0]; t->way[t->nWay].y = (int)a[1]; t->nWay++;
        }
    }
    (void)it; (void)line; (void)v;
    int bosses = 0; for (int i = 0; i < t->nSpawn; i++) bosses += t->spawn[i].role == ROLE_BOSS;
    if (!bosses) ConfError(d, 0, "eine Karte hat keinen Boss (spawn ... boss)");
}

static int LoadMapsDat(const char *dir) {
    int before = confErrors;
    char path[256]; ConfDoc d;
    snprintf(path, sizeof path, "%s%smaps.dat", dir, *dir ? "/" : "");
    if (!ConfLoad(&d, path)) { fprintf(stderr, "Fehler: %s nicht gefunden\n", path); return 1; }
    memset(Templates, 0, sizeof Templates);
    StageCount = 0;

    TemplateCount = ConfSectionCount(&d, "map");
    StoryChapters = ConfSectionCount(&d, "chapter");
    TempleCount = ConfSectionCount(&d, "temple");
    RuinsCount = ConfSectionCount(&d, "ruins");
    if (TemplateCount < 1 || TemplateCount > MAX_TEMPLATES) { ConfError(&d, 0, "Anzahl [map]: %d (erlaubt 1 bis %d)", TemplateCount, MAX_TEMPLATES); TemplateCount = TemplateCount < 1 ? 1 : MAX_TEMPLATES; }
    if (StoryChapters < 1 || StoryChapters > MAX_CHAPTERS) { ConfError(&d, 0, "Anzahl [chapter]: %d (erlaubt 1 bis %d)", StoryChapters, MAX_CHAPTERS); StoryChapters = StoryChapters < 1 ? 1 : MAX_CHAPTERS; }
    if (TempleCount > MAX_CHAPTERS) TempleCount = MAX_CHAPTERS;
    if (RuinsCount > MAX_CHAPTERS) RuinsCount = MAX_CHAPTERS;
    StoryMapsPerChapter = TemplateCount;
    if (StoryChapters * StoryMapsPerChapter + TempleCount + RuinsCount > MAX_STAGES) {
        ConfError(&d, 0, "zu viele Karten insgesamt (%d, max. %d)", StoryChapters * StoryMapsPerChapter + TempleCount + RuinsCount, MAX_STAGES);
        ConfFree(&d); return confErrors - before;
    }
    for (int m = 0; m < TemplateCount; m++) ParseMap(&d, ConfSection(&d, "map", m), &Templates[m]);

    for (int c = 0; c < StoryChapters; c++) {
        int sec = ConfSection(&d, "chapter", c);
        const char *nm = ConfStr(&d, sec, "name", 1);
        snprintf(ChapterNames[c], sizeof ChapterNames[c], "%.39s", nm ? nm : "?");
        double lo = ConfNum(&d, sec, "level_first"), hi = ConfNum(&d, sec, "level_last");
        int party = (int)ConfNumOpt(&d, sec, "party", 1);
        if (party < 1 || party > 2) { ConfError(&d, 0, "Kapitel %d: party muss 1 oder 2 sein", c + 1); party = 1; }
        for (int m = 0; m < StoryMapsPerChapter; m++) {
            StageDef *s = &Stages[StageCount];
            memset(s, 0, sizeof *s);
            s->kind = STAGE_STORY; s->chapter = c; s->index = m;
            double lv = StoryMapsPerChapter > 1 ? lo + (hi - lo) * m / (StoryMapsPerChapter - 1) : lo;
            s->level = (int)(lv + 0.5);
            if (s->level < 1) s->level = 1;
            if (s->level > 100) s->level = 100;
            s->tmpl = m;
            s->partyMax = party;
            s->hpMul = party > 1 ? cfg.party_hp_mul : 1.0;
            s->atkMul = 1.0; s->xpMul = 1.0;
            s->req = StageCount - 1;
            snprintf(s->name, sizeof s->name, "%d-%d %s", c + 1, m + 1, ChapterNames[c]);
            StageCount++;
        }
    }
    for (int kind = 0; kind < 2; kind++) {
        const char *sname = kind == 0 ? "temple" : "ruins";
        int cnt = kind == 0 ? TempleCount : RuinsCount;
        for (int k = 0; k < cnt; k++) {
            int sec = ConfSection(&d, sname, k);
            StageDef *s = &Stages[StageCount];
            memset(s, 0, sizeof *s);
            s->kind = kind == 0 ? STAGE_TEMPLE : STAGE_RUINS; s->chapter = k; s->index = 0;
            const char *nm = ConfStr(&d, sec, "name", 1);
            snprintf(s->name, sizeof s->name, "%.39s", nm ? nm : "?");
            snprintf(kind == 0 ? TempleNames[k] : RuinsNames[k], 40, "%.39s", s->name);
            int map = (int)ConfNum(&d, sec, "map");
            if (map < 1 || map > TemplateCount) { ConfError(&d, 0, "%s %d: map %d existiert nicht (1 bis %d)", sname, k + 1, map, TemplateCount); map = 1; }
            s->tmpl = map - 1;
            s->level = (int)ConfNum(&d, sec, "level");
            if (s->level < 1) s->level = 1;
            if (s->level > 100) s->level = 100;
            int party = (int)ConfNumOpt(&d, sec, "party", 1);
            if (party < 1 || party > 2) party = 1;
            s->partyMax = party;
            double pm = party > 1 ? cfg.party_hp_mul : 1.0;
            s->hpMul = (kind == 0 ? cfg.temple_hp_mul : cfg.ruins_hp_mul) * pm;
            s->atkMul = kind == 0 ? cfg.temple_atk_mul : cfg.ruins_atk_mul;
            s->xpMul = kind == 0 ? cfg.temple_xp_mul : cfg.ruins_xp_mul;
            int after = (int)ConfNumOpt(&d, sec, "after_chapter", 0);
            s->req = (after >= 1 && after <= StoryChapters) ? (after - 1) * StoryMapsPerChapter + (StoryMapsPerChapter - 1) : -1;
            StageCount++;
        }
    }
    ConfFree(&d);
    return confErrors - before;
}

static const char *StageKindName(int kind) {
    return kind == STAGE_STORY ? "Geschichte" : (kind == STAGE_TEMPLE ? "Tempel (zähe Gegner)" : "Ruinen (starke Gegner)");
}
