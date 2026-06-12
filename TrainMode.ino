#include "Train.h"
#include "Kart.h"
#include <math.h>

// =====================================================
// Train Mode 基本設定
// =====================================================
static const int TRAIN_SCREEN_W = 64;
static const int TRAIN_SCREEN_H = 64;

static const int TRAIN_WORLD_W = 368;
static const int TRAIN_WORLD_H = 704;

static const int TRAIN_TILE_W = 16;
static const int TRAIN_TILE_H = 16;
static const int TRAIN_SHEET_W = 96;

static const int TRAIN_CITY_W = 32;
static const int TRAIN_CITY_H = 16;
static const int TRAIN_CITY_SHEET_W = 512;

// =====================================================
// 全部圖資共用透明色
// 只要圖資像素顏色是 0xDB84，就不會被畫出來
// =====================================================
static const uint16_t TRAIN_TRANSPARENT_COLOR = 0xDB84;

static const uint8_t TRAIN_FRAME_H_BODY_A = 0;
static const uint8_t TRAIN_FRAME_V_BODY_A = 1;
static const uint8_t TRAIN_FRAME_H_BODY_B = 2;
static const uint8_t TRAIN_FRAME_V_BODY_B = 3;
static const uint8_t TRAIN_FRAME_RAIL_STRAIGHT = 4;
static const uint8_t TRAIN_FRAME_RAIL_CORNER = 5;


// =====================================================
// MarioTrain Mode 角色圖資設定
// 來源：KartMode.ino / Kart.h 的 KART_MARIO、KART_LUIGI ...
// MarioTrainMode() 會沿用 TrainMode 的軌道、抽籤、移動、鏡頭邏輯，
// 只把三節火車替換成隨機三位賽車角色。
// =====================================================
static const uint16_t MARIO_TRAIN_TRANSPARENT_COLOR = 0x1c27;

struct MarioTrainDriverDef {
  const uint8_t* sheet;
  const uint16_t* palette;
  int sheetW;
  int frameW;
  int frameH;
};

static const MarioTrainDriverDef MARIO_TRAIN_DRIVER_MARIO = {
  KART_MARIO, KART_MARIO_PALETTE, 300, 25, 25
};

static const MarioTrainDriverDef MARIO_TRAIN_DRIVER_LUIGI = {
  KART_LUIGI, KART_LUIGI_PALETTE, 300, 25, 26
};

static const MarioTrainDriverDef MARIO_TRAIN_DRIVER_YOSHI = {
  KART_YOSHI, KART_YOSHI_PALETTE, 300, 25, 26
};

static const MarioTrainDriverDef MARIO_TRAIN_DRIVER_TODE = {
  KART_TODE, KART_TODE_PALETTE, 300, 25, 24
};

static const MarioTrainDriverDef MARIO_TRAIN_DRIVER_CUBA = {
  KART_CUBA, KART_CUBA_PALETTE, 300, 25, 27
};

static const MarioTrainDriverDef MARIO_TRAIN_DRIVER_KONG = {
  KART_KONG, KART_KONG_PALETTE, 312, 26, 26
};

static const MarioTrainDriverDef* const MARIO_TRAIN_ALL_DRIVERS[] = {
  &MARIO_TRAIN_DRIVER_MARIO,
  &MARIO_TRAIN_DRIVER_LUIGI,
  &MARIO_TRAIN_DRIVER_YOSHI,
  &MARIO_TRAIN_DRIVER_TODE,
  &MARIO_TRAIN_DRIVER_CUBA,
  &MARIO_TRAIN_DRIVER_KONG
};

static const uint8_t MARIO_TRAIN_DRIVER_COUNT =
  sizeof(MARIO_TRAIN_ALL_DRIVERS) / sizeof(MARIO_TRAIN_ALL_DRIVERS[0]);

static const MarioTrainDriverDef* marioTrainDrivers[3] = {
  &MARIO_TRAIN_DRIVER_MARIO,
  &MARIO_TRAIN_DRIVER_LUIGI,
  &MARIO_TRAIN_DRIVER_YOSHI
};

static uint8_t marioTrainDriverIndexes[3] = {0, 1, 2};

// 角色圖資比 16x16 火車大，因此用較大的間距比較不會互相蓋住。
// 如果想更像原本三節車廂，可改回 TRAIN_CAR_GAP。
static const float MARIO_TRAIN_DRIVER_GAP = 24.0f;

// 角色貼齊軌道用偏移。
// 如果角色太高 / 太低，調這兩個值。
static const int MARIO_TRAIN_DRIVER_OFFSET_X = 0;
static const int MARIO_TRAIN_DRIVER_OFFSET_Y = -2;


// =====================================================
// MarioTrain Mode：其他角色 NPC 獨立移動設定
//
// slot 0 = 主角，沿用 TrainMode 原本邏輯
// slot 1 / slot 2 = NPC，各自隨機城市移動
// =====================================================
static const unsigned long MARIO_TRAIN_NPC_WAIT_MIN_MS = 2UL * 60UL * 1000UL;
static const unsigned long MARIO_TRAIN_NPC_WAIT_MAX_MS = 3UL * 60UL * 1000UL;

static const uint8_t MARIO_TRAIN_ACTOR_WAITING = 0;
static const uint8_t MARIO_TRAIN_ACTOR_MOVING  = 1;

struct MarioTrainActor {
  float distance;
  uint8_t currentStationIndex;
  uint8_t targetStationIndex;
  int8_t moveDir;
  uint8_t state;
  unsigned long nextMoveMs;
};

// 只用 slot 1 / slot 2 給 NPC。
// slot 0 主角仍然使用 trainHeadDistance / trainRunState。
static MarioTrainActor marioTrainActors[3];

// TrainMode 的反向修正是給三節火車用的。
// MarioTrainMode 主角是單一角色，不需要用舊車尾位置修正。
static bool trainReverseUseBodyGapFix = true;


// =====================================================
// 可調整參數
// =====================================================


// 火車移動速度，數字越大越快
static const float TRAIN_SPEED = 0.8f;

// 每節車廂間距，建議維持 16，因為車廂圖資是 16x16
static const float TRAIN_CAR_GAP = 16.0f;

// 火車圖資如果看起來沒有貼齊軌道，可調這裡
// 水平移動時的火車偏移
static const int TRAIN_H_OFFSET_X = 0;
static const int TRAIN_H_OFFSET_Y = 0;

// 垂直移動時的火車偏移
static const int TRAIN_V_OFFSET_X = 0;
static const int TRAIN_V_OFFSET_Y = 0;

// 鏡頭 Y 偏移
static const int TRAIN_CAMERA_OFFSET_Y = 16;


// =====================================================
// 隨機目的地 / 抽籤動畫設定
// =====================================================

// 每隔多久抽取下一個目的地
// 5UL * 60UL * 1000UL = 5 分鐘
static const unsigned long TRAIN_DRAW_INTERVAL_MS = 5UL * 60UL * 1000UL;

// 進入 TrainMode 後第一次抽籤前等待多久
// 測試時可用 1000UL；如果想第一次也等 5 分鐘，改成 TRAIN_DRAW_INTERVAL_MS
static const unsigned long TRAIN_FIRST_DRAW_DELAY_MS = 1000UL;

// 左下角快速亂數顯示地名時間
static const unsigned long TRAIN_LOTTERY_RANDOM_MS = 5000UL;

// 快速亂數後，慢慢減速決定目的地的時間
static const unsigned long TRAIN_LOTTERY_SLOW_MS = 2500UL;

// 快速亂數階段，每隔幾 ms 換一次地名
static const unsigned long TRAIN_LOTTERY_FAST_STEP_MS = 80UL;

// 慢速決定階段，切換地名的最小 / 最大間隔
static const unsigned long TRAIN_LOTTERY_SLOW_STEP_MIN_MS = 160UL;
static const unsigned long TRAIN_LOTTERY_SLOW_STEP_MAX_MS = 650UL;

// 目的地確定後閃爍次數
static const uint8_t TRAIN_TARGET_BLINK_COUNT = 3;

// 目的地閃爍速度
static const unsigned long TRAIN_TARGET_BLINK_HALF_MS = 250UL;

// 抽籤動畫顯示位置
static const int TRAIN_LOTTERY_PANEL_X = 16;
static const int TRAIN_LOTTERY_PANEL_Y = 21;

// 抽籤動畫底色
static const uint16_t TRAIN_LOTTERY_PANEL_BG = hsv2rgb(hue, saturation, value);

// =====================================================
// 背景海洋 / 陸地 Tile Map 設定
// =====================================================

// 背景地圖每格大小，因為你的圖是 16x16 方塊，所以用 16
static const int TRAIN_BG_TILE_SIZE = 16;

// 368 / 16 = 23
static const int TRAIN_BG_MAP_W = 23;

// 704 / 16 = 44
static const int TRAIN_BG_MAP_H = 44;

// 海洋顏色
static const uint16_t TRAIN_OCEAN_COLOR = 0x0293;

// 陸地顏色，目前沿用你原本的背景色
static const uint16_t TRAIN_LAND_COLOR = 0x7732;

// 背景底色
// 0 的區域會先用這個顏色填滿
static const uint16_t TRAIN_BG_COLOR = 0x7732;


// =====================================================
// 方向常數
// =====================================================
static const uint8_t TRAIN_FACE_RIGHT = 0;
static const uint8_t TRAIN_FACE_DOWN  = 1;
static const uint8_t TRAIN_FACE_LEFT  = 2;
static const uint8_t TRAIN_FACE_UP    = 3;

