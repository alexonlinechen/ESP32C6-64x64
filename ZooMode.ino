#include "Zoo.h"


// -------------------- 畫面設定 --------------------

static const int ZOO_SCR_W = 64;
static const int ZOO_SCR_H = 64;

static const uint16_t ZOO_BG_COLOR = 0x6b2c;  //0x0000
static const uint16_t ZOO_TRANSPARENT_COLOR = 0xF81F;

// -------------------- Tile sheet 設定 --------------------

static const int ZOO_TILE_SHEET_W = 600;
static const int ZOO_TILE_FRAME_W = 60;
static const int ZOO_TILE_FRAME_H = 29;
static const uint8_t ZOO_TILE_ROAD = 5;


// -------------------- Player / NPC Character sheet 設定 --------------------

static const uint8_t ZOO_CHARACTER_COUNT = 10;

static const int ZOO_P1_SHEET_W = 68;
static const int ZOO_P1_FRAME_W = 17;
static const int ZOO_P1_FRAME_H = 22;
static const int ZOO_P1_ANCHOR_X = 8;
static const int ZOO_P1_ANCHOR_Y = 21;

static const int ZOO_P2_SHEET_W = 64;
static const int ZOO_P2_FRAME_W = 16;
static const int ZOO_P2_FRAME_H = 23;
static const int ZOO_P2_ANCHOR_X = 8;
static const int ZOO_P2_ANCHOR_Y = 22;

static const int ZOO_P3_SHEET_W = 64;
static const int ZOO_P3_FRAME_W = 16;
static const int ZOO_P3_FRAME_H = 23;
static const int ZOO_P3_ANCHOR_X = 8;
static const int ZOO_P3_ANCHOR_Y = 22;

static const int ZOO_P4_SHEET_W = 76;
static const int ZOO_P4_FRAME_W = 19;
static const int ZOO_P4_FRAME_H = 23;
static const int ZOO_P4_ANCHOR_X = 9;
static const int ZOO_P4_ANCHOR_Y = 22;

static const int ZOO_P5_SHEET_W = 72;
static const int ZOO_P5_FRAME_W = 18;
static const int ZOO_P5_FRAME_H = 22;
static const int ZOO_P5_ANCHOR_X = 9;
static const int ZOO_P5_ANCHOR_Y = 21;

static const int ZOO_P6_SHEET_W = 68;
static const int ZOO_P6_FRAME_W = 17;
static const int ZOO_P6_FRAME_H = 22;
static const int ZOO_P6_ANCHOR_X = 8;
static const int ZOO_P6_ANCHOR_Y = 21;

static const int ZOO_P7_SHEET_W = 72;
static const int ZOO_P7_FRAME_W = 18;
static const int ZOO_P7_FRAME_H = 24;
static const int ZOO_P7_ANCHOR_X = 9;
static const int ZOO_P7_ANCHOR_Y = 23;

static const int ZOO_P8_SHEET_W = 68;
static const int ZOO_P8_FRAME_W = 17;
static const int ZOO_P8_FRAME_H = 25;
static const int ZOO_P8_ANCHOR_X = 8;
static const int ZOO_P8_ANCHOR_Y = 24;

static const int ZOO_P9_SHEET_W = 60;
static const int ZOO_P9_FRAME_W = 15;
static const int ZOO_P9_FRAME_H = 24;
static const int ZOO_P9_ANCHOR_X = 7;
static const int ZOO_P9_ANCHOR_Y = 23;

static const int ZOO_P10_SHEET_W = 56;
static const int ZOO_P10_FRAME_W = 14;
static const int ZOO_P10_FRAME_H = 23;
static const int ZOO_P10_ANCHOR_X = 7;
static const int ZOO_P10_ANCHOR_Y = 22;




// -------------------- Fence sheet 設定 --------------------
static const int ZOO_FENCE_SHEET_W = 58;
static const int ZOO_FENCE_FRAME_W = 29;
static const int ZOO_FENCE_FRAME_H = 18;

// -------------------- Object sheet 設定 --------------------

static const int ZOO_OBJ1_W = 33;
static const int ZOO_OBJ1_H = 26;

static const int ZOO_OBJ2_W = 51;
static const int ZOO_OBJ2_H = 46;

static const int ZOO_OBJ3_W = 50;
static const int ZOO_OBJ3_H = 47;

static const int ZOO_OBJ4_W = 54;
static const int ZOO_OBJ4_H = 48;

static const int ZOO_OBJ5_W = 44;
static const int ZOO_OBJ5_H = 41;


// -------------------- Animal sheet 設定 --------------------

static const int ZOO_ANIMAL1_SHEET_W = 156; // 北極熊 39 x 22, 4 frames
static const int ZOO_ANIMAL1_FRAME_W = 39;
static const int ZOO_ANIMAL1_FRAME_H = 22;
static const int ZOO_ANIMAL1_ANCHOR_X = 19;
static const int ZOO_ANIMAL1_ANCHOR_Y = 21;

static const int ZOO_ANIMAL2_SHEET_W = 80;  // 企鵝 20 x 19, 4 frames
static const int ZOO_ANIMAL2_FRAME_W = 20;
static const int ZOO_ANIMAL2_FRAME_H = 19;
static const int ZOO_ANIMAL2_ANCHOR_X = 10;
static const int ZOO_ANIMAL2_ANCHOR_Y = 18;

static const int ZOO_ANIMAL3_SHEET_W = 136; // 熊貓 34 x 22, 4 frames
static const int ZOO_ANIMAL3_FRAME_W = 34;
static const int ZOO_ANIMAL3_FRAME_H = 22;
static const int ZOO_ANIMAL3_ANCHOR_X = 17;
static const int ZOO_ANIMAL3_ANCHOR_Y = 21;

static const int ZOO_ANIMAL4_SHEET_W = 148; // 棕熊 37 x 22, 4 frames
static const int ZOO_ANIMAL4_FRAME_W = 37;
static const int ZOO_ANIMAL4_FRAME_H = 22;
static const int ZOO_ANIMAL4_ANCHOR_X = 18;
static const int ZOO_ANIMAL4_ANCHOR_Y = 21;


static const int ZOO_ANIMAL5_SHEET_W = 88; // 無尾熊 88 x 17, 4 frames
static const int ZOO_ANIMAL5_FRAME_W = 22;
static const int ZOO_ANIMAL5_FRAME_H = 17;
static const int ZOO_ANIMAL5_ANCHOR_X = 11;
static const int ZOO_ANIMAL5_ANCHOR_Y = 16;


static const int ZOO_ANIMAL6_SHEET_W = 116; //  長頸鹿 116 x 33, 4 frames
static const int ZOO_ANIMAL6_FRAME_W = 29;
static const int ZOO_ANIMAL6_FRAME_H = 33;
static const int ZOO_ANIMAL6_ANCHOR_X = 15;
static const int ZOO_ANIMAL6_ANCHOR_Y = 32;


static const int ZOO_ANIMAL7_SHEET_W = 96; //  河馬 96 x 15, 4 frames
static const int ZOO_ANIMAL7_FRAME_W = 24;
static const int ZOO_ANIMAL7_FRAME_H = 15;
static const int ZOO_ANIMAL7_ANCHOR_X = 12;
static const int ZOO_ANIMAL7_ANCHOR_Y = 14;


static const int ZOO_ANIMAL8_SHEET_W = 112; //  大象 112 x 18, 4 frames
static const int ZOO_ANIMAL8_FRAME_W = 28;
static const int ZOO_ANIMAL8_FRAME_H = 18;
static const int ZOO_ANIMAL8_ANCHOR_X = 14;
static const int ZOO_ANIMAL8_ANCHOR_Y = 17;


static const int ZOO_ANIMAL9_SHEET_W = 156; //  獅子 156 x 23, 4 frames
static const int ZOO_ANIMAL9_FRAME_W = 39;
static const int ZOO_ANIMAL9_FRAME_H = 23;
static const int ZOO_ANIMAL9_ANCHOR_X = 20;
static const int ZOO_ANIMAL9_ANCHOR_Y = 21;



static const int ZOO_ANIMAL10_SHEET_W = 68; //  猩猩 68 x 16, 4 frames
static const int ZOO_ANIMAL10_FRAME_W = 17;
static const int ZOO_ANIMAL10_FRAME_H = 16;
static const int ZOO_ANIMAL10_ANCHOR_X = 8;
static const int ZOO_ANIMAL10_ANCHOR_Y = 15;


static const int ZOO_ANIMAL11_SHEET_W = 80; //  狐狸 80 x 12, 4 frames
static const int ZOO_ANIMAL11_FRAME_W = 20;
static const int ZOO_ANIMAL11_FRAME_H = 12;
static const int ZOO_ANIMAL11_ANCHOR_X = 10;
static const int ZOO_ANIMAL11_ANCHOR_Y = 11;


static const int ZOO_ANIMAL12_SHEET_W = 96; //  綿羊 96 x 18, 4 frames
static const int ZOO_ANIMAL12_FRAME_W = 24;
static const int ZOO_ANIMAL12_FRAME_H = 18;
static const int ZOO_ANIMAL12_ANCHOR_X = 12;
static const int ZOO_ANIMAL12_ANCHOR_Y = 17;


static const int ZOO_ANIMAL13_SHEET_W = 128; //  浣熊 128 x 15, 4 frames
static const int ZOO_ANIMAL13_FRAME_W = 32;
static const int ZOO_ANIMAL13_FRAME_H = 15;
static const int ZOO_ANIMAL13_ANCHOR_X = 16;
static const int ZOO_ANIMAL13_ANCHOR_Y = 14;


static const int ZOO_ANIMAL14_SHEET_W = 108; //  小熊貓 108 x 15, 4 frames
static const int ZOO_ANIMAL14_FRAME_W = 27;
static const int ZOO_ANIMAL14_FRAME_H = 15;
static const int ZOO_ANIMAL14_ANCHOR_X = 13;
static const int ZOO_ANIMAL14_ANCHOR_Y = 14;

static const int ZOO_ANIMAL15_SHEET_W = 68; //  兔子 68 x 15, 4 frames
static const int ZOO_ANIMAL15_FRAME_W = 17;
static const int ZOO_ANIMAL15_FRAME_H = 15;
static const int ZOO_ANIMAL15_ANCHOR_X = 8;
static const int ZOO_ANIMAL15_ANCHOR_Y = 14;

static const int ZOO_ANIMAL16_SHEET_W = 60; //  浣熊 60 x 11, 4 frames
static const int ZOO_ANIMAL16_FRAME_W = 15;
static const int ZOO_ANIMAL16_FRAME_H = 11;
static const int ZOO_ANIMAL16_ANCHOR_X = 7;
static const int ZOO_ANIMAL16_ANCHOR_Y = 10;



// -------------------- 地圖設定 --------------------

static const int ZOO_MAP_COLS = 19;
static const int ZOO_MAP_ROWS = 18;

static const int ZOO_STEP_X = 30;
static const int ZOO_STEP_Y = 15;


static const int ZOO_WORLD_W = 1130;
static const int ZOO_WORLD_H = 574;

static const int ZOO_CROP_OFFSET_X = 520;
static const int ZOO_CROP_OFFSET_Y = 10;

// -------------------- 移動設定 --------------------

static const unsigned long ZOO_MOVE_INTERVAL_MS = 120UL;
static const unsigned long ZOO_ANIM_INTERVAL_MS = 120UL;

static const int ZOO_MOVE_STEP_X = 2;
static const int ZOO_MOVE_STEP_Y = 1;

// -------------------- 方向 --------------------

static const uint8_t ZOO_DIR_DOWN = 0;
static const uint8_t ZOO_DIR_LEFT = 1;
static const uint8_t ZOO_DIR_RIGHT = 2;
static const uint8_t ZOO_DIR_UP = 3;


// -------------------- 行走車道偏移 --------------------

static const int ZOO_PLAYER_LANE_OFFSET_X = 4;
static const int ZOO_PLAYER_LANE_OFFSET_Y = 8;

static int zooGetPlayerLaneOffsetX(uint8_t dir) {
  if (dir == ZOO_DIR_DOWN)  return -ZOO_PLAYER_LANE_OFFSET_X;
  if (dir == ZOO_DIR_UP)    return  ZOO_PLAYER_LANE_OFFSET_X;
  if (dir == ZOO_DIR_LEFT)  return -ZOO_PLAYER_LANE_OFFSET_X;
  if (dir == ZOO_DIR_RIGHT) return  ZOO_PLAYER_LANE_OFFSET_X;

  return 0;
}

static int zooGetPlayerLaneOffsetY(uint8_t dir) {
  if (dir == ZOO_DIR_DOWN)  return  ZOO_PLAYER_LANE_OFFSET_Y;
  if (dir == ZOO_DIR_UP)    return -ZOO_PLAYER_LANE_OFFSET_Y;
  if (dir == ZOO_DIR_LEFT)  return -ZOO_PLAYER_LANE_OFFSET_Y;
  if (dir == ZOO_DIR_RIGHT) return  ZOO_PLAYER_LANE_OFFSET_Y;

  return 0;
}


static uint8_t zooGetDirFromCellStep(
  int fromCol,
  int fromRow,
  int toCol,
  int toRow
) {
  if (toCol > fromCol) return ZOO_DIR_DOWN;
  if (toCol < fromCol) return ZOO_DIR_UP;
  if (toRow > fromRow) return ZOO_DIR_LEFT;
  if (toRow < fromRow) return ZOO_DIR_RIGHT;

  return ZOO_DIR_DOWN;
}


// -------------------- 地圖資料 --------------------
//
// 這張表已經轉成 ZOO_TILE 的實際 frame code。
// 5 代表馬路，也就是主角可行走區域。

