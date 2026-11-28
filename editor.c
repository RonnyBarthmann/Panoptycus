/* ==================================================================
 * Savegame-Editor  (Start:  ./game --editor)
 *
 * Vier Ansichten:
 *   EV_HEROES       alle Heldenplätze mit Ausrüstung + Editor-Slot
 *   EV_PLAYER       das komplette Spieler-Inventar (168 Slots) + Editor-Slot
 *   EV_HERO_META    alle übrigen Daten eines Helden
 *   EV_PLAYER_META  alle übrigen Daten des Spielers
 *
 * Bedienung der Slots:
 *   Linke Maustaste gedrückt halten und über Slots fahren: der Inhalt des
 *   Editor-Slots wird in jeden Slot übernommen, der ihn akzeptiert.
 *   Rechtsklick auf einen Slot: Inhalt 1:1 in den Editor-Slot kopieren.
 * Zahlenfelder: anklicken, tippen, Enter. Die Knöpfe - / + ändern um 1.
 * ================================================================== */

enum { EV_HEROES = 0, EV_PLAYER, EV_HERO_META, EV_PLAYER_META };

static int  edView = EV_HEROES, edMetaHero = 0;
static Item edItem;
static int  edDrag = 0;
static int  edBackedUp = 0;

/* Zahlen-/Textfelder, Auswahl und Checkboxen sind in ui.c (gemeinsam mit dem Spiel) */
#define EdCommit UiCommit
#define EdFocusField UiFocusField
#define EdNum(id, x, y, lw, w, label, t, p, mn, mx) UiNumField(id, x, y, lw, w, label, t, p, mn, mx, 0)
#define EdText UiTextField
#define EdCheck UiCheck
#define EdSel UiSelect
#define edFocus uiFocus
#define edBuf uiEditBuf

/* ---- Items ---- */

static int EdHasLevel(int cat) {
    return cat == CAT_WEAPON || cat == CAT_OFFHAND || cat == CAT_HELM || cat == CAT_CHEST ||
           cat == CAT_GLOVES || cat == CAT_BOOTS || cat == CAT_RING || cat == CAT_NECKLACE;
}
static int EdHasIdx(int cat) {
    return cat == CAT_WEAPON || cat == CAT_HELM || cat == CAT_CHEST || cat == CAT_GLOVES || cat == CAT_BOOTS;
}
static int EdKindMax(int cat) {
    switch (cat) {
    case CAT_WEAPON: case CAT_OFFHAND: return CLASS_COUNT - 1;
    case CAT_RING: case CAT_NECKLACE:  return JEW_COUNT - 1;
    case CAT_POTION:                   return POT_COUNT - 1;
    case CAT_MATERIAL:                 return MAT_COUNT - 1;
    }
    return 0;
}
/* bringt ein Item in einen gültigen Zustand */
static void EdNormalize(Item *it) {
    if (it->cat == CAT_NONE || it->cat >= CAT_COUNT) { memset(it, 0, sizeof *it); return; }
    if (it->tier >= TIER_COUNT) it->tier = TIER_COUNT - 1;
    if (it->kind > EdKindMax(it->cat)) it->kind = (unsigned char)EdKindMax(it->cat);
    if (EdHasIdx(it->cat)) { int n = TierSlotCount(it->tier); if (it->idx >= n) it->idx = (unsigned char)(n - 1); }
    else it->idx = 0;
    if (!EdHasLevel(it->cat)) it->level = 0; else if (it->level > ITEM_MAX_LEVEL) it->level = ITEM_MAX_LEVEL;
    if (it->cat == CAT_POTION) { if (it->count > 100) it->count = 100; }
    else if (it->cat == CAT_MATERIAL) { if (it->count < 1) it->count = 1; }
    else it->count = 1;
}

/* ein Slot: zeichnet, nimmt per Drag (Maustaste halten) den Editor-Inhalt an,
 * Rechtsklick kopiert den Inhalt in den Editor-Slot. label = Text für leere Slots */
static void EdSlot(double x, double y, double size, Item *dst, int acceptsEdit, const char *label) {
    int hov = UiHover(x, y, size, size);
    UiItemSlot(x, y, size, dst, 0);
    if (label && ItemIsEmpty(dst)) UiTextC(x + size / 2, y + size / 2 - 6, 11, COL_MUTED, label);
    if (!hov) return;
    if (uiClick) edDrag = 1;
    if (edDrag && uiDown && acceptsEdit && memcmp(dst, &edItem, sizeof(Item)) != 0) *dst = edItem;
    if (uiRClick) edItem = *dst;
}

