/* Darstellung: Texturen, Monster-Sprites, Heldenansicht, HUD.
 * (wird von game.c eingebunden) */

static int Tex_head, Tex_handL, Tex_handR, Tex_wingL, Tex_wingR;
static int Tex_arrow, Tex_arcArrow, Tex_target;
static int Tex_autopilotOn, Tex_autopilotOff, Tex_tiled, Tex_fullscreen;
static int Tex_won, Tex_lost;
static int Tex_skillBG, Tex_skillKey[4], Tex_skillActive, Tex_skill5, Tex_skillCircle, Tex_skillSpray, Tex_skillShotgun;
static int Tex_monster[MONSTER_TYPES];
static int Tex_glow;
static int CanvasBG, HeroBG, VoidBG;

/* Klassenfarben (Ring unter dem Helden, Namen in Listen) */
static const unsigned int CLASS_COLORS[CLASS_COUNT] = {
    0x9AA7B8, 0x5A8CFF, 0x57C257, 0xE0793A, 0x8E5BD0, 0x33B5A5, 0xF2D04D
};

/* ================================================================== */
/* Prozedurale Texturen                                               */
/* ================================================================== */

static int MakeGlowTexture(void) {
    int img = CreateImage(128, 128);
    DrawOutput(img);
    for (int y = 0; y < 128; y++)
        for (int x = 0; x < 128; x++) {
            double d = Dist(x, y, 63.5, 63.5) / 64.0;
            if (d >= 1.0) continue;
            double a = 1.0 - d; a = a * a * 255.0 * 1.2; if (a > 255) a = 255;
            Plot(x, y, RGBA(255, 255, 255, (int)a));
        }
    DrawOutput(0);
    return img;
}

static void MonsterDraw(int type) {
    switch (type) {
    case MONSTER_SLIME:
        IcCircle(64, 74, 46, 0x6FD36F); IcCircle(64, 62, 42, 0x6FD36F); IcCircle(46, 48, 9, 0xD8FFD8);
        IcCircle(48, 64, 8, 0xFFFFFF); IcCircle(80, 64, 8, 0xFFFFFF);
        if (icPass == 1) { IcCircle(50, 66, 4, 0x202020); IcCircle(82, 66, 4, 0x202020); IcRect(52, 86, 24, 4, 0x2E7A2E); }
        break;
    case MONSTER_BAT:
        IcCircle(24, 62, 22, 0x5A3A80); IcCircle(104, 62, 22, 0x5A3A80); IcCircle(12, 74, 12, 0x5A3A80); IcCircle(116, 74, 12, 0x5A3A80);
        IcCircle(64, 66, 30, 0x7A4BA8); IcCircle(50, 38, 9, 0x7A4BA8); IcCircle(78, 38, 9, 0x7A4BA8);
        IcCircle(54, 62, 6, 0xFF3030); IcCircle(74, 62, 6, 0xFF3030);
        IcRect(55, 80, 5, 10, 0xFFFFFF); IcRect(69, 80, 5, 10, 0xFFFFFF);
        break;
    case MONSTER_BRUTE:
        IcCircle(12, 86, 17, 0x8C7258); IcCircle(116, 86, 17, 0x8C7258);
        IcCircle(64, 64, 54, 0xA38C74); IcRect(34, 40, 60, 11, 0x5A4632);
        IcCircle(48, 62, 6, 0xFFFFFF); IcCircle(80, 62, 6, 0xFFFFFF);
        if (icPass == 1) { IcCircle(48, 63, 3, 0xC02020); IcCircle(80, 63, 3, 0xC02020); }
        IcRect(40, 88, 9, 18, 0xFFF3D8); IcRect(79, 88, 9, 18, 0xFFF3D8);
        break;
    case MONSTER_SPITTER:
        IcLine(40, 32, 30, 10, 3, 0xA6B04A); IcLine(88, 32, 98, 10, 3, 0xA6B04A);
        IcCircle(30, 10, 9, 0xFFFFFF); IcCircle(98, 10, 9, 0xFFFFFF);
        IcCircle(64, 68, 50, 0xA6B04A); IcCircle(64, 80, 17, 0x301810); IcCircle(64, 86, 8, 0xE04060);
        if (icPass == 1) { IcCircle(30, 11, 4, 0x101010); IcCircle(98, 11, 4, 0x101010); }
        IcCircle(46, 56, 6, 0xE8F0A0); IcCircle(82, 54, 5, 0xE8F0A0);
        break;
    }
}