static const uint8_t zooMapTileGrid[ZOO_MAP_ROWS][ZOO_MAP_COLS] PROGMEM = {
  {  0,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8 },
  { 10,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  9 },
  { 10,  5,  4,  4,  4,  5,  4,  4,  4,  5,  0,  0,  0,  5,  0,  0,  0,  5,  9 },
  { 10,  5,  4,  4,  4,  5,  4,  4,  4,  5,  0,  0,  0,  5,  2,  2,  2,  5,  9 },
  { 10,  5,  4,  4,  4,  5,  7,  7,  7,  5,  0,  0,  0,  5,  2,  2,  2,  5,  9 },
  { 10,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  9 },
  { 10,  5,  1,  1,  1,  5,  0,  0,  0,  0,  0,  5,  0,  0,  0,  0,  0,  5,  9 },
  { 10,  5,  1,  1,  1,  5,  0,  6,  6,  6,  0,  5,  0,  1,  1,  1,  0,  5,  9 },
  { 10,  5,  1,  1,  1,  5,  0,  6,  0,  6,  0,  5,  0,  1,  0,  1,  0,  5,  9 },
  { 10,  5,  5,  5,  5,  5,  0,  6,  0,  6,  0,  5,  0,  1,  0,  1,  0,  5,  9 },
  { 10,  5,  0,  0,  0,  5,  0,  6,  0,  6,  0,  5,  0,  1,  0,  1,  0,  5,  9 },
  { 10,  5,  0,  0,  0,  5,  0,  6,  6,  6,  0,  5,  0,  1,  1,  1,  0,  5,  9 },
  { 10,  5,  0,  0,  0,  5,  0,  0,  0,  0,  0,  5,  0,  0,  0,  0,  0,  5,  9 },
  { 10,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  9 },
  { 10,  5,  7,  7,  7,  5,  2,  2,  5,  0,  0,  5,  1,  1,  5,  0,  0,  5,  9 },
  { 10,  5,  7,  7,  7,  5,  2,  2,  5,  0,  0,  5,  1,  1,  5,  0,  0,  5,  9 },
  { 10,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  9 },
  { 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,  9 }
};

// -------------------- 主角狀態 --------------------

static bool zooInited = false;

static int zooPlayerX = 0;
static int zooPlayerY = 0;

static int zooCameraX = 0;
static int zooCameraY = 0;

static int zooCurrentCol = 1;
static int zooCurrentRow = 1;

static int zooFinalCol = 1;
static int zooFinalRow = 1;

static int zooNextCol = 1;
static int zooNextRow = 1;

static int zooMoveTargetX = 0;
static int zooMoveTargetY = 0;

static bool zooHasMoveTarget = false;

static uint8_t zooDir = ZOO_DIR_DOWN;
static uint8_t zooAnimStep = 0;
static uint8_t zooPlayerCharacterIndex = 0;
static unsigned long zooLastMoveMs = 0;
static unsigned long zooLastAnimMs = 0;


// -------------------- 主角行為狀態機 --------------------

static const uint8_t ZOO_PLAYER_BEHAVIOR_RANDOM = 0;
static const uint8_t ZOO_PLAYER_BEHAVIOR_TARGET = 1;
static const uint8_t ZOO_PLAYER_BEHAVIOR_CUTE_AREA = 2;
static const uint8_t ZOO_PLAYER_BEHAVIOR_SHOP_AREA = 3;

static const uint8_t ZOO_PLAYER_STATE_PICK_NEXT = 0;
static const uint8_t ZOO_PLAYER_STATE_MOVING = 1;
static const uint8_t ZOO_PLAYER_STATE_WAIT_RANDOM = 2;
static const uint8_t ZOO_PLAYER_STATE_WAIT_TARGET_BEFORE_CAM = 3;

// Camera 平滑移到動物
static const uint8_t ZOO_PLAYER_STATE_CAM_PAN_LEFT = 4;

// Camera 鎖定動物觀察中
static const uint8_t ZOO_PLAYER_STATE_OBSERVE_ANIMAL = 5;

// Camera 平滑回主角
static const uint8_t ZOO_PLAYER_STATE_CAM_RETURN = 6;

static const uint8_t ZOO_PLAYER_STATE_WAIT_TARGET_AFTER_CAM = 7;

// 主角在可愛動物區自由走動
static const uint8_t ZOO_PLAYER_STATE_CUTE_AREA_MOVING = 8;

// 主角在可愛動物區休息
static const uint8_t ZOO_PLAYER_STATE_CUTE_AREA_RESTING = 9;


// 四種行為抽取機率： 商店區 20% 可愛動物區 20%  觀察動物 40%  隨機走路剩下 25%
static const uint8_t ZOO_PLAYER_SHOP_AREA_CHANCE_PERCENT = 20;
static const uint8_t ZOO_PLAYER_CUTE_AREA_CHANCE_PERCENT = 20;
static const uint8_t ZOO_PLAYER_TARGET_CHANCE_PERCENT = 40;

static const unsigned long ZOO_PLAYER_RANDOM_WAIT_MS = 10000UL;
static const unsigned long ZOO_PLAYER_TARGET_BEFORE_CAM_WAIT_MS = 3000UL;
static const unsigned long ZOO_PLAYER_OBSERVE_WAIT_MS = 20000UL;
static const unsigned long ZOO_PLAYER_TARGET_AFTER_CAM_WAIT_MS = 3000UL;


// 可愛動物區 
static const uint8_t ZOO_PLAYER_CUTE_AREA_INDEX = 11;
static const uint8_t ZOO_PLAYER_SHOP_AREA_INDEX = 12;

// 主角在可愛動物區維持 2 分鐘
static const unsigned long ZOO_PLAYER_CUTE_AREA_TOTAL_MS = 120000UL;

// 主角在可愛動物區每次休息時間
static const unsigned long ZOO_PLAYER_CUTE_AREA_REST_MIN_MS = 5000UL;
static const unsigned long ZOO_PLAYER_CUTE_AREA_REST_MAX_MS = 8000UL;

static const int ZOO_PLAYER_OBSERVE_TARGET_COUNT = 11;

static const int zooPlayerObserveTargetX[ZOO_PLAYER_OBSERVE_TARGET_COUNT] = {
  579, 699, 459, 339, 819, 939, 219 ,311, 401, 491, 581
};

static const int zooPlayerObserveTargetY[ZOO_PLAYER_OBSERVE_TARGET_COUNT] = {
  126, 186, 186, 246, 246, 306 ,306, 350, 395, 440, 485
};


// 每個目標點對應一個動物區：
// 0 北極熊、1 企鵝、2 熊貓、3 棕熊、4 無尾熊、5 長頸鹿 6 河馬、7 大象、8 獅子、9 猩猩

static const uint8_t zooPlayerObserveTargetArea[ZOO_PLAYER_OBSERVE_TARGET_COUNT] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
};


static uint8_t zooPlayerBehavior = ZOO_PLAYER_BEHAVIOR_RANDOM;
static uint8_t zooPlayerState = ZOO_PLAYER_STATE_PICK_NEXT;
static unsigned long zooPlayerStateStartMs = 0;

static uint8_t zooPlayerRoamAreaIndex = ZOO_PLAYER_CUTE_AREA_INDEX;

static int zooPlayerRoamTargetX = 0;
static int zooPlayerRoamTargetY = 0;

static unsigned long zooPlayerRoamAreaStartMs = 0;
static unsigned long zooPlayerRoamAreaRestUntilMs = 0;


// -------------------- NPC 狀態 --------------------

static const uint8_t ZOO_NPC_COUNT = 9;

static const uint8_t ZOO_NPC_BEHAVIOR_RANDOM = 0;
static const uint8_t ZOO_NPC_BEHAVIOR_CUTE_AREA = 1;
static const uint8_t ZOO_NPC_BEHAVIOR_SHOP_AREA = 2;

static const uint8_t ZOO_NPC_STATE_PICK_NEXT = 0;
static const uint8_t ZOO_NPC_STATE_MOVING = 1;
static const uint8_t ZOO_NPC_STATE_WAIT_RANDOM = 2;
static const uint8_t ZOO_NPC_STATE_ROAM_MOVING = 3;
static const uint8_t ZOO_NPC_STATE_ROAM_RESTING = 4;

static const uint8_t ZOO_NPC_SHOP_AREA_CHANCE_PERCENT = 25;  //NPC去商店區機率
static const uint8_t ZOO_NPC_CUTE_AREA_CHANCE_PERCENT = 20;  //NPC去動物區機率

static const unsigned long ZOO_NPC_RANDOM_WAIT_MIN_MS = 5000UL;
static const unsigned long ZOO_NPC_RANDOM_WAIT_MAX_MS = 12000UL;

static const unsigned long ZOO_NPC_ROAM_TOTAL_MS = 90000UL;  //NPC 在區域停留時間
static const unsigned long ZOO_NPC_ROAM_REST_MIN_MS = 4000UL;
static const unsigned long ZOO_NPC_ROAM_REST_MAX_MS = 9000UL;

static const int ZOO_NPC_MIN_DISTANCE = 6;

struct ZooNpc {
  uint8_t characterIndex;

  int x;
  int y;

  int currentCol;
  int currentRow;

  int finalCol;
  int finalRow;

  int nextCol;
  int nextRow;

  int moveTargetX;
  int moveTargetY;

  bool hasMoveTarget;

  uint8_t dir;
  uint8_t animStep;

  uint8_t behavior;
  uint8_t state;

  uint8_t roamAreaIndex;

  int roamTargetX;
  int roamTargetY;

  unsigned long stateStartMs;
  unsigned long waitUntilMs;
  unsigned long roamStartMs;
  unsigned long roamRestUntilMs;

  unsigned long lastMoveMs;
  unsigned long lastAnimMs;
};

static ZooNpc zooNpcs[ZOO_NPC_COUNT];
static int zooNpcCount = 0;


// -------------------- Camera 鎖定模式 --------------------

static const uint8_t ZOO_CAMERA_FOLLOW_PLAYER = 0;
static const uint8_t ZOO_CAMERA_FOLLOW_ANIMAL = 1;

static uint8_t zooCameraFollowMode = ZOO_CAMERA_FOLLOW_PLAYER;
static int zooCameraFollowAnimalIndex = -1;
static uint8_t zooPlayerObserveArea = 0;

// Camera 每幀平移速度  數字越大，Camera 移動越快
static const int ZOO_CAMERA_FOLLOW_STEP_X = 1;
static const int ZOO_CAMERA_FOLLOW_STEP_Y = 1;

// 到目標幾 px 內就算抵達
static const int ZOO_CAMERA_ARRIVE_TOLERANCE = 1;

// -------------------- 動物狀態 --------------------

static const uint8_t ZOO_ANIMAL_KIND_POLAR = 0;
static const uint8_t ZOO_ANIMAL_KIND_PENGUIN = 1;
static const uint8_t ZOO_ANIMAL_KIND_PANDA = 2;
static const uint8_t ZOO_ANIMAL_KIND_BEAR = 3;
static const uint8_t ZOO_ANIMAL_KIND_KOALA = 4;
static const uint8_t ZOO_ANIMAL_KIND_GIRAFFE = 5;
static const uint8_t ZOO_ANIMAL_KIND_HIPPO = 6;
static const uint8_t ZOO_ANIMAL_KIND_ELEPHANT = 7;
static const uint8_t ZOO_ANIMAL_KIND_LION = 8;
static const uint8_t ZOO_ANIMAL_KIND_APE = 9;
static const uint8_t ZOO_ANIMAL_KIND_FOX = 10;
static const uint8_t ZOO_ANIMAL_KIND_SHEEP = 11;
static const uint8_t ZOO_ANIMAL_KIND_RACCOON = 12;
static const uint8_t ZOO_ANIMAL_KIND_SPANDA = 13;
static const uint8_t ZOO_ANIMAL_KIND_RABBIT = 14;
static const uint8_t ZOO_ANIMAL_KIND_GUINEA = 15;

static const uint8_t ZOO_ANIMAL_AREA_POLAR = 0;
static const uint8_t ZOO_ANIMAL_AREA_PENGUIN = 1;
static const uint8_t ZOO_ANIMAL_AREA_PANDA = 2;
static const uint8_t ZOO_ANIMAL_AREA_BEAR = 3;
static const uint8_t ZOO_ANIMAL_AREA_KOALA = 4;
static const uint8_t ZOO_ANIMAL_AREA_GIRAFFE = 5;
static const uint8_t ZOO_ANIMAL_AREA_HIPPO = 6;
static const uint8_t ZOO_ANIMAL_AREA_ELEPHANT = 7;
static const uint8_t ZOO_ANIMAL_AREA_LION = 8;
static const uint8_t ZOO_ANIMAL_AREA_APE = 9;
static const uint8_t ZOO_ANIMAL_AREA_FOX = 10;

static const uint8_t ZOO_ANIMAL_AREA_SHEEP = 11;
static const uint8_t ZOO_ANIMAL_AREA_RACCOON = 11;
static const uint8_t ZOO_ANIMAL_AREA_SPANDA = 11;
static const uint8_t ZOO_ANIMAL_AREA_RABBIT = 11;
static const uint8_t ZOO_ANIMAL_AREA_GUINEA = 11;

static const int ZOO_ANIMAL_TOTAL = 43;  //動物總數量

static const unsigned long ZOO_ANIMAL_MOVE_INTERVAL_MS = 240UL;
static const unsigned long ZOO_ANIMAL_ANIM_INTERVAL_MS = 240UL;


//動物休息時間
static const unsigned long ZOO_ANIMAL_REST_MIN_MS = 3000UL;
static const unsigned long ZOO_ANIMAL_REST_MAX_MS = 6000UL;


static const int ZOO_ANIMAL_MOVE_STEP_X = 2;
static const int ZOO_ANIMAL_MOVE_STEP_Y = 1;

struct ZooAnimalArea {
  int ux;
  int uy;
  int rx;
  int ry;
  int dx;
  int dy;
  int lx;
  int ly;
};

struct ZooAnimal {
  uint8_t kind;
  uint8_t area;
  int x;
  int y;
  int targetX;
  int targetY;
  uint8_t dir;
  uint8_t animStep;
bool hasTarget;
bool isResting;
unsigned long restUntilMs;
unsigned long lastMoveMs;
unsigned long lastAnimMs;  
};