/* der Editor-Slot samt Eigenschaften (rechte Seite) */
static void EdItemEditor(double x0, double y0, int allowMaterial) {
    (void)allowMaterial;
    UiPanel(x0, y0, 720, 960);
    UiPanelTitle(x0, y0, 720, "Editor-Slot");
    UiItemSlot(x0 + 24, y0 + 56, 128, &edItem, 0);
    char nm[64], lines[14][72];
    if (ItemIsEmpty(&edItem)) UiText(x0 + 170, y0 + 70, 24, COL_MUTED, "(leer)");
    else {
        ItemName(&edItem, nm, sizeof nm);
        UiText(x0 + 170, y0 + 60, 26, COL_INK, nm);
        int n = ItemDescribe(&edItem, lines, 14);
        for (int i = 0; i < n && i < 5; i++) UiText(x0 + 170, y0 + 96 + i * 22, 18, i == 0 ? COL_MUTED : COL_INK, lines[i]);
    }
    UiText(x0 + 24, y0 + 214, 18, COL_MUTED, "Linke Maustaste halten und über Slots fahren = einsetzen. Rechtsklick auf Slot = kopieren.");
    double y = y0 + 256, lw = 150, w = 300;
    if (UiButtonEx(x0 + 24, y, 200, 40, "Leeren", 1, 0xE8B0A8)) memset(&edItem, 0, sizeof edItem);
    y += 56;
    int cat = edItem.cat;
    if (EdSel(x0 + 24, y, lw, w, "Kategorie", &cat, 0, CAT_COUNT - 1, cat == CAT_NONE ? "(leer)" : CategoryName(cat))) {
        edItem = cat == CAT_NONE ? (Item){0} : ItemMake(cat, edItem.tier, 0, 0);
        if (cat == CAT_MATERIAL) edItem.count = 1;
    }
    y += 52;
    if (edItem.cat != CAT_NONE) {
        int t = edItem.tier;
        if (EdSel(x0 + 24, y, lw, w, "Stufe", &t, 0, TIER_COUNT - 1, TierName(t))) edItem.tier = (unsigned char)t;
        y += 52;
        int cmax = EdKindMax(edItem.cat);
        if (cmax > 0) {
            int k = edItem.kind; const char *kn = "";
            const char *klabel = "Art";
            switch (edItem.cat) {
            case CAT_WEAPON: case CAT_OFFHAND: kn = ClassName(k); klabel = "Klasse"; break;
            case CAT_RING: kn = JewelryName(k, 0); break;
            case CAT_NECKLACE: kn = JewelryName(k, 1); break;
            case CAT_POTION: kn = PotionName(k); break;
            case CAT_MATERIAL: kn = MaterialName(k); break;
            }
            if (EdSel(x0 + 24, y, lw, w, klabel, &k, 0, cmax, kn)) edItem.kind = (unsigned char)k;
            y += 52;
        }
        if (EdHasIdx(edItem.cat)) {
            int n = TierSlotCount(edItem.tier), ix = edItem.idx; char b[32];
            snprintf(b, sizeof b, "%d von %d", ix + 1, n);
            if (EdSel(x0 + 24, y, lw, w, "Nummer", &ix, 0, n - 1, b)) edItem.idx = (unsigned char)ix;
            y += 52;
        }
        if (EdHasLevel(edItem.cat)) {
            EdNum(9001, x0 + 24, y, lw, 150, "Item-Level", T_UC, &edItem.level, 0, ITEM_MAX_LEVEL);
            y += 52;
        }
        if (edItem.cat == CAT_POTION) { EdNum(9002, x0 + 24, y, lw, 150, "Ladung (0-100)", T_US, &edItem.count, 0, 100); y += 52; }
        if (edItem.cat == CAT_MATERIAL) { EdNum(9003, x0 + 24, y, lw, 150, "Anzahl", T_US, &edItem.count, 1, 65535); y += 52; }
    }
    EdNormalize(&edItem);
    /* Hinweis zu den Mindest-Leveln */
    if (!ItemIsEmpty(&edItem)) UiTextf(x0 + 24, y + 10, 20, COL_MUTED, "Benötigtes Helden-Level: %d", ItemRequiredLevel(&edItem));
}

