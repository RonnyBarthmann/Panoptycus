/* Simulation: Helden, Inventar, Karten-Instanzen, Monster-KI, Kampf, Skills.
 * (wird von game.c eingebunden; alle Zustände sind globale Arrays) */

#define M_PIMALDAUMEN 3.14159265358979323846
#define BIGFLOAT 9999999999999999.9

static Hero       heroes[MAX_HEROES];
static Instance   instances[MAX_HEROES];
static Projectile projectiles[MAX_PROJECTILES];
static Player     player;
static FloatText  floatTexts[MAX_FLOATTEXT];
static int        engineStep;

/* Visuelle Effekte (Blitze, Ringe, Kegel) */
enum { FX_BOLT = 0, FX_RING, FX_CONE, FX_FLASH };
typedef struct { int used, inst, kind, life, maxLife; double x, y, x2, y2, r; unsigned int color; } Effect;
#define MAX_FX 256
static Effect fxs[MAX_FX];

#define AGGRO_RANGE      720.0
#define HERO_RADIUS      26
#define ELITE_RESPAWN_OK 1

/* ================================================================== */
/* Mathe                                                              */
/* ================================================================== */

static double Dist(double x1, double y1, double x2, double y2) {
    double dx = x2 - x1, dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}
/* 0 = oben, im Uhrzeigersinn (wie bisher) */
static double LookAt(double x, double y, double tx, double ty) {
    double a = atan2(ty - y, tx - x) * (180.0 / M_PIMALDAUMEN) + 90.0;
    if (a < 0.0) a += 360.0;
    return a;
}
static double AngleDiff(double a, double b) {
    double d = fmod(a - b, 360.0);
    if (d > 180.0) d -= 360.0;
    if (d < -180.0) d += 360.0;
    return d;
}
static void Dir(double angle, double *dx, double *dy) {
    double r = angle / (180.0 / M_PIMALDAUMEN);
    *dx = sin(r); *dy = -cos(r);
}
static int Rnd(int n) { return n > 0 ? rand() % n : 0; }
static double Rnd01(void) { return rand() / (RAND_MAX + 1.0); }

static void rotate_point(double *x, double *y, double a) {
    a = fmod(a, 360.0);
    if (a < 0.0) a += 360.0;
    double rad = a * (M_PIMALDAUMEN / 180.0);
    double nx = (*x) * cos(rad) - (*y) * sin(rad);
    double ny = (*x) * sin(rad) + (*y) * cos(rad);
    *x = nx; *y = ny;
}

/* ================================================================== */
/* Popups / Effekte                                                   */
/* ================================================================== */

static void AddFloat(int inst, double x, double y, double value, unsigned int color, int kind) {
    for (int i = 0; i < MAX_FLOATTEXT; i++) {
        if (floatTexts[i].life <= 0) {
            floatTexts[i].inst = inst; floatTexts[i].x = x; floatTexts[i].y = y;
            floatTexts[i].life = 45; floatTexts[i].value = value;
            floatTexts[i].color = (int)color; floatTexts[i].kind = kind;
            return;
        }
    }
}
static void AddFx(int inst, int kind, double x, double y, double x2, double y2, double r, int life, unsigned int color) {
    for (int i = 0; i < MAX_FX; i++) {
        if (!fxs[i].used) {
            fxs[i].used = 1; fxs[i].inst = inst; fxs[i].kind = kind;
            fxs[i].x = x; fxs[i].y = y; fxs[i].x2 = x2; fxs[i].y2 = y2; fxs[i].r = r;
            fxs[i].life = fxs[i].maxLife = life; fxs[i].color = color;
            return;
        }
    }
}

/* ================================================================== */
/* Inventar                                                           */
/* ================================================================== */

#define MAT_STACK_MAX 9999

static int InvAdd(const Item *it) {
    if (ItemIsEmpty(it)) return 1;
    if (ItemIsStackable(it)) {
        for (int i = 0; i < MAX_INV; i++) {
            Item *s = &player.inv[i];
            if (s->cat == it->cat && s->tier == it->tier && s->kind == it->kind && s->count < MAT_STACK_MAX) {
                int space = MAT_STACK_MAX - s->count, add = it->count < space ? it->count : space;
                s->count += add;
                if (add == it->count) return 1;
                Item rest = *it; rest.count -= add;
                return InvAdd(&rest);
            }
        }
    }
    for (int i = 0; i < MAX_INV; i++) {
        if (ItemIsEmpty(&player.inv[i])) { player.inv[i] = *it; return 1; }
    }
    return 0;
}
static int InvFreeSlots(void) {
    int n = 0;
    for (int i = 0; i < MAX_INV; i++) if (ItemIsEmpty(&player.inv[i])) n++;
    return n;
}
static int InvMaterialCount(int tier, int kind) {
    int n = 0;
    for (int i = 0; i < MAX_INV; i++) {
        Item *s = &player.inv[i];
        if (s->cat == CAT_MATERIAL && s->tier == tier && s->kind == kind) n += s->count;
    }
    return n;
}
static void InvTakeMaterial(int tier, int kind, int n) {
    for (int i = 0; i < MAX_INV && n > 0; i++) {
        Item *s = &player.inv[i];
        if (s->cat == CAT_MATERIAL && s->tier == tier && s->kind == kind) {
            int t = s->count < n ? s->count : n;
            s->count -= t; n -= t;
            if (s->count == 0) memset(s, 0, sizeof *s);
        }
    }
}
static int RecipeAffordable(const Recipe *r) {
    if (player.gold < r->gold) return 0;
    for (int i = 0; i < r->matCount; i++)
        if (InvMaterialCount(r->mats[i].tier, r->mats[i].kind) < r->mats[i].count) return 0;
    return 1;
}
static void RecipePay(const Recipe *r) {
    player.gold -= r->gold;
    for (int i = 0; i < r->matCount; i++) InvTakeMaterial(r->mats[i].tier, r->mats[i].kind, r->mats[i].count);
}
static int InvCompare(const void *a, const void *b) {
    const Item *x = a, *y = b;
    if (ItemIsEmpty(x) != ItemIsEmpty(y)) return ItemIsEmpty(x) - ItemIsEmpty(y);
    if (x->cat != y->cat) return x->cat - y->cat;
    if (x->tier != y->tier) return y->tier - x->tier;
    if (x->kind != y->kind) return x->kind - y->kind;
    if (x->idx != y->idx) return y->idx - x->idx;
    return y->level - x->level;
}
static void InvSort(void) { qsort(player.inv, MAX_INV, sizeof(Item), InvCompare); }

/* ================================================================== */
/* Helden: Werte, Ausrüstung, Level                                   */
/* ================================================================== */

static int  HeroCount(void) { int n = 0; for (int i = 0; i < MAX_HEROES; i++) if (heroes[i].isHero) n++; return n; }
static int  HeroSlots(void) { return HeroSlotsForPlayerLevel(player.level); }

static void RecalcHero(int h) {
    Hero *H = &heroes[h];
    ItemStats s; memset(&s, 0, sizeof s);
    for (int i = 0; i < EQ_COUNT; i++) {
        if (i >= EQ_POT1 && i <= EQ_POT3) continue;
        if (!ItemIsEmpty(&H->equip[i])) ItemAddStats(&s, &H->equip[i]);
    }
    double hpRatio = H->hpMax > 0 ? H->hp / H->hpMax : 1.0;
    double mpRatio = H->manaMax > 0 ? H->mana / H->manaMax : 1.0;
    H->hpMax   = HeroBaseHP(H->class, H->level) * (1.0 + s.maxHpPct / 100.0);
    H->manaMax = HeroBaseMana(H->class, H->level) * (1.0 + s.maxManaPct / 100.0);
    H->hp = H->hpMax * hpRatio; H->mana = H->manaMax * mpRatio;
    H->atk = (HeroBaseAttack(H->class, H->level) + s.attack) * (1.0 + s.attackPct / 100.0);
    H->def = s.defense;
    H->crit = 5.0 + s.critChance; if (H->crit > 80.0) H->crit = 80.0;
    H->critMul = (H->class == CLASS_Assassin) ? 2.5 : 2.0;
    H->atkSpeed = 1.0 + s.attackSpeedPct / 100.0;
    H->moveSpeed = ClassBaseMoveSpeed(H->class) * (1.0 + s.moveSpeedPct / 100.0);
    H->manaRegen = H->manaMax * 0.015 * (1.0 + s.manaRegenPct / 100.0);
    H->hpRegen = H->hpMax * (0.002 + s.hpRegenPct / 100.0);
    H->healPower = s.healPowerPct;
    H->block = s.blockPct > 60.0 ? 60.0 : s.blockPct;
    H->capacity = CHEST_BASE_CAPACITY + (int)(s.capacity + 0.5);
}

