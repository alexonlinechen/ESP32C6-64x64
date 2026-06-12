
#include "drill.h"

// =====================================================
// 螢幕設定
// =====================================================
static const int DRILL_SCR_W = 64;
static const int DRILL_SCR_H = 64;

// 人物和 BLOCK 圖資背景色
static const uint16_t DRILL_BG_COLOR = 0x3940;
static const uint16_t DRILL_TRANSPARENT = 0x4e84;

// =====================================================
// Driller 圖資設定
// DRILLER[] 133x22，共 7 frame，每張 19x22
// =====================================================
static const int DRILLER_SHEET_W = 133;
static const int DRILLER_FRAME_W = 19;
static const int DRILLER_FRAME_H = 22;

// 人物視覺中心點。
// 如果你的角色看起來左右或上下偏移，可以微調這兩個值。
static const int DRILLER_CENTER_OFFSET_X = 9;
static const int DRILLER_CENTER_OFFSET_Y = 11;

// =====================================================
// BLOCK 圖資設定
// DRILL_BLOCK[] 112x16，共 7 frame，每張 16x16
// =====================================================
static const int DRILL_BLOCK_SHEET_W = 112;
static const int DRILL_BLOCK_FRAME_W = 16;
static const int DRILL_BLOCK_FRAME_H = 16;

static const int DRILL_TILE_SIZE = 16;

// camY = 0 時，第 0 row 會畫在螢幕 y = 32
static const int DRILL_BLOCK_WORLD_Y0 = 32;

// =====================================================
// 地圖設定
// =====================================================
static const int DRILL_MAP_COLS = 4;
static const int DRILL_MAP_ROWS = 96;

static const uint8_t DRILL_CELL_FIXED = 0;
static const uint8_t DRILL_CELL_BLUE = 1;
static const uint8_t DRILL_CELL_RED = 2;
static const uint8_t DRILL_CELL_GREEN = 3;
static const uint8_t DRILL_CELL_YELLOW = 4;
static const uint8_t DRILL_CELL_AIR = 5;
static const uint8_t DRILL_CELL_HEART = 6;

// 空洞，不對應圖資 frame
static const uint8_t DRILL_CELL_EMPTY = 255;

// =====================================================
// 可調參數
// =====================================================
static const unsigned long DRILL_X_MOVE_INTERVAL_MS = 35;
static const unsigned long DRILL_Y_MOVE_INTERVAL_MS = 38;
static const unsigned long DRILL_ANIM_INTERVAL_MS = 110;

// 墜落速度。數字越小掉越快，建議 22~30。
static const unsigned long DRILL_FALL_MOVE_INTERVAL_MS = 24;


// 每鑽完一格後停一下，讓畫面比較像遊戲節奏
static const unsigned long DRILL_LAYER_HOLD_MS = 360;

// 顏色 BLOCK 閃爍次數
static const int DRILL_BLOCK_FLASH_COUNT = 4;
static const unsigned long DRILL_BLOCK_FLASH_INTERVAL_MS = 85;

// 同色相連 BLOCK 最多一次消除幾顆。
static const int DRILL_FLASH_GROUP_MAX = 48;

// 空氣出現
static const int DRILL_AIR_ROWS_AHEAD = 5;

// Camera
static const unsigned long DRILL_CAMERA_INTERVAL_MS = 30;
static const int DRILL_CAMERA_ANCHOR_Y = 15;
static const int DRILL_CAMERA_SOFT_LIMIT_Y = 52;
static const int DRILL_CAMERA_NORMAL_STEP = 1;
static const int DRILL_CAMERA_RECENTER_STEP = 2;

// 鑽幾層後，Camera 把主角平滑拉回 y=15
static const int DRILL_ROWS_BEFORE_RECENTER = 2;

// 起始設定
static const int DRILL_START_COL = 1;
static const int DRILL_START_SCREEN_Y = 15;

// 開場在上方自由左右移動幾次後才開始鑽
static const int DRILL_TOP_FREE_SIDE_MOVES = 1;

// 如果被固定磚卡住，原地等一下再重選，不破壞固定磚
static const unsigned long DRILL_STUCK_RETRY_MS = 600;


// 卡住超過多久就整個重新初始化。
// 可調整，例如 3000、5000、8000。
static const unsigned long DRILL_STUCK_RESET_MS = 5000;

static bool drillStuckTimerActive = false;
static unsigned long drillStuckStartMs = 0;

// 體力條設定
static const int DRILL_HP_MAX = 64;
static const int DRILL_HP_LOW_THRESHOLD = 20;

// 主角每向下移動幾層扣 1 點體力，可調整。
static const int DRILL_HP_DOWN_ROWS_PER_DAMAGE = 1;

static const uint16_t DRILL_HP_COLOR_GREEN = 0x07E0;
static const uint16_t DRILL_HP_COLOR_RED = 0xF800;
static const uint16_t DRILL_HP_COLOR_BG = 0x0000;
static const unsigned long DRILL_HP_BLINK_MS = 220;

// 挖到第幾層出現愛心，可調整。
static const long DRILL_WIN_DEPTH_ROWS = 70;

// game over / win 顯示後等待多久再重新初始化。
static const unsigned long DRILL_RESULT_WAIT_MS = 3000;


// =====================================================
// 流程狀態
// =====================================================
enum DrillPhase {
  DRILL_PHASE_CHOOSE = 0,
  DRILL_PHASE_BREAK_BLOCK,
  DRILL_PHASE_MOVE_X,
  DRILL_PHASE_MOVE_Y,
  DRILL_PHASE_HOLD,
  DRILL_PHASE_RECENTER,
  DRILL_PHASE_STUCK_WAIT,
  DRILL_PHASE_GAME_OVER,
  DRILL_PHASE_WIN
};

static DrillPhase drillPhase = DRILL_PHASE_CHOOSE;

// =====================================================
// 動作類型
// =====================================================
static const uint8_t DRILL_ACTION_NONE = 0;
static const uint8_t DRILL_ACTION_SIDE = 1;
static const uint8_t DRILL_ACTION_DOWN = 2;

// =====================================================
// 主角狀態
// =====================================================
struct DrillPlayerState {
  int row;       // -1 代表還在第一層 BLOCK 上方
  int col;

  int worldX;
  int worldY;

  int targetRow;
  int targetCol;
  int targetWorldX;
  int targetWorldY;

  int dir;       // -1 = 左，0 = 往下 / 待機，1 = 右
  int animStep;

  unsigned long lastMoveMs;
  unsigned long lastAnimMs;
};

static DrillPlayerState drillPlayer;
static const uint16_t* drillActiveDrillerSheet = DRILLER;

// =====================================================
// 目標動作
// =====================================================
struct DrillActionState {
  uint8_t kind;
  int row;
  int col;
  int dir;
};

static DrillActionState drillAction;

