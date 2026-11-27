/* Karten-Kacheln: Grafik, Overlay, Kollision. (wird von game.c eingebunden)
 * Die Inhalte der Karten (Monster, Wegpunkte, Kapitel) stehen in data.c. */

#define MAX_TILES 32

struct MapTile {
    int exist;
    int loaded;
    char mapFilename[256];
    char overlayFilename[256];
    char collisionFilename[256];
    int map;
    int overlay;
    int collision;
    char *collisionShadow;
    int x;
    int y;
    int width;
    int height;
};

static struct MapTile MapTiles[MAX_TILES];
static int MapRenderDistance;

static void SetMapTile(int i, const char *map, const char *overlay, const char *collision, int x, int y, int w, int h) {
    MapTiles[i].exist = 1;
    strncpy(MapTiles[i].mapFilename, map, 255);
    strncpy(MapTiles[i].overlayFilename, overlay, 255);
    strncpy(MapTiles[i].collisionFilename, collision, 255);
    MapTiles[i].x = x; MapTiles[i].y = y; MapTiles[i].width = w; MapTiles[i].height = h;
}

static void InitMapTiles(void) {
    for (int i = 0; i < MAX_TILES; i++) {
        memset(&MapTiles[i], 0, sizeof MapTiles[i]);
        MapTiles[i].map = -1;
        MapTiles[i].overlay = -1;
        MapTiles[i].collision = -1;
    }
    /* Layouts 0-3 (Karten 1-1 .. 1-4) */
    SetMapTile(0, "assets/map/dev/bigmap.png",  "assets/map/dev/bigmapTop.png",  "assets/map/dev/bigmapAlpha.png",  0, 0, 8192, 8192);
    /* Layout 4 (Karte 2-1) */
    SetMapTile(1, "assets/map/dev/bigmap2.png", "assets/map/dev/bigmap2Top.png", "assets/map/dev/bigmap2Alpha.png", 10000, 10000, 8192, 8192);
}

static void InitMapCollision(int tile) {
    if (MapTiles[tile].loaded == 0) return;
    int w = 0, h = 0;
    unsigned char *green = GetImageGreenChannel(MapTiles[tile].collision, &w, &h);
    if (green == NULL) return;
    MapTiles[tile].width = w;
    MapTiles[tile].height = h;
    MapTiles[tile].collisionShadow = (char *)malloc((size_t)w * h);
    if (MapTiles[tile].collisionShadow != NULL) {
        for (int i = 0; i < w * h; i++)
            MapTiles[tile].collisionShadow[i] = (green[i] > 127) ? 0 : 1;
    }
    free(green);
}

static void LoadMapTile(int tile) {
    MapTiles[tile].loaded = 1;
    MapTiles[tile].map = LoadImage(MapTiles[tile].mapFilename);
    MapTiles[tile].overlay = LoadImage(MapTiles[tile].overlayFilename);
    MapTiles[tile].collision = LoadImage(MapTiles[tile].collisionFilename);
    if (MapTiles[tile].collision != 0) InitMapCollision(tile);
}

static int MapTileNeeded(int i, double xPos, double yPos, int dist) {
    if (MapTiles[i].exist == 0) return 0;
    if (xPos < MapTiles[i].x - dist) return 0;
    if (xPos > MapTiles[i].x + MapTiles[i].width + dist) return 0;
    if (yPos < MapTiles[i].y - dist) return 0;
    if (yPos > MapTiles[i].y + MapTiles[i].height + dist) return 0;
    return 1;
}

static void DrawImageScaled(int ImageID, double x, double y, double w, double h) {
    DrawImage(ImageID, (int)(x * scale), (int)(y * scale), (int)(w * scale + 0.999), (int)(h * scale + 0.999));
}

static void DrawMapLayer(double xPos, double yPos, int overlay) {
    for (int i = 0; i < MAX_TILES; i++) {
        if (!MapTileNeeded(i, xPos, yPos, MapRenderDistance)) continue;
        if (MapTiles[i].loaded == 0) LoadMapTile(i);
        int img = overlay ? MapTiles[i].overlay : MapTiles[i].map;
        if (img > -1)
            DrawImageScaled(img, MapTiles[i].x - xPos + 960, MapTiles[i].y - yPos + 540, MapTiles[i].width, MapTiles[i].height);
    }
}
static void DrawMap(double xPos, double yPos)     { DrawMapLayer(xPos, yPos, 0); }
static void DrawOverlay(double xPos, double yPos) { DrawMapLayer(xPos, yPos, 1); }

/* 1 = Wand / blockiert (auch außerhalb aller Karten), 0 = frei */
static int getMapCollision(int xPos, int yPos) {
    for (int i = 0; i < MAX_TILES; i++) {
        if (MapTiles[i].exist == 0) continue;
        if (xPos < MapTiles[i].x || yPos < MapTiles[i].y) continue;
        if (xPos >= MapTiles[i].x + MapTiles[i].width || yPos >= MapTiles[i].y + MapTiles[i].height) continue;
        if (MapTiles[i].loaded == 0) LoadMapTile(i);
        if (MapTiles[i].collisionShadow != NULL)
            return MapTiles[i].collisionShadow[(xPos - MapTiles[i].x) + (yPos - MapTiles[i].y) * MapTiles[i].width];
        return 0;
    }
    return 1;
}

/* prüft die Ecken und die Mitte statt jedes Pixels (viel schneller als die Vollprüfung) */
static int getMapCollisionBoxCenter(double x, double y, int r) {
    int xi = (int)x, yi = (int)y;
    if (getMapCollision(xi, yi)) return 1;
    if (r <= 0) return 0;
    int d = r * 7 / 10;
    return getMapCollision(xi - r, yi) | getMapCollision(xi + r, yi) | getMapCollision(xi, yi - r) | getMapCollision(xi, yi + r) |
           getMapCollision(xi - d, yi - d) | getMapCollision(xi + d, yi - d) | getMapCollision(xi - d, yi + d) | getMapCollision(xi + d, yi + d);
}
