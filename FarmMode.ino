// =====================================================
// FarmMode.ino
// 牧場物語主題時鐘模式
//
// 需要 Farm.h 提供以下陣列：
//   FARM_MAP
//   FARM_TILE
//   FARM_BOY
//   FARM_COW
//   FARM_SCOW
//   FARM_CHICKEN
//   FARM_SCHICKEN
//   FARM_HOUSE
//   FARM_HOUSE2
//   FARM_HOUSE3
//   FARM_TREE
//   FARM_POOL
//   FARM_EXIT
//   FARM_EAT
//   FARM_EAT2
//   FARM_DRINK
//
// FARM_MAP 規則：
//   0      = 隨機自由目標區，可以被主角隨機抽中
//   1 ~ 6  = 限制區，不可通行
//   7 ~ I  = 作物 / 裝飾顯示區，可通行，但不會被隨機自由目標抽中
//
// 主角行為流程：
//   1. 隨機抽一個 FARM_MAP == 0 的自由區座標
//   2. 用 BFS 路徑移動到該座標
//   3. 到達後抽動作：待機 / 工作 / 睡覺
//   4. 如果抽到工作，再移動到 FARM_WORK_POINTS[] 中的隨機工作點
//   5. 到達工作點後播放工作動畫
// =====================================================

#include <math.h>
#include "Farm.h"


// =====================================================
// 世界與螢幕設定
// =====================================================
static const int FARM_TILE_SIZE = 16;

static const int FARM_MAP_COLS = 38;
static const int FARM_MAP_ROWS = 25;

static const int FARM_WORLD_W = FARM_MAP_COLS * FARM_TILE_SIZE;  // 608
static const int FARM_WORLD_H = FARM_MAP_ROWS * FARM_TILE_SIZE;  // 400

static const int FARM_SCREEN_W = 64;
static const int FARM_SCREEN_H = 64;


// =====================================================
// 背景 tile 設定
//
// FARM_TILE[]：288 x 16
// 單格：16 x 16
// 共 18 frames
//
// FARM_MAP 對應：
//   0      -> frame0
//   2      -> frame1
//   3      -> frame2
//   4      -> frame3
//   5      -> frame4
//   6      -> frame5
//   7      -> frame6
//   8      -> frame7
//   9      -> frame8
//   A / 10 -> frame9
//   B / 11 -> frame10
//   C / 12 -> frame11
//   D / 13 -> frame12
//   E / 14 -> frame13
//   F / 15 -> frame14
//   G / 16 -> frame15
//   H / 17 -> frame16
//   I / 18 -> frame17
//
// 注意：FARM_MAP 1 沒有指定 tile frame，預設不繪製。
// =====================================================
static const int FARM_BG_TILE_W = 16;
static const int FARM_BG_TILE_H = 16;
static const int FARM_TILE_SHEET_W = 288;
static const int FARM_TILE_TOTAL_FRAMES = 18;

static const int FARM_TILE_CODE_1_FRAME = 0 ; // 圖資1 取用frame 0 圖片

// 背景底色。若 tile 沒畫出來，就會看到這個顏色。
static const uint16_t FARM_BG_COLOR = 0x7732;


// =====================================================
// 主角 BOY sprite 設定
//
// FARM_BOY[]：286 x 32
// 單格：22 x 32
// 共 13 frames
//
// 動作：
//   下    = frame 0 ~ 2
//   上    = frame 3 ~ 5
//   右    = frame 6 ~ 8
//   左    = 右 frame 水平翻轉
//   睡覺  = frame 9 ~ 10
//   工作  = frame 11 ~ 12
//   待機  = frame 0
// =====================================================
static const int FARM_BOY_W = 22;
static const int FARM_BOY_H = 32;
static const int FARM_BOY_TOTAL_FRAMES = 13;
static const int FARM_BOY_SHEET_W = 286;

// 所有 sprite 共用透明色
static const uint16_t FARM_TRANSPARENT_COLOR = 0xFBFF;


// =====================================================
// 動物 sprite 設定
// =====================================================

// 牛：162 x 27，單格 27 x 27，共 6 frames
static const int FARM_COW_W = 27;
static const int FARM_COW_H = 27;
static const int FARM_COW_TOTAL_FRAMES = 6;
static const int FARM_COW_SHEET_W = 162;

// 小牛：114 x 17，單格 19 x 17，共 6 frames
static const int FARM_SCOW_W = 19;
static const int FARM_SCOW_H = 17;
static const int FARM_SCOW_TOTAL_FRAMES = 6;
static const int FARM_SCOW_SHEET_W = 114;

// 雞：32 x 15，單格 16 x 15，共 2 frames
static const int FARM_CHICKEN_W = 16;
static const int FARM_CHICKEN_H = 15;
static const int FARM_CHICKEN_TOTAL_FRAMES = 2;
static const int FARM_CHICKEN_SHEET_W = 32;

// 小雞：16 x 10，單格 8 x 10，共 2 frames
static const int FARM_SCHICKEN_W = 8;
static const int FARM_SCHICKEN_H = 10;
static const int FARM_SCHICKEN_TOTAL_FRAMES = 2;
static const int FARM_SCHICKEN_SHEET_W = 16;


// =====================================================
// 固定物件 sprite 尺寸設定
//
// 這些尺寸要跟 Farm.h 裡的圖片陣列完全一致。
// 如果之後重轉圖片，優先檢查這裡。
// =====================================================
static const int FARM_HOUSE_W = 144;
static const int FARM_HOUSE_H = 64;
static const int FARM_HOUSE_SHEET_W = 144;

static const int FARM_HOUSE2_W = 160;
static const int FARM_HOUSE2_H = 48;
static const int FARM_HOUSE2_SHEET_W = 160;

static const int FARM_HOUSE3_W = 32;
static const int FARM_HOUSE3_H = 48;
static const int FARM_HOUSE3_SHEET_W = 32;

static const int FARM_TREE_W = 48;
static const int FARM_TREE_H = 64;
static const int FARM_TREE_SHEET_W = 48;

static const int FARM_POOL_W = 48;
static const int FARM_POOL_H = 64;
static const int FARM_POOL_SHEET_W = 48;

static const int FARM_EXIT_W = 48;
static const int FARM_EXIT_H = 32;
static const int FARM_EXIT_SHEET_W = 48;

static const int FARM_DRINK_W = 32;
static const int FARM_DRINK_H = 19;
static const int FARM_DRINK_SHEET_W = 32;

static const int FARM_EAT_W = 80;
static const int FARM_EAT_H = 21;
static const int FARM_EAT_SHEET_W = 80;

static const int FARM_EAT2_W = 32;
static const int FARM_EAT2_H = 19;
static const int FARM_EAT2_SHEET_W = 32;


// =====================================================
// 主角行為參數
// =====================================================

// 主角移動速度。數值越大，移動越快。
static const float FARM_BOY_SPEED = 1.8f;

// 待機 / 睡覺 / 工作持續時間，單位 ms。
static const unsigned long FARM_BOY_ACTION_HOLD_MS = 10000UL;

// 主角動畫切換間隔，數值越小動畫越快。
static const unsigned long FARM_BOY_ANIM_INTERVAL_MS = 180UL;

// 到達隨機自由區後，抽取動作的機率。
// 三者建議總和為 100。
static const int FARM_BOY_IDLE_RATE = 40;
static const int FARM_BOY_WORK_RATE = 50;
static const int FARM_BOY_SLEEP_RATE = 10;


// =====================================================
// 動物行為參數
// =====================================================

// 動物移動速度。數值越大，移動越快。
static const float FARM_COW_SPEED = 0.45f;
static const float FARM_SCOW_SPEED = 0.45f;
static const float FARM_CHICKEN_SPEED = 0.45f;
static const float FARM_SCHICKEN_SPEED = 0.45f;

// 動物多久重新抽一次方向。
static const unsigned long FARM_ANIMAL_AI_MIN_MS = 900UL;
static const unsigned long FARM_ANIMAL_AI_MAX_MS = 2600UL;

