// =====================================================
// 圖形數據 (Sprite Data)
// =====================================================
// ESP8266：把大型常數表放到 Flash，避免佔用 RAM
#include <pgmspace.h>

// Pac-Man 張嘴圖案 (0=透, 1=身)
const uint8_t PROGMEM pacman2[4][4] = {
  {0,1,1,1},
  {1,1,0,0},
  {1,1,0,0},
  {0,1,1,1}
};

// Pac-Man 閉嘴圖案
const uint8_t PROGMEM pacmanClosed[4][4] = {
  {0,1,1,0},
  {1,1,1,1},
  {1,1,1,1},
  {0,1,1,0}
};

// 鬼的圖案 (2=眼睛)
const uint8_t PROGMEM ghost1[4][4] = {
  {0,1,1,0},
  {1,2,2,1},
  {1,1,1,1},
  {1,1,1,1}
};

// =====================================================
// 地圖數據 (Map Data)
// 0=路, 1=牆, 2=鬼屋入口(視覺上也是空地)
// =====================================================
const uint8_t PROGMEM mazeC1[16][16] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,1,1,0,0,0,0,1,1,0,0,0,1},
  {1,0,1,0,0,0,0,1,1,0,0,0,0,1,0,1},
  {1,0,0,0,1,1,0,0,0,0,1,1,0,0,0,1},
  {1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1},
  {0,0,0,1,1,2,2,2,2,2,2,1,1,0,0,0},
  {1,1,0,1,1,2,2,2,2,2,2,1,1,0,1,1},
  {1,0,0,1,1,2,2,2,2,2,2,1,1,0,0,1},
  {1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1},
  {1,0,1,0,0,0,0,1,1,0,0,0,0,1,0,1},
  {1,0,0,0,1,1,0,0,0,0,1,1,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

const uint8_t PROGMEM mazeR1[16][16] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,1,1,1,1,0,0,1,1,1,1,1,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1},
  {1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
  {1,1,0,1,1,1,1,1,1,1,1,1,1,0,0,1},
  {0,0,0,1,1,2,2,2,2,2,2,1,1,0,0,1},
  {1,1,0,1,1,2,2,2,2,2,2,1,1,0,1,1},
  {1,1,0,1,1,2,2,2,2,2,2,1,1,0,1,1},
  {1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1},
  {1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
  {1,0,0,0,1,1,1,1,1,1,1,1,0,0,0,1},
  {1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
  {1,0,0,0,1,1,1,0,0,1,1,1,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

const uint8_t PROGMEM mazeL1[16][16] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,1,1,1,1,1,0,1,1,1,0,1,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1},
  {1,0,0,1,1,2,2,0,2,2,2,1,1,0,0,0},
  {1,1,0,1,1,2,2,0,2,2,2,1,1,0,1,1},
  {1,0,0,1,1,2,2,0,2,2,2,1,1,0,0,1},
  {1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1},
  {1,1,0,0,0,0,1,0,0,1,0,0,0,1,0,1},
  {1,0,0,1,1,0,0,0,0,0,0,1,0,1,0,1},
  {1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// =====================================================
// 全域變數與設定
// =====================================================
enum MapId : uint8_t { MAP_L1, MAP_C1, MAP_R1 };
static MapId curMapId = MAP_C1;

// 豆子系統：用 16x16 bitset 取代 bool[16][16]，大幅減少 RAM
// dotRowBits[y] 的第 x bit = 1 代表該格有豆子
static uint16_t dotRowBits[16];
unsigned long dotsEatenTime = 0;
bool allDotsEaten = false;

// 鬼 (Ghost) - 只在 R1 活動
static int gHX = 14, gHY = 7;
static int gHDir = 2;
static bool ghostInit = false;

// 遊戲狀態
static bool gameOver = false;
static unsigned long gameOverAt = 0;
static bool gameWin = false;
static unsigned long gameWinAt = 0;

// Power Block (無敵道具) - 只在 L1
static bool powerBlockActive = true;
static bool invincible = false;
static unsigned long invincibleUntil = 0;
const int PBX = 7;
const int PBY = 7;

// 方向向量 (0:右, 1:下, 2:左, 3:上)
const int dx[4] = { 1, 0,-1, 0 };
const int dy[4] = { 0, 1, 0,-1 };

// [可調參數]
const uint8_t PAC_TURN_CHANCE = 25;
const uint8_t GHOST_TURN_CHANCE = 30;

// [平滑移動參數]
// 1 格 = 4px。每個動畫 frame 只前進 1px，所以看起來會滑動，而不是每 200ms 跳一格。
const uint8_t CELL_PX = 4;
const uint8_t ANIM_STEPS = 4;
const uint16_t ANIM_FRAME_MS = 45;  // 45ms * 4 = 180ms / 格。想慢一點可改 55~60。

static inline void setMazeById(MapId id);
static inline void drawClockOverlay(bool forceRedrawNumbers);

// =====================================================
// PROGMEM 地圖/豆子存取工具
// =====================================================
static inline uint8_t mazeCell(int x, int y) {
  if ((uint8_t)x > 15 || (uint8_t)y > 15) return 1;

  const uint8_t* base = nullptr;
  switch (curMapId) {
    case MAP_L1: base = &mazeL1[0][0]; break;
    case MAP_R1: base = &mazeR1[0][0]; break;
    case MAP_C1:
    default:     base = &mazeC1[0][0]; break;
  }

  return pgm_read_byte(base + (y * 16) + x);
}

static inline bool dotGet(int x, int y) {
  if ((uint8_t)x > 15 || (uint8_t)y > 15) return false;
  return (dotRowBits[y] >> x) & 0x01;
}

static inline void dotSet(int x, int y, bool v) {
  if ((uint8_t)x > 15 || (uint8_t)y > 15) return;

  const uint16_t mask = (uint16_t)1u << x;
  if (v) dotRowBits[y] |= mask;
  else   dotRowBits[y] &= (uint16_t)~mask;
}

// =====================================================
// 工具函式
// =====================================================
static inline bool isWalkableCell(int x, int y) {
  if (x < 0 || x > 15 || y < 0 || y > 15) return false;
  return (mazeCell(x, y) == 0);
}

static inline bool isWall(int x, int y) {
  if (x < 0 || x > 15 || y < 0 || y > 15) return false;
  return (mazeCell(x, y) == 1);
}

// =====================================================
// 牆面繪製邏輯
// =====================================================
void drawFullMaze() {
  uint16_t wallColor = display.color565(33, 33, 255);

  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      if (!isWall(x, y)) continue;

      int px = x * 4;
      int py = y * 4;

      if (!isWall(x, y - 1)) display.drawLine(px,     py,     px + 3, py,     wallColor);
      if (!isWall(x, y + 1)) display.drawLine(px,     py + 3, px + 3, py + 3, wallColor);
      if (!isWall(x - 1, y)) display.drawLine(px,     py,     px,     py + 3, wallColor);
      if (!isWall(x + 1, y)) display.drawLine(px + 3, py,     px + 3, py + 3, wallColor);

      if (isWall(x + 1, y) && isWall(x, y + 1) && !isWall(x + 1, y + 1))
        display.drawPixel(px + 3, py + 3, wallColor);
      if (isWall(x + 1, y) && isWall(x, y - 1) && !isWall(x + 1, y - 1))
        display.drawPixel(px + 3, py, wallColor);
      if (isWall(x - 1, y) && isWall(x, y + 1) && !isWall(x - 1, y + 1))
        display.drawPixel(px, py + 3, wallColor);
      if (isWall(x - 1, y) && isWall(x, y - 1) && !isWall(x - 1, y - 1))
        display.drawPixel(px, py, wallColor);
    }
  }
}

// =====================================================
// 豆子邏輯
// =====================================================
void resetDots() {
  for (int y = 0; y < 16; y++) {
    dotRowBits[y] = 0;

    for (int x = 0; x < 16; x++) {
      const bool put = (mazeCell(x, y) == 0) && (x != 0) && (x != 15);
      dotSet(x, y, put);

      if (put) {
        display.drawPixel(x * 4 + 2, y * 4 + 2, display.color565(150, 150, 150));
      }
    }
  }
}

void checkDotsStatus() {
  bool found = false;

  for (int y = 0; y < 16; y++) {
    if (dotRowBits[y] != 0) {
      found = true;
      break;
    }
  }

  if (!found && !allDotsEaten) {
    allDotsEaten = true;
    dotsEatenTime = millis();
    Serial.println(F("豆子吃光了！1分鐘後重置..."));
  }
}

// =====================================================
// 地圖切換
// =====================================================
static inline void setMazeById(MapId id) {
  curMapId = id;

  display.fillScreen(0);
  drawFullMaze();
  resetDots();
  drawClockOverlay(true);
}

// =====================================================
// AI 移動邏輯
// =====================================================
static int countChoicesNoBack(int x, int y, int curDir) {
  int backDir = (curDir + 2) & 3;
  int c = 0;

  for (int dir = 0; dir < 4; dir++) {
    if (dir == backDir) continue;

    int nx = x + dx[dir];
    int ny = y + dy[dir];

    if (isWalkableCell(nx, ny)) c++;
  }

  return c;
}

static int chooseTurnAtJunction(int x, int y, int curDir, uint8_t chancePct) {
  int backDir = (curDir + 2) & 3;
  int opts[4], n = 0;

  for (int dir = 0; dir < 4; dir++) {
    if (dir == backDir) continue;
    if (isWalkableCell(x + dx[dir], y + dy[dir])) opts[n++] = dir;
  }

  if (n < 2) return curDir;
  if (random(100) >= chancePct) return curDir;

  int turnOpts[4], m = 0;
  for (int i = 0; i < n; i++) {
    if (opts[i] != curDir) turnOpts[m++] = opts[i];
  }

  if (m > 0) return turnOpts[random(m)];
  return curDir;
}

static int chooseDirWhenBlocked(int x, int y, int curDir) {
  int backDir = (curDir + 2) & 3;
  int opts[4], n = 0;

  for (int dir = 0; dir < 4; dir++) {
    if (dir == backDir) continue;

    if (isWalkableCell(x + dx[dir], y + dy[dir])) {
      opts[n++] = dir;
    }
  }

  if (n > 0) return opts[random(n)];
  return backDir;
}

// =====================================================
// 繪圖：文字顯示
// =====================================================
static inline void drawStatusText(bool win) {
  display.fillRect(13, 25, 38, 18, 0);

  if (win) {
    display.setTextColor(display.color565(0, 255, 0));
    display.setCursor(15, 24); display.print("YOU");
    display.setCursor(29, 42); display.print("WIN");
  } else {
    display.setTextColor(display.color565(255, 0, 0));
    display.setCursor(15, 24); display.print("GAME");
    display.setCursor(25, 42); display.print("OVER");
  }
}

// =====================================================
// 繪圖：Power Block
// =====================================================
static inline bool isOnPowerBlock(int x, int y) {
  return powerBlockActive && (x == PBX || x == PBX + 1) && (y == PBY || y == PBY + 1);
}

static inline void drawPowerBlock() {
  uint16_t c = display.color565(255, 255, 0);
  int px = PBX * 4 + 2;
  int py = PBY * 4 + 1;
  display.fillRect(px, py, 3, 3, c);
}

static inline void clearPowerBlockArea() {
  int px = PBX * 4 + 2;
  int py = PBY * 4 + 1;
  display.fillRect(px, py, 3, 3, 0);
}

// =====================================================
// 繪圖：時鐘重疊層
// =====================================================
static inline void drawClockOverlay(bool forceRedrawNumbers) {
  static int lastDrawnM = -1;

  if (forceRedrawNumbers || M != lastDrawnM) {
    display.fillRect(13, 25, 38, 18, 0);
    showbit12number(H, 7, 12, 14, 28, hsv2rgb(hueh, saturation, value));
    showbit12number(M, 7, 12, 34, 28, hsv2rgb(huem, saturation, value));
    lastDrawnM = M;
  }

  if (S % 2 == 0) {
    display.drawPixel(32, 32, hsv2rgb(hue, saturation, value));
    display.drawPixel(32, 36, hsv2rgb(hue, saturation, value));
  } else {
    display.drawPixel(32, 32, 0);
    display.drawPixel(32, 36, 0);
  }
}

// =====================================================
// 繪圖：Pacman Sprite 處理
// =====================================================
static inline uint8_t sprRead4(const uint8_t spr[4][4], int x, int y) {
  return pgm_read_byte(&spr[y][x]);
}

static inline uint8_t spriteAt4(const uint8_t spr[4][4], int sx, int sy, int dir) {
  switch (dir & 3) {
    case 0: return sprRead4(spr, sx, sy);
    case 2: return sprRead4(spr, 3 - sx, sy);
    case 1: return sprRead4(spr, sy, 3 - sx);
    case 3: return sprRead4(spr, 3 - sy, sx);
  }

  return 0;
}

static inline void drawSprite4x4Px(int px, int py, const uint8_t spr[4][4], int dir, uint16_t bodyColor) {
  for (int sy = 0; sy < 4; sy++) {
    for (int sx = 0; sx < 4; sx++) {
      uint8_t v = spriteAt4(spr, sx, sy, dir);
      if (v == 0) continue;
      display.drawPixel(px + sx, py + sy, bodyColor);
    }
  }
}

static inline void drawPacmanSpritePx(int px, int py, int dir, bool invincible, bool mouthOpen) {
  uint16_t body = invincible ? display.color565(255, 255, 255) : display.color565(255, 255, 0);
  const uint8_t (*spr)[4] = mouthOpen ? pacman2 : pacmanClosed;
  drawSprite4x4Px(px, py, spr, dir, body);
}

static inline void drawGhostPx(int px, int py) {
  uint16_t body = display.color565(255, 0, 0);
  uint16_t eye  = display.color565(255, 255, 255);

  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      uint8_t v = pgm_read_byte(&ghost1[y][x]);
      if (v == 0) continue;
      display.drawPixel(px + x, py + y, (v == 2) ? eye : body);
    }
  }
}