// =====================================================
// BLOCK 閃爍狀態
// =====================================================
struct DrillBlockFlashState {
  bool active;

  uint8_t color;

  // 同色消除的最上方 row。
  // 低於這個 row 才能加入閃爍群組。
  // 這樣可以避免主角上方的 BLOCK 一起閃爍消失。
  int minRow;

  bool visible;
  int toggleCount;
  unsigned long lastToggleMs;

  int count;
  uint8_t rows[DRILL_FLASH_GROUP_MAX];
  uint8_t cols[DRILL_FLASH_GROUP_MAX];
};

static DrillBlockFlashState drillBlockFlash;

// =====================================================
// 地圖與 Camera
// =====================================================
static uint8_t drillMap[DRILL_MAP_ROWS][DRILL_MAP_COLS];

static int drillCamY = 0;
static int drillRowsSinceRecenter = 0;
static long drillDepthRows = 0;

static int drillTopFreeMovesLeft = DRILL_TOP_FREE_SIDE_MOVES;

static unsigned long drillHoldStartMs = 0;
static unsigned long drillLastCameraMs = 0;
static unsigned long drillLastAirSpawnMs = 0;

static int drillHp = DRILL_HP_MAX;
static int drillRowsSinceHpDamage = 0;
static unsigned long drillResultStartMs = 0;

static bool drillHeartSpawned = false;
static int drillHeartRow = -1;
static int drillHeartCol = -1;

// =====================================================
// 工具函式
// =====================================================
static int drillClampInt(int v, int minV, int maxV) {
  if (v < minV) return minV;
  if (v > maxV) return maxV;
  return v;
}

static bool drillInMap(int row, int col) {
  return row >= 0 && row < DRILL_MAP_ROWS &&
         col >= 0 && col < DRILL_MAP_COLS;
}

static bool drillIsColorBlock(uint8_t cell) {
  return cell >= DRILL_CELL_BLUE && cell <= DRILL_CELL_YELLOW;
}

static bool drillIsFixedBlock(uint8_t cell) {
  return cell == DRILL_CELL_FIXED;
}

static bool drillIsAir(uint8_t cell) {
  return cell == DRILL_CELL_AIR;
}

static bool drillIsHeart(uint8_t cell) {
  return cell == DRILL_CELL_HEART;
}

static bool drillIsBreakableBlock(uint8_t cell) {
  return drillIsColorBlock(cell) || drillIsHeart(cell);
}

static void drillAddHp(int amount) {
  drillHp += amount;

  if (drillHp > DRILL_HP_MAX) {
    drillHp = DRILL_HP_MAX;
  }

  if (drillHp < 0) {
    drillHp = 0;
  }
}

static uint8_t drillRandomColorBlock() {
  return (uint8_t)random(DRILL_CELL_BLUE, DRILL_CELL_YELLOW + 1);
}

static uint8_t drillRandomMapCell() {
  int r = random(100);

  // 固定磚比例不要太高，否則 64x64 畫面很容易卡住。
  if (r < 6) {
    return DRILL_CELL_FIXED;
  }

  return drillRandomColorBlock();
}

// BLOCK 中心 x = col * 16 + 8
// Driller 中心 x = worldX + DRILLER_CENTER_OFFSET_X
static int drillPlayerXForCol(int col) {
  col = drillClampInt(col, 0, DRILL_MAP_COLS - 1);

  int blockCenterX = col * DRILL_TILE_SIZE + DRILL_TILE_SIZE / 2;
  return blockCenterX - DRILLER_CENTER_OFFSET_X;
}

// BLOCK 中心 y = 32 + row * 16 + 8
// Driller 中心 y = worldY + DRILLER_CENTER_OFFSET_Y
static int drillPlayerYForRow(int row) {
  int blockCenterY =
    DRILL_BLOCK_WORLD_Y0 +
    row * DRILL_TILE_SIZE +
    DRILL_TILE_SIZE / 2;

  return blockCenterY - DRILLER_CENTER_OFFSET_Y;
}

static int drillPlayerScreenY() {
  return drillPlayer.worldY - drillCamY;
}


static void drillStartStuckTimer(unsigned long now) {
  if (!drillStuckTimerActive) {
    drillStuckTimerActive = true;
    drillStuckStartMs = now;
  }
}

static void drillClearStuckTimer() {
  drillStuckTimerActive = false;
  drillStuckStartMs = 0;
}



static void drillStartGameOver(unsigned long now) {
  if (drillPhase == DRILL_PHASE_GAME_OVER ||
      drillPhase == DRILL_PHASE_WIN) {
    return;
  }

  drillPhase = DRILL_PHASE_GAME_OVER;
  drillHp = 0;
  drillResultStartMs = now;

  Serial.println("game over");
  GameOverText();        
  wait_with_display(50000);        
  ClearEffect();    // 播放過場動畫
  wait_with_display(3000);
  Drill_Init();             
  return;
}

static void drillStartWin(unsigned long now) {
  if (drillPhase == DRILL_PHASE_GAME_OVER ||
      drillPhase == DRILL_PHASE_WIN) {
    return;
  }

  drillPhase = DRILL_PHASE_WIN;
  drillResultStartMs = now;

  Serial.println("you win");

  WinText();             
  wait_with_display(50000);        
  ClearEffect(); 
  wait_with_display(3000);
  Drill_Init();              
  return;
}

// =====================================================
// 地圖產生
// =====================================================
static int drillSafeCol = DRILL_START_COL;

static void drillGenerateRow(int row) {
  if (row < 0 || row >= DRILL_MAP_ROWS) return;

  int fixedCount = 0;

  for (int c = 0; c < DRILL_MAP_COLS; c++) {
    drillMap[row][c] = drillRandomMapCell();

    if (drillMap[row][c] == DRILL_CELL_FIXED) {
      fixedCount++;
    }
  }

  // -------------------------------------------------
  // 安全通道：
  // 讓通道偶爾左右偏移，像自然地形，
  // 但保證這一層一定有一格不是固定磚。
  // -------------------------------------------------
  int move = random(3) - 1;   // -1, 0, 1
  drillSafeCol += move;
  drillSafeCol = drillClampInt(drillSafeCol, 0, DRILL_MAP_COLS - 1);

  drillMap[row][drillSafeCol] = drillRandomColorBlock();

  // -------------------------------------------------
  // 避免整排都是固定磚。
  // 理論上安全通道已經避免了，但保留保險。
  // -------------------------------------------------
  if (fixedCount >= DRILL_MAP_COLS) {
    drillMap[row][drillSafeCol] = drillRandomColorBlock();
  }

  // -------------------------------------------------
  // 額外優化：
  // 讓安全通道旁邊也偶爾可破壞，
  // 避免通道太窄，看起來不像遊戲。
  // -------------------------------------------------
  if (random(100) < 55) {
    int sideCol = drillSafeCol + (random(2) == 0 ? -1 : 1);

    if (sideCol >= 0 && sideCol < DRILL_MAP_COLS) {
      if (drillMap[row][sideCol] == DRILL_CELL_FIXED) {
        drillMap[row][sideCol] = drillRandomColorBlock();
      }
    }
  }
}

