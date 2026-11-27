#ifndef ITEMS_H
#define ITEMS_H

/* ------------------------------------------------------------------
 * Items, Tiers, Balancing  (reine Logik, keine SDL-Abhängigkeit)
 *
 * Alle Zahlen der Waffenstärken stammen aus Strength.ods (Tabelle
 * Y3:AU27, siehe items_table.inc). Alle anderen Werte (Rüstung,
 * andere Klassen, Monster, Erfahrung) wachsen mit derselben
 * logarithmischen Kurve (siehe PowerCurve()).
 * ------------------------------------------------------------------ */

/* ---- Tiers ---- */
#define TIER_COUNT 6
enum { TIER_NORMAL = 0, TIER_MAGIC, TIER_RARE, TIER_UNIQUE, TIER_BOSS, TIER_LEGENDARY };

/* ---- Heldenklassen (die ersten drei Werte entsprechen dem alten Code) ---- */
#define CLASS_COUNT 7
enum { CLASS_Knight = 0, CLASS_Mage = 1, CLASS_Archer = 2,
       CLASS_Warrior = 3, CLASS_Assassin = 4, CLASS_Shaman = 5, CLASS_Priest = 6 };

/* ---- Kategorien / Slots ---- */
enum {
    CAT_NONE = 0,
    CAT_WEAPON,    /* klassenspezifisch, 24 pro Klasse            */
    CAT_OFFHAND,   /* Sekundärwaffe, klassenspezifisch, 6 Tiers    */
    CAT_HELM,      /* 24 Stück: Crit-Chance                        */
    CAT_CHEST,     /* 24 Stück: Kistenkapazität                    */
    CAT_GLOVES,    /* 24 Stück: Angriffsgeschwindigkeit            */
    CAT_BOOTS,     /* 24 Stück: Bewegungsgeschwindigkeit           */
    CAT_BAG,       /* Tragetasche: Kistenkapazität, 6 Tiers        */
    CAT_NECKLACE,  /* Halskette = doppelt so stark wie ein Ring    */
    CAT_RING,
    CAT_POTION,
    CAT_WINGS,
    CAT_MATERIAL,
    CAT_COUNT
};

enum { JEW_WARRIOR = 0, JEW_ASSASSIN, JEW_ELEMENTAL, JEW_LIFE, JEW_COUNT };
enum { POT_HEALTH = 0, POT_MANA, POT_SPEED, POT_ARMOR, POT_ATKSPEED, POT_COUNT };
enum { MAT_ORE = 0, MAT_WOOD, MAT_LEATHER, MAT_ESSENCE, MAT_COUNT };

#define ITEM_MAX_LEVEL        21   /* 21 nur durch den "Level-Sprung" von 19 aus */
#define ITEM_NORMAL_MAX_LEVEL 20
#define LEVEL_SKIP_CHANCE     20   /* Prozent, siehe ItemLevelUp()               */
#define SLOTS_PER_CLASS       24

/* Ein Item. Alles 0 = leerer Slot. */
typedef struct {
    unsigned char  cat;
    unsigned char  tier;
    unsigned char  kind;   /* Waffe/Offhand: Klasse, Schmuck/Trank/Material: Art */
    unsigned char  idx;    /* Waffe/Rüstung: Nummer innerhalb des Tiers          */
    unsigned char  level;  /* Item-Level 0..21                                   */
    unsigned short count;  /* Material: Stapel; Trank: Ladung 0..100             */
} Item;

/* Zusammengerechnete Werte aller angelegten Items eines Helden. */
typedef struct {
    double attack;         /* flach (Waffe)              */
    double defense;        /* flach (Rüstung)            */
    double attackPct;      /* +% Angriff                 */
    double critChance;     /* Prozentpunkte              */
    double attackSpeedPct; /* +% Angriffsgeschwindigkeit */
    double moveSpeedPct;   /* +% Laufgeschwindigkeit     */
    double maxManaPct;
    double manaRegenPct;
    double maxHpPct;
    double hpRegenPct;     /* % der Max-Leben pro Sekunde */
    double capacity;       /* zusätzliche Kistenkapazität */
    double blockPct;       /* zusätzliche Schadensreduktion */
    double healPowerPct;
} ItemStats;

