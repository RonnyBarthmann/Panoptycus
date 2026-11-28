#include "items.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

TierConf     Tiers[TIER_COUNT];
ClassConf    ClassCfg[CLASS_COUNT];
StrengthConf Str;

/* ================================================================== */
/* Tiers                                                              */
/* ================================================================== */

const char  *TierName(int t)  { return (t >= 0 && t < TIER_COUNT) ? Tiers[t].name : "?"; }
unsigned int TierColor(int t) { return (t >= 0 && t < TIER_COUNT) ? Tiers[t].color : 0x808080; }
unsigned int TierColorDark(int t) { return (t >= 0 && t < TIER_COUNT) ? Tiers[t].colorDark : 0x404040; }
long TierChestDenominator(int t) { return (t >= 0 && t < TIER_COUNT) ? Tiers[t].chestDen : 10; }
int  TierSlotCount(int t) { return (t >= 0 && t < TIER_COUNT) ? Tiers[t].slotCount : 0; }
int  TierFirstSlot(int t) { return (t >= 0 && t < TIER_COUNT) ? Tiers[t].slotFirst : 0; }
int  TierMinMapLevel(int t) { return (t >= 0 && t < TIER_COUNT) ? Tiers[t].slotLevel[0] : 0; }

/* ================================================================== */
/* Namen                                                              */
/* ================================================================== */

typedef struct { const char *name, *desc; } ClassInfo;

static const ClassInfo CLASSES[CLASS_COUNT] = {
    { "Ritter",       "Schwer gepanzerter Frontkämpfer. Hält viel aus und schützt sich mit dem Schild." },
    { "Magier",       "Fernkämpfer mit mächtigen Arkanzaubern. Viel Mana, wenig Leben." },
    { "Bogenschütze", "Schneller Fernkämpfer. Viele Pfeile, hohe Beweglichkeit." },
    { "Krieger",      "Wuchtige Flächenschläge mit schweren Waffen. Langsam, aber verheerend." },
    { "Assassine",    "Blitzschneller Nahkämpfer mit hohen kritischen Treffern und Gift." },
    { "Schamane",     "Beschwört Blitze und Totems. Schwächt Gegner mit Gift und heilt über Zeit." },
    { "Priester",     "Heiler und Lichtmagier. Heilt sich und schützt mit Segen." },
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
    slot  = Clampi(slot, 0, Str.rows > 0 ? Str.rows - 1 : 0);
    level = Clampi(level, 0, ITEM_MAX_LEVEL);
    return Str.table[slot][level];
}

/* 0..1: wo liegt ein Wert logarithmisch zwischen kleinstem und größtem Tabellenwert */
static double LogNorm(double v) {
    double lo = Str.table[0][0];
    double hi = Str.table[Str.rows > 0 ? Str.rows - 1 : 0][ITEM_MAX_LEVEL];
    if (v <= lo) return 0.0;
    if (v >= hi) return 1.0;
    return log(v / lo) / log(hi / lo);
}

static double LevelMul(int level) { return 1.0 + Str.itemLevelBonus * level; }

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
        return Tiers[Clampi(it->tier, 0, TIER_COUNT - 1)].slotLevel[Clampi(it->idx, 0, TierSlotCount(it->tier) - 1)];
    case CAT_OFFHAND:
        return TierMinMapLevel(it->tier);
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
    if (it->level == ITEM_NORMAL_MAX_LEVEL - 1 && rnd < (int)cfg.level_skip_chance) it->level = ITEM_MAX_LEVEL;
    else it->level++;
    return it->level;
}

/* ================================================================== */
/* Werte                                                              */
/* ================================================================== */

double ItemPower(const Item *it) {
    switch (it->cat) {
    case CAT_WEAPON:  return TableStrength(SlotOf(it), it->level) * ClassCfg[it->kind % CLASS_COUNT].atkMul;
    case CAT_HELM: case CAT_CHEST: case CAT_GLOVES: case CAT_BOOTS:
        return TableStrength(SlotOf(it), it->level);
    case CAT_OFFHAND: return TableStrength(TierFirstSlot(it->tier), it->level) * Str.offhandStrength;
    }
    return 0.0;
}

double WingsSpeedPct(int tier) { return Tiers[Clampi(tier, 0, TIER_COUNT - 1)].wingsPct; }

