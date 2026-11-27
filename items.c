#include "items.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "items_table.inc"   /* KnightReq[24], KnightStrength[24][22] aus Strength.ods */

/* ================================================================== */
/* Tiers                                                              */
/* ================================================================== */

static const char *TIER_NAMES[TIER_COUNT] = {
    "Normal", "Magisch", "Selten", "Einzigartig", "Boss", "Legendär"
};
/* normal=grau, magisch=blau, selten=gelb, einzigartig=orange, boss=rot, legendär=rosa */
static const unsigned int TIER_COLORS[TIER_COUNT] = {
    0x9AA0A6, 0x3B82F6, 0xFACC15, 0xFB923C, 0xEF4444, 0xF472B6
};
static const unsigned int TIER_COLORS_DARK[TIER_COUNT] = {
    0x4B5057, 0x1D3F8A, 0x8A6F08, 0x93480B, 0x8A1C1C, 0x8E2B5F
};
static const long CHEST_DEN[TIER_COUNT] = { 10, 100, 1000, 10000, 100000, 1000000 };
static const int  SLOT_COUNT[TIER_COUNT] = { 7, 5, 5, 5, 1, 1 };
static const int  SLOT_FIRST[TIER_COUNT] = { 0, 7, 12, 17, 22, 23 };

const char  *TierName(int t)  { return (t >= 0 && t < TIER_COUNT) ? TIER_NAMES[t] : "?"; }
unsigned int TierColor(int t) { return (t >= 0 && t < TIER_COUNT) ? TIER_COLORS[t] : 0x808080; }
unsigned int TierColorDark(int t) { return (t >= 0 && t < TIER_COUNT) ? TIER_COLORS_DARK[t] : 0x404040; }
long TierChestDenominator(int t) { return (t >= 0 && t < TIER_COUNT) ? CHEST_DEN[t] : 10; }
int  TierSlotCount(int t) { return (t >= 0 && t < TIER_COUNT) ? SLOT_COUNT[t] : 0; }
int  TierFirstSlot(int t) { return (t >= 0 && t < TIER_COUNT) ? SLOT_FIRST[t] : 0; }
int  TierMinMapLevel(int t) { return KnightReq[TierFirstSlot(t)]; }

/* ================================================================== */
/* Namen                                                              */
/* ================================================================== */

typedef struct {
    const char *name, *desc;
    double hpMul, manaMul, atkMul, moveSpeed, range;
    int atkInterval, ranged;
} ClassInfo;

static const ClassInfo CLASSES[CLASS_COUNT] = {
    /* Knight  */ { "Ritter",       "Schwer gepanzerter Frontkämpfer. Hält viel aus und schützt sich mit dem Schild.",     1.50, 0.60, 1.00,  9.0,  90.0, 36, 0 },
    /* Mage    */ { "Magier",       "Fernkämpfer mit mächtigen Arkanzaubern. Viel Mana, wenig Leben.",                      0.70, 2.00, 1.25,  9.0, 520.0, 45, 1 },
    /* Archer  */ { "Bogenschütze", "Schneller Fernkämpfer. Viele Pfeile, hohe Beweglichkeit.",                             0.85, 1.00, 1.10, 11.0, 560.0, 30, 1 },
    /* Warrior */ { "Krieger",      "Wuchtige Flächenschläge mit schweren Waffen. Langsam, aber verheerend.",               1.30, 0.60, 1.20,  9.0, 100.0, 55, 0 },
    /* Assassin*/ { "Assassine",    "Blitzschneller Nahkämpfer mit hohen kritischen Treffern und Gift.",                    0.85, 0.90, 0.90, 13.0,  70.0, 18, 0 },
    /* Shaman  */ { "Schamane",     "Beschwört Blitze und Totems. Schwächt Gegner mit Gift und heilt über Zeit.",           1.00, 1.50, 1.05,  9.0, 450.0, 42, 1 },
    /* Priest  */ { "Priester",     "Heiler und Lichtmagier. Heilt sich und schützt mit Segen.",                            1.10, 1.80, 0.85,  9.0, 420.0, 48, 1 },
};

const char *ClassName(int c)        { return (c >= 0 && c < CLASS_COUNT) ? CLASSES[c].name : "?"; }
const char *ClassDescription(int c) { return (c >= 0 && c < CLASS_COUNT) ? CLASSES[c].desc : ""; }

static const char *CAT_NAMES[CAT_COUNT] = {
    "-", "Waffe", "Sekundärwaffe", "Helm", "Brustpanzer", "Handschuhe", "Stiefel",
    "Tragetasche", "Halskette", "Ring", "Trank", "Flügel", "Material"
};
const char *CategoryName(int c) { return (c >= 0 && c < CAT_COUNT) ? CAT_NAMES[c] : "?"; }