/* ---- Speichern ---- */

static void EdSanitize(void) {
    for (int i = 0; i < MAX_HEROES; i++) {
        Hero *H = &heroes[i];
        if (!H->isHero) continue;
        if (H->class < 0 || H->class >= CLASS_COUNT) H->class = 0;
        if (H->level < 1) H->level = 1;
        if (H->level > HERO_MAX_LEVEL) H->level = HERO_MAX_LEVEL;
        if (H->xp < 0) H->xp = 0;
        if (H->stage < -1 || H->stage >= StageCount) H->stage = -1;
        if (H->partner < -1 || H->partner >= MAX_HEROES || H->partner == i || !heroes[H->partner < 0 ? 0 : H->partner].isHero ||
            (H->partner >= 0 && heroes[H->partner].stage != H->stage)) H->partner = -1;
        for (int s = 0; s < 4; s++) if (H->skillSlots[s] < -1 || H->skillSlots[s] >= CLASS_COUNT * SKILLS_PER_CLASS) H->skillSlots[s] = -1;
        for (int s = 0; s < EQ_COUNT; s++) EdNormalize(&H->equip[s]);
        for (int t = 0; t < TIER_COUNT; t++) { if (H->chests[t] < 0) H->chests[t] = 0; if (H->chestLevel[t] < 0) H->chestLevel[t] = 0; }
        H->inst = -1; H->focus = -1;
        RecalcHero(i);
        H->hp = H->hpMax; H->mana = H->manaMax;
    }
    for (int i = 0; i < MAX_INV; i++) EdNormalize(&player.inv[i]);
    if (player.level < 1) player.level = 1;
    if (player.level > PLAYER_MAX_LEVEL) player.level = PLAYER_MAX_LEVEL;
    if (player.gold < 0) player.gold = 0;
    if (player.activeLayout < 0 || player.activeLayout >= LAYOUT_COUNT) player.activeLayout = 0;
    for (int l = 0; l < LAYOUT_COUNT; l++) for (int c = 0; c < 4; c++) {
        int h = player.layouts[l][c];
        if (h < 0 || h >= MAX_HEROES || !heroes[h].isHero) player.layouts[l][c] = -1;
    }
    if (player.shopTimer < 0) player.shopTimer = 0;
    for (int s = 0; s < MAX_STAGES; s++) player.cleared[s] = player.cleared[s] ? 1 : 0;
}

static void EdBackup(void) {
    if (edBackedUp) return;
    FILE *in = fopen(SAVE_FILE, "rb");
    if (!in) return;
    FILE *out = fopen(SAVE_FILE ".bak", "wb");
    if (out) {
        char buf[4096]; size_t n;
        while ((n = fread(buf, 1, sizeof buf, in)) > 0) fwrite(buf, 1, n, out);
        fclose(out); edBackedUp = 1;
    }
    fclose(in);
}

static void EdSave(void) {
    EdCommit();
    EdBackup();
    EdSanitize();
    SaveGame();
    Toast("Gespeichert (Sicherung: " SAVE_FILE ".bak)");
}

/* ---- gemeinsame Kopfleiste ---- */

static int edQuit = 0;

static void EdTopBar(void) {
    UiFill(0, 0, 1920, 72, 0x2B2118);
    if (UiButtonEx(20, 12, 170, 48, "Spieler", edView != EV_PLAYER, COL_PARCH_D)) { EdCommit(); edView = EV_PLAYER; }
    if (UiButtonEx(206, 12, 170, 48, "Helden", edView != EV_HEROES, COL_PARCH_D)) { EdCommit(); edView = EV_HEROES; }
    UiText(420, 20, 28, COL_PARCH_L, "Savegame-Editor");
    if (UiButtonEx(1160, 12, 220, 48, "Speichern", 1, 0xBFE3A8)) EdSave();
    if (UiButtonEx(1396, 12, 270, 48, "Speichern & Beenden", 1, 0xBFE3A8)) { EdSave(); edQuit = 1; }
    if (UiButtonEx(1682, 12, 220, 48, "Beenden", 1, 0xE8B0A8)) edQuit = 1;
}

/* ---- Ansicht: Helden ---- */