// 四個活動範圍都是菱形：centerX, centerY, radiusX, radiusY
static const ZooAnimalArea zooAnimalAreas[13] = {
  { 549, 79, 619, 114, 549, 149, 480, 114 }, // 北極熊：(549,79)(619,114)(549,149)(480,114)
  { 669, 139, 739, 174, 669, 209, 600, 174 }, // 企鵝：(669,139)(739,174)(669,209)(600,174)
  { 429, 139, 499, 174, 429, 209, 360, 174 },// 熊貓：(429,139)(499,174)(429,209)(360,174)
  { 309, 199, 379, 234, 309, 269, 240, 234 }, // 棕熊：(309,199)(379,234)(309,269)(240,234)
  { 789, 199, 859, 234, 789, 269, 720, 234 },  // 無尾熊
  { 909, 259, 979, 294, 909, 329, 840, 294 },  // 長頸鹿
  
  { 191, 254, 271, 294, 218, 320, 138, 280 },  // 河馬
  { 311, 314, 361, 339, 310, 364, 261, 339 },  // 大象
  { 401, 359, 451, 384, 400, 409, 351, 384 },  // 獅子
  { 491, 404, 541, 429, 490, 454, 441, 429 },  // 猩猩
  { 581, 449, 631, 474, 580, 499, 531, 474 },  // 狐狸

  { 730, 288, 859, 354, 669, 449, 540, 384 },  // 可愛動物區
  { 541, 231, 635, 278, 481, 355, 386, 308 },  // 商店區
};

static ZooAnimal zooAnimals[ZOO_ANIMAL_TOTAL];
static int zooAnimalCount = 0;


// BFS 暫存
static int16_t zooPrev[ZOO_MAP_COLS * ZOO_MAP_ROWS];
static uint16_t zooQueue[ZOO_MAP_COLS * ZOO_MAP_ROWS];

// ============================================================
// 基礎工具
// ============================================================

static int zooAbs(int v) {
  return v < 0 ? -v : v;
}

static int zooSign(int v) {
  if (v > 0) return 1;
  if (v < 0) return -1;
  return 0;
}

static int zooLimitedStep(int delta, int stepSize) {
  int a = zooAbs(delta);

  if (a <= stepSize) return delta;

  return zooSign(delta) * stepSize;
}

static uint8_t zooGetTileCode(int col, int row) {
  if (col < 0 || col >= ZOO_MAP_COLS ||
      row < 0 || row >= ZOO_MAP_ROWS) {
    return 255;
  }

  return pgm_read_byte(&(zooMapTileGrid[row][col]));
}

static bool zooIsRoadCell(int col, int row) {
  return zooGetTileCode(col, row) == ZOO_TILE_ROAD;
}

static uint16_t zooCellIndex(int col, int row) {
  return (uint16_t)(row * ZOO_MAP_COLS + col);
}

static int zooCellCenterX(int col, int row) {
  return (col - row) * ZOO_STEP_X + ZOO_CROP_OFFSET_X + ZOO_TILE_FRAME_W / 2;
}

static int zooCellCenterY(int col, int row) {
  return (col + row) * ZOO_STEP_Y + ZOO_CROP_OFFSET_Y + ZOO_TILE_FRAME_H / 2;
}

static int zooTileX(int col, int row) {
  return (col - row) * ZOO_STEP_X + ZOO_CROP_OFFSET_X;
}

static int zooTileY(int col, int row) {
  return (col + row) * ZOO_STEP_Y + ZOO_CROP_OFFSET_Y;
}


static bool zooFindNearestRoadCellToWorld(
  int worldX,
  int worldY,
  int* outCol,
  int* outRow
) {
  long bestDist = 2147483647L;
  bool found = false;

  for (int r = 0; r < ZOO_MAP_ROWS; r++) {
    for (int c = 0; c < ZOO_MAP_COLS; c++) {
      if (!zooIsRoadCell(c, r)) continue;

      int cx = zooCellCenterX(c, r);
      int cy = zooCellCenterY(c, r);

      long dx = (long)cx - (long)worldX;
      long dy = (long)cy - (long)worldY;
      long dist = dx * dx + dy * dy;

      if (!found || dist < bestDist) {
        bestDist = dist;
        *outCol = c;
        *outRow = r;
        found = true;
      }
    }
  }

  return found;
}

// ============================================================
// Tile frame 對應
// ============================================================

static int zooGetTileFrame(uint8_t tileCode) {
  if (tileCode <= 8) return tileCode;

  // 11.png
  if (tileCode == 9) return 9;

  // 10.png = 9.png 的左右翻轉
  if (tileCode == 10) return 8;

  // 12.png = 11.png 的左右翻轉
  if (tileCode == 11) return 9;

  return 0;
}

static bool zooGetTileFlipX(uint8_t tileCode) {
  if (tileCode == 10) return true;
  if (tileCode == 11) return true;

  return false;
}

// ============================================================
//  frame 繪製
// ============================================================
static void drawZooRgb565Frame(
  const uint8_t* bitmap,
  const uint16_t* palette,
  int sheetW,
  int frameW,
  int frameH,
  int frameIndex,
  bool flipX,
  int x,
  int y
) {
  if (frameIndex < 0) return;

  if (x <= -frameW || x >= ZOO_SCR_W ||
      y <= -frameH || y >= ZOO_SCR_H) {
    return;
  }

  int frameStartX = frameIndex * frameW;

  for (int j = 0; j < frameH; j++) {
    int dy = y + j;

    if (dy < 0 || dy >= ZOO_SCR_H) continue;

    for (int i = 0; i < frameW; i++) {
      int dx = x + i;

      if (dx < 0 || dx >= ZOO_SCR_W) continue;

      int sx = flipX ? (frameW - 1 - i) : i;

      uint32_t pos =
        (uint32_t)j * (uint32_t)sheetW +
        (uint32_t)frameStartX +
        (uint32_t)sx;

      uint8_t colorIndex = pgm_read_byte(&(bitmap[pos]));
      uint16_t color = pgm_read_word(&(palette[colorIndex]));

      if (color == ZOO_TRANSPARENT_COLOR) continue;

      display.drawPixel(dx, dy, color);
    }
  }
}


// ============================================================
// 固定物件：ZOO_OBJ 商店區物件
// ============================================================
static void drawZooObjectFrame(
  const uint8_t* bitmap,
  const uint16_t* palette,
  int objW,
  int objH,
  int worldX,
  int worldY
) {
  drawZooRgb565Frame(
    bitmap,
    palette,
    objW,
    objW,
    objH,
    0,
    false,
    worldX - zooCameraX,
    worldY - zooCameraY
  );
}


static void drawZooObjects() {
  // ZOO_OBJ1 有 4 個擺放點
  drawZooObjectFrame(ZOO_OBJ1, ZOO_OBJ1_PALETTE, ZOO_OBJ1_W, ZOO_OBJ1_H, 524, 190);
  drawZooObjectFrame(ZOO_OBJ1, ZOO_OBJ1_PALETTE, ZOO_OBJ1_W, ZOO_OBJ1_H, 658, 257);
  drawZooObjectFrame(ZOO_OBJ1, ZOO_OBJ1_PALETTE, ZOO_OBJ1_W, ZOO_OBJ1_H, 349, 277);
  drawZooObjectFrame(ZOO_OBJ1, ZOO_OBJ1_PALETTE, ZOO_OBJ1_W, ZOO_OBJ1_H, 483, 344);

  drawZooObjectFrame(ZOO_OBJ2, ZOO_OBJ2_PALETTE, ZOO_OBJ2_W, ZOO_OBJ2_H, 480, 205);
  drawZooObjectFrame(ZOO_OBJ3, ZOO_OBJ3_PALETTE, ZOO_OBJ3_W, ZOO_OBJ3_H, 382, 252);
  drawZooObjectFrame(ZOO_OBJ4, ZOO_OBJ4_PALETTE, ZOO_OBJ4_W, ZOO_OBJ4_H, 546, 200);
  drawZooObjectFrame(ZOO_OBJ5, ZOO_OBJ5_PALETTE, ZOO_OBJ5_W, ZOO_OBJ5_H, 604, 231);
}


// ============================================================
// 固定物件：ZOO_FENCE 柵欄
// ============================================================
static void drawZooFenceFrame(
  int frameIndex,
  bool flipX,
  int worldX,
  int worldY
) {
  drawZooRgb565Frame(
    ZOO_FENCE,
    ZOO_FENCE_PALETTE,
    ZOO_FENCE_SHEET_W,
    ZOO_FENCE_FRAME_W,
    ZOO_FENCE_FRAME_H,
    frameIndex,
    flipX,
    worldX - zooCameraX,
    worldY - zooCameraY
  );
}


static void drawZoo_Fence_Up() {
  // W_UL 白色柵欄上左：ZOO_FENCE frame0
  drawZooFenceFrame(0, false, 400, 129);
  drawZooFenceFrame(0, false, 372, 143);
  drawZooFenceFrame(0, false, 344, 157);
  drawZooFenceFrame(0, false, 280, 189);
  drawZooFenceFrame(0, false, 252, 203);
  drawZooFenceFrame(0, false, 224, 217);

  drawZooFenceFrame(0, false, 760, 189);
  drawZooFenceFrame(0, false, 732, 203);
  drawZooFenceFrame(0, false, 704, 217);

  drawZooFenceFrame(0, false, 880, 249);
  drawZooFenceFrame(0, false, 852, 263);
  drawZooFenceFrame(0, false, 824, 277);

  drawZooFenceFrame(0, false, 460, 397);
  drawZooFenceFrame(0, false, 432, 411); 

  drawZooFenceFrame(0, false, 550, 442);
  drawZooFenceFrame(0, false, 522, 456);
    

  // W_UR 白色柵欄上右：ZOO_FENCE frame0 水平翻轉
  drawZooFenceFrame(0, true, 429, 129);
  drawZooFenceFrame(0, true, 457, 143);
  drawZooFenceFrame(0, true, 485, 157);
  drawZooFenceFrame(0, true, 300, 189);
  drawZooFenceFrame(0, true, 337, 203);
  drawZooFenceFrame(0, true, 365, 217);
  
  drawZooFenceFrame(0, true, 789, 189);
  drawZooFenceFrame(0, true, 817, 203);
  drawZooFenceFrame(0, true, 845, 217);
  
  drawZooFenceFrame(0, true, 909, 249);
  drawZooFenceFrame(0, true, 937, 263);
  drawZooFenceFrame(0, true, 965, 277);

  drawZooFenceFrame(0, true, 489, 397);
  drawZooFenceFrame(0, true, 517, 411);

  drawZooFenceFrame(0, true, 579, 442);
  drawZooFenceFrame(0, true, 607, 456);

  // B_UL 棕色柵欄上左：ZOO_FENCE frame1
  drawZooFenceFrame(1, false, 520, 69);
  drawZooFenceFrame(1, false, 492, 83);
  drawZooFenceFrame(1, false, 464, 97);
  drawZooFenceFrame(1, false, 640, 129);
  drawZooFenceFrame(1, false, 612, 143);
  drawZooFenceFrame(1, false, 584, 157);

  drawZooFenceFrame(1, false, 280, 307);
  drawZooFenceFrame(1, false, 252, 321);
  
  drawZooFenceFrame(1, false, 370, 352);
  drawZooFenceFrame(1, false, 342, 366);  

  // B_UR 棕色柵欄上右：ZOO_FENCE frame1 水平翻轉
  drawZooFenceFrame(1, true, 549, 69);
  drawZooFenceFrame(1, true, 577, 83);
  drawZooFenceFrame(1, true, 605, 97);
  drawZooFenceFrame(1, true, 669, 129);
  drawZooFenceFrame(1, true, 697, 143);
  drawZooFenceFrame(1, true, 725, 157);

  drawZooFenceFrame(1, true, 309, 307);
  drawZooFenceFrame(1, true, 337, 321);

  drawZooFenceFrame(1, true, 399, 352);
  drawZooFenceFrame(1, true, 427, 366);  
  
}

static void drawZoo_Fence_Down() {
  // W_DL 白色柵欄下左：ZOO_FENCE frame0 水平翻轉
  drawZooFenceFrame(0, true, 344, 172);
  drawZooFenceFrame(0, true, 372, 186);
  drawZooFenceFrame(0, true, 400, 200);
  drawZooFenceFrame(0, true, 224, 232);
  drawZooFenceFrame(0, true, 252, 246);
  drawZooFenceFrame(0, true, 280, 260);

  drawZooFenceFrame(0, true, 704, 232);
  drawZooFenceFrame(0, true, 732, 246);
  drawZooFenceFrame(0, true, 760, 260);

  drawZooFenceFrame(0, true, 824, 292);
  drawZooFenceFrame(0, true, 852, 306);
  drawZooFenceFrame(0, true, 880, 320);

  drawZooFenceFrame(0, true, 432, 426);
  drawZooFenceFrame(0, true, 460, 440);
  
  drawZooFenceFrame(0, true, 522, 471);
  drawZooFenceFrame(0, true, 550, 485);

  
  // W_DR 白色柵欄下右：ZOO_FENCE frame0
  drawZooFenceFrame(0, false, 487, 172);
  drawZooFenceFrame(0, false, 459, 186);
  drawZooFenceFrame(0, false, 431, 200);
  drawZooFenceFrame(0, false, 367, 232);
  drawZooFenceFrame(0, false, 339, 246);
  drawZooFenceFrame(0, false, 311, 260);


  drawZooFenceFrame(0, false, 847, 232);
  drawZooFenceFrame(0, false, 819, 246);
  drawZooFenceFrame(0, false, 791, 260);

  drawZooFenceFrame(0, false, 967, 292);
  drawZooFenceFrame(0, false, 939, 306);
  drawZooFenceFrame(0, false, 911, 320);

  drawZooFenceFrame(0, false, 519, 426);
  drawZooFenceFrame(0, false, 491, 440);
    
  drawZooFenceFrame(0, false, 609, 471);
  drawZooFenceFrame(0, false, 581, 485);


  // B_DL 棕色柵欄下左：ZOO_FENCE frame1 水平翻轉
  drawZooFenceFrame(1, true, 464, 112);
  drawZooFenceFrame(1, true, 492, 126);
  drawZooFenceFrame(1, true, 520, 140);
  drawZooFenceFrame(1, true, 584, 172);
  drawZooFenceFrame(1, true, 612, 186);
  drawZooFenceFrame(1, true, 640, 200);

  drawZooFenceFrame(1, true, 252, 336);
  drawZooFenceFrame(1, true, 280, 350); 

  drawZooFenceFrame(1, true, 342, 381);
  drawZooFenceFrame(1, true, 370, 395);
  

  // B_DR 棕色柵欄下右：ZOO_FENCE frame1
  drawZooFenceFrame(1, false, 607, 112);
  drawZooFenceFrame(1, false, 579, 126);
  drawZooFenceFrame(1, false, 552, 140);
  drawZooFenceFrame(1, false, 727, 172);
  drawZooFenceFrame(1, false, 699, 186);
  drawZooFenceFrame(1, false, 671, 200);

  drawZooFenceFrame(1, false, 339, 336);
  drawZooFenceFrame(1, false, 311, 350);

  drawZooFenceFrame(1, false, 429, 381);
  drawZooFenceFrame(1, false, 401, 395);  
}