static const char *MAT_NAMES[MAT_COUNT] = { "Erz", "Holz", "Leder", "Essenz" };
const char *MaterialName(int k) { return (k >= 0 && k < MAT_COUNT) ? MAT_NAMES[k] : "?"; }

static const char *POT_NAMES[POT_COUNT] = {
    "Heiltrank", "Manatrank", "Laufgeschwindigkeits-Trank", "Rüstungstrank", "Angriffstempo-Trank"
};
const char *PotionName(int k) { return (k >= 0 && k < POT_COUNT) ? POT_NAMES[k] : "?"; }

static const char *RING_NAMES[JEW_COUNT]  = { "Kriegerring", "Assassinenring", "Elementarring", "Ring des Lebens" };
static const char *NECK_NAMES[JEW_COUNT]  = { "Kriegerkette", "Assassinenkette", "Elementarkette", "Halskette des Lebens" };
const char *JewelryName(int k, int neck) {
    if (k < 0 || k >= JEW_COUNT) return "?";
    return neck ? NECK_NAMES[k] : RING_NAMES[k];
}

static const char *WEAPON_NAMES[CLASS_COUNT][SLOTS_PER_CLASS] = {
    /* Ritter: Schwerter */
    { "Rostiges Schwert","Kurzschwert","Langschwert","Breitschwert","Stahlschwert","Ritterschwert","Gardeschwert",
      "Runenklinge","Sturmklinge","Mondklinge","Frostklinge","Flammenklinge",
      "Dämmerklinge","Gerechtigkeit","Wolfsfang","Königsschwert","Himmelsspalter",
      "Drachentöter","Sonnenklinge","Phantomklinge","Seelenschneider","Weltenbrecher",
      "Klinge des Abgrunds","Schwert der Ewigkeit" },
    /* Magier: Stäbe */
    { "Knorriger Stab","Holzstab","Kristallstab","Zauberstab","Eichenstab","Magierstab","Runenstab",
      "Feuerstab","Eisstab","Blitzstab","Arkanstab","Sternenstab",
      "Erzmagier-Stab","Seelenstab","Äonenstab","Mondstab","Sonnenstab",
      "Stab der Weisheit","Stab des Chaos","Stab der Leere","Stab der Elemente","Stab der Unendlichkeit",
      "Stab des Abgrunds","Weltenstab" },
    /* Bogenschütze: Bögen */
    { "Kurzbogen","Jagdbogen","Langbogen","Eibenbogen","Kompositbogen","Reiterbogen","Waldbogen",
      "Windbogen","Falkenbogen","Sturmbogen","Mondbogen","Elfenbogen",
      "Schattenbogen","Adlerauge","Drachenbogen","Sonnenbogen","Frostbogen",
      "Sternenbogen","Phönixbogen","Donnerbogen","Geisterbogen","Himmelsbogen",
      "Bogen des Verderbens","Bogen der Ewigkeit" },
    /* Krieger: Äxte und Hämmer */
    { "Holzfälleraxt","Handaxt","Kriegsbeil","Streitaxt","Doppelaxt","Schlachtbeil","Kriegshammer",
      "Eisenzahn","Blutbeil","Donnerhammer","Schädelspalter","Berserkeraxt",
      "Orkbrecher","Titanenhammer","Gletscheraxt","Henkersbeil","Lavahammer",
      "Gigantenbrecher","Ragnarök","Blutdurst","Zorn der Berge","Urgewalt",
      "Axt des Verderbens","Hammer der Ewigkeit" },
    /* Assassine: Dolche */
    { "Rostdolch","Küchenmesser","Dolch","Stilett","Kampfmesser","Parierdolch","Klauendolch",
      "Giftdolch","Nachtdorn","Mondsichel","Skorpionstachel","Schattenfang",
      "Schlangenzahn","Seelendolch","Geisterklinge","Blutnadel","Kehlenschneider",
      "Phantomdolch","Todesbote","Schweigen","Nebelfang","Schattenmeister",
      "Dolch des Verrats","Klinge der Stille" },
    /* Schamane: Totemstäbe und Rasseln */
    { "Knochenstab","Holzrassel","Federstab","Totemstab","Krallenstab","Geisterrassel","Ahnenstab",
      "Wolfstotem","Bärentotem","Adlertotem","Schlangentotem","Donnertotem",
      "Geistertotem","Ahnentotem","Sturmrassel","Nebelstab","Seelenrassel",
      "Stab der Urahnen","Totem des Windes","Totem der Erde","Totem des Feuers","Totem des Sturms",
      "Totem des Abgrunds","Weltentotem" },
    /* Priester: Zepter und Kolben */
    { "Holzkeule","Pilgerstock","Messingkolben","Weihrauchkolben","Betzepter","Klerikerkolben","Lichtkolben",
      "Segenszepter","Gnadenzepter","Heiliger Kolben","Engelszepter","Sonnenzepter",
      "Hohepriester-Zepter","Lichtbringer","Himmelszepter","Seraphim-Stab","Heiliger Hammer",
      "Zepter der Reinheit","Zepter des Glaubens","Zepter der Gnade","Zepter des Urteils","Zepter der Erlösung",
      "Zepter des Falls","Zepter der Ewigkeit" },
};

