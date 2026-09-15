#include "Metro.h"

// =====================================================
// Metro 2D / 2.5D 整合控制器
//
// METRO_STARTUP_MODE 可設定為：
//   METRO_STARTUP_2D     固定使用 2D
//   METRO_STARTUP_25D    固定使用 2.5D
//   METRO_STARTUP_RANDOM 每次進入 MetroMode 時隨機選一種
//
// 模式只會在 initSelectedMetroMode() 決定一次；之後 MetroMode()
// 會持續執行同一模式，直到外部重新把 ModefirstRun 設為 true。
// =====================================================

enum MetroStartupMode : uint8_t {
  METRO_STARTUP_2D = 0,
  METRO_STARTUP_25D,
  METRO_STARTUP_RANDOM
};

// 在這裡選擇啟動策略。
static const MetroStartupMode METRO_STARTUP_MODE =
  METRO_STARTUP_RANDOM;

static const uint8_t METRO_ACTIVE_2D = 0;
static const uint8_t METRO_ACTIVE_25D = 1;

static uint8_t metroActiveMode =
  METRO_ACTIVE_25D;

// =====================================================
// 2D Metro 實作
// =====================================================
namespace Metro2D {


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
static void init() {
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
static void run() {
  init();

  unsigned long nowMs = millis();

  updateMetroBackground(nowMs);
  updateMetroState(nowMs);

  renderMetroScene();

  wait_with_display(METRO_FRAME_DELAY_MS);
}

}  // namespace Metro2D

// =====================================================
// 2.5D Metro 實作
// =====================================================
namespace Metro25D {


// =====================================================
// Metro Mode 2.5D - RGB565
//
// Metro.h 內的主要圖資使用 8 位元索引圖＋RGB565 色盤：
//   const uint8_t  METRO_GROUND[] PROGMEM;
//   const uint16_t METRO_GROUND_PALETTE[] PROGMEM;
//   const uint8_t  METRO_TILE1[] PROGMEM;
//   const uint16_t METRO_TILE1_PALETTE[] PROGMEM;
//   const uint8_t  METRO_TRAIN[] PROGMEM;
//   const uint16_t METRO_TRAIN_PALETTE[] PROGMEM;
//
// 所有索引圖的索引 0 都視為透明色。
// =====================================================

static const int METRO_SCR_W = 64;
static const int METRO_SCR_H = 64;

static const uint16_t METRO_CLEAR_COLOR = 0x9492;
static const uint8_t METRO_TRANSPARENT_INDEX = 0;

// =====================================================
// 圖片尺寸
// =====================================================

static const int GROUND_SHEET_W = 160;
static const int GROUND_SHEET_H = 20;
static const int GROUND_FRAME_W = 40;
static const int GROUND_FRAME_H = 20;

static const int TILE_SHEET_W = 120;
static const int TILE_SHEET_H = 39;
static const int TILE_FRAME_W = 60;
static const int TILE_FRAME_H = 39;

static const int K_TRAIN_W = 96;
static const int K_TRAIN_H = 86;


// =====================================================
// 車站站牌圖資（8 位元索引圖＋RGB565 色盤）
//
// Metro.h 中每一張站牌由兩個陣列組成：
// 1. METRO_STATION_xxx：每個像素儲存色盤索引
// 2. METRO_STATION_xxx_PALETTE：索引對應的 RGB565 顏色
//
// 索引 0 視為透明色，不會繪製。
// =====================================================

static const int METRO_STATION_Y = 2;

struct MetroStationDef {
  const uint8_t* sprite;
  const uint16_t* palette;
  int w;
  int h;
};

static const MetroStationDef metroStationList[] = {
  { METRO_STATION_01,   METRO_STATION_01_PALETTE,   59, 25 },  // 哈瑪星
  { METRO_STATION_02,   METRO_STATION_02_PALETTE,   59, 25 },  // 鹽埕埔
  { METRO_STATION_04,   METRO_STATION_04_PALETTE,   48, 25 },  // 前金
  { METRO_STATION_C,    METRO_STATION_C_PALETTE,    61, 25 },  // 美麗島
  { METRO_STATION_06,   METRO_STATION_06_PALETTE,   72, 25 },  // 信義國小
  { METRO_STATION_07,   METRO_STATION_07_PALETTE,   72, 25 },  // 文化中心
  { METRO_STATION_08,   METRO_STATION_08_PALETTE,   59, 25 },  // 五塊厝
  { METRO_STATION_09,   METRO_STATION_09_PALETTE,  100, 25 },  // 苓雅運動園區
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
  { METRO_STATION_R19,  METRO_STATION_R19_PALETTE, 100, 25 },  // 楠梓科技園區
  { METRO_STATION_R20,  METRO_STATION_R20_PALETTE,  48, 25 },  // 後勁
  { METRO_STATION_R21,  METRO_STATION_R21_PALETTE,  72, 25 },  // 都會公園
  { METRO_STATION_R22,  METRO_STATION_R22_PALETTE,  48, 25 },  // 青埔
  { METRO_STATION_R22A, METRO_STATION_R22A_PALETTE, 72, 25 },  // 橋頭糖廠
  { METRO_STATION_R23,  METRO_STATION_R23_PALETTE,  72, 25 },  // 橋頭車站
  { METRO_STATION_R24,  METRO_STATION_R24_PALETTE,  72, 25 },  // 岡山高醫
  { METRO_STATION_RK1,  METRO_STATION_RK1_PALETTE,  72, 25 }   // 岡山車站
};

// 路線索引表。每條路線第一站都是美麗島（索引 3）。
static const uint8_t metroRouteWest[] = {
  3, 2, 1, 0
};

static const uint8_t metroRouteEast[] = {
  3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

static const uint8_t metroRouteSouth[] = {
  3, 21, 20, 19, 18, 17, 16, 15, 14
};

static const uint8_t metroRouteNorth[] = {
  3, 22, 23, 24, 25, 26, 27, 28, 29,
  30, 31, 32, 33, 34, 35, 36, 37
};

static const uint8_t METRO_ROUTE_WEST = 0;
static const uint8_t METRO_ROUTE_EAST = 1;
static const uint8_t METRO_ROUTE_SOUTH = 2;
static const uint8_t METRO_ROUTE_NORTH = 3;

static const uint8_t METRO_CENTER_STATION_INDEX = 3;

// =====================================================
// Frame 編號
// =====================================================

static const uint8_t GROUND_RAIL = 0;
static const uint8_t GROUND_FLOOR = 1;
static const uint8_t GROUND_DECOR_2 = 2;
static const uint8_t GROUND_DECOR_3 = 3;

static const uint8_t TILE_UPPER = 0;
static const uint8_t TILE_LOWER = 1;

// =====================================================
// 初始世界座標
// =====================================================

// 鐵軌：
// (30,24) (50,34) (70,44) (90,54)
static const int RAIL_BASE_X = 30;
static const int RAIL_BASE_Y = 24;
static const int RAIL_STEP_X = 20;
static const int RAIL_STEP_Y = 10;

// 上月台：
// (48,0) (78,15) (108,30)
static const int UPPER_BASE_X = 48;
static const int UPPER_BASE_Y = 0;

// 下月台：
// (0,24) (30,39) (60,54)
static const int LOWER_BASE_X = 0;
static const int LOWER_BASE_Y = 24;

static const int PLATFORM_STEP_X = 30;
static const int PLATFORM_STEP_Y = 15;

// 地板：
// 月台座標先 Y + 15，
// 再套用 X + 20、Y - 9。
// 合計為 X + 20、Y + 6。
static const int FLOOR_OFFSET_X = 20;
static const int FLOOR_OFFSET_Y = 6;

// 車廂
static const float TRAIN_START_X = -36.0f;
static const float TRAIN_START_Y = -52.0f;

static const float TRAIN_STOP_X = 29.0f;
static const float TRAIN_STOP_Y = -19.0f;

// CAM
static const float CAMERA_HOME_X = 50.0f;
static const float CAMERA_LEAVE_X = 85.0f;
static const float CAMERA_Y = 2.0f;

// =====================================================
// 動畫時間設定

// =====================================================

// 模式開始後，初始畫面停留的時間。
static const unsigned long FIRST_WAIT_MS = 3000UL;

// 列車從起始位置移動到停靠位置所需的時間。
// 列車總位移約為 65 個像素，畫面約每 30 ms 更新一次。
// 1950 ms = 1.95 秒。
static const unsigned long TRAIN_IN_MS = 1950UL;

// 列車進站並停靠後，月台畫面維持不動的時間。
// 此階段會顯示時鐘文字，也可在未來加入開門或乘客動畫。
// 10000 ms = 15 秒。
static const unsigned long PLATFORM_WAIT_MS = 15000UL;

// 攝影機從初始位置平滑移動到列車離站視角所需的時間。
static const unsigned long CAMERA_MOVE_MS = 2000UL;

// 攝影機移動到離站視角後，在月台開始移動之前額外等待的時間。 2 秒。
static const unsigned long BEFORE_RUN_WAIT_MS = 2000UL;

// 月台場景開始移動並由慢速逐漸加速到最高速度的時間。
// 此階段仍以月台圖資為主。10 秒。
static const unsigned long PLATFORM_ACCEL_MS = 10000UL;

// 地板圖資完整顯示後，場景以最高速度持續移動的時間。15 秒。
static const unsigned long GROUND_RUN_MS = 15000UL;

// 地板切換回月台後，場景由最高速度逐漸減速到完全停止所需的時間。10 秒。
static const unsigned long PLATFORM_DECEL_MS = 10000UL;

// 月台完全停止後，畫面繼續停留的時間。此階段結束後，攝影機會移回初始位置。 3 秒。
static const unsigned long STOP_WAIT_MS = 3000UL;

// 月台圖資與地板圖資進行滑動切換所需的時間。
// 新圖資會從畫面右側逐漸滑入，取代原本的圖資。
// 數值越大，切換速度越慢；數值越小，切換速度越快。0.8 秒。
static const unsigned long SURFACE_TRANSITION_MS = 800UL;

// 車站站牌每移動 1 像素的時間。
// 數值越小，站牌滑入與滑出越快。
static const unsigned long STATION_MOVE_INTERVAL_MS = 35UL;

// 站牌滑入中央後的停留時間。
static const unsigned long STATION_HOLD_MS = 5000UL;

// 每次完成狀態更新與畫面繪製後的等待時間。
// 此數值決定動畫更新頻率。
// 30 ms 約等於每秒更新 33 次：1000 ÷ 30 ≈ 33 FPS。
static const unsigned long FRAME_DELAY_MS = 30UL;



// =====================================================
// 場景移動速度
//
// 單位：X pixel / second。
// Y 移動量固定為 X 的一半。
// =====================================================

static const float SCROLL_START_PPS = 2.0f;
static const float SCROLL_MAX_PPS = 36.0f;

// 480 同時可被：
// 鐵軌間距 20
// 月台間距 30
// 2222/3333 的 160px 週期
// 整除。
static const float SCROLL_WRAP_PX = 480.0f;

// =====================================================
// 狀態機
// =====================================================

enum MetroState : uint8_t {
  METRO_FIRST_WAIT = 0,
  METRO_TRAIN_IN,
  METRO_PLATFORM_WAIT,
  METRO_CAMERA_TO_LEAVE,
  METRO_WAIT_BEFORE_RUN,
  METRO_PLATFORM_ACCEL,

  // 月台加速完成後，站牌滑入、停留、滑出。
  METRO_STATION_IN,
  METRO_STATION_HOLD,
  METRO_STATION_OUT,

  METRO_GROUND_RUN,
  METRO_PLATFORM_DECEL,
  METRO_STOP_WAIT,
  METRO_CAMERA_HOME
};

static MetroState metroState = METRO_FIRST_WAIT;

static unsigned long metroStateStartMs = 0;
static unsigned long metroLastMotionMs = 0;

// =====================================================
// 場景變數
// =====================================================

static float metroCameraX = CAMERA_HOME_X;

static float metroTrainX = TRAIN_START_X;
static float metroTrainY = TRAIN_START_Y;

// 正值表示鐵軌、月台、地板已往左上移動。
static float metroScrollPx = 0.0f;
static float metroScrollSpeedPps = 0.0f;

// 0 = 2222 3333
// 1 = 3333 2222
static uint8_t metroGroundPatternOffset = 0;

// 圖資滑動切換遮罩
static bool metroSurfaceClipEnabled = false;

// 只有畫面 X 大於等於此位置的像素才會繪製
static int metroSurfaceClipX = METRO_SCR_W;


// =====================================================
// 車站站牌控制
// =====================================================

// 目前顯示的站牌索引。
static uint8_t metroCurrentStationIndex =
  METRO_CENTER_STATION_INDEX;

// 目前路線與上一條路線。
static uint8_t metroCurrentRoute = METRO_ROUTE_EAST;
static uint8_t metroLastRoute = 255;

// 目前位於路線索引表中的位置。
static int metroRoutePos = 0;

// 1 表示由美麗島前往終點；-1 表示由終點返回美麗島。
static int metroRouteDir = 1;

// 站牌目前在螢幕上的 X 座標。
static int metroStationX = METRO_SCR_W;

// 上一次移動站牌的時間。
static unsigned long metroStationLastMoveMs = 0;

// =====================================================
// 數學工具
// =====================================================

static float clamp01(float value) {
  if (value < 0.0f) {
    return 0.0f;
  }

  if (value > 1.0f) {
    return 1.0f;
  }

  return value;
}

static float smoothStep(float value) {
  value = clamp01(value);

  return value * value * (3.0f - 2.0f * value);
}

static int roundToInt(float value) {
  if (value >= 0.0f) {
    return (int)(value + 0.5f);
  }

  return (int)(value - 0.5f);
}

static int floorDiv(int value, int divisor) {
  int quotient = value / divisor;
  int remainder = value % divisor;

  if (remainder != 0 && value < 0) {
    quotient--;
  }

  return quotient;
}

static float sceneShiftX() {
  return -metroScrollPx;
}

static float sceneShiftY() {
  return -metroScrollPx * 0.5f;
}

// =====================================================
// 索引圖＋RGB565 色盤繪製
//
// 圖片陣列儲存 uint8_t 色盤索引，
// 實際顏色由對應的 RGB565 palette 取得。
// 支援：
// 1. 世界座標
// 2. CAM 裁切
// 3. Frame 裁切
// 4. 洋紅色透明
// =====================================================

static void drawFrame565World(
  float worldX,
  float worldY,
  int sheetW,
  int sheetH,
  int srcX,
  int srcY,
  int frameW,
  int frameH,
  const uint8_t* sprite,
  const uint16_t* palette
) {
  if (!sprite || !palette) {
    return;
  }

  if (srcX < 0 || srcY < 0) {
    return;
  }

  if (srcX + frameW > sheetW) {
    return;
  }

  if (srcY + frameH > sheetH) {
    return;
  }

  int screenX = roundToInt(
    worldX - metroCameraX
  );

  int screenY = roundToInt(
    worldY - CAMERA_Y
  );

  // 整張圖片在畫面外。
  if (screenX >= METRO_SCR_W) {
    return;
  }

  if (screenY >= METRO_SCR_H) {
    return;
  }

  if (screenX + frameW <= 0) {
    return;
  }

  if (screenY + frameH <= 0) {
    return;
  }

  int beginX = 0;
  int beginY = 0;
  int endX = frameW;
  int endY = frameH;

  if (screenX < 0) {
    beginX = -screenX;
  }

  if (screenY < 0) {
    beginY = -screenY;
  }

  if (screenX + endX > METRO_SCR_W) {
    endX = METRO_SCR_W - screenX;
  }

  if (screenY + endY > METRO_SCR_H) {
    endY = METRO_SCR_H - screenY;
  }

  for (int frameY = beginY; frameY < endY; frameY++) {
    int drawY = screenY + frameY;

    uint32_t sourceRow =
      (uint32_t)(srcY + frameY) *
      (uint32_t)sheetW;

    for (int frameX = beginX; frameX < endX; frameX++) {
      int drawX = screenX + frameX;

      uint32_t sourcePos =
        sourceRow +
        (uint32_t)(srcX + frameX);

      uint8_t paletteIndex =
        pgm_read_byte(&(sprite[sourcePos]));

      // 索引 0 是透明背景。
      if (paletteIndex == METRO_TRANSPARENT_INDEX) {
        continue;
      }

      // 月台與地板切換時，
      // 只繪製遮罩右側的像素。
      if (
        metroSurfaceClipEnabled &&
        drawX < metroSurfaceClipX
      ) {
        continue;
      }

      uint16_t color =
        pgm_read_word(&(palette[paletteIndex]));

      display.drawPixel(
        drawX,
        drawY,
        color
      );
    }
  }
}

// =====================================================
// 繪製 8 位元索引圖＋RGB565 色盤
//
// sprite 中每個像素只儲存一個色盤索引。
// palette 中才是實際的 RGB565 顏色。
// 索引 0 當作透明色。
// =====================================================

static void drawIndexedSprite565(
  int x,
  int y,
  int w,
  int h,
  const uint8_t* sprite,
  const uint16_t* palette
) {
  if (!sprite || !palette) {
    return;
  }

  for (int sourceY = 0; sourceY < h; sourceY++) {
    int drawY = y + sourceY;

    if (drawY < 0 || drawY >= METRO_SCR_H) {
      continue;
    }

    uint32_t sourceRow =
      (uint32_t)sourceY * (uint32_t)w;

    for (int sourceX = 0; sourceX < w; sourceX++) {
      int drawX = x + sourceX;

      if (drawX < 0 || drawX >= METRO_SCR_W) {
        continue;
      }

      uint32_t sourcePos =
        sourceRow + (uint32_t)sourceX;

      uint8_t paletteIndex =
        pgm_read_byte(&(sprite[sourcePos]));

      // 色盤索引 0 為透明背景。
      if (paletteIndex == 0) {
        continue;
      }

      uint16_t color =
        pgm_read_word(&(palette[paletteIndex]));

      display.drawPixel(
        drawX,
        drawY,
        color
      );
    }
  }
}

// =====================================================
// 圖資 Frame 包裝
// =====================================================

static void drawGroundFrame(
  float x,
  float y,
  uint8_t frame
) {
  if (frame > 3) {
    return;
  }

  drawFrame565World(
    x,
    y,
    GROUND_SHEET_W,
    GROUND_SHEET_H,
    frame * GROUND_FRAME_W,
    0,
    GROUND_FRAME_W,
    GROUND_FRAME_H,
    METRO_GROUND,
    METRO_GROUND_PALETTE
  );
}

static void drawTileFrame(
  float x,
  float y,
  uint8_t frame
) {
  if (frame > 1) {
    return;
  }

  drawFrame565World(
    x,
    y,
    TILE_SHEET_W,
    TILE_SHEET_H,
    frame * TILE_FRAME_W,
    0,
    TILE_FRAME_W,
    TILE_FRAME_H,
    METRO_TILE1,
    METRO_TILE1_PALETTE
  );
}

static void drawTrainFrame0(
  float x,
  float y
) {
  drawFrame565World(
    x,
    y,
    K_TRAIN_W,
    K_TRAIN_H,
    0,
    0,
    K_TRAIN_W,
    K_TRAIN_H,
    METRO_TRAIN2,
    METRO_TRAIN2_PALETTE
  );
}

// =====================================================
// 計算目前 CAM 需要補畫的圖塊範圍
// =====================================================

static void visibleRepeatRange(
  float baseX,
  int stepX,
  int frameW,
  int& firstK,
  int& lastK
) {
  float shiftedBaseX =
    baseX + sceneShiftX();

  float leftWorld =
    metroCameraX - (float)frameW;

  float rightWorld =
    metroCameraX + (float)METRO_SCR_W;

  firstK =
    (int)floorf(
      (leftWorld - shiftedBaseX) /
      (float)stepX
    ) - 1;

  lastK =
    (int)ceilf(
      (rightWorld - shiftedBaseX) /
      (float)stepX
    ) + 1;

  // 初始座標從 k=0 開始。
  // 場景往左上移動時，用正 k 補右下畫面。
  if (firstK < 0) {
    firstK = 0;
  }
}

// =====================================================
// 重複繪製 METRO_GROUND
// =====================================================

static void drawGroundLine(
  float baseX,
  float baseY,
  int stepX,
  int stepY,
  uint8_t frame
) {
  int firstK;
  int lastK;

  visibleRepeatRange(
    baseX,
    stepX,
    GROUND_FRAME_W,
    firstK,
    lastK
  );

  float shiftX = sceneShiftX();
  float shiftY = sceneShiftY();

  for (int k = firstK; k <= lastK; k++) {
    float x =
      baseX +
      (float)(k * stepX) +
      shiftX;

    float y =
      baseY +
      (float)(k * stepY) +
      shiftY;

    drawGroundFrame(
      x,
      y,
      frame
    );
  }
}

// =====================================================
// 重複繪製 METRO_TILE1
// =====================================================

static void drawTileLine(
  float baseX,
  float baseY,
  uint8_t frame
) {
  int firstK;
  int lastK;

  visibleRepeatRange(
    baseX,
    PLATFORM_STEP_X,
    TILE_FRAME_W,
    firstK,
    lastK
  );

  float shiftX = sceneShiftX();
  float shiftY = sceneShiftY();

  for (int k = firstK; k <= lastK; k++) {
    float x =
      baseX +
      (float)(k * PLATFORM_STEP_X) +
      shiftX;

    float y =
      baseY +
      (float)(k * PLATFORM_STEP_Y) +
      shiftY;

    drawTileFrame(
      x,
      y,
      frame
    );
  }
}

// =====================================================
// 鐵軌
//
// frame0：
// (30,24)
// (50,34)
// (70,44)
// (90,54)
// =====================================================

static void drawRails() {
  drawGroundLine(
    (float)RAIL_BASE_X,
    (float)RAIL_BASE_Y,
    RAIL_STEP_X,
    RAIL_STEP_Y,
    GROUND_RAIL
  );
}

// =====================================================
// 上月台
//
// frame0：
// (48,0)
// (78,15)
// (108,30)
// =====================================================

static void drawUpperPlatform() {
  drawTileLine(
    (float)UPPER_BASE_X,
    (float)UPPER_BASE_Y,
    TILE_UPPER
  );
}

// =====================================================
// 下月台
//
// frame1：
// (0,24)
// (30,39)
// (60,54)
// =====================================================

static void drawLowerPlatform() {
  drawTileLine(
    (float)LOWER_BASE_X,
    (float)LOWER_BASE_Y,
    TILE_LOWER
  );
}

// =====================================================
// 地板裝飾排列
//
// 2222 3333 2222 3333...
// =====================================================

static uint8_t groundDecorFrame(int k) {
  int block = floorDiv(k, 4);

  int parity = block % 2;

  if (parity < 0) {
    parity += 2;
  }

  parity =
    (parity + metroGroundPatternOffset) & 1;

  if (parity == 0) {
    return GROUND_DECOR_2;
  }

  return GROUND_DECOR_3;
}

// =====================================================
// 鐵軌右上側地板
//
// 第一排：GROUND frame1
// 第二排：GROUND frame2 / frame3
// 第三排：GROUND frame1，與第一排相同
//
// 每一排往右上移動：
// X + 20
// Y - 10
// =====================================================

static void drawUpperGround() {
  float shiftX = sceneShiftX();
  float shiftY = sceneShiftY();

  // ===================================================
  // 第一排地板
  //
  // 緊鄰鐵軌右上方。
  // 使用 GROUND frame1。
  // ===================================================

  const float innerX =
    (float)RAIL_BASE_X + 20.0f;

  const float innerY =
    (float)RAIL_BASE_Y - 10.0f;

  int firstK1;
  int lastK1;

  visibleRepeatRange(
    innerX,
    RAIL_STEP_X,
    GROUND_FRAME_W,
    firstK1,
    lastK1
  );

  for (int k = firstK1; k <= lastK1; k++) {
    float x =
      innerX +
      (float)(k * RAIL_STEP_X) +
      shiftX;

    float y =
      innerY +
      (float)(k * RAIL_STEP_Y) +
      shiftY;

    drawGroundFrame(
      x,
      y,
      GROUND_FLOOR
    );
  }

  // ===================================================
  // 第二排裝飾地板
  //
  // 從第一排再往右上移：
  // X + 20
  // Y - 10
  //
  // 使用 GROUND frame2 / frame3。
  // 排列方式為 2222 3333。
  // ===================================================

  const float decorX =
    innerX + 20.0f;

  const float decorY =
    innerY - 10.0f;

  int firstK2;
  int lastK2;

  visibleRepeatRange(
    decorX,
    RAIL_STEP_X,
    GROUND_FRAME_W,
    firstK2,
    lastK2
  );

  for (int k = firstK2; k <= lastK2; k++) {
    float x =
      decorX +
      (float)(k * RAIL_STEP_X) +
      shiftX;

    float y =
      decorY +
      (float)(k * RAIL_STEP_Y) +
      shiftY;

    drawGroundFrame(
      x,
      y,
      groundDecorFrame(k)
    );
  }

  // ===================================================
  // 第三排地板
  //
  // 從第二排再往右上移：
  // X + 20
  // Y - 10
  //
  // 圖資與第一排相同，
  // 使用 GROUND frame1。
  // ===================================================

  const float thirdX =
    decorX + 20.0f;

  const float thirdY =
    decorY - 10.0f;

  int firstK3;
  int lastK3;

  visibleRepeatRange(
    thirdX,
    RAIL_STEP_X,
    GROUND_FRAME_W,
    firstK3,
    lastK3
  );

  for (int k = firstK3; k <= lastK3; k++) {
    float x =
      thirdX +
      (float)(k * RAIL_STEP_X) +
      shiftX;

    float y =
      thirdY +
      (float)(k * RAIL_STEP_Y) +
      shiftY;

    drawGroundFrame(
      x,
      y,
      GROUND_FLOOR
    );
  }
}

// =====================================================
// 鐵軌左下側地板
//
// 全部使用 GROUND frame1。
// =====================================================

static void drawLowerGround() {
  drawGroundLine(
    (float)(
      LOWER_BASE_X +
      FLOOR_OFFSET_X
    ),
    (float)(
      LOWER_BASE_Y +
      FLOOR_OFFSET_Y
    ),
    RAIL_STEP_X,
    RAIL_STEP_Y,
    GROUND_FLOOR
  );
}

// =====================================================
// 車站路線與站牌動畫
// =====================================================

static const uint8_t* getMetroRouteArray(
  uint8_t route
) {
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

static uint8_t getMetroRouteLength(
  uint8_t route
) {
  switch (route) {
    case METRO_ROUTE_WEST:
      return sizeof(metroRouteWest) /
             sizeof(metroRouteWest[0]);

    case METRO_ROUTE_EAST:
      return sizeof(metroRouteEast) /
             sizeof(metroRouteEast[0]);

    case METRO_ROUTE_SOUTH:
      return sizeof(metroRouteSouth) /
             sizeof(metroRouteSouth[0]);

    case METRO_ROUTE_NORTH:
      return sizeof(metroRouteNorth) /
             sizeof(metroRouteNorth[0]);
  }

  return sizeof(metroRouteEast) /
         sizeof(metroRouteEast[0]);
}

static void metroPickNewRoute() {
  uint8_t newRoute;

  do {
    newRoute = (uint8_t)random(0, 4);
  } while (newRoute == metroLastRoute);

  metroCurrentRoute = newRoute;
  metroLastRoute = newRoute;
  metroRoutePos = 0;
  metroRouteDir = 1;

  const uint8_t* routeArray =
    getMetroRouteArray(metroCurrentRoute);

  metroCurrentStationIndex =
    routeArray[metroRoutePos];
}

static void metroNextStation() {
  const uint8_t* routeArray =
    getMetroRouteArray(metroCurrentRoute);

  uint8_t routeLength =
    getMetroRouteLength(metroCurrentRoute);

  metroRoutePos += metroRouteDir;

  // 到達終點後，改為往美麗島方向返回。
  if (metroRoutePos >= routeLength) {
    metroRoutePos = routeLength - 2;
    metroRouteDir = -1;
  }

  // 返回美麗島後，重新隨機選擇下一條路線。
  if (metroRoutePos <= 0 && metroRouteDir < 0) {
    metroPickNewRoute();
    return;
  }

  metroCurrentStationIndex =
    routeArray[metroRoutePos];
}

static int getMetroStationCenterX() {
  const MetroStationDef& station =
    metroStationList[metroCurrentStationIndex];

  return
    (METRO_SCR_W - station.w) / 2;
}

static bool shouldDrawStationBoard() {
  return
    metroState == METRO_STATION_IN ||
    metroState == METRO_STATION_HOLD ||
    metroState == METRO_STATION_OUT;
}

static void drawMetroStationBoard() {
  if (!shouldDrawStationBoard()) {
    return;
  }

  const MetroStationDef& station =
    metroStationList[metroCurrentStationIndex];

  drawIndexedSprite565(
    metroStationX,
    METRO_STATION_Y,
    station.w,
    station.h,
    station.sprite,
    station.palette
  );
}

// =====================================================
// 場景判斷
// =====================================================

static bool useGround() {
  return metroState == METRO_GROUND_RUN;
}

static bool shouldDrawClock() {
  return metroState == METRO_PLATFORM_WAIT;
}

// =====================================================
// 完整場景繪製
//
// 順序：
// 1. 鐵軌
// 2. 上月台／右上地板
// 3. 車廂
// 4. 下月台／左下地板
// 5. 時鐘文字
// =====================================================

static void renderMetroScene() {
  display.fillScreen(
    METRO_CLEAR_COLOR
  );

  unsigned long nowMs = millis();

  unsigned long stateElapsed =
    nowMs - metroStateStartMs;

  // 地板剛開始出現
  bool groundEntering =
    metroState == METRO_GROUND_RUN &&
    stateElapsed < SURFACE_TRANSITION_MS;

  // 月台剛開始重新出現
  bool platformEntering =
    metroState == METRO_PLATFORM_DECEL &&
    stateElapsed < SURFACE_TRANSITION_MS;

  // 計算切換遮罩的位置。
  // 64 -> 0，表示從畫面右邊逐漸移向左邊。
  int transitionX = METRO_SCR_W;

  if (groundEntering || platformEntering) {
    float progress =
      (float)stateElapsed /
      (float)SURFACE_TRANSITION_MS;

    float eased =
      smoothStep(progress);

    transitionX =
      METRO_SCR_W -
      roundToInt(
        eased *
        (float)METRO_SCR_W
      );

    if (transitionX < 0) {
      transitionX = 0;
    }

    if (transitionX > METRO_SCR_W) {
      transitionX = METRO_SCR_W;
    }
  }

  // ===================================================
  // 1. 鐵軌
  // ===================================================

  metroSurfaceClipEnabled = false;

  drawRails();

  // ===================================================
  // 2. 上方月台／地板
  // ===================================================

  if (groundEntering) {
    // 原本的月台先完整畫出。
    metroSurfaceClipEnabled = false;

    drawUpperPlatform();

    // 新地板從右邊逐漸滑入。
    metroSurfaceClipX = transitionX;
    metroSurfaceClipEnabled = true;

    drawUpperGround();
  }
  else if (platformEntering) {
    // 原本的地板先完整畫出。
    metroSurfaceClipEnabled = false;

    drawUpperGround();

    // 新月台從右邊逐漸滑入。
    metroSurfaceClipX = transitionX;
    metroSurfaceClipEnabled = true;

    drawUpperPlatform();
  }
  else {
    metroSurfaceClipEnabled = false;

    if (useGround()) {
      drawUpperGround();
    } else {
      drawUpperPlatform();
    }
  }

  metroSurfaceClipEnabled = false;

  // ===================================================
  // 3. 車廂
  // ===================================================

  drawTrainFrame0(
    metroTrainX,
    metroTrainY
  );

  // ===================================================
  // 4. 下方月台／地板
  // ===================================================

  if (groundEntering) {
    // 原本的月台先完整畫出。
    metroSurfaceClipEnabled = false;

    drawLowerPlatform();

    // 新地板從右邊逐漸滑入。
    metroSurfaceClipX = transitionX;
    metroSurfaceClipEnabled = true;

    drawLowerGround();
  }
  else if (platformEntering) {
    // 原本的地板先完整畫出。
    metroSurfaceClipEnabled = false;

    drawLowerGround();

    // 新月台從右邊逐漸滑入。
    metroSurfaceClipX = transitionX;
    metroSurfaceClipEnabled = true;

    drawLowerPlatform();
  }
  else {
    metroSurfaceClipEnabled = false;

    if (useGround()) {
      drawLowerGround();
    } else {
      drawLowerPlatform();
    }
  }

  metroSurfaceClipEnabled = false;

  // ===================================================
  // 5. 車站站牌
  //
  // 站牌使用索引圖＋RGB565 色盤，並繪製在場景最上層。
  // ===================================================

  drawMetroStationBoard();

  // ===================================================
  // 6. 時鐘文字
  // ===================================================

  if (shouldDrawClock()) {
    drawThemeClockText();
  }
}

// =====================================================
// 狀態切換
// =====================================================

static void setState(
  uint8_t nextState,
  unsigned long nowMs
) {
  metroState = (MetroState)nextState;
  metroStateStartMs = nowMs;
  metroLastMotionMs = nowMs;

  if (metroState == METRO_STATION_IN) {
    metroStationX = METRO_SCR_W;
    metroStationLastMoveMs = nowMs;
  }

  if (metroState == METRO_STATION_OUT) {
    metroStationLastMoveMs = nowMs;
  }
}

// =====================================================
// 更新場景偏移
// =====================================================

static void updateScroll(
  unsigned long nowMs,
  float speedPps
) {
  unsigned long deltaMs =
    nowMs - metroLastMotionMs;

  metroLastMotionMs = nowMs;

  // 避免系統偶爾停頓時場景突然跳太遠。
  if (deltaMs > 200UL) {
    deltaMs = 200UL;
  }

  metroScrollSpeedPps = speedPps;

  metroScrollPx +=
    speedPps *
    ((float)deltaMs / 1000.0f);

  while (metroScrollPx >= SCROLL_WRAP_PX) {
    metroScrollPx -= SCROLL_WRAP_PX;
  }

  while (metroScrollPx < 0.0f) {
    metroScrollPx += SCROLL_WRAP_PX;
  }
}

// =====================================================
// 動畫狀態更新
// =====================================================

static void updateMetroState(
  unsigned long nowMs
) {
  unsigned long elapsed =
    nowMs - metroStateStartMs;

  switch (metroState) {
    // -------------------------------------------------
    // 初始畫面等待 3 秒
    // -------------------------------------------------
    case METRO_FIRST_WAIT: {
      metroCameraX = CAMERA_HOME_X;

      metroTrainX = TRAIN_START_X;
      metroTrainY = TRAIN_START_Y;

      metroScrollSpeedPps = 0.0f;

      if (elapsed >= FIRST_WAIT_MS) {
        setState(
          METRO_TRAIN_IN,
          nowMs
        );
      }

      break;
    }

    // -------------------------------------------------
    // 車廂逐幀由 (-36,-52) 移動至 (29,-19)
    // -------------------------------------------------
    case METRO_TRAIN_IN: {
      float progress =
        (float)elapsed /
        (float)TRAIN_IN_MS;

      float eased =
        smoothStep(progress);

      metroTrainX =
        TRAIN_START_X +
        (TRAIN_STOP_X - TRAIN_START_X) *
        eased;

      metroTrainY =
        TRAIN_START_Y +
        (TRAIN_STOP_Y - TRAIN_START_Y) *
        eased;

      if (elapsed >= TRAIN_IN_MS) {
        metroTrainX = TRAIN_STOP_X;
        metroTrainY = TRAIN_STOP_Y;

        setState(
          METRO_PLATFORM_WAIT,
          nowMs
        );
      }

      break;
    }

    // -------------------------------------------------
    // 停靠 10 秒
    // 後續開門與乘客動畫可放在這個階段
    // -------------------------------------------------
    case METRO_PLATFORM_WAIT: {
      metroTrainX = TRAIN_STOP_X;
      metroTrainY = TRAIN_STOP_Y;

      metroScrollSpeedPps = 0.0f;

      if (elapsed >= PLATFORM_WAIT_MS) {
        setState(
          METRO_CAMERA_TO_LEAVE,
          nowMs
        );
      }

      break;
    }

    // -------------------------------------------------
    // CAM 由 (50,2) 平滑移到 (85,2)
    // -------------------------------------------------
    case METRO_CAMERA_TO_LEAVE: {
      float progress =
        (float)elapsed /
        (float)CAMERA_MOVE_MS;

      float eased =
        smoothStep(progress);

      metroCameraX =
        CAMERA_HOME_X +
        (CAMERA_LEAVE_X - CAMERA_HOME_X) *
        eased;

      if (elapsed >= CAMERA_MOVE_MS) {
        metroCameraX = CAMERA_LEAVE_X;

        setState(
          METRO_WAIT_BEFORE_RUN,
          nowMs
        );
      }

      break;
    }

    // -------------------------------------------------
    // CAM 到達 (85,2) 後等待 2 秒
    // -------------------------------------------------
    case METRO_WAIT_BEFORE_RUN: {
      metroCameraX = CAMERA_LEAVE_X;
      metroScrollSpeedPps = 0.0f;

      if (elapsed >= BEFORE_RUN_WAIT_MS) {
        setState(
          METRO_PLATFORM_ACCEL,
          nowMs
        );
      }

      break;
    }

    // -------------------------------------------------
    // 月台場景運行 8 秒，由慢到快
    // -------------------------------------------------
    case METRO_PLATFORM_ACCEL: {
      float progress =
        (float)elapsed /
        (float)PLATFORM_ACCEL_MS;

      float eased =
        smoothStep(progress);

      float speed =
        SCROLL_START_PPS +
        (
          SCROLL_MAX_PPS -
          SCROLL_START_PPS
        ) *
        eased;

      updateScroll(
        nowMs,
        speed
      );

      if (elapsed >= PLATFORM_ACCEL_MS) {
        setState(
          METRO_STATION_IN,
          nowMs
        );
      }

      break;
    }

    // -------------------------------------------------
    // 站牌由畫面右側滑入中央。
    // 場景仍維持最高速度移動。
    // -------------------------------------------------
    case METRO_STATION_IN: {
      updateScroll(
        nowMs,
        SCROLL_MAX_PPS
      );

      if (
        nowMs - metroStationLastMoveMs >=
        STATION_MOVE_INTERVAL_MS
      ) {
        metroStationLastMoveMs = nowMs;
        metroStationX--;

        int centerX =
          getMetroStationCenterX();

        if (metroStationX <= centerX) {
          metroStationX = centerX;

          setState(
            METRO_STATION_HOLD,
            nowMs
          );
        }
      }

      break;
    }

    // -------------------------------------------------
    // 站牌停留在畫面中央。
    // 場景仍維持最高速度移動。
    // -------------------------------------------------
    case METRO_STATION_HOLD: {
      updateScroll(
        nowMs,
        SCROLL_MAX_PPS
      );

      if (elapsed >= STATION_HOLD_MS) {
        setState(
          METRO_STATION_OUT,
          nowMs
        );
      }

      break;
    }

    // -------------------------------------------------
    // 站牌由中央繼續往左滑出畫面。
    // 完全離開後才進入地板運行階段。
    // -------------------------------------------------
    case METRO_STATION_OUT: {
      updateScroll(
        nowMs,
        SCROLL_MAX_PPS
      );

      if (
        nowMs - metroStationLastMoveMs >=
        STATION_MOVE_INTERVAL_MS
      ) {
        metroStationLastMoveMs = nowMs;
        metroStationX--;

        const MetroStationDef& station =
          metroStationList[metroCurrentStationIndex];

        if (metroStationX + station.w < 0) {
          // 下一次循環改顯示路線中的下一站。
          metroNextStation();

          setState(
            METRO_GROUND_RUN,
            nowMs
          );
        }
      }

      break;
    }

    // -------------------------------------------------
    // 地板取代月台，持續運行。
    // -------------------------------------------------
    case METRO_GROUND_RUN: {
      updateScroll(
        nowMs,
        SCROLL_MAX_PPS
      );

      if (elapsed >= GROUND_RUN_MS) {
        setState(
          METRO_PLATFORM_DECEL,
          nowMs
        );
      }

      break;
    }

    // -------------------------------------------------
    // 月台重新出現，5 秒內逐漸停止
    // -------------------------------------------------
    case METRO_PLATFORM_DECEL: {
      float progress =
        (float)elapsed /
        (float)PLATFORM_DECEL_MS;

      float eased =
        smoothStep(progress);

      float speed =
        SCROLL_MAX_PPS *
        (1.0f - eased);

      updateScroll(
        nowMs,
        speed
      );

      if (elapsed >= PLATFORM_DECEL_MS) {
        metroScrollSpeedPps = 0.0f;

        setState(
          METRO_STOP_WAIT,
          nowMs
        );
      }

      break;
    }

    // -------------------------------------------------
    // 月台停止後等待 5 秒
    // -------------------------------------------------
    case METRO_STOP_WAIT: {
      metroScrollSpeedPps = 0.0f;

      if (elapsed >= STOP_WAIT_MS) {
        setState(
          METRO_CAMERA_HOME,
          nowMs
        );
      }

      break;
    }

    // -------------------------------------------------
    // CAM 由 (85,2) 平滑移回 (50,2)
    // 完成後回到第 2 步的 10 秒停靠
    // -------------------------------------------------
    case METRO_CAMERA_HOME: {
      float progress =
        (float)elapsed /
        (float)CAMERA_MOVE_MS;

      float eased =
        smoothStep(progress);

      metroCameraX =
        CAMERA_LEAVE_X +
        (CAMERA_HOME_X - CAMERA_LEAVE_X) *
        eased;

      if (elapsed >= CAMERA_MOVE_MS) {
        metroCameraX = CAMERA_HOME_X;

        // 不重新播放第一次進站，
        // 直接回到 10 秒停靠。
        setState(
          METRO_PLATFORM_WAIT,
          nowMs
        );
      }

      break;
    }

    default: {
      metroCameraX = CAMERA_HOME_X;

      metroTrainX = TRAIN_STOP_X;
      metroTrainY = TRAIN_STOP_Y;

      metroScrollSpeedPps = 0.0f;

      setState(
        METRO_PLATFORM_WAIT,
        nowMs
      );

      break;
    }
  }
}

// =====================================================
// 初始化
// =====================================================

static void init() {
  if (!ModefirstRun) {
    return;
  }

  unsigned long nowMs = millis();

  randomSeed(nowMs);

  metroState = METRO_FIRST_WAIT;
  metroStateStartMs = nowMs;
  metroLastMotionMs = nowMs;

  metroCameraX = CAMERA_HOME_X;

  metroTrainX = TRAIN_START_X;
  metroTrainY = TRAIN_START_Y;

  metroScrollPx = 0.0f;
  metroScrollSpeedPps = 0.0f;

  metroGroundPatternOffset = 0;

  // 從美麗島開始，並隨機選擇一條路線。
  metroCurrentStationIndex =
    METRO_CENTER_STATION_INDEX;

  metroLastRoute = 255;
  metroPickNewRoute();

  metroStationX = METRO_SCR_W;
  metroStationLastMoveMs = nowMs;

  ModefirstRun = false;
}

// =====================================================
// 主函式
// =====================================================

static void run() {
  init();

  unsigned long nowMs = millis();

  updateMetroState(nowMs);
  renderMetroScene();

  wait_with_display(
    FRAME_DELAY_MS 
  );
}

}  // namespace Metro25D


// =====================================================
// 外層模式選擇
// =====================================================

static uint8_t chooseMetroActiveMode() {
  switch (METRO_STARTUP_MODE) {
    case METRO_STARTUP_2D:
      return METRO_ACTIVE_2D;

    case METRO_STARTUP_25D:
      return METRO_ACTIVE_25D;

    case METRO_STARTUP_RANDOM:
    default:
      return (random(0, 2) == 0)
        ? METRO_ACTIVE_2D
        : METRO_ACTIVE_25D;
  }
}

// =====================================================
// 統一初始化
// =====================================================

static void initSelectedMetroMode() {
  // 沿用原本主程式的模式初始化旗標。
  // 只在剛進入 MetroMode 時選擇一次模式。
  if (!ModefirstRun) {
    return;
  }

  randomSeed(millis());
  metroActiveMode = chooseMetroActiveMode();

  // 只初始化被選中的模式。
  // 被選中的內部 Init 會把 ModefirstRun 設為 false。
  switch (metroActiveMode) {
    case METRO_ACTIVE_2D:
      Metro2D::init();
      break;

    case METRO_ACTIVE_25D:
    default:
      Metro25D::init();
      break;
  }
}

// =====================================================
// 統一主函式
// =====================================================

void MetroMode() {
  initSelectedMetroMode();

  // 初始化後不再重新判斷，持續執行同一個模式。
  switch (metroActiveMode) {
    case METRO_ACTIVE_2D:
      Metro2D::run();
      break;

    case METRO_ACTIVE_25D:
    default:
      Metro25D::run();
      break;
  }
}
