#include "Duck.h"
extern int cameraX;
extern int cameraY;

// =====================================================
// 基本世界設定
// =====================================================
static const int DUCK_TILE_SIZE = 16;
static const int DUCK_MAP_COLS  = 40;
static const int DUCK_MAP_ROWS  = 30;

static const int DUCK_WORLD_W = 640;
static const int DUCK_WORLD_H = 480;

static const int DUCK_SCREEN_W = 64;
static const int DUCK_SCREEN_H = 64;

// =====================================================
// 大鴨 Sprite 設定
// 0~5   朝下
// 6~11  朝上
// 12~17 朝左
// 朝右 = 朝左翻轉
// =====================================================
static const int DUCK_SPRITE_W = 32;
static const int DUCK_SPRITE_H = 31;
static const int DUCK_TOTAL_FRAMES = 18;
static const int DUCK_SHEET_W = 576;

// =====================================================
// 小鴨 Sprite 設定
// 19x19，總寬 342
// =====================================================
static const int DUCKLING_SPRITE_W = 19;
static const int DUCKLING_SPRITE_H = 19;
static const int DUCKLING_TOTAL_FRAMES = 18;
static const int DUCKLING_SHEET_W  = 342;

// =====================================================
// DUCK_OBJ 新寬度
// 83~86 對應第 28~31 張（0-based = 27~30）
// =====================================================
static const int DUCK_OBJ_SHEET_W = 528;
static const int DUCK_HOME_SHEET_W = 608;

static const uint16_t DUCK_TRANSPARENT_COLOR = 0x33D8;


// =====================================================
// DUCK_FISH 設定
// =====================================================
static const int FISH_COUNT = 20;
static const int FISH_W = 9;
static const int FISH_H = 19;
static const int FISH_TOTAL_FRAMES = 8;
static const int FISH_SHEET_W = 72; // 9 * 8

struct FishState {
  float x, y;
  int dir;           // 0:下, 1:上, 2:左, 3:右
  int animFrame;
  unsigned long lastAnimMs;
  unsigned long nextActionMs;
  bool active;
};

FishState fishes[FISH_COUNT];




// =====================================================
// 碰撞盒設定
// =====================================================
static const int DUCK_HITBOX_W = 20;
static const int DUCK_HITBOX_H = 24;
static const int DUCK_HITBOX_OFFSET_X = 6;
static const int DUCK_HITBOX_OFFSET_Y = 3;

static const int DUCKLING_HITBOX_W = 12;
static const int DUCKLING_HITBOX_H = 16;
static const int DUCKLING_HITBOX_OFFSET_X = 4;
static const int DUCKLING_HITBOX_OFFSET_Y = 1;

// =====================================================
// 速度 / AI / 動畫參數
// =====================================================
static const float DUCK_SPEED = 0.8f;
static const float DUCKLING_WANDER_SPEED = 0.35f;
static const float DUCKLING_FOLLOW_SPEED = 1.0f;

static const unsigned long DUCK_ANIM_INTERVAL = 150;
static const unsigned long DUCK_AI_MIN_MS = 2000;
static const unsigned long DUCK_AI_MAX_MS = 5000;
static const unsigned long DUCK_IDLE_PICK_INTERVAL  = 2500;
static const unsigned long DUCK_SLEEP_PICK_INTERVAL = 2500;

// 小鴨平常遊走 / 停住時間
static const unsigned long DUCKLING_AI_MIN_MS = 1200;
static const unsigned long DUCKLING_AI_MAX_MS = 3500;

// BFS 路徑重算間隔
static const unsigned long DUCK_PATH_RECALC_MS = 800;

// 主角切換冷卻：1 分鐘
static const unsigned long DUCK_SWITCH_COOLDOWN_MS = 60000;

// =====================================================
// 數量設定
// ducks[0] = DUCK_A（預設母鴨）
// ducks[1] = DUCK_B（公鴨）
// =====================================================
static const int DUCK_COUNT = 2;
static const int DUCKLING_MAX = 10;

// =====================================================
// 狀態定義
// 大鴨：
// 0 = 下
// 1 = 上
// 2 = 左
// 3 = 右
// 4 = 待機
// 5 = 睡覺
// 6 = 帶小鴨回巢中
//
// 小鴨：
// 0 = 下
// 1 = 上
// 2 = 左
// 3 = 右
// 4 = 停住
// =====================================================
struct DuckActor {
  const uint8_t* sprite;
  const uint16_t* palette;
  const char* name;

  float x;
  float y;
  float lastX;
  float lastY;

  int state;
  int moveFrame;
  int stillFrame;

  unsigned long lastAnimMs;
  unsigned long nextActionMs;
  unsigned long lastStillPickMs;
};

struct DucklingActor {
  bool active;
  bool following;
  bool rescued;

  float x;
  float y;
  float lastX;
  float lastY;

  int dir;
  int moveFrame;
  int stillFrame;

  unsigned long lastAnimMs;
  unsigned long nextActionMs;
};

// =====================================================
// 全域資料
// =====================================================
static const uint8_t* duckSpriteSheets[DUCK_COUNT] = {
  Duck::DUCK_A,
  Duck::DUCK_B
};

static const uint16_t* duckSpritePalettes[DUCK_COUNT] = {
  Duck::DUCK_A_PALETTE,
  Duck::DUCK_B_PALETTE
};

static const char* duckNames[DUCK_COUNT] = {
  "A",
  "B"
};

static DuckActor ducks[DUCK_COUNT];
static DucklingActor ducklings[DUCKLING_MAX];

static int duckLeaderIndex = 0;

static int duckCameraTarget = 0;
static int duckCameraX = 0;
static int duckCameraY = 0;

static bool duckLeaderReturningNest = false;
static int duckCarryingCount = 0;
static unsigned long duckLastSwitchMs = 0;


static int duckReturnBlockedCount = 0;      // 連續卡住次數
static int duckEscapeDir = 0;               // 0=無, 1=左, 2=右, 3=上, 4=下
static int duckEscapeStepsLeft = 0;
static int duckLastSuccessfulSideDir = 0;   // 記住最近一次成功的副方向
static const int DUCK_ESCAPE_MAX_STEPS = 4; // 每次脫困最多偏移幾步

static const int DUCK_RETURN_BLOCKED_LIMIT = 8;


static unsigned long duckReturnStuckStartMs = 0;
static float duckReturnProgressX = 0.0f;
static float duckReturnProgressY = 0.0f;

// =====================================================
// 回巢路徑資料
// =====================================================
static int16_t duckPathX[DUCK_MAP_COLS * DUCK_MAP_ROWS];
static int16_t duckPathY[DUCK_MAP_COLS * DUCK_MAP_ROWS];
static int duckPathLen = 0;
static int duckPathIndex = 0;
static unsigned long duckLastPathCalcMs = 0;

// BFS 工作陣列
static bool duckVisited[DUCK_MAP_ROWS][DUCK_MAP_COLS];
static int16_t duckPrevX[DUCK_MAP_ROWS][DUCK_MAP_COLS];
static int16_t duckPrevY[DUCK_MAP_ROWS][DUCK_MAP_COLS];
static int16_t duckQueueX[DUCK_MAP_COLS * DUCK_MAP_ROWS];
static int16_t duckQueueY[DUCK_MAP_COLS * DUCK_MAP_ROWS];
static int16_t duckRevPathX[DUCK_MAP_COLS * DUCK_MAP_ROWS];
static int16_t duckRevPathY[DUCK_MAP_COLS * DUCK_MAP_ROWS];

// =====================================================
// DEBUG
// =====================================================
static bool DUCK_DEBUG_ENABLE = false ;
static unsigned long duckDebugLastHeartbeatMs = 0;
static const unsigned long DUCK_DEBUG_HEARTBEAT_MS = 10000;