void ItemAddStats(ItemStats *s, const Item *it) {
    double pct = Tiers[Clampi(it->tier, 0, TIER_COUNT - 1)].effectPct * LevelMul(it->level);
    switch (it->cat) {
    case CAT_WEAPON:
        s->attack += ItemPower(it);
        break;
    case CAT_OFFHAND:
        s->defense += ItemPower(it);
        switch (it->kind) {
        case CLASS_Knight:   s->blockPct       += pct * Str.offKnightBlock > Str.offKnightBlockMax ? Str.offKnightBlockMax : pct * Str.offKnightBlock; break;
        case CLASS_Warrior:  s->attackPct      += pct * Str.offWarriorAtk; break;
        case CLASS_Mage:     s->maxManaPct     += pct * Str.offMageMana; break;
        case CLASS_Archer:   s->attackSpeedPct += pct * Str.offArcherAs; break;
        case CLASS_Assassin: s->critChance     += pct * Str.offAssassinCrit; break;
        case CLASS_Shaman:   s->manaRegenPct   += pct * Str.offShamanRegen; break;
        case CLASS_Priest:   s->healPowerPct   += pct * Str.offPriestHeal; break;
        }
        break;
    case CAT_HELM: case CAT_CHEST: case CAT_GLOVES: case CAT_BOOTS: {
        double str = ItemPower(it);
        double n = LogNorm(str);
        s->defense += str * Str.armorWeight[it->cat - CAT_HELM];
        if (it->cat == CAT_HELM)   s->critChance     += Str.helmCritMax * n;
        if (it->cat == CAT_CHEST)  s->capacity       += Tiers[Clampi(it->tier, 0, TIER_COUNT - 1)].chestArmorCap;
        if (it->cat == CAT_GLOVES) s->attackSpeedPct += Str.glovesAtkSpeedMax * n;
        if (it->cat == CAT_BOOTS)  s->moveSpeedPct   += Str.bootsMoveMax * n;
        break;
    }
    case CAT_BAG:
        s->capacity += Tiers[Clampi(it->tier, 0, TIER_COUNT - 1)].bagCap;
        break;
    case CAT_RING: case CAT_NECKLACE: {
        double p = pct * (it->cat == CAT_NECKLACE ? Str.necklaceFactor : 1.0);   /* Halskette = zwei Ringe */
        switch (it->kind) {
        case JEW_WARRIOR:   s->attackPct += p; s->critChance += p * Str.jewWarriorCrit; break;
        case JEW_ASSASSIN:  s->critChance += p * Str.jewAssassinCrit; s->attackSpeedPct += p * Str.jewAssassinAs; break;
        case JEW_ELEMENTAL: s->maxManaPct += p * Str.jewElementalMana; s->manaRegenPct += p * Str.jewElementalRegen; break;
        case JEW_LIFE:      s->maxHpPct += p * Str.jewLifeHp; s->hpRegenPct += p * Str.jewLifeRegen; break;
        }
        break;
    }
    case CAT_WINGS:
        s->moveSpeedPct += WingsSpeedPct(it->tier);
        break;
    }
}

/* ---- Tränke ---- */
double PotionEffectPct(int kind, int tier) {
    tier = Clampi(tier, 0, TIER_COUNT - 1);
    return (kind >= 0 && kind < POT_COUNT) ? Tiers[tier].potEffect[kind] : 0;
}
int PotionDurationFrames(int tier) { return Tiers[Clampi(tier, 0, TIER_COUNT - 1)].potSeconds * 60; }
int PotionUseCost(int tier)        { return Tiers[Clampi(tier, 0, TIER_COUNT - 1)].potCost; }
int PotionKillRefill(int tier)     { return Tiers[Clampi(tier, 0, TIER_COUNT - 1)].potRefill; }

/* ================================================================== */
/* Namen / Beschreibung                                               */
/* ================================================================== */

