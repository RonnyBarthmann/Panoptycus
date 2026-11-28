/* Bildschirme: Vollbild-Held, 2x2-Raster, Menü (Helden, Inventar, Handwerk, Shop, Karten).
 * (wird von game.c eingebunden) */

enum { SCR_HERO = 0, SCR_GRID, SCR_MENU };
enum { TAB_HEROES = 0, TAB_INV, TAB_CRAFT, TAB_SHOP, TAB_MAPS, TAB_SETTINGS, TAB_COUNT };

/* Scroll-Zustände der Listen */
static UiScroll scHeroes, scChests, scCraft, scChapters, scStages, scMapHeroes, scSettings;
static char newHeroName[20];

static int  screen = SCR_GRID;
static int  menuTab = TAB_HEROES;
static int  activeHero = 0;
static int  selHero = 0;                 /* im Menü gewählter Held */
static int  selInv = -1;                 /* gewähltes Inventar-Item */
static int  selSlot = -1;                /* gewählter Ausrüstungs-Slot (Helden-Tab) */
static int  pickClass = 0;               /* Klassenwahl-Dialog offen */
static int  confirmDismiss = 0;
static int  craftCat = 0, craftClass = 0, craftPotion = 0, craftSel = -1;
static int  mapGroup = 0, mapSel = -1, mapHeroA = -1, mapHeroB = -1;
static int  shopSellTier = 1;
static char toast[120]; static int toastTimer = 0;

static const char *NAV_NAMES[TAB_COUNT] = { "Helden", "Inventar", "Handwerk", "Shop", "Karten", "Einstellungen" };

#define Toast(...) do { snprintf(toast, sizeof toast, __VA_ARGS__); toastTimer = 200; } while (0)

static const char *EQ_NAMES[EQ_COUNT] = {
    "Waffe", "Sekundär", "Helm", "Brust", "Handschuhe", "Stiefel", "Tasche",
    "Halskette", "Ring", "Ring", "Trank 1", "Trank 2", "Trank 3", "Flügel"
};

static int NextHeroAfter(int h, int dir) {
    for (int i = 1; i <= MAX_HEROES; i++) {
        int k = ((h + dir * i) % MAX_HEROES + MAX_HEROES) % MAX_HEROES;
        if (heroes[k].isHero) return k;
    }
    return h;
}

static int FirstHero(void) { for (int i = 0; i < MAX_HEROES; i++) if (heroes[i].isHero) return i; return -1; }

static int CollectAllChests(void) {
    int total = 0, items = 0;
    for (int h = 0; h < MAX_HEROES; h++) {
        if (!heroes[h].isHero) continue;
        int it = 0; total += CollectChests(h, &it); items += it;
    }
    if (total > 0) Toast("%d Kisten geöffnet (%d Ausrüstungsteile)", total, items);
    else Toast("Keine Kisten zum Einsammeln");
    return total;
}

/* ================================================================== */
/* Navigation / Kopfzeile                                             */
/* ================================================================== */

static void DrawNavBar(int showTabs) {
    UiFill(0, 0, 1920, 78, 0x2B2118);
    double x = 16;
    if (UiButtonEx(x, 10, 150, 56, "Raster", 1, screen == SCR_GRID ? 0xFFE9CC : COL_PARCH_D)) screen = SCR_GRID;
    x += 166;
    if (UiButtonEx(x, 10, 150, 56, "Vollbild", FirstHero() >= 0, screen == SCR_HERO ? 0xFFE9CC : COL_PARCH_D)) { if (!heroes[activeHero].isHero) activeHero = FirstHero(); screen = SCR_HERO; }
    x += 182;
    for (int t = 0; t < TAB_COUNT; t++) {
        if (UiButtonEx(x, 10, t == TAB_SETTINGS ? 190 : 160, 56, NAV_NAMES[t], 1, (screen == SCR_MENU && menuTab == t) ? 0xFFE9CC : COL_PARCH_D)) { screen = SCR_MENU; menuTab = t; }
        x += 172;
    }
    (void)showTabs;
    char a[32]; FormatNumber(player.gold, a, sizeof a);
    UiTextf(1500, 6, 28, 0xFFD34D, "Gold %s", a);
    UiTextf(1500, 38, 24, 0xFFE9CC, "Spieler Lv %d   Helden %d/%d", player.level, HeroCount(), HeroSlots());
    UiBar(1700, 12, 200, 12, player.level >= PLAYER_MAX_LEVEL ? 1 : player.xp / PlayerXpNeeded(player.level), 0xD9A400);
}

static void DrawToast(void) {
    if (toastTimer <= 0) return;
    toastTimer--;
    double w = UiTextWidth(toast, 28) + 40;
    UiFill(960 - w / 2 - 3, 990 - 3, w + 6, 52 + 6, COL_INK);
    UiFill(960 - w / 2, 990, w, 52, COL_PARCH_L);
    UiTextC(960, 998, 28, COL_INK, toast);
}

/* ================================================================== */
/* Vollbild-Held: Eingabe                                             */
/* ================================================================== */

static int HudZone(double x, double y) {
    HudLayout L = HudGet(1.0);
    if (x < 410 && y < 190) return 1;
    if (x > 1500 && y < 110) return 1;
    if (y > L.stripY - 4) return 1;
    if (y > L.skillY && x > L.skillX && x < 1910) return 1;
    return 0;
}

static void HeroScreenInput(int h) {
    Hero *H = &heroes[h];
    HudLayout L = HudGet(1.0);
    double sz = 80;
    if (uiClick && UiHover(1900 - sz, 20, sz, sz))      { H->autopilotEnabled = !H->autopilotEnabled; H->manualTimer = 0; H->xTarget = H->x; H->yTarget = H->y; uiClickUsed = 1; }
    else if (uiClick && UiHover(1900 - 2 * sz - 20, 20, sz, sz)) { screen = SCR_GRID; uiClickUsed = 1; }
    else if (uiClick && UiHover(1900 - 3 * sz - 40, 20, sz, sz)) { screen = SCR_MENU; uiClickUsed = 1; }
    else if (uiClick && uiMY > L.stripY - 4) {
        double bsz = L.stripH - 6; int tog = -1;
        for (int t = 0; t < TIER_COUNT; t++) if (UiHover(20 + t * (bsz + 14), L.stripY + 3, bsz, bsz)) tog = t;
        if (tog >= 0) { H->chestMask ^= 1u << tog; uiClickUsed = 1; }
        else { int n = 0; CollectChests(h, &n); Toast("Kisten von %s eingesammelt", H->name); uiClickUsed = 1; }
    }
    else if (uiClick) {
        for (int s = 0; s < 4; s++) if (UiHover(L.skillX + s * (L.size + L.gap), L.skillY, L.size, L.size)) { HeroUseSkill(h, s); uiClickUsed = 1; }
        for (int s = 0; s < 3; s++) if (UiHover(L.potX + s * (L.size + L.gap), L.potY, L.size, L.size)) { HeroUsePotion(h, s); uiClickUsed = 1; }
    }
    if (uiClickUsed || H->stage < 0 || H->inst < 0 || H->hp <= 0) return;
    if (HudZone(uiMX, uiMY)) return;

    double wx = H->x + (uiMX - 960), wy = H->y + (uiMY - 540);
    Instance *I = &instances[H->inst];
    if (uiClick || uiDown) {
        int mon = -1;
        if (uiClick) for (int i = 0; i < MAX_MONSTERS; i++)
            if (I->mon[i].alive && Dist(wx, wy, I->mon[i].x, I->mon[i].y) < I->mon[i].radius + 14) { mon = i; break; }
        if (mon >= 0) { H->focus = mon; H->manualTimer = 240; }
        else if (uiDown) { H->focus = -1; H->xTarget = wx; H->yTarget = wy; H->manualTimer = 120; }   /* Maus halten = dauerhaft laufen */
    }
    if (uiRClick && !H->attacking) { H->manualTimer = 60; HeroBasicAttack(h, LookAt(H->x, H->y, wx, wy), -1); }
    if (!H->autopilotEnabled || H->manualTimer > 0)
        if (!H->attacking && !H->skilling && H->focus < 0 && !H->walking) H->angle = LookAt(H->x, H->y, wx, wy);
}