// ============================================================
// 方向與動畫
// ============================================================

static uint8_t zooGetDirFromVector(int dx, int dy) {
  int adx = zooAbs(dx);
  int ady = zooAbs(dy);

  if (adx <= ZOO_MOVE_STEP_X && ady <= ZOO_MOVE_STEP_Y) {
    return zooDir;
  }

  if (adx <= ZOO_MOVE_STEP_X) {
    return dy >= 0 ? ZOO_DIR_DOWN : ZOO_DIR_UP;
  }

  if (ady <= ZOO_MOVE_STEP_Y) {
    return dx >= 0 ? ZOO_DIR_RIGHT : ZOO_DIR_LEFT;
  }

  if (dx >= 0 && dy >= 0) return ZOO_DIR_DOWN;
  if (dx <  0 && dy >= 0) return ZOO_DIR_LEFT;
  if (dx >= 0 && dy <  0) return ZOO_DIR_RIGHT;

  return ZOO_DIR_UP;
}





static uint8_t zooGetDirFromVectorWithFallback(
  uint8_t fallbackDir,
  int dx,
  int dy
) {
  int adx = zooAbs(dx);
  int ady = zooAbs(dy);

  if (adx <= ZOO_MOVE_STEP_X && ady <= ZOO_MOVE_STEP_Y) {
    return fallbackDir;
  }

  if (adx <= ZOO_MOVE_STEP_X) {
    return dy >= 0 ? ZOO_DIR_DOWN : ZOO_DIR_UP;
  }

  if (ady <= ZOO_MOVE_STEP_Y) {
    return dx >= 0 ? ZOO_DIR_RIGHT : ZOO_DIR_LEFT;
  }

  if (dx >= 0 && dy >= 0) return ZOO_DIR_DOWN;
  if (dx <  0 && dy >= 0) return ZOO_DIR_LEFT;
  if (dx >= 0 && dy <  0) return ZOO_DIR_RIGHT;

  return ZOO_DIR_UP;
}

static int zooGetCharacterFrame(uint8_t dir, uint8_t animStep) {
  if (dir == ZOO_DIR_DOWN ||
      dir == ZOO_DIR_LEFT) {
    return animStep ? 1 : 0;
  }

  return animStep ? 3 : 2;
}

static bool zooGetCharacterFlipX(uint8_t dir) {
  return dir == ZOO_DIR_LEFT ||
         dir == ZOO_DIR_UP;
}

static void zooGetCharacterSprite(
  uint8_t characterIndex,
  const uint8_t** bitmap,
  const uint16_t** palette,
  int* sheetW,
  int* frameW,
  int* frameH,
  int* anchorX,
  int* anchorY
) {
  *bitmap = ZOO_P1;
  *palette = ZOO_P1_PALETTE;
  *sheetW = ZOO_P1_SHEET_W;
  *frameW = ZOO_P1_FRAME_W;
  *frameH = ZOO_P1_FRAME_H;
  *anchorX = ZOO_P1_ANCHOR_X;
  *anchorY = ZOO_P1_ANCHOR_Y;

  if (characterIndex == 1) {
    *bitmap = ZOO_P2;
    *palette = ZOO_P2_PALETTE;
    *sheetW = ZOO_P2_SHEET_W;
    *frameW = ZOO_P2_FRAME_W;
    *frameH = ZOO_P2_FRAME_H;
    *anchorX = ZOO_P2_ANCHOR_X;
    *anchorY = ZOO_P2_ANCHOR_Y;
  } else if (characterIndex == 2) {
    *bitmap = ZOO_P3;
    *palette = ZOO_P3_PALETTE;
    *sheetW = ZOO_P3_SHEET_W;
    *frameW = ZOO_P3_FRAME_W;
    *frameH = ZOO_P3_FRAME_H;
    *anchorX = ZOO_P3_ANCHOR_X;
    *anchorY = ZOO_P3_ANCHOR_Y;
  } else if (characterIndex == 3) {
    *bitmap = ZOO_P4;
    *palette = ZOO_P4_PALETTE;
    *sheetW = ZOO_P4_SHEET_W;
    *frameW = ZOO_P4_FRAME_W;
    *frameH = ZOO_P4_FRAME_H;
    *anchorX = ZOO_P4_ANCHOR_X;
    *anchorY = ZOO_P4_ANCHOR_Y;
  } else if (characterIndex == 4) {
    *bitmap = ZOO_P5;
    *palette = ZOO_P5_PALETTE;
    *sheetW = ZOO_P5_SHEET_W;
    *frameW = ZOO_P5_FRAME_W;
    *frameH = ZOO_P5_FRAME_H;
    *anchorX = ZOO_P5_ANCHOR_X;
    *anchorY = ZOO_P5_ANCHOR_Y;
  } else if (characterIndex == 5) {
    *bitmap = ZOO_P6;
    *palette = ZOO_P6_PALETTE;
    *sheetW = ZOO_P6_SHEET_W;
    *frameW = ZOO_P6_FRAME_W;
    *frameH = ZOO_P6_FRAME_H;
    *anchorX = ZOO_P6_ANCHOR_X;
    *anchorY = ZOO_P6_ANCHOR_Y;
  } else if (characterIndex == 6) {
    *bitmap = ZOO_P7;
    *palette = ZOO_P7_PALETTE;
    *sheetW = ZOO_P7_SHEET_W;
    *frameW = ZOO_P7_FRAME_W;
    *frameH = ZOO_P7_FRAME_H;
    *anchorX = ZOO_P7_ANCHOR_X;
    *anchorY = ZOO_P7_ANCHOR_Y;
  } else if (characterIndex == 7) {
    *bitmap = ZOO_P8;
    *palette = ZOO_P8_PALETTE;
    *sheetW = ZOO_P8_SHEET_W;
    *frameW = ZOO_P8_FRAME_W;
    *frameH = ZOO_P8_FRAME_H;
    *anchorX = ZOO_P8_ANCHOR_X;
    *anchorY = ZOO_P8_ANCHOR_Y;
  } else if (characterIndex == 8) {
    *bitmap = ZOO_P9;
    *palette = ZOO_P9_PALETTE;
    *sheetW = ZOO_P9_SHEET_W;
    *frameW = ZOO_P9_FRAME_W;
    *frameH = ZOO_P9_FRAME_H;
    *anchorX = ZOO_P9_ANCHOR_X;
    *anchorY = ZOO_P9_ANCHOR_Y;
  } else if (characterIndex == 9) {
    *bitmap = ZOO_P10;
    *palette = ZOO_P10_PALETTE;
    *sheetW = ZOO_P10_SHEET_W;
    *frameW = ZOO_P10_FRAME_W;
    *frameH = ZOO_P10_FRAME_H;
    *anchorX = ZOO_P10_ANCHOR_X;
    *anchorY = ZOO_P10_ANCHOR_Y;
  }
}


static void drawZooCharacterFrame(
  uint8_t characterIndex,
  int worldX,
  int worldY,
  uint8_t dir,
  uint8_t animStep
) {
  const uint8_t* bitmap = ZOO_P1;
  const uint16_t* palette = ZOO_P1_PALETTE;

  int sheetW = ZOO_P1_SHEET_W;
  int frameW = ZOO_P1_FRAME_W;
  int frameH = ZOO_P1_FRAME_H;
  int anchorX = ZOO_P1_ANCHOR_X;
  int anchorY = ZOO_P1_ANCHOR_Y;

  zooGetCharacterSprite(
    characterIndex,
    &bitmap,
    &palette,
    &sheetW,
    &frameW,
    &frameH,
    &anchorX,
    &anchorY
  );

  int frame = zooGetCharacterFrame(dir, animStep);
  bool flipX = zooGetCharacterFlipX(dir);

  drawZooRgb565Frame(
    bitmap,
    palette,
    sheetW,
    frameW,
    frameH,
    frame,
    flipX,
    worldX - zooCameraX - anchorX,
    worldY - zooCameraY - anchorY
  );
}



// ============================================================
// 動物：方向動畫，套用主角同邏輯
// ============================================================

static int zooGetAnimalFrame(uint8_t dir, uint8_t animStep) {
  // DOWN / LEFT 共用 frame0, frame1
  // LEFT 靠水平翻轉
  if (dir == ZOO_DIR_DOWN ||
      dir == ZOO_DIR_LEFT) {
    return animStep ? 1 : 0;
  }

  // RIGHT / UP 共用 frame2, frame3
  // UP 靠水平翻轉
  return animStep ? 3 : 2;
}

static bool zooGetAnimalFlipX(uint8_t dir) {
  return dir == ZOO_DIR_LEFT ||
         dir == ZOO_DIR_UP;
}

static long zooCross(
  int ax, int ay,
  int bx, int by,
  int px, int py
) {
  return (long)(bx - ax) * (long)(py - ay) -
         (long)(by - ay) * (long)(px - ax);
}


static bool zooAnimalPointInArea(int x, int y, uint8_t areaIndex) {
  const ZooAnimalArea* area = &zooAnimalAreas[areaIndex];

  long c1 = zooCross(area->ux, area->uy, area->rx, area->ry, x, y);
  long c2 = zooCross(area->rx, area->ry, area->dx, area->dy, x, y);
  long c3 = zooCross(area->dx, area->dy, area->lx, area->ly, x, y);
  long c4 = zooCross(area->lx, area->ly, area->ux, area->uy, x, y);

  bool allPositive = c1 >= 0 && c2 >= 0 && c3 >= 0 && c4 >= 0;
  bool allNegative = c1 <= 0 && c2 <= 0 && c3 <= 0 && c4 <= 0;

  return allPositive || allNegative;
}




static void zooPickAnimalPointInArea(uint8_t areaIndex, int* outX, int* outY) {
  const ZooAnimalArea* area = &zooAnimalAreas[areaIndex];

  int minX = area->ux;
  int maxX = area->ux;
  int minY = area->uy;
  int maxY = area->uy;

  if (area->rx < minX) minX = area->rx;
  if (area->dx < minX) minX = area->dx;
  if (area->lx < minX) minX = area->lx;

  if (area->rx > maxX) maxX = area->rx;
  if (area->dx > maxX) maxX = area->dx;
  if (area->lx > maxX) maxX = area->lx;

  if (area->ry < minY) minY = area->ry;
  if (area->dy < minY) minY = area->dy;
  if (area->ly < minY) minY = area->ly;

  if (area->ry > maxY) maxY = area->ry;
  if (area->dy > maxY) maxY = area->dy;
  if (area->ly > maxY) maxY = area->ly;

  for (int tries = 0; tries < 120; tries++) {
    int x = random(minX, maxX + 1);
    int y = random(minY, maxY + 1);

    if (zooAnimalPointInArea(x, y, areaIndex)) {
      *outX = x;
      *outY = y;
      return;
    }
  }

  // 保險值：用四點平均當中心
  *outX = (area->ux + area->rx + area->dx + area->lx) / 4;
  *outY = (area->uy + area->ry + area->dy + area->ly) / 4;
}

static void zooAnimalPickTarget(int animalIndex) {
  ZooAnimal* animal = &zooAnimals[animalIndex];

  zooPickAnimalPointInArea(
    animal->area,
    &(animal->targetX),
    &(animal->targetY)
  );

  animal->hasTarget = true;
}

static void zooAddAnimal(uint8_t kind, uint8_t area) {
  if (zooAnimalCount >= ZOO_ANIMAL_TOTAL) return;

  ZooAnimal* animal = &zooAnimals[zooAnimalCount];

  animal->kind = kind;
  animal->area = area;

  zooPickAnimalPointInArea(area, &(animal->x), &(animal->y));

  animal->targetX = animal->x;
  animal->targetY = animal->y;

  animal->dir = random(4);
  animal->animStep = random(2);
  animal->hasTarget = false;
  animal->isResting = false;
  animal->restUntilMs = 0;

  unsigned long nowMs = millis();
  animal->lastMoveMs = nowMs + random(0, 120);
  animal->lastAnimMs = nowMs + random(0, 240);

  zooAnimalCount++;
}