// 動物動畫切換間隔。
static const unsigned long FARM_COW_ANIM_INTERVAL_MS = 240UL;
static const unsigned long FARM_CHICKEN_ANIM_INTERVAL_MS = 180UL;


// =====================================================
// 動物活動範圍
//
// 牛區域：矩形 (0,120) - (127,239)
// 雞區域：矩形 (443,10) - (591,95)
// =====================================================
static const int FARM_COW_AREA_X1 = 0;
static const int FARM_COW_AREA_Y1 = 120;
static const int FARM_COW_AREA_X2 = 127;
static const int FARM_COW_AREA_Y2 = 239;

static const int FARM_CHICKEN_AREA_X1 = 443;
static const int FARM_CHICKEN_AREA_Y1 = 10;
static const int FARM_CHICKEN_AREA_X2 = 591;
static const int FARM_CHICKEN_AREA_Y2 = 95;


// =====================================================
// BFS 路徑設定
// =====================================================
static const int FARM_BFS_NODE_COUNT = FARM_MAP_ROWS * FARM_MAP_COLS;
static const int FARM_MAX_PATH_POINTS = 256;

static int16_t farmBfsPrev[FARM_MAP_ROWS][FARM_MAP_COLS];
static uint16_t farmBfsQueue[FARM_BFS_NODE_COUNT];
static uint16_t farmBfsReversePath[FARM_BFS_NODE_COUNT];

static float farmPathX[FARM_MAX_PATH_POINTS];
static float farmPathY[FARM_MAX_PATH_POINTS];
static int farmPathLen = 0;
static int farmPathIndex = 0;