static const char *ARMOR_SETS[SLOTS_PER_CLASS] = {
    "Lumpen","Leder","Kupfer","Bronze","Eisen","Stahl","Ritter",
    "Silber","Ketten","Mithril","Runen","Sturm",
    "Schuppen","Platten","Obsidian","Drachen","Frost",
    "Titan","Phönix","Äther","Sonnen","Mond",
    "Abgrund","Ewigkeits"
};
static const char *ARMOR_PARTS[4] = { "helm", "panzer", "handschuhe", "stiefel" };
static const char *OFFHAND_NAMES[CLASS_COUNT] = {
    "Schild", "Kristallkugel", "Köcher", "Wurfbeil", "Wurfdolch", "Rassel", "Gebetbuch"
};

/* ================================================================== */
/* Hilfsfunktionen                                                    */
/* ================================================================== */

static int Clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

double TableStrength(int slot, int level) {
    slot  = Clampi(slot, 0, SLOTS_PER_CLASS - 1);
    level = Clampi(level, 0, ITEM_MAX_LEVEL);
    return KnightStrength[slot][level];
}

/* 0..1: wo liegt ein Wert logarithmisch zwischen kleinstem und größtem Tabellenwert */
static double LogNorm(double v) {
    double lo = KnightStrength[0][0];
    double hi = KnightStrength[23][ITEM_MAX_LEVEL];
    if (v <= lo) return 0.0;
    if (v >= hi) return 1.0;
    return log(v / lo) / log(hi / lo);
}

static const double PCT[TIER_COUNT] = { 3, 6, 12, 25, 50, 100 };   /* Basiseffekt Schmuck/Offhand in % */
static double LevelMul(int level) { return 1.0 + 0.06 * level; }

static int SlotOf(const Item *it) {
    return TierFirstSlot(it->tier) + Clampi(it->idx, 0, TierSlotCount(it->tier) - 1);
}

int ItemIsEmpty(const Item *it) { return it->cat == CAT_NONE; }
int ItemIsStackable(const Item *it) { return it->cat == CAT_MATERIAL; }
int ItemSame(const Item *a, const Item *b) {
    if (a->cat != b->cat || a->tier != b->tier || a->kind != b->kind) return 0;
    if (a->cat == CAT_MATERIAL) return 1;
    return a->idx == b->idx && a->level == b->level;
}

Item ItemMake(int cat, int tier, int kind, int idx) {
    Item it; memset(&it, 0, sizeof it);
    it.cat = (unsigned char)cat; it.tier = (unsigned char)tier;
    it.kind = (unsigned char)kind; it.idx = (unsigned char)idx;
    it.count = (cat == CAT_POTION) ? 100 : 1;
    return it;
}
Item ItemMakeMaterial(int tier, int kind, int count) {
    Item it = ItemMake(CAT_MATERIAL, tier, kind, 0);
    it.count = (unsigned short)count;
    return it;
}

int ItemRequiredLevel(const Item *it) {
    switch (it->cat) {
    case CAT_WEAPON: case CAT_HELM: case CAT_CHEST: case CAT_GLOVES: case CAT_BOOTS:
        return KnightReq[SlotOf(it)];
    case CAT_OFFHAND:
        return KnightReq[TierFirstSlot(it->tier)];
    case CAT_NONE:
        return 0;
    default:               /* Schmuck, Tränke, Flügel, Tasche: immer ab Level 1 */
        return 1;
    }
}

int ItemCanLevel(const Item *it) {
    switch (it->cat) {
    case CAT_WEAPON: case CAT_OFFHAND: case CAT_HELM: case CAT_CHEST:
    case CAT_GLOVES: case CAT_BOOTS: case CAT_RING: case CAT_NECKLACE:
        return it->level < ITEM_NORMAL_MAX_LEVEL;
    }
    return 0;
}

int ItemIsCraftable(const Item *it) {
    switch (it->cat) {
    case CAT_WEAPON: case CAT_OFFHAND: case CAT_HELM: case CAT_CHEST:
    case CAT_GLOVES: case CAT_BOOTS: case CAT_BAG: case CAT_POTION:
        return 1;
    }
    return 0;   /* Schmuck und Flügel gibt es nur in Kisten */
}

