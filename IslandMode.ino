#include "Island.h"

static const int ISLAND_SCR_W = 64;
static const int ISLAND_SCR_H = 64;

static const int ISLAND_WORLD_W = 720;
static const int ISLAND_WORLD_H = 377;

static const uint16_t ISLAND_BG_COLOR = 0x0000;
static const uint16_t ISLAND_TRANSPARENT_COLOR = 0xF81F;

// SEA_TILE：60 x 29，一排 6 張，sheet = 360 x 29
static const int ISLAND_TILE_SHEET_W = 360;
static const int ISLAND_TILE_W = 60;
static const int ISLAND_TILE_H = 29;
static const int ISLAND_TILE_STEP_X = 60;
static const int ISLAND_TILE_STEP_Y = 15;
static const int ISLAND_FIRST_TILE_Y = -21;

// SEA_BOY：60 x 21，4 frames，每格 15 x 21
static const int ISLAND_BOY_SHEET_W = 60;
static const int ISLAND_BOY_FRAME_W = 15;
static const int ISLAND_BOY_FRAME_H = 21;
static const int ISLAND_BOY_ANCHOR_X = 7;
static const int ISLAND_BOY_ANCHOR_Y = 20;

// SEA_BOY_SHIP：76 x 36，2 frames，每格 38 x 36
static const int ISLAND_SHIP_SHEET_W = 76;
static const int ISLAND_SHIP_FRAME_W = 38;
static const int ISLAND_SHIP_FRAME_H = 36;
static const int ISLAND_SHIP_ANCHOR_X = 19;
static const int ISLAND_SHIP_ANCHOR_Y = 35;


// SEA_BOY_SHIP2：48 x 30，2 frames，每格 24 x 32
static const int ISLAND_SHIP2_SHEET_W = 48;
static const int ISLAND_SHIP2_FRAME_W = 24;
static const int ISLAND_SHIP2_FRAME_H = 30;
static const int ISLAND_SHIP2_ANCHOR_X = 12;
static const int ISLAND_SHIP2_ANCHOR_Y = 29;

static const uint8_t ISLAND_SHIP_TYPE_1 = 0;
static const uint8_t ISLAND_SHIP_TYPE_2 = 1;


static const uint8_t ISLAND_PLAYER_BOY = 0;
static const uint8_t ISLAND_PLAYER_GIRL = 1;

// SEA_GIRL：60 x 22，4 frames，每格 15 x 22
static const int ISLAND_GIRL_SHEET_W = 60;
static const int ISLAND_GIRL_FRAME_W = 15;
static const int ISLAND_GIRL_FRAME_H = 22;
static const int ISLAND_GIRL_ANCHOR_X = 7;
static const int ISLAND_GIRL_ANCHOR_Y = 21;

// SEA_GIRL_SHIP：76 x 37，2 frames，每格 38 x 37
static const int ISLAND_GIRL_SHIP_SHEET_W = 76;
static const int ISLAND_GIRL_SHIP_FRAME_W = 38;
static const int ISLAND_GIRL_SHIP_FRAME_H = 37;
static const int ISLAND_GIRL_SHIP_ANCHOR_X = 19;
static const int ISLAND_GIRL_SHIP_ANCHOR_Y = 36;

// SEA_GIRL_SHIP2：48 x 31，2 frames，每格 24 x 31
static const int ISLAND_GIRL_SHIP2_SHEET_W = 48;
static const int ISLAND_GIRL_SHIP2_FRAME_W = 24;
static const int ISLAND_GIRL_SHIP2_FRAME_H = 31;
static const int ISLAND_GIRL_SHIP2_ANCHOR_X = 12;
static const int ISLAND_GIRL_SHIP2_ANCHOR_Y = 30;


// 固定物件：SEA_B1 ~ SEA_B5
static const int ISLAND_B1_W = 86;
static const int ISLAND_B1_H = 56;

static const int ISLAND_B2_W = 86;
static const int ISLAND_B2_H = 68;

static const int ISLAND_B3_W = 64;
static const int ISLAND_B3_H = 56;

static const int ISLAND_B4_W = 52;
static const int ISLAND_B4_H = 46;

static const int ISLAND_B5_W = 44;
static const int ISLAND_B5_H = 43;



// 陸地動物 SEA_L1：44 x 14，共 2 格，每格 22 x 14
static const int ISLAND_L1_SHEET_W = 44;
static const int ISLAND_L1_FRAME_W = 22;
static const int ISLAND_L1_FRAME_H = 14;
static const int ISLAND_L1_ANCHOR_X = 11;
static const int ISLAND_L1_ANCHOR_Y = 13;

// 陸地動物 SEA_L2：50 x 21，共 2 格，每格 25 x 21
static const int ISLAND_L2_SHEET_W = 50;
static const int ISLAND_L2_FRAME_W = 25;
static const int ISLAND_L2_FRAME_H = 21;
static const int ISLAND_L2_ANCHOR_X = 12;
static const int ISLAND_L2_ANCHOR_Y = 20;

static const int ISLAND_LAND_ANIMAL_COUNT = 2;

static const uint8_t ISLAND_LAND_ANIMAL_L1 = 0;
static const uint8_t ISLAND_LAND_ANIMAL_L2 = 1;

static const unsigned long ISLAND_LAND_ANIMAL_MOVE_INTERVAL_MS = 420UL;
static const unsigned long ISLAND_LAND_ANIMAL_ANIM_INTERVAL_MS = 260UL;
static const unsigned long ISLAND_LAND_ANIMAL_IDLE_MIN_MS = 1800UL;
static const unsigned long ISLAND_LAND_ANIMAL_IDLE_MAX_MS = 5000UL;

static const int ISLAND_LAND_ANIMAL_MOVE_STEP_X = 2;
static const int ISLAND_LAND_ANIMAL_MOVE_STEP_Y = 1;



// 前景物件：會畫在主角前面，用來覆蓋主角
static const int ISLAND_OBJ1_W = 24;
static const int ISLAND_OBJ1_H = 13;

static const int ISLAND_OBJ2_W = 34;
static const int ISLAND_OBJ2_H = 24;

// 沙灘椅
static const int ISLAND_SEAT_SHEET_W = 96;
static const int ISLAND_SEAT_FRAME_W = 24;
static const int ISLAND_SEAT_FRAME_H = 23;
static const int ISLAND_SEAT_FRAME_COUNT = 4;
static const int ISLAND_SEAT_COUNT = 9;



// 海洋 NPC：SEA_NPC，80 x 22，共 4 個獨立角色，每格 20 x 22
static const int ISLAND_NPC_SHEET_W = 80;
static const int ISLAND_NPC_FRAME_W = 20;
static const int ISLAND_NPC_FRAME_H = 22;
static const int ISLAND_NPC_ANCHOR_X = 10;
static const int ISLAND_NPC_ANCHOR_Y = 21;

static const int ISLAND_NPC_FRAME_COUNT = 4;

// 實際生成幾個 NPC
static const int ISLAND_NPC_COUNT = 4;

// NPC 移動很慢
static const unsigned long ISLAND_NPC_MOVE_INTERVAL_MS = 520UL;
static const unsigned long ISLAND_NPC_IDLE_MIN_MS = 2500UL;
static const unsigned long ISLAND_NPC_IDLE_MAX_MS = 7000UL;

static const int ISLAND_NPC_MOVE_STEP_X = 1;
static const int ISLAND_NPC_MOVE_STEP_Y = 1;


// 海洋小魚 SEA_FISH：160 x 20
static const int ISLAND_FISH_SHEET_W = 160;
static const int ISLAND_FISH_FRAME_W = 20;
static const int ISLAND_FISH_FRAME_H = 20;
static const int ISLAND_FISH_ANCHOR_X = 10;
static const int ISLAND_FISH_ANCHOR_Y = 19;
static const int ISLAND_FISH_TYPE_COUNT = 4;

// 海洋大魚 SEA_BIG_FISH：198 x 22
static const int ISLAND_BIG_FISH_SHEET_W = 198;
static const int ISLAND_BIG_FISH_FRAME_W = 33;
static const int ISLAND_BIG_FISH_FRAME_H = 22;
static const int ISLAND_BIG_FISH_ANCHOR_X = 16;
static const int ISLAND_BIG_FISH_ANCHOR_Y = 21;
static const int ISLAND_BIG_FISH_TYPE_COUNT = 3;
// 魚群數量，可自行調整

static const int ISLAND_FISH_COUNT = 4;
static const int ISLAND_BIG_FISH_COUNT = 3;

// 魚移動速度，比 NPC 再慢一點
static const unsigned long ISLAND_FISH_MOVE_INTERVAL_MS = 680UL;
static const unsigned long ISLAND_BIG_FISH_MOVE_INTERVAL_MS = 850UL;

static const unsigned long ISLAND_FISH_IDLE_MIN_MS = 1500UL;
static const unsigned long ISLAND_FISH_IDLE_MAX_MS = 5000UL;

static const int ISLAND_FISH_MOVE_STEP_X = 1;
static const int ISLAND_FISH_MOVE_STEP_Y = 1;