// =====================================================
// TRAIN_CITY[] 順序
// =====================================================
static const uint8_t CITY_TAIPEI    = 0;
static const uint8_t CITY_XINBEI    = 1;
static const uint8_t CITY_TAOYUAN   = 2;
static const uint8_t CITY_HSINCHU   = 3;
static const uint8_t CITY_MIAOLI    = 4;
static const uint8_t CITY_TAICHUNG  = 5;
static const uint8_t CITY_CHANGHUA  = 6;
static const uint8_t CITY_YUNLIN    = 7;
static const uint8_t CITY_CHIAYI    = 8;
static const uint8_t CITY_TAINAN    = 9;
static const uint8_t CITY_KAOHSIUNG = 10;
static const uint8_t CITY_PINGTUNG  = 11;
static const uint8_t CITY_TAITUNG   = 12;
static const uint8_t CITY_HUALIEN   = 13;
static const uint8_t CITY_YILAN     = 14;
static const uint8_t CITY_KEELUNG   = 15;


// =====================================================
// 背景物件設定
//
// 新增背景物件的方法：
// 1. 在 Train.h 新增圖資，例如：T_TREE[]
// 2. 在下面新增一個物件類型，例如：TRAIN_OBJ_TREE
// 3. 在 TRAIN_BG_OBJECTS[] 新增座標
// 4. 在 drawTrainBgObjectByType() 加上 case
// =====================================================

// 背景物件類型
static const uint8_t TRAIN_OBJ_TP101 = 0;
static const uint8_t TRAIN_OBJ_GOD   = 1;
static const uint8_t TRAIN_OBJ_SHIP  = 2;
static const uint8_t TRAIN_OBJ_LHOUSE  = 3; //燈塔
static const uint8_t TRAIN_OBJ_GY  = 4;   //阿里山
static const uint8_t TRAIN_OBJ_GY2  = 5;
static const uint8_t TRAIN_OBJ_UL  = 6; //朝天宮
static const uint8_t TRAIN_OBJ_TN  = 7;  //安平古堡
static const uint8_t TRAIN_OBJ_EL  = 8;  // 龜山島
static const uint8_t TRAIN_OBJ_TU  = 9;   //機場
static const uint8_t TRAIN_OBJ_KS  = 10; // 85大樓
static const uint8_t TRAIN_OBJ_KS2  = 11; // 大樓

static const uint8_t TRAIN_OBJ_HL  = 12;
static const uint8_t TRAIN_OBJ_GH  = 13;
static const uint8_t TRAIN_OBJ_SHIP2  = 14;
static const uint8_t TRAIN_OBJ_SHIP3  = 15;

static const uint8_t TRAIN_OBJ_ML  = 16;
static const uint8_t TRAIN_OBJ_TG  = 17;
static const uint8_t TRAIN_OBJ_HL2  = 18;
static const uint8_t TRAIN_OBJ_TD  = 19;
static const uint8_t TRAIN_OBJ_TU2  = 20;
static const uint8_t TRAIN_OBJ_SB  = 21;


// 背景物件資料結構
// x, y 是世界座標，也就是地圖上的固定位置，不是螢幕座標
struct TrainBgObject {
  int16_t x;
  int16_t y;
  uint8_t type;
};

// 背景物件座標表
// 格式：{世界X, 世界Y, 物件類型}
static const TrainBgObject TRAIN_BG_OBJECTS[] PROGMEM = {
  {128,  32, TRAIN_OBJ_TP101},  // 台北 101，圖資 32x32
  {112, 320, TRAIN_OBJ_GOD},    // 神像/地標，圖資 32x32
  { 10, 514, TRAIN_OBJ_SHIP},    // 船，圖資 21x41
  { 272, 0, TRAIN_OBJ_LHOUSE},    // 燈塔，圖資 32x32
  { 128, 624, TRAIN_OBJ_LHOUSE},
  { 126, 416, TRAIN_OBJ_GY},
        { 48, 432, TRAIN_OBJ_GY2},
        { 64, 432, TRAIN_OBJ_GY2},
        { 80, 432, TRAIN_OBJ_GY2},
        { 96, 432, TRAIN_OBJ_GY2},
        { 112, 432, TRAIN_OBJ_GY2},
  { 32, 368, TRAIN_OBJ_UL}, 
  { 96, 464, TRAIN_OBJ_TN},
      
  { 304, 64, TRAIN_OBJ_EL}, 
   
  { 240, 128, TRAIN_OBJ_TU},  
  { 240, 112, TRAIN_OBJ_TU2},  
  { 160, 128, TRAIN_OBJ_TU2},  

  
  { 96, 512, TRAIN_OBJ_KS},  
        { 48, 528, TRAIN_OBJ_KS2},  
        { 64, 528, TRAIN_OBJ_KS2}, 
        { 80, 528, TRAIN_OBJ_KS2}, 

         { 128, 160, TRAIN_OBJ_KS2}, 
         { 128, 144, TRAIN_OBJ_KS2}, 
         { 144, 144, TRAIN_OBJ_KS2}, 
         { 160, 144, TRAIN_OBJ_KS2}, 
         { 112, 144, TRAIN_OBJ_KS2},
         { 112, 160, TRAIN_OBJ_KS2},

  { 304, 208, TRAIN_OBJ_HL},  
  { 304, 160, TRAIN_OBJ_HL}, 
  
  { 144, 352, TRAIN_OBJ_GH}, 

      { 208, 16, TRAIN_OBJ_SHIP2}, 
      { 32, 640, TRAIN_OBJ_SHIP2}, 
      { 80, 640, TRAIN_OBJ_SHIP2}, 
      { 272, 240, TRAIN_OBJ_SHIP2}, 

            { 16, 592, TRAIN_OBJ_SHIP3}, 
            { 16, 624, TRAIN_OBJ_SHIP3}, 
            { 160, 672, TRAIN_OBJ_SHIP3},
            { 192, 672, TRAIN_OBJ_SHIP3},
            { 306, 192, TRAIN_OBJ_SHIP3},

  { 80, 179, TRAIN_OBJ_ML}, 

  { 96, 272, TRAIN_OBJ_TG}, 
  { 32, 576, TRAIN_OBJ_TG}, 

  { 240, 192, TRAIN_OBJ_HL2},
  { 192, 448, TRAIN_OBJ_TD},
  
  { 195, 80, TRAIN_OBJ_SB},
  
};

static const uint8_t TRAIN_BG_OBJECT_COUNT =
  sizeof(TRAIN_BG_OBJECTS) / sizeof(TRAIN_BG_OBJECTS[0]);


// =====================================================
// 環島軌道座標
//
// 座標代表 16x16 軌道圖塊左上角。
// 最後一點會自動接回第 0 點。
// =====================================================
static const TrainRailPoint TRAIN_RAIL_POINTS[] PROGMEM = {
  {240,  32},  // 0  基隆
  {208,  32},
  {208,  64},
  {160,  64},  // 3  台北
  {128,  64},
  {128, 112},
  {160, 112},  // 6  新北
  {224, 112},
  {224, 144},
  {192, 144},  // 9  桃園
  {176, 144},
  {176, 176},
  {144, 176},  // 12 新竹
  {128, 176},
  {128, 208},
  { 96, 208},  // 15 苗栗
  { 80, 208},
  { 80, 240},
  {144, 240},
  {144, 256},
  {176, 256},
  {176, 288},
  {112, 288},  // 22 台中
  { 64, 288},
  { 64, 352},
  { 80, 352},  // 25 彰化
  {128, 352},
  {128, 400},
  { 80, 400},  // 28 雲林
  { 32, 400},
  { 32, 448},
  { 80, 448},  // 31 嘉義
  {128, 448},
  {128, 496},
  { 64, 496},  // 34 台南
  { 32, 496},
  { 32, 544},
  { 64, 544},  // 37 高雄
  {112, 544},
  {112, 592},
  { 64, 592},  // 40 屏東
  { 32, 592},
  { 32, 624},
  {112, 624},
  {112, 656},
  {192, 656},
  {192, 464},
  {240, 464},  // 47 台東
  {272, 464},
  {272, 320},
  {256, 320},
  {256, 256},
  {240, 256},
  {240, 224},
  {272, 224},  // 54 花蓮
  {288, 224},
  {288, 128},
  {320, 128},  // 57 宜蘭
  {336, 128},
  {336,  96},
  {288,  96},
  {288,  32}
};

static const uint16_t TRAIN_RAIL_COUNT =
  sizeof(TRAIN_RAIL_POINTS) / sizeof(TRAIN_RAIL_POINTS[0]);