static int SkillUnlocked(int h, int skill) {
    return skill >= 0 && skill < SKILL_COUNT && SkillDefs[skill].cls == heroes[h].class && heroes[h].level >= SkillDefs[skill].unlock;
}

static void HeroAutoAssignSkills(int h) {
    Hero *H = &heroes[h];
    for (int s = 0; s < 4; s++)
        if (H->skillSlots[s] >= 0 && !SkillUnlocked(h, H->skillSlots[s])) H->skillSlots[s] = -1;
    for (int s = 0; s < 4; s++) {
        if (H->skillSlots[s] >= 0) continue;
        for (int i = 0; i < SKILLS_PER_CLASS; i++) {
            int id = SKILL_ID(H->class, i), used = 0;
            for (int k = 0; k < 4; k++) if (H->skillSlots[k] == id) used = 1;
            if (!used && SkillUnlocked(h, id)) { H->skillSlots[s] = id; break; }
        }
    }
}

static int EquipSlotFor(const Item *it, int preferred) {
    switch (it->cat) {
    case CAT_WEAPON:   return EQ_WEAPON;
    case CAT_OFFHAND:  return EQ_OFFHAND;
    case CAT_HELM:     return EQ_HELM;
    case CAT_CHEST:    return EQ_CHEST;
    case CAT_GLOVES:   return EQ_GLOVES;
    case CAT_BOOTS:    return EQ_BOOTS;
    case CAT_BAG:      return EQ_BAG;
    case CAT_NECKLACE: return EQ_NECK;
    case CAT_WINGS:    return EQ_WINGS;
    case CAT_RING:
        if (preferred == EQ_RING1 || preferred == EQ_RING2) return preferred;
        return EQ_RING1;      /* wird in HeroEquip auf freien Ring-Slot umgelenkt */
    case CAT_POTION:
        if (preferred >= EQ_POT1 && preferred <= EQ_POT3) return preferred;
        return EQ_POT1;
    }
    return -1;
}

/* 0 = ok, sonst Grund (für die UI) */
static const char *HeroEquipProblem(int h, const Item *it) {
    if (ItemIsEmpty(it) || EquipSlotFor(it, -1) < 0) return "Nicht ausrüstbar";
    if ((it->cat == CAT_WEAPON || it->cat == CAT_OFFHAND) && it->kind != heroes[h].class) return "Falsche Klasse";
    if (ItemRequiredLevel(it) > heroes[h].level) return "Held-Level zu niedrig";
    return NULL;
}

/* Legt inv[invIdx] an. Das bisherige Item landet im Inventar. */
static int HeroEquip(int h, int invIdx, int preferredSlot) {
    Item it = player.inv[invIdx];
    if (HeroEquipProblem(h, &it)) return 0;
    int slot = EquipSlotFor(&it, preferredSlot);
    if (it.cat == CAT_RING && preferredSlot != EQ_RING1 && preferredSlot != EQ_RING2)
        slot = ItemIsEmpty(&heroes[h].equip[EQ_RING1]) ? EQ_RING1 : (ItemIsEmpty(&heroes[h].equip[EQ_RING2]) ? EQ_RING2 : EQ_RING1);
    if (it.cat == CAT_POTION && (preferredSlot < EQ_POT1 || preferredSlot > EQ_POT3)) {
        slot = EQ_POT1;
        for (int s = EQ_POT1; s <= EQ_POT3; s++) if (ItemIsEmpty(&heroes[h].equip[s])) { slot = s; break; }
    }
    Item old = heroes[h].equip[slot];
    heroes[h].equip[slot] = it;
    memset(&player.inv[invIdx], 0, sizeof(Item));
    if (!ItemIsEmpty(&old)) InvAdd(&old);
    RecalcHero(h);
    return 1;
}
static int HeroUnequip(int h, int slot) {
    if (ItemIsEmpty(&heroes[h].equip[slot])) return 0;
    if (!InvAdd(&heroes[h].equip[slot])) return 0;
    memset(&heroes[h].equip[slot], 0, sizeof(Item));
    RecalcHero(h);
    return 1;
}

static void HeroGainXp(int h, double xp) {
    Hero *H = &heroes[h];
    if (H->level >= HERO_MAX_LEVEL) return;
    H->xp += xp;
    while (H->level < HERO_MAX_LEVEL && H->xp >= XpNeeded(H->level)) {
        H->xp -= XpNeeded(H->level);
        H->level++;
        RecalcHero(h);
        H->hp = H->hpMax; H->mana = H->manaMax;
        HeroAutoAssignSkills(h);
    }
    if (H->level >= HERO_MAX_LEVEL) H->xp = 0;
}
static void PlayerGainXp(double xp) {
    if (player.level >= PLAYER_MAX_LEVEL) return;
    player.xp += xp;
    while (player.level < PLAYER_MAX_LEVEL && player.xp >= PlayerXpNeeded(player.level)) {
        player.xp -= PlayerXpNeeded(player.level);
        player.level++;
    }
}

static void ClearHero(int h) {
    memset(&heroes[h], 0, sizeof heroes[h]);
    heroes[h].stage = -1; heroes[h].inst = -1; heroes[h].focus = -1; heroes[h].partner = -1;
    for (int i = 0; i < 4; i++) heroes[h].skillSlots[i] = -1;
    heroes[h].hpMax = heroes[h].manaMax = 1; heroes[h].hp = heroes[h].mana = 1;
    heroes[h].autoNext = 1;
}

/* legt einen neuen Helden der Klasse an; -1 falls kein Platz */
static int CreateHero(int cls) {
    if (HeroCount() >= HeroSlots()) return -1;
    int h = -1;
    for (int i = 0; i < MAX_HEROES; i++) if (!heroes[i].isHero) { h = i; break; }
    if (h < 0) return -1;
    ClearHero(h);
    Hero *H = &heroes[h];
    H->isHero = 1; H->class = cls; H->level = 1;
    H->chestMask = (1u << TIER_COUNT) - 1;
    H->autopilotEnabled = 1;
    int n = 1;
    for (int i = 0; i < MAX_HEROES; i++) if (i != h && heroes[i].isHero && heroes[i].class == cls) n++;
    snprintf(H->name, sizeof H->name, "%s %d", ClassName(cls), n);
    H->equip[EQ_WEAPON] = ItemMake(CAT_WEAPON, TIER_NORMAL, cls, 0);
    H->equip[EQ_POT1] = ItemMake(CAT_POTION, TIER_NORMAL, POT_HEALTH, 0);
    H->equip[EQ_POT2] = ItemMake(CAT_POTION, TIER_NORMAL, POT_MANA, 0);
    HeroAutoAssignSkills(h);
    RecalcHero(h);
    H->hp = H->hpMax; H->mana = H->manaMax;
    return h;
}

/* ================================================================== */
/* Kisten                                                             */
/* ================================================================== */

static int HeroChestTotal(int h) { int n = 0; for (int t = 0; t < TIER_COUNT; t++) n += heroes[h].chests[t]; return n; }

static void AddChest(int h, int tier, int mapLevel) {
    Hero *H = &heroes[h];
    if (!(H->chestMask & (1u << tier))) return;         /* Stufe abgewählt: Kiste wird nicht aufgehoben */
    if (HeroChestTotal(h) >= H->capacity) return;      /* Rucksack voll: Kiste verfällt */
    H->chests[tier]++;
    if (mapLevel > H->chestLevel[tier]) H->chestLevel[tier] = mapLevel;
}

/* öffnet alle Kisten eines Helden ins Spieler-Inventar. Rückgabe: Anzahl geöffnet */
static int CollectChests(int h, int *outItems) {
    Hero *H = &heroes[h];
    int opened = 0, items = 0;
    for (int t = TIER_COUNT - 1; t >= 0; t--) {
        while (H->chests[t] > 0) {
            Item it = ChestLoot(t, H->chestLevel[t] > 0 ? H->chestLevel[t] : 1);
            if (!InvAdd(&it)) return opened;           /* Inventar voll */
            if (it.cat != CAT_MATERIAL) items++;
            H->chests[t]--; opened++;
        }
        H->chestLevel[t] = 0;
    }
    if (outItems) *outItems = items;
    return opened;
}

/* ================================================================== */
/* Potions                                                            */
/* ================================================================== */

static void HeroFillPotions(int h, int refill) {
    for (int s = EQ_POT1; s <= EQ_POT3; s++) {
        Item *p = &heroes[h].equip[s];
        if (p->cat != CAT_POTION) continue;
        int add = refill < 0 ? 100 : PotionKillRefill(p->tier);
        int c = p->count + add; p->count = (unsigned short)(c > 100 ? 100 : c);
    }
}

static int HeroBuffActive(int h, int b) { return heroes[h].buffF[b] > 0; }
static double HeroBuffPct(int h, int b) { return heroes[h].buffF[b] > 0 ? heroes[h].buffPct[b] : 0.0; }
static void HeroSetBuff(int h, int b, double pct, int frames) {
    if (heroes[h].buffF[b] <= 0 || pct >= heroes[h].buffPct[b] || frames > heroes[h].buffF[b]) {
        heroes[h].buffPct[b] = pct; heroes[h].buffF[b] = frames;
    }
}