int ItemLevelUp(Item *it, int rnd) {
    if (!ItemCanLevel(it)) return it->level;
    if (it->level == ITEM_NORMAL_MAX_LEVEL - 1 && rnd < LEVEL_SKIP_CHANCE) it->level = ITEM_MAX_LEVEL;
    else it->level++;
    return it->level;
}

/* ================================================================== */
/* Werte                                                              */
/* ================================================================== */

double ItemPower(const Item *it) {
    switch (it->cat) {
    case CAT_WEAPON:  return TableStrength(SlotOf(it), it->level) * CLASSES[it->kind % CLASS_COUNT].atkMul;
    case CAT_HELM: case CAT_CHEST: case CAT_GLOVES: case CAT_BOOTS:
        return TableStrength(SlotOf(it), it->level);
    case CAT_OFFHAND: return TableStrength(TierFirstSlot(it->tier), it->level) * 0.35;
    }
    return 0.0;
}

static const double ARMOR_WEIGHT[4] = { 0.30, 0.50, 0.15, 0.15 };
static const double CHEST_ARMOR_CAPACITY[TIER_COUNT] = { 1, 3, 10, 30, 100, 300 };
static const double BAG_CAPACITY[TIER_COUNT] = { 3, 10, 30, 100, 300, 1000 };
static const double WINGS_PCT[TIER_COUNT]    = { 10, 30, 60, 110, 190, 300 };

double WingsSpeedPct(int tier) { return WINGS_PCT[Clampi(tier, 0, TIER_COUNT - 1)]; }

void ItemAddStats(ItemStats *s, const Item *it) {
    double pct = PCT[Clampi(it->tier, 0, TIER_COUNT - 1)] * LevelMul(it->level);
    switch (it->cat) {
    case CAT_WEAPON:
        s->attack += ItemPower(it);
        break;
    case CAT_OFFHAND:
        s->defense += ItemPower(it);
        switch (it->kind) {
        case CLASS_Knight:   s->blockPct       += pct * 0.3 > 30.0 ? 30.0 : pct * 0.3; break;
        case CLASS_Warrior:  s->attackPct      += pct * 0.5; break;
        case CLASS_Mage:     s->maxManaPct     += pct;       break;
        case CLASS_Archer:   s->attackSpeedPct += pct * 0.4; break;
        case CLASS_Assassin: s->critChance     += pct * 0.2; break;
        case CLASS_Shaman:   s->manaRegenPct   += pct;       break;
        case CLASS_Priest:   s->healPowerPct   += pct;       break;
        }
        break;
    case CAT_HELM: case CAT_CHEST: case CAT_GLOVES: case CAT_BOOTS: {
        double str = ItemPower(it);
        double n = LogNorm(str);
        s->defense += str * ARMOR_WEIGHT[it->cat - CAT_HELM];
        if (it->cat == CAT_HELM)   s->critChance     += 40.0 * n;
        if (it->cat == CAT_CHEST)  s->capacity       += CHEST_ARMOR_CAPACITY[Clampi(it->tier, 0, TIER_COUNT - 1)];
        if (it->cat == CAT_GLOVES) s->attackSpeedPct += 100.0 * n;
        if (it->cat == CAT_BOOTS)  s->moveSpeedPct   += 60.0 * n;
        break;
    }
    case CAT_BAG:
        s->capacity += BAG_CAPACITY[Clampi(it->tier, 0, TIER_COUNT - 1)];
        break;
    case CAT_RING: case CAT_NECKLACE: {
        double p = pct * (it->cat == CAT_NECKLACE ? 2.0 : 1.0);   /* Halskette = zwei Ringe */
        switch (it->kind) {
        case JEW_WARRIOR:   s->attackPct += p; s->critChance += p / 5.0; break;
        case JEW_ASSASSIN:  s->critChance += p / 4.0; s->attackSpeedPct += p / 2.0; break;
        case JEW_ELEMENTAL: s->maxManaPct += p * 1.5; s->manaRegenPct += p; break;
        case JEW_LIFE:      s->maxHpPct += p * 1.5; s->hpRegenPct += p / 20.0; break;
        }
        break;
    }
    case CAT_WINGS:
        s->moveSpeedPct += WingsSpeedPct(it->tier);
        break;
    }
}

/* ---- Tränke ---- */
static const double PotTblHeal[TIER_COUNT]  = { 10, 20, 35, 55, 80, 100 };
static const double PotTblMana[TIER_COUNT]  = { 15, 30, 50, 70, 90, 100 };
static const double PotTblSpd[TIER_COUNT]   = { 10, 20, 35, 50, 75, 100 };
static const double PotTblArm[TIER_COUNT]   = { 10, 25, 50, 100, 175, 300 };
static const double PotTblAs[TIER_COUNT]    = { 10, 20, 35, 60, 90, 150 };
static const int    POT_SECONDS[TIER_COUNT] = { 6, 8, 10, 12, 15, 20 };
static const int    POT_COST[TIER_COUNT]  = { 25, 20, 16, 12, 8, 5 };
static const int    POT_REFILL[TIER_COUNT] = { 3, 3, 4, 4, 5, 6 };