// =====================================================
// 車站資料
//
// 格式：
// { railIndex, cityFrame, 站牌座標偏移X, 站牌座標偏移Y }
//
// railIndex：火車停靠的軌道點
// cityFrame：TRAIN_CITY[] 裡的站名編號
// 偏移X/Y：站牌相對於停靠點的顯示位置
// =====================================================
static const TrainStation TRAIN_STATIONS[] PROGMEM = {
  { 0,  CITY_KEELUNG,    0, -16},  // 基隆  x240 y16
  { 3,  CITY_TAIPEI,     0, -16},  // 台北  x160 y48
  { 6,  CITY_XINBEI,     0, -16},  // 新北  x160 y96
  { 9,  CITY_TAOYUAN,    0, -16},  // 桃園  x192 y128
  {12,  CITY_HSINCHU,    0, -16},  // 新竹  x144 y160
  {15,  CITY_MIAOLI,     0, -16},  // 苗栗  x96  y192
  {22,  CITY_TAICHUNG,   0, -16},  // 台中  x112 y272
  {25,  CITY_CHANGHUA,   0, -16},  // 彰化  x80  y336
  {28,  CITY_YUNLIN,     0, -16},  // 雲林  x80  y384
  {31,  CITY_CHIAYI,     0, -16},  // 嘉義  x80  y432
  {34,  CITY_TAINAN,     0, -16},  // 台南  x64  y480
  {37,  CITY_KAOHSIUNG,  0, -16},  // 高雄  x64  y528
  {40,  CITY_PINGTUNG,   0, -16},  // 屏東  x64  y576
  {47,  CITY_TAITUNG,    0, -16},  // 台東  x224 y448
  {54,  CITY_HUALIEN,  -16, -16},  // 花蓮  x256 y208
  {57,  CITY_YILAN,    -16, -16}   // 宜蘭  x304 y112
};

static const uint8_t TRAIN_STATION_COUNT =
  sizeof(TRAIN_STATIONS) / sizeof(TRAIN_STATIONS[0]);


// =====================================================
// 火車狀態
// =====================================================
static float trainHeadDistance = 0.0f;
static float trainTotalDistance = 1.0f;

// 運行狀態
static const uint8_t TRAIN_STATE_WAIT_LOTTERY = 0;  // 停在目前車站，等待下一次抽籤
static const uint8_t TRAIN_STATE_LOTTERY      = 1;  // 左下角播放抽籤動畫
static const uint8_t TRAIN_STATE_BLINK_TARGET = 2;  // 目的地確定後閃爍
static const uint8_t TRAIN_STATE_MOVING       = 3;  // 火車前往目標車站

static uint8_t trainRunState = TRAIN_STATE_WAIT_LOTTERY;

static uint8_t trainCurrentStationIndex = 1;  // 目前所在車站，預設台北
static uint8_t trainTargetStationIndex = 2;   // 抽籤後的目標車站

// 1 = 順時針前進，-1 = 逆時針前進
// 抽到目的地後會自動選比較近的方向
static int8_t trainMoveDir = 1;
// 抽籤決定的下一次行進方向
// 不要在抽籤階段直接改 trainMoveDir，否則車廂排列會立刻跳位
static int8_t trainNextMoveDir = 1;

// 下次開始抽籤的時間點
static unsigned long trainNextLotteryMs = 0;

// 目前狀態開始時間
static unsigned long trainStateStartMs = 0;

// 抽籤動畫目前顯示哪一個地名
static uint8_t trainLotteryDisplayStationIndex = 1;

// 抽籤動畫切換用
static unsigned long trainLotteryLastStepMs = 0;

// 目的地閃爍控制
static bool trainLotteryPanelVisible = false;
static bool trainLotteryBlinkVisible = true;
static uint8_t trainLotteryBlinkHalfCycles = 0;
static unsigned long trainLotteryBlinkLastMs = 0;

static bool trainRandomSeeded = false;

static int trainCameraX = 0;
static int trainCameraY = 0;