static int HeroUsePotion(int h, int slot) {
    Hero *H = &heroes[h];
    Item *p = &H->equip[EQ_POT1 + slot];
    if (H->hp <= 0 || p->cat != CAT_POTION) return 0;
    int cost = PotionUseCost(p->tier);
    if (p->count < cost) return 0;
    double e = PotionEffectPct(p->kind, p->tier);
    switch (p->kind) {
    case POT_HEALTH:
        if (H->hp >= H->hpMax) return 0;
        H->hp += H->hpMax * e / 100.0 * (1.0 + H->healPower / 100.0); if (H->hp > H->hpMax) H->hp = H->hpMax;
        AddFloat(H->inst, H->x, H->y - 40, H->hpMax * e / 100.0, 0x40D060, 1);
        break;
    case POT_MANA:
        if (H->mana >= H->manaMax) return 0;
        H->mana += H->manaMax * e / 100.0; if (H->mana > H->manaMax) H->mana = H->manaMax;
        break;
    case POT_SPEED:    if (HeroBuffActive(h, BUFF_SPEED))    return 0; HeroSetBuff(h, BUFF_SPEED, e, PotionDurationFrames(p->tier)); break;
    case POT_ARMOR:    if (HeroBuffActive(h, BUFF_ARMOR))    return 0; HeroSetBuff(h, BUFF_ARMOR, e, PotionDurationFrames(p->tier)); break;
    case POT_ATKSPEED: if (HeroBuffActive(h, BUFF_ATKSPEED)) return 0; HeroSetBuff(h, BUFF_ATKSPEED, e, PotionDurationFrames(p->tier)); break;
    }
    p->count -= cost;
    AddFx(H->inst, FX_FLASH, H->x, H->y, 0, 0, 50, 15, p->kind == POT_MANA ? 0x4060FF : 0x40E060);
    return 1;
}

/* ================================================================== */
/* Karten-Instanzen                                                   */
/* ================================================================== */

static int StageUnlocked(int s) {
    if (s < 0 || s >= StageCount) return 0;
    return Stages[s].req < 0 || player.cleared[Stages[s].req];
}

static void KillInstanceObjects(int in) {
    for (int i = 0; i < MAX_PROJECTILES; i++) if (projectiles[i].valid && projectiles[i].inst == in) projectiles[i].valid = 0;
    for (int i = 0; i < MAX_FX; i++) if (fxs[i].used && fxs[i].inst == in) fxs[i].used = 0;
    for (int i = 0; i < MAX_FLOATTEXT; i++) if (floatTexts[i].life > 0 && floatTexts[i].inst == in) floatTexts[i].life = 0;
}

static void LeaveInstance(int h) {
    Hero *H = &heroes[h];
    if (H->inst >= 0) {
        Instance *I = &instances[H->inst];
        int n = 0;
        for (int k = 0; k < I->nHeroes; k++) if (I->heroId[k] != h) I->heroId[n++] = I->heroId[k];
        I->nHeroes = n;
        if (n == 0) { KillInstanceObjects(H->inst); I->used = 0; }
        else heroes[I->heroId[0]].partner = -1;
    }
    H->inst = -1; H->stage = -1; H->partner = -1; H->walking = 0; H->focus = -1;
}

static int PickMonsterType(int level, int role) {
    if (role != ROLE_NORMAL) return MONSTER_BRUTE;
    int r = Rnd(100);
    if (level < 6)  return r < 70 ? MONSTER_HUMAN : MONSTER_SLIME;
    if (level < 18) return r < 45 ? MONSTER_HUMAN : (r < 70 ? MONSTER_SLIME : MONSTER_BAT);
    if (level < 35) return r < 30 ? MONSTER_HUMAN : (r < 50 ? MONSTER_SLIME : (r < 75 ? MONSTER_BAT : MONSTER_SPITTER));
    return r < 20 ? MONSTER_HUMAN : (r < 35 ? MONSTER_SLIME : (r < 55 ? MONSTER_BAT : (r < 80 ? MONSTER_SPITTER : MONSTER_BRUTE)));
}

static void InitMonster(Monster *m, const StageDef *S, int type, int role, double x, double y) {
    int tier = TIER_NORMAL;
    if (role == ROLE_BOSS) tier = TIER_BOSS;
    else if (role == ROLE_MINIBOSS) tier = TIER_RARE;
    else {
        int r = Rnd(2000);
        tier = r < 1 ? TIER_LEGENDARY : (r < 11 ? TIER_UNIQUE : (r < 51 ? TIER_RARE : (r < 251 ? TIER_MAGIC : TIER_NORMAL)));
    }
    const MonsterInfo *mi = &MonsterInfos[type];
    memset(m, 0, sizeof *m);
    m->exist = m->alive = 1; m->type = type; m->tier = tier; m->role = role; m->level = S->level;
    m->x = x; m->y = y;
    m->hpMax = m->hp = MonsterBaseHP(S->level) * S->hpMul * mi->hp * MONSTER_TIER_HP[tier];
    m->atk = MonsterBaseAttack(S->level) * S->atkMul * mi->atk * MONSTER_TIER_ATK[tier];
    m->speed = mi->speed * (role == ROLE_BOSS ? 0.9 : 1.0);
    m->radius = mi->radius * (role == ROLE_BOSS ? 1.7 : (role == ROLE_MINIBOSS ? 1.3 : (tier >= TIER_UNIQUE ? 1.15 : 1.0)));
    m->aggro = (role == ROLE_NORMAL) ? 900 : 1200;
    m->lastHero = -1;
    m->attackCd = 20 + Rnd(30); m->shootCd = 60 + Rnd(60); m->specialCd = 240 + Rnd(120);
}

static void SpawnInstanceMonsters(int in) {
    Instance *I = &instances[in];
    const StageDef *S = &Stages[I->stage];
    const Template *T = &Templates[S->tmpl];
    memset(I->mon, 0, sizeof I->mon);
    int n = 0;
    for (int i = 0; i < T->nSpawn && n < MAX_MONSTERS; i++) {
        double x = T->spawn[i].x, y = T->spawn[i].y;
        int role = T->spawn[i].role;
        if (getMapCollisionBoxCenter(x, y, 24)) {          /* Position in der Wand: Umgebung absuchen */
            int found = 0;
            for (int r = 40; r <= 240 && !found; r += 40)
                for (int a = 0; a < 8 && !found; a++) {
                    double dx, dy; Dir(a * 45.0, &dx, &dy);
                    if (!getMapCollisionBoxCenter(x + dx * r, y + dy * r, 24)) { x += dx * r; y += dy * r; found = 1; }
                }
            if (!found) continue;
        }
        InitMonster(&I->mon[n++], S, PickMonsterType(S->level, role), role, x, y);
    }
}

static void PlaceHeroAtStart(int h, int slot, const Template *T) {
    Hero *H = &heroes[h];
    H->x = T->startX + slot * 90; H->y = T->startY + (slot ? 60 : 0);
    if (getMapCollisionBoxCenter(H->x, H->y, HERO_RADIUS)) { H->x = T->startX; H->y = T->startY; }
    H->xTarget = H->x; H->yTarget = H->y;
    H->angle = T->angle;
    H->hp = H->hpMax; H->mana = H->manaMax;
    H->walking = H->attacking = H->skilling = 0; H->walkCycle = 0; H->attackCycle = 0;
    H->waypoint = 0; H->retreat = 0; H->focus = -1; H->manualTimer = 0;
    for (int i = 0; i < 4; i++) { H->skillCd[i] = 0; H->buffF[i] = 0; }
    HeroFillPotions(h, -1);
}

static void RestartInstance(int in) {
    Instance *I = &instances[in];
    KillInstanceObjects(in);
    SpawnInstanceMonsters(in);
    const Template *T = &Templates[Stages[I->stage].tmpl];
    for (int k = 0; k < I->nHeroes; k++) PlaceHeroAtStart(I->heroId[k], k, T);
    I->state = RUN_PLAYING; I->timer = 0; I->nextStage = -1;
}

/* Startet Karte "stage" für Held ha (und optional hb, falls die Karte Gruppen erlaubt). */
static int StartStage(int ha, int stage, int hb) {
    if (!StageUnlocked(stage) || !heroes[ha].isHero) return 0;
    if (hb >= 0 && (hb == ha || !heroes[hb].isHero || Stages[stage].partyMax < 2)) hb = -1;
    LeaveInstance(ha);
    if (hb >= 0) LeaveInstance(hb);
    int in = -1;
    for (int i = 0; i < MAX_HEROES; i++) if (!instances[i].used) { in = i; break; }
    if (in < 0) return 0;
    Instance *I = &instances[in];
    memset(I, 0, sizeof *I);
    I->used = 1; I->stage = stage; I->nextStage = -1;
    I->heroId[I->nHeroes++] = ha;
    if (hb >= 0) I->heroId[I->nHeroes++] = hb;
    for (int k = 0; k < I->nHeroes; k++) {
        Hero *H = &heroes[I->heroId[k]];
        H->inst = in; H->stage = stage;
        H->partner = (I->nHeroes == 2) ? I->heroId[1 - k] : -1;
    }
    RestartInstance(in);
    return 1;
}