static void drillGenerateMap() {
  for (int r = 0; r < DRILL_MAP_ROWS; r++) {
    drillGenerateRow(r);
  }

  // 起始附近不要放固定磚，讓開場一定能動。
  for (int r = 0; r < 4; r++) {
    drillMap[r][DRILL_START_COL] = drillRandomColorBlock();

    if (DRILL_START_COL > 0) {
      drillMap[r][DRILL_START_COL - 1] = drillRandomColorBlock();
    }

    if (DRILL_START_COL < DRILL_MAP_COLS - 1) {
      drillMap[r][DRILL_START_COL + 1] = drillRandomColorBlock();
    }
  }
}

static void drillRecycleMapIfNeeded() {
  if (drillPlayer.row < DRILL_MAP_ROWS - 20) return;

  int shiftRows = DRILL_MAP_ROWS / 2;
  int shiftPixels = shiftRows * DRILL_TILE_SIZE;

  for (int r = 0; r < DRILL_MAP_ROWS - shiftRows; r++) {
    for (int c = 0; c < DRILL_MAP_COLS; c++) {
      drillMap[r][c] = drillMap[r + shiftRows][c];
    }
  }

  for (int r = DRILL_MAP_ROWS - shiftRows; r < DRILL_MAP_ROWS; r++) {
    drillGenerateRow(r);
  }

  drillPlayer.row -= shiftRows;
  drillPlayer.targetRow -= shiftRows;

  drillAction.row -= shiftRows;
  
if (drillHeartSpawned) {
  drillHeartRow -= shiftRows;

  if (drillHeartRow < 0 || drillHeartRow >= DRILL_MAP_ROWS) {
    drillHeartSpawned = false;
    drillHeartRow = -1;
    drillHeartCol = -1;
  }
}

  drillPlayer.worldY -= shiftPixels;
  drillPlayer.targetWorldY -= shiftPixels;

  drillCamY -= shiftPixels;
  if (drillCamY < 0) drillCamY = 0;

if (drillBlockFlash.active) {
  drillBlockFlash.minRow -= shiftRows;
  if (drillBlockFlash.minRow < 0) {
    drillBlockFlash.minRow = 0;
  }

  int newCount = 0;

  for (int i = 0; i < drillBlockFlash.count; i++) {
    int rr = (int)drillBlockFlash.rows[i] - shiftRows;
    int cc = drillBlockFlash.cols[i];

    if (rr >= 0 && rr < DRILL_MAP_ROWS) {
      drillBlockFlash.rows[newCount] = (uint8_t)rr;
      drillBlockFlash.cols[newCount] = (uint8_t)cc;
      newCount++;
    }
  }

  drillBlockFlash.count = newCount;

  if (drillBlockFlash.count <= 0) {
    drillBlockFlash.active = false;
  }
}
}

// =====================================================
// 畫圖通用函式
// =====================================================
static void drawDrillFrameTransform(
  const uint16_t* sheet,
  int sheetW,
  int frameW,
  int frameH,
  int frameIndex,
  int xOnScreen,
  int yOnScreen,
  bool mirrorX,
  bool mirrorY
) {
  int frameStartX = frameIndex * frameW;

  for (int y = 0; y < frameH; y++) {
    int dy = yOnScreen + y;
    if (dy < 0 || dy >= DRILL_SCR_H) continue;

    for (int x = 0; x < frameW; x++) {
      int dx = xOnScreen + x;
      if (dx < 0 || dx >= DRILL_SCR_W) continue;

      int sx = mirrorX ? frameW - 1 - x : x;
      int sy = mirrorY ? frameH - 1 - y : y;

      uint32_t pos =
        (uint32_t)sy * (uint32_t)sheetW +
        (uint32_t)(frameStartX + sx);

      uint16_t color = pgm_read_word(&(sheet[pos]));

      if (color == DRILL_TRANSPARENT) continue;

      display.drawPixel(dx, dy, color);
    }
  }
}

// =====================================================
// BLOCK 繪圖
// =====================================================
static bool drillBlockHiddenByFlash(int row, int col) {
  if (!drillBlockFlash.active) return false;
  if (drillBlockFlash.visible) return false;

  for (int i = 0; i < drillBlockFlash.count; i++) {
    if (drillBlockFlash.rows[i] == row &&
        drillBlockFlash.cols[i] == col) {
      return true;
    }
  }

  return false;
}

static void drillDrawBlockTile(int row, int col) {
  if (!drillInMap(row, col)) return;

  uint8_t cell = drillMap[row][col];
  if (cell == DRILL_CELL_EMPTY) return;

  if (drillBlockHiddenByFlash(row, col)) return;

  int screenX = col * DRILL_TILE_SIZE;
  int screenY =
    DRILL_BLOCK_WORLD_Y0 +
    row * DRILL_TILE_SIZE -
    drillCamY;

  if (screenX <= -DRILL_TILE_SIZE || screenX >= DRILL_SCR_W) return;
  if (screenY <= -DRILL_TILE_SIZE || screenY >= DRILL_SCR_H) return;

  drawDrillFrameTransform(
    DRILL_BLOCK,
    DRILL_BLOCK_SHEET_W,
    DRILL_BLOCK_FRAME_W,
    DRILL_BLOCK_FRAME_H,
    cell,
    screenX,
    screenY,
    false,
    false
  );
}

static void drillDrawBlocks() {
  int startRow = 0;

  if (drillCamY > DRILL_BLOCK_WORLD_Y0) {
    startRow = (drillCamY - DRILL_BLOCK_WORLD_Y0) / DRILL_TILE_SIZE - 1;
  }

  int endRow =
    (drillCamY + DRILL_SCR_H - DRILL_BLOCK_WORLD_Y0) /
    DRILL_TILE_SIZE + 2;

  if (startRow < 0) startRow = 0;
  if (endRow >= DRILL_MAP_ROWS) endRow = DRILL_MAP_ROWS - 1;

  for (int r = startRow; r <= endRow; r++) {
    for (int c = 0; c < DRILL_MAP_COLS; c++) {
      drillDrawBlockTile(r, c);
    }
  }
}