/* ================================================================== */
/* Raster-Ansicht (2x2)                                               */
/* ================================================================== */

static const double CELL_X[4] = { 238, 970, 238, 970 };
static const double CELL_Y[4] = { 130, 130, 550, 550 };
#define CELL_W 712
#define CELL_H 400

static int swapCell = -1;     /* Feld, für das gerade ein Held gewählt wird */

static void GridScreen(int *heroCanvas, int canvas) {
    int *lay = player.layouts[player.activeLayout];
    for (int c = 0; c < 4; c++) {
        int h = lay[c];
        DrawOutput(canvas);
        if (h >= 0 && heroes[h].isHero) {
            DrawHeroView(heroCanvas[c], h, 2.0);
            DrawOutput(canvas);
            DrawImageScaled(heroCanvas[c], CELL_X[c], CELL_Y[c], CELL_W, CELL_H);
        } else DrawImageScaled(HeroBG, CELL_X[c], CELL_Y[c], CELL_W, CELL_H);
    }
    DrawOutput(canvas);
    for (int c = 0; c < 4; c++) {
        int h = lay[c];
        double x = CELL_X[c], y = CELL_Y[c];
        if (h < 0 || !heroes[h].isHero) UiTextC(x + CELL_W / 2, y + CELL_H / 2 - 16, 32, 0xFFE9CC, "Leer - Held wählen");
        int inCell = UiHover(x, y, CELL_W, CELL_H);
        /* Wechsel- und Einsammeln-Knöpfe */
        int en = swapCell < 0;
        if (UiButtonEx(x + CELL_W - 128, y + 6, 122, 34, "Wechseln", en, COL_PARCH_D)) swapCell = c;
        if (h >= 0 && heroes[h].isHero) {
            int tot = HeroChestTotal(h);
            if (UiButtonEx(x + CELL_W - 166, y + 46, 160, 34, "Einsammeln", en && tot > 0, tot > 0 ? 0xFFD34D : COL_PARCH_D)) { int n; CollectChests(h, &n); Toast("%s: Kisten eingesammelt", heroes[h].name); }
            if (en && inCell && uiClick && !uiClickUsed) { activeHero = h; screen = SCR_HERO; uiClickUsed = 1; }
        }
    }
    /* Layout-Reiter unter dem Raster */
    for (int l = 0; l < LAYOUT_COUNT; l++) {
        char b[32]; snprintf(b, sizeof b, "Anordnung %d", l + 1);
        if (UiButtonEx(238 + l * 190, 975, 176, 46, b, swapCell < 0, l == player.activeLayout ? 0xFFE9CC : COL_PARCH_D)) player.activeLayout = l;
    }
    UiText(1010, 984, 22, COL_INK, "Klick auf ein Fenster = Vollbild");
    /* Heldenwahl für ein Feld */
    if (swapCell >= 0) {
        int cur = lay[swapCell];
        UiFillA(0, 78, 1920, 1002, 0x000000, 150);
        UiPanel(520, 150, 880, 760);
        UiPanelTitle(520, 150, 880, "Held für dieses Feld wählen");
        int row = 0;
        for (int k = 0; k < MAX_HEROES; k++) {
            if (!heroes[k].isHero) continue;
            double bx = 544 + (row % 2) * 420, by = 210 + (row / 2) * 58;
            char lab[64]; int shownAt = -1;
            for (int q = 0; q < 4; q++) if (lay[q] == k && q != swapCell) shownAt = q;
            if (shownAt >= 0) snprintf(lab, sizeof lab, "%s Lv %d (Feld %d)", heroes[k].name, heroes[k].level, shownAt + 1);
            else snprintf(lab, sizeof lab, "%s Lv %d", heroes[k].name, heroes[k].level);
            UiFill(bx - 6, by, 6, 50, CLASS_COLORS[heroes[k].class]);
            if (UiButtonEx(bx, by, 394, 50, lab, 1, k == cur ? 0xFFE9CC : COL_PARCH_D)) {
                if (shownAt >= 0) lay[shownAt] = cur;           /* Held war schon woanders: tauschen */
                lay[swapCell] = k; swapCell = -1;
                break;
            }
            row++;
        }
        if (swapCell >= 0 && UiButtonEx(544, 800, 394, 56, "Feld leeren", 1, 0xE8B0A8)) { lay[swapCell] = -1; swapCell = -1; }
        if (swapCell >= 0 && UiButton(964, 800, 394, 56, "Abbrechen")) swapCell = -1;
    }
}

/* ================================================================== */
/* Helden-Tab                                                         */
/* ================================================================== */

static int SlotCategoryOK(int slot, const Item *it, int cls) {
    switch (slot) {
    case EQ_WEAPON:  return it->cat == CAT_WEAPON && it->kind == cls;
    case EQ_OFFHAND: return it->cat == CAT_OFFHAND && it->kind == cls;
    case EQ_HELM:    return it->cat == CAT_HELM;
    case EQ_CHEST:   return it->cat == CAT_CHEST;
    case EQ_GLOVES:  return it->cat == CAT_GLOVES;
    case EQ_BOOTS:   return it->cat == CAT_BOOTS;
    case EQ_BAG:     return it->cat == CAT_BAG;
    case EQ_NECK:    return it->cat == CAT_NECKLACE;
    case EQ_RING1: case EQ_RING2: return it->cat == CAT_RING;
    case EQ_POT1: case EQ_POT2: case EQ_POT3: return it->cat == CAT_POTION;
    case EQ_WINGS:   return it->cat == CAT_WINGS;
    }
    return 0;
}

static void DrawStatLine(double x, double y, const char *label, const char *value) {
    UiText(x, y, 22, COL_MUTED, label);
    UiTextR(x + 300, y, 22, COL_INK, value);
}