static int InstanceAliveHeroes(int in) {
    int n = 0;
    for (int k = 0; k < instances[in].nHeroes; k++) if (heroes[instances[in].heroId[k]].hp > 0) n++;
    return n;
}

/* ================================================================== */
/* Schaden                                                            */
/* ================================================================== */

static void StageCleared(int in) {
    Instance *I = &instances[in];
    int first = !player.cleared[I->stage];
    player.cleared[I->stage] = 1;
    I->state = RUN_CLEARED; I->timer = 240;
    const StageDef *S = &Stages[I->stage];
    double bonus = GoldPerKill(S->level) * (first ? 60 : 15);
    player.gold += bonus;
    for (int k = 0; k < I->nHeroes; k++) HeroGainXp(I->heroId[k], XpForKill(S->level) * (first ? 40 : 10) * S->xpMul);
    PlayerGainXp(XpForKill(S->level) * (first ? 40 : 10));
    /* automatisch zur nächsten Geschichts-Karte, wenn das erlaubt ist */
    int next = I->stage + 1;
    if (next < StageCount && !player.cleared[next] && Stages[next].kind == STAGE_STORY && StageUnlocked(next) && Stages[next].partyMax >= I->nHeroes) {
        int all = 1;
        for (int k = 0; k < I->nHeroes; k++) if (!heroes[I->heroId[k]].autoNext) all = 0;
        if (all) I->nextStage = next;
    }
}

static const int TIER_ROLLS[TIER_COUNT] = { 1, 2, 3, 5, 10, 30 };

static void MonsterDies(int in, int mi) {
    Instance *I = &instances[in];
    Monster *m = &I->mon[mi];
    if (!m->alive) return;
    m->alive = 0; m->hp = 0;
    const StageDef *S = &Stages[I->stage];
    double xp = XpForKill(m->level) * MONSTER_TIER_XP[m->tier] * S->xpMul;
    for (int k = 0; k < I->nHeroes; k++) {
        int h = I->heroId[k];
        if (heroes[h].hp <= 0) continue;
        HeroGainXp(h, xp);
        heroes[h].kills++;
        HeroFillPotions(h, 0);
    }
    PlayerGainXp(xp);
    player.totalKills++;
    player.gold += GoldPerKill(m->level) * MONSTER_TIER_XP[m->tier] * 0.5;
    /* Kisten: der Held, der den Kill gemacht hat (sonst der erste lebende) */
    int owner = m->lastHero >= 0 ? m->lastHero : I->heroId[0];
    if (owner < 0 || heroes[owner].hp <= 0) owner = I->heroId[0];
    for (int r = 0; r < TIER_ROLLS[m->tier]; r++)
        for (int t = 0; t < TIER_COUNT; t++)
            if ((unsigned)rand() % (unsigned)TierChestDenominator(t) == 0) AddChest(owner, t, S->level);
    AddFx(in, FX_FLASH, m->x, m->y, 0, 0, m->radius * 1.6, 12, 0xFFFFFF);
    if (m->role == ROLE_BOSS && I->state == RUN_PLAYING) StageCleared(in);
}

static void DamageMonster(int in, int mi, double dmg, int owner, int crit) {
    Monster *m = &instances[in].mon[mi];
    if (!m->alive || dmg <= 0) return;
    m->hp -= dmg; m->hitFlash = 6;
    if (owner >= 0) m->lastHero = owner;
    AddFloat(in, m->x + Rnd(30) - 15, m->y - m->radius, dmg, crit ? 0xFFD020 : 0xFFFFFF, crit ? 2 : 0);
    if (m->hp <= 0) MonsterDies(in, mi);
}

static void HeroTakeDamage(int h, double raw, int level) {
    Hero *H = &heroes[h];
    if (H->hp <= 0 || raw <= 0) return;
    double def = H->def * (1.0 + HeroBuffPct(h, BUFF_ARMOR) / 100.0);
    double k = MonsterArmorK(level);
    double dmg = raw * k / (k + def) * (1.0 - H->block / 100.0);
    if (dmg < 1) dmg = 1;
    H->hp -= dmg;
    AddFloat(H->inst, H->x + Rnd(20) - 10, H->y - 50, dmg, 0xFF5050, 3);
    if (H->hp <= 0) { H->hp = 0; H->walking = 0; H->attacking = 0; H->skilling = 0; }
}

/* Schadenswurf des Helden: Zufall +-10 %, Buffs, Krit */
static double HeroRollDamage(int h, double mul, int *crit) {
    Hero *H = &heroes[h];
    double d = H->atk * mul * (1.0 + HeroBuffPct(h, BUFF_ATTACK) / 100.0) * (0.9 + 0.2 * Rnd01());
    *crit = (Rnd(1000) < (int)(H->crit * 10.0));
    if (*crit) d *= H->critMul;
    return d;
}

/* ================================================================== */
/* Projektile                                                         */
/* ================================================================== */

static void SpawnProjectile(int in, int faction, int owner, double x, double y, double angle, int visual,
                            double speed, double damage, int pierce, double radius, int life, int crit, double poison) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (projectiles[i].valid == 0) {
            Projectile *p = &projectiles[i];
            memset(p, 0, sizeof *p);
            p->valid = 1; p->inst = in; p->faction = faction; p->owner = owner; p->visual = visual;
            p->x = x; p->y = y; p->angle = angle; p->speed = speed; p->damage = damage;
            p->pierce = pierce; p->radius = radius; p->life = life; p->crit = crit; p->poison = poison;
            return;
        }
    }
}

static void ProjectileUpdate(int pi) {
    Projectile *p = &projectiles[pi];
    if (!p->valid) return;
    Instance *I = &instances[p->inst];
    if (!I->used || --p->life <= 0) { p->valid = 0; return; }
    double dx, dy; Dir(p->angle, &dx, &dy);
    p->x += dx * p->speed; p->y += dy * p->speed;
    if (getMapCollision((int)p->x, (int)p->y)) { p->valid = 0; return; }
    if (p->faction == 0) {
        for (int m = 0; m < MAX_MONSTERS && p->valid; m++) {
            Monster *mo = &I->mon[m];
            if (!mo->alive) continue;
            if (Dist(p->x, p->y, mo->x, mo->y) > mo->radius + p->radius) continue;
            int seen = 0;
            for (int k = 0; k < p->nhit; k++) if (p->hit[k] == m) seen = 1;
            if (seen) continue;
            DamageMonster(p->inst, m, p->damage, p->owner, p->crit);
            if (p->poison > 0 && mo->alive) { mo->poisonDps = p->poison; mo->poisonFrames = 180; }
            if (p->nhit < 8) p->hit[p->nhit++] = m;
            if (p->pierce-- <= 0) p->valid = 0;
        }
    } else {
        for (int k = 0; k < I->nHeroes && p->valid; k++) {
            int h = I->heroId[k];
            if (heroes[h].hp > 0 && Dist(p->x, p->y, heroes[h].x, heroes[h].y) < HERO_RADIUS + p->radius) {
                HeroTakeDamage(h, p->damage, Stages[I->stage].level);
                p->valid = 0;
            }
        }
    }
}

/* ================================================================== */
/* Monster-KI                                                         */
/* ================================================================== */

static int MonsterNearestHero(int in, const Monster *m, double *outDist) {
    Instance *I = &instances[in];
    int best = -1; double bd = BIGFLOAT;
    for (int k = 0; k < I->nHeroes; k++) {
        int h = I->heroId[k];
        if (heroes[h].hp <= 0) continue;
        double d = Dist(m->x, m->y, heroes[h].x, heroes[h].y);
        if (d < bd) { bd = d; best = h; }
    }
    if (outDist) *outDist = bd;
    return best;
}

static void MonsterMove(Monster *m, double angle, double speed) {
    double dx, dy; Dir(angle, &dx, &dy);
    double nx = m->x + dx * speed, ny = m->y + dy * speed;
    if (!getMapCollisionBoxCenter(nx, ny, 12)) { m->x = nx; m->y = ny; }
    else if (!getMapCollisionBoxCenter(m->x, ny, 12)) m->y = ny;
    else if (!getMapCollisionBoxCenter(nx, m->y, 12)) m->x = nx;
}