// =====================================================
// Driller 繪圖
// =====================================================
static int drillGetPlayerFrame(bool* mirrorX) {
  *mirrorX = false;

bool downAnim =
  drillPhase == DRILL_PHASE_BREAK_BLOCK &&
  drillAction.kind == DRILL_ACTION_DOWN;

  bool sideAnim =
    drillPhase == DRILL_PHASE_MOVE_X ||
    (drillPhase == DRILL_PHASE_BREAK_BLOCK &&
     drillAction.kind == DRILL_ACTION_SIDE);

  if (downAnim) {
    return 4 + drillPlayer.animStep;
  }

  if (sideAnim) {
    if (drillPlayer.dir > 0) {
      *mirrorX = true;
    }

    return 1 + drillPlayer.animStep;
  }

  return 0;
}

static void drillDrawPlayer() {
  bool mirrorX = false;
  int frameIndex = drillGetPlayerFrame(&mirrorX);

  int screenX = drillPlayer.worldX;
  int screenY = drillPlayer.worldY - drillCamY;

  drawDrillFrameTransform(
    drillActiveDrillerSheet,
    DRILLER_SHEET_W,
    DRILLER_FRAME_W,
    DRILLER_FRAME_H,
    frameIndex,
    screenX,
    screenY,
    mirrorX,
    false
  );
}

// =====================================================
// 顏色 BLOCK 閃爍
// =====================================================
static bool drillFlashGroupContains(int row, int col, int count) {
  for (int i = 0; i < count; i++) {
    if (drillBlockFlash.rows[i] == row &&
        drillBlockFlash.cols[i] == col) {
      return true;
    }
  }

  return false;
}

static void drillTryAddFlashCell(
  int row,
  int col,
  uint8_t color,
  int* count
) {
  if (*count >= DRILL_FLASH_GROUP_MAX) return;
  if (!drillInMap(row, col)) return;

  // 不允許把目標 BLOCK 上方的同色 BLOCK 加進消除群組。
  // 這樣主角上方已經路過的 BLOCK 不會突然閃爍消失。
  if (row < drillBlockFlash.minRow) return;

  if (drillMap[row][col] != color) return;
  if (drillFlashGroupContains(row, col, *count)) return;

  drillBlockFlash.rows[*count] = (uint8_t)row;
  drillBlockFlash.cols[*count] = (uint8_t)col;

  (*count)++;
}

static bool drillStartBlockFlash(int row, int col, unsigned long now) {
  if (!drillInMap(row, col)) return false;

  uint8_t color = drillMap[row][col];

// 顏色 BLOCK 和愛心可以被鑽破。
// 固定磚不能破壞，空氣也不需要閃爍。
if (!drillIsBreakableBlock(color)) {
  return false;
}

drillBlockFlash.active = true;
drillBlockFlash.color = color;

// 以正在被鑽的這顆 BLOCK 當作同色消除的上界。
// 目標 row 以上的同色 BLOCK 不會加入閃爍群組。
drillBlockFlash.minRow = row;

drillBlockFlash.visible = true;
drillBlockFlash.toggleCount = 0;
drillBlockFlash.lastToggleMs = now;
drillBlockFlash.count = 0;

if (drillIsHeart(color)) {
  drillBlockFlash.rows[0] = (uint8_t)row;
  drillBlockFlash.cols[0] = (uint8_t)col;
  drillBlockFlash.count = 1;
  return true;
}

  // -------------------------------------------------
  // 搜尋上下左右相連的同色 BLOCK
  // 使用簡單 queue，不用遞迴，避免 ESP32 stack 壓力。
  // -------------------------------------------------
  drillTryAddFlashCell(row, col, color, &drillBlockFlash.count);

  int scanIndex = 0;

  while (scanIndex < drillBlockFlash.count) {
    int r = drillBlockFlash.rows[scanIndex];
    int c = drillBlockFlash.cols[scanIndex];

    drillTryAddFlashCell(r - 1, c, color, &drillBlockFlash.count);
    drillTryAddFlashCell(r + 1, c, color, &drillBlockFlash.count);
    drillTryAddFlashCell(r, c - 1, color, &drillBlockFlash.count);
    drillTryAddFlashCell(r, c + 1, color, &drillBlockFlash.count);

    scanIndex++;

    // 避免超過群組上限
    if (drillBlockFlash.count >= DRILL_FLASH_GROUP_MAX) {
      break;
    }
  }

  return drillBlockFlash.count > 0;
}

static bool drillUpdateBlockFlash(unsigned long now) {
  if (!drillBlockFlash.active) return true;

  if (now - drillBlockFlash.lastToggleMs < DRILL_BLOCK_FLASH_INTERVAL_MS) {
    return false;
  }

  drillBlockFlash.lastToggleMs = now;
  drillBlockFlash.visible = !drillBlockFlash.visible;
  drillBlockFlash.toggleCount++;

  if (drillBlockFlash.toggleCount >= DRILL_BLOCK_FLASH_COUNT * 2) {
    for (int i = 0; i < drillBlockFlash.count; i++) {
      int r = drillBlockFlash.rows[i];
      int c = drillBlockFlash.cols[i];

      if (drillInMap(r, c)) {
        // 只清掉同色顏色 BLOCK。
        // 固定磚絕對不會被清掉。
        if (drillMap[r][c] == drillBlockFlash.color) {
          drillMap[r][c] = DRILL_CELL_EMPTY;
        }
      }
    }

    drillBlockFlash.active = false;
    drillBlockFlash.visible = true;
    drillBlockFlash.count = 0;

    return true;
  }

  return false;
}

// =====================================================
// 空氣 BLOCK 出現
// =====================================================
static void drillMaybeSpawnAir(unsigned long now) {
  static unsigned long drillNextAirSpawnMs = 0;

  // 第一次進來時，先抽一次等待時間。
  if (drillNextAirSpawnMs == 0) {
    drillNextAirSpawnMs = random(4000, 6001);  // 10 秒 ~ 20 秒
  }

  if (now - drillLastAirSpawnMs < drillNextAirSpawnMs) return;

  drillLastAirSpawnMs = now;

  // 這次時間到了，重新抽下一次空氣出現時間。
  drillNextAirSpawnMs = random(4000, 6001);  // 10 秒 ~ 20 秒

  int row = drillPlayer.row + DRILL_AIR_ROWS_AHEAD + random(4);
  int col = random(DRILL_MAP_COLS);

  if (!drillInMap(row, col)) return;

  // 不覆蓋固定磚，也不覆蓋已經被鑽空的位置。
  if (drillMap[row][col] != DRILL_CELL_FIXED &&
      drillMap[row][col] != DRILL_CELL_EMPTY) {
    drillMap[row][col] = DRILL_CELL_AIR;
  }
}

// =====================================================
// 目標判斷
// =====================================================
static bool drillCanUseTargetCell(int row, int col) {
  if (col < 0 || col >= DRILL_MAP_COLS) return false;

  // row < 0 代表在最上方空間左右移動，沒有 BLOCK 阻擋。
  if (row < 0) return true;

  if (!drillInMap(row, col)) return false;

  uint8_t cell = drillMap[row][col];

  // 固定磚不能破壞，也不能走進去。
  if (drillIsFixedBlock(cell)) return false;

  return true;
}