static void initZooAnimals() {
  zooAnimalCount = 0;

  // ZOO_ANIMAL1 北極熊 x3
  for (int i = 0; i < 3; i++) {
    zooAddAnimal(ZOO_ANIMAL_KIND_POLAR, ZOO_ANIMAL_AREA_POLAR);
  }

  // ZOO_ANIMAL2 企鵝 x4
  for (int i = 0; i < 4; i++) {
    zooAddAnimal(ZOO_ANIMAL_KIND_PENGUIN, ZOO_ANIMAL_AREA_PENGUIN);
  }

  // ZOO_ANIMAL3 熊貓 x3
  for (int i = 0; i < 3; i++) {
    zooAddAnimal(ZOO_ANIMAL_KIND_PANDA, ZOO_ANIMAL_AREA_PANDA);
  }

  // ZOO_ANIMAL4 棕熊 x3
  for (int i = 0; i < 3; i++) {
    zooAddAnimal(ZOO_ANIMAL_KIND_BEAR, ZOO_ANIMAL_AREA_BEAR);
  }

  // ZOO_ANIMAL5 無尾熊 x3
  for (int i = 0; i < 3; i++) {
    zooAddAnimal(ZOO_ANIMAL_KIND_KOALA, ZOO_ANIMAL_AREA_KOALA);
  }

  // ZOO_ANIMAL6 長頸鹿 x2
  for (int i = 0; i < 2; i++) {
    zooAddAnimal(ZOO_ANIMAL_KIND_GIRAFFE, ZOO_ANIMAL_AREA_GIRAFFE);
  }

  // ZOO_ANIMAL7 河馬 x3
  for (int i = 0; i < 3; i++) {
    zooAddAnimal(ZOO_ANIMAL_KIND_HIPPO, ZOO_ANIMAL_AREA_HIPPO);
  }


  // ZOO_ANIMAL8 大象 x2
  for (int i = 0; i < 2; i++) {
    zooAddAnimal(ZOO_ANIMAL_KIND_ELEPHANT, ZOO_ANIMAL_AREA_ELEPHANT);
  }


  // ZOO_ANIMAL9 獅子 x2
  for (int i = 0; i < 2; i++) {
    zooAddAnimal(ZOO_ANIMAL_KIND_LION, ZOO_ANIMAL_AREA_LION);
  }


  // ZOO_ANIMAL10 猩猩 x3
  for (int i = 0; i < 3; i++) {
    zooAddAnimal(ZOO_ANIMAL_KIND_APE, ZOO_ANIMAL_AREA_APE);
  }

  // ZOO_ANIMAL11 狐狸 x3
  for (int i = 0; i < 3; i++) {
    zooAddAnimal(ZOO_ANIMAL_KIND_FOX, ZOO_ANIMAL_AREA_FOX);
  }


  // ZOO_ANIMAL12 綿羊 x2
  for (int i = 0; i < 2; i++) {
    zooAddAnimal(ZOO_ANIMAL_KIND_SHEEP, ZOO_ANIMAL_AREA_SHEEP);
  }

    // ZOO_ANIMAL13 浣熊 x2
  for (int i = 0; i < 2; i++) {
    zooAddAnimal(ZOO_ANIMAL_KIND_RACCOON, ZOO_ANIMAL_AREA_RACCOON);
  }

    // ZOO_ANIMAL14 小熊貓 x2
  for (int i = 0; i < 2; i++) {
    zooAddAnimal(ZOO_ANIMAL_KIND_SPANDA, ZOO_ANIMAL_AREA_SPANDA);
  }

    // ZOO_ANIMAL15 兔子 x3
  for (int i = 0; i < 3; i++) {
    zooAddAnimal(ZOO_ANIMAL_KIND_RABBIT, ZOO_ANIMAL_AREA_RABBIT);
  }

    // ZOO_ANIMAL16 天竺鼠 x3
  for (int i = 0; i < 3; i++) {
    zooAddAnimal(ZOO_ANIMAL_KIND_GUINEA, ZOO_ANIMAL_AREA_GUINEA);
  }



  
}

static void updateZooAnimals(unsigned long nowMs) {
  for (int i = 0; i < zooAnimalCount; i++) {
    ZooAnimal* animal = &zooAnimals[i];

    if (animal->isResting) {
  if (nowMs >= animal->restUntilMs) {
    animal->isResting = false;
    animal->lastAnimMs = nowMs;
    animal->lastMoveMs = nowMs;
  } else {
    continue;
  }
}

if (nowMs - animal->lastAnimMs >= ZOO_ANIMAL_ANIM_INTERVAL_MS) {
  animal->lastAnimMs = nowMs;
  animal->animStep = !animal->animStep;
}

if (nowMs - animal->lastMoveMs < ZOO_ANIMAL_MOVE_INTERVAL_MS) {
  continue;
}

animal->lastMoveMs = nowMs;
    

    if (!animal->hasTarget ||
        (animal->x == animal->targetX && animal->y == animal->targetY)) {
      zooAnimalPickTarget(i);
    }

    int dx = animal->targetX - animal->x;
    int dy = animal->targetY - animal->y;

    animal->dir = zooGetDirFromVector(dx, dy);

    int sx = zooLimitedStep(dx, ZOO_ANIMAL_MOVE_STEP_X);
    int sy = zooLimitedStep(dy, ZOO_ANIMAL_MOVE_STEP_Y);

    animal->x += sx;
    animal->y += sy;

if (animal->x == animal->targetX &&
    animal->y == animal->targetY) {
  animal->hasTarget = false;
  animal->isResting = true;
  animal->animStep = 0;
  animal->restUntilMs = nowMs + random(
    ZOO_ANIMAL_REST_MIN_MS,
    ZOO_ANIMAL_REST_MAX_MS + 1
  );
}
  }
}

static void drawZooAnimalFrame(int animalIndex) {
  ZooAnimal* animal = &zooAnimals[animalIndex];

  const uint8_t* bitmap = ZOO_ANIMAL1;
  const uint16_t* palette = ZOO_ANIMAL1_PALETTE;
  int sheetW = ZOO_ANIMAL1_SHEET_W;
  int frameW = ZOO_ANIMAL1_FRAME_W;
  int frameH = ZOO_ANIMAL1_FRAME_H;
  int anchorX = ZOO_ANIMAL1_ANCHOR_X;
  int anchorY = ZOO_ANIMAL1_ANCHOR_Y;

  if (animal->kind == ZOO_ANIMAL_KIND_PENGUIN) {
    bitmap = ZOO_ANIMAL2;
    palette = ZOO_ANIMAL2_PALETTE;
    sheetW = ZOO_ANIMAL2_SHEET_W;
    frameW = ZOO_ANIMAL2_FRAME_W;
    frameH = ZOO_ANIMAL2_FRAME_H;
    anchorX = ZOO_ANIMAL2_ANCHOR_X;
    anchorY = ZOO_ANIMAL2_ANCHOR_Y;
  } else if (animal->kind == ZOO_ANIMAL_KIND_PANDA) {
    bitmap = ZOO_ANIMAL3;
    palette = ZOO_ANIMAL3_PALETTE;
    sheetW = ZOO_ANIMAL3_SHEET_W;
    frameW = ZOO_ANIMAL3_FRAME_W;
    frameH = ZOO_ANIMAL3_FRAME_H;
    anchorX = ZOO_ANIMAL3_ANCHOR_X;
    anchorY = ZOO_ANIMAL3_ANCHOR_Y;
  } else if (animal->kind == ZOO_ANIMAL_KIND_BEAR) {
    bitmap = ZOO_ANIMAL4;
    palette = ZOO_ANIMAL4_PALETTE;
    sheetW = ZOO_ANIMAL4_SHEET_W;
    frameW = ZOO_ANIMAL4_FRAME_W;
    frameH = ZOO_ANIMAL4_FRAME_H;
    anchorX = ZOO_ANIMAL4_ANCHOR_X;
    anchorY = ZOO_ANIMAL4_ANCHOR_Y;
  } else if (animal->kind == ZOO_ANIMAL_KIND_KOALA) {
    bitmap = ZOO_ANIMAL5;
    palette = ZOO_ANIMAL5_PALETTE;
    sheetW = ZOO_ANIMAL5_SHEET_W;
    frameW = ZOO_ANIMAL5_FRAME_W;
    frameH = ZOO_ANIMAL5_FRAME_H;
    anchorX = ZOO_ANIMAL5_ANCHOR_X;
    anchorY = ZOO_ANIMAL5_ANCHOR_Y;
  } else if (animal->kind == ZOO_ANIMAL_KIND_GIRAFFE) {
    bitmap = ZOO_ANIMAL6;
    palette = ZOO_ANIMAL6_PALETTE;
    sheetW = ZOO_ANIMAL6_SHEET_W;
    frameW = ZOO_ANIMAL6_FRAME_W;
    frameH = ZOO_ANIMAL6_FRAME_H;
    anchorX = ZOO_ANIMAL6_ANCHOR_X;
    anchorY = ZOO_ANIMAL6_ANCHOR_Y;
  }else if (animal->kind == ZOO_ANIMAL_KIND_HIPPO) {
    bitmap = ZOO_ANIMAL7;
    palette = ZOO_ANIMAL7_PALETTE;
    sheetW = ZOO_ANIMAL7_SHEET_W;
    frameW = ZOO_ANIMAL7_FRAME_W;
    frameH = ZOO_ANIMAL7_FRAME_H;
    anchorX = ZOO_ANIMAL7_ANCHOR_X;
    anchorY = ZOO_ANIMAL7_ANCHOR_Y;
  }else if (animal->kind == ZOO_ANIMAL_KIND_ELEPHANT) {
    bitmap = ZOO_ANIMAL8;
    palette = ZOO_ANIMAL8_PALETTE;
    sheetW = ZOO_ANIMAL8_SHEET_W;
    frameW = ZOO_ANIMAL8_FRAME_W;
    frameH = ZOO_ANIMAL8_FRAME_H;
    anchorX = ZOO_ANIMAL8_ANCHOR_X;
    anchorY = ZOO_ANIMAL8_ANCHOR_Y;
  }else if (animal->kind == ZOO_ANIMAL_KIND_LION) {
    bitmap = ZOO_ANIMAL9;
    palette = ZOO_ANIMAL9_PALETTE;
    sheetW = ZOO_ANIMAL9_SHEET_W;
    frameW = ZOO_ANIMAL9_FRAME_W;
    frameH = ZOO_ANIMAL9_FRAME_H;
    anchorX = ZOO_ANIMAL9_ANCHOR_X;
    anchorY = ZOO_ANIMAL9_ANCHOR_Y;
  }else if (animal->kind == ZOO_ANIMAL_KIND_APE) {
    bitmap = ZOO_ANIMAL10;
    palette = ZOO_ANIMAL10_PALETTE;
    sheetW = ZOO_ANIMAL10_SHEET_W;
    frameW = ZOO_ANIMAL10_FRAME_W;
    frameH = ZOO_ANIMAL10_FRAME_H;
    anchorX = ZOO_ANIMAL10_ANCHOR_X;
    anchorY = ZOO_ANIMAL10_ANCHOR_Y;
  }else if (animal->kind == ZOO_ANIMAL_KIND_FOX) {
    bitmap = ZOO_ANIMAL11;
    palette = ZOO_ANIMAL11_PALETTE;
    sheetW = ZOO_ANIMAL11_SHEET_W;
    frameW = ZOO_ANIMAL11_FRAME_W;
    frameH = ZOO_ANIMAL11_FRAME_H;
    anchorX = ZOO_ANIMAL11_ANCHOR_X;
    anchorY = ZOO_ANIMAL11_ANCHOR_Y;
  }else if (animal->kind == ZOO_ANIMAL_KIND_SHEEP) {
    bitmap = ZOO_ANIMAL12;
    palette = ZOO_ANIMAL12_PALETTE;
    sheetW = ZOO_ANIMAL12_SHEET_W;
    frameW = ZOO_ANIMAL12_FRAME_W;
    frameH = ZOO_ANIMAL12_FRAME_H;
    anchorX = ZOO_ANIMAL12_ANCHOR_X;
    anchorY = ZOO_ANIMAL12_ANCHOR_Y;
  }else if (animal->kind == ZOO_ANIMAL_KIND_RACCOON) {
    bitmap = ZOO_ANIMAL13;
    palette = ZOO_ANIMAL13_PALETTE;
    sheetW = ZOO_ANIMAL13_SHEET_W;
    frameW = ZOO_ANIMAL13_FRAME_W;
    frameH = ZOO_ANIMAL13_FRAME_H;
    anchorX = ZOO_ANIMAL13_ANCHOR_X;
    anchorY = ZOO_ANIMAL13_ANCHOR_Y;
  }else if (animal->kind == ZOO_ANIMAL_KIND_SPANDA) {
    bitmap = ZOO_ANIMAL14;
    palette = ZOO_ANIMAL14_PALETTE;
    sheetW = ZOO_ANIMAL14_SHEET_W;
    frameW = ZOO_ANIMAL14_FRAME_W;
    frameH = ZOO_ANIMAL14_FRAME_H;
    anchorX = ZOO_ANIMAL14_ANCHOR_X;
    anchorY = ZOO_ANIMAL14_ANCHOR_Y;
  }else if (animal->kind == ZOO_ANIMAL_KIND_RABBIT) {
    bitmap = ZOO_ANIMAL15;
    palette = ZOO_ANIMAL15_PALETTE;
    sheetW = ZOO_ANIMAL15_SHEET_W;
    frameW = ZOO_ANIMAL15_FRAME_W;
    frameH = ZOO_ANIMAL15_FRAME_H;
    anchorX = ZOO_ANIMAL15_ANCHOR_X;
    anchorY = ZOO_ANIMAL15_ANCHOR_Y;
  }else if (animal->kind == ZOO_ANIMAL_KIND_GUINEA) {
    bitmap = ZOO_ANIMAL16;
    palette = ZOO_ANIMAL16_PALETTE;
    sheetW = ZOO_ANIMAL16_SHEET_W;
    frameW = ZOO_ANIMAL16_FRAME_W;
    frameH = ZOO_ANIMAL16_FRAME_H;
    anchorX = ZOO_ANIMAL16_ANCHOR_X;
    anchorY = ZOO_ANIMAL16_ANCHOR_Y;
  }

  int frame = zooGetAnimalFrame(animal->dir, animal->animStep);
  bool flipX = zooGetAnimalFlipX(animal->dir);

  int drawX = animal->x - zooCameraX - anchorX;
  int drawY = animal->y - zooCameraY - anchorY;

drawZooRgb565Frame(
  bitmap,
  palette,
  sheetW,
  frameW,
  frameH,
  frame,
  flipX,
  drawX,
  drawY
);


}


static int zooPickAnimalIndexInArea(uint8_t areaIndex) {
  int count = 0;

  for (int i = 0; i < zooAnimalCount; i++) {
    if (zooAnimals[i].area == areaIndex) {
      count++;
    }
  }

  if (count <= 0) {
    return -1;
  }

  int pick = random(count);

  for (int i = 0; i < zooAnimalCount; i++) {
    if (zooAnimals[i].area == areaIndex) {
      if (pick == 0) {
        return i;
      }

      pick--;
    }
  }

  return -1;
}