static void MonsterUpdate(int in, int mi) {
    Instance *I = &instances[in];
    Monster *m = &I->mon[mi];
    const StageDef *S = &Stages[I->stage];
    if (!m->alive) return;
    if (m->attackCd > 0) m->attackCd--;
    if (m->shootCd > 0) m->shootCd--;
    if (m->specialCd > 0) m->specialCd--;
    if (m->hitFlash > 0) m->hitFlash--;
    if (m->poisonFrames > 0) {
        m->poisonFrames--;
        m->hp -= m->poisonDps / 60.0;
        if (m->hp <= 0) { MonsterDies(in, mi); return; }
    }
    double d;
    int h = MonsterNearestHero(in, m, &d);
    if (h < 0) return;
    if (d > m->aggro) {                           /* ruhig umherstehen */
        if (--m->wanderT <= 0) { m->wanderT = 60 + Rnd(120); m->wanderA = Rnd(360); }
        if (m->wanderT > 40) MonsterMove(m, m->wanderA, m->speed * 0.3);
        return;
    }
    m->angle = LookAt(m->x, m->y, heroes[h].x, heroes[h].y);
    double contact = m->radius + HERO_RADIUS + 6;

    switch (m->type) {
    case MONSTER_SPITTER:
        if (d > 460) MonsterMove(m, m->angle, m->speed);
        else if (d < 300) MonsterMove(m, m->angle + 180, m->speed * 0.8);
        if (d < 700 && m->shootCd <= 0) {
            SpawnProjectile(in, 1, -1, m->x, m->y, m->angle, PROJ_ENEMY, 9, m->atk, 0, 14, 110, 0, 0);
            m->shootCd = 100;
        }
        break;
    case MONSTER_BAT: {
        double a = m->angle + sin(engineStep * 0.09 + mi) * 55;
        if (d > contact * 0.9) MonsterMove(m, a, m->speed);
        if (d < contact && m->attackCd <= 0) { HeroTakeDamage(h, m->atk, S->level); m->attackCd = 28; }
        break; }
    default:
        if (d > contact * 0.9) MonsterMove(m, m->angle, m->speed);
        if (d < contact && m->attackCd <= 0) {
            HeroTakeDamage(h, m->atk, S->level);
            m->attackCd = (m->type == MONSTER_BRUTE) ? 70 : 40;
        }
        break;
    }
    /* Boss-Muster: Projektilring */
    if (m->role != ROLE_NORMAL && m->specialCd <= 0 && d < 900) {
        int n = (m->role == ROLE_BOSS) ? 16 : 10;
        double off = Rnd(360);
        for (int i = 0; i < n; i++)
            SpawnProjectile(in, 1, -1, m->x, m->y, off + i * 360.0 / n, PROJ_ENEMY, 6, m->atk * 0.5, 0, 16, 150, 0, 0);
        AddFx(in, FX_RING, m->x, m->y, 0, 0, m->radius * 2.2, 20, 0xFF3030);
        m->specialCd = (m->role == ROLE_BOSS) ? 300 : 420;
    }
}

/* Gegner schieben sich auseinander (kein Stapeln) */
static void SeparateMonsters(int in) {
    Instance *I = &instances[in];
    for (int i = 0; i < MAX_MONSTERS; i++) {
        Monster *a = &I->mon[i];
        if (!a->alive) continue;
        for (int j = i + 1; j < MAX_MONSTERS; j++) {
            Monster *b = &I->mon[j];
            if (!b->alive) continue;
            double dx = b->x - a->x, dy = b->y - a->y;
            double d2 = dx * dx + dy * dy, r = (a->radius + b->radius) * 0.7;
            if (d2 >= r * r) continue;
            double d = sqrt(d2);
            if (d < 0.01) { dx = 1; dy = 0; d = 1; }
            double push = (r - d) * 0.25 > 2.5 ? 2.5 : (r - d) * 0.25;
            double ux = dx / d * push, uy = dy / d * push;
            if (!getMapCollisionBoxCenter(a->x - ux, a->y - uy, 10)) { a->x -= ux; a->y -= uy; }
            if (!getMapCollisionBoxCenter(b->x + ux, b->y + uy, 10)) { b->x += ux; b->y += uy; }
        }
    }
}

/* ================================================================== */
/* Helden-Kampf                                                       */
/* ================================================================== */

static int HeroAttackTotal(int h) {
    double s = heroes[h].atkSpeed * (1.0 + HeroBuffPct(h, BUFF_ATKSPEED) / 100.0);
    int t = (int)(ClassAttackInterval(heroes[h].class) / s);
    return t < 4 ? 4 : t;
}
static double HeroMoveSpeed(int h) { return heroes[h].moveSpeed * (1.0 + HeroBuffPct(h, BUFF_SPEED) / 100.0); }

/* freie Sichtlinie (keine Wand dazwischen)? */
static int HasLineOfSight(double x1, double y1, double x2, double y2) {
    double d = Dist(x1, y1, x2, y2);
    int n = (int)(d / 24.0);
    for (int i = 1; i < n; i++) {
        double t = (double)i / n;
        if (getMapCollision((int)(x1 + (x2 - x1) * t), (int)(y1 + (y2 - y1) * t))) return 0;
    }
    return 1;
}

static int HeroNearestMonster(int h, double maxDist, double *outDist) {
    Hero *H = &heroes[h];
    if (H->inst < 0) return -1;
    Instance *I = &instances[H->inst];
    int best = -1; double bd = maxDist;
    for (int i = 0; i < MAX_MONSTERS; i++) {
        if (!I->mon[i].alive) continue;
        double d = Dist(H->x, H->y, I->mon[i].x, I->mon[i].y);
        if (d < bd) { bd = d; best = i; }
    }
    if (outDist) *outDist = bd;
    return best;
}

static int CountMonstersNear(int h, double radius) {
    Hero *H = &heroes[h]; Instance *I = &instances[H->inst]; int n = 0;
    for (int i = 0; i < MAX_MONSTERS; i++)
        if (I->mon[i].alive && Dist(H->x, H->y, I->mon[i].x, I->mon[i].y) - I->mon[i].radius < radius) n++;
    return n;
}

/* Nahkampf: trifft Monster in Reichweite und Winkel (arc >= 360: Vollkreis).
 * maxTargets == 1: nur das nächste Ziel, sonst alle. */
static int MeleeHit(int h, double cx, double cy, double angle, double range, double arc, int maxTargets, double mul) {
    Hero *H = &heroes[h]; Instance *I = &instances[H->inst];
    int hits = 0, best = -1; double bd = BIGFLOAT;
    for (int i = 0; i < MAX_MONSTERS; i++) {
        Monster *m = &I->mon[i];
        if (!m->alive) continue;
        double d = Dist(cx, cy, m->x, m->y) - m->radius;
        if (d > range) continue;
        /* Winkeltoleranz wächst für nahe/große Gegner */
        double tol = arc / 2.0 + (d < m->radius ? 90.0 : 0.0);
        if (arc < 360.0 && fabs(AngleDiff(LookAt(cx, cy, m->x, m->y), angle)) > tol) continue;
        if (maxTargets == 1) { if (d < bd) { bd = d; best = i; } continue; }
        int crit; double dmg = HeroRollDamage(h, mul, &crit);
        DamageMonster(H->inst, i, dmg, h, crit);
        hits++;
    }
    if (maxTargets == 1 && best >= 0) {
        int crit; double dmg = HeroRollDamage(h, mul, &crit);
        DamageMonster(H->inst, best, dmg, h, crit);
        hits = 1;
    }
    return hits;
}

/* Fläche: alle Monster innerhalb von radius (und optional Winkel) */
static int AreaHit(int h, double cx, double cy, double facing, double radius, double arc, double mul, double poisonDps, int poisonFrames) {
    Hero *H = &heroes[h]; Instance *I = &instances[H->inst];
    int hits = 0;
    for (int i = 0; i < MAX_MONSTERS; i++) {
        Monster *m = &I->mon[i];
        if (!m->alive) continue;
        double d = Dist(cx, cy, m->x, m->y) - m->radius * 0.6;
        if (d > radius) continue;
        if (arc < 360.0 && fabs(AngleDiff(LookAt(cx, cy, m->x, m->y), facing)) > arc / 2.0) continue;
        int crit; double dmg = HeroRollDamage(h, mul, &crit);
        if (poisonFrames > 0) { m->poisonDps = poisonDps; m->poisonFrames = poisonFrames; }
        DamageMonster(H->inst, i, dmg, h, crit);
        hits++;
    }
    return hits;
}

static void HeroHealSelfAndPartner(int h, double pctOfMax) {
    Hero *H = &heroes[h];
    int ids[2] = { h, H->partner };
    for (int k = 0; k < 2; k++) {
        if (ids[k] < 0 || heroes[ids[k]].hp <= 0) continue;
        Hero *T = &heroes[ids[k]];
        double heal = T->hpMax * pctOfMax / 100.0 * (1.0 + H->healPower / 100.0);
        T->hp += heal; if (T->hp > T->hpMax) T->hp = T->hpMax;
        AddFloat(T->inst, T->x, T->y - 40, heal, 0x40D060, 1);
        AddFx(T->inst, FX_FLASH, T->x, T->y, 0, 0, 60, 18, 0x40E070);
    }
}