static void InitMonsterTextures(void) {
    Tex_monster[MONSTER_NULL] = 0;
    Tex_monster[MONSTER_HUMAN] = LoadImage("assets/monsters/human.png");
    icN = 128;
    for (int t = MONSTER_SLIME; t < MONSTER_TYPES; t++) {
        for (int i = 0; i < 128 * 128; i++) icBuf[i] = 0;
        for (icPass = 0; icPass < 2; icPass++) MonsterDraw(t);
        Tex_monster[t] = CreateImage(128, 128);
        DrawOutput(Tex_monster[t]);
        for (int y = 0; y < 128; y++) for (int x = 0; x < 128; x++) if (icBuf[y * 128 + x]) Plot(x, y, icBuf[y * 128 + x]);
    }
    icN = IC_SIZE;
    DrawOutput(0);
}

#define LOAD(var, path) do { if (((var) = LoadImage(path)) == 0) { fprintf(stderr, "Textur fehlt: %s\n", path); return 0; } } while (0)

static int LoadTextures(void) {
    LOAD(Tex_head, "assets/head.png");
    LOAD(Tex_handL, "assets/handL.png");
    LOAD(Tex_handR, "assets/handR.png");
    LOAD(Tex_wingL, "assets/LeftWing1.png");
    LOAD(Tex_wingR, "assets/RightWing1.png");
    LOAD(Tex_arrow, "assets/arrow.png");
    LOAD(Tex_arcArrow, "assets/arc_arrow.png");
    LOAD(Tex_target, "assets/target.png");
    LOAD(Tex_autopilotOn, "assets/autopilot_on.png");
    LOAD(Tex_autopilotOff, "assets/autopilot_off.png");
    LOAD(Tex_tiled, "assets/tiled.png");
    LOAD(Tex_fullscreen, "assets/fullscreen.png");
    LOAD(Tex_won, "assets/won.png");
    LOAD(Tex_lost, "assets/lost.png");
    LOAD(VoidBG, "assets/canvas.png");
    LOAD(CanvasBG, "assets/bg.png");
    LOAD(HeroBG, "assets/bgHero.png");
    LOAD(Tex_skillBG, "assets/skill_BG.png");
    LOAD(Tex_skillKey[0], "assets/skill_Q.png");
    LOAD(Tex_skillKey[1], "assets/skill_W.png");
    LOAD(Tex_skillKey[2], "assets/skill_E.png");
    LOAD(Tex_skillKey[3], "assets/skill_R.png");
    LOAD(Tex_skillActive, "assets/skill_active.png");
    LOAD(Tex_skill5, "assets/skill_5.png");
    LOAD(Tex_skillCircle, "assets/skill_circle.png");
    LOAD(Tex_skillSpray, "assets/skill_spray.png");
    LOAD(Tex_skillShotgun, "assets/skill_shotgun.png");
    if (!UiInitFont()) { fprintf(stderr, "Textur fehlt: assets/font/ui_font.png\n"); return 0; }
    UiInitIcons();
    InitMonsterTextures();
    Tex_glow = MakeGlowTexture();
    return 1;
}

/* ================================================================== */
/* Zeichen-Helfer                                                     */
/* ================================================================== */