static void EdHeroInit(int h) {
    ClearHero(h);
    Hero *H = &heroes[h];
    H->isHero = 1; H->class = CLASS_Knight; H->level = 1;
    H->chestMask = (1u << TIER_COUNT) - 1;
    H->autopilotEnabled = 1;
    snprintf(H->name, sizeof H->name, "Held %d", h + 1);
    HeroAutoAssignSkills(h);
    RecalcHero(h);
    H->hp = H->hpMax; H->mana = H->manaMax;
}

static void EdHeroesView(void) {
    EdTopBar();
    for (int h = 0; h < MAX_HEROES; h++) {
        Hero *H = &heroes[h];
        double y = 80 + h * 60;
        UiFill(10, y, 1150, 56, h & 1 ? 0xF0D2A8 : COL_PARCH);
        if (H->isHero) {
            Item ic = ItemMake(CAT_WEAPON, TIER_NORMAL, H->class, 0);
            UiFill(14, y + 2, 56, 52, CLASS_COLORS[H->class]);
            UiItemIcon(16, y + 2, 52, &ic);
            if (UiHover(14, y + 2, 56, 52)) {
                UiFrame(12, y, 60, 56, 2, 0xFFFFFF);
                if (uiClick && !uiClickUsed) { uiClickUsed = 1; EdCommit(); edMetaHero = h; edView = EV_HERO_META; }
            }
            UiTextf(80, y + 4, 20, COL_INK, "%s", H->name);
            UiTextf(80, y + 30, 16, COL_MUTED, "%s - Lv %d", ClassName(H->class), H->level);
            for (int s = 0; s < EQ_COUNT; s++)
                EdSlot(300 + s * 58, y + 2, 52, &H->equip[s], ItemIsEmpty(&edItem) || SlotCategoryOK(s, &edItem, H->class), EQ_NAMES[s]);
        } else {
            UiFill(14, y + 2, 56, 52, 0xBDB3A3);
            UiTextC(42, y + 10, 30, COL_MUTED, "+");
            if (UiHover(14, y + 2, 56, 52)) {
                UiFrame(12, y, 60, 56, 2, 0xFFFFFF);
                if (uiClick && !uiClickUsed) { uiClickUsed = 1; EdCommit(); edMetaHero = h; edView = EV_HERO_META; }
            }
            UiTextf(80, y + 16, 20, COL_MUTED, "Platz %d (leer) - Klick auf das Symbol", h + 1);
        }
    }
    EdItemEditor(1180, 80, 0);
}

/* ---- Ansicht: Spieler-Inventar ---- */

static void EdPlayerView(void) {
    EdTopBar();
    UiPanel(10, 80, 1160, 960);
    UiPanelTitle(10, 80, 1160, "Spieler-Inventar (168 Slots, jeder Slot nimmt alles an)");
    for (int i = 0; i < MAX_INV; i++) {
        double x = 30 + (i % 14) * 80, y = 136 + (i / 14) * 76;
        EdSlot(x, y, 68, &player.inv[i], 1, NULL);
    }
    /* Symbol -> Spieler-Daten */
    double ix = 760, iy = 8;
    int hov = UiHover(ix, iy, 56, 56);
    UiFill(ix - 2, iy - 2, 60, 60, COL_INK);
    UiFill(ix, iy, 56, 56, hov ? COL_PARCH_L : 0xE8C060);
    DrawImageScaled(UiIcon[IC_CHESTBOX], ix + 6, iy + 6, 44, 44);
    UiText(ix + 66, iy + 14, 24, COL_PARCH_L, "Spieler-Daten");
    if (hov && uiClick && !uiClickUsed) { uiClickUsed = 1; EdCommit(); edView = EV_PLAYER_META; }
    EdItemEditor(1180, 80, 1);
}

/* ---- Ansicht: Helden-Daten ---- */