static void StartHeroAttackAnim(int h) {
    heroes[h].attackTotal = HeroAttackTotal(h);
    heroes[h].attacking = 1; heroes[h].attackCycle = 0;
}

/* Basisangriff in Richtung angle (Ziel monster mi oder -1) */
static void HeroBasicAttack(int h, double angle, int mi) {
    Hero *H = &heroes[h];
    if (H->hp <= 0 || H->attacking || H->skilling || H->inst < 0) return;
    Instance *I = &instances[H->inst];
    H->angle = angle;
    double range = ClassAttackRange(H->class);
    int crit; double dmg;
    switch (H->class) {
    case CLASS_Knight:   MeleeHit(h, H->x, H->y, angle, range, 110, 99, 1.0); AddFx(H->inst, FX_CONE, H->x, H->y, angle, 110, range, 8, 0xFFFFFF); break;
    case CLASS_Warrior:  AreaHit(h, H->x, H->y, angle, range + 10, 360, 1.0, 0, 0); AddFx(H->inst, FX_RING, H->x, H->y, 0, 0, range + 10, 8, 0xFFD0A0); break;
    case CLASS_Assassin: MeleeHit(h, H->x, H->y, angle, range, 140, 1, 1.0); AddFx(H->inst, FX_CONE, H->x, H->y, angle, 60, range, 5, 0xC0C0FF); break;
    case CLASS_Archer:
        dmg = HeroRollDamage(h, 1.0, &crit);
        SpawnProjectile(H->inst, 0, h, H->x, H->y, angle, PROJ_ARROW, 20, dmg, 0, 14, 80, crit, 0); break;
    case CLASS_Mage:
        if (H->mana >= 1) { H->mana -= 1; dmg = HeroRollDamage(h, 1.0, &crit); }
        else dmg = HeroRollDamage(h, 0.4, &crit);
        SpawnProjectile(H->inst, 0, h, H->x, H->y, angle, PROJ_ARCANE, 11, dmg, 0, 22, 90, crit, 0); break;
    case CLASS_Priest:
        dmg = HeroRollDamage(h, 1.0, &crit);
        SpawnProjectile(H->inst, 0, h, H->x, H->y, angle, PROJ_HOLY, 13, dmg, 0, 20, 90, crit, 0);
        HeroHealSelfAndPartner(h, 0.8); break;
    case CLASS_Shaman: {
        int first = mi;
        if (first < 0 || !I->mon[first].alive) first = HeroNearestMonster(h, range, NULL);
        if (first < 0) break;
        int prev = -1; int hit[3] = { -1, -1, -1 };
        double px = H->x, py = H->y;
        for (int n = 0; n < 3; n++) {
            int t = (n == 0) ? first : -1;
            if (n > 0) {
                double bd = 230;
                for (int i = 0; i < MAX_MONSTERS; i++) {
                    if (!I->mon[i].alive || i == hit[0] || i == hit[1]) continue;
                    double d = Dist(px, py, I->mon[i].x, I->mon[i].y);
                    if (d < bd) { bd = d; t = i; }
                }
            }
            if (t < 0) break;
            hit[n] = t; prev = t;
            dmg = HeroRollDamage(h, n == 0 ? 1.0 : 0.6, &crit);
            AddFx(H->inst, FX_BOLT, px, py, I->mon[t].x, I->mon[t].y, 0, 8, 0xB0FF60);
            px = I->mon[t].x; py = I->mon[t].y;
            DamageMonster(H->inst, t, dmg, h, crit);
        }
        (void)prev;
        break; }
    }
    StartHeroAttackAnim(h);
}

/* ---- Skills ---- */

static int SkillManaCost(int h, const SkillDef *sk) {
    double c = heroes[h].manaMax * sk->manaPct / 100.0;
    return (int)(c < 1 ? 1 : c);
}

static void ChannelFire(int h, const SkillDef *sk) {
    Hero *H = &heroes[h];
    int crit; double dmg = HeroRollDamage(h, sk->mult, &crit);
    SpawnProjectile(H->inst, 0, h, H->x, H->y, H->angle + (Rnd01() - 0.5) * sk->angle, sk->visual, sk->speed, dmg, 0, 18, 80, crit, 0);
}

static int HeroUseSkill(int h, int slot) {
    Hero *H = &heroes[h];
    if (slot < 0 || slot > 3 || H->hp <= 0 || H->inst < 0 || H->skilling) return 0;
    int id = H->skillSlots[slot];
    if (!SkillUnlocked(h, id) || H->skillCd[slot] > 0) return 0;
    const SkillDef *sk = &SkillDefs[id];
    int cost = SkillManaCost(h, sk);
    if (H->mana < cost) return 0;
    Instance *I = &instances[H->inst];
    H->mana -= cost;
    H->skillCd[slot] = (int)(sk->cd / (1.0 + (H->atkSpeed - 1.0) * 0.5));
    int crit; double dmg;
    switch (sk->type) {
    case SKT_FAN: case SKT_CIRCLE: {
        int n = sk->n;
        for (int i = 0; i < n; i++) {
            double a;
            if (sk->type == SKT_CIRCLE) a = H->angle + i * 360.0 / n;
            else a = n == 1 ? H->angle : H->angle - sk->angle / 2 + sk->angle * i / (n - 1);
            dmg = HeroRollDamage(h, sk->mult, &crit);
            int pierce = (sk->visual == PROJ_DAGGER && n == 1) ? 5 : 0;
            double poison = sk->pct > 0 ? H->atk * sk->pct / 100.0 : 0;
            SpawnProjectile(H->inst, 0, h, H->x, H->y, a, sk->visual, sk->speed, dmg, pierce, 18, 90, crit, poison);
        }
        break; }
    case SKT_NOVA:
        AreaHit(h, H->x, H->y, H->angle, sk->radius, sk->angle, sk->mult, 0, 0);
        AddFx(H->inst, FX_RING, H->x, H->y, 0, 0, sk->radius, 14, 0xFFFFFF);
        break;
    case SKT_CONE:
        AreaHit(h, H->x, H->y, H->angle, sk->radius, sk->angle, sk->mult, 0, 0);
        AddFx(H->inst, FX_CONE, H->x, H->y, H->angle, sk->angle, sk->radius, 12, 0xFFE0A0);
        break;
    case SKT_POISON:
        AreaHit(h, H->x, H->y, H->angle, sk->radius, 360, sk->mult * 0.3, H->atk * sk->mult, sk->dur);
        AddFx(H->inst, FX_RING, H->x, H->y, 0, 0, sk->radius, 20, 0x60D040);
        break;
    case SKT_DASH: {
        double dx, dy; Dir(H->angle, &dx, &dy);
        double sx = H->x, sy = H->y, travelled = 0;
        while (travelled < sk->radius) {
            double nx = H->x + dx * 20, ny = H->y + dy * 20;
            if (getMapCollisionBoxCenter(nx, ny, HERO_RADIUS)) break;
            H->x = nx; H->y = ny; travelled += 20;
            for (int i = 0; i < MAX_MONSTERS; i++) {
                Monster *m = &I->mon[i];
                if (!m->alive || m->hp <= 0) continue;
                if (Dist(H->x, H->y, m->x, m->y) < m->radius + 50 && m->hitFlash != 5) {
                    dmg = HeroRollDamage(h, sk->mult, &crit);
                    DamageMonster(H->inst, i, dmg, h, crit);
                    if (m->alive) m->hitFlash = 5;
                }
            }
        }
        AddFx(H->inst, FX_BOLT, sx, sy, H->x, H->y, 0, 10, 0xFFFFFF);
        H->xTarget = H->x; H->yTarget = H->y;
        break; }
    case SKT_CHAIN: {
        int hit[8]; int nh = 0;
        double px = H->x, py = H->y;
        for (int n = 0; n < sk->n && n < 8; n++) {
            int t = -1; double bd = sk->radius;
            for (int i = 0; i < MAX_MONSTERS; i++) {
                if (!I->mon[i].alive) continue;
                int used = 0; for (int k = 0; k < nh; k++) if (hit[k] == i) used = 1;
                if (used) continue;
                double d = Dist(px, py, I->mon[i].x, I->mon[i].y);
                if (d < bd) { bd = d; t = i; }
            }
            if (t < 0) break;
            hit[nh++] = t;
            AddFx(H->inst, FX_BOLT, px, py, I->mon[t].x, I->mon[t].y, 0, 10, 0x80C0FF);
            px = I->mon[t].x; py = I->mon[t].y;
            dmg = HeroRollDamage(h, sk->mult, &crit);
            DamageMonster(H->inst, t, dmg, h, crit);
        }
        if (nh == 0) { H->mana += cost; H->skillCd[slot] = 0; return 0; }   /* kein Ziel: nichts verbraucht */
        break; }
    case SKT_HEAL:
        HeroHealSelfAndPartner(h, sk->pct);
        break;
    case SKT_BUFF:
        HeroSetBuff(h, sk->buff, sk->pct, sk->dur);
        AddFx(H->inst, FX_FLASH, H->x, H->y, 0, 0, 70, 20, 0xFFE040);
        break;
    case SKT_RAIN:
        for (int n = 0; n < sk->n; n++) {
            double tx, ty;
            int t = -1, tries = 0;
            while (tries++ < 12) { int c = Rnd(MAX_MONSTERS); if (I->mon[c].alive && Dist(H->x, H->y, I->mon[c].x, I->mon[c].y) < sk->radius) { t = c; break; } }
            if (t >= 0) { tx = I->mon[t].x; ty = I->mon[t].y; }
            else { double dx, dy; Dir(Rnd(360), &dx, &dy); double r = Rnd01() * sk->radius; tx = H->x + dx * r; ty = H->y + dy * r; }
            AddFx(H->inst, FX_RING, tx, ty, 0, 0, 80, 16 + (n % 6), 0xFFB040);
            for (int i = 0; i < MAX_MONSTERS; i++) {
                Monster *m = &I->mon[i];
                if (!m->alive || Dist(tx, ty, m->x, m->y) > 90 + m->radius * 0.5) continue;
                dmg = HeroRollDamage(h, sk->mult, &crit);
                DamageMonster(H->inst, i, dmg, h, crit);
            }
        }
        break;
    case SKT_CHANNEL:
        H->skilling = 1; H->skillActive = id; H->skillCycle = 0;
        break;
    }
    if (sk->type != SKT_CHANNEL) StartHeroAttackAnim(h);
    return 1;
}