// =====================================================
// 平滑移動：背景還原
// =====================================================
static inline void redrawCellBackground(int cellX, int cellY) {
  if (cellX < 0 || cellX > 15 || cellY < 0 || cellY > 15) return;

  const int px = cellX * CELL_PX;
  const int py = cellY * CELL_PX;

  display.fillRect(px, py, CELL_PX, CELL_PX, 0);

  if (dotGet(cellX, cellY)) {
    display.drawPixel(px + 2, py + 2, display.color565(150, 150, 150));
  }
}

static inline void restoreSpriteBackgroundPx(int px, int py) {
  if (px < -10 || py < -10) return;

  int x0 = px / CELL_PX;
  int y0 = py / CELL_PX;
  int x1 = (px + 3) / CELL_PX;
  int y1 = (py + 3) / CELL_PX;

  if (x0 < 0) x0 = 0;
  if (y0 < 0) y0 = 0;
  if (x1 > 15) x1 = 15;
  if (y1 > 15) y1 = 15;

  for (int y = y0; y <= y1; y++) {
    for (int x = x0; x <= x1; x++) {
      redrawCellBackground(x, y);
    }
  }

  if (curMapId == MAP_L1 && powerBlockActive) {
    drawPowerBlock();
  }
}

static inline bool rectHitsClockArea(int px, int py) {
  return !(px + 3 < 13 || px > 50 || py + 3 < 25 || py > 42);
}