static void ClassPickDialog(void) {
    UiFill(0, 78, 1920, 1002, 0x000000 | 0); UiFillA(0, 78, 1920, 1002, 0x000000, 150);
    UiPanel(360, 160, 1200, 760);
    UiPanelTitle(360, 160, 1200, "Neuen Helden erstellen - Klasse wählen");
    UiTextField(5002, 390, 214, 220, 360, "Name (leer = automatisch)", newHeroName, (int)sizeof newHeroName);
    for (int c = 0; c < CLASS_COUNT; c++) {
        double x = 390 + (c % 4) * 292, y = 270 + (c / 4) * 250;
        UiFill(x - 3, y - 3, 270 + 6, 230 + 6, COL_INK);
        UiFill(x, y, 270, 230, UiHover(x, y, 270, 230) ? COL_PARCH_L : COL_PARCH_D);
        UiFill(x, y, 270, 8, CLASS_COLORS[c]);
        DrawImageScaled(UiIcon[WEAPON_ICON[c]], x + 99, y + 18, 72, 72);
        UiTextC(x + 135, y + 96, 30, COL_INK, ClassName(c));
        /* Beschreibung in 2-3 Zeilen umbrechen */
        const char *d = ClassDescription(c); char line[64]; int n = 0; double ly = y + 134;
        while (*d && ly < y + 215) {
            const char *sp = d; int len = 0, last = 0;
            while (sp[len] && len < 40) { if (sp[len] == ' ') last = len; len++; }
            if (sp[len] && last > 0) len = last;
            n = len < 63 ? len : 63; memcpy(line, d, n); line[n] = 0;
            UiTextC(x + 135, ly, 18, COL_INK, line);
            d += len; while (*d == ' ') d++; ly += 22;
        }
        if (UiHover(x, y, 270, 230) && uiClick && !uiClickUsed) {
            uiClickUsed = 1;
            int h = CreateHero(c);
            if (h >= 0) {
                if (newHeroName[0]) snprintf(heroes[h].name, sizeof heroes[h].name, "%s", newHeroName);
                StartStage(h, 0, -1); selHero = h; Toast("%s erstellt", heroes[h].name);
            }
            else Toast("Kein freier Heldenplatz");
            pickClass = 0;
        }
    }
    if (UiButton(1380, 860, 160, 46, "Abbrechen")) pickClass = 0;
}