double PotionEffectPct(int kind, int tier) {
    tier = Clampi(tier, 0, TIER_COUNT - 1);
    switch (kind) {
    case POT_HEALTH:   return PotTblHeal[tier];
    case POT_MANA:     return PotTblMana[tier];
    case POT_SPEED:    return PotTblSpd[tier];
    case POT_ARMOR:    return PotTblArm[tier];
    case POT_ATKSPEED: return PotTblAs[tier];
    }
    return 0;
}
int PotionDurationFrames(int tier) { return POT_SECONDS[Clampi(tier, 0, TIER_COUNT - 1)] * 60; }
int PotionUseCost(int tier)        { return POT_COST[Clampi(tier, 0, TIER_COUNT - 1)]; }
int PotionKillRefill(int tier)     { return POT_REFILL[Clampi(tier, 0, TIER_COUNT - 1)]; }

/* ================================================================== */
/* Namen / Beschreibung                                               */
/* ================================================================== */

void ItemName(const Item *it, char *buf, int n) {
    switch (it->cat) {
    case CAT_WEAPON:
        snprintf(buf, n, "%s", WEAPON_NAMES[it->kind % CLASS_COUNT][SlotOf(it)]);
        break;
    case CAT_HELM: case CAT_CHEST: case CAT_GLOVES: case CAT_BOOTS:
        snprintf(buf, n, "%s%s", ARMOR_SETS[SlotOf(it)], ARMOR_PARTS[it->cat - CAT_HELM]);
        break;
    case CAT_OFFHAND:
        snprintf(buf, n, "%s [%s]", OFFHAND_NAMES[it->kind % CLASS_COUNT], TierName(it->tier));
        break;
    case CAT_BAG:
        snprintf(buf, n, "Tragetasche [%s]", TierName(it->tier));
        break;
    case CAT_RING: case CAT_NECKLACE:
        snprintf(buf, n, "%s [%s]", JewelryName(it->kind, it->cat == CAT_NECKLACE), TierName(it->tier));
        break;
    case CAT_POTION:
        snprintf(buf, n, "%s [%s]", PotionName(it->kind), TierName(it->tier));
        break;
    case CAT_WINGS:
        snprintf(buf, n, "Flügel [%s]", TierName(it->tier));
        break;
    case CAT_MATERIAL:
        snprintf(buf, n, "%s [%s]", MaterialName(it->kind), TierName(it->tier));
        break;
    default:
        snprintf(buf, n, "-");
    }
}

int ItemDescribe(const Item *it, char lines[][72], int max) {
    int c = 0;
    char a[32], b[32];
#define LINE(...) do { if (c < max) snprintf(lines[c++], 72, __VA_ARGS__); } while (0)
    if (it->cat == CAT_NONE) return 0;
    LINE("%s - %s", CategoryName(it->cat), TierName(it->tier));
    if (it->cat == CAT_WEAPON || it->cat == CAT_OFFHAND) LINE("Klasse: %s", ClassName(it->kind));
    if (ItemRequiredLevel(it) > 0 && it->cat != CAT_MATERIAL) LINE("Benötigtes Helden-Level: %d", ItemRequiredLevel(it));
    if (ItemCanLevel(it) || it->level > 0) LINE("Item-Level: %d / %d", it->level, ITEM_NORMAL_MAX_LEVEL);
    ItemStats s; memset(&s, 0, sizeof s);
    if (it->cat != CAT_MATERIAL && it->cat != CAT_POTION) ItemAddStats(&s, it);
    if (s.attack > 0)         { FormatNumber(s.attack, a, sizeof a);  LINE("Angriff: %s", a); }
    if (s.defense > 0)        { FormatNumber(s.defense, a, sizeof a); LINE("Rüstung: %s", a); }
    if (s.attackPct > 0)      LINE("Angriff: +%.1f%%", s.attackPct);
    if (s.critChance > 0)     LINE("Krit. Trefferchance: +%.1f%%", s.critChance);
    if (s.attackSpeedPct > 0) LINE("Angriffsgeschwindigkeit: +%.1f%%", s.attackSpeedPct);
    if (s.moveSpeedPct > 0)   LINE("Laufgeschwindigkeit: +%.1f%%", s.moveSpeedPct);
    if (s.maxManaPct > 0)     LINE("Max. Mana: +%.1f%%", s.maxManaPct);
    if (s.manaRegenPct > 0)   LINE("Manaregeneration: +%.1f%%", s.manaRegenPct);
    if (s.maxHpPct > 0)       LINE("Max. Leben: +%.1f%%", s.maxHpPct);
    if (s.hpRegenPct > 0)     LINE("Lebensregeneration: %.2f%%/s", s.hpRegenPct);
    if (s.capacity > 0)       LINE("Kistenkapazität: +%.0f", s.capacity);
    if (s.blockPct > 0)       LINE("Schadensreduktion: +%.1f%%", s.blockPct);
    if (s.healPowerPct > 0)   LINE("Heilkraft: +%.1f%%", s.healPowerPct);
    if (it->cat == CAT_POTION) {
        double e = PotionEffectPct(it->kind, it->tier);
        switch (it->kind) {
        case POT_HEALTH:   LINE("Stellt sofort %.0f%% Leben wieder her", e); break;
        case POT_MANA:     LINE("Stellt sofort %.0f%% Mana wieder her", e); break;
        case POT_SPEED:    LINE("+%.0f%% Laufgeschwindigkeit für %ds", e, PotionDurationFrames(it->tier) / 60); break;
        case POT_ARMOR:    LINE("+%.0f%% Rüstung für %ds", e, PotionDurationFrames(it->tier) / 60); break;
        case POT_ATKSPEED: LINE("+%.0f%% Angriffstempo für %ds", e, PotionDurationFrames(it->tier) / 60); break;
        }
        LINE("%d Benutzungen pro Ladung, füllt sich bei Kills auf", 100 / PotionUseCost(it->tier));
    }
    if (it->cat == CAT_MATERIAL) LINE("Baumaterial (Tier: %s)", TierName(it->tier));
    if (it->cat == CAT_RING || it->cat == CAT_NECKLACE || it->cat == CAT_WINGS) LINE("Nur in Kisten zu finden (nicht herstellbar)");
    if (it->cat != CAT_MATERIAL) { FormatNumber(ItemSellValue(it), b, sizeof b); LINE("Verkaufswert: %s Gold", b); }
#undef LINE
    return c;
}

