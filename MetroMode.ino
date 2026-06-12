#include "Metro.h"

// =====================================================
// Metro Mode DEMO
// 捷運進站效果
// =====================================================

// =====================================================
// 螢幕大小
// =====================================================
static const int METRO_SCR_W = 64;
static const int METRO_SCR_H = 64;

// =====================================================
// 透明色
// =====================================================
static const uint16_t METRO_TRANSPARENT = 0x001f;

// =====================================================
// 圖資尺寸
// =====================================================
static const int METRO_BG_W = 8;
static const int METRO_BG_H = 64;

static const int METRO_STATION_Y = 6;

struct MetroStationDef {
  const uint8_t* sprite;
  const uint16_t* palette;
  int w;
  int h;
};

// 寬高請填你實際圖資尺寸
static const MetroStationDef metroStationList[] = {
  { METRO_STATION_01,   METRO_STATION_01_PALETTE,   59, 25 },  // 哈瑪星
  { METRO_STATION_02,   METRO_STATION_02_PALETTE,   59, 25 },  // 鹽埕埔
  { METRO_STATION_04,   METRO_STATION_04_PALETTE,   48, 25 },  // 前金
  { METRO_STATION_C,    METRO_STATION_C_PALETTE,    61, 25 },  // 美麗島
  { METRO_STATION_06,   METRO_STATION_06_PALETTE,   72, 25 },  // 信義國小
  { METRO_STATION_07,   METRO_STATION_07_PALETTE,   72, 25 },  // 文化中心
  { METRO_STATION_08,   METRO_STATION_08_PALETTE,   59, 25 },  // 五塊厝
  { METRO_STATION_09,   METRO_STATION_09_PALETTE,   100, 25 }, // 苓雅運動園區
  { METRO_STATION_10,   METRO_STATION_10_PALETTE,   59, 25 },  // 衛武營
  { METRO_STATION_11,   METRO_STATION_11_PALETTE,   72, 25 },  // 鳳山西站
  { METRO_STATION_12,   METRO_STATION_12_PALETTE,   48, 25 },  // 鳳山
  { METRO_STATION_13,   METRO_STATION_13_PALETTE,   48, 25 },  // 大東
  { METRO_STATION_14,   METRO_STATION_14_PALETTE,   69, 25 },  // 鳳山國中
  { METRO_STATION_15,   METRO_STATION_15_PALETTE,   48, 25 },  // 大寮

  { METRO_STATION_R3,   METRO_STATION_R3_PALETTE,   48, 25 },  // 小港
  { METRO_STATION_R4,   METRO_STATION_R4_PALETTE,   72, 25 },  // 高雄機場
  { METRO_STATION_R4A,  METRO_STATION_R4A_PALETTE,  48, 25 },  // 草衙
  { METRO_STATION_R5,   METRO_STATION_R5_PALETTE,   72, 25 },  // 前鎮高中
  { METRO_STATION_R6,   METRO_STATION_R6_PALETTE,   48, 25 },  // 凱旋
  { METRO_STATION_R7,   METRO_STATION_R7_PALETTE,   48, 25 },  // 獅甲
  { METRO_STATION_R8,   METRO_STATION_R8_PALETTE,   72, 25 },  // 三多商圈
  { METRO_STATION_R9,   METRO_STATION_R9_PALETTE,   72, 25 },  // 中央公園

  { METRO_STATION_R11,  METRO_STATION_R11_PALETTE,  72, 25 },  // 高雄車站
  { METRO_STATION_R12,  METRO_STATION_R12_PALETTE,  48, 25 },  // 後驛
  { METRO_STATION_R13,  METRO_STATION_R13_PALETTE,  59, 25 },  // 凹子底
  { METRO_STATION_R14,  METRO_STATION_R14_PALETTE,  48, 25 },  // 巨蛋
  { METRO_STATION_R15,  METRO_STATION_R15_PALETTE,  72, 25 },  // 生態園區
  { METRO_STATION_R16,  METRO_STATION_R16_PALETTE,  48, 25 },  // 左營
  { METRO_STATION_R17,  METRO_STATION_R17_PALETTE,  48, 25 },  // 世運
  { METRO_STATION_R18,  METRO_STATION_R18_PALETTE,  72, 25 },  // 油廠國小
  { METRO_STATION_R19,  METRO_STATION_R19_PALETTE,  100, 25 }, // 楠梓科技園區
  { METRO_STATION_R20,  METRO_STATION_R20_PALETTE,  48, 25 },  // 後勁
  { METRO_STATION_R21,  METRO_STATION_R21_PALETTE,  72, 25 },  // 都會公園
  { METRO_STATION_R22,  METRO_STATION_R22_PALETTE,  48, 25 },  // 青埔
  { METRO_STATION_R22A, METRO_STATION_R22A_PALETTE, 72, 25 },  // 橋頭糖廠
  { METRO_STATION_R23,  METRO_STATION_R23_PALETTE,  72, 25 },  // 橋頭車站
  { METRO_STATION_R24,  METRO_STATION_R24_PALETTE,  72, 25 },  // 岡山高醫
  { METRO_STATION_RK1,  METRO_STATION_RK1_PALETTE,  72, 25 },  // 岡山車站
};

// =====================================================
// 高雄捷運路線索引表
// 陣列第一站一定是美麗島
// =====================================================

// 橘線往西：美麗島 -> 前金 -> 鹽埕埔 -> 哈瑪星
static const uint8_t metroRouteWest[] = {
  3, 2, 1, 0
};