void ItemName(const Item *it, char *buf, int n) {
    switch (it->cat) {
    case CAT_WEAPON:
        { int sl = SlotOf(it); if (sl < SLOTS_PER_CLASS) snprintf(buf, n, "%s", WEAPON_NAMES[it->kind % CLASS_COUNT][sl]); else snprintf(buf, n, "%s %d", ClassName(it->kind), sl + 1); }
        break;
    case CAT_HELM: case CAT_CHEST: case CAT_GLOVES: case CAT_BOOTS:
        { int sl = SlotOf(it); if (sl < SLOTS_PER_CLASS) snprintf(buf, n, "%s%s", ARMOR_SETS[sl], ARMOR_PARTS[it->cat - CAT_HELM]); else snprintf(buf, n, "Set %d %s", sl + 1, ARMOR_PARTS[it->cat - CAT_HELM]); }
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
    const TierConf *T = &Tiers[Clampi(it->tier, 0, TIER_COUNT - 1)];
    double t = T->valueMul, b = Str.valueBase * t;
    switch (it->cat) {
    case CAT_WEAPON: case CAT_HELM: case CAT_CHEST: case CAT_GLOVES: case CAT_BOOTS:
        return b * (1.0 + Str.valueIdxStep * it->idx);
    case CAT_OFFHAND: return b * Str.valueOffhand;
    case CAT_BAG:     return b * Str.valueBag;
    case CAT_RING:    return b * Str.valueRing;
    case CAT_NECKLACE:return b * Str.valueNeck;
    case CAT_WINGS:   return b * Str.valueWings;
    case CAT_POTION:  return b * Str.valuePotion;
    case CAT_MATERIAL:return T->materialValue;
    }
    return 0.0;
}

double ItemSellValue(const Item *it) {
    if (it->cat == CAT_NONE) return 0.0;
    double v = BaseValue(it) * (1.0 + Str.valueLevel * it->level) * cfg.sell_factor;
    if (it->cat == CAT_MATERIAL) v = BaseValue(it) * cfg.sell_factor * (it->count ? it->count : 1);
    return floor(v < 1.0 ? 1.0 : v);
}

static int TierCount(int tier, int idx) { const TierConf *T = &Tiers[Clampi(tier, 0, TIER_COUNT - 1)]; return T->craftBase + T->craftStep * idx; }
#define SIMPLE_COUNT(t)  (Tiers[Clampi((t), 0, TIER_COUNT - 1)].craftSimple)
#define POTION_COUNT_(t) (Tiers[Clampi((t), 0, TIER_COUNT - 1)].craftPotion)

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
        n = SIMPLE_COUNT(tier);
        a = CLASS_MATS[t->kind % CLASS_COUNT][0]; b = CLASS_MATS[t->kind % CLASS_COUNT][1];
        AddMat(r, tier, a, n); AddMat(r, tier, b, (n + 1) / 2);
        break;
    case CAT_HELM:   n = TierCount(tier, t->idx); AddMat(r, tier, MAT_ORE, n);     AddMat(r, tier, MAT_LEATHER, (n + 1) / 2); break;
    case CAT_CHEST:  n = TierCount(tier, t->idx); AddMat(r, tier, MAT_LEATHER, n); AddMat(r, tier, MAT_ORE, (n + 1) / 2);     break;
    case CAT_GLOVES: n = TierCount(tier, t->idx); AddMat(r, tier, MAT_LEATHER, n); AddMat(r, tier, MAT_WOOD, (n + 1) / 2);    break;
    case CAT_BOOTS:  n = TierCount(tier, t->idx); AddMat(r, tier, MAT_WOOD, n);    AddMat(r, tier, MAT_LEATHER, (n + 1) / 2); break;
    case CAT_BAG:
        n = SIMPLE_COUNT(tier);
        AddMat(r, tier, MAT_LEATHER, n); AddMat(r, tier, MAT_WOOD, (n + 1) / 2);
        break;
    case CAT_POTION: {
        static const int second[POT_COUNT] = { MAT_WOOD, MAT_ORE, MAT_LEATHER, MAT_ORE, MAT_WOOD };
        n = POTION_COUNT_(tier);
        AddMat(r, tier, MAT_ESSENCE, n); AddMat(r, tier, second[t->kind % POT_COUNT], (n + 1) / 2);
        break;
    }
    }
    r->gold = floor(BaseValue(&base) * cfg.craft_gold_factor);
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
    r->gold = floor(BaseValue(&base) * cfg.upgrade_gold_factor * pow(cfg.upgrade_gold_growth, it->level));
    return 1;
}

/* ================================================================== */
/* Kisten-Loot                                                        */
/* ================================================================== */