// 檢查某一格的正下方是否能支撐主角水平移動。
// 空洞和空氣都不能當地板，否則主角會看起來懸空走路。
static bool drillHasSupportBelow(int row, int col) {
  if (row < 0) return true;

  int belowRow = row + 1;

  if (!drillInMap(belowRow, col)) return false;

  uint8_t belowCell = drillMap[belowRow][col];

  if (belowCell == DRILL_CELL_EMPTY) return false;
  if (belowCell == DRILL_CELL_AIR) return false;

  return true;
}



// 專門給「同一層追空氣」使用。
// 比 drillCanUseTargetCell 更嚴格：
// 1. 固定磚不能通過
// 2. 空洞不能水平通過
// 3. 顏色磚、愛心、空氣可以當作追空氣路徑
static bool drillCanChaseAirThroughCell(int row, int col) {
  if (!drillInMap(row, col)) return false;

  uint8_t cell = drillMap[row][col];

  // 同一層路徑本身不能是固定磚
  if (drillIsFixedBlock(cell)) return false;

  // 同一層路徑本身不能是空洞
  if (cell == DRILL_CELL_EMPTY) return false;

  // 新增重點：
  // 這一格的腳下也必須有支撐，否則主角會懸空橫移。
  if (!drillHasSupportBelow(row, col)) return false;

  return true;
}


static void drillAddCandidate(
  uint8_t* kindList,
  int* rowList,
  int* colList,
  int* dirList,
  int* count,
  int maxCount,
  uint8_t kind,
  int row,
  int col,
  int dir
) {
if (*count >= maxCount) return;
if (!drillCanUseTargetCell(row, col)) return;

// 一般左右移動也不能走到腳下沒支撐的位置。
// 否則不是追氣泡時，也可能發生懸空橫移。
if (kind == DRILL_ACTION_SIDE && row >= 0) {
  if (!drillHasSupportBelow(row, col)) {
    return;
  }
}

  kindList[*count] = kind;
  rowList[*count] = row;
  colList[*count] = col;
  dirList[*count] = dir;

  (*count)++;
}


static bool drillFindSameRowAirStep(int* nextCol, int* dir) {
  if (drillPlayer.row < 0) return false;

  int currentRow = drillPlayer.row;
  int currentCol = drillPlayer.col;

  int bestDistance = 999;
  int bestDir = 0;
  int bestNextCol = currentCol;

  for (int c = 0; c < DRILL_MAP_COLS; c++) {
    if (c == currentCol) continue;
    if (!drillInMap(currentRow, c)) continue;
    if (!drillIsAir(drillMap[currentRow][c])) continue;

    int stepDir = c > currentCol ? 1 : -1;
    bool pathOk = true;

    for (int scanCol = currentCol + stepDir;
         scanCol != c + stepDir;
         scanCol += stepDir) {
      if (!drillCanChaseAirThroughCell(currentRow, scanCol)) {
        pathOk = false;
        break;
      }
    }

    if (!pathOk) continue;

    int distance = abs(c - currentCol);
    if (distance < bestDistance) {
      bestDistance = distance;
      bestDir = stepDir;
      bestNextCol = currentCol + stepDir;
    }
  }

  if (bestDir == 0) return false;

  *nextCol = bestNextCol;
  *dir = bestDir;
  return true;
}


// 只要同一層有可以水平抵達的空氣，就立刻往那邊走。
// 回傳 true 代表已經決定動作，外面不要再隨機選往下。
static bool drillTryChaseSameRowAir(unsigned long now) {
  int airNextCol = drillPlayer.col;
  int airDir = 0;

  if (!drillFindSameRowAirStep(&airNextCol, &airDir)) {
    return false;
  }

  drillStartAction(
    DRILL_ACTION_SIDE,
    drillPlayer.row,
    airNextCol,
    airDir,
    now
  );

  return true;
}


static bool drillMaybeSpawnWinHeart() {
  if (drillHeartSpawned) return true;
  if (drillDepthRows < DRILL_WIN_DEPTH_ROWS) return false;

  int row1 = drillPlayer.row + 1;
  int row2 = drillPlayer.row + 2;

  if (!drillInMap(row1, 0)) return false;
  if (!drillInMap(row2, 0)) return false;

  // 下方 2 層全部清空
  for (int c = 0; c < DRILL_MAP_COLS; c++) {
    drillMap[row1][c] = DRILL_CELL_EMPTY;
    drillMap[row2][c] = DRILL_CELL_EMPTY;
  }

  // 愛心隨機放在第 2 層其中一欄
  int col = random(DRILL_MAP_COLS);
  drillMap[row2][col] = DRILL_CELL_HEART;

  drillHeartSpawned = true;
  drillHeartRow = row2;
  drillHeartCol = col;

  Serial.println("heart spawned");

  return true;
}

// =====================================================
// 開始 X / Y 移動
// =====================================================
static void drillStartMoveX(unsigned long now) {
  drillPlayer.targetCol = drillAction.col;
  drillPlayer.targetWorldX = drillPlayerXForCol(drillAction.col);

  if (drillPlayer.targetWorldX < drillPlayer.worldX) {
    drillPlayer.dir = -1;
  } else if (drillPlayer.targetWorldX > drillPlayer.worldX) {
    drillPlayer.dir = 1;
  } else {
    drillPlayer.dir = 0;
  }

  drillPlayer.animStep = 0;
  drillPlayer.lastMoveMs = now;

  drillPhase = DRILL_PHASE_MOVE_X;
}

static void drillStartMoveY(unsigned long now) {
  drillPlayer.targetRow = drillAction.row;
  drillPlayer.targetWorldY = drillPlayerYForRow(drillAction.row);

  drillPlayer.dir = 0;
  drillPlayer.animStep = 0;
  drillPlayer.lastMoveMs = now;

  drillPhase = DRILL_PHASE_MOVE_Y;
}

static bool drillCellIsFallSpace(int row, int col) {
  if (!drillInMap(row, col)) return false;

  uint8_t cell = drillMap[row][col];

  // 空洞可以掉落。
  if (cell == DRILL_CELL_EMPTY) return true;

  // 空氣也視為可以進入。
  // 掉進去時會被吃掉，變成空洞。
  if (cell == DRILL_CELL_AIR) return true;

  return false;
}

static bool drillTryStartFall(unsigned long now) {
  // 開場還在最上方時，不自動掉落。
  // 必須先選擇往下鑽第一顆 BLOCK。
  if (drillPlayer.row < 0) return false;

  int belowRow = drillPlayer.row + 1;
  int col = drillPlayer.col;

  if (!drillInMap(belowRow, col)) return false;
  if (!drillCellIsFallSpace(belowRow, col)) return false;

// 如果下方是空氣，掉進去前先吃掉並補 1 點體力。
if (drillMap[belowRow][col] == DRILL_CELL_AIR) {
  drillMap[belowRow][col] = DRILL_CELL_EMPTY;
  drillAddHp(1);
}
  drillAction.kind = DRILL_ACTION_DOWN;
  drillAction.row = belowRow;
  drillAction.col = col;
  drillAction.dir = 0;

  drillStartMoveY(now);
  return true;
}