static void drawZooAnimals() {
  // 依 Y 座標由上到下畫，避免動物互相遮擋怪怪的
  bool drawn[ZOO_ANIMAL_TOTAL];

  for (int i = 0; i < ZOO_ANIMAL_TOTAL; i++) {
    drawn[i] = false;
  }

  for (int pass = 0; pass < zooAnimalCount; pass++) {
    int bestIndex = -1;
    int bestY = 32767;

    for (int i = 0; i < zooAnimalCount; i++) {
      if (drawn[i]) continue;

      if (zooAnimals[i].y < bestY) {
        bestY = zooAnimals[i].y;
        bestIndex = i;
      }
    }

    if (bestIndex >= 0) {
      drawZooAnimalFrame(bestIndex);
      drawn[bestIndex] = true;
    }
  }
}


// ============================================================
// Camera 平滑追蹤
// ============================================================

static void zooComputeCameraTarget(int* outCameraX, int* outCameraY) {
  int focusX = zooPlayerX;
  int focusY = zooPlayerY;


  if (zooCameraFollowMode == ZOO_CAMERA_FOLLOW_ANIMAL &&
      zooCameraFollowAnimalIndex >= 0 &&
      zooCameraFollowAnimalIndex < zooAnimalCount) {
    focusX = zooAnimals[zooCameraFollowAnimalIndex].x;
    focusY = zooAnimals[zooCameraFollowAnimalIndex].y;

  }

  int targetCameraX = focusX - ZOO_SCR_W / 2;
  int targetCameraY = focusY - ZOO_SCR_H / 2 - 25;

  if (targetCameraX < 0) targetCameraX = 0;
  if (targetCameraY < 0) targetCameraY = 0;

  int maxCameraX = ZOO_WORLD_W - ZOO_SCR_W;
  int maxCameraY = ZOO_WORLD_H - ZOO_SCR_H;

  if (targetCameraX > maxCameraX) targetCameraX = maxCameraX;
  if (targetCameraY > maxCameraY) targetCameraY = maxCameraY;

  *outCameraX = targetCameraX;
  *outCameraY = targetCameraY;
}

static bool zooCameraAtTarget() {
  int targetCameraX = 0;
  int targetCameraY = 0;

  zooComputeCameraTarget(&targetCameraX, &targetCameraY);

  return zooAbs(zooCameraX - targetCameraX) <= ZOO_CAMERA_ARRIVE_TOLERANCE &&
         zooAbs(zooCameraY - targetCameraY) <= ZOO_CAMERA_ARRIVE_TOLERANCE;
}

static void zooSnapCameraToTarget() {
  int targetCameraX = 0;
  int targetCameraY = 0;

  zooComputeCameraTarget(&targetCameraX, &targetCameraY);

  zooCameraX = targetCameraX;
  zooCameraY = targetCameraY;
}


// ============================================================
// BFS 尋路，只走馬路 tile
// ============================================================

static bool zooFindNextStepTo(
  int startCol,
  int startRow,
  int targetCol,
  int targetRow,
  int* outNextCol,
  int* outNextRow
) {
  if (!zooIsRoadCell(startCol, startRow)) return false;
  if (!zooIsRoadCell(targetCol, targetRow)) return false;

  if (startCol == targetCol && startRow == targetRow) {
    return false;
  }

  const int totalCells = ZOO_MAP_COLS * ZOO_MAP_ROWS;

  for (int i = 0; i < totalCells; i++) {
    zooPrev[i] = -1;
  }

  uint16_t head = 0;
  uint16_t tail = 0;

  uint16_t startIndex = zooCellIndex(startCol, startRow);
  uint16_t targetIndex = zooCellIndex(targetCol, targetRow);

  zooQueue[tail++] = startIndex;
  zooPrev[startIndex] = startIndex;

  while (head < tail) {
    uint16_t current = zooQueue[head++];

    if (current == targetIndex) break;

    int c = current % ZOO_MAP_COLS;
    int r = current / ZOO_MAP_COLS;

    const int dc[4] = { 1, -1, 0, 0 };
    const int dr[4] = { 0, 0, 1, -1 };

    for (int i = 0; i < 4; i++) {
      int nc = c + dc[i];
      int nr = r + dr[i];

      if (!zooIsRoadCell(nc, nr)) continue;

      uint16_t ni = zooCellIndex(nc, nr);

      if (zooPrev[ni] != -1) continue;

      zooPrev[ni] = current;
      zooQueue[tail++] = ni;
    }
  }

  if (zooPrev[targetIndex] == -1) {
    return false;
  }

  uint16_t walk = targetIndex;

  while (zooPrev[walk] != startIndex) {
    walk = zooPrev[walk];
  }

  *outNextCol = walk % ZOO_MAP_COLS;
  *outNextRow = walk / ZOO_MAP_COLS;

  return true;
}

static bool zooPickRandomRoadTarget() {
  for (int tries = 0; tries < 80; tries++) {
    int c = random(ZOO_MAP_COLS);
    int r = random(ZOO_MAP_ROWS);

    if (!zooIsRoadCell(c, r)) continue;

    if (c == zooCurrentCol && r == zooCurrentRow) continue;

    int nextC = zooCurrentCol;
    int nextR = zooCurrentRow;

    if (zooFindNextStepTo(
          zooCurrentCol,
          zooCurrentRow,
          c,
          r,
          &nextC,
          &nextR
        )) {
      zooFinalCol = c;
      zooFinalRow = r;

      return true;
    }
  }

  return false;
}


static void zooPrepareNextCellMove() {
  if (zooCurrentCol == zooFinalCol &&
      zooCurrentRow == zooFinalRow) {
    zooHasMoveTarget = false;
    return;
  }

  int nextC = zooCurrentCol;
  int nextR = zooCurrentRow;

  if (!zooFindNextStepTo(
        zooCurrentCol,
        zooCurrentRow,
        zooFinalCol,
        zooFinalRow,
        &nextC,
        &nextR
      )) {
    zooHasMoveTarget = false;
    return;
  }

  zooNextCol = nextC;
  zooNextRow = nextR;

  uint8_t nextDir = zooGetDirFromCellStep(
    zooCurrentCol,
    zooCurrentRow,
    zooNextCol,
    zooNextRow
  );

  zooDir = nextDir;

  zooMoveTargetX = zooCellCenterX(zooNextCol, zooNextRow)
                 + zooGetPlayerLaneOffsetX(nextDir);

  zooMoveTargetY = zooCellCenterY(zooNextCol, zooNextRow)
                 + zooGetPlayerLaneOffsetY(nextDir);

  zooHasMoveTarget = true;
}



static bool zooNpcArrivedFinalTarget(int npcIndex) {
  ZooNpc* npc = &zooNpcs[npcIndex];

  return !npc->hasMoveTarget &&
         npc->currentCol == npc->finalCol &&
         npc->currentRow == npc->finalRow;
}

static void zooNpcSetState(int npcIndex, uint8_t newState, unsigned long nowMs) {
  ZooNpc* npc = &zooNpcs[npcIndex];

  npc->state = newState;
  npc->stateStartMs = nowMs;
}

static void zooNpcSetNearestRoadLogicPosition(int npcIndex) {
  ZooNpc* npc = &zooNpcs[npcIndex];

  int col = npc->currentCol;
  int row = npc->currentRow;

  if (zooFindNearestRoadCellToWorld(npc->x, npc->y, &col, &row)) {
    npc->currentCol = col;
    npc->currentRow = row;

    npc->finalCol = col;
    npc->finalRow = row;

    npc->nextCol = col;
    npc->nextRow = row;

    npc->hasMoveTarget = false;
  }
}


static bool zooNpcPointTooCloseToPeople(int selfIndex, int x, int y) {
  long dxPlayer = (long)x - (long)zooPlayerX;
  long dyPlayer = (long)y - (long)zooPlayerY;

  if (dxPlayer * dxPlayer + dyPlayer * dyPlayer <
      (long)ZOO_NPC_MIN_DISTANCE * (long)ZOO_NPC_MIN_DISTANCE) {
    return true;
  }

  for (int i = 0; i < zooNpcCount; i++) {
    if (i == selfIndex) continue;

    long dx = (long)x - (long)zooNpcs[i].x;
    long dy = (long)y - (long)zooNpcs[i].y;

    if (dx * dx + dy * dy <
        (long)ZOO_NPC_MIN_DISTANCE * (long)ZOO_NPC_MIN_DISTANCE) {
      return true;
    }
  }

  return false;
}


static bool zooNpcPickRandomRoadTarget(int npcIndex) {
  ZooNpc* npc = &zooNpcs[npcIndex];

  for (int tries = 0; tries < 100; tries++) {
    int c = random(ZOO_MAP_COLS);
    int r = random(ZOO_MAP_ROWS);

    if (!zooIsRoadCell(c, r)) continue;

    if (c == npc->currentCol && r == npc->currentRow) continue;

    // 不要挑主角目前所在格
    if (c == zooCurrentCol && r == zooCurrentRow) continue;

    int nextC = npc->currentCol;
    int nextR = npc->currentRow;

    if (zooFindNextStepTo(
          npc->currentCol,
          npc->currentRow,
          c,
          r,
          &nextC,
          &nextR
        )) {
      npc->finalCol = c;
      npc->finalRow = r;
      return true;
    }
  }

  return false;
}

static void zooNpcPrepareNextCellMove(int npcIndex) {
  ZooNpc* npc = &zooNpcs[npcIndex];

  if (npc->currentCol == npc->finalCol &&
      npc->currentRow == npc->finalRow) {
    npc->hasMoveTarget = false;
    return;
  }

  int nextC = npc->currentCol;
  int nextR = npc->currentRow;

  if (!zooFindNextStepTo(
        npc->currentCol,
        npc->currentRow,
        npc->finalCol,
        npc->finalRow,
        &nextC,
        &nextR
      )) {
    npc->hasMoveTarget = false;
    return;
  }

  npc->nextCol = nextC;
  npc->nextRow = nextR;

  uint8_t nextDir = zooGetDirFromCellStep(
    npc->currentCol,
    npc->currentRow,
    npc->nextCol,
    npc->nextRow
  );

  npc->dir = nextDir;

  npc->moveTargetX = zooCellCenterX(npc->nextCol, npc->nextRow)
                   + zooGetPlayerLaneOffsetX(nextDir);

  npc->moveTargetY = zooCellCenterY(npc->nextCol, npc->nextRow)
                   + zooGetPlayerLaneOffsetY(nextDir);

  npc->hasMoveTarget = true;
}

static void zooNpcStartRandomTarget(int npcIndex, unsigned long nowMs) {
  ZooNpc* npc = &zooNpcs[npcIndex];

  npc->behavior = ZOO_NPC_BEHAVIOR_RANDOM;

  if (!zooNpcPickRandomRoadTarget(npcIndex)) {
    npc->waitUntilMs = nowMs + random(
      ZOO_NPC_RANDOM_WAIT_MIN_MS,
      ZOO_NPC_RANDOM_WAIT_MAX_MS + 1
    );

    zooNpcSetState(npcIndex, ZOO_NPC_STATE_WAIT_RANDOM, nowMs);
    return;
  }

  npc->hasMoveTarget = false;
  zooNpcSetState(npcIndex, ZOO_NPC_STATE_MOVING, nowMs);
}

static void zooNpcStartRoamRest(int npcIndex, unsigned long nowMs) {
  ZooNpc* npc = &zooNpcs[npcIndex];

  npc->animStep = 0;

  npc->roamRestUntilMs = nowMs + random(
    ZOO_NPC_ROAM_REST_MIN_MS,
    ZOO_NPC_ROAM_REST_MAX_MS + 1
  );

  zooNpcSetState(npcIndex, ZOO_NPC_STATE_ROAM_RESTING, nowMs);
}

static void zooNpcPickNextRoamPoint(int npcIndex, unsigned long nowMs) {
  ZooNpc* npc = &zooNpcs[npcIndex];

  for (int tries = 0; tries < 30; tries++) {
    int x = 0;
    int y = 0;

    zooPickAnimalPointInArea(
      npc->roamAreaIndex,
      &x,
      &y
    );

    if (!zooNpcPointTooCloseToPeople(npcIndex, x, y) ||
        tries == 29) {
      npc->roamTargetX = x;
      npc->roamTargetY = y;
      break;
    }
  }

  int dx = npc->roamTargetX - npc->x;
  int dy = npc->roamTargetY - npc->y;

  if (dx == 0 && dy == 0) {
    zooNpcStartRoamRest(npcIndex, nowMs);
    return;
  }

  npc->dir = zooGetDirFromVectorWithFallback(npc->dir, dx, dy);

  zooNpcSetState(npcIndex, ZOO_NPC_STATE_ROAM_MOVING, nowMs);
}

static void zooNpcBeginRoamArea(int npcIndex, unsigned long nowMs) {
  ZooNpc* npc = &zooNpcs[npcIndex];

  npc->roamStartMs = nowMs;
  npc->roamRestUntilMs = 0;
  npc->hasMoveTarget = false;

  zooNpcPickNextRoamPoint(npcIndex, nowMs);
}

static void zooNpcFinishRoamArea(int npcIndex, unsigned long nowMs) {
  zooNpcSetNearestRoadLogicPosition(npcIndex);
  zooNpcSetState(npcIndex, ZOO_NPC_STATE_PICK_NEXT, nowMs);
}