// 主角目前使用哪一種船
static uint8_t islandShipType = ISLAND_SHIP_TYPE_1;

static uint8_t islandPlayerGender = ISLAND_PLAYER_BOY;

// 小魚狀態
static int islandFishX[ISLAND_FISH_COUNT];
static int islandFishY[ISLAND_FISH_COUNT];
static int islandFishTargetX[ISLAND_FISH_COUNT];
static int islandFishTargetY[ISLAND_FISH_COUNT];
static uint8_t islandFishDir[ISLAND_FISH_COUNT];
static uint8_t islandFishType[ISLAND_FISH_COUNT];

static unsigned long islandFishLastMoveMs[ISLAND_FISH_COUNT];
static unsigned long islandFishIdleUntilMs[ISLAND_FISH_COUNT];

// 大魚狀態
static int islandBigFishX[ISLAND_BIG_FISH_COUNT];
static int islandBigFishY[ISLAND_BIG_FISH_COUNT];
static int islandBigFishTargetX[ISLAND_BIG_FISH_COUNT];
static int islandBigFishTargetY[ISLAND_BIG_FISH_COUNT];
static uint8_t islandBigFishDir[ISLAND_BIG_FISH_COUNT];
static uint8_t islandBigFishType[ISLAND_BIG_FISH_COUNT];
static unsigned long islandBigFishLastMoveMs[ISLAND_BIG_FISH_COUNT];
static unsigned long islandBigFishIdleUntilMs[ISLAND_BIG_FISH_COUNT];



static int islandLandAnimalX[ISLAND_LAND_ANIMAL_COUNT];
static int islandLandAnimalY[ISLAND_LAND_ANIMAL_COUNT];
static int islandLandAnimalTargetX[ISLAND_LAND_ANIMAL_COUNT];
static int islandLandAnimalTargetY[ISLAND_LAND_ANIMAL_COUNT];

static uint8_t islandLandAnimalType[ISLAND_LAND_ANIMAL_COUNT];
static uint8_t islandLandAnimalDir[ISLAND_LAND_ANIMAL_COUNT];
static uint8_t islandLandAnimalAnimPhase[ISLAND_LAND_ANIMAL_COUNT];

static unsigned long islandLandAnimalLastMoveMs[ISLAND_LAND_ANIMAL_COUNT];
static unsigned long islandLandAnimalLastAnimMs[ISLAND_LAND_ANIMAL_COUNT];
static unsigned long islandLandAnimalIdleUntilMs[ISLAND_LAND_ANIMAL_COUNT];



// 主角與沙灘椅碰撞盒
// 這裡用主角身體範圍，不用整張圖，避免太容易卡住
static const int ISLAND_PLAYER_COLLISION_W = 12;
static const int ISLAND_PLAYER_COLLISION_H = 16;
static const int ISLAND_PLAYER_COLLISION_OFFSET_X = -6;
static const int ISLAND_PLAYER_COLLISION_OFFSET_Y = -16;



static const unsigned long ISLAND_MODE_DURATION_MS = 300000UL; // 5 分鐘
static const unsigned long ISLAND_IDLE_MS = 10000UL;  // 主角待機時間
static const unsigned long ISLAND_LAND_MOVE_INTERVAL_MS = 140UL;  // 主角陸地移動速度
static const unsigned long ISLAND_OCEAN_MOVE_INTERVAL_MS = 100UL;  // 主角海洋移動速度
static const unsigned long ISLAND_ANIM_INTERVAL_MS = 240UL;
static const uint8_t ISLAND_DIR_STABLE_TICKS = 5;

static const int ISLAND_MOVE_STEP_X = 2;
static const int ISLAND_MOVE_STEP_Y = 1;

// 主角初始出生點座標
static const int ISLAND_START_X = 262;
static const int ISLAND_START_Y = 117;

// 陸地 / 海洋模式切換交會點座標
static const int ISLAND_GATE_X = 70;  
static const int ISLAND_GATE_Y = 355;

enum IslandAreaMode {
  ISLAND_AREA_LAND = 0,
  ISLAND_AREA_OCEAN = 1
};

enum IslandPlayerState {
  ISLAND_STATE_MOVING = 0,
  ISLAND_STATE_IDLE = 1,
  ISLAND_STATE_TO_GATE = 2
};

enum IslandDir {
  ISLAND_DIR_UP = 0,
  ISLAND_DIR_DOWN = 1,
  ISLAND_DIR_LEFT = 2,
  ISLAND_DIR_RIGHT = 3
};

static const int16_t islandLandPolygon[][2] = {
  {496,   0},
  {  0, 248},
  { 27, 376},
  {719,  30}
};

static const int16_t islandOceanPolygon[][2] = {
  { 27, 376},
  {719,  30},
  {719, 376}
};

static const int ISLAND_LAND_POLYGON_COUNT =
  sizeof(islandLandPolygon) / sizeof(islandLandPolygon[0]);

static const int ISLAND_OCEAN_POLYGON_COUNT =
  sizeof(islandOceanPolygon) / sizeof(islandOceanPolygon[0]);

// SEA_MAP 由 SEA_TILE 拼接而成。
static const int ISLAND_MAP_ROW_MAX_TILES = 18;

static const int16_t islandMapRowStartX[] PROGMEM = {
   390, 360,
   330, 300, 270, 240, 210,
   180, 150, 120,  90,  60,
    30,   0, -30,   0, -30,
     0, -30,   0, -30,   0,
   -30,   0, -30,   0, -30
};

static const char islandMapRows[][ISLAND_MAP_ROW_MAX_TILES] PROGMEM = {
  "1122234",
  "1122234",

  "1122234",
  "1122234",
  "01222345",
  "10222345",
  "112223455",
  "012223455",
  "1022234555",
  "1122234555",
  "01222345555",
  "1022234555",
  "11222345555",
  "112223455555",
  "1122234555555",
  "122234555555",
  "1222345555555",
  "222345555555",
  "2223455555555",
  "223455555555",
  "2234555555555",
  "234555555555",
  "2345555555555",
  "345555555555",
  "3455555555555",
  "455555555555",
  "4555555555555"
};

static const int ISLAND_MAP_ROW_COUNT =
  sizeof(islandMapRows) / sizeof(islandMapRows[0]);



// 沙灘椅固定座標，座標代表圖片左上角的世界座標
static const int16_t islandSeatPos[ISLAND_SEAT_COUNT][2] PROGMEM = {
  {449,  55},
  {419,  68},
  {349, 102},
  {322, 116},
  {265, 145},
  {240, 158},
  { 85, 226},
  {64, 238},
  { 42, 251}
};

// 每個座標對應 SEA_SEAT 的隨機 frame
static uint8_t islandSeatFrame[ISLAND_SEAT_COUNT];

static int islandNpcX[ISLAND_NPC_COUNT];
static int islandNpcY[ISLAND_NPC_COUNT];
static int islandNpcTargetX[ISLAND_NPC_COUNT];
static int islandNpcTargetY[ISLAND_NPC_COUNT];

static uint8_t islandNpcDir[ISLAND_NPC_COUNT];
static unsigned long islandNpcLastMoveMs[ISLAND_NPC_COUNT];
static unsigned long islandNpcIdleUntilMs[ISLAND_NPC_COUNT];
// 每個 NPC 使用 SEA_NPC 的哪一個獨立角色圖
static uint8_t islandNpcFrame[ISLAND_NPC_COUNT];

static bool islandModeReady = false;

static int islandCameraX = 0;
static int islandCameraY = 0;

static int islandPlayerX = ISLAND_START_X;
static int islandPlayerY = ISLAND_START_Y;
static int islandTargetX = ISLAND_START_X;
static int islandTargetY = ISLAND_START_Y;

static uint8_t islandCurrentArea = ISLAND_AREA_LAND;
static uint8_t islandNextArea = ISLAND_AREA_LAND;
static uint8_t islandPlayerState = ISLAND_STATE_MOVING;
static uint8_t islandDir = ISLAND_DIR_DOWN;
static uint8_t islandPendingDir = ISLAND_DIR_DOWN;
static uint8_t islandDirStableCount = 0;
static uint8_t islandAnimPhase = 0;

static unsigned long islandModeStartMs = 0;
static unsigned long islandIdleStartMs = 0;
static unsigned long islandLastMoveMs = 0;
static unsigned long islandLastAnimMs = 0;

