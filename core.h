#ifndef CORE_H
#define CORE_H

/* Gemeinsame Konstanten und Datenstrukturen des Spiels.
 * (game.c bindet die übrigen .c-Dateien direkt ein, wie schon maps.c.) */

#include "items.h"

#define MAX_HEROES      16
#define MAX_MONSTERS    80
#define MAX_PROJECTILES 2048
#define MAX_WAYPOINTS   64
#define MAX_INV         160
#define MAX_STAGES      64
#define MAX_TEMPLATES   8
#define MAX_FLOATTEXT   160
#define LAYOUT_COUNT    4
#define FPS             60
#define HERO_MAX_LEVEL  100
#define PLAYER_MAX_LEVEL 100
#define CHEST_BASE_CAPACITY 10

/* ---- Ausrüstungs-Slots (14 Stück laut Anforderungskatalog) ---- */
enum {
    EQ_WEAPON, EQ_OFFHAND, EQ_HELM, EQ_CHEST, EQ_GLOVES, EQ_BOOTS, EQ_BAG,
    EQ_NECK, EQ_RING1, EQ_RING2, EQ_POT1, EQ_POT2, EQ_POT3, EQ_WINGS, EQ_COUNT
};

/* ---- Monster ---- */
#define MONSTER_NULL   0
#define MONSTER_HUMAN  1   /* "Dämon": Standard-Nahkämpfer                */
#define MONSTER_SLIME  2   /* langsam, viel Leben                          */
#define MONSTER_BAT    3   /* schnell, flatterhaft, wenig Leben            */
#define MONSTER_BRUTE  4   /* groß, langsam, schlägt hart                  */
#define MONSTER_SPITTER 5  /* Fernkämpfer, hält Abstand                    */
#define MONSTER_TYPES  6

enum { ROLE_NORMAL = 0, ROLE_MINIBOSS = 1, ROLE_BOSS = 2 };

typedef struct {
    int    exist, alive, type, tier, role, level;
    double x, y, angle, hp, hpMax, atk, speed, radius;
    int    attackCd, shootCd, specialCd, hitFlash, poisonFrames, aggro, lastHero;
    double poisonDps, wanderA;
    int    wanderT;
} Monster;

/* ---- Projektile ---- */
#define PROJ_ARROW   0
#define PROJ_ARCANE  1
#define PROJ_DAGGER  2
#define PROJ_HOLY    3
#define PROJ_VENOM   4
#define PROJ_ENEMY   5

typedef struct {
    int    valid, inst, faction /*0=Held 1=Monster*/, owner, visual, pierce, life, nhit, crit;
    int    hit[8];
    double x, y, angle, speed, damage, radius, poison;
} Projectile;

/* ---- Skills ---- */
enum { SKT_NONE = 0, SKT_FAN, SKT_CIRCLE, SKT_NOVA, SKT_CONE, SKT_DASH, SKT_CHAIN,
       SKT_HEAL, SKT_BUFF, SKT_POISON, SKT_RAIN, SKT_CHANNEL };
enum { BUFF_SPEED = 0, BUFF_ARMOR, BUFF_ATTACK, BUFF_ATKSPEED };

typedef struct {
    const char *name;
    const char *desc;
    int    cls, unlock, type, cd /*Frames*/, n, visual, dur, buff;
    double manaPct /* % des max. Mana */, mult /* x Angriff */, radius, angle, speed, pct;
} SkillDef;

#define SKILLS_PER_CLASS 6

/* ---- Helden ---- */
typedef struct {
    int    isHero, class, level;
    char   name[20];
    double xp;
    Item   equip[EQ_COUNT];

    int    stage, inst;              /* stage -1 = im Lager (keine Karte) */
    double x, y;
    double xTarget, yTarget;
    double angle;
    double hp, hpMax, mana, manaMax;

    /* abgeleitete Werte (RecalcHero) */
    double atk, def, crit, critMul, atkSpeed, moveSpeed, manaRegen, hpRegen, healPower, block;
    int    capacity;

    int    autopilotEnabled, autopilot, manualTimer, waypoint, retreat, focus;
    int    walking;  double walkCycle;
    int    attacking; double attackCycle; int attackTotal;
    int    skilling, skillActive, skillCycle;
    int    skillSlots[4];            /* Index in SkillDefs oder -1 */
    int    skillCd[4];

    int    buffF[4];  double buffPct[4];

    int    chests[TIER_COUNT];
    int    chestLevel[TIER_COUNT];

    double stuckX, stuckY; int stuckT, ignoreT;   /* Festhänge-Erkennung */
    int    autoNext;                 /* nach dem ersten Sieg automatisch zur nächsten Karte */
    int    partner;                  /* zweiter Held auf derselben Karte oder -1 */
    long   kills;
    unsigned chestMask;              /* Bit t gesetzt = Kisten der Stufe t werden gesammelt */
} Hero;

/* ---- Karten ---- */
enum { STAGE_STORY = 0, STAGE_TEMPLE, STAGE_RUINS };

typedef struct {
    char   name[28];
    int    kind, chapter, index, level, tmpl, partyMax;
    double hpMul, atkMul, xpMul;
    int    req;                       /* Stage-Index, der geschafft sein muss (-1 = frei) */
} StageDef;

typedef struct { int x, y, role; } Spawn;
typedef struct { int x, y; } Pt;

typedef struct {
    int    startX, startY, angle;
    int    nSpawn;  Spawn spawn[MAX_MONSTERS];
    int    nWay;    Pt way[MAX_WAYPOINTS];
} Template;

enum { RUN_PLAYING = 0, RUN_CLEARED, RUN_LOST };

typedef struct {
    int     used, stage, nHeroes, heroId[2];
    int     state, timer, nextStage;
    Monster mon[MAX_MONSTERS];
} Instance;

/* ---- Spieler ---- */
typedef struct {
    double gold;
    int    level;
    double xp;
    Item   inv[MAX_INV];
    unsigned char cleared[MAX_STAGES];
    int    layouts[LAYOUT_COUNT][4];
    int    activeLayout;
    long   totalKills;
    unsigned int shopSeed;
    int    shopStock[TIER_COUNT][MAT_COUNT];
} Player;

typedef struct {
    int inst; double x, y; int life; double value; int color, kind;
} FloatText;

#endif /* CORE_H */