static void HeroesTab(void) {
    if (!heroes[selHero].isHero) selHero = FirstHero() < 0 ? 0 : FirstHero();
    /* Heldenliste */
    UiPanel(20, 96, 520, 960);
    UiPanelTitle(20, 96, 520, "Helden");
    int row = 0;
    double off = UiScrollBegin(&scHeroes, 24, 142, 512, 808, HeroCount() * 66 + 8);
    for (int h = 0; h < MAX_HEROES; h++) {
        if (!heroes[h].isHero) continue;
        double y = 148 + row * 66 - off;
        int sel = (h == selHero);
        UiFill(28, y, 504, 60, sel ? 0xFFE9CC : COL_PARCH_D);
        UiFill(28, y, 10, 60, CLASS_COLORS[heroes[h].class]);
        UiTextf(50, y + 4, 26, COL_INK, "%s", heroes[h].name);
        UiTextf(50, y + 33, 20, COL_MUTED, "Lv %d  %s", heroes[h].level, heroes[h].stage >= 0 ? Stages[heroes[h].stage].name : "im Lager");
        if (heroes[h].hp <= 0) UiTextR(520, y + 4, 22, COL_BAD, "gefallen");
        else if (HeroChestTotal(h) > 0) UiTextf(400, y + 4, 22, COL_GOLD, "%d Kisten", HeroChestTotal(h));
        if (UiHover(28, y, 504, 60) && uiClick && !uiClickUsed) { selHero = h; selSlot = -1; confirmDismiss = 0; uiClickUsed = 1; }
        row++;
    }
    UiScrollEnd();
    if (UiButtonEx(36, 996, 488, 46, "+ Neuen Helden erstellen", HeroCount() < HeroSlots(), 0xBFE3A8)) { pickClass = 1; newHeroName[0] = 0; }
    if (HeroCount() >= HeroSlots()) UiText(40, 960, 20, COL_MUTED, "Mehr Plätze: Spieler-Level steigern");

    if (!heroes[selHero].isHero) { UiText(600, 200, 32, COL_INK, "Noch kein Held. Erstelle deinen ersten Helden!"); return; }
    Hero *H = &heroes[selHero];

    /* Kopf */
    UiPanel(560, 96, 1340, 130);
    UiFill(560, 96, 14, 130, CLASS_COLORS[H->class]);
    UiTextField(5001, 590, 100, 0, 330, "", H->name, (int)sizeof H->name);
    UiTextf(590, 150, 24, COL_MUTED, "%s - Level %d", ClassName(H->class), H->level);
    UiBar(590, 190, 420, 16, H->level >= HERO_MAX_LEVEL ? 1 : H->xp / XpNeeded(H->level), 0xD9A400);
    { char a[32], b[32]; FormatNumber(H->xp, a, sizeof a); FormatNumber(XpNeeded(H->level), b, sizeof b); UiTextf(1020, 186, 20, COL_INK, "EP %s / %s", a, b); }
    if (UiButton(1240, 108, 200, 46, "Vollbild")) { activeHero = selHero; screen = SCR_HERO; }
    if (UiButtonEx(1456, 108, 200, 46, H->autopilotEnabled ? "Autopilot: AN" : "Autopilot: AUS", 1, H->autopilotEnabled ? 0xBFE3A8 : 0xE8B0A8)) { H->autopilotEnabled = !H->autopilotEnabled; H->manualTimer = 0; }
    if (UiButton(1672, 108, 214, 46, "Karte wählen")) { screen = SCR_MENU; menuTab = TAB_MAPS; mapHeroA = selHero; }
    if (UiButton(1240, 166, 200, 46, H->autoNext ? "Auto-Weiter: AN" : "Auto-Weiter: AUS")) H->autoNext = !H->autoNext;
    int equipEmpty = 1;
    for (int s = 0; s < EQ_COUNT; s++) if (!ItemIsEmpty(&H->equip[s])) equipEmpty = 0;
    if (!equipEmpty) { confirmDismiss = 0; UiText(590, 208, 16, COL_MUTED, "Entlassen nur mit leerer Ausrüstung (alles ablegen)"); }
    if (UiButtonEx(1456, 166, 200, 46, confirmDismiss ? "Wirklich?" : "Entlassen", HeroCount() > 1 && equipEmpty, confirmDismiss ? 0xE8B0A8 : COL_PARCH_D)) {
        if (!confirmDismiss) confirmDismiss = 1;
        else {
            LeaveInstance(selHero); ClearHero(selHero);
            for (int l = 0; l < LAYOUT_COUNT; l++) for (int c = 0; c < 4; c++) if (player.layouts[l][c] == selHero) player.layouts[l][c] = -1;
            selHero = FirstHero(); confirmDismiss = 0; return;
        }
    }
    UiText(930, 104, 18, COL_MUTED, "Sammelt Kisten (Klick = an/aus)");
    for (int t = 0; t < TIER_COUNT; t++) {
        double tx = 930 + t * 46, ty = 130;
        int on = (H->chestMask >> t) & 1;
        UiFill(tx - 2, ty - 2, 40, 40, COL_INK);
        UiFill(tx, ty, 36, 36, on ? TierColor(t) : 0x8A8278);
        if (!on) UiText(tx + 8, ty + 2, 30, 0x3A332B, "x");
        if (UiHover(tx, ty, 36, 36)) {
            UiFrame(tx - 3, ty - 3, 42, 42, 2, 0xFFFFFF);
            if (uiClick && !uiClickUsed) { uiClickUsed = 1; H->chestMask ^= 1u << t; }
        }
    }
    if (UiButton(1672, 166, 214, 46, "Kisten holen")) { int n; CollectChests(selHero, &n); Toast("Kisten von %s eingesammelt", H->name); }

    /* Ausrüstung */
    UiPanel(560, 246, 640, 520);
    UiPanelTitle(560, 246, 640, "Ausrüstung");
    static const int LAYOUT_SLOTS[4][4] = {
        { EQ_HELM, EQ_CHEST, EQ_GLOVES, EQ_BOOTS },
        { EQ_WEAPON, EQ_OFFHAND, EQ_BAG, EQ_WINGS },
        { EQ_NECK, EQ_RING1, EQ_RING2, -1 },
        { EQ_POT1, EQ_POT2, EQ_POT3, -1 } };
    for (int r = 0; r < 4; r++) for (int c = 0; c < 4; c++) {
        int slot = LAYOUT_SLOTS[r][c];
        if (slot < 0) continue;
        double x = 596 + c * 150, y = 306 + r * 110;
        int res = UiItemSlot(x, y, 84, &H->equip[slot], slot == selSlot);
        UiTextC(x + 42, y + 86, 18, COL_MUTED, EQ_NAMES[slot]);
        if (res == 1) selSlot = (selSlot == slot) ? -1 : slot;
        if (res == 2 && !ItemIsEmpty(&H->equip[slot])) { if (HeroUnequip(selHero, slot)) Toast("Abgelegt"); else Toast("Inventar voll"); }
    }
    /* Werte */
    UiPanel(1220, 246, 680, 520);
    UiPanelTitle(1220, 246, 680, "Werte");
    char b[48]; double y = 300;
    FormatNumber(H->atk, b, sizeof b); DrawStatLine(1244, y, "Angriff", b); y += 30;
    FormatNumber(H->def, b, sizeof b); DrawStatLine(1244, y, "Rüstung", b); y += 30;
    FormatNumber(H->hpMax, b, sizeof b); DrawStatLine(1244, y, "Leben", b); y += 30;
    FormatNumber(H->manaMax, b, sizeof b); DrawStatLine(1244, y, "Mana", b); y += 30;
    snprintf(b, sizeof b, "%.1f%% (x%.1f)", H->crit, H->critMul); DrawStatLine(1244, y, "Krit. Treffer", b); y += 30;
    snprintf(b, sizeof b, "%.0f%%", H->atkSpeed * 100); DrawStatLine(1244, y, "Angriffstempo", b); y += 30;
    snprintf(b, sizeof b, "%.1f", H->moveSpeed); DrawStatLine(1244, y, "Lauftempo", b); y += 30;
    snprintf(b, sizeof b, "%.1f / s", H->manaRegen); DrawStatLine(1244, y, "Manaregeneration", b); y += 30;
    snprintf(b, sizeof b, "%.1f / s", H->hpRegen); DrawStatLine(1244, y, "Lebensregeneration", b); y += 30;
    snprintf(b, sizeof b, "%d", H->capacity); DrawStatLine(1244, y, "Kistenkapazität", b); y += 30;
    snprintf(b, sizeof b, "%.0f%%", H->block); DrawStatLine(1244, y, "Schadensreduktion", b); y += 30;
    snprintf(b, sizeof b, "%ld", H->kills); DrawStatLine(1244, y, "Besiegte Gegner", b);
    /* Aktionen für den gewählten Slot */
    if (selSlot >= 0 && !ItemIsEmpty(&H->equip[selSlot])) {
        Recipe r;
        int can = ItemUpgradeRecipe(&H->equip[selSlot], &r);
        char lab[64]; snprintf(lab, sizeof lab, can ? "Verbessern (%s Gold)" : "Verbessern", can ? (FormatNumber(r.gold, b, sizeof b), b) : "-");
        if (UiButtonEx(1560, 590, 320, 46, lab, can && RecipeAffordable(&r), 0xBFE3A8)) { UpgradeItemPtr(&H->equip[selSlot]); RecalcHero(selHero); Toast("Item verbessert: +%d", H->equip[selSlot].level); }
        if (can) for (int i = 0; i < r.matCount; i++) {
            char nm[48]; ItemName(&r.mats[i], nm, sizeof nm);
            int have = InvMaterialCount(r.mats[i].tier, r.mats[i].kind);
            UiTextf(1244, 640 + i * 26, 20, have >= r.mats[i].count ? COL_INK : COL_BAD, "%dx %s (hast %d)", r.mats[i].count, nm, have);
        }
    }
    /* Skills */
    UiPanel(560, 786, 1340, 270);
    UiPanelTitle(560, 786, 1340, "Skills (bis zu 4 zugewiesen - Tasten Q W E R)");
    for (int i = 0; i < SKILLS_PER_CLASS; i++) {
        int id = SKILL_ID(H->class, i);
        const SkillDef *sk = &SkillDefs[id];
        double x = 580 + (i % 3) * 440, yy = 836 + (i / 3) * 100;
        int unlocked = SkillUnlocked(selHero, id), assigned = -1;
        for (int s = 0; s < 4; s++) if (H->skillSlots[s] == id) assigned = s;
        UiFill(x - 2, yy - 2, 424, 90, COL_INK);
        UiFill(x, yy, 420, 86, assigned >= 0 ? 0xE0F0C8 : (unlocked ? COL_PARCH_L : 0xBDB3A3));
        UiText(x + 8, yy + 3, 24, unlocked ? COL_INK : COL_MUTED, sk->name);
        UiText(x + 8, yy + 32, 18, COL_MUTED, sk->desc);
        if (unlocked) UiTextf(x + 8, yy + 58, 18, COL_INK, "Mana %d   Abklingzeit %.1fs", SkillManaCost(selHero, sk), sk->cd / 60.0);
        else UiTextf(x + 8, yy + 58, 18, COL_BAD, "Ab Level %d", sk->unlock);
        if (assigned >= 0) UiTextR(x + 410, yy + 3, 26, COL_GOOD, assigned == 0 ? "[Q]" : (assigned == 1 ? "[W]" : (assigned == 2 ? "[E]" : "[R]")));
        if (selSlot < 0 && unlocked && UiHover(x, yy, 420, 86) && uiClick && !uiClickUsed) {
            uiClickUsed = 1;
            if (assigned >= 0) H->skillSlots[assigned] = -1;
            else {
                int placed = 0;
                for (int s = 0; s < 4 && !placed; s++) if (H->skillSlots[s] < 0) { H->skillSlots[s] = id; placed = 1; }
                if (!placed) Toast("Alle 4 Skill-Plätze belegt - erst einen abwählen");
            }
        }
    }
    /* Auswahl-Liste für den gewählten Slot */
    if (selSlot >= 0) {
        double px = 596, py = 790;
        UiFill(556, 782, 1348, 278, COL_INK); UiFill(560, 786, 1340, 270, COL_PARCH);
        UiPanelTitle(560, 786, 1340, "Passende Gegenstände im Inventar");
        int n = 0;
        for (int i = 0; i < MAX_INV && n < 18; i++) {
            Item *it = &player.inv[i];
            if (ItemIsEmpty(it) || !SlotCategoryOK(selSlot, it, H->class)) continue;
            double x = px + (n % 9) * 140, yy = py + 56 + (n / 9) * 100;
            const char *prob = HeroEquipProblem(selHero, it);
            int res = UiItemSlot(x, yy, 72, it, 0);
            if (prob) { UiFillA(x, yy, 72, 72, 0x000000, 130); UiText(x + 78, yy + 4, 16, COL_BAD, "Lv"); UiTextf(x + 78, yy + 24, 18, COL_BAD, "%d", ItemRequiredLevel(it)); }
            else UiTextf(x + 78, yy + 4, 18, COL_INK, "Lv %d", ItemRequiredLevel(it));
            if (res == 1) { if (prob) Toast("%s", prob); else { HeroEquip(selHero, i, selSlot); Toast("Angelegt"); } }
            n++;
        }
        if (n == 0) UiText(600, 860, 24, COL_MUTED, "Nichts Passendes im Inventar (Handwerk oder Kisten).");
        if (UiButton(1700, 1000, 180, 40, "Schließen")) selSlot = -1;
    }
    if (pickClass) ClassPickDialog();
}