static void HeroChannelUpdate(int h) {
    Hero *H = &heroes[h];
    if (!H->skilling) return;
    const SkillDef *sk = &SkillDefs[H->skillActive];
    int every = sk->dur / (sk->n > 0 ? sk->n : 1); if (every < 1) every = 1;
    if (H->hp <= 0) { H->skilling = 0; return; }
    if (H->skillCycle % every == 0) ChannelFire(h, sk);
    if (++H->skillCycle >= sk->dur) { H->skilling = 0; H->skillCycle = 0; }
}

/* ================================================================== */
/* Helden-Bewegung + KI                                               */
/* ================================================================== */

/* bewegt den Helden Richtung (tx,ty); Rückgabe 1 falls bewegt */
static int HeroStepTo(int h, double tx, double ty) {
    Hero *H = &heroes[h];
    double dist = Dist(H->x, H->y, tx, ty);
    if (dist < 2.0) return 0;
    double speed = HeroMoveSpeed(h);
    double step = dist < speed ? dist : speed;
    double ang = LookAt(H->x, H->y, tx, ty);
    double dx, dy; Dir(ang, &dx, &dy);
    int sub = (int)(step / 8.0) + 1;
    int moved = 0;
    for (int i = 0; i < sub; i++) {
        double s = step / sub;
        double nx = H->x + dx * s, ny = H->y + dy * s;
        if (!getMapCollisionBoxCenter(nx, ny, HERO_RADIUS)) { H->x = nx; H->y = ny; moved = 1; }
        else if (!getMapCollisionBoxCenter(H->x, ny, HERO_RADIUS)) { H->y = ny; moved = 1; }
        else if (!getMapCollisionBoxCenter(nx, H->y, HERO_RADIUS)) { H->x = nx; moved = 1; }
        else break;
    }
    return moved;
}

static int HeroPotionSlotOfKind(int h, int kind) {
    for (int s = 0; s < 3; s++) {
        Item *p = &heroes[h].equip[EQ_POT1 + s];
        if (p->cat == CAT_POTION && p->kind == kind && p->count >= PotionUseCost(p->tier)) return s;
    }
    return -1;
}

static void HeroAIPotions(int h, int nNear, int bossNear) {
    Hero *H = &heroes[h];
    int s;
    if (H->hp < H->hpMax * 0.5 && (s = HeroPotionSlotOfKind(h, POT_HEALTH)) >= 0) HeroUsePotion(h, s);
    if (H->mana < H->manaMax * 0.25 && (s = HeroPotionSlotOfKind(h, POT_MANA)) >= 0) HeroUsePotion(h, s);
    if (nNear >= 3 || bossNear) {
        if ((s = HeroPotionSlotOfKind(h, POT_ARMOR)) >= 0)    HeroUsePotion(h, s);
        if ((s = HeroPotionSlotOfKind(h, POT_ATKSPEED)) >= 0) HeroUsePotion(h, s);
    }
    if (nNear >= 1 && H->hp < H->hpMax * 0.7 && (s = HeroPotionSlotOfKind(h, POT_SPEED)) >= 0) HeroUsePotion(h, s);
}

static void HeroAISkills(int h, int tgt, double tgtDist) {
    Hero *H = &heroes[h];
    if (H->skilling || H->attacking) return;
    for (int slot = 0; slot < 4; slot++) {
        int id = H->skillSlots[slot];
        if (!SkillUnlocked(h, id) || H->skillCd[slot] > 0) continue;
        const SkillDef *sk = &SkillDefs[id];
        int cost = SkillManaCost(h, sk);
        int reserve = (int)(H->manaMax * 0.08);
        int use = 0;
        int nR = tgt >= 0 ? CountMonstersNear(h, sk->radius > 0 ? sk->radius * 0.9 : 300) : 0;
        int boss = 0;
        if (tgt >= 0) { Monster *m = &instances[H->inst].mon[tgt]; boss = m->role != ROLE_NORMAL || m->tier >= TIER_RARE; }
        switch (sk->type) {
        case SKT_HEAL:   use = H->hp < H->hpMax * 0.55 && H->mana >= cost; break;
        case SKT_BUFF:   use = tgt >= 0 && !HeroBuffActive(h, sk->buff) && (nR >= 3 || boss) && tgtDist < 500; break;
        case SKT_NOVA: case SKT_CONE: case SKT_POISON:
                         use = tgt >= 0 && (nR >= 2 || (boss && tgtDist < sk->radius)); break;
        case SKT_DASH:   use = tgt >= 0 && tgtDist > 180 && tgtDist < sk->radius * 0.9 && H->hp > H->hpMax * 0.4; break;
        case SKT_CHAIN:  use = tgt >= 0 && tgtDist < sk->radius * 0.9; break;
        case SKT_RAIN:   use = tgt >= 0 && tgtDist < sk->radius && (nR >= 3 || boss); break;
        case SKT_CIRCLE: use = tgt >= 0 && CountMonstersNear(h, 500) >= 3; break;
        default:         use = tgt >= 0 && tgtDist < 600; break;
        }
        if (use && (sk->type == SKT_HEAL || H->mana >= cost + reserve)) { if (HeroUseSkill(h, slot)) return; }
    }
}

/* Gehe zum Wegpunkt-Pfad; liefert 1 falls ein Ziel gesetzt wurde */
static void HeroPatrol(int h) {
    Hero *H = &heroes[h];
    const Template *T = &Templates[Stages[H->stage].tmpl];
    if (H->waypoint < T->nWay) {
        double wx = T->way[H->waypoint].x, wy = T->way[H->waypoint].y;
        if (Dist(H->x, H->y, wx, wy) < HeroMoveSpeed(h) * 1.1 + 12) { H->waypoint++; return; }
        H->walking = HeroStepTo(h, wx, wy);
        if (H->walking) H->angle = LookAt(H->x, H->y, wx, wy);
        return;
    }
    /* Weg zu Ende: übrige Gegner direkt ansteuern (kein Pathfinding) */
    int m = HeroNearestMonster(h, BIGFLOAT, NULL);
    if (m >= 0) {
        Monster *mo = &instances[H->inst].mon[m];
        H->walking = HeroStepTo(h, mo->x, mo->y);
        if (H->walking) H->angle = LookAt(H->x, H->y, mo->x, mo->y);
    }
}

