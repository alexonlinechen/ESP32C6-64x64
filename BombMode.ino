// =====================================================
// Bomberman 主題模式
// =====================================================
//
// Bomb.h 內需要有以下圖資：
//
// BOMB_DATA[]   : 地圖圖資
//                 總寬 96，高 16
//                 單張 16x16，共 6 張
//
// BOMB_MAN_A[]  : 主角 A
// BOMB_MAN_B[]  : 主角 B
//                 總寬 144，高 16
//                 共 9 張，每張寬 16
//
// BOMB_MST_[]   : 怪物圖資
//                 總寬 64，高 16
//                 共 4 張，每張 16x16
//
// BOMB_BOMB[]   : 炸彈 / 爆炸圖資
//                 總寬 240，高 16
//                 共 15 張，每張 16x16
//
// BOMB_MAP[13][15] : 地圖資料
//

enum {
  A = 12,   // 可破壞物件，對應 BOMB_DATA frame 7
  B = 13,   // 怪物生成點
  C = 14    // 固定不可破壞物件，對應 BOMB_DATA frame 8
};

#include "Bomb.h"


// =====================================================
// 螢幕設定
// =====================================================
static const int BOMB_SCR_W = 64;
static const int BOMB_SCR_H = 64;

// =====================================================
// 透明色
// =====================================================
static const uint16_t BOMB_TRANSPARENT = 0xf81f;

// =====================================================
// 基本 tile 設定
// =====================================================
static const int BOMB_TILE_SIZE = 16;

// 你的地圖是 13 列 x 15 欄
static const int BOMB_MAP_ROWS = 13;
static const int BOMB_MAP_COLS = 15;

// =====================================================
// BOMB_DATA[] 地圖圖資設定
//
// BOMB_MAP 數字 0~3：直接對應 BOMB_DATA frame 0~3
// BOMB_MAP 數字 6,7：對應 BOMB_DATA frame 1,2，並水平翻轉
// BOMB_MAP 數字 4,5：對應 BOMB_DATA frame 1,3，並垂直翻轉
// BOMB_MAP 數字 8：對應 BOMB_DATA frame 1，並右旋180度

// A：BOMB_DATA frame 7，可破壞物件
// C：BOMB_DATA frame 8，固定不可破壞物件
// B：怪物生成點，不顯示，實際會轉成地板
// =====================================================
static const int BOMB_DATA_SHEET_W = 96;
static const int BOMB_DATA_FRAME_W = 16;
static const int BOMB_DATA_FRAME_H = 16;

static const uint8_t* currentBombDataSheet = BOMB_DATA;
static const uint16_t* currentBombDataPalette = BOMB_DATA_PALETTE;

// =====================================================
// 主角圖資設定
//
// BOMB_MAN_A[] / BOMB_MAN_B[]
// 144x19，共 9 張
//
// frame 0,1,2 = 向下
// frame 3,4,5 = 向上
// frame 6,7,8 = 向右
// 向左使用向右 frame 水平翻轉
// =====================================================
static const int BOMB_MAN_SHEET_W = 144;
static const int BOMB_MAN_FRAME_W = 16;
static const int BOMB_MAN_FRAME_H = 16;

// =====================================================
// 怪物圖資設定
// 怪物 A：frame 0,1
// 怪物 B：frame 2,3
// =====================================================
static const int BOMB_MST_SHEET_W = 96;
static const int BOMB_MST_FRAME_W = 16;
static const int BOMB_MST_FRAME_H = 16;
static const int BOMB_MONSTER_TYPE_COUNT = 3;  // 怪物種類數量：A/B/C

// =====================================================
// 炸彈 / 爆炸圖資設定
// BOMB_BOMB[]
// frame 0~2   = 炸彈動畫
// frame 3~5   = 爆炸階段 1：中心 / 身體 / 尾端
// frame 6~8   = 爆炸階段 2：中心 / 身體 / 尾端

// =====================================================
static const int BOMB_EFFECT_SHEET_W = 144;
static const int BOMB_EFFECT_FRAME_W = 16;
static const int BOMB_EFFECT_FRAME_H = 16;

// =====================================================
// 遊戲參數
// =====================================================
static const int BOMB_RANGE = 2;              // 爆炸中心往四方向各 2 格
static const int BOMB_MAX_MONSTERS = 8;       // 最多怪物數
static const int BOMB_MAX_SPAWNS = 12;        // 最多 B 生成點
static const int BOMB_MAX_FLAMES = 9;         // 中心 + 四方向各 2 格 = 9 格

// 主角固定生成位置，世界 pixel 座標為 (32,32)
static const int BOMB_PLAYER_START_X = 32;
static const int BOMB_PLAYER_START_Y = 32;
static const int BOMB_PLAYER_START_COL = BOMB_PLAYER_START_X / BOMB_TILE_SIZE;
static const int BOMB_PLAYER_START_ROW = BOMB_PLAYER_START_Y / BOMB_TILE_SIZE;

// =====================================================
// 動畫與節奏參數
// =====================================================
static const unsigned long BOMB_PLAYER_MOVE_INTERVAL_MS = 45;        // 主角移動速度，越大越慢
static const unsigned long BOMB_PLAYER_ANIM_INTERVAL_MS = 120;       // 主角走路動畫速度，越大越慢

static const unsigned long BOMB_MONSTER_MOVE_INTERVAL_MS = 90;       // 怪物移動速度，越大越慢
static const unsigned long BOMB_MONSTER_ANIM_INTERVAL_MS = 240;      // 怪物動畫速度，越大越慢

static const unsigned long BOMB_MONSTER_DEATH_BLINK_INTERVAL_MS = 120;  // 怪物死亡閃爍速度，越大越慢
static const int BOMB_MONSTER_DEATH_BLINK_COUNT = 6;                   // 怪物死亡閃爍次數，6 = 明暗切換 3 次

static const unsigned long BOMB_FUSE_MS = 4000;                      // 炸彈爆炸前等待時間，越大越久
static const unsigned long BOMB_FORCE_EXPLODE_MS = 7000;  
static const unsigned long BOMB_BOMB_ANIM_INTERVAL_MS = 180;         // 炸彈閃爍動畫速度，越大越慢
static const unsigned long BOMB_FIRE_STAGE_INTERVAL_MS = 150;        // 爆炸火焰動畫速度，越大越慢
static const unsigned long BOMB_EXPLOSION_TOTAL_MS = 900;            // 爆炸火焰停留時間，越大越久
static const unsigned long BOMB_SOFT_BLINK_INTERVAL_MS = 120;         // 可破壞物件閃爍速度，越大越慢

static const unsigned long BOMB_CAMERA_INTERVAL_MS = 30;             // 鏡頭移動更新速度，越大越慢
static const unsigned long BOMB_CAMERA_FOCUS_HOLD_MS = 1000;        // 鏡頭到炸彈後等待多久才爆炸
static const unsigned long BOMB_WIN_HOLD_MS = 5000;                  // YOU WIN 顯示時間
static const unsigned long BOMB_GAME_OVER_HOLD_MS = 5000;            // GAME OVER 顯示時間



// =====================================================
// 方向
// =====================================================
enum BombDir {
  BOMB_DIR_DOWN = 0,
  BOMB_DIR_UP,
  BOMB_DIR_RIGHT,
  BOMB_DIR_LEFT
};

static const int BOMB_DIR_DR[4] = { 1, -1, 0, 0 };
static const int BOMB_DIR_DC[4] = { 0, 0, 1, -1 };

// =====================================================
// 流程狀態
// =====================================================
enum BombPhase {
  BOMB_PHASE_WALK = 0,        // 主角尋找可炸物或怪物
  BOMB_PHASE_ESCAPE,          // 放炸彈後逃離爆炸範圍
  BOMB_PHASE_FOCUS_BOMB,      // 鏡頭移動並鎖定炸彈
  BOMB_PHASE_EXPLODING,       // 爆炸中
  BOMB_PHASE_WIN,             // 所有怪物消失，顯示 YOU WIN
  BOMB_PHASE_GAME_OVER        // 主角碰到怪物，顯示 GAME OVER 後重開
};

static BombPhase bombPhase = BOMB_PHASE_WALK;

// =====================================================
// 地圖翻轉模式
// 0 = 原圖
// 1 = 水平翻轉
// 2 = 垂直翻轉
// =====================================================
static int bombLayoutMode = 0;
static bool bombMapFlipX = false;
static bool bombMapFlipY = false;

// =====================================================
// 主角資料
// =====================================================
struct BombPlayerState {
  const uint8_t* sheet;
  const uint16_t* palette;
  int row;
  int col;

  int x;
  int y;

  int targetRow;
  int targetCol;
  int targetX;
  int targetY;

  int dir;
  int animStep;

  bool moving;

  unsigned long lastMoveMs;
  unsigned long lastAnimMs;
};

static BombPlayerState bombPlayer;

// =====================================================
// 怪物資料
// =====================================================
struct BombMonsterState {
  bool active;
  bool dying;                 // 是否正在死亡閃爍
  bool visible;               // 死亡閃爍時是否顯示
  int deathBlinkCount;        // 死亡閃爍計數
  unsigned long lastDeathBlinkMs;  // 上次死亡閃爍時間

  int type;   // 0 = 怪物 A，1 = 怪物 B

  int row;
  int col;

  int x;
  int y;

  int targetRow;
  int targetCol;
  int targetX;
  int targetY;

  int dir;
  int animStep;

  bool moving;

  unsigned long lastMoveMs;
  unsigned long lastAnimMs;
};

static BombMonsterState bombMonsters[BOMB_MAX_MONSTERS];
static int bombMonsterCount = 0;

// =====================================================
// 炸彈資料
// =====================================================
struct BombBombState {
  bool active;
  bool exploding;

  int row;
  int col;

  unsigned long placedMs;
  unsigned long explodeStartMs;

  int bombAnimStep;
  int fireStage;

  unsigned long lastBombAnimMs;
  unsigned long lastFireStageMs;
};