/* ================================================================== */
/* Inventar-Tab                                                       */
/* ================================================================== */

#define INV_COLS 14
#define INV_SIZE 60

static void InventoryTab(void) {
    UiPanel(20, 96, INV_COLS * 70 + 28, 960);
    UiPanelTitle(20, 96, INV_COLS * 70 + 28, "Inventar");
    UiTextf(300, 102, 22, COL_INK, "%d / %d belegt", MAX_INV - InvFreeSlots(), MAX_INV);
    for (int i = 0; i < MAX_INV; i++) {
        double x = 34 + (i % INV_COLS) * 70, y = 152 + (i / INV_COLS) * 70;
        int res = UiItemSlot(x, y, INV_SIZE, &player.inv[i], i == selInv);
        if (res == 1) selInv = (selInv == i) ? -1 : i;
        if (res == 2 && !ItemIsEmpty(&player.inv[i]) && heroes[selHero].isHero) {
            if (!HeroEquipProblem(selHero, &player.inv[i])) { HeroEquip(selHero, i, -1); Toast("Angelegt"); }
            else Toast("%s", HeroEquipProblem(selHero, &player.inv[i]));
        }
    }
    UiText(34, 1018, 18, COL_MUTED, "Linksklick: auswählen   Rechtsklick: an gewählten Helden anlegen");

    double rx = 20 + INV_COLS * 70 + 56, rw = 1900 - rx;
    /* gewähltes Item */
    UiPanel(rx, 96, rw, 400);
    UiPanelTitle(rx, 96, rw, "Gegenstand");
    if (selInv >= 0 && !ItemIsEmpty(&player.inv[selInv])) {
        Item *it = &player.inv[selInv];
        char nm[64], lines[14][72]; ItemName(it, nm, sizeof nm);
        UiItemIcon(rx + 14, 150, 90, it);
        UiText(rx + 120, 150, 28, COL_INK, nm);
        int n = ItemDescribe(it, lines, 14);
        for (int i = 0; i < n && i < 9; i++) UiText(rx + 120, 186 + i * 24, 19, i == 0 ? COL_MUTED : COL_INK, lines[i]);
        Recipe r; int can = ItemUpgradeRecipe(it, &r);
        double by = 410;
        char lab[64], b[32]; if (can) FormatNumber(r.gold, b, sizeof b);
        snprintf(lab, sizeof lab, can ? "Verbessern (%s G)" : "Verbessern", can ? b : "-");
        if (UiButtonEx(rx + 14, by, 230, 44, lab, can && RecipeAffordable(&r), 0xBFE3A8)) { UpgradeItemPtr(it); Toast("Item verbessert: +%d", it->level); }
        FormatNumber(ItemSellValue(it), b, sizeof b); snprintf(lab, sizeof lab, "Verkaufen (+%s)", b);
        if (UiButton(rx + 258, by, 230, 44, lab)) { SellItem(selInv); selInv = -1; }
        const char *prob = heroes[selHero].isHero ? HeroEquipProblem(selHero, it) : "Kein Held";
        snprintf(lab, sizeof lab, "Anlegen: %s", heroes[selHero].isHero ? heroes[selHero].name : "-");
        if (UiButtonEx(rx + 14, by - 54, 474, 44, lab, prob == NULL, 0xA8D0F0)) { HeroEquip(selHero, selInv, -1); selInv = -1; Toast("Angelegt"); }
        else if (prob && it->cat != CAT_MATERIAL) UiText(rx + 500, by - 44, 18, COL_BAD, prob);
    } else UiText(rx + 14, 160, 22, COL_MUTED, "Gegenstand anklicken");

    /* Kisten */
    UiPanel(rx, 516, rw, 420);
    UiPanelTitle(rx, 516, rw, "Kisten der Helden");
    int row = 0;
    double off = UiScrollBegin(&scChests, rx + 4, 560, rw - 8, 312, HeroCount() * 42 + 4);
    for (int h = 0; h < MAX_HEROES; h++) {
        if (!heroes[h].isHero) continue;
        double y = 570 + row * 42 - off;
        UiTextf(rx + 14, y + 6, 22, COL_INK, "%s", heroes[h].name);
        double x = rx + 200;
        for (int t = 0; t < TIER_COUNT; t++) {
            UiFill(x, y + 2, 26, 30, TierColor(t));
            UiTextf(x + 30, y + 6, 20, COL_INK, "%d", heroes[h].chests[t]);
            x += 70;
        }
        UiTextf(x + 4, y + 6, 20, COL_MUTED, "%d/%d", HeroChestTotal(h), heroes[h].capacity);
        row++;
    }
    UiScrollEnd();
    if (UiButton(rx + 14, 880, rw - 28, 46, "Alle Kisten einsammeln (G)")) CollectAllChests();

    UiPanel(rx, 956, rw, 100);
    if (UiButton(rx + 14, 976, (rw - 42) / 2, 56, "Sortieren")) { InvSort(); selInv = -1; }
    UiTextf(rx + 24 + (rw - 42) / 2, 986, 20, COL_MUTED, "Material wird gestapelt.");
    UiTextf(rx + 24 + (rw - 42) / 2, 1010, 18, COL_MUTED, "Verkauf & Einkauf: Shop");
}

