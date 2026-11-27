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
/* Monster-Typen                                                      */
/* ================================================================== */

typedef struct { const char *name; double hp, atk, speed, radius; int ranged; } MonsterInfo;

static const MonsterInfo MonsterInfos[MONSTER_TYPES] = {
    { "-",        1.0, 1.0, 0.0, 32, 0 },
    { "Dämon",    1.0, 1.0, 2.2, 32, 0 },
    { "Schleim",  1.6, 0.7, 1.2, 34, 0 },
    { "Fledermaus", 0.5, 0.8, 4.2, 24, 0 },
    { "Unhold",   2.2, 1.5, 1.6, 46, 0 },
    { "Spucker",  0.8, 1.0, 1.8, 30, 1 },
};

static const double MONSTER_TIER_HP[TIER_COUNT]  = { 1, 2, 4, 8, 7, 64 };
static const double MONSTER_TIER_ATK[TIER_COUNT] = { 1, 1.2, 1.5, 2, 1.6, 5 };
static const double MONSTER_TIER_XP[TIER_COUNT]  = { 1, 2.5, 6, 15, 40, 200 };

/* ================================================================== */
/* Kartenvorlagen (Layouts). Es gibt bisher 5 Karten-Grafiken        */
/* (assets/map/dev); Kapitel verwenden die Layouts mehrfach.         */
/* ================================================================== */

static Template Templates[MAX_TEMPLATES];

static void TplSpawn(Template *t, int x, int y, int role) {
    if (t->nSpawn >= MAX_MONSTERS) return;
    t->spawn[t->nSpawn].x = x; t->spawn[t->nSpawn].y = y; t->spawn[t->nSpawn].role = role;
    t->nSpawn++;
}
static void TplWay(Template *t, int x, int y) {
    if (t->nWay >= MAX_WAYPOINTS) return;
    t->way[t->nWay].x = x; t->way[t->nWay].y = y; t->nWay++;
}

static void InitTemplates(void) {
    memset(Templates, 0, sizeof Templates);
    Template *t;

    /* Layout 0 (Karte "1-1") */
    t = &Templates[0];
    t->startX = 1020; t->startY = 6900; t->angle = 90;
    TplSpawn(t, 6800, 4500, ROLE_BOSS);
    TplSpawn(t, 4500, 6000, 0); TplSpawn(t, 4700, 6000, 0); TplSpawn(t, 4500, 5200, 0);
    TplSpawn(t, 4700, 5200, 0); TplSpawn(t, 5200, 4200, 0); TplSpawn(t, 5200, 4500, 0);
    TplWay(t, 4600, 6800); TplWay(t, 4600, 4500); TplWay(t, 6800, 4500);

    /* Layout 1 (Karte "1-2"): Monster-Raster */
    t = &Templates[1];
    t->startX = 6150; t->startY = 7300; t->angle = 90;
    TplSpawn(t, 7500, 5900, ROLE_BOSS);
    for (int y = 5800; y <= 7300; y += 300)
        for (int x = 6000; x <= 7500; x += 300) TplSpawn(t, x, y, 0);
    TplWay(t, 7500, 7300); TplWay(t, 6100, 6000); TplWay(t, 7500, 5900);

    /* Layout 2 (Karte "1-3"): lange Monsterreihe */
    t = &Templates[2];
    t->startX = 2200; t->startY = 3000; t->angle = 90;
    TplSpawn(t, 7400, 1950, ROLE_BOSS);
    for (int x = 5200; x <= 6700; x += 50) TplSpawn(t, x, 3050, 0);
    TplWay(t, 2200, 5300); TplWay(t, 3500, 5300); TplWay(t, 3500, 3100);
    TplWay(t, 6700, 3000); TplWay(t, 6700, 2000); TplWay(t, 7400, 1950);

    /* Layout 3 (Karte "1-4"): Zickzack mit Zwischenboss */
    t = &Templates[3];
    t->startX = 1100; t->startY = 5500; t->angle = 90;
    TplSpawn(t, 5900, 900, ROLE_BOSS);
    TplSpawn(t, 5700, 2000, ROLE_MINIBOSS);
    { static const int gx[2][2] = { { 2000, 2400 }, { 4000, 4500 } };
      static const int gy[2][3] = { { 700, 850, 1000 }, { 1850, 2000, 2150 } };
      for (int r = 0; r < 2; r++) for (int c = 0; c < 2; c++)
          for (int xi = 0; xi < 2; xi++) for (int yi = 0; yi < 3; yi++)
              TplSpawn(t, gx[c][xi], gy[r][yi], 0); }
    TplWay(t, 1100, 2000); TplWay(t, 5700, 2000); TplWay(t, 1100, 2000);
    TplWay(t, 1100, 900);  TplWay(t, 5900, 900);

    /* Layout 4 (Karte "2-1"): zweite Karten-Grafik */
    t = &Templates[4];
    t->startX = 16000; t->startY = 17500; t->angle = 90;
    TplSpawn(t, 11600, 17400, ROLE_BOSS);
    TplSpawn(t, 13500, 13000, ROLE_MINIBOSS);
    TplSpawn(t, 15800, 13000, ROLE_MINIBOSS);
    static const Pt way4[20] = {
        {16000,15500},{15836,15236},{15844,15272},{15804,13048},{15308,13016},
        {14852,13012},{14464,13024},{14328,13124},{13876,13120},{13452,12960},
        {13472,14400},{13464,15108},{13356,15416},{13356,15416},{13164,15488},
        {11944,15476},{11740,15532},{11608,15744},{11596,16824},{11600,17400} };
    for (int i = 0; i < 20; i++) TplWay(t, way4[i].x, way4[i].y);
    /* zusätzliche Gegner entlang des Weges (Waypoint-Mitte = begehbar) */
    for (int i = 3; i < 19; i += 2) { TplSpawn(t, way4[i].x + 40, way4[i].y + 20, 0); TplSpawn(t, way4[i].x - 40, way4[i].y - 20, 0); }
}