static BombBombState bombBomb;

// =====================================================
// 爆炸火焰資料
//
// dir：
// -1 = 中心
//  0 = 右
//  1 = 左
//  2 = 下
//  3 = 上
// =====================================================
struct BombFlameCell {
  bool active;

  int row;
  int col;

  int dir;
  bool endPart;
};

static BombFlameCell bombFlames[BOMB_MAX_FLAMES];
static int bombFlameCount = 0;

// =====================================================
// 可破壞物件閃爍資料
// =====================================================
struct BombFlashBlock {
  bool active;

  int row;
  int col;

  bool visible;
  int toggleCount;

  unsigned long lastToggleMs;
};

static BombFlashBlock bombFlashBlocks[BOMB_MAX_FLAMES];
static int bombFlashCount = 0;

// =====================================================
// RAM 地圖
//
// BOMB_MAP 是 Bomb.h 裡的初始地圖，放在 PROGMEM。
// bombMap 是實際遊戲用地圖，可被炸彈修改。
// =====================================================
static uint8_t bombMap[BOMB_MAP_ROWS][BOMB_MAP_COLS];

// =====================================================
// 怪物生成點
// =====================================================
static int bombSpawnRows[BOMB_MAX_SPAWNS];
static int bombSpawnCols[BOMB_MAX_SPAWNS];
static int bombSpawnCount = 0;

// =====================================================
// Camera
// =====================================================
static int bombCamX = 0;
static int bombCamY = 0;

static int bombTargetCamX = 0;
static int bombTargetCamY = 0;

static unsigned long bombLastCameraMs = 0;

// =====================================================
// 勝利 失敗 顯示
// =====================================================
static unsigned long bombWinStartMs = 0;
static unsigned long bombGameOverStartMs = 0;  // GAME OVER 開始時間

//炸彈爆炸參數
static unsigned long bombFocusBombStartMs = 0;  // 鏡頭到炸彈後的等待開始時間
static bool bombFocusBombArrived = false;       // 鏡頭是否已經到炸彈位置

// =====================================================
// 炸彈動畫序列
// =====================================================
static const uint8_t BOMB_BOMB_SEQ[] = {
  0, 1, 2, 1
};

static const int BOMB_BOMB_SEQ_LEN =
  sizeof(BOMB_BOMB_SEQ) / sizeof(BOMB_BOMB_SEQ[0]);

// =====================================================
// 爆炸 frame 對照表
//
// 每一列是一個爆炸階段：
// 第 0 欄 = 中心
// 第 1 欄 = 身體
// 第 2 欄 = 尾端
// =====================================================
static const uint8_t BOMB_FIRE_FRAME[2][3] PROGMEM = {
  { 3,  4,  5 },
  { 6,  7,  8 },
};

// =====================================================
// 工具：限制數值範圍
// =====================================================
static int bombClampInt(int v, int minV, int maxV) {
  if (v < minV) return minV;
  if (v > maxV) return maxV;
  return v;
}

// =====================================================
// 工具：是否在地圖內
// =====================================================
static bool bombInMap(int row, int col) {
  return row >= 0 && row < BOMB_MAP_ROWS && col >= 0 && col < BOMB_MAP_COLS;
}

// =====================================================
// 地圖格判斷
// =====================================================
static bool bombIsSoftBlock(uint8_t cell) {
  return cell == A;
}

static bool bombIsHardBlock(uint8_t cell) {
  if (cell == C) return true;
  if (cell >= 1 && cell <= 11) return true;
  return false;
}

static bool bombIsFloor(uint8_t cell) {
  return cell == 0;
}

// =====================================================
// 該格是否有尚未爆炸的炸彈
// =====================================================
static bool bombHasBombAt(int row, int col) {
  if (!bombBomb.active) return false;
  if (bombBomb.exploding) return false;

  return bombBomb.row == row && bombBomb.col == col;
}

// =====================================================
// 主角與怪物只能在 BOMB_MAP 數字 0 的區域移動
// =====================================================
static bool bombCanStandAt(int row, int col) {
  if (!bombInMap(row, col)) return false;
  if (!bombIsFloor(bombMap[row][col])) return false;
  if (bombHasBombAt(row, col)) return false;

  return true;
}

// =====================================================
// 畫圖函式
//
// rotateMode：
// 0 = 不旋轉
// 1 = 右轉 90 度
// 2 = 旋轉 180 度
// 3 = 左轉 90 度
// =====================================================

//palette 讀色小工具

static inline uint16_t bombReadPaletteColor(
  const uint8_t* sheet,
  const uint16_t* palette,
  uint32_t pixelPos
) {
  uint8_t colorIndex = pgm_read_byte(&(sheet[pixelPos]));
  return pgm_read_word(&(palette[colorIndex]));
}



static void drawBombFrameTransform(
  const uint8_t* sheet,
  const uint16_t* palette,
  int sheetW,
  int frameW,
  int frameH,
  int frameIndex,
  int xOnScreen,
  int yOnScreen,
  bool mirrorX,
  bool mirrorY,
  int rotateMode
) {
  int outW = frameW;
  int outH = frameH;

  if (rotateMode == 1 || rotateMode == 3) {
    outW = frameH;
    outH = frameW;
  }

  int frameStartX = frameIndex * frameW;

  for (int y = 0; y < outH; y++) {
    int dy = yOnScreen + y;
    if (dy < 0 || dy >= BOMB_SCR_H) continue;

    for (int x = 0; x < outW; x++) {
      int dx = xOnScreen + x;
      if (dx < 0 || dx >= BOMB_SCR_W) continue;

      int sx = x;
      int sy = y;

      if (rotateMode == 0) {
        sx = x;
        sy = y;
      } else if (rotateMode == 1) {
        sx = y;
        sy = frameH - 1 - x;
      } else if (rotateMode == 2) {
        sx = frameW - 1 - x;
        sy = frameH - 1 - y;
      } else {
        sx = frameW - 1 - y;
        sy = x;
      }

      if (mirrorX) sx = frameW - 1 - sx;
      if (mirrorY) sy = frameH - 1 - sy;

      if (sx < 0 || sx >= frameW || sy < 0 || sy >= frameH) continue;

      uint32_t pos =
        (uint32_t)sy * (uint32_t)sheetW +
        (uint32_t)(frameStartX + sx);

      uint16_t color =
        bombReadPaletteColor(sheet, palette, pos);

      if (color == BOMB_TRANSPARENT) continue;

      display.drawPixel(dx, dy, color);
    }
  }
}

// =====================================================
// BOMB_MAP 數值轉成 BOMB_DATA frame
// =====================================================
static void bombGetTileFrame(
  uint8_t cell,
  int* frameIndex,
  bool* mirrorX,
  bool* mirrorY,
  int* rotateMode
) {
  *frameIndex = 0;
  *mirrorX = false;
  *mirrorY = false;
  *rotateMode = 0;

  // 0~3：直接使用 frame 0~3
  if (cell <= 3) {
    *frameIndex = cell;
    return;
  }

  // 4：frame 1 垂直翻轉
  if (cell == 4) {
    *frameIndex = 1;
    *mirrorY = true;
    return;
  }

  // 5：frame 3 垂直翻轉
  if (cell == 5) {
    *frameIndex = 3;
    *mirrorY = true;
    return;
  }

  // 6：frame 1 水平翻轉
  if (cell == 6) {
    *frameIndex = 1;
    *mirrorX = true;
    return;
  }

  // 7：frame 2 水平翻轉
  if (cell == 7) {
    *frameIndex = 2;
    *mirrorX = true;
    return;
  }

  // 8：frame 1 旋轉 180 度
  if (cell == 8) {
    *frameIndex = 1;
    *rotateMode = 2;
    return;
  }


  if (cell == A) {
    *frameIndex = 4;
    return;
  }

  if (cell == C) {
    *frameIndex = 5;
    return;
  }

  // B：怪物生成點，不顯示，當地板
  *frameIndex = 0;
}

// =====================================================
// 可破壞物件是否正在閃爍且暫時隱藏
// =====================================================
static bool bombSoftBlockInvisibleNow(int row, int col) {
  for (int i = 0; i < bombFlashCount; i++) {
    if (!bombFlashBlocks[i].active) continue;

    if (bombFlashBlocks[i].row == row && bombFlashBlocks[i].col == col) {
      return !bombFlashBlocks[i].visible;
    }
  }

  return false;
}

// =====================================================
// 畫單一地圖格
// =====================================================
// =====================================================
// 畫單一地圖格
// =====================================================
static void bombDrawMapTile(int row, int col) {
  if (!bombInMap(row, col)) return;

  uint8_t cell = bombMap[row][col];

  if (cell == A && bombSoftBlockInvisibleNow(row, col)) {
    cell = 0;
  }

  int frameIndex = 0;
  bool localMirrorX = false;
  bool localMirrorY = false;
  int rotateMode = 0;

  bombGetTileFrame(
    cell,
    &frameIndex,
    &localMirrorX,
    &localMirrorY,
    &rotateMode
  );

  // 地圖可以翻轉排列，但 tile 圖本身不要跟著整體翻轉。
  // 只保留 bombGetTileFrame() 指定的翻轉 / 旋轉。
  bool finalMirrorX = localMirrorX;
  bool finalMirrorY = localMirrorY;

  int x = col * BOMB_TILE_SIZE - bombCamX;
  int y = row * BOMB_TILE_SIZE - bombCamY;

drawBombFrameTransform(
  currentBombDataSheet,
  currentBombDataPalette,
  BOMB_DATA_SHEET_W,
  BOMB_DATA_FRAME_W,
  BOMB_DATA_FRAME_H,
  frameIndex,
  x,
  y,
  finalMirrorX,
  finalMirrorY,
  rotateMode
);
}