#define W_WEAPON   cfg.loot_w_weapon
#define W_OFFHAND  cfg.loot_w_offhand
#define W_HELM     cfg.loot_w_helm
#define W_CHEST    cfg.loot_w_chest
#define W_GLOVES   cfg.loot_w_gloves
#define W_BOOTS    cfg.loot_w_boots
#define W_BAG      cfg.loot_w_bag
#define W_POTION   cfg.loot_w_potion
#define W_RING     cfg.loot_w_ring
#define W_NECKLACE cfg.loot_w_necklace
#define W_TOTAL (W_WEAPON+W_OFFHAND+W_HELM+W_CHEST+W_GLOVES+W_BOOTS+W_BAG+W_POTION+W_RING+W_NECKLACE)

static int EligibleSlots(int tier, int mapLevel, int *out) {
    int c = 0;
    for (int i = 0; i < TierSlotCount(tier); i++)
        if (Tiers[tier].slotLevel[i] <= mapLevel) out[c++] = i;
    return c;
}

int ItemRollFromChest(int tier, int mapLevel, Item *out) {
    if (rand() % (int)cfg.chest_wings_den == 0) { *out = ItemMake(CAT_WINGS, tier, 0, 0); return 1; }
    if (rand() % 100 >= (int)cfg.chest_item_percent) return 0;
    for (int tries = 0; tries < 8; tries++) {
        double r = (rand() / (RAND_MAX + 1.0)) * (W_TOTAL); int cat;
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
        int slots[MAX_TIER_SLOTS], n;
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
    return cfg.power_base * pow(10.0, cfg.power_exp * level / 100.0);
}
double MonsterBaseHP(int l)     { return cfg.monster_hp_factor * PowerCurve(l); }
double MonsterBaseAttack(int l) { return cfg.monster_atk_factor * PowerCurve(l); }
double MonsterArmorK(int l)     { return cfg.monster_armor_factor * PowerCurve(l); }
double GoldPerKill(int l)       { return cfg.gold_per_kill_factor * PowerCurve(l); }
double XpNeeded(int l)          { double q = l / 100.0; return cfg.hero_xp_base * pow(10.0, cfg.hero_xp_growth * l + cfg.hero_xp_curve * q * q); }
double XpForKill(int l)         { return cfg.kill_xp_base * pow(10.0, cfg.kill_xp_growth * l); }
double PlayerXpNeeded(int l)    { return cfg.player_xp_base * pow(10.0, cfg.player_xp_growth * l); }

int HeroSlotsForPlayerLevel(int pl) {
    int lo = (int)cfg.hero_slots_start, hi = (int)cfg.hero_slots_max;
    if (hi > 16) hi = 16;
    if (lo < 1) lo = 1;
    if (hi < lo) hi = lo;
    return Clampi(lo + (pl * (hi - lo)) / 100, lo, hi);
}
double HeroBaseHP(int c, int l)     { return cfg.hero_hp_base * ClassCfg[c % CLASS_COUNT].hpMul * pow(10.0, cfg.hero_hp_growth * l / 100.0); }
double HeroBaseMana(int c, int l)   { return cfg.hero_mana_base * ClassCfg[c % CLASS_COUNT].manaMul * pow(10.0, cfg.hero_mana_growth * l / 100.0); }
double HeroBaseAttack(int c, int l) { return cfg.hero_atk_base * ClassCfg[c % CLASS_COUNT].atkMul * pow(10.0, cfg.hero_atk_growth * l / 100.0); }
double ClassBaseMoveSpeed(int c)    { return ClassCfg[c % CLASS_COUNT].moveSpeed; }
int    ClassAttackInterval(int c)   { return ClassCfg[c % CLASS_COUNT].atkInterval; }
double ClassAttackRange(int c)      { return ClassCfg[c % CLASS_COUNT].range; }
int    ClassIsRanged(int c)         { return ClassCfg[c % CLASS_COUNT].ranged; }

void FormatNumber(double v, char *buf, int n) {
    if (v < 0) v = 0;
    if (v < 1000.0)            snprintf(buf, n, "%.0f", v);
    else if (v < 1e6)          snprintf(buf, n, "%.1fK", v / 1e3);
    else if (v < 1e9)          snprintf(buf, n, "%.2fM", v / 1e6);
    else if (v < 1e12)         snprintf(buf, n, "%.2fB", v / 1e9);
    else                       snprintf(buf, n, "%.2fT", v / 1e12);
}

/* ================================================================== */
/* strength.conf / tiers.conf laden                                   */
/* ================================================================== */

static unsigned int ParseColor(ConfDoc *d, int sec, const char *key) {
    const char *v = ConfStr(d, sec, key, 1);
    return v ? (unsigned int)strtoul(v, NULL, 16) : 0;
}

static void LoadStrength(ConfDoc *d) {
    double a[8];
    memset(&Str, 0, sizeof Str);
    if (ConfNums(d, 0, "armor_weight", a, 4, 1) == 4) for (int i = 0; i < 4; i++) Str.armorWeight[i] = a[i];
    else ConfError(d, 0, "armor_weight braucht 4 Werte (Helm Brust Handschuhe Stiefel)");
#define N(field, key) Str.field = ConfNum(d, 0, key);
    N(offhandStrength, "offhand_strength") N(itemLevelBonus, "item_level_bonus")
    N(helmCritMax, "helm_crit_max") N(glovesAtkSpeedMax, "gloves_attack_speed_max") N(bootsMoveMax, "boots_move_speed_max")
    N(offKnightBlock, "offhand_knight_block") N(offKnightBlockMax, "offhand_knight_block_max") N(offWarriorAtk, "offhand_warrior_attack")
    N(offMageMana, "offhand_mage_mana") N(offArcherAs, "offhand_archer_attack_speed") N(offAssassinCrit, "offhand_assassin_crit")
    N(offShamanRegen, "offhand_shaman_mana_regen") N(offPriestHeal, "offhand_priest_heal")
    N(jewWarriorCrit, "jewelry_warrior_crit") N(jewAssassinCrit, "jewelry_assassin_crit") N(jewAssassinAs, "jewelry_assassin_attack_speed")
    N(jewElementalMana, "jewelry_elemental_mana") N(jewElementalRegen, "jewelry_elemental_regen")
    N(jewLifeHp, "jewelry_life_hp") N(jewLifeRegen, "jewelry_life_regen") N(necklaceFactor, "necklace_factor")
    N(valueBase, "value_base") N(valueIdxStep, "value_index_step") N(valueOffhand, "value_offhand") N(valueBag, "value_bag")
    N(valueRing, "value_ring") N(valueNeck, "value_necklace") N(valueWings, "value_wings") N(valuePotion, "value_potion") N(valueLevel, "value_level")
#undef N
    int nc = ConfSectionCount(d, "class");
    if (nc != CLASS_COUNT) ConfError(d, 0, "es müssen genau %d [class]-Abschnitte vorhanden sein (gefunden: %d)", CLASS_COUNT, nc);
    for (int c = 0; c < CLASS_COUNT && c < nc; c++) {
        int s = ConfSection(d, "class", c);
        ClassConf *k = &ClassCfg[c];
        k->hpMul = ConfNum(d, s, "hp"); k->manaMul = ConfNum(d, s, "mana"); k->atkMul = ConfNum(d, s, "attack");
        k->moveSpeed = ConfNum(d, s, "move_speed"); k->range = ConfNum(d, s, "range");
        k->atkInterval = (int)ConfNum(d, s, "attack_interval"); k->ranged = (int)ConfNum(d, s, "ranged");
    }
    int ts = ConfSection(d, "table", 0);
    if (ts < 0) { ConfError(d, 0, "Abschnitt [table] fehlt"); return; }
    int it = 0; const char *v;
    while (ConfNext(d, ts, "row", &it, &v, NULL)) {
        if (Str.rows >= MAX_SLOT_TOTAL) { ConfError(d, 0, "zu viele Tabellenzeilen (max. %d)", MAX_SLOT_TOTAL); break; }
        double r[22];
        int n = ConfParseNums(v, r, 22);
        if (n != ITEM_MAX_LEVEL + 1) { ConfError(d, 0, "Tabellenzeile %d hat %d statt %d Werten", Str.rows + 1, n, ITEM_MAX_LEVEL + 1); continue; }
        memcpy(Str.table[Str.rows++], r, sizeof r);
    }
}

static void LoadTiers(ConfDoc *d) {
    int n = ConfSectionCount(d, "tier");
    if (n != TIER_COUNT) { ConfError(d, 0, "es müssen genau %d [tier]-Abschnitte vorhanden sein (gefunden: %d)", TIER_COUNT, n); return; }
    int first = 0;
    for (int t = 0; t < TIER_COUNT; t++) {
        int s = ConfSection(d, "tier", t);
        TierConf *T = &Tiers[t];
        memset(T, 0, sizeof *T);
        const char *nm = ConfStr(d, s, "name", 1);
        snprintf(T->name, sizeof T->name, "%s", nm ? nm : "?");
        T->color = ParseColor(d, s, "color"); T->colorDark = ParseColor(d, s, "color_dark");
        T->chestDen = (long)ConfNum(d, s, "chest_denominator");
        if (T->chestDen < 1) T->chestDen = 1;
        double lv[MAX_TIER_SLOTS];
        int cnt = ConfNums(d, s, "slot_levels", lv, MAX_TIER_SLOTS, 1);
        if (cnt < 1) { ConfError(d, 0, "tier %d: slot_levels braucht mindestens einen Wert", t + 1); cnt = 1; lv[0] = 0; }
        T->slotCount = cnt; T->slotFirst = first; first += cnt;
        for (int i = 0; i < cnt; i++) T->slotLevel[i] = (int)lv[i];
        T->effectPct = ConfNum(d, s, "effect_pct"); T->wingsPct = ConfNum(d, s, "wings_speed_pct");
        T->bagCap = ConfNum(d, s, "bag_capacity"); T->chestArmorCap = ConfNum(d, s, "chest_armor_capacity");
        double pe[POT_COUNT];
        if (ConfNums(d, s, "potion_effect", pe, POT_COUNT, 1) == POT_COUNT) for (int i = 0; i < POT_COUNT; i++) T->potEffect[i] = pe[i];
        else ConfError(d, 0, "tier %d: potion_effect braucht %d Werte (Heil Mana Tempo Rüstung Angriffstempo)", t + 1, POT_COUNT);
        T->potSeconds = (int)ConfNum(d, s, "potion_seconds"); T->potCost = (int)ConfNum(d, s, "potion_cost"); T->potRefill = (int)ConfNum(d, s, "potion_refill");
        T->craftBase = (int)ConfNum(d, s, "craft_base"); T->craftStep = (int)ConfNum(d, s, "craft_step");
        T->craftSimple = (int)ConfNum(d, s, "craft_simple"); T->craftPotion = (int)ConfNum(d, s, "craft_potion");
        T->valueMul = ConfNum(d, s, "value_mul"); T->materialValue = ConfNum(d, s, "material_value");
        T->monHp = ConfNum(d, s, "monster_hp"); T->monAtk = ConfNum(d, s, "monster_attack"); T->monXp = ConfNum(d, s, "monster_xp");
        T->monGold = ConfNum(d, s, "monster_gold"); T->monSize = ConfNum(d, s, "monster_size");
        T->monSpawnPct = ConfNum(d, s, "monster_spawn_percent"); T->monChestRolls = (int)ConfNum(d, s, "monster_chest_rolls");
        T->shopUnlock = (int)ConfNum(d, s, "shop_unlock_level"); T->shopStockBase = (int)ConfNum(d, s, "shop_stock_base");
        T->shopStockVar = (int)ConfNum(d, s, "shop_stock_random");
    }
}

int LoadItemConfigs(const char *dir) {
    int before = confErrors;
    char path[256];
    ConfDoc d;
    snprintf(path, sizeof path, "%s%sstrength.conf", dir, *dir ? "/" : "");
    if (!ConfLoad(&d, path)) { fprintf(stderr, "Fehler: %s nicht gefunden\n", path); return confErrors++ - before + 1; }
    LoadStrength(&d);
    ConfFree(&d);
    snprintf(path, sizeof path, "%s%stiers.conf", dir, *dir ? "/" : "");
    if (!ConfLoad(&d, path)) { fprintf(stderr, "Fehler: %s nicht gefunden\n", path); return confErrors++ - before + 1; }
    LoadTiers(&d);
    int total = 0; for (int t = 0; t < TIER_COUNT; t++) total += Tiers[t].slotCount;
    if (Str.rows != total) ConfError(&d, 0, "strength.conf hat %d Tabellenzeilen, tiers.conf definiert aber %d Waffen-/Rüstungs-Slots", Str.rows, total);
    ConfFree(&d);
    return confErrors - before;
}