/* ================================================================== */
/* Karten / Kapitel                                                   */
/* ================================================================== */

static StageDef Stages[MAX_STAGES];
static int      StageCount = 0;

#define STORY_CHAPTERS 10
#define STORY_MAPS_PER_CHAPTER 4
#define TEMPLE_COUNT 5
#define RUINS_COUNT 5

static const char *CHAPTER_NAMES[STORY_CHAPTERS] = {
    "Grenzland", "Dunkelwald", "Sumpf der Vergessenen", "Aschewüste", "Frostgipfel",
    "Kristallhöhlen", "Verlorene Stadt", "Vulkanschmiede", "Schattenreich", "Der Abgrund"
};
static const char *TEMPLE_NAMES[TEMPLE_COUNT] = { "Tempel des Eisens", "Tempel des Feuers", "Tempel des Sturms", "Tempel der Schatten", "Tempel des Himmels" };
static const char *RUINS_NAMES[RUINS_COUNT]   = { "Ruinen I", "Ruinen II", "Ruinen III", "Ruinen IV", "Ruinen V" };

static void BuildStages(void) {
    StageCount = 0;
    for (int c = 0; c < STORY_CHAPTERS; c++) {
        for (int m = 0; m < STORY_MAPS_PER_CHAPTER; m++) {
            StageDef *s = &Stages[StageCount];
            int idx = c * STORY_MAPS_PER_CHAPTER + m;
            memset(s, 0, sizeof *s);
            s->kind = STAGE_STORY; s->chapter = c; s->index = m;
            s->level = 1 + idx * 99 / (STORY_CHAPTERS * STORY_MAPS_PER_CHAPTER - 1);
            s->tmpl = (m == 3 && (c & 1)) ? 4 : m;
            s->partyMax = (c >= 5) ? 2 : 1;           /* mehrere Helden erst in späteren Kapiteln */
            s->hpMul = (s->partyMax > 1) ? 1.6 : 1.0;
            s->atkMul = 1.0; s->xpMul = 1.0;
            s->req = StageCount - 1;
            snprintf(s->name, sizeof s->name, "%d-%d %s", c + 1, m + 1, CHAPTER_NAMES[c]);
            StageCount++;
        }
    }
    for (int k = 0; k < TEMPLE_COUNT; k++) {
        StageDef *s = &Stages[StageCount];
        memset(s, 0, sizeof *s);
        s->kind = STAGE_TEMPLE; s->chapter = k; s->index = 0;
        s->level = 5 + 20 * k; s->tmpl = 2;
        s->partyMax = (k >= 2) ? 2 : 1;
        s->hpMul = 6.0 * (s->partyMax > 1 ? 1.6 : 1.0); s->atkMul = 0.5; s->xpMul = 1.6;
        s->req = k * 8 + 3;                              /* Kapitel 1,3,5,7,9 abgeschlossen */
        snprintf(s->name, sizeof s->name, "%s", TEMPLE_NAMES[k]);
        StageCount++;
    }
    for (int k = 0; k < RUINS_COUNT; k++) {
        StageDef *s = &Stages[StageCount];
        memset(s, 0, sizeof *s);
        s->kind = STAGE_RUINS; s->chapter = k; s->index = 0;
        s->level = 10 + 20 * k; s->tmpl = 1;
        s->partyMax = (k >= 2) ? 2 : 1;
        s->hpMul = 0.6 * (s->partyMax > 1 ? 1.6 : 1.0); s->atkMul = 5.0; s->xpMul = 1.6;
        s->req = k * 8 + 7;                              /* Kapitel 2,4,6,8,10 abgeschlossen */
        snprintf(s->name, sizeof s->name, "%s", RUINS_NAMES[k]);
        StageCount++;
    }
}

static const char *StageKindName(int kind) {
    return kind == STAGE_STORY ? "Geschichte" : (kind == STAGE_TEMPLE ? "Tempel (zähe Gegner)" : "Ruinen (starke Gegner)");
}