// =====================================================
// 畫可見範圍內的地圖
// =====================================================
static void bombDrawMap() {
  int startCol = bombCamX / BOMB_TILE_SIZE - 1;
  int endCol = (bombCamX + BOMB_SCR_W) / BOMB_TILE_SIZE + 1;

  int startRow = bombCamY / BOMB_TILE_SIZE - 1;
  int endRow = (bombCamY + BOMB_SCR_H) / BOMB_TILE_SIZE + 1;

  if (startCol < 0) startCol = 0;
  if (startRow < 0) startRow = 0;
  if (endCol >= BOMB_MAP_COLS) endCol = BOMB_MAP_COLS - 1;
  if (endRow >= BOMB_MAP_ROWS) endRow = BOMB_MAP_ROWS - 1;

  for (int r = startRow; r <= endRow; r++) {
    for (int c = startCol; c <= endCol; c++) {
      bombDrawMapTile(r, c);
    }
  }
}

// =====================================================
// 取得主角 frame
// =====================================================
static int bombGetPlayerFrame(bool* mirrorX) {
  *mirrorX = false;

  if (bombPlayer.dir == BOMB_DIR_DOWN) {
    return 0 + bombPlayer.animStep;
  }

  if (bombPlayer.dir == BOMB_DIR_UP) {
    return 3 + bombPlayer.animStep;
  }

  if (bombPlayer.dir == BOMB_DIR_RIGHT) {
    return 6 + bombPlayer.animStep;
  }

  *mirrorX = true;
  return 6 + bombPlayer.animStep;
}

// =====================================================
// 畫主角
// =====================================================
static void bombDrawPlayer() {
  bool mirrorX = false;
  int frameIndex = bombGetPlayerFrame(&mirrorX);

  int x = bombPlayer.x - bombCamX;
  int y = bombPlayer.y - bombCamY - (BOMB_MAN_FRAME_H - BOMB_TILE_SIZE);

drawBombFrameTransform(
  bombPlayer.sheet,
  bombPlayer.palette,
  BOMB_MAN_SHEET_W,
  BOMB_MAN_FRAME_W,
  BOMB_MAN_FRAME_H,
  frameIndex,
  x,
  y,
  mirrorX,
  false,
  0
);
}

// =====================================================
// 畫怪物
// =====================================================
static void bombDrawMonsters() {
  for (int i = 0; i < bombMonsterCount; i++) {
  if (!bombMonsters[i].active) continue;
  if (bombMonsters[i].dying) continue;

    int frameIndex = bombMonsters[i].type * 2 + bombMonsters[i].animStep;

    int x = bombMonsters[i].x - bombCamX;
    int y = bombMonsters[i].y - bombCamY;

 drawBombFrameTransform(
  BOMB_MST,
  BOMB_MST_PALETTE,
  BOMB_MST_SHEET_W,
  BOMB_MST_FRAME_W,
  BOMB_MST_FRAME_H,
  frameIndex,
  x,
  y,
  false,
  false,
  0
);
  }
}

// =====================================================
// 畫死亡閃爍中的怪物
// 這個會畫在爆炸火焰上方，避免被火焰完全蓋住
// =====================================================
static void bombDrawDyingMonsters() {
  for (int i = 0; i < bombMonsterCount; i++) {
    if (!bombMonsters[i].active) continue;
    if (!bombMonsters[i].dying) continue;
    if (!bombMonsters[i].visible) continue;

    int frameIndex = bombMonsters[i].type * 2 + bombMonsters[i].animStep;

    int x = bombMonsters[i].x - bombCamX;
    int y = bombMonsters[i].y - bombCamY;

 drawBombFrameTransform(
  BOMB_MST,
  BOMB_MST_PALETTE,
  BOMB_MST_SHEET_W,
  BOMB_MST_FRAME_W,
  BOMB_MST_FRAME_H,
  frameIndex,
  x,
  y,
  false,
  false,
  0
);
  }
}


// =====================================================
// 畫炸彈
// =====================================================
static void bombDrawBomb() {
  if (!bombBomb.active) return;
  if (bombBomb.exploding) return;

  int seqIndex = bombBomb.bombAnimStep % BOMB_BOMB_SEQ_LEN;
  int frameIndex = BOMB_BOMB_SEQ[seqIndex];

  int x = bombBomb.col * BOMB_TILE_SIZE - bombCamX;
  int y = bombBomb.row * BOMB_TILE_SIZE - bombCamY;

drawBombFrameTransform(
  BOMB_EFFECT,
  BOMB_EFFECT_PALETTE,
  BOMB_EFFECT_SHEET_W,
  BOMB_EFFECT_FRAME_W,
  BOMB_EFFECT_FRAME_H,
  frameIndex,
  x,
  y,
  false,
  false,
  0
);
}

// =====================================================
// 畫爆炸火焰
// =====================================================
static void bombDrawExplosion() {
  if (!bombBomb.active) return;
  if (!bombBomb.exploding) return;

  int stage = bombBomb.fireStage;
  if (stage < 0) stage = 0;
  if (stage > 1) stage = 1;

  for (int i = 0; i < bombFlameCount; i++) {
    if (!bombFlames[i].active) continue;

    int part = 1;

    if (bombFlames[i].dir == -1) {
      part = 0;
    } else if (bombFlames[i].endPart) {
      part = 2;
    }

    int frameIndex = pgm_read_byte(&BOMB_FIRE_FRAME[stage][part]);

    bool mirrorX = false;
    int rotateMode = 0;

    if (bombFlames[i].dir == 0) {
      mirrorX = false;
      rotateMode = 0;
    } else if (bombFlames[i].dir == 1) {
      mirrorX = true;
      rotateMode = 0;
    } else if (bombFlames[i].dir == 2) {
      mirrorX = false;
      rotateMode = 1;
    } else if (bombFlames[i].dir == 3) {
      mirrorX = false;
      rotateMode = 3;
    }

    int x = bombFlames[i].col * BOMB_TILE_SIZE - bombCamX;
    int y = bombFlames[i].row * BOMB_TILE_SIZE - bombCamY;

    drawBombFrameTransform(
  BOMB_EFFECT,
  BOMB_EFFECT_PALETTE,
  BOMB_EFFECT_SHEET_W,
  BOMB_EFFECT_FRAME_W,
  BOMB_EFFECT_FRAME_H,
  frameIndex,
  x,
  y,
  mirrorX,
  false,
  rotateMode
);
  }
}

// =====================================================
// 畫勝利文字
// =====================================================
static void bombDrawWinText() {
  if (bombPhase != BOMB_PHASE_WIN) return;

  display.fillRect(10, 17, 43, 11, 0x0000);

  display.setTextWrap(false);
  display.setTextSize(1);

  display.setTextColor(0x0000);
  display.setCursor(12, 20);
  display.print("YOU WIN");

  display.setTextColor(0xffff);
  display.setCursor(11, 19);
  display.print("YOU WIN");
}


// =====================================================
// 畫 GAME OVER 文字
// =====================================================
static void bombDrawGameOverText() {
  if (bombPhase != BOMB_PHASE_GAME_OVER) return;

  display.fillRect(4, 17, 58, 11, 0x0000);

  display.setTextWrap(false);
  display.setTextSize(1);

  display.setTextColor(0x0000);
  display.setCursor(8, 20);
  display.print("GAME OVER");

  display.setTextColor(0xf800);
  display.setCursor(7, 19);
  display.print("GAME OVER");
}

// =====================================================
// 設定主角移動目標
// =====================================================
static void bombSetPlayerTarget(int row, int col, int dir) {
  if (!bombCanStandAt(row, col)) return;

  // 主角不能把怪物所在格當成移動目標
  if (bombMonsterAt(row, col)) return;

  bombPlayer.targetRow = row;
  bombPlayer.targetCol = col;
  bombPlayer.targetX = col * BOMB_TILE_SIZE;
  bombPlayer.targetY = row * BOMB_TILE_SIZE;
  bombPlayer.dir = dir;
  bombPlayer.moving = true;
}

// =====================================================
// 設定怪物移動目標
// 使用 index，避免 Arduino IDE 自動產生 prototype 時出錯
// =====================================================
static void bombSetMonsterTarget(int idx, int row, int col, int dir) {
  if (idx < 0 || idx >= bombMonsterCount) return;
  if (!bombCanStandAt(row, col)) return;

  bombMonsters[idx].targetRow = row;
  bombMonsters[idx].targetCol = col;
  bombMonsters[idx].targetX = col * BOMB_TILE_SIZE;
  bombMonsters[idx].targetY = row * BOMB_TILE_SIZE;
  bombMonsters[idx].dir = dir;
  bombMonsters[idx].moving = true;
}

// =====================================================
// 更新主角像素移動
// =====================================================
static void bombUpdatePlayerMove(unsigned long now) {
  if (!bombPlayer.moving) return;
  if (now - bombPlayer.lastMoveMs < BOMB_PLAYER_MOVE_INTERVAL_MS) return;

  bombPlayer.lastMoveMs = now;

  if (bombPlayer.x < bombPlayer.targetX) bombPlayer.x++;
  if (bombPlayer.x > bombPlayer.targetX) bombPlayer.x--;

  if (bombPlayer.y < bombPlayer.targetY) bombPlayer.y++;
  if (bombPlayer.y > bombPlayer.targetY) bombPlayer.y--;

  if (bombPlayer.x == bombPlayer.targetX && bombPlayer.y == bombPlayer.targetY) {
    bombPlayer.row = bombPlayer.targetRow;
    bombPlayer.col = bombPlayer.targetCol;
    bombPlayer.moving = false;
  }

  if (now - bombPlayer.lastAnimMs >= BOMB_PLAYER_ANIM_INTERVAL_MS) {
    bombPlayer.lastAnimMs = now;
    bombPlayer.animStep++;

    if (bombPlayer.animStep > 2) {
      bombPlayer.animStep = 0;
    }
  }
}