/* ================================================================== */
/* Wert, Rezepte                                                      */
/* ================================================================== */

static double BaseValue(const Item *it) {
    double t = pow(8.0, it->tier);
    switch (it->cat) {
    case CAT_WEAPON: case CAT_HELM: case CAT_CHEST: case CAT_GLOVES: case CAT_BOOTS:
        return 5.0 * t * (1.0 + 0.3 * it->idx);
    case CAT_OFFHAND: return 5.0 * t * 1.5;
    case CAT_BAG:     return 5.0 * t * 1.2;
    case CAT_RING:    return 5.0 * t * 3.0;
    case CAT_NECKLACE:return 5.0 * t * 6.0;
    case CAT_WINGS:   return 5.0 * t * 20.0;
    case CAT_POTION:  return 5.0 * t * 0.8;
    case CAT_MATERIAL:return 2.0 * pow(7.0, it->tier);
    }
    return 0.0;
}

double ItemSellValue(const Item *it) {
    if (it->cat == CAT_NONE) return 0.0;
    double v = BaseValue(it) * (1.0 + 0.4 * it->level) * 0.35;
    if (it->cat == CAT_MATERIAL) v = BaseValue(it) * 0.35 * (it->count ? it->count : 1);
    return floor(v < 1.0 ? 1.0 : v);
}

static int TierCount(int tier, int idx) {
    switch (tier) {
    case TIER_NORMAL: return 5 + 3 * idx;
    case TIER_MAGIC:  return 4 + 2 * idx;
    case TIER_RARE:   return 3 + 2 * idx;
    case TIER_UNIQUE: return 2 + idx;
    case TIER_BOSS:   return 2;
    default:          return 1;
    }
}
static const int SIMPLE_COUNT[TIER_COUNT] = { 4, 3, 3, 2, 2, 1 };
static const int POTION_COUNT_[TIER_COUNT] = { 3, 3, 2, 2, 2, 1 };

static const int CLASS_MATS[CLASS_COUNT][2] = {
    /* Knight  */ { MAT_ORE,     MAT_LEATHER },
    /* Mage    */ { MAT_WOOD,    MAT_ESSENCE },
    /* Archer  */ { MAT_WOOD,    MAT_LEATHER },
    /* Warrior */ { MAT_ORE,     MAT_WOOD    },
    /* Assassin*/ { MAT_LEATHER, MAT_ORE     },
    /* Shaman  */ { MAT_ESSENCE, MAT_LEATHER },
    /* Priest  */ { MAT_ORE,     MAT_ESSENCE },
};

static void AddMat(Recipe *r, int tier, int kind, int count) {
    if (count < 1) count = 1;
    r->mats[r->matCount++] = ItemMakeMaterial(tier, kind, count);
}