static inline int interpPx(int fromCell, int toCell, uint8_t step) {
  return fromCell * CELL_PX + (toCell - fromCell) * step;
}

// =====================================================
// 平滑移動：規劃下一格
// =====================================================
static void planPacmanNextCell(int pX, int pY, int &pDir, int &toX, int &toY, bool &mapChanged) {
  mapChanged = false;

  if (countChoicesNoBack(pX, pY, pDir) >= 2) {
    pDir = chooseTurnAtJunction(pX, pY, pDir, PAC_TURN_CHANCE);
  }

  int nextPX = pX + dx[pDir];
  int nextPY = pY + dy[pDir];

  if (pDir == 0 && nextPX > 15) {
    if (curMapId == MAP_C1)      setMazeById(MAP_R1);
    else if (curMapId == MAP_L1) setMazeById(MAP_C1);

    nextPX = 0;
    mapChanged = true;
  } else if (pDir == 2 && nextPX < 0) {
    if (curMapId == MAP_C1)      setMazeById(MAP_L1);
    else if (curMapId == MAP_R1) setMazeById(MAP_C1);

    nextPX = 15;
    mapChanged = true;
  }

  if (isWalkableCell(nextPX, nextPY)) {
    toX = nextPX;
    toY = nextPY;
    return;
  }

  pDir = chooseDirWhenBlocked(pX, pY, pDir);
  nextPX = pX + dx[pDir];
  nextPY = pY + dy[pDir];

  if (isWalkableCell(nextPX, nextPY)) {
    toX = nextPX;
    toY = nextPY;
  } else {
    toX = pX;
    toY = pY;
  }
}