// =====================================================
// 更新單一怪物
// =====================================================
static void bombUpdateMonsterOne(int idx, unsigned long now) {
  if (idx < 0 || idx >= bombMonsterCount) return;
  if (!bombMonsters[idx].active) return;
  if (bombMonsters[idx].dying) return;

  if (now - bombMonsters[idx].lastAnimMs >= BOMB_MONSTER_ANIM_INTERVAL_MS) {
    bombMonsters[idx].lastAnimMs = now;
    bombMonsters[idx].animStep++;

    if (bombMonsters[idx].animStep > 1) {
      bombMonsters[idx].animStep = 0;
    }
  }

  if (bombMonsters[idx].moving) {
    if (now - bombMonsters[idx].lastMoveMs < BOMB_MONSTER_MOVE_INTERVAL_MS) return;

    bombMonsters[idx].lastMoveMs = now;

    if (bombMonsters[idx].x < bombMonsters[idx].targetX) bombMonsters[idx].x++;
    if (bombMonsters[idx].x > bombMonsters[idx].targetX) bombMonsters[idx].x--;

    if (bombMonsters[idx].y < bombMonsters[idx].targetY) bombMonsters[idx].y++;
    if (bombMonsters[idx].y > bombMonsters[idx].targetY) bombMonsters[idx].y--;

    if (bombMonsters[idx].x == bombMonsters[idx].targetX &&
        bombMonsters[idx].y == bombMonsters[idx].targetY) {
      bombMonsters[idx].row = bombMonsters[idx].targetRow;
      bombMonsters[idx].col = bombMonsters[idx].targetCol;
      bombMonsters[idx].moving = false;
    }

    return;
  }

  int choices[4];
  int choiceCount = 0;

  for (int d = 0; d < 4; d++) {
    int nr = bombMonsters[idx].row + BOMB_DIR_DR[d];
    int nc = bombMonsters[idx].col + BOMB_DIR_DC[d];

    if (bombCanStandAt(nr, nc)) {
      choices[choiceCount] = d;
      choiceCount++;
    }
  }

  if (choiceCount <= 0) return;

  int chosenDir = choices[random(choiceCount)];

  int nr = bombMonsters[idx].row + BOMB_DIR_DR[chosenDir];
  int nc = bombMonsters[idx].col + BOMB_DIR_DC[chosenDir];

  bombSetMonsterTarget(idx, nr, nc, chosenDir);
}

// =====================================================
// 更新全部怪物
// =====================================================
static void bombUpdateMonsters(unsigned long now) {
  for (int i = 0; i < bombMonsterCount; i++) {
    bombUpdateMonsterOne(i, now);
  }
}

// =====================================================
// 更新怪物死亡閃爍
// 怪物被炸到後，不會立刻消失，而是閃爍幾次後才消失
// =====================================================
static void bombUpdateMonsterDeathBlink(unsigned long now) {
  for (int i = 0; i < bombMonsterCount; i++) {
    if (!bombMonsters[i].active) continue;
    if (!bombMonsters[i].dying) continue;

    if (now - bombMonsters[i].lastDeathBlinkMs >= BOMB_MONSTER_DEATH_BLINK_INTERVAL_MS) {
      bombMonsters[i].lastDeathBlinkMs = now;
      bombMonsters[i].visible = !bombMonsters[i].visible;
      bombMonsters[i].deathBlinkCount++;

      if (bombMonsters[i].deathBlinkCount >= BOMB_MONSTER_DEATH_BLINK_COUNT) {
        bombMonsters[i].active = false;
        bombMonsters[i].dying = false;
        bombMonsters[i].visible = false;
      }
    }
  }
}

// =====================================================
// 檢查同列或同行中間是否被牆擋住
// =====================================================
static bool bombLineClearBetween(int r1, int c1, int r2, int c2) {
  if (r1 != r2 && c1 != c2) return false;

  if (r1 == r2) {
    int startC = c1 < c2 ? c1 + 1 : c2 + 1;
    int endC = c1 < c2 ? c2 - 1 : c1 - 1;

    for (int c = startC; c <= endC; c++) {
      uint8_t cell = bombMap[r1][c];

      if (bombIsHardBlock(cell) || bombIsSoftBlock(cell)) {
        return false;
      }
    }

    return true;
  }

  int startR = r1 < r2 ? r1 + 1 : r2 + 1;
  int endR = r1 < r2 ? r2 - 1 : r1 - 1;

  for (int r = startR; r <= endR; r++) {
    uint8_t cell = bombMap[r][c1];

    if (bombIsHardBlock(cell) || bombIsSoftBlock(cell)) {
      return false;
    }
  }

  return true;
}

// =====================================================
// 該格是否在目前炸彈爆炸範圍內
// =====================================================
static bool bombTileInDanger(int row, int col) {
  if (!bombBomb.active) return false;

  int br = bombBomb.row;
  int bc = bombBomb.col;

  if (row == br && col == bc) return true;

  if (row == br) {
    int dist = abs(col - bc);
    if (dist > BOMB_RANGE) return false;

    return bombLineClearBetween(br, bc, row, col);
  }

  if (col == bc) {
    int dist = abs(row - br);
    if (dist > BOMB_RANGE) return false;

    return bombLineClearBetween(br, bc, row, col);
  }

  return false;
}

// =====================================================
// 指定格是否有怪物
// 用於主角避開怪物
// =====================================================
static bool bombMonsterAt(int row, int col) {
  for (int i = 0; i < bombMonsterCount; i++) {
    if (!bombMonsters[i].active) continue;
    if (bombMonsters[i].dying) continue;
    
    if (bombMonsters[i].row == row && bombMonsters[i].col == col) {
      return true;
    }

    if (bombMonsters[i].moving &&
        bombMonsters[i].targetRow == row &&
        bombMonsters[i].targetCol == col) {
      return true;
    }
  }

  return false;
}

// =====================================================
// 指定格是否太靠近怪物
// 主角會盡量避開怪物本格與相鄰一格
// =====================================================
static bool bombMonsterDangerAt(int row, int col) {
  for (int i = 0; i < bombMonsterCount; i++) {
    if (!bombMonsters[i].active) continue;

    int dist =
      abs(bombMonsters[i].row - row) +
      abs(bombMonsters[i].col - col);

    if (dist <= 1) {
      return true;
    }

    if (bombMonsters[i].moving) {
      int targetDist =
        abs(bombMonsters[i].targetRow - row) +
        abs(bombMonsters[i].targetCol - col);

      if (targetDist <= 1) {
        return true;
      }
    }
  }

  return false;
}

// =====================================================
// 從指定格放炸彈，是否能炸到怪物
// =====================================================
static bool bombCanBombMonsterFrom(int row, int col) {
  for (int i = 0; i < bombMonsterCount; i++) {
    if (!bombMonsters[i].active) continue;

    int mr = bombMonsters[i].row;
    int mc = bombMonsters[i].col;

    if (mr == row && abs(mc - col) <= BOMB_RANGE) {
      if (bombLineClearBetween(row, col, mr, mc)) {
        return true;
      }
    }

    if (mc == col && abs(mr - row) <= BOMB_RANGE) {
      if (bombLineClearBetween(row, col, mr, mc)) {
        return true;
      }
    }
  }

  return false;
}

// =====================================================
// BFS：找一個可以炸到怪物的位置
// 會避開怪物附近的危險格
// =====================================================
static bool bombFindStepToMonsterAttack(int* outRow, int* outCol) {
  static bool visited[BOMB_MAP_ROWS][BOMB_MAP_COLS];
  static int prevR[BOMB_MAP_ROWS][BOMB_MAP_COLS];
  static int prevC[BOMB_MAP_ROWS][BOMB_MAP_COLS];

  int queueR[BOMB_MAP_ROWS * BOMB_MAP_COLS];
  int queueC[BOMB_MAP_ROWS * BOMB_MAP_COLS];

  for (int r = 0; r < BOMB_MAP_ROWS; r++) {
    for (int c = 0; c < BOMB_MAP_COLS; c++) {
      visited[r][c] = false;
      prevR[r][c] = -1;
      prevC[r][c] = -1;
    }
  }

  int head = 0;
  int tail = 0;

  queueR[tail] = bombPlayer.row;
  queueC[tail] = bombPlayer.col;
  tail++;

  visited[bombPlayer.row][bombPlayer.col] = true;

  int foundR = -1;
  int foundC = -1;

  while (head < tail) {
    int r = queueR[head];
    int c = queueC[head];
    head++;

    if (!(r == bombPlayer.row && c == bombPlayer.col)) {
      if (bombCanStandAt(r, c) &&
          !bombMonsterDangerAt(r, c) &&
          bombCanBombMonsterFrom(r, c)) {
        foundR = r;
        foundC = c;
        break;
      }
    }

    for (int d = 0; d < 4; d++) {
      int nr = r + BOMB_DIR_DR[d];
      int nc = c + BOMB_DIR_DC[d];

      if (!bombInMap(nr, nc)) continue;
      if (visited[nr][nc]) continue;
      if (!bombCanStandAt(nr, nc)) continue;
      if (bombMonsterAt(nr, nc)) continue;

      visited[nr][nc] = true;
      prevR[nr][nc] = r;
      prevC[nr][nc] = c;

      queueR[tail] = nr;
      queueC[tail] = nc;
      tail++;
    }
  }

  if (foundR < 0) return false;

  int cr = foundR;
  int cc = foundC;

  while (!(prevR[cr][cc] == bombPlayer.row && prevC[cr][cc] == bombPlayer.col)) {
    int pr = prevR[cr][cc];
    int pc = prevC[cr][cc];

    if (pr < 0 || pc < 0) break;

    cr = pr;
    cc = pc;
  }

  *outRow = cr;
  *outCol = cc;

  return true;
}