static void DrawSprite(int img, double x, double y, double angle, double s) {
    DrawImageRotated(img, (int)(x * scale), (int)(y * scale), angle, s * scale);
}
static void DrawSpriteTint(int img, double x, double y, double angle, double s, unsigned int rgb, int alpha) {
    DrawImageRotatedTint(img, (int)(x * scale), (int)(y * scale), angle, s * scale, RGBA((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255, alpha));
}
static void DrawGlow(double x, double y, double radius, unsigned int rgb, int alpha) {
    DrawImageTint(Tex_glow, (int)((x - radius) * scale), (int)((y - radius) * scale),
                  (int)(radius * 2 * scale), (int)(radius * 2 * scale),
                  RGBA((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255, alpha));
}
static void DrawRing(double x, double y, double r, unsigned int rgb, int alpha) {
    int c = RGBA((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255, alpha);
    int segs = 40;
    double px = x + r, py = y;
    for (int i = 1; i <= segs; i++) {
        double a = i * 2 * M_PIMALDAUMEN / segs;
        double nx = x + cos(a) * r, ny = y + sin(a) * r;
        DrawLine((int)(px * scale), (int)(py * scale), (int)(nx * scale), (int)(ny * scale), c);
        px = nx; py = ny;
    }
}
static void DrawWorldLine(double x1, double y1, double x2, double y2, unsigned int rgb, int alpha) {
    DrawLine((int)(x1 * scale), (int)(y1 * scale), (int)(x2 * scale), (int)(y2 * scale),
             RGBA((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255, alpha));
}

/* ================================================================== */
/* Welt                                                               */
/* ================================================================== */

static void DrawMonster(int in, int mi, double cx, double cy) {
    Monster *m = &instances[in].mon[mi];
    if (!m->alive) return;
    double sx = 960 + m->x - cx, sy = 540 + m->y - cy;
    if (sx < -200 || sx > 2120 || sy < -200 || sy > 1280) return;
    double r = m->radius;
    if (m->tier >= TIER_MAGIC) {              /* Schimmer in der Tier-Farbe (Boss rot, legendär rosa) */
        double pulse = 0.85 + 0.15 * sin(engineStep * 0.12 + mi);
        int alpha = m->tier >= TIER_BOSS ? 230 : (m->tier == TIER_UNIQUE ? 170 : 120);
        DrawGlow(sx, sy, r * (m->tier >= TIER_BOSS ? 2.3 : 1.9) * pulse, TierColor(m->tier), alpha);
    }
    int tex = Tex_monster[m->type];
    if (tex) {
        if (m->hitFlash > 0) DrawSpriteTint(tex, sx, sy, 0, r / 64.0, 0xFF8080, 255);
        else DrawSprite(tex, sx, sy, 0, r / 64.0);
    }
    /* Lebensbalken */
    double frac = m->hp / m->hpMax;
    if (m->role == ROLE_BOSS) {
        UiBar(sx - 150, sy - r - 40, 300, 22, frac, 0xC03030);
    } else {
        double w = r * 2 + 14 > 60 ? r * 2 + 14 : 60;
        UiBar(sx - w / 2, sy - r - 18, w, 8, frac, m->tier >= TIER_MAGIC ? TierColor(m->tier) : 0xC03030);
    }
}

static void DrawProjectiles(int in, double cx, double cy) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        Projectile *p = &projectiles[i];
        if (!p->valid || p->inst != in) continue;
        double sx = 960 + p->x - cx, sy = 540 + p->y - cy;
        if (sx < -50 || sx > 1970 || sy < -50 || sy > 1130) continue;
        switch (p->visual) {
        case PROJ_ARROW:  DrawSprite(Tex_arrow, sx, sy, p->angle, 0.5); break;
        case PROJ_ARCANE: DrawSprite(Tex_arcArrow, sx, sy, p->angle, 0.5); break;
        case PROJ_DAGGER: DrawSpriteTint(Tex_arrow, sx, sy, p->angle, 0.5, 0x606070, 255); break;
        case PROJ_HOLY:   DrawGlow(sx, sy, 26, 0xFFF0A0, 230); DrawSpriteTint(Tex_arcArrow, sx, sy, p->angle, 0.5, 0xFFE060, 255); break;
        case PROJ_VENOM:  DrawSpriteTint(Tex_arrow, sx, sy, p->angle, 0.5, 0x60E040, 255); break;
        default:          DrawGlow(sx, sy, 22, 0xFF3030, 230); break;     /* PROJ_ENEMY */
        }
    }
}

static void DrawEffects(int in, double cx, double cy) {
    for (int i = 0; i < MAX_FX; i++) {
        Effect *e = &fxs[i];
        if (!e->used || e->inst != in) continue;
        double f = (double)e->life / e->maxLife;
        int alpha = (int)(255 * f);
        double sx = 960 + e->x - cx, sy = 540 + e->y - cy;
        switch (e->kind) {
        case FX_BOLT: {
            double x2 = 960 + e->x2 - cx, y2 = 540 + e->y2 - cy;
            double mx = (sx + x2) / 2 + Rnd(20) - 10, my = (sy + y2) / 2 + Rnd(20) - 10;
            DrawWorldLine(sx, sy, mx, my, e->color, alpha); DrawWorldLine(mx, my, x2, y2, e->color, alpha);
            DrawWorldLine(sx + 1, sy, mx + 1, my, 0xFFFFFF, alpha); DrawWorldLine(mx + 1, my, x2 + 1, y2, 0xFFFFFF, alpha);
            break; }
        case FX_RING:  DrawRing(sx, sy, e->r * (1.0 - 0.3 * f), e->color, alpha); DrawRing(sx, sy, e->r * (0.8 - 0.3 * f), e->color, alpha / 2); break;
        case FX_FLASH: DrawGlow(sx, sy, e->r * (1.3 - 0.3 * f), e->color, alpha); break;
        case FX_CONE: {
            double a0 = e->x2 - e->y2 / 2, a1 = e->x2 + e->y2 / 2;     /* x2 = Blickwinkel, y2 = Öffnungswinkel */
            double px = 0, py = 0;
            for (int k = 0; k <= 12; k++) {
                double a = a0 + (a1 - a0) * k / 12, dx, dy; Dir(a, &dx, &dy);
                double nx = sx + dx * e->r, ny = sy + dy * e->r;
                if (k > 0) DrawWorldLine(px, py, nx, ny, e->color, alpha);
                if (k == 0 || k == 12) DrawWorldLine(sx, sy, nx, ny, e->color, alpha);
                px = nx; py = ny;
            }
            break; }
        }
    }
}

static void DrawFloatTexts(int in, double cx, double cy) {
    for (int i = 0; i < MAX_FLOATTEXT; i++) {
        FloatText *f = &floatTexts[i];
        if (f->life <= 0 || f->inst != in) continue;
        char b[24]; FormatNumber(f->value, b, sizeof b);
        double sx = 960 + f->x - cx, sy = 540 + f->y - cy;
        double size = f->kind == 2 ? 36 : 26;
        UiTextC(sx + 1, sy + 1, size, 0x000000, b);
        UiTextC(sx, sy, size, (unsigned)f->color, b);
    }
}

static const char *HeroWingsTier(const Hero *H) { return H->equip[EQ_WINGS].cat == CAT_WINGS ? TierName(H->equip[EQ_WINGS].tier) : NULL; }

static void DrawHeroFigure(int h) {
    Hero *H = &heroes[h];
    int dead = H->hp <= 0;
    int alpha = dead ? 110 : 255;
    double x0 = 960, y0 = 540, ang = H->angle;
    /* Ring in Klassenfarbe */
    DrawGlow(x0, y0, 70, CLASS_COLORS[H->class % CLASS_COUNT], dead ? 60 : 140);
    /* Flügel nur mit ausgerüsteten Flügeln, getönt in der Tier-Farbe */
    if (HeroWingsTier(H)) {
        unsigned int wc = TierColor(H->equip[EQ_WINGS].tier);
        for (int side = 0; side < 2; side++) {
            double x = side ? 20 : -20, y = 40;
            double sw = H->walking ? sin(H->walkCycle / 10) * 10 : 0;
            double a = ang + (side ? sw : -sw);
            rotate_point(&x, &y, ang);
            DrawSpriteTint(side ? Tex_wingR : Tex_wingL, x0 + x, y0 + y, a, 0.5, wc, alpha);
        }
    }
    DrawSpriteTint(Tex_head, x0, y0, ang, 0.5, 0xFFFFFF, alpha);
    /* Hände + Waffe */
    double sw = H->walking ? sin(H->walkCycle / 10) * 8 : 0;
    double aSw = H->walking ? -sin(H->walkCycle / 10) * 10 : 0;
    double lx = -40 + sw, ly = -sw * (H->walking ? 1 : 0);
    double rx = 40 + sw, ry = sw * (H->walking ? 1 : 0);
    if (H->attacking && H->attackTotal > 0) {                    /* Schlag-/Stoß-Animation */
        double t = (double)H->attackCycle / H->attackTotal;
        ry -= sin(t * M_PIMALDAUMEN) * 26;
    }
    rotate_point(&lx, &ly, ang); rotate_point(&rx, &ry, ang);
    DrawSpriteTint(Tex_handL, x0 + lx, y0 + ly, ang + aSw, 0.5, 0xFFFFFF, alpha);
    int wic = WEAPON_ICON[H->class % CLASS_COUNT];
    DrawImageRotated(UiIcon[wic], (int)((x0 + rx) * scale), (int)((y0 + ry) * scale), ang + aSw + (H->attacking ? sin((double)H->attackCycle / (H->attackTotal ? H->attackTotal : 1) * M_PIMALDAUMEN) * 50 : 0), 2.2 * scale);
    DrawSpriteTint(Tex_handR, x0 + rx, y0 + ry, ang + aSw, 0.5, 0xFFFFFF, alpha);
    if (H->equip[EQ_OFFHAND].cat == CAT_OFFHAND) {
        double ox = -48, oy = -8; rotate_point(&ox, &oy, ang);
        DrawImageRotated(UiIcon[OFFHAND_ICON[H->class % CLASS_COUNT]], (int)((x0 + ox + lx * 0.2) * scale), (int)((y0 + oy + ly * 0.2) * scale), ang, 1.1 * scale);
    }
    /* Buff-Anzeige */
    if (H->buffF[BUFF_ARMOR] > 0) DrawRing(x0, y0, 52, 0x80C0FF, 200);
    if (H->buffF[BUFF_ATTACK] > 0) DrawRing(x0, y0, 58, 0xFF6040, 200);
    if (H->buffF[BUFF_ATKSPEED] > 0) DrawRing(x0, y0, 64, 0xFFE040, 200);
    if (H->buffF[BUFF_SPEED] > 0) DrawRing(x0, y0, 70, 0x60E060, 200);
}

/* ================================================================== */
/* HUD                                                                */
/* ================================================================== */

static const int KEY_LETTER[4] = { 'Q', 'W', 'E', 'R' };

static int SkillTexture(const SkillDef *sk) {
    if (sk->type == SKT_FAN && sk->n == 5) return Tex_skill5;
    if (sk->type == SKT_CIRCLE) return Tex_skillCircle;
    if (sk->type == SKT_CHANNEL) return Tex_skillSpray;
    if (sk->type == SKT_FAN && sk->n >= 40) return Tex_skillShotgun;
    return 0;
}

/* HUD-Layout. u = Vergrößerung (1 = Vollbild, 2 = Kachelansicht) */
typedef struct { double size, gap, skillX, skillY, potX, potY, stripY, stripH; } HudLayout;
static HudLayout HudGet(double u) {
    HudLayout L;
    L.size = 80 * u; L.gap = 14 * u;
    L.stripH = 44 * u; L.stripY = 1080 - L.stripH - 4;
    L.skillY = L.stripY - L.size - 8 * u;
    L.potY = L.skillY;
    double potW = 3 * L.size + 2 * L.gap, skillW = 4 * L.size + 3 * L.gap;
    L.potX = 1900 - potW;
    L.skillX = 960 - skillW / 2;
    if (L.skillX + skillW > L.potX - 20) L.skillX = L.potX - 20 - skillW;
    return L;
}

static void DrawSkillSlot(int h, int slot, double x, double y, double size, double u) {
    Hero *H = &heroes[h];
    int id = H->skillSlots[slot];
    DrawImageScaled(Tex_skillBG, x, y, size, size);
    if (id >= 0) {
        const SkillDef *sk = &SkillDefs[id];
        int tex = SkillTexture(sk);
        if (tex) DrawImageScaled(tex, x, y, size, size);
        else {
            char buf[32]; snprintf(buf, sizeof buf, "%s", sk->name);
            char *sp = strchr(buf, ' ');
            if (sp) { *sp = 0; UiTextC(x + size / 2, y + size * 0.22, 15 * u, COL_INK, buf); UiTextC(x + size / 2, y + size * 0.46, 15 * u, COL_INK, sk->name + (sp - buf) + 1); }
            else UiTextC(x + size / 2, y + size * 0.32, 15 * u, COL_INK, buf);
        }
        if (H->skillCd[slot] > 0) {
            double f = (double)H->skillCd[slot] / (sk->cd > 0 ? sk->cd : 1); if (f > 1) f = 1;
            UiFillA(x, y, size, size * f, 0x000000, 150);
        } else if (H->mana < SkillManaCost(h, sk)) UiFillA(x, y, size, size, 0x2040A0, 110);
        if (H->skilling && H->skillActive == id) DrawImageScaled(Tex_skillActive, x, y, size, size);
        char b[16]; snprintf(b, sizeof b, "%d", SkillManaCost(h, sk));
        UiTextR(x + size - 4 * u, y + size - 22 * u, 14 * u, 0x1030A0, b);
    }
    DrawImageScaled(Tex_skillKey[slot], x, y, size, size);
}

static void DrawHud(int h, double u) {
    Hero *H = &heroes[h];
    char a[32], b[32], line[80];
    /* links oben: Name, Level, Leben, Mana, Erfahrung */
    UiFillA(14, 14, 372 * u * 0.9 + 20, 150 * u * 0.9 + 6, 0xF3D9B1, 215);
    UiFrame(14, 14, 372 * u * 0.9 + 20, 150 * u * 0.9 + 6, 3, COL_INK);
    snprintf(line, sizeof line, "%s   Lv %d", H->name, H->level);
    UiText(26, 20, 28 * u * 0.9, CLASS_COLORS[H->class] == 0 ? COL_INK : COL_INK, line);
    double bw = 340 * u * 0.9;
    UiBar(28, 20 + 38 * u * 0.9, bw, 30 * u * 0.9, H->hp / H->hpMax, 0xD04040);
    FormatNumber(H->hp, a, sizeof a); FormatNumber(H->hpMax, b, sizeof b); snprintf(line, sizeof line, "%s / %s", a, b);
    UiTextC(28 + bw / 2, 20 + 38 * u * 0.9 + 1, 22 * u * 0.9, 0xFFFFFF, line);
    UiBar(28, 20 + 78 * u * 0.9, bw, 26 * u * 0.9, H->mana / H->manaMax, 0x3A6FE0);
    FormatNumber(H->mana, a, sizeof a); FormatNumber(H->manaMax, b, sizeof b); snprintf(line, sizeof line, "%s / %s", a, b);
    UiTextC(28 + bw / 2, 20 + 78 * u * 0.9, 20 * u * 0.9, 0xFFFFFF, line);
    double need = XpNeeded(H->level);
    UiBar(28, 20 + 114 * u * 0.9, bw, 12 * u * 0.9, H->level >= HERO_MAX_LEVEL ? 1.0 : H->xp / need, 0xD9A400);
    Instance *I = H->inst >= 0 ? &instances[H->inst] : NULL;
    if (I) UiText(26, 20 + 130 * u * 0.9, 20 * u * 0.9, COL_INK, Stages[I->stage].name);

    /* rechts oben: Autopilot-Anzeige (+ Kachel-Symbol im Vollbild) */
    double sz = 80 * (u > 1 ? 1.4 : 1);
    DrawImageScaled(H->autopilotEnabled ? Tex_autopilotOn : Tex_autopilotOff, 1900 - sz, 20, sz, sz);
    if (u <= 1) {
        DrawImageScaled(Tex_tiled, 1900 - 2 * sz - 20, 20, sz, sz);
        UiButtonEx(1900 - 3 * sz - 40, 20, sz, sz, "Menü", 1, COL_PARCH_D);
    }

    /* unten: Skills, Tränke, Kisten */
    HudLayout L = HudGet(u);
    for (int s = 0; s < 4; s++) DrawSkillSlot(h, s, L.skillX + s * (L.size + L.gap), L.skillY, L.size, u);
    for (int s = 0; s < 3; s++) {
        double x = L.potX + s * (L.size + L.gap);
        Item *p = &H->equip[EQ_POT1 + s];
        UiItemIcon(x, L.skillY, L.size, p);
        char k[4]; snprintf(k, sizeof k, "%d", s + 1);
        UiText(x + 4 * u, L.skillY + 2 * u, 22 * u, 0x000000, k);
    }
    /* Kisten-Leiste */
    UiFillA(0, L.stripY - 2, 1920, L.stripH + 6, 0x2B2118, 200);
    double bx = 20;
    for (int t = 0; t < TIER_COUNT; t++) {
        double bsz = L.stripH - 6;
        UiFill(bx - 2, L.stripY + 1, bsz + 4, bsz + 4, TierColorDark(t));
        UiFill(bx, L.stripY + 3, bsz, bsz, (H->chestMask >> t) & 1 ? TierColor(t) : 0x6E675F);
        DrawImageScaled(UiIcon[IC_CHESTBOX], bx + bsz * 0.08, L.stripY + 3 + bsz * 0.08, bsz * 0.84, bsz * 0.84);
        snprintf(line, sizeof line, "%d", H->chests[t]);
        UiTextR(bx + bsz - 2, L.stripY + 3 + bsz - 20 * u, 20 * u, 0x000000, line);
        bx += bsz + 14 * u;
    }
    snprintf(line, sizeof line, "Kisten %d / %d", HeroChestTotal(h), H->capacity);
    UiText(bx + 10, L.stripY + 6 * u, 26 * u, HeroChestTotal(h) >= H->capacity ? 0xFF7070 : 0xFFE9CC, line);
    if (u <= 1) UiTextR(1900, L.stripY + 8, 22, 0xFFE9CC, "Klick = einsammeln (F) - Stufe anklicken = sammeln an/aus");
}

/* ================================================================== */
/* Heldenansicht                                                      */
/* ================================================================== */

static void DrawHeroView(int canvas, int h, double u) {
    Hero *H = &heroes[h];
    DrawOutput(canvas);
    if (H->stage < 0 || H->inst < 0) {          /* im Lager */
        DrawImageScaled(HeroBG, 0, 0, 1920, 1080);
        UiTextC(960, 440, 52 * (u > 1 ? 1.2 : 1), 0xFFE9CC, H->name);
        UiTextC(960, 520, 34 * (u > 1 ? 1.2 : 1), 0xFFE9CC, "Dieser Held ist im Lager.");
        UiTextC(960, 570, 30 * (u > 1 ? 1.2 : 1), 0xFFC89A, "Menü > Karten: Karte für ihn wählen");
        return;
    }
    int in = H->inst;
    DrawMap(H->x, H->y);
    if (H->walking && (!H->autopilotEnabled || H->manualTimer > 0))
        DrawSprite(Tex_target, 960 + H->xTarget - H->x, 540 + H->yTarget - H->y, 0, 1.0);
    DrawHeroFigure(h);
    if (instances[in].nHeroes > 1) {              /* Partner zeichnen (nur Figur als Ring + Name) */
        int p = H->partner;
        if (p >= 0) {
            double px = 960 + heroes[p].x - H->x, py = 540 + heroes[p].y - H->y;
            DrawGlow(px, py, 60, CLASS_COLORS[heroes[p].class % CLASS_COUNT], 150);
            DrawSprite(Tex_head, px, py, heroes[p].angle, 0.5);
            UiTextC(px, py - 70, 22, 0xFFFFFF, heroes[p].name);
            UiBar(px - 40, py + 48, 80, 8, heroes[p].hp / heroes[p].hpMax, 0xD04040);
        }
    }
    for (int i = 0; i < MAX_MONSTERS; i++) DrawMonster(in, i, H->x, H->y);
    DrawProjectiles(in, H->x, H->y);
    DrawEffects(in, H->x, H->y);
    DrawOverlay(H->x, H->y);
    DrawFloatTexts(in, H->x, H->y);
    /* kleine Balken über dem Helden */
    UiBar(900, 446, 120, 10, H->hp / H->hpMax, 0xD04040);
    UiBar(900, 460, 120, 6, H->mana / H->manaMax, 0x3A6FE0);
    DrawHud(h, u);
    if (instances[in].state == RUN_CLEARED) DrawImageScaled(Tex_won, 0, 0, 1920, 1080);
    if (instances[in].state == RUN_LOST)    DrawImageScaled(Tex_lost, 0, 0, 1920, 1080);
}