static void EdHeroMetaView(void) {
    Hero *H = &heroes[edMetaHero];
    UiFill(0, 0, 1920, 72, 0x2B2118);
    if (UiButtonEx(20, 12, 200, 48, "Zurück", 1, COL_PARCH_D)) { EdCommit(); edView = EV_HEROES; }
    UiTextf(250, 20, 28, COL_PARCH_L, "Held-Daten - Platz %d", edMetaHero + 1);
    UiPanel(10, 90, 640, 950); UiPanel(660, 90, 620, 950); UiPanel(1290, 90, 620, 950);
    UiPanelTitle(10, 90, 640, "Allgemein"); UiPanelTitle(660, 90, 620, "Kisten"); UiPanelTitle(1290, 90, 620, "Skills");
    double y = 148, x = 30;
    int active = H->isHero;
    if (EdCheck(x, y, "Held aktiv", &active)) {
        if (active) EdHeroInit(edMetaHero); else ClearHero(edMetaHero);
    }
    UiText(x + 160, y + 5, 16, COL_MUTED, "Abwählen löscht den Helden samt Ausrüstung.");
    y += 56;
    if (!H->isHero) { UiText(x, y, 22, COL_MUTED, "Dieser Heldenplatz ist leer. Haken setzen, um einen Helden anzulegen."); return; }

    int cls = H->class;
    if (EdSel(x, y, 215, 200, "Klasse", &cls, 0, CLASS_COUNT - 1, ClassName(cls))) {
        H->class = cls;
        for (int s = 0; s < 4; s++) H->skillSlots[s] = -1;
        HeroAutoAssignSkills(edMetaHero);
    }
    y += 52;
    EdText(1001, x, y, 215, 300, "Name", H->name, (int)sizeof H->name); y += 52;
    EdNum(1002, x, y, 215, 200, "Level", T_INT, &H->level, 1, HERO_MAX_LEVEL); y += 52;
    EdNum(1003, x, y, 215, 200, "Erfahrung (EP)", T_DBL, &H->xp, 0, 1e18); y += 52;
    UiTextf(x + 215 + 38, y - 17, 13, COL_MUTED, "Level-Up bei %.0f", XpNeeded(H->level));
    EdNum(1004, x, y, 215, 200, "Besiegte Gegner", T_LONG, &H->kills, 0, 9e15); y += 52;
    EdNum(1005, x, y, 215, 100, "Karte (-1=Lager)", T_INT, &H->stage, -1, StageCount - 1);
    UiTextf(x + 215 + 38 + 100 + 50, y + 6, 16, COL_MUTED, "%s", H->stage >= 0 ? Stages[H->stage].name : "kein Einsatz");
    y += 52;
    EdNum(1006, x, y, 215, 100, "Partner (-1=keiner)", T_INT, &H->partner, -1, MAX_HEROES - 1); y += 52;
    EdCheck(x, y, "Autopilot", &H->autopilotEnabled); y += 44;
    EdCheck(x, y, "Auto-Weiter (nächste Karte)", &H->autoNext); y += 52;
    /* Warnungen */
    for (int s = EQ_WEAPON; s <= EQ_OFFHAND; s++)
        if (!ItemIsEmpty(&H->equip[s]) && H->equip[s].kind != H->class) { UiText(x, y, 18, COL_BAD, s == EQ_WEAPON ? "Achtung: Waffe passt nicht zur Klasse." : "Achtung: Nebenhand passt nicht zur Klasse."); y += 26; }
    y += 10;
    UiText(x, y, 16, COL_MUTED, "Nicht editierbar (werden beim Laden neu berechnet):");
    UiText(x, y + 22, 16, COL_MUTED, "Position, Leben/Mana, Abklingzeiten, Buffs.");

    /* Kisten */
    double cx = 680; y = 148;
    UiText(cx, y, 20, COL_INK, "Welche Stufen sammelt der Held?"); y += 36;
    for (int t = 0; t < TIER_COUNT; t++) {
        int on = (H->chestMask >> t) & 1;
        UiFill(cx + t * 96 - 2, y - 2, 88, 44, COL_INK);
        UiFill(cx + t * 96, y, 84, 40, on ? TierColor(t) : 0x8A8278);
        UiTextC(cx + t * 96 + 42, y + 11, 16, 0x000000, on ? TierName(t) : "aus");
        if (UiHover(cx + t * 96, y, 84, 40) && uiClick && !uiClickUsed) { uiClickUsed = 1; H->chestMask ^= 1u << t; }
    }
    y += 70;
    UiText(cx, y, 20, COL_INK, "Gesammelte Kisten"); y += 36;
    for (int t = 0; t < TIER_COUNT; t++) { EdNum(1100 + t, cx, y, 150, 150, TierName(t), T_INT, &H->chests[t], 0, 100000); y += 46; }
    y += 14;
    UiText(cx, y, 20, COL_INK, "Karten-Level der Kisten (bestimmt den Inhalt)"); y += 36;
    for (int t = 0; t < TIER_COUNT; t++) { EdNum(1200 + t, cx, y, 150, 150, TierName(t), T_INT, &H->chestLevel[t], 0, 100); y += 46; }

    /* Skills */
    double sx = 1310; y = 148;
    UiText(sx, y, 20, COL_INK, "Belegte Skill-Plätze (0 = leer)"); y += 40;
    static const char *keys[4] = { "Taste Q", "Taste W", "Taste E", "Taste R" };
    for (int s = 0; s < 4; s++) {
        int v = H->skillSlots[s] >= 0 ? H->skillSlots[s] - SKILL_ID(H->class, 0) + 1 : 0;
        if (v < 0 || v > SKILLS_PER_CLASS) v = 0;
        if (EdNum(1300 + s, sx, y, 110, 70, keys[s], T_INT, &v, 0, SKILLS_PER_CLASS)) H->skillSlots[s] = v > 0 ? SKILL_ID(H->class, v - 1) : -1;
        UiText(sx + 110 + 38 + 70 + 50, y + 6, 20, COL_INK, v > 0 ? SkillDefs[SKILL_ID(H->class, v - 1)].name : "-");
        y += 46;
    }
    y += 20;
    UiText(sx, y, 20, COL_INK, "Skills dieser Klasse"); y += 34;
    for (int i = 0; i < SKILLS_PER_CLASS; i++) {
        const SkillDef *sk = &SkillDefs[SKILL_ID(H->class, i)];
        UiTextf(sx, y, 20, COL_INK, "%d  %s", i + 1, sk->name);
        UiTextf(sx + 330, y + 2, 16, COL_MUTED, "ab Level %d", sk->unlock);
        y += 32;
    }
    RecalcHero(edMetaHero);
}