static const uint16_t* getIslandPaletteForBitmap(const uint8_t* bitmap) {
  if (bitmap == SEA_TILE) return SEA_TILE_PALETTE;

  if (bitmap == SEA_BOY) return SEA_BOY_PALETTE;
  if (bitmap == SEA_BOY_SHIP) return SEA_BOY_SHIP_PALETTE;
  if (bitmap == SEA_BOY_SHIP2) return SEA_BOY_SHIP2_PALETTE;

  if (bitmap == SEA_GIRL) return SEA_GIRL_PALETTE;
  if (bitmap == SEA_GIRL_SHIP) return SEA_GIRL_SHIP_PALETTE;
  if (bitmap == SEA_GIRL_SHIP2) return SEA_GIRL_SHIP2_PALETTE;

  if (bitmap == SEA_B1) return SEA_B1_PALETTE;
  if (bitmap == SEA_B2) return SEA_B2_PALETTE;
  if (bitmap == SEA_B3) return SEA_B3_PALETTE;
  if (bitmap == SEA_B4) return SEA_B4_PALETTE;
  if (bitmap == SEA_B5) return SEA_B5_PALETTE;

  if (bitmap == SEA_L1) return SEA_L1_PALETTE;
  if (bitmap == SEA_L2) return SEA_L2_PALETTE;

  if (bitmap == SEA_OBJ1) return SEA_OBJ1_PALETTE;
  if (bitmap == SEA_OBJ2) return SEA_OBJ2_PALETTE;
  if (bitmap == SEA_SEAT) return SEA_SEAT_PALETTE;

  if (bitmap == SEA_NPC) return SEA_NPC_PALETTE;
  if (bitmap == SEA_FISH) return SEA_FISH_PALETTE;
  if (bitmap == SEA_BIG_FISH) return SEA_BIG_FISH_PALETTE;

  return NULL;
}

static void drawIslandRgb565Frame(
  const uint8_t* bitmap,
  int sheetW,
  int frameW,
  int frameH,
  int frameIndex,
  bool flipX,
  int x,
  int y
) {
  if (frameIndex < 0) return;

  if (x <= -frameW || x >= ISLAND_SCR_W ||
      y <= -frameH || y >= ISLAND_SCR_H) {
    return;
  }

  const uint16_t* palette = getIslandPaletteForBitmap(bitmap);
  if (palette == NULL) return;

  int frameStartX = frameIndex * frameW;

  for (int j = 0; j < frameH; j++) {
    int dy = y + j;
    if (dy < 0 || dy >= ISLAND_SCR_H) continue;

    for (int i = 0; i < frameW; i++) {
      int dx = x + i;
      if (dx < 0 || dx >= ISLAND_SCR_W) continue;

      int sx = flipX ? (frameW - 1 - i) : i;

      uint32_t pos =
        (uint32_t)j * (uint32_t)sheetW +
        (uint32_t)frameStartX +
        (uint32_t)sx;

      // 索引圖：先讀 1 byte 的 palette index
      uint8_t colorIndex = pgm_read_byte(&(bitmap[pos]));

      // 再用 index 去 palette 讀 RGB565 顏色
      uint16_t color = pgm_read_word(&(palette[colorIndex]));

      if (color == ISLAND_TRANSPARENT_COLOR) continue;

      display.drawPixel(dx, dy, color);
    }
  }
}

static void clampIslandCamera() {
  if (islandCameraX < 0) islandCameraX = 0;
  if (islandCameraY < 0) islandCameraY = 0;

  int maxCameraX = ISLAND_WORLD_W - ISLAND_SCR_W;
  int maxCameraY = ISLAND_WORLD_H - ISLAND_SCR_H;

  if (maxCameraX < 0) maxCameraX = 0;
  if (maxCameraY < 0) maxCameraY = 0;

  if (islandCameraX > maxCameraX) islandCameraX = maxCameraX;
  if (islandCameraY > maxCameraY) islandCameraY = maxCameraY;
}

static void updateIslandCamera() {
  islandCameraX = islandPlayerX - (ISLAND_SCR_W / 2);
  islandCameraY = islandPlayerY - (ISLAND_SCR_H / 2) - 25;

  clampIslandCamera();
}

static bool isIslandPointInPolygon(
  int x,
  int y,
  const int16_t polygon[][2],
  int count
) {
  bool inside = false;

  for (int i = 0, j = count - 1; i < count; j = i++) {
    long xi = polygon[i][0];
    long yi = polygon[i][1];
    long xj = polygon[j][0];
    long yj = polygon[j][1];

    bool intersect =
      ((yi > y) != (yj > y)) &&
      ((long)x <
       (xj - xi) * ((long)y - yi) / (yj - yi) + xi);

    if (intersect) inside = !inside;
  }

  return inside;
}

static bool isIslandPointInArea(uint8_t area, int x, int y) {
  if (x < 0 || x >= ISLAND_WORLD_W) return false;
  if (y < 0 || y >= ISLAND_WORLD_H) return false;

  if (area == ISLAND_AREA_LAND) {
    return isIslandPointInPolygon(
      x,
      y,
      islandLandPolygon,
      ISLAND_LAND_POLYGON_COUNT
    );
  }

  return isIslandPointInPolygon(
    x,
    y,
    islandOceanPolygon,
    ISLAND_OCEAN_POLYGON_COUNT
  );
}


static bool islandRectOverlap(
  int ax,
  int ay,
  int aw,
  int ah,
  int bx,
  int by,
  int bw,
  int bh
) {
  if (ax + aw <= bx) return false;
  if (bx + bw <= ax) return false;
  if (ay + ah <= by) return false;
  if (by + bh <= ay) return false;

  return true;
}

static bool isIslandPlayerCollidingSeatAt(int playerX, int playerY) {
  int playerLeft =
    playerX + ISLAND_PLAYER_COLLISION_OFFSET_X;

  int playerTop =
    playerY + ISLAND_PLAYER_COLLISION_OFFSET_Y;

  for (int i = 0; i < ISLAND_SEAT_COUNT; i++) {
    int seatX = (int16_t)pgm_read_word(&(islandSeatPos[i][0]));
    int seatY = (int16_t)pgm_read_word(&(islandSeatPos[i][1]));

    // 超出世界邊界的座椅不用檢查
    if (seatX >= ISLAND_WORLD_W || seatY >= ISLAND_WORLD_H) {
      continue;
    }

    if (islandRectOverlap(
          playerLeft,
          playerTop,
          ISLAND_PLAYER_COLLISION_W,
          ISLAND_PLAYER_COLLISION_H,
          seatX,
          seatY,
          ISLAND_SEAT_FRAME_W,
          ISLAND_SEAT_FRAME_H
        )) {
      return true;
    }
  }

  return false;
}


static bool isIslandLandAnimalCollidingSeatAt(
  int index,
  int animalX,
  int animalY
) {
  int frameW = ISLAND_L1_FRAME_W;
  int frameH = ISLAND_L1_FRAME_H;
  int anchorX = ISLAND_L1_ANCHOR_X;
  int anchorY = ISLAND_L1_ANCHOR_Y;

  if (islandLandAnimalType[index] == ISLAND_LAND_ANIMAL_L2) {
    frameW = ISLAND_L2_FRAME_W;
    frameH = ISLAND_L2_FRAME_H;
    anchorX = ISLAND_L2_ANCHOR_X;
    anchorY = ISLAND_L2_ANCHOR_Y;
  }

  int animalLeft = animalX - anchorX;
  int animalTop = animalY - anchorY;

  for (int i = 0; i < ISLAND_SEAT_COUNT; i++) {
    int seatX = (int16_t)pgm_read_word(&(islandSeatPos[i][0]));
    int seatY = (int16_t)pgm_read_word(&(islandSeatPos[i][1]));

    if (seatX >= ISLAND_WORLD_W || seatY >= ISLAND_WORLD_H) {
      continue;
    }

    if (islandRectOverlap(
          animalLeft,
          animalTop,
          frameW,
          frameH,
          seatX,
          seatY,
          ISLAND_SEAT_FRAME_W,
          ISLAND_SEAT_FRAME_H
        )) {
      return true;
    }
  }

  return false;
}


static void chooseIslandRandomPointInArea(uint8_t area, int* outX, int* outY) {
  int minX = 0;
  int maxX = ISLAND_WORLD_W - 1;
  int minY = 0;
  int maxY = ISLAND_WORLD_H - 1;

  if (area == ISLAND_AREA_LAND) {
    minX = 0;
    maxX = 719;
    minY = 0;
    maxY = 376;
  } else {
    minX = 27;
    maxX = 719;
    minY = 30;
    maxY = 376;
  }

  for (int tries = 0; tries < 256; tries++) {
    int x = random(minX, maxX + 1);
    int y = random(minY, maxY + 1);

if (isIslandPointInArea(area, x, y)) {
  // 陸地移動時，避免目標點落在沙灘椅碰撞範圍內
  if (area == ISLAND_AREA_LAND &&
      isIslandPlayerCollidingSeatAt(x, y)) {
    continue;
  }

  *outX = x;
  *outY = y;
  return;
}
  }

  if (area == ISLAND_AREA_LAND) {
    *outX = 262;
    *outY = 117;
  } else {
    *outX = 560;
    *outY = 260;
  }
}

static void beginIslandMoveToRandomTarget(uint8_t area) {
  chooseIslandRandomPointInArea(
    area,
    &islandTargetX,
    &islandTargetY
  );

  islandPlayerState = ISLAND_STATE_MOVING;
}

static void beginIslandArea(uint8_t area, unsigned long nowMs) {
  islandCurrentArea = area;
  islandNextArea = area;
  islandModeStartMs = nowMs;

  beginIslandMoveToRandomTarget(area);
}

