#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../items.h"
#include "../config.h"

static int fails = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

int main(void) {
    CfgInit();
    if (LoadItemConfigs("..") != 0) { printf("FAIL: Konfiguration\n"); return 1; }
    /* Werte aus Strength.ods */
    CHECK(fabs(TableStrength(0, 0) - 5.0) < 1e-9);
    CHECK(fabs(TableStrength(23, 21) - 10237399.7) < 1.0);
    CHECK(fabs(TableStrength(6, 21) - 405.8) < 0.1);
    /* Tier-Layout 7,5,5,5,1,1 = 24 */
    int sum = 0; for (int t = 0; t < TIER_COUNT; t++) sum += TierSlotCount(t);
    CHECK(sum == 24);
    int reqN[] = {0,1,5,10,15,20,25}, reqM[] = {20,25,35,45,50}, reqR[] = {40,50,60,70,80},
        reqU[] = {60,70,80,90,100};
    for (int i = 0; i < 7; i++) { Item w = ItemMake(CAT_WEAPON, TIER_NORMAL, 0, i); CHECK(ItemRequiredLevel(&w) == reqN[i]); }
    for (int i = 0; i < 5; i++) { Item w = ItemMake(CAT_WEAPON, TIER_MAGIC,  0, i); CHECK(ItemRequiredLevel(&w) == reqM[i]); }
    for (int i = 0; i < 5; i++) { Item w = ItemMake(CAT_WEAPON, TIER_RARE,   0, i); CHECK(ItemRequiredLevel(&w) == reqR[i]); }
    for (int i = 0; i < 5; i++) { Item w = ItemMake(CAT_WEAPON, TIER_UNIQUE, 0, i); CHECK(ItemRequiredLevel(&w) == reqU[i]); }
    Item w = ItemMake(CAT_WEAPON, TIER_BOSS, 0, 0);      CHECK(ItemRequiredLevel(&w) == 80);
    w = ItemMake(CAT_WEAPON, TIER_LEGENDARY, 0, 0);      CHECK(ItemRequiredLevel(&w) == 100);
    Item ring = ItemMake(CAT_RING, TIER_BOSS, JEW_LIFE, 0); CHECK(ItemRequiredLevel(&ring) == 1);
    Item wing = ItemMake(CAT_WINGS, TIER_LEGENDARY, 0, 0);  CHECK(ItemRequiredLevel(&wing) == 1);
    Item pot  = ItemMake(CAT_POTION, TIER_BOSS, POT_MANA, 0); CHECK(ItemRequiredLevel(&pot) == 1);

    /* Level 19 -> 21 Sprung */
    Item s = ItemMake(CAT_WEAPON, TIER_NORMAL, 0, 0); s.level = 19;
    CHECK(ItemLevelUp(&s, 0) == 21);
    s.level = 19; CHECK(ItemLevelUp(&s, 99) == 20);
    CHECK(!ItemCanLevel(&s));
    s.level = 21; CHECK(!ItemCanLevel(&s));
    s.level = 5; CHECK(ItemLevelUp(&s, 0) == 6);

    /* Halskette = 2 Ringe */
    Item rg = ItemMake(CAT_RING, TIER_RARE, JEW_WARRIOR, 0), nk = ItemMake(CAT_NECKLACE, TIER_RARE, JEW_WARRIOR, 0);
    ItemStats a, b, c; memset(&a,0,sizeof a); memset(&b,0,sizeof b); memset(&c,0,sizeof c);
    ItemAddStats(&a, &rg); ItemAddStats(&b, &rg); ItemAddStats(&b, &rg); ItemAddStats(&c, &nk);
    CHECK(fabs(b.attackPct - c.attackPct) < 1e-9 && fabs(b.critChance - c.critChance) < 1e-9);

    /* Flügel 10% .. 300% */
    CHECK(WingsSpeedPct(TIER_NORMAL) == 10.0 && WingsSpeedPct(TIER_LEGENDARY) == 300.0);

    /* Craftbarkeit */
    Recipe rc;
    CHECK(!ItemRecipe(&ring, &rc)); CHECK(!ItemRecipe(&wing, &rc));
    Item bw = ItemMake(CAT_WEAPON, TIER_BOSS, CLASS_Mage, 0);
    CHECK(ItemRecipe(&bw, &rc) && rc.mats[0].tier == TIER_BOSS);   /* Boss-Waffe braucht Boss-Material */

    /* Wachstum: Waffen-Stärke monoton in Level und Slot (je Tier) */
    for (int slot = 0; slot < 24; slot++) for (int l = 1; l <= 21; l++) CHECK(TableStrength(slot, l) >= TableStrength(slot, l-1));
    /* Monster wachsen logarithmisch */
    CHECK(MonsterBaseHP(100) > 1e6 * 5 && MonsterBaseHP(1) < 100);
    CHECK(HeroSlotsForPlayerLevel(1) == 4 && HeroSlotsForPlayerLevel(100) == 16);

    /* Kisten-Loot: Tier bleibt erhalten, Mindestlevel wird respektiert */
    srand(1);
    int items = 0, mats = 0;
    for (int i = 0; i < 20000; i++) {
        Item it = ChestLoot(TIER_RARE, 45);
        CHECK(it.tier == TIER_RARE);
        if (it.cat == CAT_MATERIAL) mats++; else items++;
        if (it.cat == CAT_WEAPON || (it.cat >= CAT_HELM && it.cat <= CAT_BOOTS)) CHECK(ItemRequiredLevel(&it) <= 45);
    }
    printf("Kisten: %d Items, %d Materialien (Items seltener: %s)\n", items, mats, items < mats ? "ja" : "NEIN");
    CHECK(items < mats);

    /* Ausgabe der Kurven zur Sichtprüfung */
    char b1[32], b2[32], b3[32];
    for (int l = 1; l <= 100; l += (l == 1 ? 9 : 10)) {
        FormatNumber(MonsterBaseHP(l), b1, 32); FormatNumber(MonsterBaseAttack(l), b2, 32);
        FormatNumber(HeroBaseHP(CLASS_Knight, l), b3, 32);
        printf("Map/Lv %3d: Monster-HP %8s  ATK %8s | Ritter-HP %8s\n", l, b1, b2, b3);
    }
    printf(fails ? "%d FEHLER\n" : "ALLE TESTS OK\n", fails);
    return fails != 0;
}