// =====================================================
// 開始動作
// 重點：
// 如果目標格有顏色 BLOCK，先原地閃爍破壞。
// BLOCK 消失後，才開始移動進去。
// =====================================================
static void drillStartAction(
  uint8_t kind,
  int row,
  int col,
  int dir,
  unsigned long now
) {
  drillAction.kind = kind;
  drillAction.row = row;
  drillAction.col = col;
  drillAction.dir = dir;

  drillPlayer.dir = dir;
  drillPlayer.animStep = 0;
  drillPlayer.lastAnimMs = now;

  // 最上方空間左右移動，不需要破壞 BLOCK。
  if (row < 0 && kind == DRILL_ACTION_SIDE) {
    if (drillTopFreeMovesLeft > 0) {
      drillTopFreeMovesLeft--;
    }

    drillStartMoveX(now);
    return;
  }

  if (!drillInMap(row, col)) {
    drillPhase = DRILL_PHASE_CHOOSE;
    return;
  }

  uint8_t cell = drillMap[row][col];

  // 固定磚完全不能破壞，也不能進入。
  if (drillIsFixedBlock(cell)) {
    drillHoldStartMs = now;
    drillPhase = DRILL_PHASE_STUCK_WAIT;
    return;
  }

// 空氣可以進入，進入前先變成空洞並補 1 點體力。
if (drillIsAir(cell)) {
  drillMap[row][col] = DRILL_CELL_EMPTY;
  drillAddHp(1);
}

// 顏色 BLOCK / 愛心：先閃爍破壞，人不移動。
if (drillIsBreakableBlock(cell)) {
    bool started = drillStartBlockFlash(row, col, now);

    if (started) {
      drillPhase = DRILL_PHASE_BREAK_BLOCK;
      return;
    }
  }

  // 已經是空洞或空氣，直接移動進去。
  if (kind == DRILL_ACTION_SIDE) {
    drillStartMoveX(now);
  } else if (kind == DRILL_ACTION_DOWN) {
    drillStartMoveY(now);
  } else {
    drillPhase = DRILL_PHASE_CHOOSE;
  }
}

// =====================================================
// 選擇下一步動作
// =====================================================
static void drillChooseNextAction(unsigned long now) {
  const int MAX_CANDIDATES = 10;

  uint8_t candKind[MAX_CANDIDATES];
  int candRow[MAX_CANDIDATES];
  int candCol[MAX_CANDIDATES];
  int candDir[MAX_CANDIDATES];

  int count = 0;

  int currentRow = drillPlayer.row;
  int currentCol = drillPlayer.col;

  // -------------------------------------------------
  // 開場還在 BLOCK 上方：可以自由左右晃一下，再往下鑽
  // -------------------------------------------------
  if (currentRow < 0) {
    if (drillTopFreeMovesLeft > 0) {
      if (currentCol > 0) {
        drillAddCandidate(
          candKind,
          candRow,
          candCol,
          candDir,
          &count,
          MAX_CANDIDATES,
          DRILL_ACTION_SIDE,
          -1,
          currentCol - 1,
          -1
        );
      }

      if (currentCol < DRILL_MAP_COLS - 1) {
        drillAddCandidate(
          candKind,
          candRow,
          candCol,
          candDir,
          &count,
          MAX_CANDIDATES,
          DRILL_ACTION_SIDE,
          -1,
          currentCol + 1,
          1
        );
      }

      // 也保留往下鑽的可能。
      for (int i = 0; i < 3; i++) {
        drillAddCandidate(
          candKind,
          candRow,
          candCol,
          candDir,
          &count,
          MAX_CANDIDATES,
          DRILL_ACTION_DOWN,
          0,
          currentCol,
          0
        );
      }
    } else {
      drillAddCandidate(
        candKind,
        candRow,
        candCol,
        candDir,
        &count,
        MAX_CANDIDATES,
        DRILL_ACTION_DOWN,
        0,
        currentCol,
        0
      );
    }
  }

  // -------------------------------------------------
  // 已經在 BLOCK 區域中：左右或往下都要先判斷目標格
  // -------------------------------------------------
else {
  int downRow = currentRow + 1;

  // 只要腳下是空洞或空氣，優先往下掉。
  // 不先追氣泡，也不先隨機左右移動。
  if (drillTryStartFall(now)) {
    return;
  }

 if (drillMaybeSpawnWinHeart() && drillHeartSpawned) {
  // 愛心在正下方，就往下鑽破它
  if (drillHeartRow == downRow && drillHeartCol == currentCol) {
    drillStartAction(
      DRILL_ACTION_DOWN,
      downRow,
      currentCol,
      0,
      now
    );
    return;
  }

  // 愛心在同一層，先左右移動靠近它
  if (drillHeartRow == currentRow) {
    if (drillHeartCol < currentCol) {
      drillStartAction(
        DRILL_ACTION_SIDE,
        currentRow,
        currentCol - 1,
        -1,
        now
      );
      return;
    }

    if (drillHeartCol > currentCol) {
      drillStartAction(
        DRILL_ACTION_SIDE,
        currentRow,
        currentCol + 1,
        1,
        now
      );
      return;
    }
  }
}


  // 空氣優先權高於一般隨機行動。
  // 同一層只要有可以水平抵達的空氣，就一定先去吃。
  if (drillTryChaseSameRowAir(now)) {
    return;
  }


  // 往下權重高一點，讓畫面保持一直往下鑽。

    // 往下權重高一點，讓畫面保持一直往下鑽。
    for (int i = 0; i < 4; i++) {
      drillAddCandidate(
        candKind,
        candRow,
        candCol,
        candDir,
        &count,
        MAX_CANDIDATES,
        DRILL_ACTION_DOWN,
        downRow,
        currentCol,
        0
      );
    }

    // 左右移動權重低一點。
    drillAddCandidate(
      candKind,
      candRow,
      candCol,
      candDir,
      &count,
      MAX_CANDIDATES,
      DRILL_ACTION_SIDE,
      currentRow,
      currentCol - 1,
      -1
    );

    drillAddCandidate(
      candKind,
      candRow,
      candCol,
      candDir,
      &count,
      MAX_CANDIDATES,
      DRILL_ACTION_SIDE,
      currentRow,
      currentCol + 1,
      1
    );
  }

// 沒有可用目標，代表被固定磚或邊界卡住。
// 不破壞固定磚，只原地等一下再重選。
// 如果卡太久，就由 drillUpdate() 重新 Drill_Init()。
if (count <= 0) {
  drillStartStuckTimer(now);
  drillHoldStartMs = now;
  drillPhase = DRILL_PHASE_STUCK_WAIT;
  return;
}

  int pick = random(count);

  drillStartAction(
    candKind[pick],
    candRow[pick],
    candCol[pick],
    candDir[pick],
    now
  );
}