static void chooseIslandPlayerGender() {
  islandPlayerGender =
    random(2) ? ISLAND_PLAYER_BOY : ISLAND_PLAYER_GIRL;
}


static void chooseIslandShipType() {
  islandShipType = random(2) ? ISLAND_SHIP_TYPE_1 : ISLAND_SHIP_TYPE_2;
}




static void beginIslandMoveToGate(uint8_t nextArea) {
  islandNextArea = nextArea;

  // 由陸地準備切換到海洋時，先隨機決定這次使用哪一艘船
  if (islandCurrentArea == ISLAND_AREA_LAND &&
      nextArea == ISLAND_AREA_OCEAN) {
    chooseIslandShipType();
  }

  islandTargetX = ISLAND_GATE_X;
  islandTargetY = ISLAND_GATE_Y;

  islandPlayerState = ISLAND_STATE_TO_GATE;
}

static bool isIslandPlayerNearTarget() {
  int dx = abs(islandTargetX - islandPlayerX);
  int dy = abs(islandTargetY - islandPlayerY);

  return dx <= ISLAND_MOVE_STEP_X && dy <= ISLAND_MOVE_STEP_Y;
}

static long getIslandDistanceScore(int x, int y) {
  long dx = (long)islandTargetX - (long)x;
  long dy = (long)islandTargetY - (long)y;

  return dx * dx + dy * dy;
}

static bool chooseIslandBestStep(
  int* outStepX,
  int* outStepY,
  uint8_t* outDir
) {
  static const int8_t stepX[4] = {
    -ISLAND_MOVE_STEP_X,
     ISLAND_MOVE_STEP_X,
    -ISLAND_MOVE_STEP_X,
     ISLAND_MOVE_STEP_X
  };

  static const int8_t stepY[4] = {
    -ISLAND_MOVE_STEP_Y,
     ISLAND_MOVE_STEP_Y,
     ISLAND_MOVE_STEP_Y,
    -ISLAND_MOVE_STEP_Y
  };

  static const uint8_t stepDir[4] = {
    ISLAND_DIR_UP,
    ISLAND_DIR_DOWN,
    ISLAND_DIR_LEFT,
    ISLAND_DIR_RIGHT
  };

  long bestScore = getIslandDistanceScore(islandPlayerX, islandPlayerY);
  int bestIndex = -1;

  for (int i = 0; i < 4; i++) {
    int nextX = islandPlayerX + stepX[i];
    int nextY = islandPlayerY + stepY[i];

    if (nextX < 0 || nextX >= ISLAND_WORLD_W) continue;
    if (nextY < 0 || nextY >= ISLAND_WORLD_H) continue;

    if (islandPlayerState == ISLAND_STATE_MOVING) {
      if (!isIslandPointInArea(islandCurrentArea, nextX, nextY)) {
        continue;
      }
    }

    long score = getIslandDistanceScore(nextX, nextY);

    if (score < bestScore) {
      bestScore = score;
      bestIndex = i;
    }
  }

  if (bestIndex < 0) {
    for (int i = 0; i < 4; i++) {
      int nextX = islandPlayerX + stepX[i];
      int nextY = islandPlayerY + stepY[i];

      if (nextX < 0 || nextX >= ISLAND_WORLD_W) continue;
      if (nextY < 0 || nextY >= ISLAND_WORLD_H) continue;

      long score = getIslandDistanceScore(nextX, nextY);

      if (score < bestScore) {
        bestScore = score;
        bestIndex = i;
      }
    }
  }

  if (bestIndex < 0) return false;

  *outStepX = stepX[bestIndex];
  *outStepY = stepY[bestIndex];
  *outDir = stepDir[bestIndex];

  return true;
}

static void updateIslandAnimation(unsigned long nowMs, bool moving) {
  if (!moving) {
    islandAnimPhase = 0;
    return;
  }

  if (nowMs - islandLastAnimMs >= ISLAND_ANIM_INTERVAL_MS) {
    islandLastAnimMs = nowMs;
    islandAnimPhase = islandAnimPhase ? 0 : 1;
  }
}

static void beginIslandIdleAtCurrentPosition(unsigned long nowMs) {
  islandPlayerState = ISLAND_STATE_IDLE;
  islandIdleStartMs = nowMs;
  islandAnimPhase = 0;
}


static void finishIslandTarget(unsigned long nowMs) {
  islandPlayerX = islandTargetX;
  islandPlayerY = islandTargetY;

  if (islandPlayerState == ISLAND_STATE_TO_GATE) {
    beginIslandArea(islandNextArea, nowMs);
    return;
  }

beginIslandIdleAtCurrentPosition(nowMs);
}

static int islandLimitedStep(int delta, int maxStep) {
  if (delta > maxStep) return maxStep;
  if (delta < -maxStep) return -maxStep;
  return delta;
}


static uint8_t getIslandSeaDirFromVector(
  int dx,
  int dy,
  uint8_t fallbackDir
) {
  if (dx == 0 && dy == 0) {
    return fallbackDir;
  }

  // 純上下
  if (dx == 0) {
    return dy >= 0 ? ISLAND_DIR_DOWN : ISLAND_DIR_UP;
  }

  // 純左右
  if (dy == 0) {
    return dx >= 0 ? ISLAND_DIR_RIGHT : ISLAND_DIR_LEFT;
  }

  // 45 度斜向邏輯
  if (dx >= 0 && dy >= 0) return ISLAND_DIR_DOWN;
  if (dx <  0 && dy >= 0) return ISLAND_DIR_LEFT;
  if (dx >= 0 && dy <  0) return ISLAND_DIR_RIGHT;

  return ISLAND_DIR_UP;
}







static uint8_t getIslandNaturalDirFromVector(int dx, int dy) {
  int adx = abs(dx);
  int ady = abs(dy);

  if (adx <= ISLAND_MOVE_STEP_X && ady <= ISLAND_MOVE_STEP_Y) {
    return islandDir;
  }

  if (adx <= ISLAND_MOVE_STEP_X) {
    return dy >= 0 ? ISLAND_DIR_DOWN : ISLAND_DIR_UP;
  }

  if (ady <= ISLAND_MOVE_STEP_Y) {
    return dx >= 0 ? ISLAND_DIR_RIGHT : ISLAND_DIR_LEFT;
  }

  // 45 度方向判定
  if (dx >= 0 && dy >= 0) return ISLAND_DIR_DOWN;
  if (dx <  0 && dy >= 0) return ISLAND_DIR_LEFT;
  if (dx >= 0 && dy <  0) return ISLAND_DIR_RIGHT;

  return ISLAND_DIR_UP;
}

static void updateIslandNaturalFacing(int dx, int dy) {
  uint8_t wantedDir = getIslandNaturalDirFromVector(dx, dy);

  if (wantedDir == islandDir) {
    islandPendingDir = wantedDir;
    islandDirStableCount = 0;
    return;
  }

  if (wantedDir != islandPendingDir) {
    islandPendingDir = wantedDir;
    islandDirStableCount = 1;
    return;
  }

  if (islandDirStableCount < 255) {
    islandDirStableCount++;
  }

  if (islandDirStableCount >= ISLAND_DIR_STABLE_TICKS) {
    islandDir = wantedDir;
    islandDirStableCount = 0;

    // 方向真的切換時，從第一幀開始，避免 frame1/3 交錯造成閃爍感
    islandAnimPhase = 0;
  }
}

static bool canIslandMoveToCandidate(int x, int y) {
  if (x < 0 || x >= ISLAND_WORLD_W) return false;
  if (y < 0 || y >= ISLAND_WORLD_H) return false;

  if (islandPlayerState == ISLAND_STATE_MOVING) {
    if (!isIslandPointInArea(islandCurrentArea, x, y)) {
      return false;
    }

    // 主角在陸地活動時，不允許走進 SEA_SEAT
    if (islandCurrentArea == ISLAND_AREA_LAND &&
        isIslandPlayerCollidingSeatAt(x, y)) {
      return false;
    }
  }

  return true;
}



static bool moveIslandNaturalStep() {
  int dx = islandTargetX - islandPlayerX;
  int dy = islandTargetY - islandPlayerY;

  int sx = islandLimitedStep(dx, ISLAND_MOVE_STEP_X);
  int sy = islandLimitedStep(dy, ISLAND_MOVE_STEP_Y);

  if (sx == 0 && sy == 0) {
    return false;
  }

  // 方向只看「目標方向」，不要看每一步實際走法。
  // 這是備份版主角不會亂跳方向的關鍵。
  updateIslandNaturalFacing(dx, dy);

  int nextX = islandPlayerX + sx;
  int nextY = islandPlayerY + sy;

  if (canIslandMoveToCandidate(nextX, nextY)) {
    islandPlayerX = nextX;
    islandPlayerY = nextY;
    return true;
  }

  // 如果斜向一步撞到邊界 / 椅子，改試單軸移動，避免直接卡死。
  bool tryXFirst =
    abs(dx) * ISLAND_MOVE_STEP_Y >= abs(dy) * ISLAND_MOVE_STEP_X;

  if (tryXFirst) {
    if (sx != 0 &&
        canIslandMoveToCandidate(islandPlayerX + sx, islandPlayerY)) {
      islandPlayerX += sx;
      return true;
    }

    if (sy != 0 &&
        canIslandMoveToCandidate(islandPlayerX, islandPlayerY + sy)) {
      islandPlayerY += sy;
      return true;
    }
  } else {
    if (sy != 0 &&
        canIslandMoveToCandidate(islandPlayerX, islandPlayerY + sy)) {
      islandPlayerY += sy;
      return true;
    }

    if (sx != 0 &&
        canIslandMoveToCandidate(islandPlayerX + sx, islandPlayerY)) {
      islandPlayerX += sx;
      return true;
    }
  }

  return false;
}