int ItemRecipe(const Item *t, Recipe *r) {
    memset(r, 0, sizeof *r);
    if (!ItemIsCraftable(t)) return 0;
    int tier = t->tier;
    int n;
    int a, b;
    Item base = *t; base.level = 0;
    switch (t->cat) {
    case CAT_WEAPON:
        n = TierCount(tier, t->idx);
        a = CLASS_MATS[t->kind % CLASS_COUNT][0]; b = CLASS_MATS[t->kind % CLASS_COUNT][1];
        AddMat(r, tier, a, n); AddMat(r, tier, b, (n + 1) / 2);
        break;
    case CAT_OFFHAND:
        n = SIMPLE_COUNT[tier];
        a = CLASS_MATS[t->kind % CLASS_COUNT][0]; b = CLASS_MATS[t->kind % CLASS_COUNT][1];
        AddMat(r, tier, a, n); AddMat(r, tier, b, (n + 1) / 2);
        break;
    case CAT_HELM:   n = TierCount(tier, t->idx); AddMat(r, tier, MAT_ORE, n);     AddMat(r, tier, MAT_LEATHER, (n + 1) / 2); break;
    case CAT_CHEST:  n = TierCount(tier, t->idx); AddMat(r, tier, MAT_LEATHER, n); AddMat(r, tier, MAT_ORE, (n + 1) / 2);     break;
    case CAT_GLOVES: n = TierCount(tier, t->idx); AddMat(r, tier, MAT_LEATHER, n); AddMat(r, tier, MAT_WOOD, (n + 1) / 2);    break;
    case CAT_BOOTS:  n = TierCount(tier, t->idx); AddMat(r, tier, MAT_WOOD, n);    AddMat(r, tier, MAT_LEATHER, (n + 1) / 2); break;
    case CAT_BAG:
        n = SIMPLE_COUNT[tier];
        AddMat(r, tier, MAT_LEATHER, n); AddMat(r, tier, MAT_WOOD, (n + 1) / 2);
        break;
    case CAT_POTION: {
        static const int second[POT_COUNT] = { MAT_WOOD, MAT_ORE, MAT_LEATHER, MAT_ORE, MAT_WOOD };
        n = POTION_COUNT_[tier];
        AddMat(r, tier, MAT_ESSENCE, n); AddMat(r, tier, second[t->kind % POT_COUNT], (n + 1) / 2);
        break;
    }
    }
    r->gold = floor(BaseValue(&base) * 0.5);
    return 1;
}

int ItemUpgradeRecipe(const Item *it, Recipe *r) {
    memset(r, 0, sizeof *r);
    if (!ItemCanLevel(it)) return 0;
    Item base = *it; base.level = 0;
    int mat;
    switch (it->cat) {
    case CAT_WEAPON: case CAT_OFFHAND: mat = CLASS_MATS[it->kind % CLASS_COUNT][0]; break;
    case CAT_HELM:   mat = MAT_ORE;     break;
    case CAT_CHEST:  mat = MAT_LEATHER; break;
    case CAT_GLOVES: mat = MAT_LEATHER; break;
    case CAT_BOOTS:  mat = MAT_WOOD;    break;
    default:         mat = MAT_ESSENCE; break;     /* Schmuck */
    }
    AddMat(r, it->tier, mat, 1 + it->level / 2);
    r->gold = floor(BaseValue(&base) * 0.5 * pow(1.35, it->level));
    return 1;
}

/* ================================================================== */
/* Kisten-Loot                                                        */
/* ================================================================== */

#define W_WEAPON 20
#define W_OFFHAND 8
#define W_HELM 8
#define W_CHEST 8
#define W_GLOVES 8
#define W_BOOTS 8
#define W_BAG 6
#define W_POTION 14
#define W_RING 10
#define W_NECKLACE 5       /* halb so oft wie Ringe */
#define W_TOTAL (W_WEAPON+W_OFFHAND+W_HELM+W_CHEST+W_GLOVES+W_BOOTS+W_BAG+W_POTION+W_RING+W_NECKLACE)

#define CHEST_ITEM_PERCENT 15      /* Rest: Baumaterial -> Items sind seltener */
#define CHEST_WINGS_DEN    400     /* Flügel: extrem selten                      */

static int EligibleSlots(int tier, int mapLevel, int *out) {
    int c = 0;
    for (int i = 0; i < TierSlotCount(tier); i++)
        if (KnightReq[TierFirstSlot(tier) + i] <= mapLevel) out[c++] = i;
    return c;
}