static void zooNpcStartRoamAreaTarget(
  int npcIndex,
  unsigned long nowMs,
  uint8_t behavior,
  uint8_t areaIndex
) {
  ZooNpc* npc = &zooNpcs[npcIndex];

  npc->behavior = behavior;
  npc->roamAreaIndex = areaIndex;

  int areaCenterX = 0;
  int areaCenterY = 0;

  zooGetAnimalAreaCenter(areaIndex, &areaCenterX, &areaCenterY);

  int targetCol = npc->currentCol;
  int targetRow = npc->currentRow;

  if (!zooFindNearestRoadCellToWorld(
        areaCenterX,
        areaCenterY,
        &targetCol,
        &targetRow
      )) {
    zooNpcBeginRoamArea(npcIndex, nowMs);
    return;
  }

  npc->finalCol = targetCol;
  npc->finalRow = targetRow;

  npc->hasMoveTarget = false;

  if (npc->currentCol == npc->finalCol &&
      npc->currentRow == npc->finalRow) {
    zooNpcBeginRoamArea(npcIndex, nowMs);
  } else {
    zooNpcSetState(npcIndex, ZOO_NPC_STATE_MOVING, nowMs);
  }
}

static void zooNpcPickNextBehavior(int npcIndex, unsigned long nowMs) {
  int pick = random(100);

  if (pick < ZOO_NPC_SHOP_AREA_CHANCE_PERCENT) {
    zooNpcStartRoamAreaTarget(
      npcIndex,
      nowMs,
      ZOO_NPC_BEHAVIOR_SHOP_AREA,
      ZOO_PLAYER_SHOP_AREA_INDEX
    );
    return;
  }

  pick -= ZOO_NPC_SHOP_AREA_CHANCE_PERCENT;

  if (pick < ZOO_NPC_CUTE_AREA_CHANCE_PERCENT) {
    zooNpcStartRoamAreaTarget(
      npcIndex,
      nowMs,
      ZOO_NPC_BEHAVIOR_CUTE_AREA,
      ZOO_PLAYER_CUTE_AREA_INDEX
    );
    return;
  }

  zooNpcStartRandomTarget(npcIndex, nowMs);
}

static void zooNpcUpdateRoadMovement(int npcIndex, unsigned long nowMs) {
  ZooNpc* npc = &zooNpcs[npcIndex];

  if (nowMs - npc->lastAnimMs >= ZOO_ANIM_INTERVAL_MS) {
    npc->lastAnimMs = nowMs;
    npc->animStep = !npc->animStep;
  }

  if (nowMs - npc->lastMoveMs < ZOO_MOVE_INTERVAL_MS) {
    return;
  }

  npc->lastMoveMs = nowMs;

  if (!npc->hasMoveTarget) {
    zooNpcPrepareNextCellMove(npcIndex);
  }

  if (!npc->hasMoveTarget) {
    return;
  }

  int dx = npc->moveTargetX - npc->x;
  int dy = npc->moveTargetY - npc->y;

  int sx = zooLimitedStep(dx, ZOO_MOVE_STEP_X);
  int sy = zooLimitedStep(dy, ZOO_MOVE_STEP_Y);

  npc->x += sx;
  npc->y += sy;

  if (npc->x == npc->moveTargetX &&
      npc->y == npc->moveTargetY) {
    npc->currentCol = npc->nextCol;
    npc->currentRow = npc->nextRow;
    npc->hasMoveTarget = false;
  }
}

static void zooNpcUpdateRoamMovement(int npcIndex, unsigned long nowMs) {
  ZooNpc* npc = &zooNpcs[npcIndex];

  if (nowMs - npc->lastAnimMs >= ZOO_ANIM_INTERVAL_MS) {
    npc->lastAnimMs = nowMs;
    npc->animStep = !npc->animStep;
  }

  if (nowMs - npc->lastMoveMs < ZOO_MOVE_INTERVAL_MS) {
    return;
  }

  npc->lastMoveMs = nowMs;

  int dx = npc->roamTargetX - npc->x;
  int dy = npc->roamTargetY - npc->y;

  npc->dir = zooGetDirFromVectorWithFallback(npc->dir, dx, dy);

  int sx = zooLimitedStep(dx, ZOO_MOVE_STEP_X);
  int sy = zooLimitedStep(dy, ZOO_MOVE_STEP_Y);

  npc->x += sx;
  npc->y += sy;

  if (npc->x == npc->roamTargetX &&
      npc->y == npc->roamTargetY) {
    zooNpcStartRoamRest(npcIndex, nowMs);
  }
}

static void updateZooNpcs(unsigned long nowMs) {
  for (int i = 0; i < zooNpcCount; i++) {
    ZooNpc* npc = &zooNpcs[i];

    if (npc->state == ZOO_NPC_STATE_PICK_NEXT) {
      zooNpcPickNextBehavior(i, nowMs);
      continue;
    }

    if (npc->state == ZOO_NPC_STATE_MOVING) {
      if (zooNpcArrivedFinalTarget(i)) {
        if (npc->behavior == ZOO_NPC_BEHAVIOR_RANDOM) {
          npc->waitUntilMs = nowMs + random(
            ZOO_NPC_RANDOM_WAIT_MIN_MS,
            ZOO_NPC_RANDOM_WAIT_MAX_MS + 1
          );

          zooNpcSetState(i, ZOO_NPC_STATE_WAIT_RANDOM, nowMs);
        } else {
          zooNpcBeginRoamArea(i, nowMs);
        }

        continue;
      }

      zooNpcUpdateRoadMovement(i, nowMs);
      continue;
    }

    if (npc->state == ZOO_NPC_STATE_WAIT_RANDOM) {
      npc->animStep = 0;

      if (nowMs >= npc->waitUntilMs) {
        zooNpcSetState(i, ZOO_NPC_STATE_PICK_NEXT, nowMs);
      }

      continue;
    }

    if (npc->state == ZOO_NPC_STATE_ROAM_MOVING) {
      if (nowMs - npc->roamStartMs >= ZOO_NPC_ROAM_TOTAL_MS) {
        zooNpcFinishRoamArea(i, nowMs);
        continue;
      }

      zooNpcUpdateRoamMovement(i, nowMs);
      continue;
    }

    if (npc->state == ZOO_NPC_STATE_ROAM_RESTING) {
      npc->animStep = 0;

      if (nowMs - npc->roamStartMs >= ZOO_NPC_ROAM_TOTAL_MS) {
        zooNpcFinishRoamArea(i, nowMs);
        continue;
      }

      if (nowMs >= npc->roamRestUntilMs) {
        zooNpcPickNextRoamPoint(i, nowMs);
      }

      continue;
    }
  }
}

static bool zooNpcSpawnCellUsed(int col, int row) {
  if (col == zooCurrentCol && row == zooCurrentRow) {
    return true;
  }

  for (int i = 0; i < zooNpcCount; i++) {
    if (zooNpcs[i].currentCol == col &&
        zooNpcs[i].currentRow == row) {
      return true;
    }
  }

  return false;
}

static void zooAddNpc(uint8_t characterIndex, unsigned long nowMs) {
  if (zooNpcCount >= ZOO_NPC_COUNT) return;

  ZooNpc* npc = &zooNpcs[zooNpcCount];

  npc->characterIndex = characterIndex;

  int col = 1;
  int row = 1;

  for (int tries = 0; tries < 200; tries++) {
    int c = random(ZOO_MAP_COLS);
    int r = random(ZOO_MAP_ROWS);

    if (!zooIsRoadCell(c, r)) continue;
    if (zooNpcSpawnCellUsed(c, r)) continue;

    col = c;
    row = r;
    break;
  }

  npc->currentCol = col;
  npc->currentRow = row;

  npc->finalCol = col;
  npc->finalRow = row;

  npc->nextCol = col;
  npc->nextRow = row;

  npc->dir = random(4);
  npc->animStep = random(2);

  npc->x = zooCellCenterX(col, row) + zooGetPlayerLaneOffsetX(npc->dir);
  npc->y = zooCellCenterY(col, row) + zooGetPlayerLaneOffsetY(npc->dir);

  npc->moveTargetX = npc->x;
  npc->moveTargetY = npc->y;

  npc->hasMoveTarget = false;

  npc->behavior = ZOO_NPC_BEHAVIOR_RANDOM;
  npc->state = ZOO_NPC_STATE_PICK_NEXT;

  npc->roamAreaIndex = ZOO_PLAYER_CUTE_AREA_INDEX;
  npc->roamTargetX = npc->x;
  npc->roamTargetY = npc->y;

  npc->stateStartMs = nowMs;
  npc->waitUntilMs = nowMs;
  npc->roamStartMs = nowMs;
  npc->roamRestUntilMs = 0;

  npc->lastMoveMs = nowMs + random(0, 120);
  npc->lastAnimMs = nowMs + random(0, 240);

  zooNpcCount++;
}

static void initZooNpcs() {
  zooNpcCount = 0;

  unsigned long nowMs = millis();

  for (uint8_t i = 0; i < ZOO_CHARACTER_COUNT; i++) {
    if (i == zooPlayerCharacterIndex) continue;

    zooAddNpc(i, nowMs);
  }
}


static void zooSetPlayerState(uint8_t newState, unsigned long nowMs) {
  zooPlayerState = newState;
  zooPlayerStateStartMs = nowMs;
}

static bool zooPlayerArrivedFinalTarget() {
  return !zooHasMoveTarget &&
         zooCurrentCol == zooFinalCol &&
         zooCurrentRow == zooFinalRow;
}

static void zooStartRandomPlayerTarget(unsigned long nowMs) {
  zooPlayerBehavior = ZOO_PLAYER_BEHAVIOR_RANDOM;

  if (!zooPickRandomRoadTarget()) {
    zooSetPlayerState(ZOO_PLAYER_STATE_PICK_NEXT, nowMs);
    return;
  }

  zooHasMoveTarget = false;
  zooSetPlayerState(ZOO_PLAYER_STATE_MOVING, nowMs);
}


static void zooStartObservePlayerTarget(unsigned long nowMs) {
  zooPlayerBehavior = ZOO_PLAYER_BEHAVIOR_TARGET;

  int index = random(ZOO_PLAYER_OBSERVE_TARGET_COUNT);
  //int index = random(6, 11);  // 手動調整觀察區


  zooPlayerObserveArea = zooPlayerObserveTargetArea[index];

  int targetCol = zooCurrentCol;
  int targetRow = zooCurrentRow;

  if (!zooFindNearestRoadCellToWorld(
        zooPlayerObserveTargetX[index],
        zooPlayerObserveTargetY[index],
        &targetCol,
        &targetRow
      )) {
    zooSetPlayerState(ZOO_PLAYER_STATE_PICK_NEXT, nowMs);
    return;
  }

  zooFinalCol = targetCol;
  zooFinalRow = targetRow;

  zooHasMoveTarget = false;

  zooCameraFollowMode = ZOO_CAMERA_FOLLOW_PLAYER;
  zooCameraFollowAnimalIndex = -1;

  if (zooCurrentCol == zooFinalCol &&
      zooCurrentRow == zooFinalRow) {
    zooSetPlayerState(ZOO_PLAYER_STATE_WAIT_TARGET_BEFORE_CAM, nowMs);
  } else {
    zooSetPlayerState(ZOO_PLAYER_STATE_MOVING, nowMs);
  }
}


static void zooGetAnimalAreaCenter(uint8_t areaIndex, int* outX, int* outY) {
  const ZooAnimalArea* area = &zooAnimalAreas[areaIndex];

  *outX = (area->ux + area->rx + area->dx + area->lx) / 4;
  *outY = (area->uy + area->ry + area->dy + area->ly) / 4;
}

static void zooSetPlayerNearestRoadLogicPosition() {
  int col = zooCurrentCol;
  int row = zooCurrentRow;

  if (zooFindNearestRoadCellToWorld(zooPlayerX, zooPlayerY, &col, &row)) {
    zooCurrentCol = col;
    zooCurrentRow = row;

    zooFinalCol = col;
    zooFinalRow = row;

    zooNextCol = col;
    zooNextRow = row;

    zooHasMoveTarget = false;
  }
}

static bool zooPlayerRoamAreaTimeUp(unsigned long nowMs) {
  return nowMs - zooPlayerRoamAreaStartMs >= ZOO_PLAYER_CUTE_AREA_TOTAL_MS;
}

static void zooStartRoamAreaRest(unsigned long nowMs) {
  zooAnimStep = 0;

  zooPlayerRoamAreaRestUntilMs = nowMs + random(
    ZOO_PLAYER_CUTE_AREA_REST_MIN_MS,
    ZOO_PLAYER_CUTE_AREA_REST_MAX_MS + 1
  );

  zooSetPlayerState(ZOO_PLAYER_STATE_CUTE_AREA_RESTING, nowMs);
}

static void zooPickNextRoamAreaPoint(unsigned long nowMs) {
  zooPickAnimalPointInArea(
    zooPlayerRoamAreaIndex,
    &zooPlayerRoamTargetX,
    &zooPlayerRoamTargetY
  );

  int dx = zooPlayerRoamTargetX - zooPlayerX;
  int dy = zooPlayerRoamTargetY - zooPlayerY;

  if (dx == 0 && dy == 0) {
    zooStartRoamAreaRest(nowMs);
    return;
  }

  zooDir = zooGetDirFromVector(dx, dy);

  zooSetPlayerState(ZOO_PLAYER_STATE_CUTE_AREA_MOVING, nowMs);
}

static void zooBeginRoamArea(unsigned long nowMs) {
  zooPlayerRoamAreaStartMs = nowMs;
  zooPlayerRoamAreaRestUntilMs = 0;

  zooCameraFollowMode = ZOO_CAMERA_FOLLOW_PLAYER;
  zooCameraFollowAnimalIndex = -1;

  zooHasMoveTarget = false;

  zooPickNextRoamAreaPoint(nowMs);
}

static void zooFinishRoamArea(unsigned long nowMs) {
  zooCameraFollowMode = ZOO_CAMERA_FOLLOW_PLAYER;
  zooCameraFollowAnimalIndex = -1;

  zooSetPlayerNearestRoadLogicPosition();

  zooSetPlayerState(ZOO_PLAYER_STATE_PICK_NEXT, nowMs);
}