static unsigned long getIslandPlayerMoveInterval() {
  if (islandCurrentArea == ISLAND_AREA_OCEAN) {
    return ISLAND_OCEAN_MOVE_INTERVAL_MS;
  }

  return ISLAND_LAND_MOVE_INTERVAL_MS;
}


static void updateIslandMovement(unsigned long nowMs) {
  if (islandPlayerState == ISLAND_STATE_IDLE) {
    if (nowMs - islandIdleStartMs >= ISLAND_IDLE_MS) {
      beginIslandMoveToRandomTarget(islandCurrentArea);
    }

    updateIslandAnimation(nowMs, false);
    return;
  }

  if (isIslandPlayerNearTarget()) {
    finishIslandTarget(nowMs);
    return;
  }

if (nowMs - islandLastMoveMs < getIslandPlayerMoveInterval()) {
    updateIslandAnimation(nowMs, true);
    return;
  }

  islandLastMoveMs = nowMs;

if (moveIslandNaturalStep()) {
  updateIslandAnimation(nowMs, true);
} else {
  // 可能是路徑撞到 SEA_SEAT，原地待機，下一輪再重新抽目標
  beginIslandIdleAtCurrentPosition(nowMs);
}
}


static int islandLimitedNpcStep(int delta, int maxStep) {
  if (delta > maxStep) return maxStep;
  if (delta < -maxStep) return -maxStep;
  return delta;
}

static uint8_t getIslandNpcDirFromVector(int dx, int dy) {
  int adx = abs(dx);
  int ady = abs(dy);

  if (adx <= ISLAND_NPC_MOVE_STEP_X &&
      ady <= ISLAND_NPC_MOVE_STEP_Y) {
    return ISLAND_DIR_DOWN;
  }

  if (adx <= ISLAND_NPC_MOVE_STEP_X) {
    return dy >= 0 ? ISLAND_DIR_DOWN : ISLAND_DIR_UP;
  }

  if (ady <= ISLAND_NPC_MOVE_STEP_Y) {
    return dx >= 0 ? ISLAND_DIR_RIGHT : ISLAND_DIR_LEFT;
  }

  if (dx >= 0 && dy >= 0) return ISLAND_DIR_DOWN;
  if (dx <  0 && dy >= 0) return ISLAND_DIR_LEFT;
  if (dx >= 0 && dy <  0) return ISLAND_DIR_RIGHT;

  return ISLAND_DIR_UP;
}

static void chooseIslandNpcTarget(int index) {
  chooseIslandRandomPointInArea(
    ISLAND_AREA_OCEAN,
    &islandNpcTargetX[index],
    &islandNpcTargetY[index]
  );
}

static void initIslandNpcOne(int index, unsigned long nowMs) {
  chooseIslandRandomPointInArea(
    ISLAND_AREA_OCEAN,
    &islandNpcX[index],
    &islandNpcY[index]
  );

  chooseIslandNpcTarget(index);

  // SEA_NPC 是 4 個獨立角色，不是方向動畫
  // 這裡讓 4 個 NPC 分別使用 frame0, frame1, frame2, frame3
  islandNpcFrame[index] = index % ISLAND_NPC_FRAME_COUNT;

  islandNpcDir[index] = ISLAND_DIR_DOWN;
  islandNpcLastMoveMs[index] = nowMs + random(0, 500);
  islandNpcIdleUntilMs[index] = 0;
}

static bool isIslandNpcNearTarget(int index) {
  int dx = abs(islandNpcTargetX[index] - islandNpcX[index]);
  int dy = abs(islandNpcTargetY[index] - islandNpcY[index]);

  return dx <= ISLAND_NPC_MOVE_STEP_X &&
         dy <= ISLAND_NPC_MOVE_STEP_Y;
}

static void updateIslandNpcOne(int index, unsigned long nowMs) {
  if (islandNpcIdleUntilMs[index] > nowMs) {
    return;
  }

  if (isIslandNpcNearTarget(index)) {
    chooseIslandNpcTarget(index);

    islandNpcIdleUntilMs[index] =
      nowMs +
      random(ISLAND_NPC_IDLE_MIN_MS, ISLAND_NPC_IDLE_MAX_MS);

    return;
  }

  if (nowMs - islandNpcLastMoveMs[index] < ISLAND_NPC_MOVE_INTERVAL_MS) {
    return;
  }

  islandNpcLastMoveMs[index] = nowMs;

  int dx = islandNpcTargetX[index] - islandNpcX[index];
  int dy = islandNpcTargetY[index] - islandNpcY[index];

  int sx = islandLimitedNpcStep(dx, ISLAND_NPC_MOVE_STEP_X);
  int sy = islandLimitedNpcStep(dy, ISLAND_NPC_MOVE_STEP_Y);

  int nextX = islandNpcX[index] + sx;
  int nextY = islandNpcY[index] + sy;

  islandNpcDir[index] = getIslandNpcDirFromVector(dx, dy);

  if (isIslandPointInArea(ISLAND_AREA_OCEAN, nextX, nextY)) {
    islandNpcX[index] = nextX;
    islandNpcY[index] = nextY;
  } else {
    chooseIslandNpcTarget(index);
    islandNpcIdleUntilMs[index] = nowMs + 1500UL;
  }
}

static void updateIslandNpcs(unsigned long nowMs) {
  for (int i = 0; i < ISLAND_NPC_COUNT; i++) {
    updateIslandNpcOne(i, nowMs);
  }
}



static int islandLimitedFishStep(int delta, int maxStep) {
  if (delta > maxStep) return maxStep;
  if (delta < -maxStep) return -maxStep;
  return delta;
}

static void chooseIslandFishTarget(int* targetX, int* targetY) {
  chooseIslandRandomPointInArea(
    ISLAND_AREA_OCEAN,
    targetX,
    targetY
  );
}

static bool isIslandFishNearTarget(
  int x,
  int y,
  int targetX,
  int targetY
) {
  int dx = abs(targetX - x);
  int dy = abs(targetY - y);

  return dx <= ISLAND_FISH_MOVE_STEP_X &&
         dy <= ISLAND_FISH_MOVE_STEP_Y;
}

static void initIslandFishOne(int index, unsigned long nowMs) {
  chooseIslandRandomPointInArea(
    ISLAND_AREA_OCEAN,
    &islandFishX[index],
    &islandFishY[index]
  );

  chooseIslandFishTarget(
    &islandFishTargetX[index],
    &islandFishTargetY[index]
  );

  // SEA_FISH 有 4 隻獨立魚，每隻魚有 2 個方向
  islandFishType[index] = index % ISLAND_FISH_TYPE_COUNT;

  islandFishDir[index] = random(2) ? ISLAND_DIR_DOWN : ISLAND_DIR_RIGHT;
  islandFishLastMoveMs[index] = nowMs + random(0, 800);
  islandFishIdleUntilMs[index] = nowMs + random(0, 2000);
}

static void initIslandBigFishOne(int index, unsigned long nowMs) {
  chooseIslandRandomPointInArea(
    ISLAND_AREA_OCEAN,
    &islandBigFishX[index],
    &islandBigFishY[index]
  );

  chooseIslandFishTarget(
    &islandBigFishTargetX[index],
    &islandBigFishTargetY[index]
  );

  // SEA_BIG_FISH 有 3 隻獨立魚，每隻魚有 2 個方向
  islandBigFishType[index] = index % ISLAND_BIG_FISH_TYPE_COUNT;

  islandBigFishDir[index] = random(2) ? ISLAND_DIR_DOWN : ISLAND_DIR_RIGHT;
  islandBigFishLastMoveMs[index] = nowMs + random(0, 1200);
  islandBigFishIdleUntilMs[index] = nowMs + random(0, 3000);
}