// =====================================================
// BFS：怪物太近時，找安全格逃離
// =====================================================
static bool bombFindSafeFromMonsterStep(int* outRow, int* outCol) {
  static bool visited[BOMB_MAP_ROWS][BOMB_MAP_COLS];
  static int prevR[BOMB_MAP_ROWS][BOMB_MAP_COLS];
  static int prevC[BOMB_MAP_ROWS][BOMB_MAP_COLS];

  int queueR[BOMB_MAP_ROWS * BOMB_MAP_COLS];
  int queueC[BOMB_MAP_ROWS * BOMB_MAP_COLS];

  for (int r = 0; r < BOMB_MAP_ROWS; r++) {
    for (int c = 0; c < BOMB_MAP_COLS; c++) {
      visited[r][c] = false;
      prevR[r][c] = -1;
      prevC[r][c] = -1;
    }
  }

  int head = 0;
  int tail = 0;

  queueR[tail] = bombPlayer.row;
  queueC[tail] = bombPlayer.col;
  tail++;

  visited[bombPlayer.row][bombPlayer.col] = true;

  int foundR = -1;
  int foundC = -1;

  while (head < tail) {
    int r = queueR[head];
    int c = queueC[head];
    head++;

    if (!(r == bombPlayer.row && c == bombPlayer.col)) {
      if (bombCanStandAt(r, c) &&
          !bombMonsterDangerAt(r, c) &&
          !bombTileInDanger(r, c)) {
        foundR = r;
        foundC = c;
        break;
      }
    }

    for (int d = 0; d < 4; d++) {
      int nr = r + BOMB_DIR_DR[d];
      int nc = c + BOMB_DIR_DC[d];

      if (!bombInMap(nr, nc)) continue;
      if (visited[nr][nc]) continue;
      if (!bombCanStandAt(nr, nc)) continue;
      if (bombMonsterAt(nr, nc)) continue;

      visited[nr][nc] = true;
      prevR[nr][nc] = r;
      prevC[nr][nc] = c;

      queueR[tail] = nr;
      queueC[tail] = nc;
      tail++;
    }
  }

  if (foundR < 0) return false;

  int cr = foundR;
  int cc = foundC;

  while (!(prevR[cr][cc] == bombPlayer.row && prevC[cr][cc] == bombPlayer.col)) {
    int pr = prevR[cr][cc];
    int pc = prevC[cr][cc];

    if (pr < 0 || pc < 0) break;

    cr = pr;
    cc = pc;
  }

  *outRow = cr;
  *outCol = cc;

  return true;
}


// =====================================================
// 主角是否接近可破壞物件
// =====================================================
static bool bombPlayerNearSoftBlock() {
  for (int d = 0; d < 4; d++) {
    int nr = bombPlayer.row + BOMB_DIR_DR[d];
    int nc = bombPlayer.col + BOMB_DIR_DC[d];

    if (!bombInMap(nr, nc)) continue;

    if (bombMap[nr][nc] == A) {
      return true;
    }
  }

  return false;
}

// =====================================================
// 主角是否可以用炸彈攻擊怪物
// =====================================================
static bool bombPlayerCanBombMonster() {
  for (int i = 0; i < bombMonsterCount; i++) {
    if (!bombMonsters[i].active) continue;

    int mr = bombMonsters[i].row;
    int mc = bombMonsters[i].col;

    int manhattan = abs(mr - bombPlayer.row) + abs(mc - bombPlayer.col);

    if (manhattan == 1) {
      return true;
    }

    if (mr == bombPlayer.row && abs(mc - bombPlayer.col) <= BOMB_RANGE) {
      if (bombLineClearBetween(bombPlayer.row, bombPlayer.col, mr, mc)) {
        return true;
      }
    }

    if (mc == bombPlayer.col && abs(mr - bombPlayer.row) <= BOMB_RANGE) {
      if (bombLineClearBetween(bombPlayer.row, bombPlayer.col, mr, mc)) {
        return true;
      }
    }
  }

  return false;
}

// =====================================================
// 是否應該放炸彈
// =====================================================
static bool bombShouldPlaceBomb() {
  if (bombBomb.active) return false;

  if (bombPlayerNearSoftBlock()) return true;
  if (bombPlayerCanBombMonster()) return true;

  return false;
}

// =====================================================
// BFS：尋找安全格
// 主角放炸彈後，要離開爆炸範圍
// =====================================================
static bool bombFindSafeStep(int* outRow, int* outCol) {
  static bool visited[BOMB_MAP_ROWS][BOMB_MAP_COLS];
  static int prevR[BOMB_MAP_ROWS][BOMB_MAP_COLS];
  static int prevC[BOMB_MAP_ROWS][BOMB_MAP_COLS];

  int queueR[BOMB_MAP_ROWS * BOMB_MAP_COLS];
  int queueC[BOMB_MAP_ROWS * BOMB_MAP_COLS];

  for (int r = 0; r < BOMB_MAP_ROWS; r++) {
    for (int c = 0; c < BOMB_MAP_COLS; c++) {
      visited[r][c] = false;
      prevR[r][c] = -1;
      prevC[r][c] = -1;
    }
  }

  int head = 0;
  int tail = 0;

  queueR[tail] = bombPlayer.row;
  queueC[tail] = bombPlayer.col;
  tail++;

  visited[bombPlayer.row][bombPlayer.col] = true;

  int foundR = -1;
  int foundC = -1;

  while (head < tail) {
    int r = queueR[head];
    int c = queueC[head];
    head++;

if (!(r == bombPlayer.row && c == bombPlayer.col)) {
  if (bombCanStandAt(r, c) &&
      !bombTileInDanger(r, c) &&
      !bombMonsterAt(r, c) &&
      !bombMonsterDangerAt(r, c)) {
    foundR = r;
    foundC = c;
    break;
  }
}

    for (int d = 0; d < 4; d++) {
      int nr = r + BOMB_DIR_DR[d];
      int nc = c + BOMB_DIR_DC[d];

if (!bombInMap(nr, nc)) continue;
if (visited[nr][nc]) continue;
if (!bombCanStandAt(nr, nc)) continue;

// 逃離炸彈時，不要走進怪物所在格
if (bombMonsterAt(nr, nc)) continue;

visited[nr][nc] = true;
      prevR[nr][nc] = r;
      prevC[nr][nc] = c;

      queueR[tail] = nr;
      queueC[tail] = nc;
      tail++;
    }
  }

  if (foundR < 0) return false;

  int cr = foundR;
  int cc = foundC;

  while (!(prevR[cr][cc] == bombPlayer.row && prevC[cr][cc] == bombPlayer.col)) {
    int pr = prevR[cr][cc];
    int pc = prevC[cr][cc];

    if (pr < 0 || pc < 0) break;

    cr = pr;
    cc = pc;
  }

  *outRow = cr;
  *outCol = cc;

  return true;
}

// =====================================================
// 該格是否是主角值得靠近的目標格
// =====================================================
static bool bombIsGoalTile(int row, int col) {
  if (!bombCanStandAt(row, col)) return false;

  // 旁邊有可破壞物件
  for (int d = 0; d < 4; d++) {
    int nr = row + BOMB_DIR_DR[d];
    int nc = col + BOMB_DIR_DC[d];

    if (!bombInMap(nr, nc)) continue;

    if (bombMap[nr][nc] == A) {
      return true;
    }
  }

  // 可以炸到怪物
  for (int i = 0; i < bombMonsterCount; i++) {
    if (!bombMonsters[i].active) continue;

    int mr = bombMonsters[i].row;
    int mc = bombMonsters[i].col;

    if (mr == row && abs(mc - col) <= BOMB_RANGE) {
      if (bombLineClearBetween(row, col, mr, mc)) return true;
    }

    if (mc == col && abs(mr - row) <= BOMB_RANGE) {
      if (bombLineClearBetween(row, col, mr, mc)) return true;
    }

    int manhattan = abs(mr - row) + abs(mc - col);

    if (manhattan == 1) {
      return true;
    }
  }

  return false;
}

// =====================================================
// BFS：尋找往目標前進的下一步
// =====================================================
static bool bombFindStepToGoal(int* outRow, int* outCol) {
  static bool visited[BOMB_MAP_ROWS][BOMB_MAP_COLS];
  static int prevR[BOMB_MAP_ROWS][BOMB_MAP_COLS];
  static int prevC[BOMB_MAP_ROWS][BOMB_MAP_COLS];

  int queueR[BOMB_MAP_ROWS * BOMB_MAP_COLS];
  int queueC[BOMB_MAP_ROWS * BOMB_MAP_COLS];

  for (int r = 0; r < BOMB_MAP_ROWS; r++) {
    for (int c = 0; c < BOMB_MAP_COLS; c++) {
      visited[r][c] = false;
      prevR[r][c] = -1;
      prevC[r][c] = -1;
    }
  }

  int head = 0;
  int tail = 0;

  queueR[tail] = bombPlayer.row;
  queueC[tail] = bombPlayer.col;
  tail++;

  visited[bombPlayer.row][bombPlayer.col] = true;

  int foundR = -1;
  int foundC = -1;

  while (head < tail) {
    int r = queueR[head];
    int c = queueC[head];
    head++;

    if (!(r == bombPlayer.row && c == bombPlayer.col)) {
      if (bombIsGoalTile(r, c)) {
        foundR = r;
        foundC = c;
        break;
      }
    }

    for (int d = 0; d < 4; d++) {
      int nr = r + BOMB_DIR_DR[d];
      int nc = c + BOMB_DIR_DC[d];

      if (!bombInMap(nr, nc)) continue;
      if (visited[nr][nc]) continue;
      if (!bombCanStandAt(nr, nc)) continue;

      visited[nr][nc] = true;
      prevR[nr][nc] = r;
      prevC[nr][nc] = c;

      queueR[tail] = nr;
      queueC[tail] = nc;
      tail++;
    }
  }

  if (foundR < 0) return false;

  int cr = foundR;
  int cc = foundC;

  while (!(prevR[cr][cc] == bombPlayer.row && prevC[cr][cc] == bombPlayer.col)) {
    int pr = prevR[cr][cc];
    int pc = prevC[cr][cc];

    if (pr < 0 || pc < 0) break;

    cr = pr;
    cc = pc;
  }

  *outRow = cr;
  *outCol = cc;

  return true;
}