// =====================================================
// 小工具
// =====================================================
static inline int farmClampI(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static inline float farmClampF(float v, float lo, float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static int farmRandomRange(int lo, int hi) {
  if (hi <= lo) return lo;
  return (int)random(lo, hi + 1);
}


// =====================================================
// Camera
// =====================================================
static int farmCameraX = 0;
static int farmCameraY = 0;


// =====================================================
// 主角狀態
//
// dir:
//   0 = 下
//   1 = 上
//   2 = 右
//   3 = 左
//   4 = 待機 / 停止
//
// mode:
//   FARM_BOY_IDLE  = 待機
//   FARM_BOY_MOVE  = 移動
//   FARM_BOY_SLEEP = 睡覺
//   FARM_BOY_WORK  = 工作
// =====================================================
enum FarmBoyMode {
  FARM_BOY_IDLE = 0,
  FARM_BOY_MOVE = 1,
  FARM_BOY_SLEEP = 2,
  FARM_BOY_WORK = 3
};

static float farmBoyX = 96.0f;
static float farmBoyY = 64.0f;

static float farmBoyTargetX = 96.0f;
static float farmBoyTargetY = 64.0f;

static int farmBoyDir = 4;
static int farmBoyMode = FARM_BOY_IDLE;
static int farmBoyAnimFrame = 0;

static unsigned long farmBoyLastAnimMs = 0;
static unsigned long farmBoyActionUntilMs = 0;


// =====================================================
// 主角工作座標
//
// 抽到「工作」時，會先移動到這些座標之一，
// 到達後才播放工作動畫。
// x,y 是角色左上角世界座標。
// =====================================================
struct FarmWorkPoint {
  int x;
  int y;
};

static const FarmWorkPoint FARM_WORK_POINTS[] = {
  {304, 210},
  {384, 210},
  {464, 210},
  {544, 210},

  {304, 273},
  {384, 273},
  {464, 273},

  {490, 15},   // 雞牧草
  {555, 15},   // 雞水槽

  {35, 130},   // 牛牧草
  {105, 130},  // 牛水槽

  {179, 25},   // 蜂巢
  {48, 280},   // 蜂巢
  {565, 135},  // 蜂巢

  {250, 28},   // 狗窩

  {240, 225}  // 池塘
  
};

static const int FARM_WORK_POINT_COUNT =
  sizeof(FARM_WORK_POINTS) / sizeof(FARM_WORK_POINTS[0]);

// true = 目前正在移動到工作點。
// false = 目前正在移動到一般隨機自由區，或正在播放動作。
static bool farmBoyMovingToWorkTarget = false;


// =====================================================
// 動物資料
// =====================================================
enum FarmAnimalType {
  FARM_ANIMAL_COW = 0,
  FARM_ANIMAL_SCOW = 1,
  FARM_ANIMAL_CHICKEN = 2,
  FARM_ANIMAL_SCHICKEN = 3
};

static const int FARM_ANIMAL_MAX = 7;

struct FarmAnimal {
  bool active;
  uint8_t type;

  float x;
  float y;

  int dir;        // 0下 1上 2右 3左 4待機
  int animFrame;  // 0~1

  unsigned long lastAnimMs;
  unsigned long nextActionMs;
};

static FarmAnimal farmAnimals[FARM_ANIMAL_MAX];


// =====================================================
// 固定物件資料
// x,y 是世界座標，不是螢幕座標。
// =====================================================
enum FarmFixedObjectType {
  FARM_OBJECT_HOUSE = 0,
  FARM_OBJECT_HOUSE2 = 1,
  FARM_OBJECT_HOUSE3 = 2,
  FARM_OBJECT_TREE = 3,
  FARM_OBJECT_POOL = 4,
  FARM_OBJECT_EXIT = 5,
  FARM_OBJECT_EAT = 6,
  FARM_OBJECT_EAT2 = 7,
  FARM_OBJECT_DRINK = 8
};

struct FarmFixedObject {
  uint8_t type;
  int x;
  int y;
};


// =====================================================
// 固定物件清單
//
// 物件會畫在背景上方、動物與主角下方。
// 如果未來想讓樹或水槽遮住角色，可再改成依 bottomY 排序。
// =====================================================
static const FarmFixedObject farmFixedObjects[] = {
  { FARM_OBJECT_HOUSE, 0, 0 },
  { FARM_OBJECT_HOUSE2, 272, 0 },
  { FARM_OBJECT_HOUSE3, 240, 0 },

  { FARM_OBJECT_TREE, 32, 240 },
  { FARM_OBJECT_TREE, 544, 96 },
  { FARM_OBJECT_TREE, 160, -9 },

  { FARM_OBJECT_POOL, 224, 176 },
  { FARM_OBJECT_EXIT, 304, 368 },

  { FARM_OBJECT_EAT, 0, 125 },
  { FARM_OBJECT_EAT2, 544, 13 },

  { FARM_OBJECT_DRINK, 480, 13 },
  { FARM_OBJECT_DRINK, 96, 125 }
};

static const int FARM_FIXED_OBJECT_COUNT =
  sizeof(farmFixedObjects) / sizeof(farmFixedObjects[0]);


// =====================================================
// 讀 FARM_MAP
// =====================================================
static uint8_t getFarmMapTile(int tileX, int tileY) {
  if (tileX < 0 || tileX >= FARM_MAP_COLS || tileY < 0 || tileY >= FARM_MAP_ROWS) {
    return 1;
  }

  return pgm_read_byte(&(FARM_MAP[tileY][tileX]));
}


// =====================================================
// 地圖通行規則
//
// 真正限制移動的只有 1~6。
// 0 與 7~I 都可以通行。
// 但主角「隨機自由目標」只會抽 FARM_MAP == 0。
// =====================================================
static bool farmIsBlockedTileCode(uint8_t tileCode) {
  return (tileCode >= 1 && tileCode <= 6);
}

static bool farmIsWalkableTileCode(uint8_t tileCode) {
  return !farmIsBlockedTileCode(tileCode);
}

static bool farmIsWalkableTile(int tileX, int tileY) {
  if (tileX < 0 || tileX >= FARM_MAP_COLS || tileY < 0 || tileY >= FARM_MAP_ROWS) {
    return false;
  }

  return farmIsWalkableTileCode(getFarmMapTile(tileX, tileY));
}

static bool farmIsRandomFreeTargetTile(int tileX, int tileY) {
  if (tileX < 0 || tileX >= FARM_MAP_COLS || tileY < 0 || tileY >= FARM_MAP_ROWS) {
    return false;
  }

  return getFarmMapTile(tileX, tileY) == 0;
}


// =====================================================
// 主角 / 動物站位判斷
//
// 使用腳底中心點判斷 tile，避免 sprite 寬度超過 16 時太容易卡住。
// =====================================================
static bool farmCanStandAt(float x, float y, int w, int h) {
  if (x < 0 || y < 0) return false;
  if (x + w > FARM_WORLD_W) return false;
  if (y + h > FARM_WORLD_H) return false;

  int footX = (int)(x + (w / 2));
  int footY = (int)(y + h - 2);

  int tileX = footX / FARM_TILE_SIZE;
  int tileY = footY / FARM_TILE_SIZE;

  return farmIsWalkableTile(tileX, tileY);
}

// 保留這個 wrapper 是為了讓主角相關函式語意清楚。
// allowWorkArea 參數目前不再分流，因為現在 0 與 7~I 都可走。
static bool farmCanStandAtForBoy(float x, float y, int w, int h, bool allowWorkArea) {
  (void)allowWorkArea;
  return farmCanStandAt(x, y, w, h);
}


// =====================================================
// 取得角色腳底所在 tile
// =====================================================
static void farmGetActorFootTile(float x, float y, int w, int h, int &tileX, int &tileY) {
  int footX = (int)(x + (w / 2));
  int footY = (int)(y + h - 2);

  tileX = farmClampI(footX / FARM_TILE_SIZE, 0, FARM_MAP_COLS - 1);
  tileY = farmClampI(footY / FARM_TILE_SIZE, 0, FARM_MAP_ROWS - 1);
}


// =====================================================
// tile 轉角色左上角座標
//
// 讓角色腳底中心落在指定 tile 裡。
// =====================================================
static bool farmMakeStandPosForTile(int tileX, int tileY, int w, int h, float &outX, float &outY) {
  float footX = (float)(tileX * FARM_TILE_SIZE + (FARM_TILE_SIZE / 2));
  float footY = (float)(tileY * FARM_TILE_SIZE + FARM_TILE_SIZE - 2);

  outX = footX - (float)(w / 2);
  outY = footY - (float)(h - 2);

  outX = farmClampF(outX, 0.0f, (float)(FARM_WORLD_W - w));
  outY = farmClampF(outY, 0.0f, (float)(FARM_WORLD_H - h));

  return farmCanStandAt(outX, outY, w, h);
}

static bool farmMakeStandPosForTileForBoy(
  int tileX,
  int tileY,
  int w,
  int h,
  bool allowWorkArea,
  float &outX,
  float &outY
) {
  (void)allowWorkArea;
  return farmMakeStandPosForTile(tileX, tileY, w, h, outX, outY);
}


// =====================================================
// 如果角色目前不在可通行區，找最近可站的位置
// =====================================================
static bool farmFindNearestWalkableTile(
  int fromTileX,
  int fromTileY,
  int w,
  int h,
  int &outTileX,
  int &outTileY
) {
  int bestDist = 99999;
  bool found = false;

  for (int ty = 0; ty < FARM_MAP_ROWS; ty++) {
    for (int tx = 0; tx < FARM_MAP_COLS; tx++) {
      if (!farmIsWalkableTile(tx, ty)) continue;

      float sx;
      float sy;

      if (!farmMakeStandPosForTile(tx, ty, w, h, sx, sy)) continue;

      int d = abs(tx - fromTileX) + abs(ty - fromTileY);

      if (d < bestDist) {
        bestDist = d;
        outTileX = tx;
        outTileY = ty;
        found = true;
      }
    }
  }

  return found;
}


// =====================================================
// 隨機找一個 FARM_MAP == 0 的自由目標 tile
//
// 注意：
//   這裡只抽 0。
//   路徑本身仍可經過 7~I。
// =====================================================
static bool farmPickRandomFreeTile(int w, int h, int &outTileX, int &outTileY) {
  for (int tries = 0; tries < 300; tries++) {
    int tx = random(0, FARM_MAP_COLS);
    int ty = random(0, FARM_MAP_ROWS);

    if (!farmIsRandomFreeTargetTile(tx, ty)) continue;

    float sx;
    float sy;

    if (!farmMakeStandPosForTile(tx, ty, w, h, sx, sy)) continue;

    outTileX = tx;
    outTileY = ty;
    return true;
  }

  for (int ty = 0; ty < FARM_MAP_ROWS; ty++) {
    for (int tx = 0; tx < FARM_MAP_COLS; tx++) {
      if (!farmIsRandomFreeTargetTile(tx, ty)) continue;

      float sx;
      float sy;

      if (!farmMakeStandPosForTile(tx, ty, w, h, sx, sy)) continue;

      outTileX = tx;
      outTileY = ty;
      return true;
    }
  }

  return false;
}


// =====================================================
// 清除主角路徑
// =====================================================
static void farmClearPath() {
  farmPathLen = 0;
  farmPathIndex = 0;

  for (int i = 0; i < FARM_MAX_PATH_POINTS; i++) {
    farmPathX[i] = 0.0f;
    farmPathY[i] = 0.0f;
  }
}


// =====================================================
// BFS 建立 tile 路徑
//
// 目前規則：
//   1~6 禁止通行
//   0 與 7~I 可通行
//
// allowWorkArea 參數保留，是為了相容原本呼叫邏輯；
// 目前不再使用它區分通行規則。
// =====================================================
static bool farmBuildPathToTileForBoy(
  int startTileX,
  int startTileY,
  int targetTileX,
  int targetTileY,
  bool allowWorkArea
) {
  (void)allowWorkArea;

  farmClearPath();

  if (!farmIsWalkableTile(startTileX, startTileY)) return false;
  if (!farmIsWalkableTile(targetTileX, targetTileY)) return false;

  if (startTileX == targetTileX && startTileY == targetTileY) {
    return false;
  }

  for (int y = 0; y < FARM_MAP_ROWS; y++) {
    for (int x = 0; x < FARM_MAP_COLS; x++) {
      farmBfsPrev[y][x] = -1;
    }
  }

  int head = 0;
  int tail = 0;

  uint16_t startNode = (uint16_t)(startTileY * FARM_MAP_COLS + startTileX);
  uint16_t targetNode = (uint16_t)(targetTileY * FARM_MAP_COLS + targetTileX);

  farmBfsQueue[tail++] = startNode;
  farmBfsPrev[startTileY][startTileX] = startNode;

  bool found = false;

  while (head < tail) {
    uint16_t current = farmBfsQueue[head++];

    if (current == targetNode) {
      found = true;
      break;
    }

    int cx = current % FARM_MAP_COLS;
    int cy = current / FARM_MAP_COLS;

    const int dx[4] = { 1, -1, 0, 0 };
    const int dy[4] = { 0, 0, 1, -1 };

    for (int i = 0; i < 4; i++) {
      int nx = cx + dx[i];
      int ny = cy + dy[i];

      if (nx < 0 || nx >= FARM_MAP_COLS || ny < 0 || ny >= FARM_MAP_ROWS) continue;
      if (!farmIsWalkableTile(nx, ny)) continue;
      if (farmBfsPrev[ny][nx] != -1) continue;

      uint16_t nextNode = (uint16_t)(ny * FARM_MAP_COLS + nx);

      farmBfsPrev[ny][nx] = current;
      farmBfsQueue[tail++] = nextNode;

      if (tail >= FARM_BFS_NODE_COUNT) break;
    }
  }

  if (!found) return false;

  int reverseLen = 0;
  uint16_t node = targetNode;

  while (node != startNode && reverseLen < FARM_BFS_NODE_COUNT) {
    farmBfsReversePath[reverseLen++] = node;

    int nx = node % FARM_MAP_COLS;
    int ny = node / FARM_MAP_COLS;

    int16_t prev = farmBfsPrev[ny][nx];
    if (prev < 0) break;

    node = (uint16_t)prev;
  }

  if (reverseLen <= 0) return false;

  for (int i = reverseLen - 1; i >= 0; i--) {
    if (farmPathLen >= FARM_MAX_PATH_POINTS) break;

    int tx = farmBfsReversePath[i] % FARM_MAP_COLS;
    int ty = farmBfsReversePath[i] / FARM_MAP_COLS;

    float sx;
    float sy;

    if (!farmMakeStandPosForTile(tx, ty, FARM_BOY_W, FARM_BOY_H, sx, sy)) {
      continue;
    }

    farmPathX[farmPathLen] = sx;
    farmPathY[farmPathLen] = sy;
    farmPathLen++;
  }

  farmPathIndex = 0;

  return farmPathLen > 0;
}


// =====================================================
// FARM_MAP tile code -> FARM_TILE frame
// =====================================================
static int farmTileCodeToFrame(uint8_t tileCode) {
  if (tileCode == 0) return 0;

  if (tileCode == 1) {
    return FARM_TILE_CODE_1_FRAME;
  }

  if (tileCode >= 2 && tileCode <= 18) {
    return tileCode - 1;
  }

  return -1;
}


// =====================================================
// 畫 16x16 背景 tile frame
// =====================================================

//palette 讀色小工具
static inline uint16_t farmReadPaletteColor(
  const uint8_t* sheet,
  const uint16_t* palette,
  uint32_t pixelPos
) {
  uint8_t colorIndex = pgm_read_byte(&(sheet[pixelPos]));
  return pgm_read_word(&(palette[colorIndex]));
}

static void drawFarmTileFrame(
  const uint8_t *sheet,
  const uint16_t *palette,
  int sheetW,
  int frameIndex,
  int screenX,
  int screenY
) {
  if (frameIndex < 0 || frameIndex >= FARM_TILE_TOTAL_FRAMES) return;

  if (screenX <= -FARM_BG_TILE_W || screenX >= FARM_SCREEN_W ||
      screenY <= -FARM_BG_TILE_H || screenY >= FARM_SCREEN_H) {
    return;
  }

  int frameStartX = frameIndex * FARM_BG_TILE_W;

  for (int j = 0; j < FARM_BG_TILE_H; j++) {
    int drawY = screenY + j;
    if (drawY < 0 || drawY >= FARM_SCREEN_H) continue;

    for (int i = 0; i < FARM_BG_TILE_W; i++) {
      int drawX = screenX + i;
      if (drawX < 0 || drawX >= FARM_SCREEN_W) continue;

      uint32_t pixelPos =
        (uint32_t)j * (uint32_t)sheetW +
        (uint32_t)frameStartX +
        (uint32_t)i;

      uint16_t color =
        farmReadPaletteColor(sheet, palette, pixelPos);

      display.drawPixel(drawX, drawY, color);
    }
  }
}


// =====================================================
// 畫單一背景 tile
// =====================================================
static void drawFarmBgTile(uint8_t tileCode, int screenX, int screenY) {
  int frameIndex = farmTileCodeToFrame(tileCode);
  if (frameIndex < 0) return;

  drawFarmTileFrame(
    FARM_TILE,
    FARM_TILE_PALETTE,
    FARM_TILE_SHEET_W,
    frameIndex,
    screenX,
    screenY
  );
}


// =====================================================
// 背景 crop 重繪
// =====================================================
static void drawFarmBackgroundCrop() {
  display.fillScreen(FARM_BG_COLOR);

  int startTileX = farmCameraX / FARM_TILE_SIZE;
  int startTileY = farmCameraY / FARM_TILE_SIZE;
  int endTileX = (farmCameraX + FARM_SCREEN_W - 1) / FARM_TILE_SIZE;
  int endTileY = (farmCameraY + FARM_SCREEN_H - 1) / FARM_TILE_SIZE;

  for (int ty = startTileY; ty <= endTileY; ty++) {
    for (int tx = startTileX; tx <= endTileX; tx++) {
      uint8_t tileCode = getFarmMapTile(tx, ty);

      int worldX = tx * FARM_TILE_SIZE;
      int worldY = ty * FARM_TILE_SIZE;

      int screenX = worldX - farmCameraX;
      int screenY = worldY - farmCameraY;

      drawFarmBgTile(tileCode, screenX, screenY);
    }
  }
}


// =====================================================
// 通用 sprite frame 繪製
//
// sheetW      = 圖片陣列實際總寬
// spriteW/H   = 單一 frame 寬高
// totalFrames = frame 數
// frameIndex  = 要畫第幾格
// mirrorX     = true 時水平翻轉
// =====================================================
static void drawFarmSpriteFrame(
  const uint8_t *sheet,
  const uint16_t *palette,
  int sheetW,
  int spriteW,
  int spriteH,
  int totalFrames,
  int frameIndex,
  int screenX,
  int screenY,
  bool mirrorX
) {
  if (frameIndex < 0 || frameIndex >= totalFrames) return;

  if (screenX <= -spriteW || screenX >= FARM_SCREEN_W ||
      screenY <= -spriteH || screenY >= FARM_SCREEN_H) {
    return;
  }

  int frameStartX = frameIndex * spriteW;

  for (int j = 0; j < spriteH; j++) {
    int drawY = screenY + j;
    if (drawY < 0 || drawY >= FARM_SCREEN_H) continue;

    for (int i = 0; i < spriteW; i++) {
      int srcX = mirrorX ? (spriteW - 1 - i) : i;
      int drawX = screenX + i;
      if (drawX < 0 || drawX >= FARM_SCREEN_W) continue;

      uint32_t pixelPos =
        (uint32_t)j * (uint32_t)sheetW +
        (uint32_t)frameStartX +
        (uint32_t)srcX;

      uint16_t color =
        farmReadPaletteColor(sheet, palette, pixelPos);

      if (color == FARM_TRANSPARENT_COLOR) continue;

      display.drawPixel(drawX, drawY, color);
    }
  }
}


// =====================================================
// 畫固定物件
// =====================================================
static void drawFarmFixedObjectByIndex(int idx) {
  if (idx < 0 || idx >= FARM_FIXED_OBJECT_COUNT) return;

  uint8_t type = farmFixedObjects[idx].type;

  int screenX = farmFixedObjects[idx].x - farmCameraX;
  int screenY = farmFixedObjects[idx].y - farmCameraY;

  if (type == FARM_OBJECT_HOUSE) {
    drawFarmSpriteFrame(
      FARM_HOUSE,
      FARM_HOUSE_PALETTE,
      FARM_HOUSE_SHEET_W,
      FARM_HOUSE_W,
      FARM_HOUSE_H,
      1,
      0,
      screenX,
      screenY,
      false
    );
    return;
  }

  if (type == FARM_OBJECT_HOUSE2) {
    drawFarmSpriteFrame(
      FARM_HOUSE2,
      FARM_HOUSE2_PALETTE,
      FARM_HOUSE2_SHEET_W,
      FARM_HOUSE2_W,
      FARM_HOUSE2_H,
      1,
      0,
      screenX,
      screenY,
      false
    );
    return;
  }

  if (type == FARM_OBJECT_HOUSE3) {
    drawFarmSpriteFrame(
      FARM_HOUSE3,
      FARM_HOUSE3_PALETTE,
      FARM_HOUSE3_SHEET_W,
      FARM_HOUSE3_W,
      FARM_HOUSE3_H,
      1,
      0,
      screenX,
      screenY,
      false
    );
    return;
  }

  if (type == FARM_OBJECT_TREE) {
    drawFarmSpriteFrame(
      FARM_TREE,
      FARM_TREE_PALETTE,
      FARM_TREE_SHEET_W,
      FARM_TREE_W,
      FARM_TREE_H,
      1,
      0,
      screenX,
      screenY,
      false
    );
    return;
  }

  if (type == FARM_OBJECT_POOL) {
    drawFarmSpriteFrame(
      FARM_POOL,
      FARM_POOL_PALETTE,
      FARM_POOL_SHEET_W,
      FARM_POOL_W,
      FARM_POOL_H,
      1,
      0,
      screenX,
      screenY,
      false
    );
    return;
  }

  if (type == FARM_OBJECT_EXIT) {
    drawFarmSpriteFrame(
      FARM_EXIT,
      FARM_EXIT_PALETTE,
      FARM_EXIT_SHEET_W,
      FARM_EXIT_W,
      FARM_EXIT_H,
      1,
      0,
      screenX,
      screenY,
      false
    );
    return;
  }

  if (type == FARM_OBJECT_DRINK) {
    drawFarmSpriteFrame(
      FARM_DRINK,
      FARM_DRINK_PALETTE,
      FARM_DRINK_SHEET_W,
      FARM_DRINK_W,
      FARM_DRINK_H,
      1,
      0,
      screenX,
      screenY,
      false
    );
    return;
  }

  if (type == FARM_OBJECT_EAT) {
    drawFarmSpriteFrame(
      FARM_EAT,
      FARM_EAT_PALETTE,
      FARM_EAT_SHEET_W,
      FARM_EAT_W,
      FARM_EAT_H,
      1,
      0,
      screenX,
      screenY,
      false
    );
    return;
  }

  if (type == FARM_OBJECT_EAT2) {
    drawFarmSpriteFrame(
      FARM_EAT2,
      FARM_EAT2_PALETTE,
      FARM_EAT2_SHEET_W,
      FARM_EAT2_W,
      FARM_EAT2_H,
      1,
      0,
      screenX,
      screenY,
      false
    );
    return;
  }
}


// =====================================================
// 畫全部固定物件
// =====================================================
static void drawFarmFixedObjects() {
  for (int i = 0; i < FARM_FIXED_OBJECT_COUNT; i++) {
    drawFarmFixedObjectByIndex(i);
  }
}


// =====================================================
// 取得主角目前 frame
// =====================================================
static int getFarmBoyFrame(bool &mirrorX) {
  mirrorX = false;

  if (farmBoyMode == FARM_BOY_SLEEP) {
    return 9 + (farmBoyAnimFrame % 2);
  }

  if (farmBoyMode == FARM_BOY_WORK) {
    return 11 + (farmBoyAnimFrame % 2);
  }

  if (farmBoyMode == FARM_BOY_IDLE || farmBoyDir == 4) {
    return 0;
  }

  int f = farmBoyAnimFrame % 3;

  if (farmBoyDir == 0) return 0 + f;
  if (farmBoyDir == 1) return 3 + f;
  if (farmBoyDir == 2) return 6 + f;

  if (farmBoyDir == 3) {
    mirrorX = true;
    return 6 + f;
  }

  return 0;
}


// =====================================================
// 畫主角
// =====================================================
static void drawFarmBoy() {
  bool mirrorX = false;
  int frameIndex = getFarmBoyFrame(mirrorX);

  int screenX = (int)farmBoyX - farmCameraX;
  int screenY = (int)farmBoyY - farmCameraY;

  drawFarmSpriteFrame(
    FARM_BOY,
    FARM_BOY_PALETTE,
    FARM_BOY_SHEET_W,
    FARM_BOY_W,
    FARM_BOY_H,
    FARM_BOY_TOTAL_FRAMES,
    frameIndex,
    screenX,
    screenY,
    mirrorX
  );
}


// =====================================================
// 動物尺寸
// =====================================================
static void getFarmAnimalSizeByType(uint8_t type, int &w, int &h) {
  if (type == FARM_ANIMAL_COW) {
    w = FARM_COW_W;
    h = FARM_COW_H;
  } else if (type == FARM_ANIMAL_SCOW) {
    w = FARM_SCOW_W;
    h = FARM_SCOW_H;
  } else if (type == FARM_ANIMAL_CHICKEN) {
    w = FARM_CHICKEN_W;
    h = FARM_CHICKEN_H;
  } else {
    w = FARM_SCHICKEN_W;
    h = FARM_SCHICKEN_H;
  }
}


// =====================================================
// 動物活動範圍
// =====================================================
static void getFarmAnimalAreaByType(uint8_t type, int &x1, int &y1, int &x2, int &y2) {
  if (type == FARM_ANIMAL_COW || type == FARM_ANIMAL_SCOW) {
    x1 = FARM_COW_AREA_X1;
    y1 = FARM_COW_AREA_Y1;
    x2 = FARM_COW_AREA_X2;
    y2 = FARM_COW_AREA_Y2;
  } else {
    x1 = FARM_CHICKEN_AREA_X1;
    y1 = FARM_CHICKEN_AREA_Y1;
    x2 = FARM_CHICKEN_AREA_X2;
    y2 = FARM_CHICKEN_AREA_Y2;
  }
}


// =====================================================
// 動物速度
// =====================================================
static float getFarmAnimalSpeedByType(uint8_t type) {
  if (type == FARM_ANIMAL_COW) return FARM_COW_SPEED;
  if (type == FARM_ANIMAL_SCOW) return FARM_SCOW_SPEED;
  if (type == FARM_ANIMAL_CHICKEN) return FARM_CHICKEN_SPEED;
  return FARM_SCHICKEN_SPEED;
}


// =====================================================
// 動物動畫間隔
// =====================================================
static unsigned long getFarmAnimalAnimIntervalByType(uint8_t type) {
  if (type == FARM_ANIMAL_COW || type == FARM_ANIMAL_SCOW) {
    return FARM_COW_ANIM_INTERVAL_MS;
  }

  return FARM_CHICKEN_ANIM_INTERVAL_MS;
}


// =====================================================
// 在指定活動矩形中找可站點
// =====================================================
static bool farmFindRandomStandInArea(
  int areaX1,
  int areaY1,
  int areaX2,
  int areaY2,
  int w,
  int h,
  float &outX,
  float &outY
) {
  int maxX = areaX2 - w;
  int maxY = areaY2 - h;

  if (maxX < areaX1) maxX = areaX1;
  if (maxY < areaY1) maxY = areaY1;

  for (int tries = 0; tries < 120; tries++) {
    float rx = (float)farmRandomRange(areaX1, maxX);
    float ry = (float)farmRandomRange(areaY1, maxY);

    if (farmCanStandAt(rx, ry, w, h)) {
      outX = rx;
      outY = ry;
      return true;
    }
  }

  for (int y = areaY1; y <= maxY; y += 2) {
    for (int x = areaX1; x <= maxX; x += 2) {
      if (farmCanStandAt((float)x, (float)y, w, h)) {
        outX = (float)x;
        outY = (float)y;
        return true;
      }
    }
  }

  return false;
}


// =====================================================
// 取得動物 frame
//
// 牛 / 小牛：
//   下 = 0~1
//   上 = 2~3
//   右 = 4~5
//   左 = 右水平翻轉
//
// 雞 / 小雞：
//   左 = 0~1
//   右 = 左水平翻轉
//   上下 = frame0
// =====================================================
static int getFarmAnimalFrameByIndex(int idx, bool &mirrorX) {
  mirrorX = false;

  uint8_t type = farmAnimals[idx].type;
  int dir = farmAnimals[idx].dir;
  int f = farmAnimals[idx].animFrame % 2;

  if (type == FARM_ANIMAL_COW || type == FARM_ANIMAL_SCOW) {
    if (dir == 0) return 0 + f;
    if (dir == 1) return 2 + f;
    if (dir == 2) return 4 + f;

    if (dir == 3) {
      mirrorX = true;
      return 4 + f;
    }

    return 0;
  }

  if (dir == 3) {
    return f;
  }

  if (dir == 2) {
    mirrorX = true;
    return f;
  }

  return 0;
}


// =====================================================
// 畫動物
// =====================================================
static void drawFarmAnimalByIndex(int idx) {
  if (idx < 0 || idx >= FARM_ANIMAL_MAX) return;
  if (!farmAnimals[idx].active) return;

  bool mirrorX = false;
  int frameIndex = getFarmAnimalFrameByIndex(idx, mirrorX);

  int screenX = (int)farmAnimals[idx].x - farmCameraX;
  int screenY = (int)farmAnimals[idx].y - farmCameraY;

  uint8_t type = farmAnimals[idx].type;

  if (type == FARM_ANIMAL_COW) {
    drawFarmSpriteFrame(
      FARM_COW,
      FARM_COW_PALETTE,
      FARM_COW_SHEET_W,
      FARM_COW_W,
      FARM_COW_H,
      FARM_COW_TOTAL_FRAMES,
      frameIndex,
      screenX,
      screenY,
      mirrorX
    );
    return;
  }

  if (type == FARM_ANIMAL_SCOW) {
    drawFarmSpriteFrame(
      FARM_SCOW,
      FARM_SCOW_PALETTE,
      FARM_SCOW_SHEET_W,
      FARM_SCOW_W,
      FARM_SCOW_H,
      FARM_SCOW_TOTAL_FRAMES,
      frameIndex,
      screenX,
      screenY,
      mirrorX
    );
    return;
  }

  if (type == FARM_ANIMAL_CHICKEN) {
    drawFarmSpriteFrame(
      FARM_CHICKEN,
      FARM_CHICKEN_PALETTE,
      FARM_CHICKEN_SHEET_W,
      FARM_CHICKEN_W,
      FARM_CHICKEN_H,
      FARM_CHICKEN_TOTAL_FRAMES,
      frameIndex,
      screenX,
      screenY,
      mirrorX
    );
    return;
  }

  drawFarmSpriteFrame(
    FARM_SCHICKEN,
    FARM_SCHICKEN_PALETTE,
    FARM_SCHICKEN_SHEET_W,
    FARM_SCHICKEN_W,
    FARM_SCHICKEN_H,
    FARM_SCHICKEN_TOTAL_FRAMES,
    frameIndex,
    screenX,
    screenY,
    mirrorX
  );
}


// =====================================================
// 初始化單隻動物
// =====================================================
static void setupFarmAnimal(int idx, uint8_t type) {
  if (idx < 0 || idx >= FARM_ANIMAL_MAX) return;

  int w;
  int h;
  int x1;
  int y1;
  int x2;
  int y2;

  getFarmAnimalSizeByType(type, w, h);
  getFarmAnimalAreaByType(type, x1, y1, x2, y2);

  float sx = (float)x1;
  float sy = (float)y1;

  bool placed = farmFindRandomStandInArea(x1, y1, x2, y2, w, h, sx, sy);

  farmAnimals[idx].active = true;
  farmAnimals[idx].type = type;

  if (placed) {
    farmAnimals[idx].x = sx;
    farmAnimals[idx].y = sy;
  } else {
    farmAnimals[idx].x = (float)x1;
    farmAnimals[idx].y = (float)y1;
  }

  farmAnimals[idx].dir = random(0, 5);
  farmAnimals[idx].animFrame = 0;
  farmAnimals[idx].lastAnimMs = millis();
  farmAnimals[idx].nextActionMs =
    millis() + random(FARM_ANIMAL_AI_MIN_MS, FARM_ANIMAL_AI_MAX_MS + 1);
}


// =====================================================
// 初始化全部動物
//
// 牛 x1
// 小牛 x2
// 雞 x2
// 小雞 x2
// =====================================================
static void initFarmAnimals() {
  int idx = 0;

  setupFarmAnimal(idx++, FARM_ANIMAL_COW);

  setupFarmAnimal(idx++, FARM_ANIMAL_SCOW);
  setupFarmAnimal(idx++, FARM_ANIMAL_SCOW);

  setupFarmAnimal(idx++, FARM_ANIMAL_CHICKEN);
  setupFarmAnimal(idx++, FARM_ANIMAL_CHICKEN);

  setupFarmAnimal(idx++, FARM_ANIMAL_SCHICKEN);
  setupFarmAnimal(idx++, FARM_ANIMAL_SCHICKEN);
}


// =====================================================
// 到達工作座標後，開始播放工作動畫
// =====================================================
static void farmStartBoyWorkAtTarget(unsigned long nowMs) {
  farmBoyMovingToWorkTarget = false;

  farmBoyMode = FARM_BOY_WORK;
  farmBoyDir = 4;
  farmBoyAnimFrame = 0;
  farmBoyActionUntilMs = nowMs + FARM_BOY_ACTION_HOLD_MS;
}


// =====================================================
// 抽到工作時，先移動到隨機工作座標
//
// 成功：進入 MOVE，並設定 farmBoyMovingToWorkTarget = true
// 失敗：回傳 false，由呼叫端決定 fallback
// =====================================================
static bool farmStartBoyMoveToRandomWorkTarget(unsigned long nowMs) {
  (void)nowMs;

  int startTileX;
  int startTileY;

  farmGetActorFootTile(
    farmBoyX,
    farmBoyY,
    FARM_BOY_W,
    FARM_BOY_H,
    startTileX,
    startTileY
  );

  // 用較多次數抽工作點，避免隨機重複抽到無效點。
  const int WORK_TARGET_TRIES = FARM_WORK_POINT_COUNT * 3;

  for (int tries = 0; tries < WORK_TARGET_TRIES; tries++) {
    int targetIndex = random(0, FARM_WORK_POINT_COUNT);

    float targetX = (float)FARM_WORK_POINTS[targetIndex].x;
    float targetY = (float)FARM_WORK_POINTS[targetIndex].y;

    targetX = farmClampF(targetX, 0.0f, (float)(FARM_WORLD_W - FARM_BOY_W));
    targetY = farmClampF(targetY, 0.0f, (float)(FARM_WORLD_H - FARM_BOY_H));

    if (!farmCanStandAtForBoy(targetX, targetY, FARM_BOY_W, FARM_BOY_H, true)) {
      continue;
    }

    int targetTileX;
    int targetTileY;

    farmGetActorFootTile(
      targetX,
      targetY,
      FARM_BOY_W,
      FARM_BOY_H,
      targetTileX,
      targetTileY
    );

    if (farmBuildPathToTileForBoy(startTileX, startTileY, targetTileX, targetTileY, true)) {
      farmBoyMode = FARM_BOY_MOVE;
      farmBoyMovingToWorkTarget = true;
      farmBoyAnimFrame = 0;
      farmBoyActionUntilMs = 0;

      farmPathIndex = 0;

      // 讓路徑最後一點精準落在你指定的工作座標。
      if (farmPathLen > 0) {
        farmPathX[farmPathLen - 1] = targetX;
        farmPathY[farmPathLen - 1] = targetY;
      }

      farmBoyTargetX = farmPathX[0];
      farmBoyTargetY = farmPathY[0];

      return true;
    }
  }

  return false;
}


// =====================================================
// 主角到達隨機自由區座標後，抽取動作
//
// 待機：原地待機
// 睡覺：原地睡覺
// 工作：先移動到 FARM_WORK_POINTS[]，到達後再播放工作動畫
// =====================================================
static void farmStartBoyRandomActionAtDestination(unsigned long nowMs) {
  int dice = random(0, 100);

  farmBoyAnimFrame = 0;

  if (dice < FARM_BOY_IDLE_RATE) {
    farmBoyMode = FARM_BOY_IDLE;
    farmBoyDir = 4;
    farmBoyActionUntilMs = nowMs + FARM_BOY_ACTION_HOLD_MS;
    return;
  }

  if (dice < FARM_BOY_IDLE_RATE + FARM_BOY_WORK_RATE) {
    if (farmStartBoyMoveToRandomWorkTarget(nowMs)) {
      return;
    }

    // 如果工作座標找不到路徑，退回原地工作，避免角色卡死。
    farmStartBoyWorkAtTarget(nowMs);
    return;
  }

  farmBoyMode = FARM_BOY_SLEEP;
  farmBoyDir = 4;
  farmBoyActionUntilMs = nowMs + FARM_BOY_ACTION_HOLD_MS;
}


// =====================================================
// 主角開始移動到隨機自由區座標
//
// 只抽 FARM_MAP == 0 的 tile 當目的地。
// 路徑可以經過 7~I。
// =====================================================
static bool farmStartBoyMoveToRandomFreeTile(unsigned long nowMs) {
  int startTileX;
  int startTileY;

  farmGetActorFootTile(farmBoyX, farmBoyY, FARM_BOY_W, FARM_BOY_H, startTileX, startTileY);

  if (!farmIsWalkableTile(startTileX, startTileY)) {
    int nearX = startTileX;
    int nearY = startTileY;

    if (farmFindNearestWalkableTile(startTileX, startTileY, FARM_BOY_W, FARM_BOY_H, nearX, nearY)) {
      float sx;
      float sy;

      if (farmMakeStandPosForTile(nearX, nearY, FARM_BOY_W, FARM_BOY_H, sx, sy)) {
        farmBoyX = sx;
        farmBoyY = sy;
        startTileX = nearX;
        startTileY = nearY;
      }
    }
  }

  for (int tries = 0; tries < 80; tries++) {
    int targetTileX;
    int targetTileY;

    if (!farmPickRandomFreeTile(FARM_BOY_W, FARM_BOY_H, targetTileX, targetTileY)) {
      break;
    }

    if (targetTileX == startTileX && targetTileY == startTileY) {
      continue;
    }

    if (farmBuildPathToTileForBoy(startTileX, startTileY, targetTileX, targetTileY, false)) {
      farmBoyMode = FARM_BOY_MOVE;
      farmBoyMovingToWorkTarget = false;
      farmBoyAnimFrame = 0;
      farmBoyActionUntilMs = 0;

      farmPathIndex = 0;
      farmBoyTargetX = farmPathX[0];
      farmBoyTargetY = farmPathY[0];

      return true;
    }
  }

  farmBoyMode = FARM_BOY_IDLE;
  farmBoyDir = 4;
  farmBoyAnimFrame = 0;
  farmBoyActionUntilMs = nowMs + 2000UL;

  return false;
}


// =====================================================
// 更新主角 AI
// =====================================================
static void updateFarmBoyAI(unsigned long nowMs) {
  if (farmBoyMode == FARM_BOY_MOVE) {
    return;
  }

  if (farmBoyActionUntilMs > 0 && nowMs < farmBoyActionUntilMs) {
    return;
  }

  farmStartBoyMoveToRandomFreeTile(nowMs);
}


// =====================================================
// 更新主角動畫
// =====================================================
static void updateFarmBoyAnimation(unsigned long nowMs) {
  if (nowMs - farmBoyLastAnimMs < FARM_BOY_ANIM_INTERVAL_MS) return;

  farmBoyLastAnimMs = nowMs;

  if (farmBoyMode == FARM_BOY_IDLE) {
    farmBoyAnimFrame = 0;
    return;
  }

  if (farmBoyMode == FARM_BOY_MOVE) {
    farmBoyAnimFrame++;
    if (farmBoyAnimFrame >= 3) farmBoyAnimFrame = 0;
    return;
  }

  farmBoyAnimFrame++;
  if (farmBoyAnimFrame >= 2) farmBoyAnimFrame = 0;
}


// =====================================================
// 更新主角移動
//
// 依 BFS path 一格一格前進。
// 如果是工作移動，到達後播放工作動畫。
// 如果是一般自由區移動，到達後抽下一個動作。
// =====================================================
static void updateFarmBoyMovement(unsigned long nowMs) {
  if (farmBoyMode != FARM_BOY_MOVE) return;

  if (farmPathIndex >= farmPathLen) {
    if (farmBoyMovingToWorkTarget) {
      farmStartBoyWorkAtTarget(nowMs);
    } else {
      farmStartBoyRandomActionAtDestination(nowMs);
    }
    return;
  }

  farmBoyTargetX = farmPathX[farmPathIndex];
  farmBoyTargetY = farmPathY[farmPathIndex];

  float dx = farmBoyTargetX - farmBoyX;
  float dy = farmBoyTargetY - farmBoyY;

  float adx = fabsf(dx);
  float ady = fabsf(dy);

  if (adx <= FARM_BOY_SPEED && ady <= FARM_BOY_SPEED) {
    farmBoyX = farmBoyTargetX;
    farmBoyY = farmBoyTargetY;

    farmPathIndex++;

    if (farmPathIndex >= farmPathLen) {
      if (farmBoyMovingToWorkTarget) {
        farmStartBoyWorkAtTarget(nowMs);
      } else {
        farmStartBoyRandomActionAtDestination(nowMs);
      }
      return;
    }

    farmBoyTargetX = farmPathX[farmPathIndex];
    farmBoyTargetY = farmPathY[farmPathIndex];

    dx = farmBoyTargetX - farmBoyX;
    dy = farmBoyTargetY - farmBoyY;
    adx = fabsf(dx);
    ady = fabsf(dy);
  }

  if (adx > ady) {
    farmBoyDir = (dx >= 0.0f) ? 2 : 3;
  } else {
    farmBoyDir = (dy >= 0.0f) ? 0 : 1;
  }

  float dist = sqrtf(dx * dx + dy * dy);
  if (dist <= 0.001f) return;

  float stepX = (dx / dist) * FARM_BOY_SPEED;
  float stepY = (dy / dist) * FARM_BOY_SPEED;

  float nx = farmBoyX + stepX;
  float ny = farmBoyY + stepY;

  nx = farmClampF(nx, 0.0f, (float)(FARM_WORLD_W - FARM_BOY_W));
  ny = farmClampF(ny, 0.0f, (float)(FARM_WORLD_H - FARM_BOY_H));

  if (farmCanStandAtForBoy(nx, ny, FARM_BOY_W, FARM_BOY_H, true)) {
    farmBoyX = nx;
    farmBoyY = ny;
    return;
  }

  bool moved = false;

  float tryX = farmClampF(farmBoyX + stepX, 0.0f, (float)(FARM_WORLD_W - FARM_BOY_W));
  if (farmCanStandAtForBoy(tryX, farmBoyY, FARM_BOY_W, FARM_BOY_H, true)) {
    farmBoyX = tryX;
    moved = true;
  }

  float tryY = farmClampF(farmBoyY + stepY, 0.0f, (float)(FARM_WORLD_H - FARM_BOY_H));
  if (farmCanStandAtForBoy(farmBoyX, tryY, FARM_BOY_W, FARM_BOY_H, true)) {
    farmBoyY = tryY;
    moved = true;
  }

  if (!moved) {
    farmStartBoyMoveToRandomFreeTile(nowMs);
  }
}


// =====================================================
// 更新動物 AI
// =====================================================
static void updateFarmAnimalAI(unsigned long nowMs) {
  for (int i = 0; i < FARM_ANIMAL_MAX; i++) {
    if (!farmAnimals[i].active) continue;

    if (nowMs < farmAnimals[i].nextActionMs) continue;

    int dice = random(0, 100);

    if (dice < 25) {
      farmAnimals[i].dir = 4;
    } else {
      farmAnimals[i].dir = random(0, 4);
    }

    farmAnimals[i].nextActionMs =
      nowMs + random(FARM_ANIMAL_AI_MIN_MS, FARM_ANIMAL_AI_MAX_MS + 1);
  }
}


// =====================================================
// 更新動物動畫
// =====================================================
static void updateFarmAnimalAnimation(unsigned long nowMs) {
  for (int i = 0; i < FARM_ANIMAL_MAX; i++) {
    if (!farmAnimals[i].active) continue;

    unsigned long intervalMs = getFarmAnimalAnimIntervalByType(farmAnimals[i].type);

    if (nowMs - farmAnimals[i].lastAnimMs < intervalMs) continue;

    farmAnimals[i].lastAnimMs = nowMs;

    if (farmAnimals[i].dir == 4) {
      farmAnimals[i].animFrame = 0;
      continue;
    }

    if (farmAnimals[i].type == FARM_ANIMAL_CHICKEN ||
        farmAnimals[i].type == FARM_ANIMAL_SCHICKEN) {
      if (farmAnimals[i].dir == 2 || farmAnimals[i].dir == 3) {
        farmAnimals[i].animFrame++;
        if (farmAnimals[i].animFrame >= 2) farmAnimals[i].animFrame = 0;
      } else {
        farmAnimals[i].animFrame = 0;
      }
    } else {
      farmAnimals[i].animFrame++;
      if (farmAnimals[i].animFrame >= 2) farmAnimals[i].animFrame = 0;
    }
  }
}


// =====================================================
// 更新動物位移
//
// 動物限制在自己的活動矩形內。
// 通行規則同樣是 1~6 禁止，0 與 7~I 可走。
// =====================================================
static void updateFarmAnimalMovement() {
  for (int i = 0; i < FARM_ANIMAL_MAX; i++) {
    if (!farmAnimals[i].active) continue;
    if (farmAnimals[i].dir == 4) continue;

    int w;
    int h;
    int x1;
    int y1;
    int x2;
    int y2;

    getFarmAnimalSizeByType(farmAnimals[i].type, w, h);
    getFarmAnimalAreaByType(farmAnimals[i].type, x1, y1, x2, y2);

    int maxX = x2 - w;
    int maxY = y2 - h;

    if (maxX < x1) maxX = x1;
    if (maxY < y1) maxY = y1;

    float speed = getFarmAnimalSpeedByType(farmAnimals[i].type);

    float nx = farmAnimals[i].x;
    float ny = farmAnimals[i].y;

    if (farmAnimals[i].dir == 0) {
      ny += speed;
    } else if (farmAnimals[i].dir == 1) {
      ny -= speed;
    } else if (farmAnimals[i].dir == 2) {
      nx += speed;
    } else if (farmAnimals[i].dir == 3) {
      nx -= speed;
    }

    nx = farmClampF(nx, (float)x1, (float)maxX);
    ny = farmClampF(ny, (float)y1, (float)maxY);

    if (farmCanStandAt(nx, ny, w, h)) {
      farmAnimals[i].x = nx;
      farmAnimals[i].y = ny;
    } else {
      farmAnimals[i].dir = random(0, 5);
      farmAnimals[i].nextActionMs = millis() + random(500, 1200);
    }
  }
}


// =====================================================
// 更新 Camera
//
// boyCenterY 的 -14 會讓主角在畫面中顯示得比較下面。
// 數值越小，例如 -18，主角越靠下。
// 數值越大，例如 -6，主角越靠中間。
// =====================================================
static void updateFarmCamera() {
  int boyCenterX = (int)farmBoyX + (FARM_BOY_W / 2);
  int boyCenterY = (int)farmBoyY + (FARM_BOY_H / 2) - 14;

  farmCameraX = farmClampI(
    boyCenterX - (FARM_SCREEN_W / 2),
    0,
    FARM_WORLD_W - FARM_SCREEN_W
  );

  farmCameraY = farmClampI(
    boyCenterY - (FARM_SCREEN_H / 2),
    0,
    FARM_WORLD_H - FARM_SCREEN_H
  );
}


// =====================================================
// 整幀重繪
//
// 繪製順序：
//   1. 背景 tile
//   2. 固定物件
//   3. 腳底在主角後面的動物
//   4. 主角
//   5. 腳底在主角前面的動物
//   6. 時鐘文字
// =====================================================
static void renderFarmFullFrame() {
  drawFarmBackgroundCrop();

  drawFarmFixedObjects();

  int boyBottom = (int)farmBoyY + FARM_BOY_H;

  for (int i = 0; i < FARM_ANIMAL_MAX; i++) {
    if (!farmAnimals[i].active) continue;

    int w;
    int h;

    getFarmAnimalSizeByType(farmAnimals[i].type, w, h);

    int animalBottom = (int)farmAnimals[i].y + h;

    if (animalBottom <= boyBottom) {
      drawFarmAnimalByIndex(i);
    }
  }

  drawFarmBoy();

  for (int i = 0; i < FARM_ANIMAL_MAX; i++) {
    if (!farmAnimals[i].active) continue;

    int w;
    int h;

    getFarmAnimalSizeByType(farmAnimals[i].type, w, h);

    int animalBottom = (int)farmAnimals[i].y + h;

    if (animalBottom > boyBottom) {
      drawFarmAnimalByIndex(i);
    }
  }

  drawThemeClockText();
}


// =====================================================
// Serial 測試控制
//
// R / r：重啟
// 輸入 x,y：如果座標可站，主角跳到該世界座標
// =====================================================
static void FarmSerialControl() {
  if (Serial.available() <= 0) return;

  char firstChar = Serial.peek();

  if (firstChar == 'R' || firstChar == 'r') {
    Serial.read();
    Serial.println("Restarting ESP32-C6...");
    Serial.flush();
    delay(200);
    ESP.restart();
    return;
  }

  if (firstChar >= '0' && firstChar <= '9') {
    int tx = Serial.parseInt();
    int ty = Serial.parseInt();

    float nx = farmClampF((float)tx, 0.0f, (float)(FARM_WORLD_W - FARM_BOY_W));
    float ny = farmClampF((float)ty, 0.0f, (float)(FARM_WORLD_H - FARM_BOY_H));

    if (farmCanStandAt(nx, ny, FARM_BOY_W, FARM_BOY_H)) {
      farmBoyX = nx;
      farmBoyY = ny;

      farmBoyMode = FARM_BOY_IDLE;
      farmBoyDir = 4;
      farmBoyAnimFrame = 0;
      farmBoyActionUntilMs = millis() + 1000UL;
      farmBoyMovingToWorkTarget = false;
      farmClearPath();

      updateFarmCamera();

      Serial.print("[FARM JUMP] X:");
      Serial.print(farmBoyX);
      Serial.print(" Y:");
      Serial.println(farmBoyY);
    } else {
      Serial.println("[FARM JUMP] blocked.");
    }

    while (Serial.available() > 0) {
      Serial.read();
    }

    return;
  }

  Serial.read();
}


// =====================================================
// 初始化 FarmMode
// =====================================================
static void FarmModeInit() {
  if (!ModefirstRun) return;

  randomSeed(millis());

  farmBoyX = 96.0f;
  farmBoyY = 64.0f;

  if (!farmCanStandAt(farmBoyX, farmBoyY, FARM_BOY_W, FARM_BOY_H)) {
    int tx;
    int ty;

    farmGetActorFootTile(farmBoyX, farmBoyY, FARM_BOY_W, FARM_BOY_H, tx, ty);

    int nearX = tx;
    int nearY = ty;

    if (farmFindNearestWalkableTile(tx, ty, FARM_BOY_W, FARM_BOY_H, nearX, nearY)) {
      float sx;
      float sy;

      if (farmMakeStandPosForTile(nearX, nearY, FARM_BOY_W, FARM_BOY_H, sx, sy)) {
        farmBoyX = sx;
        farmBoyY = sy;
      }
    }
  }

  farmBoyTargetX = farmBoyX;
  farmBoyTargetY = farmBoyY;

  farmBoyDir = 4;
  farmBoyMode = FARM_BOY_IDLE;
  farmBoyAnimFrame = 0;

  farmBoyLastAnimMs = millis();
  farmBoyActionUntilMs = millis() + 1000UL;
  farmBoyMovingToWorkTarget = false;

  farmCameraX = 0;
  farmCameraY = 0;

  farmClearPath();
  initFarmAnimals();

  updateFarmCamera();
  renderFarmFullFrame();

  ModefirstRun = false;
}


// =====================================================
// 主函式
// =====================================================
void FarmMode() {
  FarmSerialControl();
  FarmModeInit();

  unsigned long nowMs = millis();

  updateFarmBoyAI(nowMs);
  updateFarmBoyAnimation(nowMs);
  updateFarmBoyMovement(nowMs);

  updateFarmAnimalAI(nowMs);
  updateFarmAnimalAnimation(nowMs);
  updateFarmAnimalMovement();

  updateFarmCamera();

  renderFarmFullFrame();

  wait_with_display(40);
}