/* ================================================================== */
/* Handwerk-Tab                                                       */
/* ================================================================== */

static const int CRAFT_CATS[8] = { CAT_WEAPON, CAT_OFFHAND, CAT_HELM, CAT_CHEST, CAT_GLOVES, CAT_BOOTS, CAT_BAG, CAT_POTION };
static const char *CRAFT_NAMES[8] = { "Waffen", "Sekundärwaffen", "Helme", "Brustpanzer", "Handschuhe", "Stiefel", "Taschen", "Tränke" };

static Item CraftItemAt(int tier, int idx) {
    int cat = CRAFT_CATS[craftCat];
    if (cat == CAT_WEAPON || cat == CAT_OFFHAND) return ItemMake(cat, tier, craftClass, idx);
    if (cat == CAT_POTION) return ItemMake(cat, tier, craftPotion, 0);
    return ItemMake(cat, tier, 0, idx);
}

static void CraftingTab(void) {
    UiPanel(20, 96, 300, 960);
    UiPanelTitle(20, 96, 300, "Kategorie");
    for (int i = 0; i < 8; i++)
        if (UiButtonEx(34, 152 + i * 62, 272, 52, CRAFT_NAMES[i], 1, i == craftCat ? 0xFFE9CC : COL_PARCH_D)) { craftCat = i; craftSel = -1; }
    UiText(34, 670, 18, COL_MUTED, "Ringe, Halsketten und Flügel");
    UiText(34, 694, 18, COL_MUTED, "kann man nicht herstellen -");
    UiText(34, 718, 18, COL_MUTED, "sie fallen nur aus Kisten.");
    UiText(34, 760, 18, COL_MUTED, "Item-Verbesserung: im Inventar");
    UiText(34, 784, 18, COL_MUTED, "oder am Helden.");

    UiPanel(340, 96, 1000, 960);
    UiPanelTitle(340, 96, 1000, CRAFT_NAMES[craftCat]);
    int cat = CRAFT_CATS[craftCat];
    /* Klassen- bzw. Trank-Auswahl */
    if (cat == CAT_WEAPON || cat == CAT_OFFHAND)
        for (int c = 0; c < CLASS_COUNT; c++) if (UiButtonEx(356 + c * 138, 150, 130, 40, ClassName(c), 1, c == craftClass ? 0xFFE9CC : COL_PARCH_D)) { craftClass = c; craftSel = -1; }
    if (cat == CAT_POTION)
        for (int p = 0; p < POT_COUNT; p++) if (UiButtonEx(356 + p * 190, 150, 184, 40, PotionName(p), 1, p == craftPotion ? 0xFFE9CC : COL_PARCH_D)) { craftPotion = p; craftSel = -1; }
    int single = (cat == CAT_OFFHAND || cat == CAT_BAG || cat == CAT_POTION);
    double content = 0;
    for (int t = 0; t < TIER_COUNT; t++) { int sl = single ? 1 : TierSlotCount(t); content += 44 + ((sl + 6) / 7) * 94 + 10; }
    double off = UiScrollBegin(&scCraft, 350, 208, 980, 840, content);
    double yTop = 214 - off;
    for (int t = 0; t < TIER_COUNT; t++) {
        int slots = single ? 1 : TierSlotCount(t);
        double blockH = 44 + ((slots + 6) / 7) * 94;
        double y = yTop;
        UiFill(356, y, 950, blockH, Mix(TierColor(t), 0xFFFFFF, 0.55));
        UiFill(356, y, 10, blockH, TierColor(t));
        UiText(376, y + 4, 24, COL_INK, TierName(t));
        for (int i = 0; i < slots; i++) {
            Item it = CraftItemAt(t, i);
            double x = 376 + (i % 7) * 128, yy = y + 36 + (i / 7) * 94;
            int id = t * 100 + i;
            int res = UiItemSlot(x, yy, 76, &it, craftSel == id);
            UiTextf(x, yy + 78, 16, COL_MUTED, single ? "" : "Lv %d", ItemRequiredLevel(&it));
            Recipe r;
            if (ItemRecipe(&it, &r) && RecipeAffordable(&r)) UiFillA(x + 62, yy - 6, 14, 14, 0x40D060, 255);
            if (res == 1) craftSel = id;
        }
        yTop += blockH + 10;
    }
    UiScrollEnd();
    /* Rezept */
    UiPanel(1360, 96, 540, 960);
    UiPanelTitle(1360, 96, 540, "Rezept");
    if (craftSel >= 0) {
        int t = craftSel / 100, i = craftSel % 100;
        Item it = CraftItemAt(t, i);
        char nm[64], lines[14][72]; ItemName(&it, nm, sizeof nm);
        UiItemIcon(1380, 150, 90, &it);
        UiText(1486, 154, 26, COL_INK, nm);
        int n = ItemDescribe(&it, lines, 14);
        for (int k = 1; k < n && k < 10; k++) UiText(1486, 188 + (k - 1) * 22, 18, COL_INK, lines[k]);
        Recipe r; ItemRecipe(&it, &r);
        double y = 440;
        UiText(1380, y, 24, COL_INK, "Benötigt:"); y += 36;
        for (int k = 0; k < r.matCount; k++) {
            Item m = r.mats[k]; m.count = 1;
            int have = InvMaterialCount(m.tier, m.kind);
            UiItemIcon(1380, y, 56, &m);
            char mn[48]; ItemName(&m, mn, sizeof mn);
            UiTextf(1450, y + 4, 22, COL_INK, "%dx %s", r.mats[k].count, mn);
            UiTextf(1450, y + 28, 20, have >= r.mats[k].count ? COL_GOOD : COL_BAD, "im Inventar: %d", have);
            y += 70;
        }
        char b[32]; FormatNumber(r.gold, b, sizeof b);
        UiTextf(1380, y + 4, 24, player.gold >= r.gold ? COL_INK : COL_BAD, "Gold: %s", b);
        if (UiButtonEx(1380, 940, 500, 60, "Herstellen", RecipeAffordable(&r) && InvFreeSlots() > 0, 0xBFE3A8)) {
            if (CraftItem(&it)) Toast("%s hergestellt", nm);
        }
        if (InvFreeSlots() == 0) UiText(1380, 1006, 18, COL_BAD, "Inventar voll");
    } else UiText(1380, 160, 22, COL_MUTED, "Gegenstand anklicken");
}

/* ================================================================== */
/* Shop-Tab                                                           */
/* ================================================================== */