// =====================================================
// X 移動更新
// 只有目標格已經變空，才會進入這個流程。
// =====================================================
static bool drillUpdateMoveX(unsigned long now) {
  if (drillPlayer.worldX == drillPlayer.targetWorldX) {
    drillPlayer.col = drillPlayer.targetCol;
    drillPlayer.dir = 0;
    return true;
  }

  if (now - drillPlayer.lastMoveMs < DRILL_X_MOVE_INTERVAL_MS) {
    return false;
  }

  drillPlayer.lastMoveMs = now;

  if (drillPlayer.worldX < drillPlayer.targetWorldX) {
    drillPlayer.worldX++;
  } else if (drillPlayer.worldX > drillPlayer.targetWorldX) {
    drillPlayer.worldX--;
  }

  if (drillPlayer.worldX == drillPlayer.targetWorldX) {
    drillPlayer.col = drillPlayer.targetCol;
    drillPlayer.dir = 0;
    return true;
  }

  return false;
}

// =====================================================
// Y 移動更新
// 只有下方 BLOCK 已經消失，才會往下移動。
// =====================================================
static bool drillUpdateMoveY(unsigned long now) {
  if (drillPlayer.worldY >= drillPlayer.targetWorldY) {
    drillPlayer.worldY = drillPlayer.targetWorldY;
    return true;
  }

if (now - drillPlayer.lastMoveMs < DRILL_FALL_MOVE_INTERVAL_MS) {
  return false;
}

  drillPlayer.lastMoveMs = now;
  drillPlayer.worldY++;

  if (drillPlayer.worldY >= drillPlayer.targetWorldY) {
    drillPlayer.worldY = drillPlayer.targetWorldY;
    return true;
  }

  return false;
}

// =====================================================
// 移動完成後
// =====================================================
static void drillFinishMoveX(unsigned long now) {
  // 左右移動完成後，如果腳下是空洞或空氣，必須先掉落。
  // 不能為了追氣泡而懸空橫移。
  if (drillTryStartFall(now)) {
    return;
  }

  // 腳下有支撐時，才繼續追同一層空氣。
  if (drillTryChaseSameRowAir(now)) {
    return;
  }

  drillHoldStartMs = now;
  drillPhase = DRILL_PHASE_HOLD;
}

static void drillFinishMoveY(unsigned long now) {
  drillPlayer.row = drillPlayer.targetRow;

  drillRowsSinceRecenter++;
  drillDepthRows++;

 // Serial.print("第");
 // Serial.print(drillDepthRows);
//Serial.println("層");

if (drillPhase != DRILL_PHASE_WIN &&
    drillPhase != DRILL_PHASE_GAME_OVER) {
  drillRowsSinceHpDamage++;

  if (drillRowsSinceHpDamage >= DRILL_HP_DOWN_ROWS_PER_DAMAGE) {
    drillRowsSinceHpDamage = 0;
    drillAddHp(-1);

    if (drillHp <= 0) {
      drillStartGameOver(now);
      return;
    }
  }
}

if (drillTryStartFall(now)) {
  return;
}

if (drillTryChaseSameRowAir(now)) {
  return;
}

  if (drillRowsSinceRecenter >= DRILL_ROWS_BEFORE_RECENTER) {
    drillPhase = DRILL_PHASE_RECENTER;
  } else {
    drillHoldStartMs = now;
    drillPhase = DRILL_PHASE_HOLD;
  }
}

// =====================================================
// 動畫更新
// =====================================================
static void drillUpdateAnim(unsigned long now) {
  bool animating =
    drillPhase == DRILL_PHASE_BREAK_BLOCK ||
    drillPhase == DRILL_PHASE_MOVE_X ||
    drillPhase == DRILL_PHASE_MOVE_Y;

  if (!animating) {
    drillPlayer.animStep = 0;
    return;
  }

  if (now - drillPlayer.lastAnimMs < DRILL_ANIM_INTERVAL_MS) return;

  drillPlayer.lastAnimMs = now;
  drillPlayer.animStep++;

  if (drillPlayer.animStep > 2) {
    drillPlayer.animStep = 0;
  }
}

// =====================================================
// Camera 更新
// =====================================================
static void drillUpdateCamera(unsigned long now) {
  if (now - drillLastCameraMs < DRILL_CAMERA_INTERVAL_MS) return;

  drillLastCameraMs = now;

  int desiredCamY = drillCamY;

  if (drillPhase == DRILL_PHASE_RECENTER) {
    // 不移動人物座標，而是讓 camera 平滑追上。
    // 這樣主角看起來會被鏡頭拉回 y=15，不會瞬移。
    desiredCamY = drillPlayer.worldY - DRILL_CAMERA_ANCHOR_Y;
  } else {
    // 平常讓主角可以往下走到接近底部，
    // 太低時 camera 才慢慢跟上。
    int screenY = drillPlayerScreenY();

    if (screenY > DRILL_CAMERA_SOFT_LIMIT_Y) {
      desiredCamY = drillPlayer.worldY - DRILL_CAMERA_SOFT_LIMIT_Y;
    }
  }

  if (desiredCamY < 0) desiredCamY = 0;

  int step =
    drillPhase == DRILL_PHASE_RECENTER ?
    DRILL_CAMERA_RECENTER_STEP :
    DRILL_CAMERA_NORMAL_STEP;

  if (drillCamY < desiredCamY) {
    drillCamY += step;

    if (drillCamY > desiredCamY) {
      drillCamY = desiredCamY;
    }
  } else if (drillCamY > desiredCamY) {
    drillCamY -= step;

    if (drillCamY < desiredCamY) {
      drillCamY = desiredCamY;
    }
  }
}

static void drillCheckRecenterDone() {
  if (drillPhase != DRILL_PHASE_RECENTER) return;

  int desiredCamY = drillPlayer.worldY - DRILL_CAMERA_ANCHOR_Y;
  if (desiredCamY < 0) desiredCamY = 0;

  if (drillCamY == desiredCamY) {
    drillRowsSinceRecenter = 0;
    drillHoldStartMs = millis();
    drillPhase = DRILL_PHASE_HOLD;
  }
}