int ItemRollFromChest(int tier, int mapLevel, Item *out) {
    if (rand() % CHEST_WINGS_DEN == 0) { *out = ItemMake(CAT_WINGS, tier, 0, 0); return 1; }
    if (rand() % 100 >= CHEST_ITEM_PERCENT) return 0;
    for (int tries = 0; tries < 8; tries++) {
        int r = rand() % W_TOTAL, cat;
        if      ((r -= W_WEAPON)   < 0) cat = CAT_WEAPON;
        else if ((r -= W_OFFHAND)  < 0) cat = CAT_OFFHAND;
        else if ((r -= W_HELM)     < 0) cat = CAT_HELM;
        else if ((r -= W_CHEST)    < 0) cat = CAT_CHEST;
        else if ((r -= W_GLOVES)   < 0) cat = CAT_GLOVES;
        else if ((r -= W_BOOTS)    < 0) cat = CAT_BOOTS;
        else if ((r -= W_BAG)      < 0) cat = CAT_BAG;
        else if ((r -= W_POTION)   < 0) cat = CAT_POTION;
        else if ((r -= W_RING)     < 0) cat = CAT_RING;
        else                            cat = CAT_NECKLACE;
        int slots[8], n;
        switch (cat) {
        case CAT_WEAPON:
            n = EligibleSlots(tier, mapLevel, slots);
            if (n == 0) continue;
            *out = ItemMake(CAT_WEAPON, tier, rand() % CLASS_COUNT, slots[rand() % n]);
            return 1;
        case CAT_HELM: case CAT_CHEST: case CAT_GLOVES: case CAT_BOOTS:
            n = EligibleSlots(tier, mapLevel, slots);
            if (n == 0) continue;
            *out = ItemMake(cat, tier, 0, slots[rand() % n]);
            return 1;
        case CAT_OFFHAND:
            if (TierMinMapLevel(tier) > mapLevel) continue;
            *out = ItemMake(cat, tier, rand() % CLASS_COUNT, 0);
            return 1;
        case CAT_BAG:
            *out = ItemMake(cat, tier, 0, 0);
            return 1;
        case CAT_POTION:
            *out = ItemMake(cat, tier, rand() % POT_COUNT, 0);
            return 1;
        default:   /* Ring / Halskette */
            *out = ItemMake(cat, tier, rand() % JEW_COUNT, 0);
            return 1;
        }
    }
    return 0;
}

Item ChestLoot(int tier, int mapLevel) {
    Item it;
    if (ItemRollFromChest(tier, mapLevel, &it)) return it;
    int count = 1 + rand() % (tier >= TIER_BOSS ? 1 : 3);
    return ItemMakeMaterial(tier, rand() % MAT_COUNT, count);
}

/* ================================================================== */
/* Balancing                                                          */
/* ================================================================== */

double PowerCurve(double level) {
    if (level < 0) level = 0;
    return 5.0 * pow(10.0, 5.69897 * level / 100.0);   /* 5 bei 0, 2.5 Mio bei 100 */
}
double MonsterBaseHP(int l)     { return 6.0 * PowerCurve(l); }
double MonsterBaseAttack(int l) { return 0.5 * PowerCurve(l); }
double MonsterArmorK(int l)     { return 4.0 * PowerCurve(l); }
double GoldPerKill(int l)       { return 0.6 * PowerCurve(l); }
double XpNeeded(int l)          { return 100.0 * pow(10.0, 0.05 * l); }
double XpForKill(int l)         { return 4.0 * pow(10.0, 0.05 * l); }
double PlayerXpNeeded(int l)    { return 300.0 * pow(10.0, 0.05 * l); }

int HeroSlotsForPlayerLevel(int pl) {
    int s = 4 + (pl * 12) / 100;
    return Clampi(s, 4, 16);
}
double HeroBaseHP(int c, int l)     { return 150.0 * CLASSES[c % CLASS_COUNT].hpMul * pow(10.0, 4.6 * l / 100.0); }
double HeroBaseMana(int c, int l)   { return 100.0 * CLASSES[c % CLASS_COUNT].manaMul * pow(10.0, 1.2 * l / 100.0); }
double HeroBaseAttack(int c, int l) { return 3.0 * CLASSES[c % CLASS_COUNT].atkMul * pow(10.0, 3.0 * l / 100.0); }
double ClassBaseMoveSpeed(int c)    { return CLASSES[c % CLASS_COUNT].moveSpeed; }
int    ClassAttackInterval(int c)   { return CLASSES[c % CLASS_COUNT].atkInterval; }
double ClassAttackRange(int c)      { return CLASSES[c % CLASS_COUNT].range; }
int    ClassIsRanged(int c)         { return CLASSES[c % CLASS_COUNT].ranged; }

void FormatNumber(double v, char *buf, int n) {
    if (v < 0) v = 0;
    if (v < 1000.0)            snprintf(buf, n, "%.0f", v);
    else if (v < 1e6)          snprintf(buf, n, "%.1fK", v / 1e3);
    else if (v < 1e9)          snprintf(buf, n, "%.2fM", v / 1e6);
    else if (v < 1e12)         snprintf(buf, n, "%.2fB", v / 1e9);
    else                       snprintf(buf, n, "%.2fT", v / 1e12);
}