// =====================================================
// 主角隨機移動一步
// =====================================================
static bool bombMovePlayerRandomStep() {
  int choices[4];
  int choiceCount = 0;

  for (int d = 0; d < 4; d++) {
    int nr = bombPlayer.row + BOMB_DIR_DR[d];
    int nc = bombPlayer.col + BOMB_DIR_DC[d];

if (bombCanStandAt(nr, nc) &&
    !bombMonsterAt(nr, nc) &&
    !bombMonsterDangerAt(nr, nc)) {
  choices[choiceCount] = d;
  choiceCount++;
}
    
  }

  if (choiceCount <= 0) return false;

  int chosenDir = choices[random(choiceCount)];

  int nr = bombPlayer.row + BOMB_DIR_DR[chosenDir];
  int nc = bombPlayer.col + BOMB_DIR_DC[chosenDir];

  bombSetPlayerTarget(nr, nc, chosenDir);

  return true;
}

// =====================================================
// 根據移動差異取得方向
// =====================================================
static int bombDirFromStep(int fromRow, int fromCol, int toRow, int toCol) {
  if (toRow > fromRow) return BOMB_DIR_DOWN;
  if (toRow < fromRow) return BOMB_DIR_UP;
  if (toCol > fromCol) return BOMB_DIR_RIGHT;

  return BOMB_DIR_LEFT;
}

// =====================================================
// 放炸彈
// =====================================================
static void bombPlaceBomb(unsigned long now) {
  if (bombBomb.active) return;

  bombBomb.active = true;
  bombBomb.exploding = false;

  bombBomb.row = bombPlayer.row;
  bombBomb.col = bombPlayer.col;

  bombBomb.placedMs = now;
  bombBomb.explodeStartMs = 0;

  bombBomb.bombAnimStep = 0;
  bombBomb.fireStage = 0;

  bombBomb.lastBombAnimMs = now;
  bombBomb.lastFireStageMs = now;

  bombFocusBombStartMs = 0;       // 重置鏡頭等待時間
  bombFocusBombArrived = false;   // 重置鏡頭到位狀態

  bombPhase = BOMB_PHASE_ESCAPE;
}

// =====================================================
// 加入爆炸火焰格
// =====================================================
static void bombAddFlame(int row, int col, int dir, bool endPart) {
  if (bombFlameCount >= BOMB_MAX_FLAMES) return;

  bombFlames[bombFlameCount].active = true;
  bombFlames[bombFlameCount].row = row;
  bombFlames[bombFlameCount].col = col;
  bombFlames[bombFlameCount].dir = dir;
  bombFlames[bombFlameCount].endPart = endPart;

  bombFlameCount++;
}

// =====================================================
// 加入可破壞物件閃爍
// =====================================================
static void bombAddFlashBlock(int row, int col, unsigned long now) {
  if (bombFlashCount >= BOMB_MAX_FLAMES) return;

  bombFlashBlocks[bombFlashCount].active = true;
  bombFlashBlocks[bombFlashCount].row = row;
  bombFlashBlocks[bombFlashCount].col = col;
  bombFlashBlocks[bombFlashCount].visible = true;
  bombFlashBlocks[bombFlashCount].toggleCount = 0;
  bombFlashBlocks[bombFlashCount].lastToggleMs = now;

  bombFlashCount++;
}

// =====================================================
// 建立爆炸範圍
//
// 中心 + 四方向各 2 格。
// 碰到硬牆停止。
// 碰到可破壞物件時，火焰到該格為止，該格閃爍後消失。
// =====================================================
static void bombBuildExplosion(unsigned long now) {
  bombFlameCount = 0;
  bombFlashCount = 0;

  for (int i = 0; i < BOMB_MAX_FLAMES; i++) {
    bombFlames[i].active = false;
    bombFlashBlocks[i].active = false;
  }

  int br = bombBomb.row;
  int bc = bombBomb.col;

  bombAddFlame(br, bc, -1, false);

  int dr[4] = { 0, 0, 1, -1 };
  int dc[4] = { 1, -1, 0, 0 };

  for (int d = 0; d < 4; d++) {
    for (int dist = 1; dist <= BOMB_RANGE; dist++) {
      int r = br + dr[d] * dist;
      int c = bc + dc[d] * dist;

      if (!bombInMap(r, c)) break;

      uint8_t cell = bombMap[r][c];

      if (bombIsHardBlock(cell)) {
        break;
      }

      bool currentSoft = bombIsSoftBlock(cell);
      bool endPart = false;

      if (dist == BOMB_RANGE) {
        endPart = true;
      } else {
        int nr = br + dr[d] * (dist + 1);
        int nc = bc + dc[d] * (dist + 1);

        if (!bombInMap(nr, nc)) {
          endPart = true;
        } else {
          uint8_t nextCell = bombMap[nr][nc];

          if (bombIsHardBlock(nextCell) || bombIsSoftBlock(nextCell)) {
            endPart = true;
          }
        }
      }

      if (currentSoft) {
        endPart = true;
      }

      bombAddFlame(r, c, d, endPart);

      if (currentSoft) {
        bombAddFlashBlock(r, c, now);
        break;
      }
    }
  }
}

// =====================================================
// 爆炸是否包含指定格
// =====================================================
static bool bombExplosionHasTile(int row, int col) {
  for (int i = 0; i < bombFlameCount; i++) {
    if (!bombFlames[i].active) continue;

    if (bombFlames[i].row == row && bombFlames[i].col == col) {
      return true;
    }
  }

  return false;
}

// =====================================================
// 爆炸打到怪物
// =====================================================
static void bombKillMonstersInExplosion() {
  unsigned long now = millis();

  for (int i = 0; i < bombMonsterCount; i++) {
    if (!bombMonsters[i].active) continue;
    if (bombMonsters[i].dying) continue;

    if (bombExplosionHasTile(bombMonsters[i].row, bombMonsters[i].col)) {
      bombMonsters[i].dying = true;
      bombMonsters[i].visible = true;
      bombMonsters[i].deathBlinkCount = 0;
      bombMonsters[i].lastDeathBlinkMs = now;
      bombMonsters[i].moving = false;
    }
  }
}

// =====================================================
// 是否還有怪物
// =====================================================
static bool bombAnyMonsterAlive() {
  for (int i = 0; i < bombMonsterCount; i++) {
    if (bombMonsters[i].active) return true;
  }

  return false;
}

// =====================================================
// 開始爆炸
// =====================================================
static void bombStartExplosion(unsigned long now) {
  if (!bombBomb.active) return;
  if (bombBomb.exploding) return;

  bombBomb.exploding = true;
  bombBomb.explodeStartMs = now;
  bombBomb.fireStage = 0;
  bombBomb.lastFireStageMs = now;

  bombBuildExplosion(now);
  bombKillMonstersInExplosion();

  bombPhase = BOMB_PHASE_EXPLODING;
}


// =====================================================
// 開始 GAME OVER
// =====================================================
static void bombStartGameOver(unsigned long now) {
  bombPhase = BOMB_PHASE_GAME_OVER;
  bombGameOverStartMs = now;

  // 停止炸彈與爆炸，避免 GAME OVER 畫面被干擾
  bombBomb.active = false;
  bombBomb.exploding = false;
  bombFlameCount = 0;
  bombFlashCount = 0;
}


// =====================================================
// 主角是否碰到怪物
// 使用 pixel 碰撞判斷，避免只靠 row/col 太不準
// =====================================================
static bool bombPlayerHitMonster() {
  int px1 = bombPlayer.x;
  int py1 = bombPlayer.y;
  int px2 = bombPlayer.x + BOMB_TILE_SIZE - 1;
  int py2 = bombPlayer.y + BOMB_TILE_SIZE - 1;

  for (int i = 0; i < bombMonsterCount; i++) {
    if (!bombMonsters[i].active) continue;
    if (bombMonsters[i].dying) continue;

    int mx1 = bombMonsters[i].x;
    int my1 = bombMonsters[i].y;
    int mx2 = bombMonsters[i].x + BOMB_TILE_SIZE - 1;
    int my2 = bombMonsters[i].y + BOMB_TILE_SIZE - 1;

    bool overlap =
      px1 <= mx2 &&
      px2 >= mx1 &&
      py1 <= my2 &&
      py2 >= my1;

    if (overlap) {
      return true;
    }
  }

  return false;
}


// =====================================================
// 更新可破壞物件閃爍
// 閃爍 3 次後變成可通行地板
// =====================================================
static void bombUpdateFlashBlocks(unsigned long now) {
  for (int i = 0; i < bombFlashCount; i++) {
    if (!bombFlashBlocks[i].active) continue;

    if (now - bombFlashBlocks[i].lastToggleMs >= BOMB_SOFT_BLINK_INTERVAL_MS) {
      bombFlashBlocks[i].lastToggleMs = now;
      bombFlashBlocks[i].visible = !bombFlashBlocks[i].visible;
      bombFlashBlocks[i].toggleCount++;

      if (bombFlashBlocks[i].toggleCount >= 6) {
        int r = bombFlashBlocks[i].row;
        int c = bombFlashBlocks[i].col;

        if (bombInMap(r, c) && bombMap[r][c] == A) {
          bombMap[r][c] = 0;
        }

        bombFlashBlocks[i].active = false;
      }
    }
  }
}

// =====================================================
// 結束爆炸
// =====================================================
static void bombFinishExplosion(unsigned long now) {
  for (int i = 0; i < bombFlashCount; i++) {
    if (!bombFlashBlocks[i].active) continue;

    int r = bombFlashBlocks[i].row;
    int c = bombFlashBlocks[i].col;

    if (bombInMap(r, c) && bombMap[r][c] == A) {
      bombMap[r][c] = 0;
    }

    bombFlashBlocks[i].active = false;
  }

  bombFlashCount = 0;
  bombFlameCount = 0;

  bombBomb.active = false;
  bombBomb.exploding = false;

  if (!bombAnyMonsterAlive()) {
    bombPhase = BOMB_PHASE_WIN;
    bombWinStartMs = now;
  } else {
    bombPhase = BOMB_PHASE_WALK;
  }
}