/* ---- Ansicht: Spieler-Daten ---- */

static void EdPlayerMetaView(void) {
    UiFill(0, 0, 1920, 72, 0x2B2118);
    if (UiButtonEx(20, 12, 200, 48, "Zurück", 1, COL_PARCH_D)) { EdCommit(); edView = EV_PLAYER; }
    UiText(250, 20, 28, COL_PARCH_L, "Spieler-Daten");
    UiPanel(10, 90, 620, 950); UiPanel(640, 90, 1270, 950);
    UiPanelTitle(10, 90, 620, "Allgemein / Shop"); UiPanelTitle(640, 90, 1270, "Anordnungen und Karten");
    double x = 30, y = 148;
    EdNum(2001, x, y, 190, 200, "Gold", T_DBL, &player.gold, 0, 1e18); y += 52;
    EdNum(2002, x, y, 190, 120, "Spieler-Level", T_INT, &player.level, 1, PLAYER_MAX_LEVEL);
    UiTextf(x + 190 + 38 + 120 + 50, y + 6, 16, COL_MUTED, "%d Heldenplätze", HeroSlotsForPlayerLevel(player.level)); y += 52;
    EdNum(2003, x, y, 190, 200, "Erfahrung (EP)", T_DBL, &player.xp, 0, 1e18); y += 52;
    UiTextf(x + 190 + 38, y - 17, 13, COL_MUTED, "Level-Up bei %.0f", PlayerXpNeeded(player.level));
    EdNum(2004, x, y, 190, 200, "Besiegte Gegner", T_LONG, &player.totalKills, 0, 9e15); y += 52;
    EdNum(2005, x, y, 190, 120, "Aktive Anordnung", T_INT, &player.activeLayout, 0, LAYOUT_COUNT - 1);
    UiText(x + 190 + 38 + 120 + 50, y + 6, 16, COL_MUTED, "0-3"); y += 52;
    EdNum(2006, x, y, 190, 120, "Shop-Timer", T_INT, &player.shopTimer, 0, RestockFrames());
    UiText(x + 190 + 38 + 120 + 50, y + 6, 16, COL_MUTED, "60 = 1 s"); y += 62;

    UiText(x, y, 20, COL_INK, "Shop-Vorrat"); y += 34;
    for (int k = 0; k < MAT_COUNT; k++) UiTextC(x + 150 + k * 110 + 50, y, 16, COL_MUTED, MaterialName(k));
    y += 24;
    for (int t = 0; t < TIER_COUNT; t++) {
        UiFill(x, y, 8, 34, TierColor(t));
        UiText(x + 14, y + 6, 18, COL_INK, TierName(t));
        for (int k = 0; k < MAT_COUNT; k++) {
            double fx = x + 150 + k * 110; int id = 2100 + t * 10 + k;
            int focused = edFocus == id, hov = UiHover(fx, y, 100, 34);
            UiFill(fx - 2, y - 2, 104, 38, focused ? 0x2E7D32 : COL_INK);
            UiFill(fx, y, 100, 34, focused ? 0xFFFFFF : (hov ? COL_PARCH_L : COL_PARCH));
            char b[80]; if (focused) snprintf(b, sizeof b, "%s_", edBuf); else snprintf(b, sizeof b, "%d", player.shopStock[t][k]);
            UiText(fx + 8, y + 6, 22, COL_INK, b);
            if (hov && uiClick && !uiClickUsed) { uiClickUsed = 1; EdFocusField(id, T_INT, &player.shopStock[t][k], 0, 1000000, 0, fx, y, 100, 34); }
        }
        y += 44;
    }

    /* Anordnungen */
    double ax = 660; y = 148;
    UiText(ax, y, 20, COL_INK, "Heldenplätze der vier Raster-Felder (Nr. der Zeile in der Helden-Ansicht, 0 = erste; -1 = leer)"); y += 40;
    for (int l = 0; l < LAYOUT_COUNT; l++) {
        UiTextf(ax, y + 6, 20, COL_INK, "Anordnung %d", l + 1);
        for (int c = 0; c < 4; c++) {
            double fx = ax + 190 + c * 120; int id = 2200 + l * 10 + c;
            int focused = edFocus == id, hov = UiHover(fx, y, 100, 34);
            UiFill(fx - 2, y - 2, 104, 38, focused ? 0x2E7D32 : COL_INK);
            UiFill(fx, y, 100, 34, focused ? 0xFFFFFF : (hov ? COL_PARCH_L : COL_PARCH));
            char b[80]; if (focused) snprintf(b, sizeof b, "%s_", edBuf); else snprintf(b, sizeof b, "%d", player.layouts[l][c]);
            UiText(fx + 8, y + 6, 22, COL_INK, b);
            if (hov && uiClick && !uiClickUsed) { uiClickUsed = 1; EdFocusField(id, T_INT, &player.layouts[l][c], -1, MAX_HEROES - 1, 0, fx, y, 100, 34); }
        }
        y += 46;
    }
    y += 24;
    UiText(ax, y, 20, COL_INK, "Karten geschafft (Klick = umschalten)"); y += 40;
    if (UiButton(ax, y, 220, 40, "Alle freischalten")) for (int s = 0; s < StageCount; s++) player.cleared[s] = 1;
    if (UiButton(ax + 236, y, 220, 40, "Alle zurücksetzen")) for (int s = 0; s < StageCount; s++) player.cleared[s] = 0;
    y += 60;
    for (int s = 0; s < StageCount; s++) {
        const StageDef *S = &Stages[s];
        char b[32];
        if (S->kind == STAGE_STORY) snprintf(b, sizeof b, "%d-%d", S->chapter + 1, S->index + 1);
        else if (S->kind == STAGE_TEMPLE) snprintf(b, sizeof b, "Tempel %d", S->chapter + 1);
        else snprintf(b, sizeof b, "Ruinen %d", S->chapter + 1);
        double bx = ax + (s % 10) * 122, by = y + (s / 10) * 50;
        if (UiButtonEx(bx, by, 112, 42, b, 1, player.cleared[s] ? 0xBFE3A8 : 0xE8B0A8)) player.cleared[s] = !player.cleared[s];
        if (UiHover(bx, by, 112, 42)) UiText(ax, y + ((StageCount + 9) / 10) * 50 + 8, 20, COL_MUTED, S->name);
    }
}