static void HeroAI(int h) {
    Hero *H = &heroes[h];
    double range = ClassAttackRange(H->class);
    double tgtDist = BIGFLOAT;
    int tgt = (H->focus >= 0 && instances[H->inst].mon[H->focus].alive) ? H->focus : -1;
    if (tgt < 0) { H->focus = -1; tgt = HeroNearestMonster(h, AGGRO_RANGE, &tgtDist); }
    else tgtDist = Dist(H->x, H->y, instances[H->inst].mon[tgt].x, instances[H->inst].mon[tgt].y);
    /* Ziele hinter Wänden ignorieren (kein Pathfinding), ebenso kurz nach dem Festhängen */
    if (tgt >= 0 && tgtDist > 100) {
        Monster *tm = &instances[H->inst].mon[tgt];
        if (H->ignoreT > 0 || !HasLineOfSight(H->x, H->y, tm->x, tm->y)) { tgt = -1; H->focus = -1; tgtDist = BIGFLOAT; }
    }

    int nNear = CountMonstersNear(h, 450);
    int bossNear = 0;
    if (tgt >= 0) { Monster *m = &instances[H->inst].mon[tgt]; bossNear = (m->role != ROLE_NORMAL || m->tier >= TIER_UNIQUE) && tgtDist < 700; }

    /* Rückzug bei niedrigem Leben und ohne Heiltrank */
    if (H->hp < H->hpMax * 0.22 && HeroPotionSlotOfKind(h, POT_HEALTH) < 0 && nNear > 0) H->retreat = 1;
    if (H->hp > H->hpMax * 0.6 || nNear == 0) H->retreat = 0;

    HeroAIPotions(h, nNear, bossNear);
    HeroAISkills(h, tgt, tgtDist);

    H->walking = 0;
    if (tgt < 0) { HeroPatrol(h); return; }

    Monster *m = &instances[H->inst].mon[tgt];
    double face = LookAt(H->x, H->y, m->x, m->y);
    double surface = tgtDist - m->radius;
    int ranged = ClassIsRanged(H->class);
    double hold = ranged ? range * 0.7 : range * 0.8;

    if (H->retreat) {                                  /* weglaufen */
        double dx, dy; Dir(face + 180, &dx, &dy);
        H->walking = HeroStepTo(h, H->x + dx * 100, H->y + dy * 100);
        if (H->walking) H->angle = face;
    } else if (ranged && surface < range * 0.30) {     /* Abstand halten (kiten) */
        double dx, dy; Dir(face + 180, &dx, &dy);
        H->walking = HeroStepTo(h, H->x + dx * 100, H->y + dy * 100);
        if (!H->walking) H->walking = 0;
        H->angle = face;
    } else if (surface > hold) {                       /* näher ran */
        H->walking = HeroStepTo(h, m->x, m->y);
        H->angle = face;
    } else {
        H->angle = face;
    }
    if (surface <= range) HeroBasicAttack(h, face, tgt);
}

/* manuelle Steuerung: Ziel-Monster oder Laufziel */
static void HeroManual(int h) {
    Hero *H = &heroes[h];
    H->walking = 0;
    if (H->focus >= 0 && instances[H->inst].mon[H->focus].alive) {
        Monster *m = &instances[H->inst].mon[H->focus];
        double range = ClassAttackRange(H->class);
        double d = Dist(H->x, H->y, m->x, m->y) - m->radius;
        double face = LookAt(H->x, H->y, m->x, m->y);
        if (d > range * 0.8) { H->walking = HeroStepTo(h, m->x, m->y); H->angle = face; }
        if (d <= range) HeroBasicAttack(h, face, H->focus);
        return;
    }
    H->focus = -1;
    if (Dist(H->x, H->y, H->xTarget, H->yTarget) > 6) {
        H->walking = HeroStepTo(h, H->xTarget, H->yTarget);
        if (H->walking) H->angle = LookAt(H->x, H->y, H->xTarget, H->yTarget);
    }
}

static void HeroUpdate(int h) {
    Hero *H = &heroes[h];
    if (!H->isHero || H->stage < 0 || H->inst < 0) return;
    for (int s = 0; s < 4; s++) { if (H->skillCd[s] > 0) H->skillCd[s]--; if (H->buffF[s] > 0) H->buffF[s]--; }
    if (H->hp <= 0) { H->walking = 0; return; }
    if (H->hp < H->hpMax) { H->hp += H->hpRegen / 60.0; if (H->hp > H->hpMax) H->hp = H->hpMax; }
    if (H->mana < H->manaMax) { H->mana += H->manaRegen / 60.0; if (H->mana > H->manaMax) H->mana = H->manaMax; }
    if (H->attacking && ++H->attackCycle >= H->attackTotal) { H->attacking = 0; H->attackCycle = 0; }
    HeroChannelUpdate(h);
    if (instances[H->inst].state != RUN_PLAYING) { H->walking = 0; return; }
    if (H->manualTimer > 0) H->manualTimer--;
    if (H->skilling) { H->walking = 0; return; }
    if (H->ignoreT > 0) H->ignoreT--;
    if (H->autopilotEnabled && H->manualTimer <= 0) HeroAI(h);
    else HeroManual(h);
    /* Festhänge-Erkennung (nur Autopilot): lange kaum bewegt, obwohl Laufen gewollt */
    if (H->autopilotEnabled && H->manualTimer <= 0) {
        if (++H->stuckT >= 150) {
            if (Dist(H->x, H->y, H->stuckX, H->stuckY) < 30 && H->walking && !H->attacking) {
                H->waypoint++; H->ignoreT = 240; H->focus = -1;
            }
            H->stuckX = H->x; H->stuckY = H->y; H->stuckT = 0;
        }
    }
    if (H->walking) H->walkCycle++; else H->walkCycle = 0;
}

static void InstanceUpdate(int in) {
    Instance *I = &instances[in];
    if (!I->used) return;
    if (I->state == RUN_PLAYING) {
        for (int i = 0; i < MAX_MONSTERS; i++) MonsterUpdate(in, i);
        SeparateMonsters(in);
        if (InstanceAliveHeroes(in) == 0) { I->state = RUN_LOST; I->timer = 240; }
    } else {
        if (--I->timer <= 0) {
            if (I->state == RUN_CLEARED && I->nextStage >= 0) {
                int a = I->heroId[0], b = I->nHeroes > 1 ? I->heroId[1] : -1, ns = I->nextStage;
                StartStage(a, ns, b);
            } else RestartInstance(in);
        }
    }
}

static void SimStep(void) {
    for (int i = 0; i < MAX_HEROES; i++) HeroUpdate(i);
    for (int i = 0; i < MAX_HEROES; i++) InstanceUpdate(i);
    for (int i = 0; i < MAX_PROJECTILES; i++) ProjectileUpdate(i);
    for (int i = 0; i < MAX_FX; i++) if (fxs[i].used && --fxs[i].life <= 0) fxs[i].used = 0;
    for (int i = 0; i < MAX_FLOATTEXT; i++) if (floatTexts[i].life > 0) { floatTexts[i].life--; floatTexts[i].y -= 1.2; }
}

/* ================================================================== */
/* Spieler-Aktionen (Crafting, Upgrade, Shop)                         */
/* ================================================================== */

static int CraftItem(const Item *target) {
    Recipe r;
    if (!ItemRecipe(target, &r) || !RecipeAffordable(&r) || InvFreeSlots() < 1) return 0;
    RecipePay(&r);
    InvAdd(target);
    return 1;
}

static int UpgradeItemPtr(Item *it) {
    Recipe r;
    if (!ItemUpgradeRecipe(it, &r) || !RecipeAffordable(&r)) return 0;
    RecipePay(&r);
    ItemLevelUp(it, Rnd(100));
    return 1;
}

static int SellItem(int invIdx) {
    Item *it = &player.inv[invIdx];
    if (ItemIsEmpty(it)) return 0;
    player.gold += ItemSellValue(it);
    memset(it, 0, sizeof *it);
    return 1;
}

/* ---- Shop: täglich wechselndes Materiallager (keine Echtgeld-Käufe) ---- */

static double MaterialBuyPrice(int tier, int kind) {
    Item m = ItemMakeMaterial(tier, kind, 1);
    return ItemSellValue(&m) * 4.0 + 1;
}
static int ShopTierUnlocked(int tier) { return tier <= (player.level >= 60 ? TIER_BOSS : (player.level >= 40 ? TIER_UNIQUE : (player.level >= 20 ? TIER_RARE : (player.level >= 8 ? TIER_MAGIC : TIER_NORMAL)))); }

static void ShopRestock(void) {
    static const int base[TIER_COUNT] = { 40, 10, 5, 2, 0, 0 };
    static const int var[TIER_COUNT]  = { 40, 10, 5, 3, 2, 0 };
    for (int t = 0; t < TIER_COUNT; t++)
        for (int k = 0; k < MAT_COUNT; k++)
            player.shopStock[t][k] = base[t] + Rnd(var[t] > 0 ? var[t] : 1);
}

static int ShopBuyMaterial(int tier, int kind) {
    if (!ShopTierUnlocked(tier) || player.shopStock[tier][kind] <= 0) return 0;
    double price = MaterialBuyPrice(tier, kind);
    if (player.gold < price) return 0;
    Item m = ItemMakeMaterial(tier, kind, 1);
    if (!InvAdd(&m)) return 0;
    player.gold -= price; player.shopStock[tier][kind]--;
    return 1;
}