// =====================================================
// 初始化
// =====================================================
void Drill_Init() {
  unsigned long now = millis();

if (random(2) == 0) {
  drillActiveDrillerSheet = DRILLER;
} else {
  drillActiveDrillerSheet = DRILLER2;
}
  
  drillSafeCol = DRILL_START_COL;
  drillGenerateMap();

  drillCamY = 0;
  drillRowsSinceRecenter = 0;
  drillDepthRows = 0;

drillRowsSinceHpDamage = 0;
drillHp = DRILL_HP_MAX;
drillResultStartMs = 0;

drillHeartSpawned = false;
drillHeartRow = -1;
drillHeartCol = -1;

  drillTopFreeMovesLeft = DRILL_TOP_FREE_SIDE_MOVES;

  drillPlayer.row = -1;
  drillPlayer.col = DRILL_START_COL;

  drillPlayer.worldX = drillPlayerXForCol(DRILL_START_COL);
  drillPlayer.worldY = DRILL_START_SCREEN_Y;

  drillPlayer.targetRow = -1;
  drillPlayer.targetCol = DRILL_START_COL;
  drillPlayer.targetWorldX = drillPlayer.worldX;
  drillPlayer.targetWorldY = drillPlayer.worldY;

  drillPlayer.dir = 0;
  drillPlayer.animStep = 0;

  drillPlayer.lastMoveMs = now;
  drillPlayer.lastAnimMs = now;

  drillAction.kind = DRILL_ACTION_NONE;
  drillAction.row = -1;
  drillAction.col = DRILL_START_COL;
  drillAction.dir = 0;

drillBlockFlash.active = false;
drillBlockFlash.color = DRILL_CELL_BLUE;
drillBlockFlash.minRow = 0;
drillBlockFlash.visible = true;
drillBlockFlash.toggleCount = 0;
drillBlockFlash.lastToggleMs = now;
drillBlockFlash.count = 0;

for (int i = 0; i < DRILL_FLASH_GROUP_MAX; i++) {
  drillBlockFlash.rows[i] = 0;
  drillBlockFlash.cols[i] = 0;
}

  drillHoldStartMs = now;
  drillLastCameraMs = now;
  drillLastAirSpawnMs = now;
  drillClearStuckTimer();
  drillPhase = DRILL_PHASE_CHOOSE;
}

// =====================================================
// 主更新邏輯
// =====================================================
static void drillUpdate(unsigned long now) {

if (drillPhase == DRILL_PHASE_GAME_OVER ||
    drillPhase == DRILL_PHASE_WIN) {
  drillUpdateCamera(now);

  if (now - drillResultStartMs >= DRILL_RESULT_WAIT_MS) {
    Drill_Init();
  }

  return;
}
  
  drillMaybeSpawnAir(now);
  drillUpdateAnim(now);

  if (drillPhase == DRILL_PHASE_CHOOSE) {
    drillChooseNextAction(now);
  }

  else if (drillPhase == DRILL_PHASE_BREAK_BLOCK) {
    // 角色在原地播放左 / 右 / 下鑽動畫。
    // BLOCK 閃爍消失以前，不會移動進目標格。
if (drillUpdateBlockFlash(now)) {
  if (drillInMap(drillAction.row, drillAction.col) &&
      drillAction.row == drillHeartRow &&
      drillAction.col == drillHeartCol &&
      drillHeartSpawned) {
    drillHeartSpawned = false;
    drillHeartRow = -1;
    drillHeartCol = -1;
    drillStartWin(now);
    return;
  }

  if (drillAction.kind == DRILL_ACTION_SIDE) {
        drillStartMoveX(now);
      } else if (drillAction.kind == DRILL_ACTION_DOWN) {
        drillStartMoveY(now);
      } else {
        drillPhase = DRILL_PHASE_CHOOSE;
      }
    }
  }

  else if (drillPhase == DRILL_PHASE_MOVE_X) {
    if (drillUpdateMoveX(now)) {
      drillFinishMoveX(now);
    }
  }

  else if (drillPhase == DRILL_PHASE_MOVE_Y) {
    if (drillUpdateMoveY(now)) {
      drillFinishMoveY(now);
    }
  }

  else if (drillPhase == DRILL_PHASE_HOLD) {
    if (now - drillHoldStartMs >= DRILL_LAYER_HOLD_MS) {
      drillPhase = DRILL_PHASE_CHOOSE;
    }
  }

  else if (drillPhase == DRILL_PHASE_RECENTER) {
    // 實際移動由 camera 完成。
  }

else if (drillPhase == DRILL_PHASE_STUCK_WAIT) {
  // 卡住太久，整個主題重新初始化。
  // 這樣不改地圖規則，也不需要破壞固定磚。
  if (drillStuckTimerActive &&
      now - drillStuckStartMs >= DRILL_STUCK_RESET_MS) {
    Drill_Init();
    return;
  }

  if (now - drillHoldStartMs >= DRILL_STUCK_RETRY_MS) {
    drillPhase = DRILL_PHASE_CHOOSE;
  }
}

  drillUpdateCamera(now);
  drillCheckRecenterDone();
  drillRecycleMapIfNeeded();
}

// =====================================================
// 畫面重繪
// =====================================================
static void drillRender() {
  display.fillScreen(DRILL_BG_COLOR);

  drillDrawBlocks();
  drillDrawPlayer();

// 正在閃爍的同色 BLOCK 群組再畫一次到人物上方。
// 這樣 BLOCK 消失前，角色視覺上也不會蓋到 BLOCK。
if (drillBlockFlash.active && drillBlockFlash.visible) {
  for (int i = 0; i < drillBlockFlash.count; i++) {
    drillDrawBlockTile(
      drillBlockFlash.rows[i],
      drillBlockFlash.cols[i]
    );
  }
}
}

static void drillDrawHpBar(unsigned long now) {
  for (int x = 0; x < DRILL_SCR_W; x++) {
    display.drawPixel(x, 0, DRILL_HP_COLOR_BG);
  }

  int hpWidth = drillClampInt(drillHp, 0, DRILL_HP_MAX);
  if (hpWidth <= 0) return;

  bool lowHp = drillHp < DRILL_HP_LOW_THRESHOLD;

  if (lowHp) {
    bool blinkOn = ((now / DRILL_HP_BLINK_MS) % 2) == 0;
    if (!blinkOn) return;
  }

  uint16_t color = lowHp ? DRILL_HP_COLOR_RED : DRILL_HP_COLOR_GREEN;

  for (int x = 0; x < hpWidth; x++) {
    display.drawPixel(x, 0, color);
  }
}


// =====================================================
// 主模式函式
// =====================================================
void DrillMode() {
  if (ModefirstRun) {
    randomSeed(millis());
    Drill_Init();
    ModefirstRun = false;
  }

  unsigned long now = millis();

  drillUpdate(now);
  drillRender();

  // 時鐘固定最上方，最後畫，避免被角色或 BLOCK 蓋住。
  drawThemeClockText();

  // 體力條放在最頂部 1px，最後畫，確保不被其他內容蓋住。
  drillDrawHpBar(now);

  wait_with_display(30);
}