static void ShopTab(void) {
    UiPanel(20, 96, 1000, 960);
    UiPanelTitle(20, 96, 1000, "Materialhändler (nur mit Gold)");
    UiText(40, 148, 20, COL_MUTED, "Linksklick: 1 kaufen   Rechtsklick: 10 kaufen");
    for (int t = 0; t < TIER_COUNT; t++) {
        double y = 190 + t * 140;
        int open = ShopTierUnlocked(t);
        UiFill(36, y, 968, 128, open ? Mix(TierColor(t), 0xFFFFFF, 0.55) : 0xBDB3A3);
        UiFill(36, y, 10, 128, TierColor(t));
        UiText(56, y + 4, 24, COL_INK, TierName(t));
        if (!open) { UiText(56, y + 50, 22, COL_MUTED, "Noch nicht verfügbar (höheres Spieler-Level)"); continue; }
        for (int k = 0; k < MAT_COUNT; k++) {
            Item m = ItemMakeMaterial(t, k, 1);
            double x = 70 + k * 235;
            int res = UiItemSlot(x, y + 36, 70, &m, 0);
            char b[32]; FormatNumber(MaterialBuyPrice(t, k), b, sizeof b);
            UiTextf(x + 78, y + 38, 20, COL_INK, "%s", MaterialName(k));
            UiTextf(x + 78, y + 62, 20, player.gold >= MaterialBuyPrice(t, k) ? COL_INK : COL_BAD, "%s G", b);
            UiTextf(x + 78, y + 86, 18, COL_MUTED, "Vorrat %d", player.shopStock[t][k]);
            int times = (res == 1) ? 1 : (res == 2 ? 10 : 0);
            int bought = 0;
            for (int q = 0; q < times; q++) if (ShopBuyMaterial(t, k)) bought++;
            if (times && bought == 0) Toast("Nicht möglich (Gold, Vorrat oder Inventar)");
        }
    }
    UiPanel(1040, 96, 860, 560);
    UiPanelTitle(1040, 96, 860, "Verkaufen");
    UiText(1060, 150, 22, COL_INK, "Alle Ausrüstungsteile bis Tier verkaufen:");
    for (int t = 0; t < TIER_COUNT - 2; t++)
        if (UiButtonEx(1060 + t * 142, 190, 134, 44, TierName(t), 1, t == shopSellTier ? 0xFFE9CC : Mix(TierColor(t), 0xFFFFFF, 0.5))) shopSellTier = t;
    double total = 0; int count = 0;
    for (int i = 0; i < MAX_INV; i++) {
        Item *it = &player.inv[i];
        if (ItemIsEmpty(it) || it->cat == CAT_MATERIAL || it->cat == CAT_WINGS || it->tier > shopSellTier) continue;
        total += ItemSellValue(it); count++;
    }
    char b[32]; FormatNumber(total, b, sizeof b);
    UiTextf(1060, 260, 24, COL_INK, "%d Gegenstände  =>  %s Gold", count, b);
    UiText(1060, 292, 18, COL_MUTED, "Material und Flügel werden nie mitverkauft.");
    if (UiButtonEx(1060, 330, 400, 56, "Jetzt verkaufen", count > 0, 0xFFD34D)) {
        for (int i = 0; i < MAX_INV; i++) {
            Item *it = &player.inv[i];
            if (ItemIsEmpty(it) || it->cat == CAT_MATERIAL || it->cat == CAT_WINGS || it->tier > shopSellTier) continue;
            SellItem(i);
        }
        Toast("%d Gegenstände verkauft", count);
    }
    UiText(1060, 420, 22, COL_INK, "Verkauf einzelner Dinge: Inventar => Verkaufen");
    UiTextf(1060, 480, 24, COL_INK, "Nächster Nachschub in %d:%02d", (RestockFrames() - player.shopTimer) / 3600, ((RestockFrames() - player.shopTimer) / 60) % 60);
    UiText(1060, 516, 18, COL_MUTED, "Der Vorrat füllt sich alle 10 Minuten von selbst auf.");
}

/* ================================================================== */
/* Karten-Tab                                                         */
/* ================================================================== */

#define MAP_GROUPS (StoryChapters + 2)

static void GroupStages(int g, int *first, int *count) {
    if (g < StoryChapters) { *first = g * StoryMapsPerChapter; *count = StoryMapsPerChapter; }
    else if (g == StoryChapters) { *first = StoryChapters * StoryMapsPerChapter; *count = TempleCount; }
    else { *first = StoryChapters * StoryMapsPerChapter + TempleCount; *count = RuinsCount; }
}