// =====================================================
// 小工具
// =====================================================
static int trainClampI(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static int trainSignI(int v) {
  if (v > 0) return 1;
  if (v < 0) return -1;
  return 0;
}

static float trainWrapDistance(float d) {
  while (d < 0.0f) d += trainTotalDistance;
  while (d >= trainTotalDistance) d -= trainTotalDistance;
  return d;
}

static int16_t trainRailX(uint16_t index) {
  if (index >= TRAIN_RAIL_COUNT) index = 0;
  return (int16_t)pgm_read_word(&(TRAIN_RAIL_POINTS[index].x));
}

static int16_t trainRailY(uint16_t index) {
  if (index >= TRAIN_RAIL_COUNT) index = 0;
  return (int16_t)pgm_read_word(&(TRAIN_RAIL_POINTS[index].y));
}

static uint16_t trainNextRailIndex(uint16_t index) {
  index++;
  if (index >= TRAIN_RAIL_COUNT) index = 0;
  return index;
}

static uint16_t trainPrevRailIndex(uint16_t index) {
  if (index == 0) return TRAIN_RAIL_COUNT - 1;
  return index - 1;
}

static uint16_t trainStationRailIndex(uint8_t stationIndex) {
  if (stationIndex >= TRAIN_STATION_COUNT) stationIndex = 0;
  return pgm_read_word(&(TRAIN_STATIONS[stationIndex].railIndex));
}

static uint8_t trainStationCityFrame(uint8_t stationIndex) {
  if (stationIndex >= TRAIN_STATION_COUNT) stationIndex = 0;
  return pgm_read_byte(&(TRAIN_STATIONS[stationIndex].cityFrame));
}

static int8_t trainStationOffsetX(uint8_t stationIndex) {
  if (stationIndex >= TRAIN_STATION_COUNT) stationIndex = 0;
  return pgm_read_byte(&(TRAIN_STATIONS[stationIndex].labelOffsetX));
}

static int8_t trainStationOffsetY(uint8_t stationIndex) {
  if (stationIndex >= TRAIN_STATION_COUNT) stationIndex = 0;
  return pgm_read_byte(&(TRAIN_STATIONS[stationIndex].labelOffsetY));
}

static int trainSegmentLength(uint16_t index) {
  uint16_t next = trainNextRailIndex(index);

  int ax = trainRailX(index);
  int ay = trainRailY(index);
  int bx = trainRailX(next);
  int by = trainRailY(next);

  return abs(bx - ax) + abs(by - ay);
}

static float trainDistanceAtRailIndex(uint16_t railIndex) {
  float d = 0.0f;

  if (railIndex >= TRAIN_RAIL_COUNT) railIndex = 0;

  for (uint16_t i = 0; i < railIndex; i++) {
    d += (float)trainSegmentLength(i);
  }

  return d;
}

static float trainForwardDistance(float fromD, float toD) {
  if (toD >= fromD) return toD - fromD;
  return trainTotalDistance - fromD + toD;
}

static void trainCalcTotalDistance() {
  trainTotalDistance = 0.0f;

  for (uint16_t i = 0; i < TRAIN_RAIL_COUNT; i++) {
    trainTotalDistance += (float)trainSegmentLength(i);
  }

  if (trainTotalDistance < 1.0f) trainTotalDistance = 1.0f;
}


// =====================================================
// 方向 / 抽籤小工具
// =====================================================
static uint8_t trainOppositeFace(uint8_t face) {
  if (face == TRAIN_FACE_RIGHT) return TRAIN_FACE_LEFT;
  if (face == TRAIN_FACE_LEFT) return TRAIN_FACE_RIGHT;
  if (face == TRAIN_FACE_DOWN) return TRAIN_FACE_UP;
  return TRAIN_FACE_DOWN;
}

static bool trainTimeReached(unsigned long nowMs, unsigned long targetMs) {
  return ((long)(nowMs - targetMs) >= 0);
}

static uint8_t trainRandomStationExceptCurrent() {
  if (TRAIN_STATION_COUNT <= 1) return trainCurrentStationIndex;

  uint8_t stationIndex;

  do {
    stationIndex = (uint8_t)random(0, TRAIN_STATION_COUNT);
  } while (stationIndex == trainCurrentStationIndex);

  return stationIndex;
}

static void trainChooseRandomTargetStation() {
  trainTargetStationIndex = trainRandomStationExceptCurrent();

  uint16_t targetRailIndex = trainStationRailIndex(trainTargetStationIndex);
  float targetDistance = trainDistanceAtRailIndex(targetRailIndex);

  float forwardDistance = trainForwardDistance(trainHeadDistance, targetDistance);
  float backwardDistance = trainForwardDistance(targetDistance, trainHeadDistance);

if (forwardDistance <= backwardDistance) {
  trainNextMoveDir = 1;
} else {
  trainNextMoveDir = -1;
}
}

static void trainStartLottery(unsigned long nowMs) {
  trainChooseRandomTargetStation();

  trainRunState = TRAIN_STATE_LOTTERY;
  trainStateStartMs = nowMs;
  trainLotteryLastStepMs = 0;
  trainLotteryDisplayStationIndex = trainRandomStationExceptCurrent();
  trainLotteryPanelVisible = true;
  trainLotteryBlinkVisible = true;
  trainLotteryBlinkHalfCycles = 0;
  trainLotteryBlinkLastMs = nowMs;
}

static void trainStartTargetBlink(unsigned long nowMs) {
  trainRunState = TRAIN_STATE_BLINK_TARGET;
  trainStateStartMs = nowMs;
  trainLotteryDisplayStationIndex = trainTargetStationIndex;
  trainLotteryPanelVisible = true;
  trainLotteryBlinkVisible = true;
  trainLotteryBlinkHalfCycles = 0;
  trainLotteryBlinkLastMs = nowMs;
}

static void trainStartMovingToTarget() {
  // TrainMode 原本是三節火車，所以反向時要把車頭換到舊車尾位置。
  // MarioTrainMode 主角是單一角色，所以不需要這個修正。
  if (trainReverseUseBodyGapFix && trainNextMoveDir != trainMoveDir) {
    float oldTailD = trainWrapDistance(
      trainHeadDistance - ((float)trainMoveDir * TRAIN_CAR_GAP * 2.0f)
    );

    trainHeadDistance = oldTailD;
  }

  trainMoveDir = trainNextMoveDir;

  trainRunState = TRAIN_STATE_MOVING;
  trainLotteryPanelVisible = false;
  trainLotteryBlinkVisible = true;
}



// =====================================================
// 由路徑距離取得座標與方向
//
// 每一節車廂都用自己的 distance 去查目前位置與方向。
// =====================================================
static void trainPositionAtDistance(
  float distanceValue,
  int *outX,
  int *outY,
  uint8_t *outFace
) {
  distanceValue = trainWrapDistance(distanceValue);

  float remain = distanceValue;

  for (uint16_t i = 0; i < TRAIN_RAIL_COUNT; i++) {
    uint16_t next = trainNextRailIndex(i);

    int ax = trainRailX(i);
    int ay = trainRailY(i);
    int bx = trainRailX(next);
    int by = trainRailY(next);

    int len = abs(bx - ax) + abs(by - ay);

    if (len <= 0) continue;

    if (remain <= (float)len) {
      int sx = trainSignI(bx - ax);
      int sy = trainSignI(by - ay);

      *outX = ax + (int)(sx * remain);
      *outY = ay + (int)(sy * remain);

      if (sx > 0) {
        *outFace = TRAIN_FACE_RIGHT;
      } else if (sx < 0) {
        *outFace = TRAIN_FACE_LEFT;
      } else if (sy > 0) {
        *outFace = TRAIN_FACE_DOWN;
      } else {
        *outFace = TRAIN_FACE_UP;
      }

      return;
    }

    remain -= (float)len;
  }

  *outX = trainRailX(0);
  *outY = trainRailY(0);
  *outFace = TRAIN_FACE_RIGHT;
}


// =====================================================
// 通用 RGB565 圖片繪製函式
//
// bitmap：圖資陣列
// bmpW：圖片寬度
// bmpH：圖片高度
// screenX / screenY：螢幕座標
//
// 透明色固定使用 TRAIN_TRANSPARENT_COLOR，也就是 0xDB84。
// =====================================================
//palette 讀色小工具
static inline uint16_t trainReadPaletteColor(
  const uint8_t* bitmap,
  const uint16_t* palette,
  uint32_t pixelPos
) {
  uint8_t colorIndex = pgm_read_byte(&(bitmap[pixelPos]));
  return pgm_read_word(&(palette[colorIndex]));
}


static void drawTrainBitmapIndexed(
  const uint8_t *bitmap,
  const uint16_t *palette,
  int bmpW,
  int bmpH,
  int screenX,
  int screenY
) {
  if (screenX <= -bmpW || screenX >= TRAIN_SCREEN_W ||
      screenY <= -bmpH || screenY >= TRAIN_SCREEN_H) {
    return;
  }

  for (int y = 0; y < bmpH; y++) {
    int drawY = screenY + y;
    if (drawY < 0 || drawY >= TRAIN_SCREEN_H) continue;

    for (int x = 0; x < bmpW; x++) {
      int drawX = screenX + x;
      if (drawX < 0 || drawX >= TRAIN_SCREEN_W) continue;

      uint32_t pixelPos =
        (uint32_t)y * (uint32_t)bmpW +
        (uint32_t)x;

      uint16_t color =
        trainReadPaletteColor(bitmap, palette, pixelPos);

      if (color == TRAIN_TRANSPARENT_COLOR) continue;

      display.drawPixel(drawX, drawY, color);
    }
  }
}


// =====================================================
// 畫 TRAIN[] 的 16x16 frame
// =====================================================
static void drawTrainFrame16(
  uint8_t frameIndex,
  int screenX,
  int screenY,
  uint8_t rotate90,
  bool mirrorX,
  bool mirrorY
) {
  if (screenX <= -TRAIN_TILE_W || screenX >= TRAIN_SCREEN_W ||
      screenY <= -TRAIN_TILE_H || screenY >= TRAIN_SCREEN_H) {
    return;
  }

  rotate90 &= 3;

  int frameStartX = frameIndex * TRAIN_TILE_W;

  for (int dy = 0; dy < TRAIN_TILE_H; dy++) {
    int drawY = screenY + dy;
    if (drawY < 0 || drawY >= TRAIN_SCREEN_H) continue;

    for (int dx = 0; dx < TRAIN_TILE_W; dx++) {
      int drawX = screenX + dx;
      if (drawX < 0 || drawX >= TRAIN_SCREEN_W) continue;

      int srcX;
      int srcY;

      if (rotate90 == 0) {
        srcX = dx;
        srcY = dy;
      } else if (rotate90 == 1) {
        srcX = dy;
        srcY = TRAIN_TILE_W - 1 - dx;
      } else if (rotate90 == 2) {
        srcX = TRAIN_TILE_W - 1 - dx;
        srcY = TRAIN_TILE_H - 1 - dy;
      } else {
        srcX = TRAIN_TILE_H - 1 - dy;
        srcY = dx;
      }

      if (mirrorX) srcX = TRAIN_TILE_W - 1 - srcX;
      if (mirrorY) srcY = TRAIN_TILE_H - 1 - srcY;

      uint32_t pixelPos =
        (uint32_t)srcY * (uint32_t)TRAIN_SHEET_W +
        (uint32_t)frameStartX +
        (uint32_t)srcX;

      uint16_t color =
        trainReadPaletteColor(TRAIN, TRAIN_PALETTE, pixelPos);

      if (color == TRAIN_TRANSPARENT_COLOR) continue;

      display.drawPixel(drawX, drawY, color);
    }
  }
}


// =====================================================
// 畫 TRAIN_CITY[] 的 32x16 frame
// =====================================================
static void drawTrainCityFrame32(
  uint8_t frameIndex,
  int screenX,
  int screenY
) {
  if (screenX <= -TRAIN_CITY_W || screenX >= TRAIN_SCREEN_W ||
      screenY <= -TRAIN_CITY_H || screenY >= TRAIN_SCREEN_H) {
    return;
  }

  int frameStartX = frameIndex * TRAIN_CITY_W;

  for (int y = 0; y < TRAIN_CITY_H; y++) {
    int drawY = screenY + y;
    if (drawY < 0 || drawY >= TRAIN_SCREEN_H) continue;

    for (int x = 0; x < TRAIN_CITY_W; x++) {
      int drawX = screenX + x;
      if (drawX < 0 || drawX >= TRAIN_SCREEN_W) continue;

      uint32_t pixelPos =
        (uint32_t)y * (uint32_t)TRAIN_CITY_SHEET_W +
        (uint32_t)frameStartX +
        (uint32_t)x;

      uint16_t color =
        trainReadPaletteColor(TRAIN_CITY, TRAIN_CITY_PALETTE, pixelPos);

      if (color == TRAIN_TRANSPARENT_COLOR) continue;

      display.drawPixel(drawX, drawY, color);
    }
  }
}



static void trainFillRectClip(int x, int y, int w, int h, uint16_t color) {
  if (x >= TRAIN_SCREEN_W || y >= TRAIN_SCREEN_H) return;
  if (x + w <= 0 || y + h <= 0) return;

  if (x < 0) {
    w += x;
    x = 0;
  }

  if (y < 0) {
    h += y;
    y = 0;
  }

  if (x + w > TRAIN_SCREEN_W) {
    w = TRAIN_SCREEN_W - x;
  }

  if (y + h > TRAIN_SCREEN_H) {
    h = TRAIN_SCREEN_H - y;
  }

  if (w <= 0 || h <= 0) return;

  display.fillRect(x, y, w, h, color);
}



// =====================================================
// 畫海洋 / 陸地大地圖
//
// 只畫目前鏡頭看到的 tile，速度比整張圖快很多。
// =====================================================
static void drawTrainBackground() {
  // 先用預設陸地底色鋪滿
  display.fillScreen(TRAIN_BG_COLOR);

  int startTileX = trainCameraX / TRAIN_BG_TILE_SIZE;
  int startTileY = trainCameraY / TRAIN_BG_TILE_SIZE;

  int offsetX = trainCameraX % TRAIN_BG_TILE_SIZE;
  int offsetY = trainCameraY % TRAIN_BG_TILE_SIZE;

  int tilesX = (TRAIN_SCREEN_W / TRAIN_BG_TILE_SIZE) + 2;
  int tilesY = (TRAIN_SCREEN_H / TRAIN_BG_TILE_SIZE) + 2;

  for (int ty = 0; ty < tilesY; ty++) {
    int mapY = startTileY + ty;
    if (mapY < 0 || mapY >= TRAIN_BG_MAP_H) continue;

    for (int tx = 0; tx < tilesX; tx++) {
      int mapX = startTileX + tx;
      if (mapX < 0 || mapX >= TRAIN_BG_MAP_W) continue;

      uint8_t tile = pgm_read_byte(&(TRAIN_BG_MAP[mapY][mapX]));

      // 0 = 預設背景，不用畫，因為前面 fillScreen 已經畫好了
      if (tile == 0) continue;

      uint16_t color;

      if (tile == 1) {
        color = TRAIN_OCEAN_COLOR;
      } else {
        color = TRAIN_LAND_COLOR;
      }

      int screenX = tx * TRAIN_BG_TILE_SIZE - offsetX;
      int screenY = ty * TRAIN_BG_TILE_SIZE - offsetY;

      trainFillRectClip(
        screenX,
        screenY,
        TRAIN_BG_TILE_SIZE,
        TRAIN_BG_TILE_SIZE,
        color
      );
    }
  }
}

// =====================================================
// 依照背景物件類型畫圖
//
// 之後新增物件時，在這裡加 case。
// =====================================================
static void drawTrainBgObjectByType(uint8_t type, int screenX, int screenY) {
  switch (type) {
    case TRAIN_OBJ_TP101:
      drawTrainBitmapIndexed(T_TP101, T_TP101_PALETTE, 32, 32, screenX, screenY);
      break;

    case TRAIN_OBJ_GOD:
      drawTrainBitmapIndexed(T_GOD, T_GOD_PALETTE, 32, 32, screenX, screenY);
      break;

    case TRAIN_OBJ_SHIP:
      drawTrainBitmapIndexed(T_SHIP, T_SHIP_PALETTE, 21, 41, screenX, screenY);
      break;

    case TRAIN_OBJ_LHOUSE:
      drawTrainBitmapIndexed(T_LHOUSE, T_LHOUSE_PALETTE, 32, 32, screenX, screenY);
      break;

    case TRAIN_OBJ_GY:
      drawTrainBitmapIndexed(T_GY, T_GY_PALETTE, 48, 32, screenX, screenY);
      break;

    case TRAIN_OBJ_GY2:
      drawTrainBitmapIndexed(T_GY2, T_GY2_PALETTE, 16, 16, screenX, screenY);
      break;

    case TRAIN_OBJ_UL:
      drawTrainBitmapIndexed(T_UL, T_UL_PALETTE, 48, 32, screenX, screenY);
      break;

    case TRAIN_OBJ_TN:
      drawTrainBitmapIndexed(T_TN, T_TN_PALETTE, 32, 32, screenX, screenY);
      break;

    case TRAIN_OBJ_EL:
      drawTrainBitmapIndexed(T_EL, T_EL_PALETTE, 48, 32, screenX, screenY);
      break;

    case TRAIN_OBJ_TU:
      drawTrainBitmapIndexed(T_TU, T_TU_PALETTE, 32, 32, screenX, screenY);
      break;

    case TRAIN_OBJ_KS:
      drawTrainBitmapIndexed(T_KS, T_KS_PALETTE, 32, 32, screenX, screenY);
      break;

    case TRAIN_OBJ_KS2:
      drawTrainBitmapIndexed(T_KS2, T_KS2_PALETTE, 16, 16, screenX, screenY);
      break;

    case TRAIN_OBJ_HL:
      drawTrainBitmapIndexed(T_HL, T_HL_PALETTE, 25, 32, screenX, screenY);
      break;

    case TRAIN_OBJ_GH:
      drawTrainBitmapIndexed(T_GH, T_GH_PALETTE, 32, 32, screenX, screenY);
      break;

    case TRAIN_OBJ_SHIP2:
      drawTrainBitmapIndexed(T_SHIP2, T_SHIP2_PALETTE, 32, 16, screenX, screenY);
      break;

    case TRAIN_OBJ_SHIP3:
      drawTrainBitmapIndexed(T_SHIP3, T_SHIP3_PALETTE, 16, 16, screenX, screenY);
      break;

    case TRAIN_OBJ_ML:
      drawTrainBitmapIndexed(T_ML, T_ML_PALETTE, 48, 29, screenX, screenY);
      break;

    case TRAIN_OBJ_TG:
      drawTrainBitmapIndexed(T_TG, T_TG_PALETTE, 80, 16, screenX, screenY);
      break;

    case TRAIN_OBJ_HL2:
      drawTrainBitmapIndexed(T_HL2, T_HL2_PALETTE, 48, 32, screenX, screenY);
      break;

    case TRAIN_OBJ_TD:
      drawTrainBitmapIndexed(T_TD, T_TD_PALETTE, 48, 16, screenX, screenY);
      break;

    case TRAIN_OBJ_TU2:
      drawTrainBitmapIndexed(T_TU2, T_TU2_PALETTE, 32, 16, screenX, screenY);
      break;

    case TRAIN_OBJ_SB:
      drawTrainBitmapIndexed(T_SB, T_SB_PALETTE, 23, 32, screenX, screenY);
      break;
  }
}


// =====================================================
// 畫所有背景物件
//
// 背景物件會跟著地圖座標走。
// 也就是火車移動時，鏡頭移動，地標會從螢幕滑過。
// =====================================================
static void drawTrainBgObjects() {
  for (uint8_t i = 0; i < TRAIN_BG_OBJECT_COUNT; i++) {
    TrainBgObject obj;
    memcpy_P(&obj, &TRAIN_BG_OBJECTS[i], sizeof(TrainBgObject));

    int screenX = obj.x - trainCameraX;
    int screenY = obj.y - trainCameraY;

    drawTrainBgObjectByType(obj.type, screenX, screenY);
  }
}


// =====================================================
// 判斷某個點是不是轉角
// =====================================================
static bool trainIsCornerIndex(uint16_t index) {
  uint16_t prevIndex = trainPrevRailIndex(index);
  uint16_t nextIndex = trainNextRailIndex(index);

  int px = trainRailX(prevIndex);
  int py = trainRailY(prevIndex);
  int cx = trainRailX(index);
  int cy = trainRailY(index);
  int nx = trainRailX(nextIndex);
  int ny = trainRailY(nextIndex);

  int dx1 = cx - px;
  int dy1 = cy - py;
  int dx2 = nx - cx;
  int dy2 = ny - cy;

  bool horizontal1 = abs(dx1) > abs(dy1);
  bool horizontal2 = abs(dx2) > abs(dy2);

  return horizontal1 != horizontal2;
}


// =====================================================
// 畫軌道直線段
// 直線會跳過轉角格，避免直線軌道與轉角軌道重疊
// =====================================================
static void drawTrainRailSegmentByIndex(uint16_t index) {
  uint16_t nextIndex = trainNextRailIndex(index);

  int ax = trainRailX(index);
  int ay = trainRailY(index);
  int bx = trainRailX(nextIndex);
  int by = trainRailY(nextIndex);

  if (ax == bx && ay == by) return;

  bool skipStart = trainIsCornerIndex(index);
  bool skipEnd   = trainIsCornerIndex(nextIndex);

  if (ay == by) {
    int dir = trainSignI(bx - ax);
    if (dir == 0) return;

    int startX = ax;
    int endX = bx;

    if (skipStart) startX += dir * TRAIN_TILE_W;
    if (skipEnd)   endX   -= dir * TRAIN_TILE_W;

    if ((dir > 0 && startX > endX) || (dir < 0 && startX < endX)) {
      return;
    }

    for (int x = startX; ; x += dir * TRAIN_TILE_W) {
      drawTrainFrame16(
        TRAIN_FRAME_RAIL_STRAIGHT,
        x - trainCameraX,
        ay - trainCameraY,
        0,
        false,
        false
      );

      if (x == endX) break;
    }

  } else if (ax == bx) {
    int dir = trainSignI(by - ay);
    if (dir == 0) return;

    int startY = ay;
    int endY = by;

    if (skipStart) startY += dir * TRAIN_TILE_H;
    if (skipEnd)   endY   -= dir * TRAIN_TILE_H;

    if ((dir > 0 && startY > endY) || (dir < 0 && startY < endY)) {
      return;
    }

    for (int y = startY; ; y += dir * TRAIN_TILE_H) {
      drawTrainFrame16(
        TRAIN_FRAME_RAIL_STRAIGHT,
        ax - trainCameraX,
        y - trainCameraY,
        1,
        false,
        false
      );

      if (y == endY) break;
    }
  }
}

static uint8_t trainDirFromTo(int ax, int ay, int bx, int by) {
  int dx = bx - ax;
  int dy = by - ay;

  if (abs(dx) >= abs(dy)) {
    return (dx >= 0) ? TRAIN_FACE_RIGHT : TRAIN_FACE_LEFT;
  }

  return (dy >= 0) ? TRAIN_FACE_DOWN : TRAIN_FACE_UP;
}


// =====================================================
// 畫轉角
//
// TRAIN[5] 原始方向視為：右 + 下。
// 其他方向用旋轉處理。
// =====================================================
static void drawTrainRailCornerAtIndex(uint16_t index) {
  uint16_t prevIndex = trainPrevRailIndex(index);
  uint16_t nextIndex = trainNextRailIndex(index);

  int px = trainRailX(prevIndex);
  int py = trainRailY(prevIndex);
  int cx = trainRailX(index);
  int cy = trainRailY(index);
  int nx = trainRailX(nextIndex);
  int ny = trainRailY(nextIndex);

  uint8_t d1 = trainDirFromTo(cx, cy, px, py);
  uint8_t d2 = trainDirFromTo(cx, cy, nx, ny);

  bool right = (d1 == TRAIN_FACE_RIGHT || d2 == TRAIN_FACE_RIGHT);
  bool down  = (d1 == TRAIN_FACE_DOWN  || d2 == TRAIN_FACE_DOWN);
  bool left  = (d1 == TRAIN_FACE_LEFT  || d2 == TRAIN_FACE_LEFT);
  bool up    = (d1 == TRAIN_FACE_UP    || d2 == TRAIN_FACE_UP);

  if ((right && left) || (up && down)) {
    return;
  }

  uint8_t rot = 0;

  if (right && down) {
    rot = 0;
  } else if (down && left) {
    rot = 1;
  } else if (left && up) {
    rot = 2;
  } else if (up && right) {
    rot = 3;
  } else {
    return;
  }

  drawTrainFrame16(
    TRAIN_FRAME_RAIL_CORNER,
    cx - trainCameraX,
    cy - trainCameraY,
    rot,
    false,
    false
  );
}


// =====================================================
// 畫整條軌道
// =====================================================
static void drawTrainRail() {
  for (uint16_t i = 0; i < TRAIN_RAIL_COUNT; i++) {
    drawTrainRailSegmentByIndex(i);
  }

  for (uint16_t i = 0; i < TRAIN_RAIL_COUNT; i++) {
    if (trainIsCornerIndex(i)) {
      drawTrainRailCornerAtIndex(i);
    }
  }
}


// =====================================================
// 畫所有車站名稱
// =====================================================
static void drawTrainStations() {
  for (uint8_t i = 0; i < TRAIN_STATION_COUNT; i++) {
    uint16_t railIndex = trainStationRailIndex(i);

    int px = trainRailX(railIndex);
    int py = trainRailY(railIndex);

    int labelX = px + trainStationOffsetX(i);
    int labelY = py + trainStationOffsetY(i);

    drawTrainCityFrame32(
      trainStationCityFrame(i),
      labelX - trainCameraX,
      labelY - trainCameraY
    );
  }
}


// =====================================================
// 畫單節車廂
//
// carRole:
// 0 = 車頭
// 1 = 中間車廂
// 2 = 車尾
// =====================================================
static void drawTrainCar(float carDistance, uint8_t carRole) {
  int worldX = 0;
  int worldY = 0;
  uint8_t face = TRAIN_FACE_RIGHT;

trainPositionAtDistance(carDistance, &worldX, &worldY, &face);

// 如果火車逆時針行駛，車廂面向要反過來
if (trainMoveDir < 0) {
  face = trainOppositeFace(face);
}

int screenX = worldX - trainCameraX;
  int screenY = worldY - trainCameraY;

  if (face == TRAIN_FACE_RIGHT || face == TRAIN_FACE_LEFT) {
    screenX += TRAIN_H_OFFSET_X;
    screenY += TRAIN_H_OFFSET_Y;

    if (carRole == 1) {
      drawTrainFrame16(
        TRAIN_FRAME_H_BODY_B,
        screenX,
        screenY,
        0,
        false,
        false
      );
    } else {
      bool mirrorX = false;

      if (carRole == 0) {
        // 車頭朝前
        mirrorX = (face == TRAIN_FACE_RIGHT);
      } else {
        // 車尾朝後
        mirrorX = (face == TRAIN_FACE_LEFT);
      }

      drawTrainFrame16(
        TRAIN_FRAME_H_BODY_A,
        screenX,
        screenY,
        0,
        mirrorX,
        false
      );
    }

  } else {
    screenX += TRAIN_V_OFFSET_X;
    screenY += TRAIN_V_OFFSET_Y;

    if (carRole == 1) {
      drawTrainFrame16(
        TRAIN_FRAME_V_BODY_B,
        screenX,
        screenY,
        0,
        false,
        false
      );
    } else {
      bool mirrorY = false;

      if (carRole == 0) {
        // 車頭朝前
        mirrorY = (face == TRAIN_FACE_UP);
      } else {
        // 車尾朝後
        mirrorY = (face == TRAIN_FACE_DOWN);
      }

      drawTrainFrame16(
        TRAIN_FRAME_V_BODY_A,
        screenX,
        screenY,
        0,
        false,
        mirrorY
      );
    }
  }
}


// =====================================================
// 畫三節車廂
// 每節車廂各自用不同 distance 查路徑，所以轉彎時會依序轉彎
// =====================================================
static void drawTrainBody() {
  float headD = trainHeadDistance;

  // 順時針：中節 / 尾節在車頭後方，所以是 -gap
  // 逆時針：中節 / 尾節在車頭後方，但距離方向相反，所以等於 +gap
  float midD  = trainWrapDistance(trainHeadDistance - ((float)trainMoveDir * TRAIN_CAR_GAP));
  float tailD = trainWrapDistance(trainHeadDistance - ((float)trainMoveDir * TRAIN_CAR_GAP * 2.0f));

  drawTrainCar(tailD, 2);
  drawTrainCar(midD,  1);
  drawTrainCar(headD, 0);
}


// =====================================================
// MarioTrain Mode：隨機選三位不重複角色
// =====================================================
static void marioTrainChooseRandomDrivers() {
  if (MARIO_TRAIN_DRIVER_COUNT <= 0) return;

  uint8_t used[3] = {0, 0, 0};

  used[0] = (uint8_t)random(0, MARIO_TRAIN_DRIVER_COUNT);

  if (MARIO_TRAIN_DRIVER_COUNT >= 2) {
    do {
      used[1] = (uint8_t)random(0, MARIO_TRAIN_DRIVER_COUNT);
    } while (used[1] == used[0]);
  } else {
    used[1] = used[0];
  }

  if (MARIO_TRAIN_DRIVER_COUNT >= 3) {
    do {
      used[2] = (uint8_t)random(0, MARIO_TRAIN_DRIVER_COUNT);
    } while (used[2] == used[0] || used[2] == used[1]);
  } else {
    used[2] = used[0];
  }

  for (uint8_t i = 0; i < 3; i++) {
    marioTrainDriverIndexes[i] = used[i];
    marioTrainDrivers[i] = MARIO_TRAIN_ALL_DRIVERS[used[i]];
  }
}


// =====================================================
// MarioTrain Mode：畫 Kart.h 的角色 frame
// slotIndex：0 = 主角，1 / 2 = 跟隨角色
// =====================================================
static void drawMarioTrainDriverFrame(
  uint8_t slotIndex,
  int frameIndex,
  int screenX,
  int screenY,
  bool mirrorX
) {
  if (slotIndex >= 3) return;
  if (frameIndex < 0) return;

  const MarioTrainDriverDef* driver = marioTrainDrivers[slotIndex];
  if (driver == NULL) return;

  if (screenX <= -driver->frameW || screenX >= TRAIN_SCREEN_W ||
      screenY <= -driver->frameH || screenY >= TRAIN_SCREEN_H) {
    return;
  }

  int frameStartX = frameIndex * driver->frameW;

  for (int y = 0; y < driver->frameH; y++) {
    int drawY = screenY + y;
    if (drawY < 0 || drawY >= TRAIN_SCREEN_H) continue;

    for (int x = 0; x < driver->frameW; x++) {
      int drawX = screenX + x;
      if (drawX < 0 || drawX >= TRAIN_SCREEN_W) continue;

      int srcX = mirrorX ? (driver->frameW - 1 - x) : x;

      uint32_t pixelPos =
        (uint32_t)y * (uint32_t)driver->sheetW +
        (uint32_t)frameStartX +
        (uint32_t)srcX;

      uint16_t color =
        trainReadPaletteColor(driver->sheet, driver->palette, pixelPos);

      if (color == MARIO_TRAIN_TRANSPARENT_COLOR) continue;

      display.drawPixel(drawX, drawY, color);
    }
  }
}


// =====================================================
// MarioTrain Mode：單一方向圖片設定
//
// 先不要動畫，只依照行走方向顯示固定 frame。
//
// 目前已知：
// 往上 = frame 0
// 往右 = frame 5
//
// 往左暫時使用 frame 5 + 水平翻轉。
// 往下暫時使用 frame 0。
// 之後你找到正確 frame，只要改下面四個數字。
// =====================================================
static const int MARIO_TRAIN_FRAME_UP    = 0;
static const int MARIO_TRAIN_FRAME_RIGHT = 6;
static const int MARIO_TRAIN_FRAME_LEFT  = 6;
static const int MARIO_TRAIN_FRAME_DOWN  = 10;

static int marioTrainFrameForFace(uint8_t face, uint8_t slotIndex) {
  if (face == TRAIN_FACE_UP) {
    return MARIO_TRAIN_FRAME_UP;
  }

  if (face == TRAIN_FACE_RIGHT) {
    return MARIO_TRAIN_FRAME_RIGHT;
  }

  if (face == TRAIN_FACE_LEFT) {
    return MARIO_TRAIN_FRAME_LEFT;
  }

  if (face == TRAIN_FACE_DOWN) {
    return MARIO_TRAIN_FRAME_DOWN;
  }

  return MARIO_TRAIN_FRAME_UP;
}

static bool marioTrainMirrorForFace(uint8_t face) {
  // 左邊先用右邊圖片水平翻轉
  return (face == TRAIN_FACE_LEFT);
}


// =====================================================
// MarioTrain Mode：畫單一角色
// =====================================================
static void drawMarioTrainDriverAtWorld(
  uint8_t slotIndex,
  int worldX,
  int worldY,
  uint8_t face
) {
  if (slotIndex >= 3) return;

  const MarioTrainDriverDef* driver = marioTrainDrivers[slotIndex];
  if (driver == NULL) return;

  int screenX =
    worldX + 8 - (driver->frameW / 2) - trainCameraX +
    MARIO_TRAIN_DRIVER_OFFSET_X;

  int screenY =
    worldY + 8 - (driver->frameH / 2) - trainCameraY +
    MARIO_TRAIN_DRIVER_OFFSET_Y;

  int frameIndex = marioTrainFrameForFace(face, slotIndex);
  bool mirrorX = marioTrainMirrorForFace(face);

  drawMarioTrainDriverFrame(
    slotIndex,
    frameIndex,
    screenX,
    screenY,
    mirrorX
  );
}

// =====================================================
// MarioTrain Mode：NPC 等待時間 2~3 分鐘
// =====================================================
static unsigned long marioTrainRandomNpcWaitMs() {
  return (unsigned long)random(
    (long)MARIO_TRAIN_NPC_WAIT_MIN_MS,
    (long)(MARIO_TRAIN_NPC_WAIT_MAX_MS + 1UL)
  );
}


// =====================================================
// MarioTrain Mode：隨機城市，但避開指定城市
// avoidA / avoidB 可傳 255 表示不避開
// =====================================================
static uint8_t marioTrainRandomStationAvoid(uint8_t avoidA, uint8_t avoidB) {
  if (TRAIN_STATION_COUNT <= 1) return 0;

  uint8_t stationIndex = 0;

  do {
    stationIndex = (uint8_t)random(0, TRAIN_STATION_COUNT);
  } while (
    stationIndex == avoidA ||
    stationIndex == avoidB
  );

  return stationIndex;
}


// =====================================================
// MarioTrain Mode：NPC 隨機選下一個目的地
// =====================================================
static void marioTrainNpcChooseTarget(uint8_t slotIndex) {
  if (slotIndex == 0 || slotIndex >= 3) return;

  MarioTrainActor *actor = &marioTrainActors[slotIndex];

  actor->targetStationIndex =
    marioTrainRandomStationAvoid(actor->currentStationIndex, 255);

  uint16_t targetRailIndex =
    trainStationRailIndex(actor->targetStationIndex);

  float targetDistance =
    trainDistanceAtRailIndex(targetRailIndex);

  float forwardDistance =
    trainForwardDistance(actor->distance, targetDistance);

  float backwardDistance =
    trainForwardDistance(targetDistance, actor->distance);

  if (forwardDistance <= backwardDistance) {
    actor->moveDir = 1;
  } else {
    actor->moveDir = -1;
  }

  actor->state = MARIO_TRAIN_ACTOR_MOVING;
}


// =====================================================
// MarioTrain Mode：初始化單一 NPC
// =====================================================
static void marioTrainInitNpcActor(
  uint8_t slotIndex,
  uint8_t stationIndex,
  unsigned long nowMs
) {
  if (slotIndex == 0 || slotIndex >= 3) return;

  MarioTrainActor *actor = &marioTrainActors[slotIndex];

  actor->currentStationIndex = stationIndex;
  actor->targetStationIndex = stationIndex;
  actor->moveDir = 1;
  actor->state = MARIO_TRAIN_ACTOR_WAITING;

  uint16_t railIndex = trainStationRailIndex(stationIndex);
  actor->distance = trainDistanceAtRailIndex(railIndex);

  actor->nextMoveMs = nowMs + marioTrainRandomNpcWaitMs();
}


// =====================================================
// MarioTrain Mode：初始化 2 個 NPC
//
// slot 1 / slot 2 會出現在隨機城市。
// 不會跟主角重複，也不會彼此重複。
// =====================================================
static void marioTrainInitNpcActors(unsigned long nowMs) {
  uint8_t mainStation = trainCurrentStationIndex;

  uint8_t npcStation1 =
    marioTrainRandomStationAvoid(mainStation, 255);

  uint8_t npcStation2 =
    marioTrainRandomStationAvoid(mainStation, npcStation1);

  marioTrainInitNpcActor(1, npcStation1, nowMs);
  marioTrainInitNpcActor(2, npcStation2, nowMs);
}


// =====================================================
// MarioTrain Mode：更新單一 NPC
// =====================================================
static void updateMarioTrainNpcActor(uint8_t slotIndex, unsigned long nowMs) {
  if (slotIndex == 0 || slotIndex >= 3) return;

  MarioTrainActor *actor = &marioTrainActors[slotIndex];

  if (actor->state == MARIO_TRAIN_ACTOR_WAITING) {
    if (trainTimeReached(nowMs, actor->nextMoveMs)) {
      marioTrainNpcChooseTarget(slotIndex);
    }
    return;
  }

  if (actor->state == MARIO_TRAIN_ACTOR_MOVING) {
    uint16_t targetRailIndex =
      trainStationRailIndex(actor->targetStationIndex);

    float targetDistance =
      trainDistanceAtRailIndex(targetRailIndex);

    float distanceToStop;

    if (actor->moveDir > 0) {
      distanceToStop =
        trainForwardDistance(actor->distance, targetDistance);
    } else {
      distanceToStop =
        trainForwardDistance(targetDistance, actor->distance);
    }

    if (distanceToStop <= TRAIN_SPEED) {
      actor->distance = targetDistance;
      actor->currentStationIndex = actor->targetStationIndex;
      actor->state = MARIO_TRAIN_ACTOR_WAITING;
      actor->nextMoveMs = nowMs + marioTrainRandomNpcWaitMs();
      return;
    }

    actor->distance =
      trainWrapDistance(actor->distance + (TRAIN_SPEED * actor->moveDir));
  }
}


// =====================================================
// MarioTrain Mode：更新所有 NPC
// =====================================================
static void updateMarioTrainNpcActors(unsigned long nowMs) {
  updateMarioTrainNpcActor(1, nowMs);
  updateMarioTrainNpcActor(2, nowMs);
}


// =====================================================
// MarioTrain Mode：畫三位角色
//
// slot 0 = 主角
//   使用 trainHeadDistance，camera 會鎖定它。
//
// slot 1 / slot 2 = NPC
//   使用 marioTrainActors[] 自己的 distance。
//   他們會各自停靠、等待、移動。
// =====================================================
static void drawMarioTrainBody() {
  int worldX[3];
  int worldY[3];
  uint8_t face[3];
  int footY[3];
  uint8_t drawOrder[3] = {0, 1, 2};

  for (uint8_t i = 0; i < 3; i++) {
    float actorDistance;
    int8_t actorMoveDir;

    if (i == 0) {
      // 主角：沿用原本 TrainMode 的位置
      actorDistance = trainHeadDistance;
      actorMoveDir = trainMoveDir;
    } else {
      // NPC：使用自己的位置
      actorDistance = marioTrainActors[i].distance;
      actorMoveDir = marioTrainActors[i].moveDir;
    }

    worldX[i] = 0;
    worldY[i] = 0;
    face[i] = TRAIN_FACE_RIGHT;

    trainPositionAtDistance(actorDistance, &worldX[i], &worldY[i], &face[i]);

    // 逆時針行駛時，角色面向要反過來
    if (actorMoveDir < 0) {
      face[i] = trainOppositeFace(face[i]);
    }

    const MarioTrainDriverDef* driver = marioTrainDrivers[i];
    footY[i] = worldY[i] + 8 + (driver ? (driver->frameH / 2) : 0);
  }

  // 依照腳底高度排序，避免上方角色蓋住下方角色。
  for (uint8_t i = 0; i < 2; i++) {
    for (uint8_t j = i + 1; j < 3; j++) {
      if (footY[drawOrder[i]] > footY[drawOrder[j]]) {
        uint8_t temp = drawOrder[i];
        drawOrder[i] = drawOrder[j];
        drawOrder[j] = temp;
      }
    }
  }

  for (uint8_t i = 0; i < 3; i++) {
    uint8_t idx = drawOrder[i];
    drawMarioTrainDriverAtWorld(idx, worldX[idx], worldY[idx], face[idx]);
  }
}

// =====================================================
// 左下角抽籤動畫顯示
// =====================================================
static void drawTrainLotteryPanel() {
  if (!trainLotteryPanelVisible) return;

  // 閃爍關閉時不畫地名
  if (trainRunState == TRAIN_STATE_BLINK_TARGET && !trainLotteryBlinkVisible) {
    return;
  }

  display.fillRect(
    TRAIN_LOTTERY_PANEL_X,
    TRAIN_LOTTERY_PANEL_Y,
    TRAIN_CITY_W,
    TRAIN_CITY_H,
    TRAIN_LOTTERY_PANEL_BG
  );

  drawTrainCityFrame32(
    trainStationCityFrame(trainLotteryDisplayStationIndex),
    TRAIN_LOTTERY_PANEL_X,
    TRAIN_LOTTERY_PANEL_Y
  );
}


// =====================================================
// 更新鏡頭：跟著車頭走
// =====================================================
static void updateTrainCamera() {
  int worldX = 0;
  int worldY = 0;
  uint8_t face = TRAIN_FACE_RIGHT;

  trainPositionAtDistance(trainHeadDistance, &worldX, &worldY, &face);

  int trainCenterX = worldX + 8;
  int trainCenterY = worldY + 8;

  trainCameraX = trainClampI(
    trainCenterX - (TRAIN_SCREEN_W / 2),
    0,
    TRAIN_WORLD_W - TRAIN_SCREEN_W
  );

  trainCameraY = trainClampI(
    trainCenterY - (TRAIN_SCREEN_H / 2) - TRAIN_CAMERA_OFFSET_Y,
    0,
    TRAIN_WORLD_H - TRAIN_SCREEN_H
  );
}


// =====================================================
// 抽籤動畫更新
// =====================================================
static void updateTrainLottery(unsigned long nowMs) {
  unsigned long elapsed = nowMs - trainStateStartMs;
  unsigned long totalLotteryMs = TRAIN_LOTTERY_RANDOM_MS + TRAIN_LOTTERY_SLOW_MS;

  if (elapsed >= totalLotteryMs) {
    trainStartTargetBlink(nowMs);
    return;
  }

  unsigned long stepMs = TRAIN_LOTTERY_FAST_STEP_MS;

  if (elapsed >= TRAIN_LOTTERY_RANDOM_MS) {
    unsigned long slowElapsed = elapsed - TRAIN_LOTTERY_RANDOM_MS;

    if (TRAIN_LOTTERY_SLOW_MS > 0) {
      stepMs = TRAIN_LOTTERY_SLOW_STEP_MIN_MS +
        ((TRAIN_LOTTERY_SLOW_STEP_MAX_MS - TRAIN_LOTTERY_SLOW_STEP_MIN_MS) * slowElapsed) /
        TRAIN_LOTTERY_SLOW_MS;
    } else {
      stepMs = TRAIN_LOTTERY_SLOW_STEP_MAX_MS;
    }
  }

  if (trainLotteryLastStepMs == 0 || nowMs - trainLotteryLastStepMs >= stepMs) {
    trainLotteryLastStepMs = nowMs;
    trainLotteryDisplayStationIndex = (uint8_t)random(0, TRAIN_STATION_COUNT);
  }
}


// =====================================================
// 目的地閃爍更新
// =====================================================
static void updateTrainTargetBlink(unsigned long nowMs) {
  if (nowMs - trainLotteryBlinkLastMs < TRAIN_TARGET_BLINK_HALF_MS) {
    return;
  }

  trainLotteryBlinkLastMs = nowMs;
  trainLotteryBlinkVisible = !trainLotteryBlinkVisible;
  trainLotteryBlinkHalfCycles++;

  if (trainLotteryBlinkHalfCycles >= TRAIN_TARGET_BLINK_COUNT * 2) {
    trainLotteryDisplayStationIndex = trainTargetStationIndex;
    trainLotteryBlinkVisible = true;
    trainStartMovingToTarget();
  }
}


// =====================================================
// 火車移動到抽中的目標車站
// =====================================================
static void updateTrainMoveToTarget(unsigned long nowMs) {
  uint16_t targetRailIndex = trainStationRailIndex(trainTargetStationIndex);
  float targetDistance = trainDistanceAtRailIndex(targetRailIndex);

  float distanceToStop;

  if (trainMoveDir > 0) {
    distanceToStop = trainForwardDistance(trainHeadDistance, targetDistance);
  } else {
    distanceToStop = trainForwardDistance(targetDistance, trainHeadDistance);
  }

  if (distanceToStop <= TRAIN_SPEED) {
    trainHeadDistance = targetDistance;
    trainCurrentStationIndex = trainTargetStationIndex;

    trainRunState = TRAIN_STATE_WAIT_LOTTERY;
    trainLotteryPanelVisible = false;
    trainNextLotteryMs = nowMs + TRAIN_DRAW_INTERVAL_MS;

    return;
  }

  trainHeadDistance = trainWrapDistance(trainHeadDistance + (TRAIN_SPEED * trainMoveDir));
}


// =====================================================
// 主狀態更新
//
// 流程：
// 1. 停在目前車站等待 TRAIN_DRAW_INTERVAL_MS
// 2. 左下角播放抽籤動畫
// 3. 確定目的地後閃爍 3 次
// 4. 火車開往該目的地
// 5. 到站後回到等待狀態，如此循環
// =====================================================
static void updateTrainMovement(unsigned long nowMs) {
  if (trainRunState == TRAIN_STATE_WAIT_LOTTERY) {
    if (trainTimeReached(nowMs, trainNextLotteryMs)) {
      trainStartLottery(nowMs);
    }
    return;
  }

  if (trainRunState == TRAIN_STATE_LOTTERY) {
    updateTrainLottery(nowMs);
    return;
  }

  if (trainRunState == TRAIN_STATE_BLINK_TARGET) {
    updateTrainTargetBlink(nowMs);
    return;
  }

  if (trainRunState == TRAIN_STATE_MOVING) {
    updateTrainMoveToTarget(nowMs);
    return;
  }
}


// =====================================================
// 整幀重繪
//
// 繪圖順序：
// 1. 背景底色
// 2. 背景地標物件
// 3. 軌道
// 4. 站名
// 5. 火車
// 6. 時鐘
// =====================================================
static void renderTrainFullFrame() {
  drawTrainBackground();
  drawTrainBgObjects();
  drawTrainRail();
  drawTrainStations();
  drawTrainBody();
  drawThemeClockText();
  drawTrainLotteryPanel();
}


// =====================================================
// 初始化
// =====================================================
static void TrainModeInit() {
  if (!ModefirstRun) return;

  if (!trainRandomSeeded) {
    randomSeed((uint32_t)micros() ^ (uint32_t)millis());
    trainRandomSeeded = true;
  }

  trainCalcTotalDistance();

  // 從台北出發，先停在台北，等待第一次抽籤
  trainCurrentStationIndex = 1;
  trainTargetStationIndex = 1;
  trainLotteryDisplayStationIndex = 1;
  trainMoveDir = 1;
  trainNextMoveDir = 1;
  trainReverseUseBodyGapFix = true;

  uint16_t startRailIndex = trainStationRailIndex(trainCurrentStationIndex);
  trainHeadDistance = trainDistanceAtRailIndex(startRailIndex);

  unsigned long nowMs = millis();
  trainRunState = TRAIN_STATE_WAIT_LOTTERY;

  // 第一次抽籤時間
  // 預設 1 秒後開始，方便測試
  trainNextLotteryMs = nowMs + TRAIN_FIRST_DRAW_DELAY_MS;

  trainStateStartMs = nowMs;
  trainLotteryLastStepMs = 0;
  trainLotteryPanelVisible = false;
  trainLotteryBlinkVisible = true;
  trainLotteryBlinkHalfCycles = 0;
  trainLotteryBlinkLastMs = nowMs;

  updateTrainCamera();
  renderTrainFullFrame();

  ModefirstRun = false;
}


// =====================================================
// 主模式
// =====================================================
void TrainMode() {
  TrainModeInit();

  unsigned long nowMs = millis();

  updateTrainMovement(nowMs);
  updateTrainCamera();
  renderTrainFullFrame();

  wait_with_display(40);
}


// =====================================================
// MarioTrain Mode：整幀重繪
//
// 與 renderTrainFullFrame() 相同，
// 只是把 drawTrainBody() 換成 drawMarioTrainBody()。
// =====================================================
static void renderMarioTrainFullFrame() {
  drawTrainBackground();
  drawTrainBgObjects();
  drawTrainRail();
  drawTrainStations();

  // 這裡改畫 Mario Kart 角色
  drawMarioTrainBody();

  drawThemeClockText();
  drawTrainLotteryPanel();
}


// =====================================================
// MarioTrain Mode：初始化
//
// 邏輯與 TrainModeInit() 幾乎相同，
// 另外會隨機選三位 Kart 角色。
// =====================================================
static void MarioTrainModeInit() {
  if (!ModefirstRun) return;

  if (!trainRandomSeeded) {
    randomSeed((uint32_t)micros() ^ (uint32_t)millis());
    trainRandomSeeded = true;
  }

  trainCalcTotalDistance();

  // 每次進入 MarioTrainMode 都隨機選三位角色
  marioTrainChooseRandomDrivers();

  // 從台北出發，先停在台北，等待第一次抽籤
  trainCurrentStationIndex = 1;
  trainTargetStationIndex = 1;
  trainLotteryDisplayStationIndex = 1;

  trainMoveDir = 1;
  trainNextMoveDir = 1;
  trainReverseUseBodyGapFix = false;

  uint16_t startRailIndex = trainStationRailIndex(trainCurrentStationIndex);
  trainHeadDistance = trainDistanceAtRailIndex(startRailIndex);

  unsigned long nowMs = millis();
  trainRunState = TRAIN_STATE_WAIT_LOTTERY;
  marioTrainInitNpcActors(nowMs);

  trainNextLotteryMs = nowMs + TRAIN_FIRST_DRAW_DELAY_MS;

  trainStateStartMs = nowMs;
  trainLotteryLastStepMs = 0;
  trainLotteryPanelVisible = false;
  trainLotteryBlinkVisible = true;
  trainLotteryBlinkHalfCycles = 0;
  trainLotteryBlinkLastMs = nowMs;

  updateTrainCamera();
  renderMarioTrainFullFrame();

  ModefirstRun = false;
}


// =====================================================
// MarioTrain Mode 主函式
//
// 運行邏輯完全沿用 TrainMode：
// 等待抽籤 -> 目的地動畫 -> 目標閃爍 -> 沿軌道移動 -> 到站等待。
//
// 不同點只有：
// 1. 車輛繪圖改成三位隨機 Kart 角色
// 2. slot 0 是主角
// 3. camera 鎖定 slot 0 主角
// =====================================================
void MarioTrainMode() {
  MarioTrainModeInit();

  unsigned long nowMs = millis();

  // 主角沿用原本 TrainMode 移動邏輯
  updateTrainMovement(nowMs);

  // 其他 2 位角色各自獨立移動
  updateMarioTrainNpcActors(nowMs);

  // camera 仍然鎖定主角
  updateTrainCamera();

  renderMarioTrainFullFrame();

  wait_with_display(40);
}