// =====================================================
// 更新炸彈與爆炸
// =====================================================
static void bombUpdateBomb(unsigned long now) {
  if (!bombBomb.active) return;

if (!bombBomb.exploding) {
  if (now - bombBomb.lastBombAnimMs >= BOMB_BOMB_ANIM_INTERVAL_MS) {
    bombBomb.lastBombAnimMs = now;
    bombBomb.bombAnimStep++;

    if (bombBomb.bombAnimStep >= BOMB_BOMB_SEQ_LEN) {
      bombBomb.bombAnimStep = 0;
    }
  }

  // 保險機制：
  // 如果主角卡住、Camera 沒進入炸彈鎖定流程，
  // 炸彈等待太久就強制爆炸，避免主題卡死。
  if (now - bombBomb.placedMs >= BOMB_FORCE_EXPLODE_MS) {
    bombStartExplosion(now);
  }

  return;
}

  bombUpdateFlashBlocks(now);

  if (now - bombBomb.lastFireStageMs >= BOMB_FIRE_STAGE_INTERVAL_MS) {
    bombBomb.lastFireStageMs = now;
    bombBomb.fireStage++;

    if (bombBomb.fireStage > 1) {
      bombBomb.fireStage = 1;
    }
  }

  if (now - bombBomb.explodeStartMs >= BOMB_EXPLOSION_TOTAL_MS) {
    bombFinishExplosion(now);
  }
}


// =====================================================
// 主角 AI
//
// 優先順序：
// 1. 可以炸怪物，就先炸怪物
// 2. 怪物太近，先閃避
// 3. 找可以炸怪物的位置
// 4. 炸可破壞物件 A
// 5. 沒目標就隨機移動
// =====================================================
static void bombUpdatePlayerAi(unsigned long now) {
  if (bombPlayer.moving) return;

  if (bombPhase == BOMB_PHASE_WALK) {

    // 已經站在可以炸到怪物的位置，優先放炸彈
    if (bombCanBombMonsterFrom(bombPlayer.row, bombPlayer.col)) {
      bombPlaceBomb(now);
      return;
    }

    // 怪物太靠近時，優先閃避
    if (bombMonsterDangerAt(bombPlayer.row, bombPlayer.col)) {
      int safeR = 0;
      int safeC = 0;

      if (bombFindSafeFromMonsterStep(&safeR, &safeC)) {
        int dir = bombDirFromStep(bombPlayer.row, bombPlayer.col, safeR, safeC);
        bombSetPlayerTarget(safeR, safeC, dir);
        return;
      }
    }

    // 主動尋找可以炸怪物的位置
    int monsterR = 0;
    int monsterC = 0;

    if (bombFindStepToMonsterAttack(&monsterR, &monsterC)) {
      int dir = bombDirFromStep(bombPlayer.row, bombPlayer.col, monsterR, monsterC);
      bombSetPlayerTarget(monsterR, monsterC, dir);
      return;
    }

    // 沒有怪物目標時，才處理可破壞物件
    if (bombPlayerNearSoftBlock()) {
      bombPlaceBomb(now);
      return;
    }

    int nr = 0;
    int nc = 0;

    if (bombFindStepToGoal(&nr, &nc)) {
      int dir = bombDirFromStep(bombPlayer.row, bombPlayer.col, nr, nc);
      bombSetPlayerTarget(nr, nc, dir);
      return;
    }

    bombMovePlayerRandomStep();
    return;
  }

if (bombPhase == BOMB_PHASE_ESCAPE) {
  if (!bombTileInDanger(bombPlayer.row, bombPlayer.col) &&
      !bombMonsterDangerAt(bombPlayer.row, bombPlayer.col)) {
    bombPhase = BOMB_PHASE_FOCUS_BOMB;
    bombFocusBombStartMs = 0;
    bombFocusBombArrived = false;
    return;
  }

  int safeR = 0;
  int safeC = 0;

  // 優先找能同時避開炸彈與怪物的路
  if (bombFindSafeFromMonsterStep(&safeR, &safeC)) {
    int dir = bombDirFromStep(bombPlayer.row, bombPlayer.col, safeR, safeC);
    bombSetPlayerTarget(safeR, safeC, dir);
    return;
  }

  // 找不到才只避開炸彈
  if (bombFindSafeStep(&safeR, &safeC)) {
    int dir = bombDirFromStep(bombPlayer.row, bombPlayer.col, safeR, safeC);
    bombSetPlayerTarget(safeR, safeC, dir);
    return;
  }

  bombMovePlayerRandomStep();
  return;
 }
}

// =====================================================
// 更新 Camera
//
// 平常鎖定主角。
// 放炸彈後，主角先逃離爆炸範圍。
// 主角安全後，鏡頭移動鎖定炸彈。
// 爆炸時持續鎖定炸彈。
// =====================================================
static void bombUpdateCamera(unsigned long now) {
  if (now - bombLastCameraMs < BOMB_CAMERA_INTERVAL_MS) return;

  bombLastCameraMs = now;

  int focusX = bombPlayer.x + BOMB_TILE_SIZE / 2;
  int focusY = bombPlayer.y + BOMB_TILE_SIZE / 2;

  if (bombPhase == BOMB_PHASE_FOCUS_BOMB ||
      bombPhase == BOMB_PHASE_EXPLODING) {
    focusX = bombBomb.col * BOMB_TILE_SIZE + BOMB_TILE_SIZE / 2;
    focusY = bombBomb.row * BOMB_TILE_SIZE + BOMB_TILE_SIZE / 2;
  }

  bombTargetCamX = focusX - BOMB_SCR_W / 2;
  bombTargetCamY = focusY - BOMB_SCR_H / 2 - 8;   // 主角稍微往下

  int maxCamX = BOMB_MAP_COLS * BOMB_TILE_SIZE - BOMB_SCR_W;
  int maxCamY = BOMB_MAP_ROWS * BOMB_TILE_SIZE - BOMB_SCR_H;

  if (maxCamX < 0) maxCamX = 0;
  if (maxCamY < 0) maxCamY = 0;

  bombTargetCamX = bombClampInt(bombTargetCamX, 0, maxCamX);
  bombTargetCamY = bombClampInt(bombTargetCamY, 0, maxCamY);

  int camStep = 1;

  if (bombCamX < bombTargetCamX) bombCamX += camStep;
  if (bombCamX > bombTargetCamX) bombCamX -= camStep;

  if (bombCamY < bombTargetCamY) bombCamY += camStep;
  if (bombCamY > bombTargetCamY) bombCamY -= camStep;

  if (abs(bombCamX - bombTargetCamX) < camStep) bombCamX = bombTargetCamX;
  if (abs(bombCamY - bombTargetCamY) < camStep) bombCamY = bombTargetCamY;

  bombCamX = bombClampInt(bombCamX, 0, maxCamX);
  bombCamY = bombClampInt(bombCamY, 0, maxCamY);
}


// =====================================================
// 判斷 Camera 是否已經移動到目標位置
// =====================================================
static bool bombCameraAtTarget() {
  return abs(bombCamX - bombTargetCamX) <= 1 &&
         abs(bombCamY - bombTargetCamY) <= 1;
}



// =====================================================
// 鏡頭鎖定炸彈後，等待一段時間才爆炸
// =====================================================
static void bombUpdateFocusBombExplosion(unsigned long now) {
  if (bombPhase != BOMB_PHASE_FOCUS_BOMB) return;
  if (!bombBomb.active) return;
  if (bombBomb.exploding) return;

  // 如果主角又回到爆炸危險範圍，先回到逃離狀態
  if (bombTileInDanger(bombPlayer.row, bombPlayer.col) || bombPlayer.moving) {
    bombPhase = BOMB_PHASE_ESCAPE;
    bombFocusBombStartMs = 0;
    bombFocusBombArrived = false;
    return;
  }

  // Camera 還沒移到炸彈位置，不爆炸
  if (!bombCameraAtTarget()) {
    bombFocusBombStartMs = 0;
    bombFocusBombArrived = false;
    return;
  }

  // Camera 第一次到位，開始計時
  if (!bombFocusBombArrived) {
    bombFocusBombArrived = true;
    bombFocusBombStartMs = now;
    return;
  }

  // Camera 到位後等待指定時間
  if (now - bombFocusBombStartMs < BOMB_CAMERA_FOCUS_HOLD_MS) {
    return;
  }

  // 保留炸彈最短引爆時間
  // 如果你想完全只看 Camera 到位，不看 BOMB_FUSE_MS，可以刪掉這段。
  if (now - bombBomb.placedMs < BOMB_FUSE_MS) {
    return;
  }

  bombStartExplosion(now);
}

// =====================================================
// 載入地圖
//
// 同一張 BOMB_MAP 會隨機使用三種格局：
// 0 = 原圖
// 1 = 水平翻轉
// 2 = 垂直翻轉
//
// B 是怪物生成點，實際地圖會轉成 0 地板。
// 第一個 B 保留給主角，不產生怪物。
// =====================================================
static void bombLoadMap() {
  bombSpawnCount = 0;

  bombLayoutMode = random(3);

  bombMapFlipX = bombLayoutMode == 1;
  bombMapFlipY = bombLayoutMode == 2;

  bool firstBReserved = false;

  for (int r = 0; r < BOMB_MAP_ROWS; r++) {
    for (int c = 0; c < BOMB_MAP_COLS; c++) {
      int srcR = r;
      int srcC = c;

      if (bombMapFlipY) {
        srcR = BOMB_MAP_ROWS - 1 - r;
      }

      if (bombMapFlipX) {
        srcC = BOMB_MAP_COLS - 1 - c;
      }

      uint8_t cell = pgm_read_byte(&BOMB_MAP[srcR][srcC]);

      if (cell == B) {
        bombMap[r][c] = 0;

        if (!firstBReserved) {
          firstBReserved = true;
        } else {
          if (bombSpawnCount < BOMB_MAX_SPAWNS) {
            bombSpawnRows[bombSpawnCount] = r;
            bombSpawnCols[bombSpawnCount] = c;
            bombSpawnCount++;
          }
        }
      } else {
        bombMap[r][c] = cell;
      }
    }
  }

  // 主角固定生成在 (32,32)，強制該格變成可通行地板。
  if (bombInMap(BOMB_PLAYER_START_ROW, BOMB_PLAYER_START_COL)) {
    bombMap[BOMB_PLAYER_START_ROW][BOMB_PLAYER_START_COL] = 0;
  }
}