static void updateIslandFishOne(
  int index,
  unsigned long nowMs
) {
  if (islandFishIdleUntilMs[index] > nowMs) {
    return;
  }

  if (isIslandFishNearTarget(
        islandFishX[index],
        islandFishY[index],
        islandFishTargetX[index],
        islandFishTargetY[index]
      )) {
    chooseIslandFishTarget(
      &islandFishTargetX[index],
      &islandFishTargetY[index]
    );

    islandFishIdleUntilMs[index] =
      nowMs +
      random(ISLAND_FISH_IDLE_MIN_MS, ISLAND_FISH_IDLE_MAX_MS);

    return;
  }

  if (nowMs - islandFishLastMoveMs[index] <
      ISLAND_FISH_MOVE_INTERVAL_MS) {
    return;
  }

  islandFishLastMoveMs[index] = nowMs;

  int dx = islandFishTargetX[index] - islandFishX[index];
  int dy = islandFishTargetY[index] - islandFishY[index];

  int sx = islandLimitedFishStep(dx, ISLAND_FISH_MOVE_STEP_X);
  int sy = islandLimitedFishStep(dy, ISLAND_FISH_MOVE_STEP_Y);

  int nextX = islandFishX[index] + sx;
  int nextY = islandFishY[index] + sy;

islandFishDir[index] = getIslandSeaDirFromVector(
  sx,
  sy,
  islandFishDir[index]
);

  if (isIslandPointInArea(ISLAND_AREA_OCEAN, nextX, nextY)) {
    islandFishX[index] = nextX;
    islandFishY[index] = nextY;
  } else {
    chooseIslandFishTarget(
      &islandFishTargetX[index],
      &islandFishTargetY[index]
    );

    islandFishIdleUntilMs[index] = nowMs + 1200UL;
  }
}

static void updateIslandBigFishOne(
  int index,
  unsigned long nowMs
) {
  if (islandBigFishIdleUntilMs[index] > nowMs) {
    return;
  }

  if (isIslandFishNearTarget(
        islandBigFishX[index],
        islandBigFishY[index],
        islandBigFishTargetX[index],
        islandBigFishTargetY[index]
      )) {
    chooseIslandFishTarget(
      &islandBigFishTargetX[index],
      &islandBigFishTargetY[index]
    );

    islandBigFishIdleUntilMs[index] =
      nowMs +
      random(ISLAND_FISH_IDLE_MIN_MS, ISLAND_FISH_IDLE_MAX_MS);

    return;
  }

  if (nowMs - islandBigFishLastMoveMs[index] <
      ISLAND_BIG_FISH_MOVE_INTERVAL_MS) {
    return;
  }

  islandBigFishLastMoveMs[index] = nowMs;

  int dx = islandBigFishTargetX[index] - islandBigFishX[index];
  int dy = islandBigFishTargetY[index] - islandBigFishY[index];

  int sx = islandLimitedFishStep(dx, ISLAND_FISH_MOVE_STEP_X);
  int sy = islandLimitedFishStep(dy, ISLAND_FISH_MOVE_STEP_Y);

  int nextX = islandBigFishX[index] + sx;
  int nextY = islandBigFishY[index] + sy;

 islandBigFishDir[index] = getIslandSeaDirFromVector(
  sx,
  sy,
  islandBigFishDir[index]
);

  if (isIslandPointInArea(ISLAND_AREA_OCEAN, nextX, nextY)) {
    islandBigFishX[index] = nextX;
    islandBigFishY[index] = nextY;
  } else {
    chooseIslandFishTarget(
      &islandBigFishTargetX[index],
      &islandBigFishTargetY[index]
    );

    islandBigFishIdleUntilMs[index] = nowMs + 1500UL;
  }
}

static void updateIslandFishAll(unsigned long nowMs) {
  for (int i = 0; i < ISLAND_FISH_COUNT; i++) {
    updateIslandFishOne(i, nowMs);
  }

  for (int i = 0; i < ISLAND_BIG_FISH_COUNT; i++) {
    updateIslandBigFishOne(i, nowMs);
  }
}


static int islandLimitedLandAnimalStep(int delta, int maxStep) {
  if (delta > maxStep) return maxStep;
  if (delta < -maxStep) return -maxStep;
  return delta;
}

static uint8_t getIslandLandAnimalDirFromVector(
  int dx,
  int dy,
  uint8_t fallbackDir
) {
  if (dx == 0 && dy == 0) {
    return fallbackDir;
  }

  if (dx == 0) {
    return dy >= 0 ? ISLAND_DIR_DOWN : ISLAND_DIR_UP;
  }

  if (dy == 0) {
    return dx >= 0 ? ISLAND_DIR_RIGHT : ISLAND_DIR_LEFT;
  }

  if (dx >= 0 && dy >= 0) return ISLAND_DIR_DOWN;
  if (dx <  0 && dy >= 0) return ISLAND_DIR_LEFT;
  if (dx >= 0 && dy <  0) return ISLAND_DIR_RIGHT;

  return ISLAND_DIR_UP;
}

static void chooseIslandLandAnimalTarget(int index) {
  chooseIslandRandomPointInArea(
    ISLAND_AREA_LAND,
    &islandLandAnimalTargetX[index],
    &islandLandAnimalTargetY[index]
  );
}

static void initIslandLandAnimalOne(
  int index,
  uint8_t animalType,
  unsigned long nowMs
) {
  islandLandAnimalType[index] = animalType;

  chooseIslandRandomPointInArea(
    ISLAND_AREA_LAND,
    &islandLandAnimalX[index],
    &islandLandAnimalY[index]
  );

  chooseIslandLandAnimalTarget(index);

  islandLandAnimalDir[index] = random(2) ? ISLAND_DIR_DOWN : ISLAND_DIR_RIGHT;
  islandLandAnimalAnimPhase[index] = 0;

  islandLandAnimalLastMoveMs[index] = nowMs + random(0, 800);
  islandLandAnimalLastAnimMs[index] = nowMs + random(0, 300);
  islandLandAnimalIdleUntilMs[index] = nowMs + random(0, 2000);
}

static bool isIslandLandAnimalNearTarget(int index) {
  int dx =
    abs(islandLandAnimalTargetX[index] - islandLandAnimalX[index]);

  int dy =
    abs(islandLandAnimalTargetY[index] - islandLandAnimalY[index]);

  return dx <= ISLAND_LAND_ANIMAL_MOVE_STEP_X &&
         dy <= ISLAND_LAND_ANIMAL_MOVE_STEP_Y;
}

static void updateIslandLandAnimalOne(
  int index,
  unsigned long nowMs
) {
  if (islandLandAnimalIdleUntilMs[index] > nowMs) {
    islandLandAnimalAnimPhase[index] = 0;
    return;
  }

  if (isIslandLandAnimalNearTarget(index)) {
    chooseIslandLandAnimalTarget(index);

    islandLandAnimalIdleUntilMs[index] =
      nowMs +
      random(
        ISLAND_LAND_ANIMAL_IDLE_MIN_MS,
        ISLAND_LAND_ANIMAL_IDLE_MAX_MS
      );

    islandLandAnimalAnimPhase[index] = 0;
    return;
  }

  if (nowMs - islandLandAnimalLastMoveMs[index] <
      ISLAND_LAND_ANIMAL_MOVE_INTERVAL_MS) {
    return;
  }

  islandLandAnimalLastMoveMs[index] = nowMs;

  int dx =
    islandLandAnimalTargetX[index] - islandLandAnimalX[index];

  int dy =
    islandLandAnimalTargetY[index] - islandLandAnimalY[index];

  islandLandAnimalDir[index] =
    getIslandLandAnimalDirFromVector(
      dx,
      dy,
      islandLandAnimalDir[index]
    );

  int sx =
    islandLimitedLandAnimalStep(
      dx,
      ISLAND_LAND_ANIMAL_MOVE_STEP_X
    );

  int sy =
    islandLimitedLandAnimalStep(
      dy,
      ISLAND_LAND_ANIMAL_MOVE_STEP_Y
    );

  int nextX = islandLandAnimalX[index] + sx;
  int nextY = islandLandAnimalY[index] + sy;

if (isIslandPointInArea(ISLAND_AREA_LAND, nextX, nextY) &&
    !isIslandLandAnimalCollidingSeatAt(index, nextX, nextY)) {
  islandLandAnimalX[index] = nextX;
  islandLandAnimalY[index] = nextY;

    if (nowMs - islandLandAnimalLastAnimMs[index] >=
        ISLAND_LAND_ANIMAL_ANIM_INTERVAL_MS) {
      islandLandAnimalLastAnimMs[index] = nowMs;
      islandLandAnimalAnimPhase[index] =
        islandLandAnimalAnimPhase[index] ? 0 : 1;
    }

    return;
  }

  // 如果斜向走不通，嘗試單軸，避免卡邊界
if (sx != 0 &&
    isIslandPointInArea(
      ISLAND_AREA_LAND,
      islandLandAnimalX[index] + sx,
      islandLandAnimalY[index]
    ) &&
    !isIslandLandAnimalCollidingSeatAt(
      index,
      islandLandAnimalX[index] + sx,
      islandLandAnimalY[index]
    )) {
  islandLandAnimalX[index] += sx;
  return;
}

if (sy != 0 &&
    isIslandPointInArea(
      ISLAND_AREA_LAND,
      islandLandAnimalX[index],
      islandLandAnimalY[index] + sy
    ) &&
    !isIslandLandAnimalCollidingSeatAt(
      index,
      islandLandAnimalX[index],
      islandLandAnimalY[index] + sy
    )) {
  islandLandAnimalY[index] += sy;
  return;
}

  chooseIslandLandAnimalTarget(index);
  islandLandAnimalIdleUntilMs[index] = nowMs + 1200UL;
}