/* ---- Hauptschleife ---- */

static void EditorMain(int Canvas) {
    memset(&edItem, 0, sizeof edItem);
    char msg[96] = "";
    if (!LoadSaveRaw()) {
        NewGame();
        snprintf(msg, sizeof msg, "Kein (passender) Spielstand gefunden - neuer Spielstand.");
    }
    for (int i = 0; i < MAX_HEROES; i++) { heroes[i].inst = -1; }
    if (msg[0]) Toast("%s", msg);
    /* Test-Hooks */
    if (getenv("PANOPTYCUS_EDDEMO")) {
        int n = 0;
        for (int t = 0; t < TIER_COUNT; t++) {
            player.inv[n++] = ItemMake(CAT_HELM, t, 0, 0);
            player.inv[n++] = ItemMake(CAT_WEAPON, t, CLASS_Knight, 0);
            player.inv[n++] = ItemMake(CAT_POTION, t, POT_SPEED, 0);
            player.inv[n++] = ItemMakeMaterial(t, MAT_ORE, 123);
        }
        heroes[0].equip[EQ_HELM] = ItemMake(CAT_HELM, TIER_RARE, 0, 1);
        edItem = ItemMake(CAT_HELM, TIER_UNIQUE, 0, 2); edItem.level = 7;
    }
    if (getenv("PANOPTYCUS_EDTEST")) {              /* Roundtrip-Test ohne Fenster-Interaktion */
        if (!strcmp(getenv("PANOPTYCUS_EDTEST"), "write")) {
            player.gold = 123456789.5; player.level = 42; player.inv[167] = ItemMakeMaterial(TIER_LEGENDARY, MAT_LEATHER, 999);
            player.cleared[7] = 1; heroes[0].level = 33; heroes[0].xp = 1e9; snprintf(heroes[0].name, sizeof heroes[0].name, "Testheld");
            heroes[0].equip[EQ_CHEST] = ItemMake(CAT_CHEST, TIER_BOSS, 0, 0);
            EdHeroInit(3); heroes[3].class = CLASS_Mage; heroes[3].equip[EQ_WINGS] = ItemMake(CAT_WINGS, TIER_RARE, 0, 0);
            EdSave();
        } else {
            printf("gold %.1f lvl %d inv167 cat %d tier %d count %d cleared7 %d | hero0 lvl %d xp %.0f name %s chest tier %d | hero3 active %d class %d wings %d\n",
                   player.gold, player.level, player.inv[167].cat, player.inv[167].tier, player.inv[167].count, player.cleared[7],
                   heroes[0].level, heroes[0].xp, heroes[0].name, heroes[0].equip[EQ_CHEST].tier, heroes[3].isHero, heroes[3].class, heroes[3].equip[EQ_WINGS].cat);
        }
        return;
    }
    if (getenv("PANOPTYCUS_EDVIEW")) edView = atoi(getenv("PANOPTYCUS_EDVIEW"));

    while (!ShouldQuit() && !edQuit) {
        int ww = WindowWidth(), wh = WindowHeight();
        double sx = (double)ww / ((double)wh * renderWidth / renderHeight);
        double sy = (double)wh / ((double)ww * renderHeight / renderWidth);
        if (sy < sx) sy = 1; else sx = 1;
        int rw = (int)((double)ww / sx), rh = (int)((double)wh / sy);
        int rx = (ww - rw) / 2, ry = (wh - rh) / 2;
        int mx = GetMouseX(), my = GetMouseY();
        uiMX = mx < 0 ? -1000 : (int)((double)(mx - rx) / rw * 1920);
        uiMY = my < 0 ? -1000 : (int)((double)(my - ry) / rh * 1080);
        uiClick = GetMousePressed(MOUSEBUTTON_Left) != 0;
        uiRClick = GetMousePressed(MOUSEBUTTON_Right) != 0;
        uiDown = GetMouseIsPressed(MOUSEBUTTON_Left) != 0;
        uiWheel = GetMouseWheel();
        uiClickUsed = 0;
        uiTipItem = NULL;
        if (!uiDown) edDrag = 0;

        int key = GetKey();
        UiFrameInput(&key);
        if (key == 27 && edView == EV_HERO_META) edView = EV_HEROES;
        else if (key == 27 && edView == EV_PLAYER_META) edView = EV_PLAYER;

        DrawOutput(Canvas);
        UiFill(0, 0, 1920, 1080, COL_BG);
        switch (edView) {
        case EV_HEROES:      EdHeroesView(); break;
        case EV_PLAYER:      EdPlayerView(); break;
        case EV_HERO_META:   EdHeroMetaView(); break;
        case EV_PLAYER_META: EdPlayerMetaView(); break;
        }
        DrawToast();
        UiDrawTooltip();

        DrawOutput(0);
        DrawImage(VoidBG, 0, 0, ww, wh);
        DrawImage(Canvas, rx, ry, rw, rh);
        if (getenv("PANOPTYCUS_SHOT") && engineStep == atoi(getenv("PANOPTYCUS_SHOT"))) {
            SaveScreenshot(getenv("PANOPTYCUS_SHOT_FILE") ? getenv("PANOPTYCUS_SHOT_FILE") : "shot.bmp");
        }
        FlipBuffers();
        engineStep++;
    }
}