// 橘線往東：美麗島 -> 信義國小 -> 文化中心 -> ... -> 大寮
static const uint8_t metroRouteEast[] = {
  3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

// 紅線往南：美麗島 -> 中央公園 -> 三多商圈 -> ... -> 小港
static const uint8_t metroRouteSouth[] = {
  3, 21, 20, 19, 18, 17, 16, 15, 14
};

// 紅線往北：美麗島 -> 高雄車站/後驛方向 -> ... -> 岡山車站
static const uint8_t metroRouteNorth[] = {
  3, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37
};


static const uint8_t METRO_STATION_COUNT =
  sizeof(metroStationList) / sizeof(metroStationList[0]);

// 預設站：美麗島
static const uint8_t METRO_CENTER_STATION_INDEX = 3;

static uint8_t metroCurrentStationIndex = METRO_CENTER_STATION_INDEX;

// 路線方向
static const uint8_t METRO_ROUTE_WEST  = 0;
static const uint8_t METRO_ROUTE_EAST  = 1;
static const uint8_t METRO_ROUTE_SOUTH = 2;
static const uint8_t METRO_ROUTE_NORTH = 3;

static uint8_t metroCurrentRoute = METRO_ROUTE_EAST;
static uint8_t metroLastRoute = 255;

// 目前在路線陣列的位置
static int metroRoutePos = 0;

// 1 = 從美麗島往終點，-1 = 從終點返回美麗島
static int metroRouteDir = 1;

static const int METRO_TRAIN_W = 126;
static const int METRO_TRAIN_H = 41;
static const int METRO_TRAIN_Y = 5;




// =====================================================
// 可調參數
// =====================================================

// 停靠站、開關門、乘客上下車等等待時間
static const unsigned long METRO_PARAM_A_MS = 3000UL;

// 站牌停留顯示時間
static const unsigned long METRO_PARAM_B_MS = 5000UL;

// 主迴圈刷新速度 (越小越流暢但耗CPU)
static const unsigned long METRO_FRAME_DELAY_MS = 30UL;

// =====================================================
// 背景移動參數
// =====================================================

// 背景正常移動速度
// 數值越小速度越快
static const unsigned long METRO_BG_FAST_INTERVAL_MS = 45UL;

// 列車進站減速時
// 每隔多久增加一次移動間隔
static const unsigned long METRO_BG_SLOW_STEP_MS = 180UL;

// 每次減速增加多少 Interval
// 數值越大煞車感越明顯
static const unsigned long METRO_BG_SLOW_ADD_MS = 22UL;

// 超過此 Interval 視為背景完全停止
static const unsigned long METRO_BG_STOP_INTERVAL_MS = 520UL;

// =====================================================
// 站牌動畫參數
// =====================================================

// 站牌滑入滑出速度
// 數值越小越快
static const unsigned long METRO_STATION_MOVE_INTERVAL_MS = 30UL;

// =====================================================
// 列車動畫參數
// =====================================================

// 列車進站移動速度
// 數值越小越快
static const unsigned long METRO_TRAIN_MOVE_INTERVAL_MS = 30UL;

// 第一節車廂車頭到達此 X 座標後
// 開始讓背景減速停止
static const int METRO_TRAIN_FRONT_TRIGGER_X = 32;

// 第二節車廂車頭停靠位置
// 車頭到達此位置後列車停止
static const int METRO_TRAIN_FRONT_STOP_X = 93;


// 離站前鏡頭移動速度
static const unsigned long METRO_TRAIN_CAMERA_MOVE_INTERVAL_MS = 30UL;

// 離站前鏡頭要移到第一節車廂車頭位置
static const int METRO_TRAIN_CAMERA_TARGET_FRONT_X = 32;


// 列車行駛多久後開始退場
static const unsigned long METRO_TRAIN_RUN_BEFORE_EXIT_MS = 20000UL;

// 列車往左退場速度
static const unsigned long METRO_TRAIN_EXIT_INTERVAL_MS = 25UL;

// =====================================================
// 離站背景加速參數
// 數字越小越快
// =====================================================
static const unsigned long METRO_LEAVE_TOTAL_MS = 30000UL;
static const unsigned long METRO_BG_LEAVE_ACCEL_STEP_MS = 150UL;
static const unsigned long METRO_BG_LEAVE_ACCEL_SUB_MS = 16UL;

// =====================================================
// 車門動畫圖資尺寸 / 座標
// =====================================================
static const int METRO_DOOR_IN_W = 21;
static const int METRO_DOOR_IN_H = 32;
static const int METRO_DOOR_IN_LEFT_X = 12;
static const int METRO_DOOR_IN_RIGHT_X = 32;
static const int METRO_DOOR_IN_Y = 14;

static const int METRO_DOOR_W = 22;
static const int METRO_DOOR_H = 34;
static const int METRO_DOOR_LEFT_X = 11;
static const int METRO_DOOR_RIGHT_X = 32;
static const int METRO_DOOR_Y = 12;

static const unsigned long METRO_DOOR_FRAME_INTERVAL_MS = 35UL;
static const uint8_t METRO_DOOR_MAX_STEP = METRO_DOOR_W;


// =====================================================
// 行人動畫參數
// =====================================================

// 行人圖資總數量
// METRO_P1 ~ METRO_P7
static const uint8_t METRO_PED_SPRITE_COUNT = 10;

// 每個人物圖資包含的動畫 Frame 數量
// frame0、frame1、frame2、frame3
static const uint8_t METRO_PED_FRAME_COUNT = 4;

// 行人移動方向定義
// 出站(往下走)
static const uint8_t METRO_PED_MOVE_DOWN = 0;

// 進站(往上走)
static const uint8_t METRO_PED_MOVE_UP = 1;

// 畫面同時允許存在的最大行人數
// 目前設定最多 3 組(1P+2P)
static const uint8_t METRO_PED_MAX_ACTIVE = 8;

// =====================================================
// 車門生成位置
// =====================================================

// 左側車門(P1)生成 X 範圍
static const int METRO_PED_1P_X_MIN = 14;
static const int METRO_PED_1P_X_MAX = 16;

// 右側車門(P2)生成 X 範圍
static const int METRO_PED_2P_X_MIN = 34;
static const int METRO_PED_2P_X_MAX = 36;

// 行人從車廂內出現的 Y 座標範圍
// 每次生成會在此範圍內隨機取值
static const int METRO_PED_START_Y_MIN = 22;
static const int METRO_PED_START_Y_MAX = 23;

// =====================================================
// 行人移動路徑控制
// =====================================================

// 當目前這組行人超過此 Y 座標
// 允許生成下一組出站行人
static const int METRO_PED_EXIT_NEXT_GROUP_Y = 33;

// 出站完成判定位置
// 超過此位置後視為離開畫面
static const int METRO_PED_EXIT_DONE_Y = 86;

// 進站起始位置
// 從畫面外下方開始往上走
static const int METRO_PED_ENTER_START_Y = 64;


// 進站時，當最新一組走到此 Y 座標以上，就生成下一組
static const int METRO_PED_ENTER_NEXT_GROUP_Y = 45;

// =====================================================
// 行人動畫速度
// =====================================================

// 走路動畫 Frame 切換速度
// 數值越小動畫越快
static const unsigned long METRO_PED_FRAME_INTERVAL_MS = 120UL;

// 行人實際移動速度
// 數值越小走路越快
static const unsigned long METRO_PED_MOVE_INTERVAL_MS = 120UL;

// 2P 比 1P 晚出發時間
// 製造先後出站效果
static const unsigned long METRO_PED_2P_DELAY_MS = 2400UL;

// =====================================================
// 行人群組設定
// =====================================================

// 每次列車停靠最少出站組數
static const uint8_t METRO_PED_EXIT_MIN_GROUPS = 3;

// 每次列車停靠最多出站組數
static const uint8_t METRO_PED_EXIT_MAX_GROUPS = 4;

struct MetroPedSpriteDef {
  const uint8_t* sprite;
  const uint16_t* palette;
  uint8_t sheetW;
  uint8_t sheetH;
  uint8_t frameW;
  uint8_t frameH;
};

static const MetroPedSpriteDef metroPedSprites[METRO_PED_SPRITE_COUNT] = {
  { METRO_P1,  METRO_P1_PALETTE,  56, 20, 14, 20 },
  { METRO_P2,  METRO_P2_PALETTE,  56, 19, 14, 19 },
  { METRO_P3,  METRO_P3_PALETTE,  56, 20, 14, 20 },
  { METRO_P4,  METRO_P4_PALETTE,  64, 19, 16, 19 },
  { METRO_P5,  METRO_P5_PALETTE,  56, 21, 14, 21 },
  { METRO_P6,  METRO_P6_PALETTE,  64, 20, 16, 20 },
  { METRO_P7,  METRO_P7_PALETTE,  64, 19, 16, 19 },
  { METRO_P8,  METRO_P8_PALETTE,  56, 21, 14, 21 },
  { METRO_P9,  METRO_P9_PALETTE,  56, 20, 14, 20 },
  { METRO_P10, METRO_P10_PALETTE, 56, 20, 14, 20 }
};

struct MetroPed {
  bool active;
  bool started;
  bool arrived;
  uint8_t spriteIndex;
  uint8_t walkDir;
  uint8_t animStep;
  int x;
  int y;
  int targetX;
  int targetY;
  unsigned long startMs;
  unsigned long lastFrameMs;
  unsigned long lastMoveMs;
};

static MetroPed metroPeds[METRO_PED_MAX_ACTIVE];

static uint8_t metroPedTotalGroups = 0;
static uint8_t metroPedSpawnedGroups = 0;
static bool metroPedGroupCanSpawnNext = true;
static unsigned long metroPedStateStartMs = 0;

// =====================================================
// 狀態機
// =====================================================
static const uint8_t METRO_STATE_BG_INTRO = 0;
static const uint8_t METRO_STATE_STATION_IN = 1;
static const uint8_t METRO_STATE_STATION_HOLD = 2;
static const uint8_t METRO_STATE_STATION_OUT = 3;
static const uint8_t METRO_STATE_BG_AFTER_STATION = 4;
static const uint8_t METRO_STATE_TRAIN_IN = 5;
static const uint8_t METRO_STATE_TRAIN_HOLD = 6;
static const uint8_t METRO_STATE_CLOCK = 7;
static const uint8_t METRO_STATE_DOOR_OPEN = 8;
static const uint8_t METRO_STATE_PED_EXIT = 9;
static const uint8_t METRO_STATE_WAIT_BEFORE_ENTER = 10;
static const uint8_t METRO_STATE_PED_ENTER = 11;
static const uint8_t METRO_STATE_WAIT_BEFORE_CLOSE = 12;
static const uint8_t METRO_STATE_DOOR_CLOSE = 13;
static const uint8_t METRO_STATE_WAIT_BEFORE_LEAVE = 14;
static const uint8_t METRO_STATE_TRAIN_CAMERA_MOVE = 15;
static const uint8_t METRO_STATE_TRAIN_LEAVE = 16;
static const uint8_t METRO_STATE_TRAIN_EXIT = 17;

static uint8_t metroState = METRO_STATE_BG_INTRO;
static unsigned long metroStateStartMs = 0;

// =====================================================
// 背景動畫控制
// =====================================================
static int metroBgX = 0;
static unsigned long metroBgLastMoveMs = 0;
static unsigned long metroBgMoveIntervalMs = METRO_BG_FAST_INTERVAL_MS;
static unsigned long metroBgSlowLastMs = 0;
static bool metroBgSlowing = false;
static bool metroBgStopped = false;
static bool metroBgReverse = false;
static unsigned long metroBgLeaveAccelLastMs = 0;

// =====================================================
// 站牌控制
// =====================================================
static int metroStationX = METRO_SCR_W;
static unsigned long metroStationLastMoveMs = 0;

// =====================================================
// 列車控制
// =====================================================
static int metroTrainX = -METRO_TRAIN_W;
static bool metroSecondTrainVisible = false;
static unsigned long metroTrainLastMoveMs = 0;

// =====================================================
// 車門動畫控制
// =====================================================
static uint8_t metroDoorStep = 0;
static unsigned long metroDoorLastFrameMs = 0;

// =====================================================
// 繪製 sprite
// =====================================================
static void drawMetroSprite(
  int x,
  int y,
  int w,
  int h,
  const uint16_t* sprite
) {
  for (int j = 0; j < h; j++) {
    int dy = y + j;
    if (dy < 0 || dy >= METRO_SCR_H) continue;

    for (int i = 0; i < w; i++) {
      int dx = x + i;
      if (dx < 0 || dx >= METRO_SCR_W) continue;

      uint32_t pos = (uint32_t)j * (uint32_t)w + (uint32_t)i;
      uint16_t color = pgm_read_word(&(sprite[pos]));

      if (color == METRO_TRANSPARENT) continue;

      display.drawPixel(dx, dy, color);
    }
  }
}

static void drawMetroSpritePal(
  int x,
  int y,
  int w,
  int h,
  const uint8_t* sprite,
  const uint16_t* palette
) {
  for (int j = 0; j < h; j++) {
    int dy = y + j;
    if (dy < 0 || dy >= METRO_SCR_H) continue;

    for (int i = 0; i < w; i++) {
      int dx = x + i;
      if (dx < 0 || dx >= METRO_SCR_W) continue;

      uint32_t pos = (uint32_t)j * (uint32_t)w + (uint32_t)i;

      uint8_t idx = pgm_read_byte(&(sprite[pos]));
      if (idx == 0) continue;   // index 0 當透明色

      uint16_t color = pgm_read_word(&(palette[idx]));
      display.drawPixel(dx, dy, color);
    }
  }
}


// =====================================================
// 繪製 sprite，支援水平翻轉、透明色與裁切
// =====================================================
static void drawMetroSpriteEx(
  int x,
  int y,
  int srcW,
  int srcH,
  const uint16_t* sprite,
  int srcX,
  int drawW,
  bool flipH
) {
  if (drawW <= 0) return;

  if (srcX < 0) {
    x -= srcX;
    drawW += srcX;
    srcX = 0;
  }

  if (srcX + drawW > srcW) {
    drawW = srcW - srcX;
  }

  if (drawW <= 0) return;

  for (int j = 0; j < srcH; j++) {
    int dy = y + j;
    if (dy < 0 || dy >= METRO_SCR_H) continue;

    for (int i = 0; i < drawW; i++) {
      int dx = x + i;
      if (dx < 0 || dx >= METRO_SCR_W) continue;

      int sx = flipH ? (srcX + drawW - 1 - i) : (srcX + i);
      uint32_t pos = (uint32_t)j * (uint32_t)srcW + (uint32_t)sx;
      uint16_t color = pgm_read_word(&(sprite[pos]));

      if (color == METRO_TRANSPARENT) continue;

      display.drawPixel(dx, dy, color);
    }
  }
}


static void drawMetroSpriteExPal(
  int x,
  int y,
  int srcW,
  int srcH,
  const uint8_t* sprite,
  const uint16_t* palette,
  int srcX,
  int drawW,
  bool flipH
) {
  if (drawW <= 0) return;

  if (srcX < 0) {
    x -= srcX;
    drawW += srcX;
    srcX = 0;
  }

  if (srcX + drawW > srcW) {
    drawW = srcW - srcX;
  }

  if (drawW <= 0) return;

  for (int j = 0; j < srcH; j++) {
    int dy = y + j;
    if (dy < 0 || dy >= METRO_SCR_H) continue;

    for (int i = 0; i < drawW; i++) {
      int dx = x + i;
      if (dx < 0 || dx >= METRO_SCR_W) continue;

      int sx = flipH ? (srcX + drawW - 1 - i) : (srcX + i);
      uint32_t pos = (uint32_t)j * (uint32_t)srcW + (uint32_t)sx;

      uint8_t idx = pgm_read_byte(&(sprite[pos]));
      if (idx == 0) continue;

      uint16_t color = pgm_read_word(&(palette[idx]));
      display.drawPixel(dx, dy, color);
    }
  }
}

// =====================================================
// 繪製行人單一 frame
// =====================================================
static void drawMetroPed(uint8_t pedIndex) {
  MetroPed& ped = metroPeds[pedIndex];
  if (!ped.active || !ped.started) return;

  const MetroPedSpriteDef& def = metroPedSprites[ped.spriteIndex];

  uint8_t frame = 0;
  bool flipH = false;

  if (ped.walkDir == METRO_PED_MOVE_DOWN) {
    if (ped.animStep == 0) {
      frame = 0;
      flipH = false;
    } else if (ped.animStep == 1) {
      frame = 1;
      flipH = false;
    } else {
      frame = 1;
      flipH = true;
    }
  } else {
    if (ped.animStep == 0) {
      frame = 2;
      flipH = false;
    } else if (ped.animStep == 1) {
      frame = 3;
      flipH = false;
    } else {
      frame = 3;
      flipH = true;
    }
  }

drawMetroSpriteExPal(
  ped.x,
  ped.y,
  def.sheetW,
  def.sheetH,
  def.sprite,
  def.palette,
  frame * def.frameW,
  def.frameW,
  flipH
);
}

// =====================================================
// 繪製所有行人
// =====================================================
static void drawMetroPeds() {
  bool drawn[METRO_PED_MAX_ACTIVE];

  for (uint8_t i = 0; i < METRO_PED_MAX_ACTIVE; i++) {
    drawn[i] = false;
  }

  for (uint8_t pass = 0; pass < METRO_PED_MAX_ACTIVE; pass++) {
    int bestIndex = -1;
    int bestY = 9999;

    for (uint8_t i = 0; i < METRO_PED_MAX_ACTIVE; i++) {
      if (drawn[i]) continue;
      if (!metroPeds[i].active || !metroPeds[i].started) continue;

      if (metroPeds[i].y < bestY) {
        bestY = metroPeds[i].y;
        bestIndex = i;
      }
    }

    if (bestIndex < 0) break;

    drawMetroPed(bestIndex);
    drawn[bestIndex] = true;
  }
}

// =====================================================
// 清除行人
// =====================================================
static void clearMetroPeds() {
  for (uint8_t i = 0; i < METRO_PED_MAX_ACTIVE; i++) {
    metroPeds[i].active = false;
    metroPeds[i].started = false;
    metroPeds[i].arrived = false;
  }
}

// =====================================================
// 建立單一行人
// =====================================================
static void setupMetroPed(
  uint8_t index,
  uint8_t dir,
  int startX,
  int startY,
  int targetX,
  int targetY,
  unsigned long startDelayMs,
  unsigned long nowMs
) {
  metroPeds[index].active = true;
  metroPeds[index].started = false;
  metroPeds[index].arrived = false;
  metroPeds[index].spriteIndex = random(0, METRO_PED_SPRITE_COUNT);
  metroPeds[index].walkDir = dir;
  metroPeds[index].animStep = random(0, 3);
  metroPeds[index].x = startX;
  metroPeds[index].y = startY;
  metroPeds[index].targetX = targetX;
  metroPeds[index].targetY = targetY;
  metroPeds[index].startMs = nowMs + startDelayMs;
  metroPeds[index].lastFrameMs = nowMs;
  metroPeds[index].lastMoveMs = nowMs;
}

// =====================================================
// 生成一組出站行人
// =====================================================
static void spawnMetroExitGroup(unsigned long nowMs) {
  uint8_t baseIndex = metroPedSpawnedGroups * 2;
  if (baseIndex + 1 >= METRO_PED_MAX_ACTIVE) return;

  int p1x = random(METRO_PED_1P_X_MIN, METRO_PED_1P_X_MAX + 1);
  int p2x = random(METRO_PED_2P_X_MIN, METRO_PED_2P_X_MAX + 1);
  int p1y = random(METRO_PED_START_Y_MIN, METRO_PED_START_Y_MAX + 1);
  int p2y = random(METRO_PED_START_Y_MIN, METRO_PED_START_Y_MAX + 1);

  setupMetroPed(
    baseIndex,
    METRO_PED_MOVE_DOWN,
    p1x,
    p1y,
    p1x,
    METRO_PED_EXIT_DONE_Y,
    0,
    nowMs
  );

  setupMetroPed(
    baseIndex + 1,
    METRO_PED_MOVE_DOWN,
    p2x,
    p2y,
    p2x,
    METRO_PED_EXIT_DONE_Y,
    METRO_PED_2P_DELAY_MS,
    nowMs
  );

  metroPedSpawnedGroups++;
  metroPedGroupCanSpawnNext = false;
}

// =====================================================
// 生成一組進站行人
// 進站最後會停在 1P / 2P 門口位置
// =====================================================
static void spawnMetroEnterGroup(unsigned long nowMs) {
  uint8_t baseIndex = metroPedSpawnedGroups * 2;
  if (baseIndex + 1 >= METRO_PED_MAX_ACTIVE) return;

  int p1x = random(METRO_PED_1P_X_MIN, METRO_PED_1P_X_MAX + 1);
  int p2x = random(METRO_PED_2P_X_MIN, METRO_PED_2P_X_MAX + 1);

  int p1TargetY = random(METRO_PED_START_Y_MIN, METRO_PED_START_Y_MAX + 1);
  int p2TargetY = random(METRO_PED_START_Y_MIN, METRO_PED_START_Y_MAX + 1);

  setupMetroPed(
    baseIndex,
    METRO_PED_MOVE_UP,
    p1x,
    METRO_PED_ENTER_START_Y,
    p1x,
    p1TargetY,
    0,
    nowMs
  );

  setupMetroPed(
    baseIndex + 1,
    METRO_PED_MOVE_UP,
    p2x,
    METRO_PED_ENTER_START_Y,
    p2x,
    p2TargetY,
    METRO_PED_2P_DELAY_MS,
    nowMs
  );

  metroPedSpawnedGroups++;
  metroPedGroupCanSpawnNext = false;
}

// =====================================================
// 開始出站
// =====================================================
static void startMetroPedExit(unsigned long nowMs) {
  clearMetroPeds();

  metroPedTotalGroups = random(METRO_PED_EXIT_MIN_GROUPS, METRO_PED_EXIT_MAX_GROUPS + 1);
  metroPedSpawnedGroups = 0;
  metroPedGroupCanSpawnNext = true;
  metroPedStateStartMs = nowMs;

  spawnMetroExitGroup(nowMs);
}

// =====================================================
// 開始進站
// =====================================================
static void startMetroPedEnter(unsigned long nowMs) {
  clearMetroPeds();

  metroPedTotalGroups = metroPedTotalGroups;
  if (metroPedTotalGroups < METRO_PED_EXIT_MIN_GROUPS) {
    metroPedTotalGroups = random(METRO_PED_EXIT_MIN_GROUPS, METRO_PED_EXIT_MAX_GROUPS + 1);
  }

  metroPedSpawnedGroups = 0;
  metroPedGroupCanSpawnNext = true;
  metroPedStateStartMs = nowMs;

  spawnMetroEnterGroup(nowMs);
}

// =====================================================
// 更新行人動畫 frame
// =====================================================
static void updateMetroPedFrame(uint8_t pedIndex, unsigned long nowMs) {
  MetroPed& ped = metroPeds[pedIndex];
  if (!ped.active || !ped.started || ped.arrived) return;

  if (nowMs - ped.lastFrameMs >= METRO_PED_FRAME_INTERVAL_MS) {
    ped.lastFrameMs = nowMs;
    ped.animStep++;
    if (ped.animStep >= 3) ped.animStep = 0;
  }
}

// =====================================================
// 更新行人出站移動
// =====================================================
static bool metroAllPedsGone() {
  for (uint8_t i = 0; i < METRO_PED_MAX_ACTIVE; i++) {
    if (metroPeds[i].active) return false;
  }

  return true;
}

static bool metroAllPedsArrived() {
  for (uint8_t i = 0; i < METRO_PED_MAX_ACTIVE; i++) {
    if (!metroPeds[i].active) continue;
    if (!metroPeds[i].arrived) return false;
  }

  return true;
}


static void updateMetroPedExitMove(uint8_t pedIndex, unsigned long nowMs) {
  MetroPed& ped = metroPeds[pedIndex];
  if (!ped.active || ped.arrived) return;

  if (!ped.started) {
    if (nowMs >= ped.startMs) {
      ped.started = true;
      ped.lastMoveMs = nowMs;
      ped.lastFrameMs = nowMs;
    } else {
      return;
    }
  }

  if (nowMs - ped.lastMoveMs < METRO_PED_MOVE_INTERVAL_MS) return;
  ped.lastMoveMs = nowMs;

  ped.y++;

  if (ped.y > METRO_PED_EXIT_NEXT_GROUP_Y) {
    int r = random(0, 100);
    if (r < 18 && ped.x > 0) {
      ped.x--;
    } else if (r >= 82 && ped.x < METRO_SCR_W - 8) {
      ped.x++;
    }
  }

  if (ped.y > METRO_PED_EXIT_DONE_Y) {
    ped.active = false;
    ped.started = false;
    ped.arrived = true;
  }
}

// =====================================================
// 更新行人進站移動
// =====================================================
static void updateMetroPedEnterMove(uint8_t pedIndex, unsigned long nowMs) {
  MetroPed& ped = metroPeds[pedIndex];
  if (!ped.active || ped.arrived) return;

  if (!ped.started) {
    if (nowMs >= ped.startMs) {
      ped.started = true;
      ped.lastMoveMs = nowMs;
      ped.lastFrameMs = nowMs;
    } else {
      return;
    }
  }

  if (nowMs - ped.lastMoveMs < METRO_PED_MOVE_INTERVAL_MS) return;
  ped.lastMoveMs = nowMs;

  if (ped.y > ped.targetY) {
    ped.y--;
  }

  if (ped.x < ped.targetX) ped.x++;
  if (ped.x > ped.targetX) ped.x--;

if (ped.y <= ped.targetY) {
  ped.y = ped.targetY;
  ped.x = ped.targetX;
  ped.arrived = true;
  ped.active = false;
  ped.started = false;
}
}

// =====================================================
// 判斷目前行人是否可生成下一組
// 出站：1P / 2P 都超過 Y=23 就生成下一組
// 進站：目前組到達門口後，若還有下一組，就清掉目前組並生成下一組
// =====================================================
static bool metroCurrentPedsPassedExitSpawnLine() {
  if (metroPedSpawnedGroups == 0) return false;

  uint8_t baseIndex = (metroPedSpawnedGroups - 1) * 2;
  if (baseIndex + 1 >= METRO_PED_MAX_ACTIVE) return false;

  for (uint8_t i = baseIndex; i <= baseIndex + 1; i++) {
    if (!metroPeds[i].active || !metroPeds[i].started) return false;

    if (metroPeds[i].y <= METRO_PED_EXIT_NEXT_GROUP_Y) {
      return false;
    }
  }

  return true;
}

//進站生成判斷
static bool metroCurrentPedsPassedEnterSpawnLine() {
  if (metroPedSpawnedGroups == 0) return false;

  uint8_t baseIndex = (metroPedSpawnedGroups - 1) * 2;
  if (baseIndex + 1 >= METRO_PED_MAX_ACTIVE) return false;

  for (uint8_t i = baseIndex; i <= baseIndex + 1; i++) {
    if (!metroPeds[i].active || !metroPeds[i].started) return false;

    if (metroPeds[i].y > METRO_PED_ENTER_NEXT_GROUP_Y) {
      return false;
    }
  }

  return true;
}

// =====================================================
// 更新出站
// =====================================================
static bool updateMetroPedExit(unsigned long nowMs) {
  for (uint8_t i = 0; i < METRO_PED_MAX_ACTIVE; i++) {
       updateMetroPedFrame(i, nowMs);
       updateMetroPedExitMove(i, nowMs);
  }

  if (!metroPedGroupCanSpawnNext && metroCurrentPedsPassedExitSpawnLine()) {
    metroPedGroupCanSpawnNext = true;
  }

  if (metroPedGroupCanSpawnNext && metroPedSpawnedGroups < metroPedTotalGroups) {
    spawnMetroExitGroup(nowMs);
  }

  if (metroPedSpawnedGroups >= metroPedTotalGroups && metroAllPedsGone()) {
    return true;
  }

  return false;
}

// =====================================================
// 更新進站
// =====================================================
static bool updateMetroPedEnter(unsigned long nowMs) {
  for (uint8_t i = 0; i < METRO_PED_MAX_ACTIVE; i++) {
    updateMetroPedFrame(i, nowMs);
    updateMetroPedEnterMove(i, nowMs);
  }

  if (!metroPedGroupCanSpawnNext && metroCurrentPedsPassedEnterSpawnLine()) {
    metroPedGroupCanSpawnNext = true;
  }

  if (metroPedGroupCanSpawnNext && metroPedSpawnedGroups < metroPedTotalGroups) {
    spawnMetroEnterGroup(nowMs);
  }

  if (metroPedSpawnedGroups >= metroPedTotalGroups && metroAllPedsGone()) {
    return true;
  }

  return false;
}

// =====================================================
// 繪製車門內部
// =====================================================
static void drawMetroDoorInside() {
  drawMetroSpriteExPal(
    METRO_DOOR_IN_LEFT_X,
    METRO_DOOR_IN_Y,
    METRO_DOOR_IN_W,
    METRO_DOOR_IN_H,
    METRO_IN,
    METRO_IN_PALETTE,
    0,
    METRO_DOOR_IN_W,
    false
  );

  drawMetroSpriteExPal(
    METRO_DOOR_IN_RIGHT_X,
    METRO_DOOR_IN_Y,
    METRO_DOOR_IN_W,
    METRO_DOOR_IN_H,
    METRO_IN,
    METRO_IN_PALETTE,
    0,
    METRO_DOOR_IN_W,
    true
  );
}

// =====================================================
// 依開門步數繪製車門
// =====================================================
static void drawMetroDoorsAtStep(uint8_t step) {
  if (step > METRO_DOOR_MAX_STEP) step = METRO_DOOR_MAX_STEP;

  int visibleW = METRO_DOOR_W - step;
  if (visibleW <= 0) return;

  drawMetroSpriteExPal(
    METRO_DOOR_LEFT_X,
    METRO_DOOR_Y,
    METRO_DOOR_W,
    METRO_DOOR_H,
    METRO_DOOR,
    METRO_DOOR_PALETTE,
    step,
    visibleW,
    false
  );

  drawMetroSpriteExPal(
    METRO_DOOR_RIGHT_X + step,
    METRO_DOOR_Y,
    METRO_DOOR_W,
    METRO_DOOR_H,
    METRO_DOOR,
    METRO_DOOR_PALETTE,
    0,
    visibleW,
    false
  );
}
// =====================================================
// 車門開啟動畫
// =====================================================
static bool METRO_DOOR_OPEN() {
  unsigned long nowMs = millis();

  drawMetroDoorInside();
  drawMetroDoorsAtStep(metroDoorStep);

  if (metroDoorStep < METRO_DOOR_MAX_STEP &&
      nowMs - metroDoorLastFrameMs >= METRO_DOOR_FRAME_INTERVAL_MS) {
    metroDoorLastFrameMs = nowMs;
    metroDoorStep++;
  }

  return metroDoorStep >= METRO_DOOR_MAX_STEP;
}

// =====================================================
// 車門關閉動畫
// =====================================================
static bool METRO_DOOR_CLOSE() {
  unsigned long nowMs = millis();

  drawMetroDoorInside();
  drawMetroDoorsAtStep(metroDoorStep);

  if (metroDoorStep > 0 &&
      nowMs - metroDoorLastFrameMs >= METRO_DOOR_FRAME_INTERVAL_MS) {
    metroDoorLastFrameMs = nowMs;
    metroDoorStep--;
  }

  return metroDoorStep == 0;
}

// =====================================================
// 取得列車車頭 X
// =====================================================
static int getMetroTrainFrontX(int trainX) {
  return trainX + METRO_TRAIN_W - 1;
}

// =====================================================
// 背景開始緩慢停止
// =====================================================
static void startMetroBgSlowStop(unsigned long nowMs) {
  if (metroBgSlowing) return;

  metroBgSlowing = true;
  metroBgSlowLastMs = nowMs;
}


static const uint8_t* getMetroRouteArray(uint8_t route) {
  switch (route) {
    case METRO_ROUTE_WEST:
      return metroRouteWest;

    case METRO_ROUTE_EAST:
      return metroRouteEast;

    case METRO_ROUTE_SOUTH:
      return metroRouteSouth;

    case METRO_ROUTE_NORTH:
      return metroRouteNorth;
  }

  return metroRouteEast;
}

static uint8_t getMetroRouteLength(uint8_t route) {
  switch (route) {
    case METRO_ROUTE_WEST:
      return sizeof(metroRouteWest) / sizeof(metroRouteWest[0]);

    case METRO_ROUTE_EAST:
      return sizeof(metroRouteEast) / sizeof(metroRouteEast[0]);

    case METRO_ROUTE_SOUTH:
      return sizeof(metroRouteSouth) / sizeof(metroRouteSouth[0]);

    case METRO_ROUTE_NORTH:
      return sizeof(metroRouteNorth) / sizeof(metroRouteNorth[0]);
  }

  return sizeof(metroRouteEast) / sizeof(metroRouteEast[0]);
}

static void metroPickNewRoute() {
  uint8_t newRoute;

  do {
    newRoute = random(0, 4);
  } while (newRoute == metroLastRoute);

  metroCurrentRoute = newRoute;
  metroLastRoute = newRoute;
  metroRoutePos = 0;
  metroRouteDir = 1;

  const uint8_t* routeArray = getMetroRouteArray(metroCurrentRoute);
  metroCurrentStationIndex = routeArray[metroRoutePos];
}

static void metroNextStation() {
  const uint8_t* routeArray = getMetroRouteArray(metroCurrentRoute);
  uint8_t routeLen = getMetroRouteLength(metroCurrentRoute);

  metroRoutePos += metroRouteDir;

  // 到終點後，開始返回美麗島
  if (metroRoutePos >= routeLen) {
    metroRoutePos = routeLen - 2;
    metroRouteDir = -1;
  }

  // 返回美麗島後，重新抽方向
  if (metroRoutePos <= 0 && metroRouteDir < 0) {
    metroPickNewRoute();
    return;
  }

  metroCurrentStationIndex = routeArray[metroRoutePos];
}

// =====================================================
// 切換狀態
// =====================================================
static void setMetroState(uint8_t newState, unsigned long nowMs) {
  metroState = newState;
  metroStateStartMs = nowMs;

  if (newState == METRO_STATE_STATION_IN) {
    metroStationX = METRO_SCR_W;
    metroStationLastMoveMs = nowMs;
  }

  if (newState == METRO_STATE_TRAIN_IN) {
    metroTrainX = -METRO_TRAIN_W;
    metroSecondTrainVisible = false;
    metroTrainLastMoveMs = nowMs;

    metroBgReverse = false;
    metroBgSlowing = false;
    metroBgStopped = false;
    metroBgMoveIntervalMs = METRO_BG_FAST_INTERVAL_MS;
    metroBgSlowLastMs = nowMs;
  }

  if (newState == METRO_STATE_CLOCK) {
    metroBgStopped = true;
    metroDoorStep = 0;
    metroDoorLastFrameMs = nowMs;
    clearMetroPeds();
  }

  if (newState == METRO_STATE_DOOR_OPEN) {
    metroDoorStep = 0;
    metroDoorLastFrameMs = nowMs;
  }

  if (newState == METRO_STATE_PED_EXIT) {
    startMetroPedExit(nowMs);
  }

  if (newState == METRO_STATE_WAIT_BEFORE_ENTER) {
    clearMetroPeds();
  }

  if (newState == METRO_STATE_PED_ENTER) {
    startMetroPedEnter(nowMs);
  }

  if (newState == METRO_STATE_WAIT_BEFORE_CLOSE) {
    // 進站完成後，最後一組人保留在畫面
  }

  if (newState == METRO_STATE_DOOR_CLOSE) {
    metroDoorLastFrameMs = nowMs;
  }

if (newState == METRO_STATE_TRAIN_CAMERA_MOVE) {
  metroBgStopped = true;
  metroBgReverse = false;
  metroTrainLastMoveMs = nowMs;
}


if (newState == METRO_STATE_TRAIN_LEAVE) {
  metroBgReverse = false;
  metroBgStopped = false;
  metroBgSlowing = false;
  metroBgMoveIntervalMs = METRO_BG_STOP_INTERVAL_MS;
  metroBgLeaveAccelLastMs = nowMs;
}
 
}

// =====================================================
// 更新背景動畫
// =====================================================
static void updateMetroBackground(unsigned long nowMs) {
  if (metroBgStopped) return;

  if (metroBgSlowing) {
    if (nowMs - metroBgSlowLastMs >= METRO_BG_SLOW_STEP_MS) {
      metroBgSlowLastMs = nowMs;

      if (metroBgMoveIntervalMs < METRO_BG_STOP_INTERVAL_MS) {
        metroBgMoveIntervalMs += METRO_BG_SLOW_ADD_MS;
      }

      if (metroBgMoveIntervalMs >= METRO_BG_STOP_INTERVAL_MS) {
        metroBgStopped = true;
        return;
      }
    }
  }

  if (metroState == METRO_STATE_TRAIN_LEAVE) {
    if (nowMs - metroBgLeaveAccelLastMs >= METRO_BG_LEAVE_ACCEL_STEP_MS) {
      metroBgLeaveAccelLastMs = nowMs;

      if (metroBgMoveIntervalMs > METRO_BG_FAST_INTERVAL_MS) {
        if (metroBgMoveIntervalMs > METRO_BG_LEAVE_ACCEL_SUB_MS) {
          metroBgMoveIntervalMs -= METRO_BG_LEAVE_ACCEL_SUB_MS;
        }

        if (metroBgMoveIntervalMs < METRO_BG_FAST_INTERVAL_MS) {
          metroBgMoveIntervalMs = METRO_BG_FAST_INTERVAL_MS;
        }
      }
    }
  }

  if (nowMs - metroBgLastMoveMs >= metroBgMoveIntervalMs) {
    metroBgLastMoveMs = nowMs;

    if (metroBgReverse) {
      metroBgX++;

      if (metroBgX >= METRO_BG_W) {
        metroBgX -= METRO_BG_W;
      }
    } else {
      metroBgX--;

      if (metroBgX <= -METRO_BG_W) {
        metroBgX += METRO_BG_W;
      }
    }
  }
}

static int getMetroStationCenterX() {
  return (METRO_SCR_W - metroStationList[metroCurrentStationIndex].w) / 2;
}


// =====================================================
// 繪製重複背景
// =====================================================
static void drawMetroBackground() {
  display.fillScreen(0x0000);

  int startX = metroBgX;

  while (startX > 0) {
    startX -= METRO_BG_W;
  }

  for (int x = startX; x < METRO_SCR_W; x += METRO_BG_W) {
drawMetroSpritePal(
  x,
  0,
  METRO_BG_W,
  METRO_BG_H,
  METRO_BG,
  METRO_BG_PALETTE
);
  }
}

// =====================================================
// 更新站牌
// =====================================================
static void updateMetroStation(unsigned long nowMs) {
  switch (metroState) {
    case METRO_STATE_STATION_IN: {
      if (nowMs - metroStationLastMoveMs >= METRO_STATION_MOVE_INTERVAL_MS) {
        metroStationLastMoveMs = nowMs;

        metroStationX--;

int centerX = getMetroStationCenterX();

if (metroStationX <= centerX) {
  metroStationX = centerX;
  setMetroState(METRO_STATE_STATION_HOLD, nowMs);
}
      }
      break;
    }

    case METRO_STATE_STATION_HOLD: {
      if (nowMs - metroStateStartMs >= METRO_PARAM_B_MS) {
        setMetroState(METRO_STATE_STATION_OUT, nowMs);
      }
      break;
    }

    case METRO_STATE_STATION_OUT: {
      if (nowMs - metroStationLastMoveMs >= METRO_STATION_MOVE_INTERVAL_MS) {
        metroStationLastMoveMs = nowMs;

        metroStationX--;

if (metroStationX + metroStationList[metroCurrentStationIndex].w < 0) {
  setMetroState(METRO_STATE_BG_AFTER_STATION, nowMs);
}
      }
      break;
    }

    default:
      break;
  }
}

// =====================================================
// 更新列車進站
// =====================================================
static void updateMetroTrain(unsigned long nowMs) {
  if (metroState != METRO_STATE_TRAIN_IN) return;

  if (nowMs - metroTrainLastMoveMs < METRO_TRAIN_MOVE_INTERVAL_MS) {
    return;
  }

  metroTrainLastMoveMs = nowMs;

  metroTrainX++;

  if (getMetroTrainFrontX(metroTrainX) >= METRO_TRAIN_FRONT_TRIGGER_X) {
    startMetroBgSlowStop(nowMs);
  }

  if (metroTrainX >= 0) {
    metroSecondTrainVisible = true;
  }

  if (metroSecondTrainVisible) {
    int secondTrainX = metroTrainX - METRO_TRAIN_W;

    if (getMetroTrainFrontX(secondTrainX) >= METRO_TRAIN_FRONT_STOP_X) {
      secondTrainX = METRO_TRAIN_FRONT_STOP_X - METRO_TRAIN_W + 1;
      metroTrainX = secondTrainX + METRO_TRAIN_W;

      metroBgStopped = true;
      setMetroState(METRO_STATE_TRAIN_HOLD, nowMs);
    }
  }
}


static void updateMetroTrainCameraMove(unsigned long nowMs) {
  if (nowMs - metroTrainLastMoveMs < METRO_TRAIN_CAMERA_MOVE_INTERVAL_MS) {
    return;
  }

  metroTrainLastMoveMs = nowMs;

  int firstTrainFrontX = getMetroTrainFrontX(metroTrainX);

  if (firstTrainFrontX > METRO_TRAIN_CAMERA_TARGET_FRONT_X) {
    metroTrainX--;
  } else if (firstTrainFrontX < METRO_TRAIN_CAMERA_TARGET_FRONT_X) {
    metroTrainX++;
  } else {
    setMetroState(METRO_STATE_TRAIN_LEAVE, nowMs);
  }
}

//退場函式：
static void updateMetroTrainExit(unsigned long nowMs) {
  if (nowMs - metroTrainLastMoveMs < METRO_TRAIN_EXIT_INTERVAL_MS) {
    return;
  }

  metroTrainLastMoveMs = nowMs;

  // 車廂往左退出畫面
  metroTrainX--;
}

// =====================================================
// 更新整體狀態
// =====================================================
static void updateMetroState(unsigned long nowMs) {
  switch (metroState) {
    case METRO_STATE_BG_INTRO: {
      if (nowMs - metroStateStartMs >= METRO_PARAM_A_MS) {
        setMetroState(METRO_STATE_STATION_IN, nowMs);
      }
      break;
    }

    case METRO_STATE_STATION_IN:
    case METRO_STATE_STATION_HOLD:
    case METRO_STATE_STATION_OUT: {
      updateMetroStation(nowMs);
      break;
    }

    case METRO_STATE_BG_AFTER_STATION: {
      if (nowMs - metroStateStartMs >= METRO_PARAM_A_MS) {
        setMetroState(METRO_STATE_TRAIN_IN, nowMs);
      }
      break;
    }

    case METRO_STATE_TRAIN_IN: {
      updateMetroTrain(nowMs);
      break;
    }

    case METRO_STATE_TRAIN_HOLD: {
      if (nowMs - metroStateStartMs >= METRO_PARAM_A_MS) {
        setMetroState(METRO_STATE_CLOCK, nowMs);
      }
      break;
    }

    case METRO_STATE_CLOCK: {
      if (nowMs - metroStateStartMs >= METRO_PARAM_A_MS) {
        setMetroState(METRO_STATE_DOOR_OPEN, nowMs);
      }
      break;
    }

    case METRO_STATE_DOOR_OPEN: {
      if (metroDoorStep >= METRO_DOOR_MAX_STEP) {
        setMetroState(METRO_STATE_PED_EXIT, nowMs);
      }
      break;
    }

    case METRO_STATE_PED_EXIT: {
      if (updateMetroPedExit(nowMs)) {
        setMetroState(METRO_STATE_WAIT_BEFORE_ENTER, nowMs);
      }
      break;
    }

    case METRO_STATE_WAIT_BEFORE_ENTER: {
      if (nowMs - metroStateStartMs >= 1000) {   //等待一秒
        setMetroState(METRO_STATE_PED_ENTER, nowMs);
      }
      break;
    }

    case METRO_STATE_PED_ENTER: {
      if (updateMetroPedEnter(nowMs)) {
        setMetroState(METRO_STATE_WAIT_BEFORE_CLOSE, nowMs);
      }
      break;
    }

    case METRO_STATE_WAIT_BEFORE_CLOSE: {
      if (nowMs - metroStateStartMs >= METRO_PARAM_A_MS) {
        setMetroState(METRO_STATE_DOOR_CLOSE, nowMs);
      }
      break;
    }

    case METRO_STATE_DOOR_CLOSE: {
      if (metroDoorStep == 0) {
        setMetroState(METRO_STATE_WAIT_BEFORE_LEAVE, nowMs);
      }
      break;
    }

  case METRO_STATE_WAIT_BEFORE_LEAVE: {
    if (nowMs - metroStateStartMs >= METRO_PARAM_A_MS) {
      setMetroState(METRO_STATE_TRAIN_CAMERA_MOVE, nowMs);
    }
    break;
  }

case METRO_STATE_TRAIN_CAMERA_MOVE: {
  updateMetroTrainCameraMove(nowMs);
  break;
}


case METRO_STATE_TRAIN_LEAVE: {
  if (nowMs - metroStateStartMs >= METRO_TRAIN_RUN_BEFORE_EXIT_MS) {
    setMetroState(METRO_STATE_TRAIN_EXIT, nowMs);
  }
  break;
}


case METRO_STATE_TRAIN_EXIT: {
  updateMetroTrainExit(nowMs);

  if (metroTrainX + METRO_TRAIN_W < 0) {

    metroNextStation();

    setMetroState(METRO_STATE_BG_INTRO, nowMs);

    metroBgReverse = false;
    metroBgStopped = false;
    metroBgSlowing = false;
    metroBgMoveIntervalMs = METRO_BG_FAST_INTERVAL_MS;

    metroTrainX = -METRO_TRAIN_W;
    metroSecondTrainVisible = false;
    metroDoorStep = 0;
    clearMetroPeds();
  }

  break;
}


    default:
      break;
  }
}

// =====================================================
// 繪製站牌
// =====================================================
static void drawMetroStationIfNeeded() {
  if (metroState == METRO_STATE_STATION_IN ||
      metroState == METRO_STATE_STATION_HOLD ||
      metroState == METRO_STATE_STATION_OUT) {
const MetroStationDef& station =
  metroStationList[metroCurrentStationIndex];

drawMetroSpritePal(
  metroStationX,
  METRO_STATION_Y,
  station.w,
  station.h,
  station.sprite,
  station.palette
);
  }
}

// =====================================================
// 繪製列車
// =====================================================
static void drawMetroTrainIfNeeded() {
  if (metroState == METRO_STATE_TRAIN_IN ||
      metroState == METRO_STATE_TRAIN_HOLD ||
      metroState == METRO_STATE_CLOCK ||
      metroState == METRO_STATE_DOOR_OPEN ||
      metroState == METRO_STATE_PED_EXIT ||
      metroState == METRO_STATE_WAIT_BEFORE_ENTER ||
      metroState == METRO_STATE_PED_ENTER ||
      metroState == METRO_STATE_WAIT_BEFORE_CLOSE ||
      metroState == METRO_STATE_DOOR_CLOSE ||
      metroState == METRO_STATE_WAIT_BEFORE_LEAVE ||
      metroState == METRO_STATE_TRAIN_CAMERA_MOVE ||
      metroState == METRO_STATE_TRAIN_LEAVE  ||
      metroState == METRO_STATE_TRAIN_EXIT) {

    if (metroSecondTrainVisible) {
drawMetroSpritePal(
  metroTrainX - METRO_TRAIN_W,
  METRO_TRAIN_Y,
  METRO_TRAIN_W,
  METRO_TRAIN_H,
  METRO_TRAIN,
  METRO_TRAIN_PALETTE
);
    }

drawMetroSpritePal(
  metroTrainX,
  METRO_TRAIN_Y,
  METRO_TRAIN_W,
  METRO_TRAIN_H,
  METRO_TRAIN,
  METRO_TRAIN_PALETTE
);
  }
}

// =====================================================
// 是否繪製時鐘
// 進站完成後，時鐘停止繪圖
// =====================================================
static bool metroShouldDrawClockText() {
  return (
    metroState == METRO_STATE_CLOCK ||
    metroState == METRO_STATE_DOOR_OPEN ||
    metroState == METRO_STATE_PED_EXIT ||
    metroState == METRO_STATE_WAIT_BEFORE_ENTER ||
    metroState == METRO_STATE_PED_ENTER
  );
}

// =====================================================
// 是否繪製車門區
// =====================================================
static bool metroShouldDrawDoorArea() {
  return (
    metroState == METRO_STATE_CLOCK ||
    metroState == METRO_STATE_DOOR_OPEN ||
    metroState == METRO_STATE_PED_EXIT ||
    metroState == METRO_STATE_WAIT_BEFORE_ENTER ||
    metroState == METRO_STATE_PED_ENTER ||
    metroState == METRO_STATE_WAIT_BEFORE_CLOSE ||
    metroState == METRO_STATE_DOOR_CLOSE ||
    metroState == METRO_STATE_WAIT_BEFORE_LEAVE
  );
}

// =====================================================
// 繪製整體捷運場景
// =====================================================
static void renderMetroScene() {
  drawMetroBackground();

  drawMetroStationIfNeeded();
  drawMetroTrainIfNeeded();

  if (metroShouldDrawDoorArea()) {
    if (metroState == METRO_STATE_DOOR_OPEN) {
      METRO_DOOR_OPEN();
    } else if (metroState == METRO_STATE_DOOR_CLOSE) {
      drawMetroPeds();
      METRO_DOOR_CLOSE();
    } else {
      drawMetroDoorInside();

      if (metroState == METRO_STATE_CLOCK) {
        drawMetroDoorsAtStep(0);
      } else {
        drawMetroPeds();
        drawMetroDoorsAtStep(metroDoorStep);
      }
    }
  }

  if (metroShouldDrawClockText()) {
    drawThemeClockText();
  }
  
}

// =====================================================
// 初始化
// =====================================================
static void MetroModeInit() {
  if (!ModefirstRun) return;

  randomSeed(millis());

  metroCurrentStationIndex = METRO_CENTER_STATION_INDEX;
  metroLastRoute = 255;
  metroPickNewRoute();

  metroState = METRO_STATE_BG_INTRO;
  metroStateStartMs = millis();

  metroBgX = 0;
  metroBgLastMoveMs = millis();
  metroBgMoveIntervalMs = METRO_BG_FAST_INTERVAL_MS;
  metroBgSlowLastMs = millis();
  metroBgSlowing = false;
  metroBgStopped = false;
  metroBgReverse = false;
  metroBgLeaveAccelLastMs = millis();

  metroStationX = METRO_SCR_W;
  metroStationLastMoveMs = millis();

  metroTrainX = -METRO_TRAIN_W;
  metroSecondTrainVisible = false;
  metroTrainLastMoveMs = millis();

  metroDoorStep = 0;
  metroDoorLastFrameMs = millis();

  metroPedTotalGroups = 0;
  metroPedSpawnedGroups = 0;
  metroPedGroupCanSpawnNext = true;
  metroPedStateStartMs = millis();
  clearMetroPeds();

  ModefirstRun = false;
}

// =====================================================
// 主函式
// =====================================================
void MetroMode() {
  MetroModeInit();

  unsigned long nowMs = millis();

  updateMetroBackground(nowMs);
  updateMetroState(nowMs);

  renderMetroScene();

  wait_with_display(METRO_FRAME_DELAY_MS);
}