static void updateIslandLandAnimals(unsigned long nowMs) {
  for (int i = 0; i < ISLAND_LAND_ANIMAL_COUNT; i++) {
    updateIslandLandAnimalOne(i, nowMs);
  }
}


static void updateIslandFiveMinuteMode(unsigned long nowMs) {
  if (islandPlayerState == ISLAND_STATE_TO_GATE) return;

  if (nowMs - islandModeStartMs < ISLAND_MODE_DURATION_MS) return;

  uint8_t nextArea = random(2);

  if (nextArea == islandCurrentArea) {
    beginIslandArea(nextArea, nowMs);
  } else {
    beginIslandMoveToGate(nextArea);
  }
}

static int getIslandBoyFrameIndex() {
  if (islandDir == ISLAND_DIR_DOWN ||
      islandDir == ISLAND_DIR_LEFT) {
    return islandAnimPhase ? 1 : 0;
  }

  return islandAnimPhase ? 3 : 2;
}

static bool getIslandBoyFlipX() {
  return islandDir == ISLAND_DIR_LEFT ||
         islandDir == ISLAND_DIR_UP;
}



static bool isIslandUsingShipSprite() {
  return islandCurrentArea == ISLAND_AREA_OCEAN;
}

static void drawIslandMap() {
  for (int row = 0; row < ISLAND_MAP_ROW_COUNT; row++) {
    int16_t startX =
      (int16_t)pgm_read_word(&(islandMapRowStartX[row]));

    int tileY =
      ISLAND_FIRST_TILE_Y +
      row * ISLAND_TILE_STEP_Y;

    int drawY = tileY - islandCameraY;

    if (drawY >= ISLAND_SCR_H || drawY <= -ISLAND_TILE_H) {
      continue;
    }

    for (int col = 0; col < 32; col++) {
      int tileX =
        startX +
        col * ISLAND_TILE_STEP_X;

      // 讓 tile 往右超出世界地圖，避免右側海洋邊界露黑
      if (tileX > ISLAND_WORLD_W + ISLAND_TILE_W) {
        break;
      }

      int drawX = tileX - islandCameraX;

      if (drawX >= ISLAND_SCR_W) {
        break;
      }

      if (drawX <= -ISLAND_TILE_W) {
        continue;
      }

      char ch = '\0';

      if (col < ISLAND_MAP_ROW_MAX_TILES) {
        ch = pgm_read_byte(&(islandMapRows[row][col]));
      }

      // row 字串結束後，右側全部自動補海水 tile 5
      if (ch == '\0') {
        ch = '5';
      }

      // 原本若有 '.' 空洞，也直接補成海水 tile 5
      if (ch == '.') {
        ch = '5';
      }

      if (ch < '0' || ch > '5') {
        continue;
      }

      int tileId = ch - '0';

      drawIslandRgb565Frame(
        SEA_TILE,
        ISLAND_TILE_SHEET_W,
        ISLAND_TILE_W,
        ISLAND_TILE_H,
        tileId,
        false,
        drawX,
        drawY
      );
    }
  }
}

static void drawIslandObject(
  const uint8_t* bitmap,
  int w,
  int h,
  int worldX,
  int worldY
){
  int drawX = worldX - islandCameraX;
  int drawY = worldY - islandCameraY;

  drawIslandRgb565Frame(
    bitmap,
    w,
    w,
    h,
    0,
    false,
    drawX,
    drawY
  );
}



static void drawIslandObjectFrame(
  const uint8_t* bitmap,
  int sheetW,
  int frameW,
  int frameH,
  int frameIndex,
  int worldX,
  int worldY
) {
  int drawX = worldX - islandCameraX;
  int drawY = worldY - islandCameraY;

  drawIslandRgb565Frame(
    bitmap,
    sheetW,
    frameW,
    frameH,
    frameIndex,
    false,
    drawX,
    drawY
  );
}

static void drawIslandStaticObjects() {
  drawIslandObject(
    SEA_B1,
    ISLAND_B1_W,
    ISLAND_B1_H,
    239,
    56
  );

  drawIslandObject(
    SEA_B2,
    ISLAND_B2_W,
    ISLAND_B2_H,
    149,
    88
  );

  drawIslandObject(
    SEA_B3,
    ISLAND_B3_W,
    ISLAND_B3_H,
    329,
    11
  );

  drawIslandObject(
    SEA_B4,
    ISLAND_B4_W,
    ISLAND_B4_H,
    94,
    141
  );

  drawIslandObject(
    SEA_B5,
    ISLAND_B5_W,
    ISLAND_B5_H,
    385,
    -7
  );

  drawIslandObject(
    SEA_B5,
    ISLAND_B5_W,
    ISLAND_B5_H,
    54,
    165
  );

  drawIslandObject(
    SEA_B5,
    ISLAND_B5_W,
    ISLAND_B5_H,
    7,
    187
  );
}


static void drawIslandSeats() {
  for (int i = 0; i < ISLAND_SEAT_COUNT; i++) {
    int worldX = (int16_t)pgm_read_word(&(islandSeatPos[i][0]));
    int worldY = (int16_t)pgm_read_word(&(islandSeatPos[i][1]));

    drawIslandObjectFrame(
      SEA_SEAT,
      ISLAND_SEAT_SHEET_W,
      ISLAND_SEAT_FRAME_W,
      ISLAND_SEAT_FRAME_H,
      islandSeatFrame[i],
      worldX,
      worldY
    );
  }
}




static void drawIslandNpcs() {
  for (int i = 0; i < ISLAND_NPC_COUNT; i++) {
    int drawX =
      islandNpcX[i] -
      islandCameraX -
      ISLAND_NPC_ANCHOR_X;

    int drawY =
      islandNpcY[i] -
      islandCameraY -
      ISLAND_NPC_ANCHOR_Y;

    drawIslandRgb565Frame(
      SEA_NPC,
      ISLAND_NPC_SHEET_W,
      ISLAND_NPC_FRAME_W,
      ISLAND_NPC_FRAME_H,
      islandNpcFrame[i],
      false,
      drawX,
      drawY
    );
  }
}



static int getIslandSeaFrameIndex(uint8_t dir) {
  if (dir == ISLAND_DIR_DOWN ||
      dir == ISLAND_DIR_LEFT) {
    return 0;
  }

  return 1;
}

static bool getIslandSeaFlipX(uint8_t dir) {
  return dir == ISLAND_DIR_LEFT ||
         dir == ISLAND_DIR_UP;
}

static int getIslandShipFrameIndex() {
  return getIslandSeaFrameIndex(islandDir);
}

static bool getIslandShipFlipX() {
  return getIslandSeaFlipX(islandDir);
}

static int getIslandFishDirFrameIndex(uint8_t dir) {
  if (dir == ISLAND_DIR_DOWN ||
      dir == ISLAND_DIR_LEFT) {
    return 0;
  }

  return 1;
}

static int getIslandSmallFishFrameIndex(uint8_t fishType, uint8_t dir) {
  return fishType * 2 + getIslandFishDirFrameIndex(dir);
}

static int getIslandBigFishFrameIndex(uint8_t fishType, uint8_t dir) {
  return fishType * 2 + getIslandFishDirFrameIndex(dir);
}

static bool getIslandFishFlipX(uint8_t dir) {
  return getIslandSeaFlipX(dir);
}



static bool getIslandLandAnimalFlipX(uint8_t dir) {
  return dir == ISLAND_DIR_LEFT ||
         dir == ISLAND_DIR_UP;
}

static void drawIslandLandAnimalOne(int index) {
const uint8_t* bitmap = SEA_L1;
  int sheetW = ISLAND_L1_SHEET_W;
  int frameW = ISLAND_L1_FRAME_W;
  int frameH = ISLAND_L1_FRAME_H;
  int anchorX = ISLAND_L1_ANCHOR_X;
  int anchorY = ISLAND_L1_ANCHOR_Y;

  if (islandLandAnimalType[index] == ISLAND_LAND_ANIMAL_L2) {
    bitmap = SEA_L2;
    sheetW = ISLAND_L2_SHEET_W;
    frameW = ISLAND_L2_FRAME_W;
    frameH = ISLAND_L2_FRAME_H;
    anchorX = ISLAND_L2_ANCHOR_X;
    anchorY = ISLAND_L2_ANCHOR_Y;
  }

  int drawX =
    islandLandAnimalX[index] -
    islandCameraX -
    anchorX;

  int drawY =
    islandLandAnimalY[index] -
    islandCameraY -
    anchorY;

  drawIslandRgb565Frame(
    bitmap,
    sheetW,
    frameW,
    frameH,
    islandLandAnimalAnimPhase[index],
    getIslandLandAnimalFlipX(islandLandAnimalDir[index]),
    drawX,
    drawY
  );
}

static void drawIslandLandAnimals() {
  for (int i = 0; i < ISLAND_LAND_ANIMAL_COUNT; i++) {
    drawIslandLandAnimalOne(i);
  }
}