static void zooStartRoamAreaPlayerTarget(
  unsigned long nowMs,
  uint8_t behavior,
  uint8_t areaIndex
) {
  zooPlayerBehavior = behavior;
  zooPlayerRoamAreaIndex = areaIndex;

  zooCameraFollowMode = ZOO_CAMERA_FOLLOW_PLAYER;
  zooCameraFollowAnimalIndex = -1;

  int areaCenterX = 0;
  int areaCenterY = 0;

  zooGetAnimalAreaCenter(
    areaIndex,
    &areaCenterX,
    &areaCenterY
  );

  int targetCol = zooCurrentCol;
  int targetRow = zooCurrentRow;

  if (!zooFindNearestRoadCellToWorld(
        areaCenterX,
        areaCenterY,
        &targetCol,
        &targetRow
      )) {
    zooBeginRoamArea(nowMs);
    return;
  }

  zooFinalCol = targetCol;
  zooFinalRow = targetRow;

  zooHasMoveTarget = false;

  if (zooCurrentCol == zooFinalCol &&
      zooCurrentRow == zooFinalRow) {
    zooBeginRoamArea(nowMs);
  } else {
    zooSetPlayerState(ZOO_PLAYER_STATE_MOVING, nowMs);
  }
}

static void zooStartCuteAreaPlayerTarget(unsigned long nowMs) {
  zooStartRoamAreaPlayerTarget(
    nowMs,
    ZOO_PLAYER_BEHAVIOR_CUTE_AREA,
    ZOO_PLAYER_CUTE_AREA_INDEX
  );
}

static void zooStartShopAreaPlayerTarget(unsigned long nowMs) {
  zooStartRoamAreaPlayerTarget(
    nowMs,
    ZOO_PLAYER_BEHAVIOR_SHOP_AREA,
    ZOO_PLAYER_SHOP_AREA_INDEX
  );
}

static void updateZooRoamAreaMovement(unsigned long nowMs) {
  if (nowMs - zooLastAnimMs >= ZOO_ANIM_INTERVAL_MS) {
    zooLastAnimMs = nowMs;
    zooAnimStep = !zooAnimStep;
  }

  if (nowMs - zooLastMoveMs < ZOO_MOVE_INTERVAL_MS) {
    return;
  }

  zooLastMoveMs = nowMs;

  int dx = zooPlayerRoamTargetX - zooPlayerX;
  int dy = zooPlayerRoamTargetY - zooPlayerY;

  zooDir = zooGetDirFromVector(dx, dy);

  int sx = zooLimitedStep(dx, ZOO_MOVE_STEP_X);
  int sy = zooLimitedStep(dy, ZOO_MOVE_STEP_Y);

  zooPlayerX += sx;
  zooPlayerY += sy;

  if (zooPlayerX == zooPlayerRoamTargetX &&
      zooPlayerY == zooPlayerRoamTargetY) {
    zooStartRoamAreaRest(nowMs);
  }
}



static void zooPickNextPlayerBehavior(unsigned long nowMs) {
  zooCameraFollowMode = ZOO_CAMERA_FOLLOW_PLAYER;
  zooCameraFollowAnimalIndex = -1;

  int pick = random(100);

  if (pick < ZOO_PLAYER_SHOP_AREA_CHANCE_PERCENT) {
    zooStartShopAreaPlayerTarget(nowMs);
    return;
  }

  pick -= ZOO_PLAYER_SHOP_AREA_CHANCE_PERCENT;

  if (pick < ZOO_PLAYER_CUTE_AREA_CHANCE_PERCENT) {
    zooStartCuteAreaPlayerTarget(nowMs);
    return;
  }

  pick -= ZOO_PLAYER_CUTE_AREA_CHANCE_PERCENT;

  if (pick < ZOO_PLAYER_TARGET_CHANCE_PERCENT) {
    zooStartObservePlayerTarget(nowMs);
    return;
  }

  zooStartRandomPlayerTarget(nowMs);
}

static void updateZooPlayerBehavior(unsigned long nowMs) {
  if (zooPlayerState == ZOO_PLAYER_STATE_PICK_NEXT) {
    zooPickNextPlayerBehavior(nowMs);
    return;
  }

if (zooPlayerState == ZOO_PLAYER_STATE_MOVING) {
  if (zooPlayerArrivedFinalTarget()) {
    if (zooPlayerBehavior == ZOO_PLAYER_BEHAVIOR_RANDOM) {
      zooSetPlayerState(ZOO_PLAYER_STATE_WAIT_RANDOM, nowMs);
    } else if (zooPlayerBehavior == ZOO_PLAYER_BEHAVIOR_TARGET) {
      zooSetPlayerState(ZOO_PLAYER_STATE_WAIT_TARGET_BEFORE_CAM, nowMs);
} else if (zooPlayerBehavior == ZOO_PLAYER_BEHAVIOR_CUTE_AREA ||
           zooPlayerBehavior == ZOO_PLAYER_BEHAVIOR_SHOP_AREA) {
  zooBeginRoamArea(nowMs);
}

    return;
  }

  updateZooMovement(nowMs);
  return;
}

  if (zooPlayerState == ZOO_PLAYER_STATE_WAIT_RANDOM) {
    if (nowMs - zooPlayerStateStartMs >= ZOO_PLAYER_RANDOM_WAIT_MS) {
      zooSetPlayerState(ZOO_PLAYER_STATE_PICK_NEXT, nowMs);
    }

    return;
  }


if (zooPlayerState == ZOO_PLAYER_STATE_CUTE_AREA_MOVING) {
  if (zooPlayerRoamAreaTimeUp(nowMs)) {
    zooFinishRoamArea(nowMs);
    return;
  }

  updateZooRoamAreaMovement(nowMs);
  return;
}

if (zooPlayerState == ZOO_PLAYER_STATE_CUTE_AREA_RESTING) {
  if (zooPlayerRoamAreaTimeUp(nowMs)) {
    zooFinishRoamArea(nowMs);
    return;
  }

  if (nowMs >= zooPlayerRoamAreaRestUntilMs) {
    zooPickNextRoamAreaPoint(nowMs);
  }

  return;
}

if (zooPlayerState == ZOO_PLAYER_STATE_WAIT_TARGET_BEFORE_CAM) {
  if (nowMs - zooPlayerStateStartMs >= ZOO_PLAYER_TARGET_BEFORE_CAM_WAIT_MS) {
    zooCameraFollowAnimalIndex = zooPickAnimalIndexInArea(zooPlayerObserveArea);

    if (zooCameraFollowAnimalIndex >= 0) {
      zooCameraFollowMode = ZOO_CAMERA_FOLLOW_ANIMAL;
    } else {
      zooCameraFollowMode = ZOO_CAMERA_FOLLOW_PLAYER;
    }

    // 開始平滑移動到動物，不立刻開始計算觀察 15 秒
    zooSetPlayerState(ZOO_PLAYER_STATE_CAM_PAN_LEFT, nowMs);
  }

  return;
}

if (zooPlayerState == ZOO_PLAYER_STATE_CAM_PAN_LEFT) {
  // Camera 到達動物附近後，才開始 15 秒觀察
  if (zooCameraAtTarget()) {
    zooSetPlayerState(ZOO_PLAYER_STATE_OBSERVE_ANIMAL, nowMs);
  }

  return;
}

if (zooPlayerState == ZOO_PLAYER_STATE_OBSERVE_ANIMAL) {
  if (nowMs - zooPlayerStateStartMs >= ZOO_PLAYER_OBSERVE_WAIT_MS) {
    zooCameraFollowMode = ZOO_CAMERA_FOLLOW_PLAYER;
    zooCameraFollowAnimalIndex = -1;

    // 開始平滑回到主角
    zooSetPlayerState(ZOO_PLAYER_STATE_CAM_RETURN, nowMs);
  }

  return;
}

if (zooPlayerState == ZOO_PLAYER_STATE_CAM_RETURN) {
  // Camera 回到主角附近後，才開始最後等待 3 秒
  if (zooCameraAtTarget()) {
    zooSetPlayerState(ZOO_PLAYER_STATE_WAIT_TARGET_AFTER_CAM, nowMs);
  }

  return;
}






  if (zooPlayerState == ZOO_PLAYER_STATE_WAIT_TARGET_AFTER_CAM) {
    if (nowMs - zooPlayerStateStartMs >= ZOO_PLAYER_TARGET_AFTER_CAM_WAIT_MS) {
      zooSetPlayerState(ZOO_PLAYER_STATE_PICK_NEXT, nowMs);
    }

    return;
  }
}



// ============================================================
// 更新
// ============================================================

static void updateZooMovement(unsigned long nowMs) {
  if (nowMs - zooLastAnimMs >= ZOO_ANIM_INTERVAL_MS) {
    zooLastAnimMs = nowMs;
    zooAnimStep = !zooAnimStep;
  }

  if (nowMs - zooLastMoveMs < ZOO_MOVE_INTERVAL_MS) {
    return;
  }

  zooLastMoveMs = nowMs;

  if (!zooHasMoveTarget) {
    zooPrepareNextCellMove();
  }

  if (!zooHasMoveTarget) {
    return;
  }

  int dx = zooMoveTargetX - zooPlayerX;
  int dy = zooMoveTargetY - zooPlayerY;

  //zooDir = zooGetDirFromVector(dx, dy);

  int sx = zooLimitedStep(dx, ZOO_MOVE_STEP_X);
  int sy = zooLimitedStep(dy, ZOO_MOVE_STEP_Y);

  zooPlayerX += sx;
  zooPlayerY += sy;

  if (zooPlayerX == zooMoveTargetX &&
      zooPlayerY == zooMoveTargetY) {
    zooCurrentCol = zooNextCol;
    zooCurrentRow = zooNextRow;
    zooHasMoveTarget = false;
  }
}

static void updateZooCamera() {
  int targetCameraX = 0;
  int targetCameraY = 0;

  zooComputeCameraTarget(&targetCameraX, &targetCameraY);

  int dx = targetCameraX - zooCameraX;
  int dy = targetCameraY - zooCameraY;

  zooCameraX += zooLimitedStep(dx, ZOO_CAMERA_FOLLOW_STEP_X);
  zooCameraY += zooLimitedStep(dy, ZOO_CAMERA_FOLLOW_STEP_Y);
}

// ============================================================
// 繪製
// ============================================================

static void drawZooMap() {
  for (int r = 0; r < ZOO_MAP_ROWS; r++) {
    for (int c = 0; c < ZOO_MAP_COLS; c++) {
      uint8_t tileCode = zooGetTileCode(c, r);

      int frame = zooGetTileFrame(tileCode);
      bool flipX = zooGetTileFlipX(tileCode);

      int x = zooTileX(c, r) - zooCameraX;
      int y = zooTileY(c, r) - zooCameraY;

drawZooRgb565Frame(
  ZOO_TILE,
  ZOO_TILE_PALETTE,
  ZOO_TILE_SHEET_W,
  ZOO_TILE_FRAME_W,
  ZOO_TILE_FRAME_H,
  frame,
  flipX,
  x,
  y
);
    }
  }
}

static void drawZooPlayer() {
  drawZooCharacterFrame(
    zooPlayerCharacterIndex,
    zooPlayerX,
    zooPlayerY,
    zooDir,
    zooAnimStep
  );
}

static void drawZooNpcs() {
  bool drawn[ZOO_NPC_COUNT];

  for (int i = 0; i < ZOO_NPC_COUNT; i++) {
    drawn[i] = false;
  }

  for (int pass = 0; pass < zooNpcCount; pass++) {
    int bestIndex = -1;
    int bestY = 32767;

    for (int i = 0; i < zooNpcCount; i++) {
      if (drawn[i]) continue;

      if (zooNpcs[i].y < bestY) {
        bestY = zooNpcs[i].y;
        bestIndex = i;
      }
    }

    if (bestIndex >= 0) {
      ZooNpc* npc = &zooNpcs[bestIndex];

      drawZooCharacterFrame(
        npc->characterIndex,
        npc->x,
        npc->y,
        npc->dir,
        npc->animStep
      );

      drawn[bestIndex] = true;
    }
  }
}


static void renderZooScene() {
  display.fillScreen(ZOO_BG_COLOR);

  drawZooMap();
  drawZooObjects();
  drawZoo_Fence_Up();
  drawZooAnimals();
  drawZoo_Fence_Down();
  drawZooNpcs();
  drawZooPlayer();
  
  drawThemeClockText();
}

// ============================================================
// Init / Main
// ============================================================

void ZooModeInit() {
  zooCurrentCol = 1;
  zooCurrentRow = 1;

zooFinalCol = zooCurrentCol;
zooFinalRow = zooCurrentRow;

  zooNextCol = zooCurrentCol;
  zooNextRow = zooCurrentRow;

zooDir = ZOO_DIR_DOWN;

zooPlayerX = zooCellCenterX(zooCurrentCol, zooCurrentRow)
           + zooGetPlayerLaneOffsetX(zooDir);

zooPlayerY = zooCellCenterY(zooCurrentCol, zooCurrentRow)
           + zooGetPlayerLaneOffsetY(zooDir);

  zooMoveTargetX = zooPlayerX;
  zooMoveTargetY = zooPlayerY;

  zooHasMoveTarget = false;

 
zooAnimStep = 0;

// init 時隨機抽一個角色當主角
zooPlayerCharacterIndex = random(ZOO_CHARACTER_COUNT);

zooLastMoveMs = millis();
zooLastAnimMs = millis();

zooPlayerState = ZOO_PLAYER_STATE_PICK_NEXT;
zooPlayerBehavior = ZOO_PLAYER_BEHAVIOR_RANDOM;
zooPlayerStateStartMs = millis();


zooCameraFollowMode = ZOO_CAMERA_FOLLOW_PLAYER;
zooCameraFollowAnimalIndex = -1;
zooPlayerObserveArea = 0;



initZooAnimals();
initZooNpcs();
zooSnapCameraToTarget();

zooInited = true;
}





void ZooMode() {
  if (ModefirstRun) {
    zooInited = false;
    ModefirstRun = false;
  }

  if (!zooInited) {
    ZooModeInit();
  }

  unsigned long nowMs = millis();

updateZooPlayerBehavior(nowMs);
updateZooNpcs(nowMs);
updateZooAnimals(nowMs);
updateZooCamera();
renderZooScene();

  wait_with_display(30);
}
