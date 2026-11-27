/* UI-Grundbausteine: Text, Panels, Buttons, Item-Slots, Piktogramme.
 * Alle Koordinaten sind "logisch" (1920x1080); "scale" rechnet in Pixel um.
 * (wird von game.c eingebunden) */

#include "font_data.inc"

static double scale = 1.0;

/* Eingabe-Zustand des aktuellen Frames (logische Koordinaten) */
static int uiMX = -1, uiMY = -1;
static int uiClick = 0, uiRClick = 0, uiDown = 0, uiWheel = 0;
static int uiClickUsed = 0;            /* Klick wurde von einem Element verbraucht */

/* ---------------------------------------------------------------- */
/* Farben                                                           */
/* ---------------------------------------------------------------- */

#define COL_INK      0x2B2118   /* Schrift / Linien                   */
#define COL_PARCH    0xF3D9B1   /* Pergament (Panels)                 */
#define COL_PARCH_D  0xD9B27C
#define COL_PARCH_L  0xFFE9CC
#define COL_BG       0xFFC89A   /* wie assets/bg.png                  */
#define COL_GOOD     0x2E7D32
#define COL_BAD      0xB3261E
#define COL_GOLD     0xB8860B
#define COL_MUTED    0x7A6A55

static int Col(unsigned int rgb)            { return RGBA((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255, 255); }
static int ColA(unsigned int rgb, int a)    { return RGBA((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255, a); }

static unsigned int Mix(unsigned int a, unsigned int b, double t) {
    int r = (int)(((a >> 16) & 255) * (1 - t) + ((b >> 16) & 255) * t);
    int g = (int)(((a >> 8) & 255) * (1 - t) + ((b >> 8) & 255) * t);
    int bl = (int)((a & 255) * (1 - t) + (b & 255) * t);
    return ((unsigned)r << 16) | ((unsigned)g << 8) | (unsigned)bl;
}

/* ---------------------------------------------------------------- */
/* Primitive                                                        */
/* ---------------------------------------------------------------- */

static void UiFillC(double x, double y, double w, double h, int color) {
    int x0 = (int)(x * scale + 0.5), y0 = (int)(y * scale + 0.5);
    int x1 = (int)((x + w) * scale + 0.5), y1 = (int)((y + h) * scale + 0.5);
    FillRect(x0, y0, x1 - x0, y1 - y0, color);
}
static void UiFill(double x, double y, double w, double h, unsigned int rgb) { UiFillC(x, y, w, h, Col(rgb)); }
static void UiFillA(double x, double y, double w, double h, unsigned int rgb, int a) { UiFillC(x, y, w, h, ColA(rgb, a)); }

static void UiFrame(double x, double y, double w, double h, double t, unsigned int rgb) {
    UiFill(x, y, w, t, rgb);
    UiFill(x, y + h - t, w, t, rgb);
    UiFill(x, y + t, t, h - 2 * t, rgb);
    UiFill(x + w - t, y + t, t, h - 2 * t, rgb);
}

static int UiHover(double x, double y, double w, double h) {
    return uiMX >= x && uiMY >= y && uiMX < x + w && uiMY < y + h;
}

/* ---------------------------------------------------------------- */
/* Text                                                             */
/* ---------------------------------------------------------------- */

static int   UiFontImg = 0;
static short GlyphOf[256];

static int UiInitFont(void) {
    UiFontImg = LoadImage("assets/font/ui_font.png");
    for (int i = 0; i < 256; i++) GlyphOf[i] = -1;
    for (int i = 0; i < UIFONT_COUNT; i++)
        if (UiFontCodepoints[i] < 256) GlyphOf[UiFontCodepoints[i]] = (short)i;
    return UiFontImg != 0;
}

static int UiNextCodepoint(const char **p) {
    const unsigned char *s = (const unsigned char *)*p;
    int c = s[0];
    if (c < 0x80)               { *p += 1; return c; }
    if ((c & 0xE0) == 0xC0 && s[1]) { *p += 2; return ((c & 0x1F) << 6) | (s[1] & 0x3F); }
    if ((c & 0xF0) == 0xE0 && s[1] && s[2]) { *p += 3; return '?'; }
    *p += 1; return '?';
}

static double UiTextWidth(const char *s, double size) {
    double k = size / UIFONT_SIZE, w = 0;
    while (*s) {
        int cp = UiNextCodepoint(&s);
        int g = (cp < 256) ? GlyphOf[cp] : -1;
        if (g < 0) g = GlyphOf['?'];
        w += UiFontAdvance[g] * k;
    }
    return w;
}

static void UiText(double x, double y, double size, unsigned int rgb, const char *s) {
    if (!UiFontImg) return;
    double k = size / UIFONT_SIZE;
    int c = Col(rgb);
    double px = x;
    while (*s) {
        int cp = UiNextCodepoint(&s);
        int g = (cp < 256) ? GlyphOf[cp] : -1;
        if (g < 0) g = GlyphOf['?'];
        if (cp != ' ') {
            int sx = (g % UIFONT_COLS) * UIFONT_CELL_W, sy = (g / UIFONT_COLS) * UIFONT_CELL_H;
            int dx0 = (int)((px - 2 * k) * scale + 0.5), dy0 = (int)((y - 4 * k) * scale + 0.5);
            DrawImagePartTint(UiFontImg, sx, sy, UIFONT_CELL_W, UIFONT_CELL_H,
                              dx0, dy0, (int)(UIFONT_CELL_W * k * scale + 0.5),
                              (int)(UIFONT_CELL_H * k * scale + 0.5), c);
        }
        px += UiFontAdvance[g] * k;
    }
}

static void UiTextC(double cx, double y, double size, unsigned int rgb, const char *s) {
    UiText(cx - UiTextWidth(s, size) / 2, y, size, rgb, s);
}
static void UiTextR(double rx, double y, double size, unsigned int rgb, const char *s) {
    UiText(rx - UiTextWidth(s, size), y, size, rgb, s);
}

static char uiBuf[256];
#define UiTextf(x, y, size, rgb, ...) do { snprintf(uiBuf, sizeof uiBuf, __VA_ARGS__); UiText(x, y, size, rgb, uiBuf); } while (0)

/* ---------------------------------------------------------------- */
/* Widgets                                                          */
/* ---------------------------------------------------------------- */

static void UiPanel(double x, double y, double w, double h) {
    UiFill(x - 3, y - 3, w + 6, h + 6, COL_INK);
    UiFill(x, y, w, h, COL_PARCH);
}

static void UiPanelTitle(double x, double y, double w, const char *title) {
    UiFill(x, y, w, 40, COL_PARCH_D);
    UiFill(x, y + 40, w, 3, COL_INK);
    UiText(x + 14, y + 7, 28, COL_INK, title);
}

/* Button. Rückgabe 1 bei Klick. enabled=0: ausgegraut und nicht klickbar. */
static int UiButtonEx(double x, double y, double w, double h, const char *label,
                      int enabled, unsigned int fill) {
    int hover = enabled && UiHover(x, y, w, h);
    UiFill(x - 2, y - 2, w + 4, h + 4, COL_INK);
    unsigned int f = enabled ? (hover ? Mix(fill, 0xFFFFFF, 0.35) : fill) : 0xBDB3A3;
    UiFill(x, y, w, h, f);
    double size = h * 0.55; if (size > 30) size = 30;
    while (size > 10 && UiTextWidth(label, size) > w - 10) size -= 1;
    UiTextC(x + w / 2, y + h / 2 - size * 0.62, size, enabled ? COL_INK : COL_MUTED, label);
    if (hover && uiClick && !uiClickUsed) { uiClickUsed = 1; return 1; }
    return 0;
}
static int UiButton(double x, double y, double w, double h, const char *label) {
    return UiButtonEx(x, y, w, h, label, 1, COL_PARCH_D);
}

static void UiBar(double x, double y, double w, double h, double frac, unsigned int rgb) {
    if (frac < 0) frac = 0;
    if (frac > 1) frac = 1;
    UiFill(x - 2, y - 2, w + 4, h + 4, COL_INK);
    UiFill(x, y, w, h, 0x6B5E4D);
    UiFill(x, y, w * frac, h, rgb);
}

/* ---------------------------------------------------------------- */
/* Piktogramme (prozedural, im Stil der vorhandenen Grafik)         */
/* ---------------------------------------------------------------- */

enum {
    IC_SWORD, IC_AXE, IC_DAGGER, IC_BOW, IC_STAFF, IC_TOTEM, IC_MACE,
    IC_SHIELD, IC_ORB, IC_QUIVER, IC_THROWAXE, IC_THROWDAGGER, IC_RATTLE, IC_BOOK,
    IC_HELM, IC_ARMOR, IC_GLOVES, IC_BOOTS, IC_BAG, IC_RING, IC_NECK,
    IC_POT_RED, IC_POT_BLUE, IC_POT_GREEN, IC_POT_GRAY, IC_POT_YELLOW,
    IC_WINGS, IC_ORE, IC_WOOD, IC_LEATHER, IC_ESSENCE, IC_CHESTBOX, IC_COUNT
};

#define IC_SIZE 64
static int  UiIcon[IC_COUNT];
static int  icBuf[128 * 128];
static int  icN = IC_SIZE;     /* aktuelle Zeichenfläche (64 für Icons, 128 für Monster) */
static int  icPass;

static void IcPix(int x, int y, int c) {
    if (x >= 0 && y >= 0 && x < icN && y < icN) icBuf[y * icN + x] = c;
}
static int IcColorFor(unsigned int rgb) { return icPass == 0 ? Col(COL_INK) : Col(rgb); }
static int IcGrow(void) { return icPass == 0 ? (icN > 64 ? 3 : 2) : 0; }

static void IcRect(int x, int y, int w, int h, unsigned int rgb) {
    int g = IcGrow(), c = IcColorFor(rgb);
    for (int yy = y - g; yy < y + h + g; yy++)
        for (int xx = x - g; xx < x + w + g; xx++) IcPix(xx, yy, c);
}
static void IcCircle(int cx, int cy, int r, unsigned int rgb) {
    int g = IcGrow(), c = IcColorFor(rgb), rr = (r + g) * (r + g);
    for (int yy = cy - r - g; yy <= cy + r + g; yy++)
        for (int xx = cx - r - g; xx <= cx + r + g; xx++)
            if ((xx - cx) * (xx - cx) + (yy - cy) * (yy - cy) <= rr) IcPix(xx, yy, c);
}
static void IcLine(int x0, int y0, int x1, int y1, int r, unsigned int rgb) {
    int steps = abs(x1 - x0) > abs(y1 - y0) ? abs(x1 - x0) : abs(y1 - y0);
    if (steps == 0) steps = 1;
    for (int i = 0; i <= steps; i++)
        IcCircle(x0 + (x1 - x0) * i / steps, y0 + (y1 - y0) * i / steps, r, rgb);
}

#define STEEL 0xE6EAF2
#define WOODC 0x8A5A2B
#define GOLDC 0xE0B030

static void IcDraw(int id) {
    switch (id) {
    case IC_SWORD:  IcLine(32, 8, 32, 40, 3, STEEL); IcRect(21, 41, 22, 5, GOLDC); IcLine(32, 47, 32, 56, 2, WOODC); IcCircle(32, 58, 3, GOLDC); break;
    case IC_AXE:    IcLine(30, 58, 34, 8, 2, WOODC); IcCircle(42, 20, 11, STEEL); IcRect(31, 10, 7, 22, STEEL); break;
    case IC_DAGGER: IcLine(32, 10, 32, 36, 2, STEEL); IcRect(24, 37, 16, 4, GOLDC); IcLine(32, 42, 32, 54, 2, WOODC); break;
    case IC_BOW:    for (int i = 0; i < 24; i++) { int y = 8 + i * 2; int d = (y - 32) * (y - 32) / 22; IcCircle(46 - d, y, 2, WOODC); }
                    IcLine(18, 9, 18, 55, 1, 0xFFFFFF); IcLine(18, 32, 54, 32, 1, STEEL); break;
    case IC_STAFF:  IcLine(22, 58, 38, 16, 2, WOODC); IcCircle(40, 12, 8, 0x60D0F0); IcCircle(38, 10, 2, 0xFFFFFF); break;
    case IC_TOTEM:  IcRect(24, 8, 16, 48, WOODC); IcRect(27, 24, 10, 8, 0xF08030); IcCircle(28, 18, 2, 0xFFFFFF); IcCircle(36, 18, 2, 0xFFFFFF);
                    IcLine(16, 12, 24, 20, 1, 0xE04040); IcLine(48, 12, 40, 20, 1, 0x40B0E0); break;
    case IC_MACE:   IcLine(32, 58, 32, 26, 2, WOODC); IcCircle(32, 18, 11, STEEL); IcLine(32, 4, 32, 8, 2, STEEL); IcLine(19, 18, 15, 18, 2, STEEL); IcLine(45, 18, 49, 18, 2, STEEL); break;
    case IC_SHIELD: IcRect(16, 10, 32, 26, STEEL); IcCircle(32, 36, 16, STEEL); IcRect(30, 14, 4, 30, 0xC03030); IcRect(22, 22, 20, 4, 0xC03030); break;
    case IC_ORB:    IcCircle(32, 32, 19, 0x70B8F0); IcCircle(26, 25, 5, 0xFFFFFF); break;
    case IC_QUIVER: IcRect(24, 14, 16, 40, WOODC); IcLine(27, 5, 27, 16, 1, STEEL); IcLine(32, 3, 32, 16, 1, STEEL); IcLine(37, 5, 37, 16, 1, STEEL); IcRect(24, 28, 16, 4, GOLDC); break;
    case IC_THROWAXE: IcLine(18, 54, 42, 16, 2, WOODC); IcCircle(44, 17, 10, STEEL); break;
    case IC_THROWDAGGER: IcLine(16, 48, 46, 18, 2, STEEL); IcLine(12, 52, 20, 44, 2, WOODC); IcLine(22, 38, 28, 44, 2, GOLDC); break;
    case IC_RATTLE: IcLine(32, 30, 32, 56, 2, WOODC); IcCircle(32, 20, 13, 0xE8C090); IcLine(22, 10, 18, 4, 1, 0xE04040); IcLine(42, 10, 46, 4, 1, 0x40B0E0); break;
    case IC_BOOK:   IcRect(16, 10, 32, 42, 0xB02020); IcRect(20, 14, 26, 34, 0xF5E6C8); IcRect(30, 20, 4, 18, GOLDC); IcRect(25, 26, 14, 4, GOLDC); break;
    case IC_HELM:   IcCircle(32, 30, 19, STEEL); IcRect(13, 30, 38, 20, STEEL); IcRect(20, 32, 24, 5, 0x20202A); IcRect(30, 8, 4, 12, 0xC03030); break;
    case IC_ARMOR:  IcRect(18, 14, 28, 40, STEEL); IcCircle(15, 19, 8, STEEL); IcCircle(49, 19, 8, STEEL); IcRect(18, 40, 28, 5, GOLDC); IcRect(30, 14, 4, 26, 0xC8CCD6); break;
    case IC_GLOVES: IcRect(20, 30, 26, 22, 0xB07840); for (int i = 0; i < 4; i++) IcRect(20 + i * 6, 12 + (i == 1 || i == 2 ? 0 : 4), 5, 20, 0xB07840); IcRect(46, 34, 8, 6, 0xB07840); IcRect(18, 46, 30, 6, GOLDC); break;
    case IC_BOOTS:  IcRect(20, 8, 20, 32, 0xA06830); IcRect(20, 34, 34, 16, 0xA06830); IcRect(18, 48, 38, 5, 0x3A2A1A); IcRect(20, 12, 20, 4, GOLDC); break;
    case IC_BAG:    IcCircle(32, 40, 19, 0xA97C50); IcRect(26, 12, 12, 14, 0xA97C50); IcRect(24, 12, 16, 4, GOLDC); IcLine(20, 36, 44, 36, 1, GOLDC); break;
    case IC_RING:   for (int a = 0; a < 16; a++) { double t = a * 6.2831853 / 16; IcCircle(32 + (int)(14 * cos(t)), 36 + (int)(14 * sin(t)), 3, GOLDC); }
                    IcCircle(32, 20, 6, 0xE03050); break;
    case IC_NECK:   for (int i = 0; i <= 12; i++) { double t = i / 12.0; int x = 12 + (int)(40 * t); int y = 10 + (int)(36 * (1 - (2 * t - 1) * (2 * t - 1))); IcCircle(x, y, 3, GOLDC); }
                    IcCircle(32, 50, 8, 0x40A0F0); break;
    case IC_POT_RED: case IC_POT_BLUE: case IC_POT_GREEN: case IC_POT_GRAY: case IC_POT_YELLOW: {
        static const unsigned liq[5] = { 0xE03030, 0x3060E0, 0x30B040, 0x909098, 0xE8C020 };
        IcCircle(32, 42, 16, liq[id - IC_POT_RED]); IcRect(27, 12, 10, 20, 0xDDEEFF); IcRect(25, 6, 14, 7, WOODC); IcCircle(26, 38, 4, 0xFFFFFF); break; }
    case IC_WINGS:  IcCircle(22, 28, 12, 0xFFFFFF); IcCircle(14, 36, 9, 0xFFFFFF); IcCircle(12, 46, 7, 0xFFFFFF);
                    IcCircle(42, 28, 12, 0xFFFFFF); IcCircle(50, 36, 9, 0xFFFFFF); IcCircle(52, 46, 7, 0xFFFFFF); break;
    case IC_ORE:    IcCircle(24, 40, 10, 0x8A93A8); IcCircle(40, 38, 13, 0x6F7A92); IcCircle(32, 24, 9, 0xA6B0C8); break;
    case IC_WOOD:   IcRect(10, 18, 42, 13, WOODC); IcCircle(52, 24, 7, 0xC9A06A); IcRect(14, 36, 42, 13, 0x9C6A35); IcCircle(14, 42, 7, 0xC9A06A); break;
    case IC_LEATHER:IcRect(14, 14, 36, 36, 0xB98050); IcRect(14, 14, 36, 8, 0x9C6A35); for (int i = 0; i < 5; i++) IcCircle(18 + i * 7, 44, 1, 0xF5E6C8); break;
    case IC_ESSENCE:IcCircle(32, 40, 15, 0xB070F0); IcLine(32, 8, 20, 32, 3, 0xB070F0); IcLine(32, 8, 44, 32, 3, 0xB070F0); IcCircle(27, 36, 4, 0xFFFFFF); break;
    case IC_CHESTBOX:IcRect(8, 28, 48, 26, 0x8A5A2B); IcRect(8, 14, 48, 16, 0xA9743C); IcRect(28, 14, 8, 40, GOLDC); IcCircle(32, 32, 4, 0x3A2A1A); break;
    }
}

static void UiInitIcons(void) {
    icN = IC_SIZE;
    for (int id = 0; id < IC_COUNT; id++) {
        for (int i = 0; i < IC_SIZE * IC_SIZE; i++) icBuf[i] = 0;
        for (icPass = 0; icPass < 2; icPass++) IcDraw(id);
        UiIcon[id] = CreateImage(IC_SIZE, IC_SIZE);
        DrawOutput(UiIcon[id]);
        for (int y = 0; y < IC_SIZE; y++)
            for (int x = 0; x < IC_SIZE; x++)
                if (icBuf[y * IC_SIZE + x]) Plot(x, y, icBuf[y * IC_SIZE + x]);
    }
    DrawOutput(0);
}

static const int WEAPON_ICON[CLASS_COUNT]  = { IC_SWORD, IC_STAFF, IC_BOW, IC_AXE, IC_DAGGER, IC_TOTEM, IC_MACE };
static const int OFFHAND_ICON[CLASS_COUNT] = { IC_SHIELD, IC_ORB, IC_QUIVER, IC_THROWAXE, IC_THROWDAGGER, IC_RATTLE, IC_BOOK };
static const int MATERIAL_ICON[MAT_COUNT]  = { IC_ORE, IC_WOOD, IC_LEATHER, IC_ESSENCE };
static const int POTION_ICON[POT_COUNT]    = { IC_POT_RED, IC_POT_BLUE, IC_POT_GREEN, IC_POT_GRAY, IC_POT_YELLOW };

static int ItemIconId(const Item *it) {
    switch (it->cat) {
    case CAT_WEAPON:   return WEAPON_ICON[it->kind % CLASS_COUNT];
    case CAT_OFFHAND:  return OFFHAND_ICON[it->kind % CLASS_COUNT];
    case CAT_HELM:     return IC_HELM;
    case CAT_CHEST:    return IC_ARMOR;
    case CAT_GLOVES:   return IC_GLOVES;
    case CAT_BOOTS:    return IC_BOOTS;
    case CAT_BAG:      return IC_BAG;
    case CAT_NECKLACE: return IC_NECK;
    case CAT_RING:     return IC_RING;
    case CAT_POTION:   return POTION_ICON[it->kind % POT_COUNT];
    case CAT_WINGS:    return IC_WINGS;
    case CAT_MATERIAL: return MATERIAL_ICON[it->kind % MAT_COUNT];
    }
    return -1;
}

/* ---------------------------------------------------------------- */
/* Item-Slot + Tooltip                                              */
/* ---------------------------------------------------------------- */

static const Item *uiTipItem = NULL;

/* Zeichnet ein Item-Icon mit Tier-Hintergrund (ohne Interaktion). */
static void UiItemIcon(double x, double y, double size, const Item *it) {
    if (ItemIsEmpty(it)) {
        UiFill(x - 2, y - 2, size + 4, size + 4, 0x4A4034);
        UiFill(x, y, size, size, 0xA89880);
        return;
    }
    UiFill(x - 2, y - 2, size + 4, size + 4, TierColorDark(it->tier));
    UiFill(x, y, size, size, TierColor(it->tier));
    int ic = ItemIconId(it);
    if (ic >= 0) DrawImage(UiIcon[ic], (int)((x + size * 0.08) * scale), (int)((y + size * 0.08) * scale),
                           (int)(size * 0.84 * scale), (int)(size * 0.84 * scale));
    if (it->cat == CAT_POTION) {   /* Füllstand */
        UiFill(x + 3, y + size - 8, size - 6, 5, 0x302820);
        UiFill(x + 3, y + size - 8, (size - 6) * it->count / 100.0, 5, 0x40D060);
    }
    char b[16];
    if (it->cat == CAT_MATERIAL && it->count > 1) {
        snprintf(b, sizeof b, "%d", it->count);
        UiTextR(x + size - 3, y + size - size * 0.34, size * 0.30, 0x000000, b);
    } else if (it->level > 0 && it->cat != CAT_POTION) {
        snprintf(b, sizeof b, "+%d", it->level);
        UiTextR(x + size - 3, y + 2, size * 0.28, 0x000000, b);
    }
}

/* Slot mit Interaktion. Rückgabe: 1 = Linksklick, 2 = Rechtsklick. */
static int UiItemSlot(double x, double y, double size, const Item *it, int selected) {
    UiItemIcon(x, y, size, it);
    if (selected) UiFrame(x - 4, y - 4, size + 8, size + 8, 3, 0xFFFFFF);
    int hover = UiHover(x, y, size, size);
    if (hover) {
        UiFrame(x - 2, y - 2, size + 4, size + 4, 2, 0xFFFFFF);
        if (!ItemIsEmpty(it)) uiTipItem = it;
        if (uiClick && !uiClickUsed)  { uiClickUsed = 1; return 1; }
        if (uiRClick)                 { return 2; }
    }
    return 0;
}

static void UiDrawTooltip(void) {
    if (!uiTipItem || ItemIsEmpty(uiTipItem)) { uiTipItem = NULL; return; }
    char name[64], lines[14][72];
    ItemName(uiTipItem, name, sizeof name);
    int n = ItemDescribe(uiTipItem, lines, 14);
    double w = UiTextWidth(name, 26) + 24;
    for (int i = 0; i < n; i++) { double lw = UiTextWidth(lines[i], 20) + 24; if (lw > w) w = lw; }
    double h = 46 + n * 25 + 8;
    double x = uiMX + 24, y = uiMY + 16;
    if (x + w > 1910) x = uiMX - w - 12;
    if (y + h > 1070) y = 1070 - h;
    if (x < 4) x = 4;
    if (y < 4) y = 4;
    UiFill(x - 3, y - 3, w + 6, h + 6, COL_INK);
    UiFill(x, y, w, h, COL_PARCH_L);
    UiFill(x, y, w, 40, TierColor(uiTipItem->tier));
    UiText(x + 12, y + 6, 26, 0x000000, name);
    for (int i = 0; i < n; i++) {
        unsigned col = COL_INK;
        if (i == 0) col = COL_MUTED;
        UiText(x + 12, y + 46 + i * 25, 20, col, lines[i]);
    }
    uiTipItem = NULL;
}