static void drawIslandFishAll() {
  for (int i = 0; i < ISLAND_FISH_COUNT; i++) {
    int drawX =
      islandFishX[i] -
      islandCameraX -
      ISLAND_FISH_ANCHOR_X;

    int drawY =
      islandFishY[i] -
      islandCameraY -
      ISLAND_FISH_ANCHOR_Y;

drawIslandRgb565Frame(
  SEA_FISH,
  ISLAND_FISH_SHEET_W,
  ISLAND_FISH_FRAME_W,
  ISLAND_FISH_FRAME_H,
  getIslandSmallFishFrameIndex(
    islandFishType[i],
    islandFishDir[i]
  ),
  getIslandFishFlipX(islandFishDir[i]),
  drawX,
  drawY
);
  }

  for (int i = 0; i < ISLAND_BIG_FISH_COUNT; i++) {
    int drawX =
      islandBigFishX[i] -
      islandCameraX -
      ISLAND_BIG_FISH_ANCHOR_X;

    int drawY =
      islandBigFishY[i] -
      islandCameraY -
      ISLAND_BIG_FISH_ANCHOR_Y;

drawIslandRgb565Frame(
  SEA_BIG_FISH,
  ISLAND_BIG_FISH_SHEET_W,
  ISLAND_BIG_FISH_FRAME_W,
  ISLAND_BIG_FISH_FRAME_H,
  getIslandBigFishFrameIndex(
    islandBigFishType[i],
    islandBigFishDir[i]
  ),
  getIslandFishFlipX(islandBigFishDir[i]),
  drawX,
  drawY
);
  }
}

static void drawIslandForegroundObjects() {
  // SEA_OBJ1，畫在主角前面
  drawIslandObject(
    SEA_OBJ1,
    ISLAND_OBJ1_W,
    ISLAND_OBJ1_H,
    70,
    355
  );

  drawIslandObject(
    SEA_OBJ1,
    ISLAND_OBJ1_W,
    ISLAND_OBJ1_H,
    55,
    364
  );

  // SEA_OBJ2，畫在主角前面
  drawIslandObject(
    SEA_OBJ2,
    ISLAND_OBJ2_W,
    ISLAND_OBJ2_H,
    83,
    337
  );
}



static void drawIslandPlayer() {
  if (isIslandUsingShipSprite()) {
    if (islandPlayerGender == ISLAND_PLAYER_GIRL) {
      if (islandShipType == ISLAND_SHIP_TYPE_2) {
        int drawX =
          islandPlayerX -
          islandCameraX -
          ISLAND_GIRL_SHIP2_ANCHOR_X;

        int drawY =
          islandPlayerY -
          islandCameraY -
          ISLAND_GIRL_SHIP2_ANCHOR_Y;

        drawIslandRgb565Frame(
          SEA_GIRL_SHIP2,
          ISLAND_GIRL_SHIP2_SHEET_W,
          ISLAND_GIRL_SHIP2_FRAME_W,
          ISLAND_GIRL_SHIP2_FRAME_H,
          getIslandShipFrameIndex(),
          getIslandShipFlipX(),
          drawX,
          drawY
        );
      } else {
        int drawX =
          islandPlayerX -
          islandCameraX -
          ISLAND_GIRL_SHIP_ANCHOR_X;

        int drawY =
          islandPlayerY -
          islandCameraY -
          ISLAND_GIRL_SHIP_ANCHOR_Y;

        drawIslandRgb565Frame(
          SEA_GIRL_SHIP,
          ISLAND_GIRL_SHIP_SHEET_W,
          ISLAND_GIRL_SHIP_FRAME_W,
          ISLAND_GIRL_SHIP_FRAME_H,
          getIslandShipFrameIndex(),
          getIslandShipFlipX(),
          drawX,
          drawY
        );
      }
    } else {
      if (islandShipType == ISLAND_SHIP_TYPE_2) {
        int drawX =
          islandPlayerX -
          islandCameraX -
          ISLAND_SHIP2_ANCHOR_X;

        int drawY =
          islandPlayerY -
          islandCameraY -
          ISLAND_SHIP2_ANCHOR_Y;

        drawIslandRgb565Frame(
          SEA_BOY_SHIP2,
          ISLAND_SHIP2_SHEET_W,
          ISLAND_SHIP2_FRAME_W,
          ISLAND_SHIP2_FRAME_H,
          getIslandShipFrameIndex(),
          getIslandShipFlipX(),
          drawX,
          drawY
        );
      } else {
        int drawX =
          islandPlayerX -
          islandCameraX -
          ISLAND_SHIP_ANCHOR_X;

        int drawY =
          islandPlayerY -
          islandCameraY -
          ISLAND_SHIP_ANCHOR_Y;

        drawIslandRgb565Frame(
          SEA_BOY_SHIP,
          ISLAND_SHIP_SHEET_W,
          ISLAND_SHIP_FRAME_W,
          ISLAND_SHIP_FRAME_H,
          getIslandShipFrameIndex(),
          getIslandShipFlipX(),
          drawX,
          drawY
        );
      }
    }
  } else {
    if (islandPlayerGender == ISLAND_PLAYER_GIRL) {
      int drawX =
        islandPlayerX -
        islandCameraX -
        ISLAND_GIRL_ANCHOR_X;

      int drawY =
        islandPlayerY -
        islandCameraY -
        ISLAND_GIRL_ANCHOR_Y;

      drawIslandRgb565Frame(
        SEA_GIRL,
        ISLAND_GIRL_SHEET_W,
        ISLAND_GIRL_FRAME_W,
        ISLAND_GIRL_FRAME_H,
        getIslandBoyFrameIndex(),
        getIslandBoyFlipX(),
        drawX,
        drawY
      );
    } else {
      int drawX =
        islandPlayerX -
        islandCameraX -
        ISLAND_BOY_ANCHOR_X;

      int drawY =
        islandPlayerY -
        islandCameraY -
        ISLAND_BOY_ANCHOR_Y;

      drawIslandRgb565Frame(
        SEA_BOY,
        ISLAND_BOY_SHEET_W,
        ISLAND_BOY_FRAME_W,
        ISLAND_BOY_FRAME_H,
        getIslandBoyFrameIndex(),
        getIslandBoyFlipX(),
        drawX,
        drawY
      );
    }
  }
}



static void renderIslandScene() {
  display.fillScreen(ISLAND_BG_COLOR);

  drawIslandMap();

  // 主角後方物件
  drawIslandStaticObjects();
  drawIslandLandAnimals();
  drawIslandSeats();
  drawIslandFishAll();
  drawIslandNpcs();


  // 主角
  drawIslandPlayer();

  // 主角前方物件，會覆蓋主角
  drawIslandForegroundObjects();

  drawThemeClockText();
}

static void IslandModeInit() {
  if (islandModeReady) return;

  randomSeed(millis());

  unsigned long nowMs = millis();

  for (int i = 0; i < ISLAND_SEAT_COUNT; i++) {
    islandSeatFrame[i] = random(ISLAND_SEAT_FRAME_COUNT);
  }

  for (int i = 0; i < ISLAND_NPC_COUNT; i++) {
    initIslandNpcOne(i, nowMs);
  }

initIslandLandAnimalOne(
  0,
  ISLAND_LAND_ANIMAL_L1,
  nowMs
);

initIslandLandAnimalOne(
  1,
  ISLAND_LAND_ANIMAL_L2,
  nowMs
);



for (int i = 0; i < ISLAND_FISH_COUNT; i++) {
  initIslandFishOne(i, nowMs);
}

for (int i = 0; i < ISLAND_BIG_FISH_COUNT; i++) {
  initIslandBigFishOne(i, nowMs);
}

chooseIslandPlayerGender();
chooseIslandShipType();

  islandPlayerX = ISLAND_START_X;
  islandPlayerY = ISLAND_START_Y;

  islandTargetX = islandPlayerX;
  islandTargetY = islandPlayerY;

  islandCurrentArea = ISLAND_AREA_LAND;
  islandNextArea = ISLAND_AREA_LAND;

  islandDir = ISLAND_DIR_DOWN;
  islandPendingDir = ISLAND_DIR_DOWN;
  islandDirStableCount = 0;
  islandAnimPhase = 0;

  islandLastMoveMs = nowMs;
  islandLastAnimMs = nowMs;
  islandIdleStartMs = nowMs;
  islandModeStartMs = nowMs;

  uint8_t firstArea = random(2);

  if (firstArea == ISLAND_AREA_OCEAN) {
    beginIslandMoveToGate(ISLAND_AREA_OCEAN);
  } else {
    beginIslandArea(ISLAND_AREA_LAND, nowMs);
  }

  updateIslandCamera();

  islandModeReady = true;
}

void IslandMode() {
  IslandModeInit();

  unsigned long nowMs = millis();

updateIslandFiveMinuteMode(nowMs);
updateIslandMovement(nowMs);
updateIslandNpcs(nowMs);
updateIslandFishAll(nowMs);
updateIslandLandAnimals(nowMs);
updateIslandCamera();

  renderIslandScene();

  wait_with_display(30);
}