// =====================================================
// 隨機選擇地圖圖資
// =====================================================
static void bombChooseDataSheet() {
  int dataType = random(4);

  if (dataType == 0) {
    currentBombDataSheet = BOMB_DATA;
    currentBombDataPalette = BOMB_DATA_PALETTE;
  } else if (dataType == 1) {
    currentBombDataSheet = BOMB_DATA1;
    currentBombDataPalette = BOMB_DATA1_PALETTE;
  } else if (dataType == 2) {
    currentBombDataSheet = BOMB_DATA2;
    currentBombDataPalette = BOMB_DATA2_PALETTE;
  } else {
    currentBombDataSheet = BOMB_DATA3;
    currentBombDataPalette = BOMB_DATA3_PALETTE;
  }
}

// =====================================================
// 初始化主角
// 隨機選 BOMB_MAN_A 或 BOMB_MAN_B
// =====================================================
static void bombInitPlayer(unsigned long now) {
  int playerType = random(3);

if (playerType == 0) {
  bombPlayer.sheet = BOMB_MAN_A;
  bombPlayer.palette = BOMB_MAN_A_PALETTE;
} else if (playerType == 1) {
  bombPlayer.sheet = BOMB_MAN_B;
  bombPlayer.palette = BOMB_MAN_B_PALETTE;
} else {
  bombPlayer.sheet = BOMB_MAN_C;
  bombPlayer.palette = BOMB_MAN_C_PALETTE;
}

  bombPlayer.row = BOMB_PLAYER_START_ROW;
  bombPlayer.col = BOMB_PLAYER_START_COL;

  bombPlayer.x = bombPlayer.col * BOMB_TILE_SIZE;
  bombPlayer.y = bombPlayer.row * BOMB_TILE_SIZE;

  bombPlayer.targetRow = bombPlayer.row;
  bombPlayer.targetCol = bombPlayer.col;
  bombPlayer.targetX = bombPlayer.x;
  bombPlayer.targetY = bombPlayer.y;

  bombPlayer.dir = BOMB_DIR_DOWN;
  bombPlayer.animStep = 0;
  bombPlayer.moving = false;

  bombPlayer.lastMoveMs = now;
  bombPlayer.lastAnimMs = now;
}

// =====================================================
// 初始化怪物
// 從 B 生成點隨機生成怪物
// 左上角第一個 B 已保留給主角，不會出怪物
// =====================================================
static void bombInitMonsters(unsigned long now) {
  bombMonsterCount = 0;

  for (int i = 0; i < BOMB_MAX_MONSTERS; i++) {
    bombMonsters[i].active = false;
  }

  for (int i = 0; i < bombSpawnCount; i++) {
    if (bombMonsterCount >= BOMB_MAX_MONSTERS) break;

    if (random(100) < 70) {
      int idx = bombMonsterCount;

      bombMonsters[idx].active = true;
      bombMonsters[idx].type = random(BOMB_MONSTER_TYPE_COUNT);
      
      bombMonsters[idx].dying = false;
      bombMonsters[idx].visible = true;
      bombMonsters[idx].deathBlinkCount = 0;
      bombMonsters[idx].lastDeathBlinkMs = now;

      bombMonsters[idx].row = bombSpawnRows[i];
      bombMonsters[idx].col = bombSpawnCols[i];

      bombMonsters[idx].x = bombMonsters[idx].col * BOMB_TILE_SIZE;
      bombMonsters[idx].y = bombMonsters[idx].row * BOMB_TILE_SIZE;

      bombMonsters[idx].targetRow = bombMonsters[idx].row;
      bombMonsters[idx].targetCol = bombMonsters[idx].col;
      bombMonsters[idx].targetX = bombMonsters[idx].x;
      bombMonsters[idx].targetY = bombMonsters[idx].y;

      bombMonsters[idx].dir = random(4);
      bombMonsters[idx].animStep = 0;
      bombMonsters[idx].moving = false;

      bombMonsters[idx].lastMoveMs = now;
      bombMonsters[idx].lastAnimMs = now;

      bombMonsterCount++;
    }
  }

  // 避免完全沒有怪物
  if (bombMonsterCount == 0 && bombSpawnCount > 0) {
    bombMonsters[0].active = true;
    bombMonsters[0].type = random(BOMB_MONSTER_TYPE_COUNT);

    bombMonsters[0].row = bombSpawnRows[0];
    bombMonsters[0].col = bombSpawnCols[0];

    bombMonsters[0].x = bombMonsters[0].col * BOMB_TILE_SIZE;
    bombMonsters[0].y = bombMonsters[0].row * BOMB_TILE_SIZE;

    bombMonsters[0].targetRow = bombMonsters[0].row;
    bombMonsters[0].targetCol = bombMonsters[0].col;
    bombMonsters[0].targetX = bombMonsters[0].x;
    bombMonsters[0].targetY = bombMonsters[0].y;

    bombMonsters[0].dir = random(4);
    bombMonsters[0].animStep = 0;
    bombMonsters[0].moving = false;

    bombMonsters[0].lastMoveMs = now;
    bombMonsters[0].lastAnimMs = now;

    bombMonsterCount = 1;
  }
}

// =====================================================
// 初始化炸彈
// =====================================================
static void bombInitBomb(unsigned long now) {
  bombBomb.active = false;
  bombBomb.exploding = false;

  bombBomb.row = 0;
  bombBomb.col = 0;

  bombBomb.placedMs = now;
  bombBomb.explodeStartMs = now;

  bombBomb.bombAnimStep = 0;
  bombBomb.fireStage = 0;

  bombBomb.lastBombAnimMs = now;
  bombBomb.lastFireStageMs = now;

  bombFlameCount = 0;
  bombFlashCount = 0;

  for (int i = 0; i < BOMB_MAX_FLAMES; i++) {
    bombFlames[i].active = false;
    bombFlashBlocks[i].active = false;
  }
}

// =====================================================
// 初始化 Camera
// =====================================================
static void bombInitCamera(unsigned long now) {
  int focusX = bombPlayer.x + BOMB_TILE_SIZE / 2;
  int focusY = bombPlayer.y + BOMB_TILE_SIZE / 2;

  bombCamX = focusX - BOMB_SCR_W / 2;
  bombCamY = focusY - BOMB_SCR_H / 2;

  int maxCamX = BOMB_MAP_COLS * BOMB_TILE_SIZE - BOMB_SCR_W;
  int maxCamY = BOMB_MAP_ROWS * BOMB_TILE_SIZE - BOMB_SCR_H;

  if (maxCamX < 0) maxCamX = 0;
  if (maxCamY < 0) maxCamY = 0;

  bombCamX = bombClampInt(bombCamX, 0, maxCamX);
  bombCamY = bombClampInt(bombCamY, 0, maxCamY);

  bombTargetCamX = bombCamX;
  bombTargetCamY = bombCamY;

  bombLastCameraMs = now;
}

// =====================================================
// Bomb 初始化函式
// =====================================================
void Bomb_Init() {
  unsigned long now = millis();

  bombChooseDataSheet();  
  bombLoadMap();
  bombInitPlayer(now);
  bombInitMonsters(now);
  bombInitBomb(now);
  bombInitCamera(now);

  bombPhase = BOMB_PHASE_WALK;
  bombWinStartMs = now;
}

// =====================================================
// 更新遊戲邏輯
// =====================================================
// =====================================================
// 更新遊戲邏輯
// =====================================================
static void bombUpdate(unsigned long now) {
  if (bombPhase == BOMB_PHASE_WIN) {
    if (now - bombWinStartMs >= BOMB_WIN_HOLD_MS) {
      Bomb_Init();
    }

    bombUpdateCamera(now);
    return;
  }

  if (bombPhase == BOMB_PHASE_GAME_OVER) {
    if (now - bombGameOverStartMs >= BOMB_GAME_OVER_HOLD_MS) {
      Bomb_Init();
    }

    bombUpdateCamera(now);
    
    return;
  }

bombUpdateBomb(now);
bombUpdateMonsterDeathBlink(now);

// 主角如果在爆炸火焰範圍內，直接 GAME OVER
if (bombBomb.active &&
    bombBomb.exploding &&
    bombExplosionHasTile(bombPlayer.row, bombPlayer.col)) {
  bombStartGameOver(now);
  return;
}


  if (bombPhase != BOMB_PHASE_EXPLODING) {
    bombUpdateMonsters(now);
    bombUpdatePlayerAi(now);
  }

  bombUpdatePlayerMove(now);

  // 主角碰到怪物，GAME OVER 後重新開始
  if (bombPlayerHitMonster()) {
    bombStartGameOver(now);
    return;
  }



  bombUpdateCamera(now);
  bombUpdateFocusBombExplosion(now);
}
// =====================================================
// 重繪畫面
// =====================================================
static void bombRender() {
  display.fillScreen(0x0000);

bombDrawMap();

bombDrawBomb();
bombDrawMonsters();
bombDrawPlayer();

bombDrawExplosion();
bombDrawDyingMonsters();

bombDrawWinText();
bombDrawGameOverText();
}

// =====================================================
// 主要運行函式
// =====================================================
void BombMode() {
  if (ModefirstRun) {
    randomSeed(millis());
    Bomb_Init();
    ModefirstRun = false;
  }

  unsigned long now = millis();

  bombUpdate(now);
  bombRender();
  drawThemeClockText();

  wait_with_display(30);
}