/* Rezept: bis zu 3 Materialien + Gold */
typedef struct {
    Item   mats[3];
    int    matCount;
    double gold;
} Recipe;

/* ---- Tiers ---- */
const char  *TierName(int tier);
unsigned int TierColor(int tier);      /* 0xRRGGBB */
unsigned int TierColorDark(int tier);
long         TierChestDenominator(int tier); /* 10, 100, 1000 ... */
int          TierSlotCount(int tier);        /* 7,5,5,5,1,1       */
int          TierFirstSlot(int tier);        /* 0,7,12,17,22,23   */
int          TierMinMapLevel(int tier);

/* ---- Klassen ---- */
const char *ClassName(int cls);
const char *ClassDescription(int cls);
const char *CategoryName(int cat);
const char *MaterialName(int kind);
const char *PotionName(int kind);
const char *JewelryName(int kind, int necklace);

/* ---- Items ---- */
int    ItemIsEmpty(const Item *it);
int    ItemSame(const Item *a, const Item *b);       /* stapelbar / gleich */
int    ItemRequiredLevel(const Item *it);            /* Helden-Level       */
int    ItemCanLevel(const Item *it);
int    ItemIsCraftable(const Item *it);
int    ItemIsStackable(const Item *it);
Item   ItemMake(int cat, int tier, int kind, int idx);
Item   ItemMakeMaterial(int tier, int kind, int count);
void   ItemName(const Item *it, char *buf, int n);
int    ItemDescribe(const Item *it, char lines[][72], int maxLines);
void   ItemAddStats(ItemStats *s, const Item *it);
double TableStrength(int slot, int level);           /* Ritter-Referenz    */
double ItemPower(const Item *it);                    /* Angriff/Verteidigung des Items */
double ItemSellValue(const Item *it);
int    ItemRecipe(const Item *target, Recipe *r);    /* 0 = nicht craftbar */
int    ItemUpgradeRecipe(const Item *it, Recipe *r); /* 0 = nicht möglich  */
/* Level +1 (bei Level 19 mit LEVEL_SKIP_CHANCE % direkt auf 21).
 * rnd = Zufallszahl 0..99. Rückgabe: neues Level. */
int    ItemLevelUp(Item *it, int rnd);

/* Zufälliger Item-Drop aus einer Kiste. Berücksichtigt das Map-Level
 * (Item darf erst ab seinem Mindestlevel gefunden werden).
 * Rückgabe 0, falls ein Material statt eines Items gewürfelt wurde. */
int    ItemRollFromChest(int tier, int mapLevel, Item *out);
/* Würfelt Kisten-Inhalt (Material oder Item). Immer gültiges Item. */
Item   ChestLoot(int tier, int mapLevel);

/* Potions */
double PotionEffectPct(int kind, int tier);
int    PotionDurationFrames(int tier);   /* 60 Frames = 1 s */
int    PotionUseCost(int tier);          /* Ladung pro Benutzung (von 100) */
int    PotionKillRefill(int tier);

/* Flügel */
double WingsSpeedPct(int tier);

/* ---- Balancing (logarithmische Kurve) ---- */
double PowerCurve(double level);                 /* 5 .. 2.5 Mio für Level 0..100 */
double MonsterBaseHP(int mapLevel);
double MonsterBaseAttack(int mapLevel);
double MonsterArmorK(int mapLevel);              /* Konstante der Schadensformel */
double GoldPerKill(int mapLevel);
double XpNeeded(int level);                      /* Held, Level -> Level+1 */
double XpForKill(int mapLevel);
double PlayerXpNeeded(int level);
int    HeroSlotsForPlayerLevel(int playerLevel); /* 4 .. 16 */
double HeroBaseHP(int cls, int level);
double HeroBaseMana(int cls, int level);
double HeroBaseAttack(int cls, int level);
double ClassBaseMoveSpeed(int cls);              /* Pixel pro Frame */
int    ClassAttackInterval(int cls);             /* Frames */
double ClassAttackRange(int cls);                /* Pixel */
int    ClassIsRanged(int cls);

/* ---- Formatierung ---- */
void FormatNumber(double v, char *buf, int n);   /* 1.2K, 3.4M, 5.6B */

#endif /* ITEMS_H */