static void planGhostNextCell(int gX, int gY, int &gDir, int &toX, int &toY) {
  if (countChoicesNoBack(gX, gY, gDir) >= 2) {
    gDir = chooseTurnAtJunction(gX, gY, gDir, GHOST_TURN_CHANCE);
  }

  int nextGX = gX + dx[gDir];
  int nextGY = gY + dy[gDir];

  if (isWalkableCell(nextGX, nextGY)) {
    toX = nextGX;
    toY = nextGY;
    return;
  }

  gDir = chooseDirWhenBlocked(gX, gY, gDir);
  nextGX = gX + dx[gDir];
  nextGY = gY + dy[gDir];

  if (isWalkableCell(nextGX, nextGY)) {
    toX = nextGX;
    toY = nextGY;
  } else {
    toX = gX;
    toY = gY;
  }
}

// =====================================================
// 主遊戲循環：PacmanMode
// =====================================================
void PacmanMode() {
  // --- Pacman 狀態 ---
  static int pX = 1, pY = 1;
  static int pDir = 0;
  static int pFromX = 1, pFromY = 1, pToX = 1, pToY = 1;

  // --- Ghost 動畫狀態 ---
  static int gFromX = 14, gFromY = 7, gToX = 14, gToY = 7;

  // 單一動畫步進：0~4。到 4 代表上一格走完，可決定下一格。
  static uint8_t animStep = ANIM_STEPS;
  static unsigned long lastFrame = 0;
  static int mouthFrame = 0;
  static int lastM = -1;

  // 上一個「已繪製」的像素座標，用來還原背景
  static int lastPPx = -99, lastPPy = -99;
  static int lastGPx = -99, lastGPy = -99;

  // 碰撞偵測用的前一格位置
  static int prevPX = -1, prevPY = -1;
  static int prevGX = -1, prevGY = -1;

  // 用來偵測地圖切換
  static MapId lastMap = MAP_C1;

  // 勝利 / 失敗處理
  if (gameWin || gameOver) {
    if (gameWin) drawStatusText(true);
    else         drawStatusText(false);

    wait_with_display(5000);

    gameWin = false;
    gameOver = false;
    ModefirstRun = true;
    ghostInit = false;
    lastM = -1;
    powerBlockActive = true;
    invincible = false;
    invincibleUntil = 0;
    return;
  }

  // 初始化
  if (ModefirstRun) {
    setMazeById(MAP_C1);

    pX = 8;
    pY = 5;
    pDir = 2;

    pFromX = pToX = pX;
    pFromY = pToY = pY;

    gHX = 14;
    gHY = 7;
    gFromX = gToX = gHX;
    gFromY = gToY = gHY;
    gHDir = 2;

    animStep = ANIM_STEPS;
    lastFrame = 0;
    mouthFrame = 0;

    lastPPx = lastPPy = -99;
    lastGPx = lastGPy = -99;

    prevPX = prevPY = -1;
    prevGX = prevGY = -1;

    ghostInit = false;
    powerBlockActive = true;
    invincible = false;
    ModefirstRun = false;
    allDotsEaten = false;
    lastMap = curMapId;
  }

  // 豆子重置冷卻機制
  if (allDotsEaten && (millis() - dotsEatenTime > 60000)) {
    resetDots();
    allDotsEaten = false;
    powerBlockActive = true;
  }

  // 動畫速度控制：每次只移動 1px，不再 200ms 瞬移 4px。
  unsigned long now = millis();
  if (now - lastFrame < ANIM_FRAME_MS) {
    drawClockOverlay(false);
    return;
  }

  lastFrame = now;

  bool clockDirty = false;

  // 清除上一幀角色圖案，並補回豆子 / Power Block
  if (lastPPx > -90) {
    clockDirty |= rectHitsClockArea(lastPPx, lastPPy);
    restoreSpriteBackgroundPx(lastPPx, lastPPy);
  }

  if (lastGPx > -90) {
    clockDirty |= rectHitsClockArea(lastGPx, lastGPy);
    restoreSpriteBackgroundPx(lastGPx, lastGPy);
  }

  // 上一格走完：提交位置、處理互動，然後決定下一格目標。
  if (animStep >= ANIM_STEPS) {
    prevPX = pX;
    prevPY = pY;
    prevGX = gHX;
    prevGY = gHY;

    pX = pToX;
    pY = pToY;

    if (curMapId == MAP_R1 && ghostInit) {
      gHX = gToX;
      gHY = gToY;
    }

    // 吃道具
    if (curMapId == MAP_L1 && isOnPowerBlock(pX, pY)) {
      powerBlockActive = false;
      clearPowerBlockArea();
      invincible = true;
    }

    // 吃豆子
    if (dotGet(pX, pY)) {
      dotSet(pX, pY, false);
      checkDotsStatus();
    }

    // 碰撞偵測
    if (curMapId == MAP_R1 && ghostInit) {
      bool sameCell  = (gHX == pX && gHY == pY);
      bool crossSwap = (prevPX == gHX && prevPY == gHY && prevGX == pX && prevGY == pY);

      if (sameCell || crossSwap) {
        if (invincible) {
          gameWin = true;
          gameWinAt = millis();
          drawStatusText(true);
        } else {
          gameOver = true;
          gameOverAt = millis();
          drawStatusText(false);
        }

        return;
      }
    }

    // 決定 Pacman 下一格
    pFromX = pX;
    pFromY = pY;

    bool mapChanged = false;
    planPacmanNextCell(pX, pY, pDir, pToX, pToY, mapChanged);

    // 若剛切地圖，不做跨地圖滑動；直接把起點同步到新入口。
    if (mapChanged) {
      pX = pToX;
      pY = pToY;
      pFromX = pToX;
      pFromY = pToY;

      lastPPx = lastPPy = -99;
      lastGPx = lastGPy = -99;

      clockDirty = true;
    }

    // 地圖切換檢查：重畫 PowerBlock
    if (curMapId != lastMap) {
      lastMap = curMapId;

      if (curMapId == MAP_L1 && powerBlockActive) {
        drawPowerBlock();
      }
    }

    // 決定 Ghost 下一格，只在 R1 活動
    if (curMapId == MAP_R1) {
      if (!ghostInit) {
        gHX = 14;
        gHY = 7;

        if (!isWalkableCell(gHX, gHY)) {
          gHX = 13;
          gHY = 7;
        }

        gHDir = 2;
        gFromX = gToX = gHX;
        gFromY = gToY = gHY;
        ghostInit = true;
      }

      gFromX = gHX;
      gFromY = gHY;

      planGhostNextCell(gHX, gHY, gHDir, gToX, gToY);
    } else {
      ghostInit = false;
      lastGPx = lastGPy = -99;
    }

    animStep = 0;
    mouthFrame++;
  }

  // 推進 1px 動畫
  if (animStep < ANIM_STEPS) {
    animStep++;
  }

  const int pPx = interpPx(pFromX, pToX, animStep);
  const int pPy = interpPx(pFromY, pToY, animStep);

  clockDirty |= rectHitsClockArea(pPx, pPy);

  drawPacmanSpritePx(
    pPx,
    pPy,
    pDir,
    invincible,
    (mouthFrame % 2 == 0)
  );

  lastPPx = pPx;
  lastPPy = pPy;

  if (curMapId == MAP_R1 && ghostInit) {
    const int gPx = interpPx(gFromX, gToX, animStep);
    const int gPy = interpPx(gFromY, gToY, animStep);

    clockDirty |= rectHitsClockArea(gPx, gPy);

    drawGhostPx(gPx, gPy);

    lastGPx = gPx;
    lastGPy = gPy;
  }

  // 時鐘顯示放最後，維持原本「時鐘覆蓋在迷宮上」的效果。
  drawClockOverlay(clockDirty);
}