// =====================================================
// 小工具
// =====================================================
static inline int duck_clamp_i(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static inline float duck_clamp_f(float v, float lo, float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static inline bool duckRectOverlap(
  int ax, int ay, int aw, int ah,
  int bx, int by, int bw, int bh
) {
  if (ax + aw <= bx) return false;
  if (bx + bw <= ax) return false;
  if (ay + ah <= by) return false;
  if (by + bh <= ay) return false;
  return true;
}

static const char* getDuckStateName(int s) {
  switch (s) {
    case 0: return "下";
    case 1: return "上";
    case 2: return "左";
    case 3: return "右";
    case 4: return "待機";
    case 5: return "睡覺";
    case 6: return "回巢";
    default: return "未知";
  }
}





static int scoreLeaderEscapeDirection(int dir, int steps) {
  float tx = ducks[duckLeaderIndex].x;
  float ty = ducks[duckLeaderIndex].y;
  int score = 0;

  for (int i = 0; i < steps; i++) {
    if (dir == 1) tx -= DUCK_SPEED; // 左
    if (dir == 2) tx += DUCK_SPEED; // 右
    if (dir == 3) ty -= DUCK_SPEED; // 上
    if (dir == 4) ty += DUCK_SPEED; // 下

    if (checkDuckCollision(tx, ty)) break;
    if (findCollidingDuckIndex(duckLeaderIndex, tx, ty) >= 0) break;

    score++;
  }

  return score;
}






// =====================================================
// 地圖資料讀取
// DUCK_MAP 規則：
// 0 = 可走地面
// 1 = 障礙物
// 2 = 巢穴交付區
// =====================================================
static uint8_t getDuckCollisionTile(int tileX, int tileY) {
  if (tileX < 0 || tileX >= DUCK_MAP_COLS || tileY < 0 || tileY >= DUCK_MAP_ROWS) {
    return 1;
  }
  return pgm_read_byte(&(Duck::DUCK_MAP[tileY][tileX]));
}

static uint8_t getDuckBgTile(int tileX, int tileY) {
  if (tileX < 0 || tileX >= DUCK_MAP_COLS || tileY < 0 || tileY >= DUCK_MAP_ROWS) {
    return 0;
  }
  return pgm_read_byte(&(Duck::DUCK_BG_MAP[tileY][tileX]));
}



// =====================================================
// 用 DUCK_MAP == 2 作為回巢 / 交付判定
// =====================================================
static bool isNestZoneTile(int tx, int ty) {
  if (tx < 0 || tx >= DUCK_MAP_COLS || ty < 0 || ty >= DUCK_MAP_ROWS) return false;
  return getDuckCollisionTile(tx, ty) == 2;
}

static bool isPointInNest(int px, int py) {
  int tx = px / DUCK_TILE_SIZE;
  int ty = py / DUCK_TILE_SIZE;
  return isNestZoneTile(tx, ty);
}

static bool isTileWalkableForDuck(int tx, int ty) {
  if (tx < 0 || tx >= DUCK_MAP_COLS || ty < 0 || ty >= DUCK_MAP_ROWS) return false;
  uint8_t c = getDuckCollisionTile(tx, ty);
  return (c == 0 || c == 2);
}

static bool isLeaderDuckInNest() {
  int px = (int)ducks[duckLeaderIndex].x + DUCK_HITBOX_OFFSET_X + (DUCK_HITBOX_W / 2);
  int py = (int)ducks[duckLeaderIndex].y + DUCK_HITBOX_OFFSET_Y + DUCK_HITBOX_H - 1;
  return isPointInNest(px, py);
}


static bool tryMoveLeaderDuck(float nx, float ny) {
  if (checkDuckCollision(nx, ny)) return false;
  if (findCollidingDuckIndex(duckLeaderIndex, nx, ny) >= 0) return false;

  ducks[duckLeaderIndex].x = duck_clamp_f(nx, 0.0f, (float)(DUCK_WORLD_W - DUCK_SPRITE_W));
  ducks[duckLeaderIndex].y = duck_clamp_f(ny, 0.0f, (float)(DUCK_WORLD_H - DUCK_SPRITE_H));
  return true;
}


// =====================================================
// DEBUG 輸出
// =====================================================
static void debugPrintLeaderCore(const char* tag) {
  if (!DUCK_DEBUG_ENABLE) return;

  DuckActor &d = ducks[duckLeaderIndex];

  Serial.print("[");
  Serial.print(tag);
  Serial.print("] 主角=");
  Serial.print(duckLeaderIndex);
  Serial.print(" 座標=(");
  Serial.print(d.x, 1);
  Serial.print(",");
  Serial.print(d.y, 1);
  Serial.print(")");

  Serial.print(" B座標=(");
  Serial.print(ducks[1].x, 1);
  Serial.print(",");
  Serial.print(ducks[1].y, 1);
  Serial.print(")");


  
  Serial.print(" 狀態=");
  Serial.print(getDuckStateName(d.state));
  Serial.print(" 帶小鴨=");
  Serial.print(duckCarryingCount);
  Serial.print(" 在巢區=");
  Serial.print(isLeaderDuckInNest() ? "是" : "否");
  Serial.print(" 路徑=");
  Serial.print(duckPathIndex);
  Serial.print("/");
  Serial.print(duckPathLen);

  if (duckPathLen > 0 && duckPathIndex < duckPathLen) {
    Serial.print(" 目標格=(");
    Serial.print(duckPathX[duckPathIndex]);
    Serial.print(",");
    Serial.print(duckPathY[duckPathIndex]);
    Serial.print(")");
  }

  Serial.println();
}

static void debugPrintTileInfo(const char* tag, int tx, int ty) {
  if (!DUCK_DEBUG_ENABLE) return;

  Serial.print("[");
  Serial.print(tag);
  Serial.print("] tile=(");
  Serial.print(tx);
  Serial.print(",");
  Serial.print(ty);
  Serial.print(")");

  if (tx >= 0 && tx < DUCK_MAP_COLS && ty >= 0 && ty < DUCK_MAP_ROWS) {
    Serial.print(" 背景=");
    Serial.print(getDuckBgTile(tx, ty));
    Serial.print(" 碰撞=");
    Serial.print(getDuckCollisionTile(tx, ty));
    Serial.print(" 巢區=");
    Serial.print(isNestZoneTile(tx, ty) ? "是" : "否");
  } else {
    Serial.print(" 超出範圍");
  }

  Serial.println();
}



static void debugLeaderHeartbeat() {
  if (!DUCK_DEBUG_ENABLE) return;

  unsigned long nowMs = millis();
  if (nowMs - duckDebugLastHeartbeatMs < DUCK_DEBUG_HEARTBEAT_MS) return;
  duckDebugLastHeartbeatMs = nowMs;

  debugPrintLeaderCore("心跳");
}

// =====================================================
// 碰撞判定
// 只有 DUCK_MAP == 1 才算阻擋
// DUCK_MAP == 2 是巢區，可站
// =====================================================
static bool checkDuckCollision(float spriteX, float spriteY) {
  int hitX = (int)spriteX + DUCK_HITBOX_OFFSET_X;
  int hitY = (int)spriteY + DUCK_HITBOX_OFFSET_Y;

  int px[4] = {
    hitX,
    hitX + DUCK_HITBOX_W - 1,
    hitX,
    hitX + DUCK_HITBOX_W - 1
  };

  int py[4] = {
    hitY,
    hitY,
    hitY + DUCK_HITBOX_H - 1,
    hitY + DUCK_HITBOX_H - 1
  };

  for (int i = 0; i < 4; i++) {
    int tileX = px[i] / DUCK_TILE_SIZE;
    int tileY = py[i] / DUCK_TILE_SIZE;
    if (getDuckCollisionTile(tileX, tileY) == 1) {
      return true;
    }
  }
  return false;
}

static bool checkDucklingCollision(float spriteX, float spriteY) {
  int hitX = (int)spriteX + DUCKLING_HITBOX_OFFSET_X;
  int hitY = (int)spriteY + DUCKLING_HITBOX_OFFSET_Y;

  int px[4] = {
    hitX,
    hitX + DUCKLING_HITBOX_W - 1,
    hitX,
    hitX + DUCKLING_HITBOX_W - 1
  };

  int py[4] = {
    hitY,
    hitY,
    hitY + DUCKLING_HITBOX_H - 1,
    hitY + DUCKLING_HITBOX_H - 1
  };

  for (int i = 0; i < 4; i++) {
    int tileX = px[i] / DUCK_TILE_SIZE;
    int tileY = py[i] / DUCK_TILE_SIZE;
    if (getDuckCollisionTile(tileX, tileY) == 1) {
      return true;
    }
  }
  return false;
}

static bool checkDuckVsDuckCollision(float ax, float ay, float bx, float by) {
  int aHitX = (int)ax + DUCK_HITBOX_OFFSET_X;
  int aHitY = (int)ay + DUCK_HITBOX_OFFSET_Y;
  int bHitX = (int)bx + DUCK_HITBOX_OFFSET_X;
  int bHitY = (int)by + DUCK_HITBOX_OFFSET_Y;

  return duckRectOverlap(
    aHitX, aHitY, DUCK_HITBOX_W, DUCK_HITBOX_H,
    bHitX, bHitY, DUCK_HITBOX_W, DUCK_HITBOX_H
  );
}

static bool checkLeaderDuckVsDucklingCollision(int ducklingIndex) {
  int aHitX = (int)ducks[duckLeaderIndex].x + DUCK_HITBOX_OFFSET_X;
  int aHitY = (int)ducks[duckLeaderIndex].y + DUCK_HITBOX_OFFSET_Y;
  int bHitX = (int)ducklings[ducklingIndex].x + DUCKLING_HITBOX_OFFSET_X;
  int bHitY = (int)ducklings[ducklingIndex].y + DUCKLING_HITBOX_OFFSET_Y;

  return duckRectOverlap(
    aHitX, aHitY, DUCK_HITBOX_W, DUCK_HITBOX_H,
    bHitX, bHitY, DUCKLING_HITBOX_W, DUCKLING_HITBOX_H
  );
}

// =====================================================
// 背景取圖
// 83~86 對應 DUCK_OBJ 第 28~31 張圖
// =====================================================

//palette 讀色小工具
static inline uint16_t readDuckPalettePixel(
  const uint8_t* sheet,
  const uint16_t* palette,
  uint32_t pixelPos
) {
  uint8_t colorIndex = pgm_read_byte(&(sheet[pixelPos]));
  return pgm_read_word(&(palette[colorIndex]));
}


static uint16_t getDuckTilePixel(uint8_t tileId, int offsetX, int offsetY) {
  if (tileId == 0 || tileId > 88) return DUCK_TRANSPARENT_COLOR;

  int frameIdx = 0;
  bool flipH = false;

  // DB1~DB6 / 83~88，使用 DUCK_OBJ 第 27~32 張
  if (tileId >= 83 && tileId <= 88) {
    frameIdx = tileId - 56;

    int px = (frameIdx * 16) + offsetX;
    uint32_t pixelPos =
      (uint32_t)offsetY * (uint32_t)DUCK_OBJ_SHEET_W +
      (uint32_t)px;

    return readDuckPalettePixel(
      Duck::DUCK_OBJ,
      Duck::DUCK_OBJ_PALETTE,
      pixelPos
    );
  }

  // D01~D44，使用 DUCK_OBJ
  if (tileId <= 44) {
    if (tileId >= 1 && tileId <= 21) {
      frameIdx = tileId - 1;
      flipH = false;
    }
    else if (tileId >= 22 && tileId <= 38) {
      frameIdx = tileId - 22;
      flipH = true;
    }
    else {
      frameIdx = tileId - 18;
      flipH = false;
    }

    int actualX = flipH ? (15 - offsetX) : offsetX;
    int px = (frameIdx * 16) + actualX;

    uint32_t pixelPos =
      (uint32_t)offsetY * (uint32_t)DUCK_OBJ_SHEET_W +
      (uint32_t)px;

    return readDuckPalettePixel(
      Duck::DUCK_OBJ,
      Duck::DUCK_OBJ_PALETTE,
      pixelPos
    );
  }

  // D45~D82，使用 DUCK_HOME
  frameIdx = tileId - 45;
  int px = (frameIdx * 16) + offsetX;

  uint32_t pixelPos =
    (uint32_t)offsetY * (uint32_t)DUCK_HOME_SHEET_W +
    (uint32_t)px;

  return readDuckPalettePixel(
    Duck::DUCK_HOME,
    Duck::DUCK_HOME_PALETTE,
    pixelPos
  );
}

// =====================================================
// 繪圖
// =====================================================
static void drawDuckBackgroundCrop() {
  for (int sy = 0; sy < DUCK_SCREEN_H; sy++) {
    int wy = cameraY + sy;
    if (wy < 0 || wy >= DUCK_WORLD_H) continue;

    int tileY = wy / DUCK_TILE_SIZE;
    int offsetY = wy % DUCK_TILE_SIZE;

    for (int sx = 0; sx < DUCK_SCREEN_W; sx++) {
      int wx = cameraX + sx;
      if (wx < 0 || wx >= DUCK_WORLD_W) continue;

      int tileX = wx / DUCK_TILE_SIZE;
      int offsetX = wx % DUCK_TILE_SIZE;

      uint8_t tileId = getDuckBgTile(tileX, tileY);
      uint16_t color = getDuckTilePixel(tileId, offsetX, offsetY);

      if (color != DUCK_TRANSPARENT_COLOR) {
        display.drawPixel(sx, sy, color);
      }
    }
  }
}

static void pickDuckStillFrameByIndex(int duckIndex) {
  if (ducks[duckIndex].state == 4) {
    int pick = random(0, 4);
    if (pick == 0) ducks[duckIndex].stillFrame = 0;
    else if (pick == 1) ducks[duckIndex].stillFrame = 6;
    else ducks[duckIndex].stillFrame = 12;
  }
  else if (ducks[duckIndex].state == 5) {
    ducks[duckIndex].stillFrame = 0;
  }
}

static int getDuckFrameByIndex(int duckIndex, bool &mirrorX) {
  mirrorX = false;

  if (ducks[duckIndex].state == 0) return 0 + ducks[duckIndex].moveFrame;
  if (ducks[duckIndex].state == 1) return 6 + ducks[duckIndex].moveFrame;
  if (ducks[duckIndex].state == 2) return 12 + ducks[duckIndex].moveFrame;
  if (ducks[duckIndex].state == 3) {
    mirrorX = true;
    return 12 + ducks[duckIndex].moveFrame;
  }

  if (ducks[duckIndex].state == 4 || ducks[duckIndex].state == 5) {
    return ducks[duckIndex].stillFrame;
  }

  if (ducks[duckIndex].lastX < ducks[duckIndex].x) {
    mirrorX = true;
    return 12 + ducks[duckIndex].moveFrame;
  }
  else if (ducks[duckIndex].lastX > ducks[duckIndex].x) {
    return 12 + ducks[duckIndex].moveFrame;
  }
  else if (ducks[duckIndex].lastY < ducks[duckIndex].y) {
    return 0 + ducks[duckIndex].moveFrame;
  }
  return 6 + ducks[duckIndex].moveFrame;
}

static int getDucklingFrameByIndex(int i, bool &mirrorX) {
  mirrorX = false;

  if (ducklings[i].dir == 0) return 0 + ducklings[i].moveFrame;
  if (ducklings[i].dir == 1) return 6 + ducklings[i].moveFrame;
  if (ducklings[i].dir == 2) return 12 + ducklings[i].moveFrame;
  if (ducklings[i].dir == 3) {
    mirrorX = true;
    return 12 + ducklings[i].moveFrame;
  }

  return 0;
}

static void drawDuckSprite(
  const uint8_t* spriteSheet,
  const uint16_t* palette,
  int screenX,
  int screenY,
  int frameIndex,
  bool mirrorX
) {
  if (frameIndex < 0 || frameIndex >= DUCK_TOTAL_FRAMES) return;

  int frameStartX = frameIndex * DUCK_SPRITE_W;

  // 每幀左上角像素當透明索引
  uint8_t transparentIndex = pgm_read_byte(&(spriteSheet[frameStartX]));

  for (int j = 0; j < DUCK_SPRITE_H; j++) {
    int drawY = screenY + j;
    if (drawY < 0 || drawY >= DUCK_SCREEN_H) continue;

    for (int i = 0; i < DUCK_SPRITE_W; i++) {
      int srcX = mirrorX ? (DUCK_SPRITE_W - 1 - i) : i;
      int drawX = screenX + i;
      if (drawX < 0 || drawX >= DUCK_SCREEN_W) continue;

      uint32_t pixelPos =
        (uint32_t)j * (uint32_t)DUCK_SHEET_W +
        (uint32_t)frameStartX +
        (uint32_t)srcX;

      uint8_t colorIndex = pgm_read_byte(&(spriteSheet[pixelPos]));

      if (colorIndex == transparentIndex) continue;

      uint16_t color = pgm_read_word(&(palette[colorIndex]));
      display.drawPixel(drawX, drawY, color);
    }
  }
}


static void drawDucklingSprite(
  int screenX,
  int screenY,
  int frameIndex,
  bool mirrorX
) {
  if (frameIndex < 0 || frameIndex >= DUCKLING_TOTAL_FRAMES) return;

  const uint8_t* spriteSheet = Duck::DUCK_S;
  const uint16_t* palette = Duck::DUCK_S_PALETTE;

  int frameStartX = frameIndex * DUCKLING_SPRITE_W;

  // 每幀左上角像素當透明索引
  uint8_t transparentIndex = pgm_read_byte(&(spriteSheet[frameStartX]));

  for (int j = 0; j < DUCKLING_SPRITE_H; j++) {
    int drawY = screenY + j;
    if (drawY < 0 || drawY >= DUCK_SCREEN_H) continue;

    for (int i = 0; i < DUCKLING_SPRITE_W; i++) {
      int srcX = mirrorX ? (DUCKLING_SPRITE_W - 1 - i) : i;
      int drawX = screenX + i;
      if (drawX < 0 || drawX >= DUCK_SCREEN_W) continue;

      uint32_t pixelPos =
        (uint32_t)j * (uint32_t)DUCKLING_SHEET_W +
        (uint32_t)frameStartX +
        (uint32_t)srcX;

      uint8_t colorIndex = pgm_read_byte(&(spriteSheet[pixelPos]));

      if (colorIndex == transparentIndex) continue;

      uint16_t color = pgm_read_word(&(palette[colorIndex]));
      display.drawPixel(drawX, drawY, color);
    }
  }
}


static void drawOneDuckByIndex(int duckIndex) {
  bool mirrorX = false;
  int frameIndex = getDuckFrameByIndex(duckIndex, mirrorX);

  int screenX = (int)ducks[duckIndex].x - duckCameraX;
  int screenY = (int)ducks[duckIndex].y - duckCameraY;

  if (screenX <= -DUCK_SPRITE_W || screenX >= DUCK_SCREEN_W ||
      screenY <= -DUCK_SPRITE_H || screenY >= DUCK_SCREEN_H) {
    return;
  }

  drawDuckSprite(
    ducks[duckIndex].sprite,
    ducks[duckIndex].palette,
    screenX,
    screenY,
    frameIndex,
    mirrorX
  );
}



static void drawOneDucklingByIndex(int i) {
  if (!ducklings[i].active) return;

  bool mirrorX = false;
  int frameIndex = getDucklingFrameByIndex(i, mirrorX);

  int screenX = (int)ducklings[i].x - duckCameraX;
  int screenY = (int)ducklings[i].y - duckCameraY;

  if (screenX <= -DUCKLING_SPRITE_W || screenX >= DUCK_SCREEN_W ||
      screenY <= -DUCKLING_SPRITE_H || screenY >= DUCK_SCREEN_H) {
    return;
  }

  drawDucklingSprite(screenX, screenY, frameIndex, mirrorX);
}

static void renderDuckFullFrame() {
  cameraX = duckCameraX;
  cameraY = duckCameraY;

  drawDuckBackgroundCrop();

  for (int i = 0; i < FISH_COUNT; i++) {
    if (fishes[i].active) {
      drawOneFish(i);
    }
  }


  for (int i = 0; i < DUCKLING_MAX; i++) {
    drawOneDucklingByIndex(i);
  }

  for (int i = 0; i < DUCK_COUNT; i++) {
    drawOneDuckByIndex(i);
  }

  drawThemeClockText();
}

// =====================================================
// 動畫更新
// =====================================================
static void updateDuckAnimationByIndex(int duckIndex, unsigned long nowMs) {
  if (ducks[duckIndex].state >= 0 && ducks[duckIndex].state <= 3) {
    if (nowMs - ducks[duckIndex].lastAnimMs >= DUCK_ANIM_INTERVAL) {
      ducks[duckIndex].lastAnimMs = nowMs;
      ducks[duckIndex].moveFrame++;
      if (ducks[duckIndex].moveFrame >= 6) ducks[duckIndex].moveFrame = 0;
    }
    return;
  }

  if (ducks[duckIndex].state == 6) {
    if (nowMs - ducks[duckIndex].lastAnimMs >= DUCK_ANIM_INTERVAL) {
      ducks[duckIndex].lastAnimMs = nowMs;
      ducks[duckIndex].moveFrame++;
      if (ducks[duckIndex].moveFrame >= 6) ducks[duckIndex].moveFrame = 0;
    }
    return;
  }

  if (ducks[duckIndex].state == 4) {
    if (nowMs - ducks[duckIndex].lastStillPickMs >= DUCK_IDLE_PICK_INTERVAL) {
      ducks[duckIndex].lastStillPickMs = nowMs;
      pickDuckStillFrameByIndex(duckIndex);
    }
  }
  else if (ducks[duckIndex].state == 5) {
    if (nowMs - ducks[duckIndex].lastStillPickMs >= DUCK_SLEEP_PICK_INTERVAL) {
      ducks[duckIndex].lastStillPickMs = nowMs;
      pickDuckStillFrameByIndex(duckIndex);
    }
  }
}

static void updateDucklingAnimationByIndex(int i, unsigned long nowMs) {
  if (!ducklings[i].active) return;

  if (nowMs - ducklings[i].lastAnimMs >= DUCK_ANIM_INTERVAL) {
    ducklings[i].lastAnimMs = nowMs;
    ducklings[i].moveFrame++;
    if (ducklings[i].moveFrame >= 6) ducklings[i].moveFrame = 0;
  }
}

// =====================================================
// AI
// =====================================================
static void updateDuckAIByIndex(int duckIndex, unsigned long nowMs) {
  if (duckIndex == duckLeaderIndex && duckCarryingCount > 0) {
    ducks[duckIndex].state = 6;
    duckLeaderReturningNest = true;
    return;
  }

  if (nowMs < ducks[duckIndex].nextActionMs) return;

  int dice = random(0, 100);

  if (dice < 10) {
    ducks[duckIndex].state = 4;
    pickDuckStillFrameByIndex(duckIndex);
    ducks[duckIndex].lastStillPickMs = nowMs;
  }
  else if (dice < 90) {
    ducks[duckIndex].state = random(0, 4);
    ducks[duckIndex].moveFrame = 0;   
  }
  else {
    ducks[duckIndex].state = 5;
    pickDuckStillFrameByIndex(duckIndex);
    ducks[duckIndex].lastStillPickMs = nowMs;
  }

  ducks[duckIndex].nextActionMs = nowMs + random(DUCK_AI_MIN_MS, DUCK_AI_MAX_MS + 1);
}

static void updateDucklingAIByIndex(int i, unsigned long nowMs) {
  if (!ducklings[i].active) return;
  if (ducklings[i].following) return;

  if (nowMs < ducklings[i].nextActionMs) return;

  int dice = random(0, 100);

  if (dice < 70) {
    ducklings[i].dir = random(0, 4);
  } else {
    ducklings[i].dir = 4;
  }

  ducklings[i].nextActionMs = nowMs + random(DUCKLING_AI_MIN_MS, DUCKLING_AI_MAX_MS + 1);
}

// =====================================================
// 尋找與大鴨碰撞
// =====================================================
static int findCollidingDuckIndex(int ignoreIndex, float nx, float ny) {
  for (int i = 0; i < DUCK_COUNT; i++) {
    if (i == ignoreIndex) continue;
    if (checkDuckVsDuckCollision(nx, ny, ducks[i].x, ducks[i].y)) {
      return i;
    }
  }
  return -1;
}

// =====================================================
// 主角腳底 tile
// =====================================================
static void getLeaderDuckFootTile(int &tx, int &ty) {
  int px = (int)ducks[duckLeaderIndex].x + DUCK_HITBOX_OFFSET_X + (DUCK_HITBOX_W / 2);
  int py = (int)ducks[duckLeaderIndex].y + DUCK_HITBOX_OFFSET_Y + DUCK_HITBOX_H - 1;
  tx = px / DUCK_TILE_SIZE;
  ty = py / DUCK_TILE_SIZE;
}


static float duck_approach_f(float current, float target, float step) {
  if (current < target) {
    current += step;
    if (current > target) current = target;
  } else if (current > target) {
    current -= step;
    if (current < target) current = target;
  }
  return current;
}



static bool canLeaderStandOnFootTile(int tx, int ty) {
  if (tx < 0 || tx >= DUCK_MAP_COLS || ty < 0 || ty >= DUCK_MAP_ROWS) return false;

  // 只有地面(0)或巢區(2)可以站
  uint8_t c = getDuckCollisionTile(tx, ty);
  if (!(c == 0 || c == 2)) return false;

  // 以「腳底中心落在該 tile 中心」反推主角左上角
  float testX = (float)(tx * DUCK_TILE_SIZE + 8 - (DUCK_HITBOX_OFFSET_X + (DUCK_HITBOX_W / 2)));
  float testY = (float)(ty * DUCK_TILE_SIZE + 8 - (DUCK_HITBOX_OFFSET_Y + DUCK_HITBOX_H - 1));

  // 邊界保護
  testX = duck_clamp_f(testX, 0.0f, (float)(DUCK_WORLD_W - DUCK_SPRITE_W));
  testY = duck_clamp_f(testY, 0.0f, (float)(DUCK_WORLD_H - DUCK_SPRITE_H));

  // 真正檢查主角整個 hitbox 能不能站進去
  if (checkDuckCollision(testX, testY)) return false;

  return true;
}



// =====================================================
// BFS 找到 DUCK_MAP == 2 的巢區
// =====================================================
static void buildDuckPathToNest() {
  duckPathLen = 0;
  duckPathIndex = 0;

  if (DUCK_DEBUG_ENABLE) {
    Serial.println("[路徑計算] 開始計算主角回巢路徑");
    debugPrintLeaderCore("路徑計算");
  }

  int startX, startY;
  getLeaderDuckFootTile(startX, startY);

  if (DUCK_DEBUG_ENABLE) {
    Serial.print("[起點格] tile=(");
    Serial.print(startX);
    Serial.print(",");
    Serial.print(startY);

    Serial.print(" 碰撞=");
    Serial.print(getDuckCollisionTile(startX, startY));
    Serial.print(" 巢區=");
    Serial.println(isNestZoneTile(startX, startY) ? "是" : "否");

    Serial.print("[起點可站立] ");
    Serial.println(canLeaderStandOnFootTile(startX, startY) ? "是" : "否");
  }

  if (startX < 0 || startX >= DUCK_MAP_COLS || startY < 0 || startY >= DUCK_MAP_ROWS) {
    if (DUCK_DEBUG_ENABLE) {
      Serial.println("[路徑失敗] 起點超出地圖範圍");
    }
    return;
  }

  // 如果起點本身就在巢區內，就不用找路
  if (isNestZoneTile(startX, startY)) {
    if (DUCK_DEBUG_ENABLE) {
      Serial.println("[路徑計算] 起點已在巢區內，不需尋路");
    }
    return;
  }

  // BFS 用陣列
  static bool visited[DUCK_MAP_ROWS][DUCK_MAP_COLS];
  static int parentX[DUCK_MAP_ROWS][DUCK_MAP_COLS];
  static int parentY[DUCK_MAP_ROWS][DUCK_MAP_COLS];
  static int queueX[DUCK_MAP_ROWS * DUCK_MAP_COLS];
  static int queueY[DUCK_MAP_ROWS * DUCK_MAP_COLS];

  for (int y = 0; y < DUCK_MAP_ROWS; y++) {
    for (int x = 0; x < DUCK_MAP_COLS; x++) {
      visited[y][x] = false;
      parentX[y][x] = -1;
      parentY[y][x] = -1;
    }
  }

  int qHead = 0;
  int qTail = 0;

  visited[startY][startX] = true;
  queueX[qTail] = startX;
  queueY[qTail] = startY;
  qTail++;

  int foundX = -1;
  int foundY = -1;

  const int dirX[4] = { -1, 1, 0, 0 };
  const int dirY[4] = { 0, 0, -1, 1 };

  while (qHead < qTail) {
    int cx = queueX[qHead];
    int cy = queueY[qHead];
    qHead++;

    if (isNestZoneTile(cx, cy)) {
      foundX = cx;
      foundY = cy;

      if (DUCK_DEBUG_ENABLE) {
        Serial.print("[找到巢區格] tile=(");
        Serial.print(foundX);
        Serial.print(",");
        Serial.print(foundY);

        Serial.print(" 碰撞=");
        Serial.print(getDuckCollisionTile(foundX, foundY));
        Serial.print(" 巢區=");
        Serial.println("是");
      }
      break;
    }

    for (int i = 0; i < 4; i++) {
      int nx = cx + dirX[i];
      int ny = cy + dirY[i];

      if (nx < 0 || nx >= DUCK_MAP_COLS || ny < 0 || ny >= DUCK_MAP_ROWS) continue;
      if (visited[ny][nx]) continue;

      // 關鍵修正：不是只看 tile 不是牆，而是看主角整個 hitbox 能不能站進去
      if (!canLeaderStandOnFootTile(nx, ny)) continue;

      visited[ny][nx] = true;
      parentX[ny][nx] = cx;
      parentY[ny][nx] = cy;

      queueX[qTail] = nx;
      queueY[qTail] = ny;
      qTail++;
    }
  }

  if (foundX < 0 || foundY < 0) {
    if (DUCK_DEBUG_ENABLE) {
      Serial.println("[路徑失敗] 找不到可到達的巢區");
    }
    return;
  }

  // 回溯路徑
const int pathCap = sizeof(duckPathX) / sizeof(duckPathX[0]);
static int revX[sizeof(duckPathX) / sizeof(duckPathX[0])];
static int revY[sizeof(duckPathY) / sizeof(duckPathY[0])];
int revLen = 0;

  int px = foundX;
  int py = foundY;

  while (!(px == startX && py == startY)) {
if (revLen >= pathCap) {
      if (DUCK_DEBUG_ENABLE) {
        Serial.println("[路徑失敗] 路徑長度超過上限");
      }
      duckPathLen = 0;
      duckPathIndex = 0;
      return;
    }

    revX[revLen] = px;
    revY[revLen] = py;
    revLen++;

    int tx = parentX[py][px];
    int ty = parentY[py][px];

    if (tx < 0 || ty < 0) {
      if (DUCK_DEBUG_ENABLE) {
        Serial.println("[路徑失敗] 回溯 parent 斷裂");
      }
      duckPathLen = 0;
      duckPathIndex = 0;
      return;
    }

    px = tx;
    py = ty;
  }

  // 反轉成正向路徑
  duckPathLen = 0;
  for (int i = revLen - 1; i >= 0; i--) {
    if (duckPathLen >= pathCap) break;
    duckPathX[duckPathLen] = revX[i];
    duckPathY[duckPathLen] = revY[i];
    duckPathLen++;
  }

  duckPathIndex = 0;

  if (DUCK_DEBUG_ENABLE) {
    Serial.print("[路徑成功] 路徑長度=");
    Serial.println(duckPathLen);

    if (duckPathLen > 0) {
      Serial.print("[第一個目標] 目標格=(");
      Serial.print(duckPathX[0]);
      Serial.print(",");
      Serial.print(duckPathY[0]);
      Serial.println(")");
    }
  }
}

// =====================================================
// 主角帶小鴨回巢
// =====================================================
static void updateLeaderDuckReturnToNest() {
  DuckActor &d = ducks[duckLeaderIndex];
  unsigned long nowMs = millis();

  // 已進巢區，等待交付
  if (isLeaderDuckInNest()) {
    if (DUCK_DEBUG_ENABLE) {
      Serial.println("[回巢] 主角已在巢區內，等待送回小鴨");
    }
    duckReturnBlockedCount = 0;
    duckEscapeDir = 0;
    duckEscapeStepsLeft = 0;
    return;
  }

  // 這一版停用脫困偏移
  duckEscapeDir = 0;
  duckEscapeStepsLeft = 0;

  // 沒路徑 / 走完 / 太久就重算
  if (duckPathLen <= 0 || duckPathIndex >= duckPathLen || (nowMs - duckLastPathCalcMs > DUCK_PATH_RECALC_MS)) {
    if (DUCK_DEBUG_ENABLE) {
      Serial.println("[回巢] 需要重新計算路徑");
    }
    buildDuckPathToNest();
    duckLastPathCalcMs = nowMs;
    duckReturnBlockedCount = 0;
  }

  if (duckPathLen <= 0 || duckPathIndex >= duckPathLen) {
    if (DUCK_DEBUG_ENABLE) {
      Serial.println("[回巢停止] 目前沒有可用路徑，主角暫停");
      debugPrintLeaderCore("回巢停止");
    }
    d.state = 4;
    pickDuckStillFrameByIndex(duckLeaderIndex);
    return;
  }

  int targetTileX = duckPathX[duckPathIndex];
  int targetTileY = duckPathY[duckPathIndex];

  int duckFootPx = (int)d.x + DUCK_HITBOX_OFFSET_X + (DUCK_HITBOX_W / 2);
  int duckFootPy = (int)d.y + DUCK_HITBOX_OFFSET_Y + DUCK_HITBOX_H - 1;

  int duckFootTileX = duckFootPx / DUCK_TILE_SIZE;
  int duckFootTileY = duckFootPy / DUCK_TILE_SIZE;

  int stepTileDx = targetTileX - duckFootTileX;
  int stepTileDy = targetTileY - duckFootTileY;

  d.lastX = d.x;
  d.lastY = d.y;

  if (DUCK_DEBUG_ENABLE) {
    Serial.print("[回巢步驟] 主角座標=(");
    Serial.print(d.x, 1);
    Serial.print(",");
    Serial.print(d.y, 1);
    Serial.print(") 腳底=(");
    Serial.print(duckFootPx);
    Serial.print(",");
    Serial.print(duckFootPy);
    Serial.print(") 腳底tile=(");
    Serial.print(duckFootTileX);
    Serial.print(",");
    Serial.print(duckFootTileY);
    Serial.print(") 目標格=(");
    Serial.print(targetTileX);
    Serial.print(",");
    Serial.print(targetTileY);
    Serial.print(") stepTileDx=");
    Serial.print(stepTileDx);
    Serial.print(" stepTileDy=");
    Serial.println(stepTileDy);
  }

  // 已進入目標格：直接算到達節點
  if (duckFootTileX == targetTileX && duckFootTileY == targetTileY) {
    duckPathIndex++;
    duckReturnBlockedCount = 0;

    if (DUCK_DEBUG_ENABLE) {
      Serial.print("[回巢] 已到達路徑節點，pathIndex -> ");
      Serial.println(duckPathIndex);
    }

    if (duckPathIndex >= duckPathLen) {
      if (DUCK_DEBUG_ENABLE) {
        Serial.println("[回巢] 已走完整條路徑");
      }
      d.state = 4;
      pickDuckStillFrameByIndex(duckLeaderIndex);
    }
    return;
  }

  // 防呆：path 下一格必須是相鄰格
  int manhattan = abs(stepTileDx) + abs(stepTileDy);
  if (manhattan != 1) {
    if (DUCK_DEBUG_ENABLE) {
      Serial.print("[回巢異常] 下一格不是相鄰格，stepTileDx=");
      Serial.print(stepTileDx);
      Serial.print(" stepTileDy=");
      Serial.print(stepTileDy);
      Serial.println("，重新計算路徑");
    }
    duckPathLen = 0;
    duckPathIndex = 0;
    duckReturnBlockedCount = 0;
    return;
  }

  // 目前 tile 中心（腳底中心應對齊的位置）
  float currentTileCenterFootX = duckFootTileX * DUCK_TILE_SIZE + 8;
  float currentTileCenterFootY = duckFootTileY * DUCK_TILE_SIZE + 8;

  // 反推 sprite 左上角應該在哪
  float alignedXForCurrentCol = currentTileCenterFootX - (DUCK_HITBOX_OFFSET_X + (DUCK_HITBOX_W / 2));
  float alignedYForCurrentRow = currentTileCenterFootY - (DUCK_HITBOX_OFFSET_Y + DUCK_HITBOX_H - 1);

  alignedXForCurrentCol = duck_clamp_f(alignedXForCurrentCol, 0.0f, (float)(DUCK_WORLD_W - DUCK_SPRITE_W));
  alignedYForCurrentRow = duck_clamp_f(alignedYForCurrentRow, 0.0f, (float)(DUCK_WORLD_H - DUCK_SPRITE_H));

  float tryX = d.x;
  float tryY = d.y;

  // -------------------------------------------------
  // 水平移動前，先把 Y 吸到目前列的中心線
  // 垂直移動前，先把 X 吸到目前行的中心線
  // -------------------------------------------------
  if (stepTileDx != 0) {
    float alignedY = duck_approach_f(d.y, alignedYForCurrentRow, DUCK_SPEED);

    // 如果還沒對齊，先只做 Y 對齊
    if (fabs(alignedY - alignedYForCurrentRow) > 0.01f || fabs(d.y - alignedYForCurrentRow) > 0.01f) {
      if (tryMoveLeaderDuck(d.x, alignedY)) {
        duckReturnBlockedCount = 0;

        if (DUCK_DEBUG_ENABLE) {
          Serial.print("[回巢對齊] 先校正Y，座標=(");
          Serial.print(d.x, 1);
          Serial.print(",");
          Serial.print(d.y, 1);
          Serial.println(")");
        }
        return;
      }
    }

    // 對齊完成後再水平走
    tryY = alignedYForCurrentRow;
    tryX = d.x + (stepTileDx < 0 ? -DUCK_SPEED : DUCK_SPEED);
    d.state = (stepTileDx < 0) ? 2 : 3;
  }
  else if (stepTileDy != 0) {
    float alignedX = duck_approach_f(d.x, alignedXForCurrentCol, DUCK_SPEED);

    // 如果還沒對齊，先只做 X 對齊
    if (fabs(alignedX - alignedXForCurrentCol) > 0.01f || fabs(d.x - alignedXForCurrentCol) > 0.01f) {
      if (tryMoveLeaderDuck(alignedX, d.y)) {
        duckReturnBlockedCount = 0;

        if (DUCK_DEBUG_ENABLE) {
          Serial.print("[回巢對齊] 先校正X，座標=(");
          Serial.print(d.x, 1);
          Serial.print(",");
          Serial.print(d.y, 1);
          Serial.println(")");
        }
        return;
      }
    }

    // 對齊完成後再垂直走
    tryX = alignedXForCurrentCol;
    tryY = d.y + (stepTileDy < 0 ? -DUCK_SPEED : DUCK_SPEED);
    d.state = (stepTileDy < 0) ? 1 : 0;
  }

  // 單方向過格
  if (tryMoveLeaderDuck(tryX, tryY)) {
    duckReturnBlockedCount = 0;

    
    if (DUCK_DEBUG_ENABLE) {
      Serial.print("[回巢移動成功] 單方向移動，座標=(");
      Serial.print(d.x, 1);
      Serial.print(",");
      Serial.print(d.y, 1);
      Serial.println(")");
    }
    
    
    return;
  }

  // 走不過去：只累積阻擋次數
  duckReturnBlockedCount++;

  if (DUCK_DEBUG_ENABLE) {
    Serial.print("[回巢受阻] 下一格方向走不過去，blockedCount=");
    Serial.println(duckReturnBlockedCount);
  }

  // 阻擋太多次就清路徑，下一幀重算
  if (duckReturnBlockedCount >= DUCK_RETURN_BLOCKED_LIMIT) {
    if (DUCK_DEBUG_ENABLE) {
      Serial.println("[回巢] 阻擋次數達上限，重新計算路徑");
    }
    duckPathLen = 0;
    duckPathIndex = 0;
    duckReturnBlockedCount = 0;
  }
}






// =====================================================
// 一般大鴨移動
// =====================================================
static void updateDuckMovementByIndex(int duckIndex) {
  if (duckIndex == duckLeaderIndex && duckCarryingCount > 0) {
    updateLeaderDuckReturnToNest();
    return;
  }

  if (ducks[duckIndex].state < 0 || ducks[duckIndex].state > 3) return;

  float nx = ducks[duckIndex].x;
  float ny = ducks[duckIndex].y;

  if (ducks[duckIndex].state == 0) ny += DUCK_SPEED;
  else if (ducks[duckIndex].state == 1) ny -= DUCK_SPEED;
  else if (ducks[duckIndex].state == 2) nx -= DUCK_SPEED;
  else if (ducks[duckIndex].state == 3) nx += DUCK_SPEED;

  nx = duck_clamp_f(nx, 0.0f, (float)(DUCK_WORLD_W - DUCK_SPRITE_W));
  ny = duck_clamp_f(ny, 0.0f, (float)(DUCK_WORLD_H - DUCK_SPRITE_H));

  ducks[duckIndex].lastX = ducks[duckIndex].x;
  ducks[duckIndex].lastY = ducks[duckIndex].y;

  bool blocked = false;

  if (checkDuckCollision(nx, ny)) blocked = true;
  if (!blocked && findCollidingDuckIndex(duckIndex, nx, ny) >= 0) blocked = true;

  if (!blocked) {
    ducks[duckIndex].x = nx;
    ducks[duckIndex].y = ny;
  } else {
    if (random(0, 100) < 70) ducks[duckIndex].state = 4;
    else ducks[duckIndex].state = 5;

    pickDuckStillFrameByIndex(duckIndex);
    ducks[duckIndex].lastStillPickMs = millis();
    ducks[duckIndex].nextActionMs = millis() + random(800, 1800);
  }
}

// =====================================================
// 小鴨移動
// =====================================================
static void updateDucklingWanderByIndex(int i) {
  if (!ducklings[i].active) return;
  if (ducklings[i].following) return;
  if (ducklings[i].dir == 4) return;

  float nx = ducklings[i].x;
  float ny = ducklings[i].y;

  if (ducklings[i].dir == 0) ny += DUCKLING_WANDER_SPEED;
  else if (ducklings[i].dir == 1) ny -= DUCKLING_WANDER_SPEED;
  else if (ducklings[i].dir == 2) nx -= DUCKLING_WANDER_SPEED;
  else if (ducklings[i].dir == 3) nx += DUCKLING_WANDER_SPEED;

  nx = duck_clamp_f(nx, 0.0f, (float)(DUCK_WORLD_W - DUCKLING_SPRITE_W));
  ny = duck_clamp_f(ny, 0.0f, (float)(DUCK_WORLD_H - DUCKLING_SPRITE_H));

  ducklings[i].lastX = ducklings[i].x;
  ducklings[i].lastY = ducklings[i].y;

  if (!checkDucklingCollision(nx, ny)) {
    ducklings[i].x = nx;
    ducklings[i].y = ny;
  } else {
    int dice = random(0, 100);
    if (dice < 75) ducklings[i].dir = random(0, 4);
    else ducklings[i].dir = 4;

    ducklings[i].nextActionMs = millis() + random(DUCKLING_AI_MIN_MS, DUCKLING_AI_MAX_MS + 1);
  }
}


static void getLeaderTailAnchor(float &ax, float &ay) {
  DuckActor &d = ducks[duckLeaderIndex];

  float centerX = d.x + DUCK_HITBOX_OFFSET_X + (DUCK_HITBOX_W * 0.5f);
  float centerY = d.y + DUCK_HITBOX_OFFSET_Y + (DUCK_HITBOX_H * 0.5f);

  const float followDistSide = 18.0f;
  const float followDistDown = 18.0f;
  const float followDistUp   = 25.0f;   // 往上時多拉遠一點

  if (d.state == 0) {          // 朝下，尾巴在上
    ax = centerX;
    ay = centerY - followDistDown;
  } else if (d.state == 1) {   // 朝上，尾巴在下
    ax = centerX;
    ay = centerY + followDistUp;
  } else if (d.state == 2) {   // 朝左，尾巴在右
    ax = centerX + followDistSide;
    ay = centerY;
  } else {                     // 朝右，尾巴在左
    ax = centerX - followDistSide;
    ay = centerY;
  }
}


static void updateDucklingFollowByIndex(int i) {
  if (!ducklings[i].active || !ducklings[i].following) return;

  float tailX, tailY;
  getLeaderTailAnchor(tailX, tailY);

  // 讓小鴨以自己的中心去追母鴨尾巴錨點
  float ducklingCenterX = ducklings[i].x + (DUCKLING_SPRITE_W * 0.5f);
  float ducklingCenterY = ducklings[i].y + (DUCKLING_SPRITE_H * 0.5f);

  ducklings[i].lastX = ducklings[i].x;
  ducklings[i].lastY = ducklings[i].y;

  float dx = tailX - ducklingCenterX;
  float dy = tailY - ducklingCenterY;

  float nx = ducklings[i].x;
  float ny = ducklings[i].y;

  if (abs(dx) >= abs(dy)) {
    if (dx > 1.0f) {
      nx += DUCKLING_FOLLOW_SPEED;
      ducklings[i].dir = 3;
    } else if (dx < -1.0f) {
      nx -= DUCKLING_FOLLOW_SPEED;
      ducklings[i].dir = 2;
    }

    if (!checkDucklingCollision(nx, ny)) ducklings[i].x = nx;

    nx = ducklings[i].x;
    ny = ducklings[i].y;

    if (dy > 1.0f) {
      ny += DUCKLING_FOLLOW_SPEED;
      ducklings[i].dir = 0;
    } else if (dy < -1.0f) {
      ny -= DUCKLING_FOLLOW_SPEED;
      ducklings[i].dir = 1;
    }

    if (!checkDucklingCollision(nx, ny)) ducklings[i].y = ny;
  }
  else {
    if (dy > 1.0f) {
      ny += DUCKLING_FOLLOW_SPEED;
      ducklings[i].dir = 0;
    } else if (dy < -1.0f) {
      ny -= DUCKLING_FOLLOW_SPEED;
      ducklings[i].dir = 1;
    }

    if (!checkDucklingCollision(nx, ny)) ducklings[i].y = ny;

    nx = ducklings[i].x;
    ny = ducklings[i].y;

    if (dx > 1.0f) {
      nx += DUCKLING_FOLLOW_SPEED;
      ducklings[i].dir = 3;
    } else if (dx < -1.0f) {
      nx -= DUCKLING_FOLLOW_SPEED;
      ducklings[i].dir = 2;
    }

    if (!checkDucklingCollision(nx, ny)) ducklings[i].x = nx;
  }

  ducklings[i].x = duck_clamp_f(ducklings[i].x, 0.0f, (float)(DUCK_WORLD_W - DUCKLING_SPRITE_W));
  ducklings[i].y = duck_clamp_f(ducklings[i].y, 0.0f, (float)(DUCK_WORLD_H - DUCKLING_SPRITE_H));
}


// =====================================================
// 撿小鴨
// =====================================================
static void checkDucklingPickup() {
  if (duckCarryingCount > 0) return;

  for (int i = 0; i < DUCKLING_MAX; i++) {
    if (!ducklings[i].active || ducklings[i].following) continue;

    if (checkLeaderDuckVsDucklingCollision(i)) {
      ducklings[i].following = true;
      duckCarryingCount = 1;
      duckLeaderReturningNest = true;

      duckPathLen = 0;
      duckPathIndex = 0;
      duckLastPathCalcMs = 0;
      duckLastSuccessfulSideDir = 0;

      duckReturnBlockedCount = 0;
      duckEscapeDir = 0;
      duckEscapeStepsLeft = 0;

      if (DUCK_DEBUG_ENABLE) {
        Serial.print("[撿到小鴨] 主角=");
        Serial.print(duckLeaderIndex);
        Serial.print(" 小鴨index=");
        Serial.print(i);
        Serial.print(" 主角座標=(");
        Serial.print(ducks[duckLeaderIndex].x, 1);
        Serial.print(",");
        Serial.print(ducks[duckLeaderIndex].y, 1);
        Serial.println(")");
      }
      break;
    }
  }
}

// =====================================================
// 隨機找位置
// =====================================================
static void findRandomWalkablePos(float &outX, float &outY, bool smallDuck) {
  for (int tries = 0; tries < 500; tries++) {
    float x;
    float y;

    if (smallDuck) {
      x = (float)random(0, DUCK_WORLD_W - DUCKLING_SPRITE_W);
      y = (float)random(0, DUCK_WORLD_H - DUCKLING_SPRITE_H);
    } else {
      x = (float)random(0, DUCK_WORLD_W - DUCK_SPRITE_W);
      y = (float)random(0, DUCK_WORLD_H - DUCK_SPRITE_H);
    }

    bool blocked = smallDuck ? checkDucklingCollision(x, y) : checkDuckCollision(x, y);
    if (blocked) continue;

    int centerX = smallDuck ? ((int)x + (DUCKLING_SPRITE_W / 2)) : ((int)x + (DUCK_SPRITE_W / 2));
    int centerY = smallDuck ? ((int)y + (DUCKLING_SPRITE_H / 2)) : ((int)y + (DUCK_SPRITE_H / 2));

    if (isPointInNest(centerX, centerY)) continue;

    outX = x;
    outY = y;
    return;
  }

  outX = 32;
  outY = 32;
}

// =====================================================
// 小鴨重生
// =====================================================
static void respawnDucklingByIndex(int i) {
  findRandomWalkablePos(ducklings[i].x, ducklings[i].y, true);

  ducklings[i].lastX = ducklings[i].x;
  ducklings[i].lastY = ducklings[i].y;
  ducklings[i].active = true;
  ducklings[i].following = false;
  ducklings[i].rescued = false;
  ducklings[i].dir = random(0, 5);
  ducklings[i].moveFrame = random(0, 6);
  ducklings[i].stillFrame = 0;
  ducklings[i].lastAnimMs = millis();
  ducklings[i].nextActionMs = millis() + random(DUCKLING_AI_MIN_MS, DUCKLING_AI_MAX_MS + 1);
}

// =====================================================
// 送回巢
// =====================================================
static void checkDucklingDeliverToNest() {
  if (duckCarryingCount <= 0) return;
  if (!isLeaderDuckInNest()) return;

  if (DUCK_DEBUG_ENABLE) {
    Serial.print("[送回成功] 主角=");
    Serial.print(duckLeaderIndex);
    Serial.print(" 主角座標=(");
    Serial.print(ducks[duckLeaderIndex].x, 1);
    Serial.print(",");
    Serial.print(ducks[duckLeaderIndex].y, 1);
    Serial.print(") 在巢區=");
    Serial.println(isLeaderDuckInNest() ? "是" : "否");
  }

  for (int i = 0; i < DUCKLING_MAX; i++) {
    if (!ducklings[i].active || !ducklings[i].following) continue;
    respawnDucklingByIndex(i);
    break;
  }

  duckCarryingCount = 0;
  duckLeaderReturningNest = false;

  duckPathLen = 0;
  duckPathIndex = 0;
  duckLastSuccessfulSideDir = 0;

  duckReturnBlockedCount = 0;
  duckEscapeDir = 0;
  duckEscapeStepsLeft = 0;

  duckReturnStuckStartMs = 0;
  duckReturnProgressX = ducks[duckLeaderIndex].x;
  duckReturnProgressY = ducks[duckLeaderIndex].y;


  ducks[duckLeaderIndex].state = 4;
  pickDuckStillFrameByIndex(duckLeaderIndex);
  ducks[duckLeaderIndex].lastStillPickMs = millis();
  ducks[duckLeaderIndex].nextActionMs = millis() + random(2000, 4000);
}


static void checkDuckReturnStuck(unsigned long nowMs) {
  // 沒在帶小鴨回巢，就清掉監控
  if (!duckLeaderReturningNest || duckCarryingCount <= 0) {
    duckReturnStuckStartMs = 0;
    duckReturnProgressX = ducks[duckLeaderIndex].x;
    duckReturnProgressY = ducks[duckLeaderIndex].y;
    return;
  }

  float dx = ducks[duckLeaderIndex].x - duckReturnProgressX;
  float dy = ducks[duckLeaderIndex].y - duckReturnProgressY;
  float dist2 = dx * dx + dy * dy;

  // 只要主角有明顯位移，就代表有進展，重置卡住計時
  if (dist2 >= 4.0f) {   // 代表移動至少約 2px
    duckReturnStuckStartMs = nowMs;
    duckReturnProgressX = ducks[duckLeaderIndex].x;
    duckReturnProgressY = ducks[duckLeaderIndex].y;
    return;
  }

  // 第一次進入回巢卡住監控
  if (duckReturnStuckStartMs == 0) {
    duckReturnStuckStartMs = nowMs;
    duckReturnProgressX = ducks[duckLeaderIndex].x;
    duckReturnProgressY = ducks[duckLeaderIndex].y;
    return;
  }

  // 超過 20 秒沒進展，整個模式重設
  if (nowMs - duckReturnStuckStartMs >= 20000UL) {
    if (DUCK_DEBUG_ENABLE) {
      Serial.println("[回巢卡住20秒] 觸發 DuckMode 重設");
    }

    ModefirstRun = true;
    duckReturnStuckStartMs = 0;
  }
}


// =====================================================
// 主角切換
// =====================================================
static void checkLeaderSwitch() {
  unsigned long nowMs = millis();

  if (duckCarryingCount > 0) return;
  if (nowMs - duckLastSwitchMs < DUCK_SWITCH_COOLDOWN_MS) return;

  int otherIndex = (duckLeaderIndex == 0) ? 1 : 0;

  if (checkDuckVsDuckCollision(
        ducks[duckLeaderIndex].x, ducks[duckLeaderIndex].y,
        ducks[otherIndex].x, ducks[otherIndex].y)) {

    duckLeaderIndex = otherIndex;
    duckCameraTarget = duckLeaderIndex;
    duckLastSwitchMs = nowMs;

    duckPathLen = 0;
    duckPathIndex = 0;
    duckLastPathCalcMs = 0;

    if (DUCK_DEBUG_ENABLE) {
      Serial.print("[主角切換] 新主角=");
      Serial.print(duckLeaderIndex);
      Serial.print(" 座標=(");
      Serial.print(ducks[duckLeaderIndex].x, 1);
      Serial.print(",");
      Serial.print(ducks[duckLeaderIndex].y, 1);
      Serial.println(")");
    }
  }
}

// =====================================================
// Camera
// =====================================================
static void updateDuckCamera() {
  int duckCenterX = (int)ducks[duckCameraTarget].x + (DUCK_SPRITE_W / 2);
  int duckCenterY = (int)ducks[duckCameraTarget].y + (DUCK_SPRITE_H *0.1);

  duckCameraX = duck_clamp_i(duckCenterX - (DUCK_SCREEN_W / 2), 0, DUCK_WORLD_W - DUCK_SCREEN_W);
  duckCameraY = duck_clamp_i(duckCenterY - (DUCK_SCREEN_H / 2), 0, DUCK_WORLD_H - DUCK_SCREEN_H);
}


// =====================================================
// 魚的碰撞偵測 (0:通行, 1&2:阻擋)
// =====================================================
// =====================================================
// 魚的完整矩形碰撞偵測 (AABB 掃描)
// =====================================================
static bool checkFishCollision(float x, float y, int dir) {
  // 1. 根據方向決定當前魚的寬 (w) 與高 (h)
  int w, h;
  if (dir == 0 || dir == 1) { // 垂直游動 (上下)
    w = 9;  h = 19;
  } else {                    // 水平游動 (左右)
    w = 19; h = 9;
  }

  // 2. 計算矩形在世界地圖上的四個邊界位置
  int left   = (int)x;
  int right  = (int)x + w - 1;
  int top    = (int)y;
  int bottom = (int)y + h - 1;

  // 3. 換算成地圖索引 (Tile Index)
  // 檢查從左上角的 Tile 到右下角的 Tile 之間的所有區域
  int startTileX = left / 16;
  int endTileX   = right / 16;
  int startTileY = top / 16;
  int endTileY   = bottom / 16;

  // 4. 迴圈掃描該區域覆蓋的所有 Tile
  for (int ty = startTileY; ty <= endTileY; ty++) {
    for (int tx = startTileX; tx <= endTileX; tx++) {
      
      // 邊界保護：超出地圖範圍視為碰撞
      if (tx < 0 || tx >= DUCK_MAP_COLS || ty < 0 || ty >= DUCK_MAP_ROWS) {
        return true; 
      }

      // 讀取地圖 ID
      uint8_t tileId = pgm_read_byte(&(Duck::DUCK_MAP[ty][tx]));

      // 只要區塊內任何一個 Tile 是 1 或 2，就代表「整張圖檔」有部分重疊了
      if (tileId == 1 || tileId == 2) {
        return true; 
      }
    }
  }

  return false; // 整個矩形範圍內都是 0 (自由通行)
}

// =====================================================
// 魚的 AI 更新
// =====================================================
static void updateFishAI(int i, unsigned long nowMs) {
  FishState &f = fishes[i];

  // 隨機換方向
  if (nowMs >= f.nextActionMs) {
    f.dir = random(4); 
    f.nextActionMs = nowMs + random(1000, 3000);
  }

  float speed = 0.25f;   //魚的速度  越大越快
  float nx = f.x;
  float ny = f.y;

  if (f.dir == 0) ny += speed;
  else if (f.dir == 1) ny -= speed;
  else if (f.dir == 2) nx -= speed;
  else if (f.dir == 3) nx += speed;

  // 使用「完整矩形」偵測
  if (!checkFishCollision(nx, ny, f.dir)) {
    f.x = nx;
    f.y = ny;
  } else {
    // 撞到東西了，立刻換方向，並往後退一點點（防止卡死在牆裡）
    f.dir = random(4);
    f.nextActionMs = nowMs + 200;
  }

  // 動畫更新
  if (nowMs - f.lastAnimMs > 100) {
    f.animFrame = (f.animFrame + 1) % FISH_TOTAL_FRAMES;
    f.lastAnimMs = nowMs;
  }
}



// =====================================================
// 繪製單條魚 (處理上下翻轉與左右旋轉)
// =====================================================
static void drawOneFish(int index) {
  FishState &f = fishes[index];

  int screenX = (int)f.x - cameraX;
  int screenY = (int)f.y - cameraY;

  // 視距裁切，旋轉後最寬約 19
  if (screenX < -20 || screenX > DUCK_SCREEN_W ||
      screenY < -20 || screenY > DUCK_SCREEN_H) {
    return;
  }

  int frameOffset = f.animFrame;

  for (int j = 0; j < FISH_H; j++) {
    for (int i = 0; i < FISH_W; i++) {
      int srcX = (frameOffset * FISH_W) + i;
      int srcY = j;

      uint32_t pixelPos =
        (uint32_t)srcY * (uint32_t)FISH_SHEET_W +
        (uint32_t)srcX;

      uint8_t colorIndex = pgm_read_byte(&(Duck::DUCK_FISH[pixelPos]));
      uint16_t color = pgm_read_word(&(Duck::DUCK_FISH_PALETTE[colorIndex]));

      if (color == DUCK_TRANSPARENT_COLOR) continue;

      int drawX = screenX;
      int drawY = screenY;

      switch (f.dir) {
        case 0: // 往下，原圖
          drawX = screenX + i;
          drawY = screenY + j;
          break;

        case 1: // 往上，垂直翻轉
          drawX = screenX + i;
          drawY = screenY + (FISH_H - 1 - j);
          break;

        case 2: // 往右，順時針旋轉 90 度
          drawX = screenX + (FISH_H - 1 - j);
          drawY = screenY + i;
          break;

        case 3: // 往左，逆時針旋轉 90 度
          drawX = screenX + j;
          drawY = screenY + (FISH_W - 1 - i);
          break;
      }

      if (drawX < 0 || drawX >= DUCK_SCREEN_W ||
          drawY < 0 || drawY >= DUCK_SCREEN_H) {
        continue;
      }

      display.drawPixel(drawX, drawY, color);
    }
  }
}











// =====================================================
// 初始化
// =====================================================
static void initOneDuckByIndex(int duckIndex, float startX, float startY) {
  ducks[duckIndex].sprite = duckSpriteSheets[duckIndex];
  ducks[duckIndex].palette = duckSpritePalettes[duckIndex];
  ducks[duckIndex].name = duckNames[duckIndex];
  ducks[duckIndex].x = startX;
  ducks[duckIndex].y = startY;
  ducks[duckIndex].lastX = startX;
  ducks[duckIndex].lastY = startY;

  ducks[duckIndex].state = 4;
  ducks[duckIndex].moveFrame = 0;
  ducks[duckIndex].stillFrame = 0;
  ducks[duckIndex].lastAnimMs = 0;
  ducks[duckIndex].nextActionMs = millis() + random(1000, 2500);
  ducks[duckIndex].lastStillPickMs = millis();

  if (checkDuckCollision(ducks[duckIndex].x, ducks[duckIndex].y)) {
    ducks[duckIndex].x = 64;
    ducks[duckIndex].y = 64;
  }

  pickDuckStillFrameByIndex(duckIndex);
}

static void initAllDucks() {
  initOneDuckByIndex(0, 300.0f, 140.0f);
  initOneDuckByIndex(1, 300.0f, 260.0f);


  if (checkDuckVsDuckCollision(ducks[0].x, ducks[0].y, ducks[1].x, ducks[1].y)) {
    ducks[1].x = 80.0f;
    ducks[1].y = 220.0f;
  }
}

static void initAllDucklings() {
  duckCarryingCount = 0;
  duckLeaderReturningNest = false;

  for (int i = 0; i < DUCKLING_MAX; i++) {
    ducklings[i].active = true;
    ducklings[i].following = false;
    ducklings[i].rescued = false;
    ducklings[i].x = 0;
    ducklings[i].y = 0;
    ducklings[i].lastX = 0;
    ducklings[i].lastY = 0;
    ducklings[i].dir = random(0, 5);
    ducklings[i].moveFrame = random(0, 6);
    ducklings[i].stillFrame = 0;
    ducklings[i].lastAnimMs = 0;
    ducklings[i].nextActionMs = millis() + random(DUCKLING_AI_MIN_MS, DUCKLING_AI_MAX_MS + 1);

    findRandomWalkablePos(ducklings[i].x, ducklings[i].y, true);
  }
}

void DuckModeInit() {
  if (!ModefirstRun) return;

  randomSeed(millis());
  initFishes();
  initAllDucks();
  initAllDucklings();

  duckLeaderIndex = 0;
  duckCameraTarget = 0;
  duckCameraX = 0;
  duckCameraY = 0;

  duckLastSwitchMs = 0;

  duckPathLen = 0;
  duckPathIndex = 0;
  duckLastPathCalcMs = 0;


  duckReturnStuckStartMs = 0;
  duckReturnProgressX = ducks[duckLeaderIndex].x;
  duckReturnProgressY = ducks[duckLeaderIndex].y;


  updateDuckCamera();
  renderDuckFullFrame();

  if (DUCK_DEBUG_ENABLE) {
    Serial.println("======================================");
    Serial.println("[初始化完成] DuckMode 已啟動");
    debugPrintLeaderCore("初始化");
  }

  ModefirstRun = false;
}


// 初始化魚的位置
void initFishes() {
  for (int i = 0; i < FISH_COUNT; i++) {
    fishes[i].active = false;   // 先清掉
    fishes[i].x = 0;
    fishes[i].y = 0;
    fishes[i].dir = 0;
    fishes[i].animFrame = 0;
    fishes[i].lastAnimMs = 0;
    fishes[i].nextActionMs = 0;

    bool posFound = false;
    int retry = 0;
    
    while (!posFound && retry < 100) {   // 50 可以，但 100 更穩
      float rx = random(0, DUCK_WORLD_W - 20);
      float ry = random(0, DUCK_WORLD_H - 20);
      
      if (!checkFishCollision(rx, ry, 0)) {
        fishes[i].x = rx;
        fishes[i].y = ry;
        fishes[i].dir = random(4);
        fishes[i].animFrame = random(FISH_TOTAL_FRAMES);
        fishes[i].lastAnimMs = millis();
        fishes[i].nextActionMs = millis() + random(500, 2000);
        fishes[i].active = true;
        posFound = true;
      }
      retry++;
    }
  }
}


// =====================================================
// 主迴圈
// =====================================================
void DuckMode() {
  TESTDUCK();
  DuckModeInit();
  

  
  display.fillScreen(0x33D8);

  unsigned long nowMs = millis();

  for (int i = 0; i < FISH_COUNT; i++) {
      updateFishAI(i, nowMs); // 更新魚
       }


  for (int i = 0; i < DUCK_COUNT; i++) {
    updateDuckAIByIndex(i, nowMs);
  }

  for (int i = 0; i < DUCK_COUNT; i++) {
    updateDuckMovementByIndex(i);
  }

  checkLeaderSwitch();
  checkDucklingPickup();

for (int i = 0; i < FISH_COUNT; i++) {
  if (fishes[i].active) {
    updateFishAI(i, nowMs);
  }
}

  for (int i = 0; i < DUCKLING_MAX; i++) {
    updateDucklingWanderByIndex(i);
  }

  for (int i = 0; i < DUCKLING_MAX; i++) {
    updateDucklingFollowByIndex(i);
  }

  checkDucklingDeliverToNest();
  checkDuckReturnStuck(nowMs);

  for (int i = 0; i < DUCK_COUNT; i++) {
    updateDuckAnimationByIndex(i, nowMs);
  }

  for (int i = 0; i < DUCKLING_MAX; i++) {
    updateDucklingAnimationByIndex(i, nowMs);
  }

  updateDuckCamera();
  renderDuckFullFrame();
  debugLeaderHeartbeat();   //顯示DEBUG

  wait_with_display(40);
}



// =====================================================
// 序列埠除錯
// R = 重開
// D = 開關中文 debug
// 300,200 = 主角跳點
// =====================================================
void TESTDUCK() {
  if (Serial.available() > 0) {
    char firstChar = Serial.peek();

    if (firstChar == 'R' || firstChar == 'r') {
      Serial.read();
      Serial.println(F("重新啟動 ESP32-C6..."));
      Serial.flush();
      delay(200);
      ESP.restart();
    }
    else if (firstChar == 'D' || firstChar == 'd') {
      Serial.read();
      DUCK_DEBUG_ENABLE = !DUCK_DEBUG_ENABLE;

      Serial.print("[DEBUG] 中文除錯輸出 = ");
      Serial.println(DUCK_DEBUG_ENABLE ? "開啟" : "關閉");

      while (Serial.available() > 0) {
        Serial.read();
      }
    }
    else if (isDigit(firstChar)) {
      int tx = Serial.parseInt();
      int ty = Serial.parseInt();

      ducks[duckLeaderIndex].x = (float)constrain(tx, 0, DUCK_WORLD_W - DUCK_SPRITE_W);
      ducks[duckLeaderIndex].y = (float)constrain(ty, 0, DUCK_WORLD_H - DUCK_SPRITE_H);

      ducks[duckLeaderIndex].state = 4;
      pickDuckStillFrameByIndex(duckLeaderIndex);
      ducks[duckLeaderIndex].nextActionMs = millis() + 5000;

      duckPathLen = 0;
      duckPathIndex = 0;
      duckLastPathCalcMs = 0;

      Serial.print(F("[手動跳點] 主角="));
      Serial.print(duckLeaderIndex);
      Serial.print(F(" X:"));
      Serial.print(ducks[duckLeaderIndex].x);
      Serial.print(F(" Y:"));
      Serial.println(ducks[duckLeaderIndex].y);

      while (Serial.available() > 0) {
        Serial.read();
      }
    }
    else {
      Serial.read();
    }
  }
}