static void MapsTab(void) {
    if (mapHeroA < 0 || !heroes[mapHeroA].isHero) mapHeroA = heroes[selHero].isHero ? selHero : FirstHero();
    UiPanel(20, 96, 360, 960);
    UiPanelTitle(20, 96, 360, "Kapitel");
    double off = UiScrollBegin(&scChapters, 24, 142, 352, 908, MAP_GROUPS * 66 + 8);
    for (int g = 0; g < MAP_GROUPS; g++) {
        char b[64];
        if (g < StoryChapters) snprintf(b, sizeof b, "%d  %s", g + 1, ChapterNames[g]);
        else snprintf(b, sizeof b, g == StoryChapters ? "Tempel (zähe Gegner)" : "Ruinen (harte Gegner)");
        int first, cnt; GroupStages(g, &first, &cnt);
        int open = cnt > 0 && StageUnlocked(first);
        if (UiButtonEx(34, 150 + g * 66 - off, 332 - 18, 56, b, open, g == mapGroup ? 0xFFE9CC : (g >= StoryChapters ? 0xE8C8F0 : COL_PARCH_D))) { mapGroup = g; mapSel = -1; scStages.off = 0; }
    }
    UiScrollEnd();
    if (mapGroup >= MAP_GROUPS) mapGroup = 0;
    int first, cnt; GroupStages(mapGroup, &first, &cnt);
    UiPanel(400, 96, 900, 960);
    UiPanelTitle(400, 96, 900, mapGroup < StoryChapters ? ChapterNames[mapGroup] : (mapGroup == StoryChapters ? "Tempel" : "Ruinen"));
    off = UiScrollBegin(&scStages, 404, 142, 892, 908, cnt * 176 + 8);
    for (int i = 0; i < cnt; i++) {
        int s = first + i;
        const StageDef *S = &Stages[s];
        double y = 156 + i * 176 - off;
        int open = StageUnlocked(s), cleared = player.cleared[s], sel = (mapSel == s);
        UiFill(414, y - 3, 854, 166, COL_INK);
        UiFill(417, y, 848, 160, sel ? 0xFFE9CC : (open ? COL_PARCH_D : 0xBDB3A3));
        UiText(436, y + 6, 32, COL_INK, S->name);
        UiTextf(436, y + 48, 22, COL_INK, "Monster-Level %d   %s", S->level, StageKindName(S->kind));
        UiTextf(436, y + 78, 20, COL_MUTED, S->partyMax > 1 ? "Gruppen: bis zu 2 Helden" : "Nur 1 Held");
        UiText(436, y + 106, 20, cleared ? COL_GOOD : (open ? COL_INK : COL_BAD), cleared ? "Geschafft OK" : (open ? "Freigeschaltet" : "Gesperrt - Voraussetzung schaffen"));
        /* Helden, die gerade hier sind */
        double hy = y + 10;
        for (int h = 0; h < MAX_HEROES; h++) if (heroes[h].isHero && heroes[h].stage == s) {
            UiFill(1000, hy, 24, 24, CLASS_COLORS[heroes[h].class]); UiText(1030, hy, 18, COL_INK, heroes[h].name); hy += 28;
        }
        if (open && UiHover(417, y, 848, 160) && uiClick && !uiClickUsed) { mapSel = s; uiClickUsed = 1; }
    }
    UiScrollEnd();
    /* Heldenwahl + Start */
    UiPanel(1320, 96, 580, 960);
    UiPanelTitle(1320, 96, 580, "Helden schicken");
    int maxHeroes = mapSel >= 0 ? Stages[mapSel].partyMax : 1;
    UiTextf(1340, 150, 20, COL_MUTED, mapSel >= 0 ? "Wähle %s" : "Zuerst Karte wählen", maxHeroes > 1 ? "bis zu 2 Helden" : "1 Helden");
    int row = 0;
    off = UiScrollBegin(&scMapHeroes, 1324, 182, 572, 780, HeroCount() * 56 + 8);
    for (int h = 0; h < MAX_HEROES; h++) {
        if (!heroes[h].isHero) continue;
        double y = 184 + row * 56 - off;
        int chosen = (h == mapHeroA || h == mapHeroB);
        UiFill(1334, y, 534, 50, chosen ? 0xBFE3A8 : COL_PARCH_D);
        UiFill(1334, y, 8, 50, CLASS_COLORS[heroes[h].class]);
        UiTextf(1354, y + 4, 24, COL_INK, "%s  Lv %d", heroes[h].name, heroes[h].level);
        UiTextf(1354, y + 28, 16, COL_MUTED, "%s", heroes[h].stage >= 0 ? Stages[heroes[h].stage].name : "im Lager");
        if (UiHover(1334, y, 534, 50) && uiClick && !uiClickUsed) {
            uiClickUsed = 1;
            if (h == mapHeroA) { mapHeroA = mapHeroB; mapHeroB = -1; }
            else if (h == mapHeroB) mapHeroB = -1;
            else if (mapHeroA < 0) mapHeroA = h;
            else if (maxHeroes > 1 && mapHeroB < 0) mapHeroB = h;
            else mapHeroA = h;
        }
        row++;
    }
    UiScrollEnd();
    int canStart = mapSel >= 0 && mapHeroA >= 0 && StageUnlocked(mapSel) && (mapHeroB < 0 || maxHeroes > 1);
    if (UiButtonEx(1340, 980, 540, 56, "Auf die Karte schicken", canStart, 0xBFE3A8)) {
        int b = (maxHeroes > 1) ? mapHeroB : -1;
        if (StartStage(mapHeroA, mapSel, b)) { Toast("Los geht's: %s", Stages[mapSel].name); activeHero = mapHeroA; }
    }
}

/* ================================================================== */
/* Einstellungen-Tab                                                  */
/* ================================================================== */

/* Schrittweite im Spiel für einzelne Einstellungen (0 = automatisch) */
static double SettingStep(const char *name) {
    if (!strcmp(name, "gamespeed")) return 0.1;
    if (!strcmp(name, "max_fps")) return 10;
    if (!strcmp(name, "render_height")) return 120;
    if (!strcmp(name, "render_distance")) return 128;
    if (!strcmp(name, "autosave_seconds")) return 5;
    return 1;
}

static void SettingsTab(void) {
    static double snapshot[256]; static int haveSnap = 0;
    UiPanel(20, 96, 1880, 960);
    UiPanelTitle(20, 96, 1880, "Einstellungen");
    UiText(44, 146, 20, COL_MUTED, "Änderungen gelten sofort und werden in game.conf gespeichert. Weitere Werte (Balancing, Karten ...) stehen in den .conf-Dateien.");
    int n = 0;
    for (int i = 0; i < cfgCount; i++) if (cfgDefs[i].ranged) n++;
    double off = UiScrollBegin(&scSettings, 24, 180, 1872, 860, n * 56 + 80);
    double y = 190 - off;
    for (int i = 0; i < cfgCount; i++) {
        CfgDef *d = &cfgDefs[i];
        if (!d->ranged) continue;
        char label[96]; snprintf(label, sizeof label, "%s", d->desc);
        char *p = strstr(label, " ("); if (p) *p = 0;
        double v = *d->val;
        if (d->userMin == 0 && d->userMax == 1) {
            int on = v != 0;
            if (UiCheck(60, y + 2, label, &on)) *d->val = on;
        } else {
            UiNumField(6000 + i, 60, y, 520, 220, label, T_DBL, d->val, d->userMin, d->userMax, SettingStep(d->name));
            UiTextf(1000, y + 8, 18, COL_MUTED, "erlaubt: %.10g bis %.10g", d->userMin, d->userMax);
            if (v < d->userMin || v > d->userMax) UiText(1380, y + 8, 18, COL_BAD, "außerhalb des Bereichs (aus game.conf übernommen)");
            if (!strcmp(d->name, "gamespeed") && v < 1) UiText(1380, y + 8, 18, COL_MUTED, "Zeitlupe");
            if (!strcmp(d->name, "max_fps") && v <= 0) UiText(1380, y + 8, 18, COL_MUTED, "unbegrenzt");
        }
        if (!strcmp(d->name, "render_height")) UiText(1380, y + 8, 18, COL_MUTED, "wirkt nach Neustart");
        y += 56;
    }
    UiScrollEnd();
    /* Änderungen erkennen und game.conf schreiben */
    int changed = !haveSnap;
    for (int i = 0; i < cfgCount && i < 256; i++) { if (snapshot[i] != *cfgDefs[i].val) changed = 1; }
    if (changed) {
        if (haveSnap && !uiFocus) CfgSave(CONFIG_FILE);
        if (!uiFocus) { for (int i = 0; i < cfgCount && i < 256; i++) snapshot[i] = *cfgDefs[i].val; haveSnap = 1; }
    }
}

/* ================================================================== */
/* Menü                                                               */
/* ================================================================== */

static void MenuScreen(void) {
    UiFill(0, 78, 1920, 1002, COL_BG);
    switch (menuTab) {
    case TAB_HEROES: HeroesTab(); break;
    case TAB_INV:    InventoryTab(); break;
    case TAB_CRAFT:  CraftingTab(); break;
    case TAB_SHOP:   ShopTab(); break;
    case TAB_MAPS:   MapsTab(); break;
    case TAB_SETTINGS: SettingsTab(); break;
    }
}
