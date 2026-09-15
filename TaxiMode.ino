#include "Taxi.h"

static const int TAXI_SCR_W = 64;
static const int TAXI_SCR_H = 64;
static const int TAXI_ROAD_SHEET_W = 704;
static const int TAXI_ROAD_TILE_W  = 64;
static const int TAXI_ROAD_TILE_H  = 32;
static const int TAXI_TAXI_SHEET_W = 76;
static const int TAXI_TAXI_FRAME_W = 38;
static const int TAXI_TAXI_FRAME_H = 22;
static const int TAXI_POLICE_SHEET_W = 72;
static const int TAXI_POLICE_FRAME_W = 36;
static const int TAXI_POLICE_FRAME_H = 22;


static const int TAXI_BOY_SHEET_W = 42;
static const int TAXI_BOY_FRAME_W = 21;
static const int TAXI_BOY_FRAME_H = 27;

// 直升機圖資：172 x 30，共 4 格。
// frame 0 / 1 是第一組方向的兩格動畫；
// frame 2 / 3 是第二組方向的兩格動畫。
static const int TAXI_AIR_SHEET_W = 172;
static const int TAXI_AIR_FRAME_W = 43;
static const int TAXI_AIR_FRAME_H = 30;
static const int TAXI_AIR_FRAME_COUNT = 4;

static const int TAXI_OBJ_SHEET_W = 832;
static const int TAXI_OBJ_TILE_W  = 64;
static const int TAXI_OBJ_TILE_H  = 32;
static const int TAXI_PASSENGER_SHEET_W = 90;
static const int TAXI_PASSENGER_FRAME_W = 15;
static const int TAXI_PASSENGER_FRAME_H = 25;
static const int TAXI_PASSENGER_FRAME_COUNT = 6;
static const int TAXI_WORLD_W = 704;
static const int TAXI_WORLD_H = 320;


static const uint16_t TAXI_BG_COLOR = 0x2A49;

// CAM 跟隨車子的 Y 軸偏移量。數值越大，畫面會越往下看，車子在畫面中會越偏上。
static const int TAXI_CAMERA_FOLLOW_OFFSET_Y = 20;

// 主角計程車的車輛槽位。
static const int TAXI_MAIN_CAR_SLOT = 0;

// 警車的車輛槽位。
static const int TAXI_POLICE_CAR_SLOT = 1;

// CITY_BOY 的車輛槽位。
static const int TAXI_BOY_CAR_SLOT = 2;

// 直升機不使用道路車輛槽位。
// 它可在整個城市範圍內自由飛行，因此使用獨立的位置與目標資料。


// 場上同時出現的候車乘客數量  最大16個
static const int TAXI_PASSENGER_ACTIVE_COUNT = 8;

// 計程車與乘客的上車觸發範圍 X 軸半徑。
// 判定點是「乘客腳底中心」對「車子的行車點」。
// 數值越大，左右距離越遠也會觸發。
static const int TAXI_PASSENGER_PICKUP_RADIUS_X = 8;

// 計程車與乘客的上車觸發範圍 Y 軸半徑。
// 判定點是「乘客腳底中心」對「車子的行車點」。
// 數值越大，上下距離越遠也會觸發。
static const int TAXI_PASSENGER_PICKUP_RADIUS_Y = 4;


// 乘客圖資碰撞的左右擴張量。
// 0 = 完全使用乘客原圖資的非透明像素。
// 如果車子明明碰到乘客卻不觸發，可改 1 或 2。
static const int TAXI_PASSENGER_COLLISION_EXTRA_X = 0;


// 下車保底判定範圍。
// 只有在前往目的地時使用，避免車子已經到目的地附近，
// 但剛好沒有跟虛擬乘客圖資像素碰撞，導致永遠不下車。
static const int TAXI_DROPOFF_FALLBACK_EXTRA_X = 4;
static const int TAXI_DROPOFF_FALLBACK_EXTRA_Y = 4;

// 下車判定：目的地乘客點進入畫面時，額外放寬多少像素。
// 這不是碰撞框，只是讓下車流程更穩定，不要在目的地附近徘徊。
static const int TAXI_DROPOFF_SCREEN_MARGIN = 4;




// 下車判定：車子的行車點需要靠近乘客腳底。
// 乘客腳底就是人物底部中心，這個點最接近道路邊。
static const int TAXI_DROPOFF_READY_DISTANCE_X = 28;
static const int TAXI_DROPOFF_READY_DISTANCE_Y = 14;

// 下車保底時間。
// 如果尋路太久仍沒到合適位置，就強制下車，避免 LOOP 卡死。
static const unsigned long TAXI_DROPOFF_FORCE_MS = 60000UL;

// 乘客上車演出時，乘客顯示/等待的時間。
// 單位：毫秒，2000UL = 2 秒。
static const unsigned long TAXI_PASSENGER_BOARD_WAIT_MS = 2000UL;

// 下車乘客圖片多久後被清掉
static const unsigned long TAXI_PASSENGER_DROPOFF_WAIT_MS = 5000UL;

// 乘客下車出現後，車子原地等待多久才開走。
static const unsigned long TAXI_DROPOFF_CAR_WAIT_MS = 3000UL;


// 乘客下車出現後，CAM 滑到乘客身上的時間。
// 建議小於或等於 TAXI_DROPOFF_CAR_WAIT_MS，
// 這樣車子還沒開走前，可以觀察車子與乘客距離。
static const unsigned long TAXI_DROPOFF_CAMERA_TO_PASSENGER_MS = 2000UL;


// 乘客上車後，計程車先自由巡航多久才開始前往目的地。
static const unsigned long TAXI_PASSENGER_RIDE_CRUISE_MS = 16000UL;

// 乘客下車後，計程車自由巡航多久才重新生成下一批乘客。
static const unsigned long TAXI_PASSENGER_AFTER_DROPOFF_CRUISE_MS = 16000UL;

// 上車演出 CAM 時間
static const unsigned long TAXI_PICKUP_CAMERA_TO_CENTER_MS = 2000UL;  // CAM 移到車與乘客中心
static const unsigned long TAXI_PICKUP_AFTER_GONE_WAIT_MS = 2000UL;   // 乘客消失後等待
static const unsigned long TAXI_PICKUP_CAMERA_RETURN_MS = 2000UL;     // CAM 移回正常視角
static const unsigned long TAXI_PICKUP_READY_WAIT_MS = 3000UL;        // CAM 回來後再等一下才開車
static const int TAXI_PICKUP_CAMERA_CENTER_OFFSET_Y = 20;              // 上車特寫畫面 Y 偏移


// 啟動模式。
// 0 = 計程車載客模式
// 1 = 警車追逐模式
// 2 = 直升機巡航模式
// 3 = 開場隨機 TAXI / 警車 / 直升機模式
static const uint8_t TAXI_START_GAME_MODE = 3;

// 警車模式：開場先正常巡航多久。15000UL
static const unsigned long TAXI_POLICE_INTRO_PATROL_MS = 15000UL;

// 警車模式：CAM 滑到 BOY 或警車的時間。
static const unsigned long TAXI_POLICE_CAMERA_MOVE_MS = 3000UL;

// 警車模式：CAM 聚焦 BOY 多久。
static const unsigned long TAXI_POLICE_BOY_FOCUS_MS = 3000UL;

// 警車模式：開始追蹤 BOY 後，追多久才進入逮捕/逃走結果。16000UL
static const unsigned long TAXI_POLICE_CHASE_MS = 16000UL;

// 逮捕演出：BOY 與警車停止多久。
static const unsigned long TAXI_POLICE_ARREST_STOP_MS = 2000UL;

// 逮捕演出：CAM 聚焦兩者中間多久。
static const unsigned long TAXI_POLICE_ARREST_FOCUS_MS = 2000UL;

// 逮捕演出：BOY 閃爍多久後消失。
static const unsigned long TAXI_POLICE_ARREST_BLINK_MS = 2000UL;

// 逮捕演出：BOY 消失後，警車等待多久回到 LOOP。
static const unsigned long TAXI_POLICE_ARREST_AFTER_GONE_MS = 2000UL;

// 逃走演出：BOY 逃跑時，CAM 聚焦 BOY 多久。
static const unsigned long TAXI_POLICE_ESCAPE_BOY_FOCUS_MS = 3000UL;

// 逃走演出：警車放棄追蹤後，等待多久回到 LOOP。
static const unsigned long TAXI_POLICE_ESCAPE_COOLDOWN_MS = 15000UL;

// BOY 閃爍速度。
static const unsigned long TAXI_POLICE_BOY_BLINK_INTERVAL_MS = 180UL;

// =====================================================
// 直升機模式設定
// =====================================================

// 直升機抵達隨機地點後停留的時間。
// 15000UL = 15 秒。
static const unsigned long TAXI_HELICOPTER_WAIT_MS = 15000UL;

// 直升機位置更新間隔。
// 數值越小，移動越流暢、速度也越快。
static const unsigned long TAXI_HELICOPTER_MOVE_INTERVAL_MS = 100UL;

// 直升機每次更新時，X 與 Y 軸最多移動的像素。
static const int TAXI_HELICOPTER_STEP_PX = 1;

// 直升機兩格動畫的切換速度。
static const unsigned long TAXI_HELICOPTER_ANIM_INTERVAL_MS = 180UL;

// 選擇新目標時，避免目標太接近目前位置。
static const int TAXI_HELICOPTER_MIN_TARGET_DISTANCE = 96;

// 直升機 CAM 使用置中視角，不使用道路車輛的向下觀看偏移。
static const int TAXI_HELICOPTER_CAMERA_OFFSET_Y = 10;


enum TaxiSpriteId {
  TAXI_SPRITE_TAXI = 0,
  TAXI_SPRITE_POLICE = 1,
  TAXI_SPRITE_BOY = 2
};

enum TaxiCarDir {
  TAXI_DIR_UP = 0,
  TAXI_DIR_DOWN = 1,
  TAXI_DIR_LEFT = 2,
  TAXI_DIR_RIGHT = 3
};

struct TaxiCarSpriteDef {
  const uint8_t* bitmap;
  const uint16_t* palette;
  int sheetW;
  int frameW;
  int frameH;
  int anchorX;
  int anchorY;
};

static const TaxiCarSpriteDef taxiCarSpriteDefs[] = {
  {
    CAR_TAXI,
    CAR_TAXI_PALETTE,
    TAXI_TAXI_SHEET_W,
    TAXI_TAXI_FRAME_W,
    TAXI_TAXI_FRAME_H,
    TAXI_TAXI_FRAME_W / 2,
    20
  },
  {
    CAR_POLICE,
    CAR_POLICE_PALETTE,
    TAXI_POLICE_SHEET_W,
    TAXI_POLICE_FRAME_W,
    TAXI_POLICE_FRAME_H,
    TAXI_POLICE_FRAME_W / 2,
    20
  },
  {
    CITY_BOY,
    CITY_BOY_PALETTE,
    TAXI_BOY_SHEET_W,
    TAXI_BOY_FRAME_W,
    TAXI_BOY_FRAME_H,
    TAXI_BOY_FRAME_W / 2,
    24
  }
};

static const int TAXI_CAR_SPRITE_DEF_COUNT =
  sizeof(taxiCarSpriteDefs) / sizeof(taxiCarSpriteDefs[0]);

struct TaxiVehicle {
  uint8_t spriteId;
  int worldX;
  int worldY;
  int currentRoadIndex;
  int targetRoadIndex;
  int previousRoadIndex;
  uint8_t dir;
  unsigned long lastMoveMs;
  unsigned long moveIntervalMs;
  int stepX;
  int stepY;
};

// 車輛總數。
static const int TAXI_VEHICLE_COUNT = 3;

// 所有車輛統一移動速度。 數值越小，車子越快；數值越大，車子越慢。
static const unsigned long TAXI_VEHICLE_MOVE_INTERVAL_MS = 100UL;

// 卡死重新初始化時間。
// 目前模式的主體車輛如果連續 2 分鐘完全沒有移動，
// 就重新執行 TaxiModeInit()，避免主題時鐘畫面長時間靜止。
// 120000UL = 2 分鐘。
static const unsigned long TAXI_STALL_REINIT_MS = 120000UL;

// 所有車輛每次移動的 X / Y 步距。
// 目前道路是斜向，所以 X:Y 建議維持 2:1。
static const int TAXI_VEHICLE_STEP_X = 2;
static const int TAXI_VEHICLE_STEP_Y = 1;


// 車輛資料陣列。
// 每一台車的位置、目標道路、方向、速度都存在這裡。
static TaxiVehicle taxiVehicles[TAXI_VEHICLE_COUNT];

// =====================================================
// 車輛卡死監控
//
// 每個模式只監控該模式的主角：
// 1. 載客模式監控主 TAXI。
// 2. 警車模式監控警車。
// 3. 直升機模式監控直升機。
//
// 背景交通工具即使暫停或互相阻塞，也不會觸發重新初始化。
// 正常上下車、CAM 演出、逮捕演出與直升機等待都遠短於 2 分鐘，
// 因此不會因短暫停車而重新初始化。
// =====================================================

static int taxiStallLastVehicleX[TAXI_VEHICLE_COUNT];
static int taxiStallLastVehicleY[TAXI_VEHICLE_COUNT];
static unsigned long taxiStallLastMoveMs[TAXI_VEHICLE_COUNT];

// 直升機使用獨立位置，因此卡死監控也獨立記錄。
static int taxiStallLastHelicopterX = 0;
static int taxiStallLastHelicopterY = 0;
static unsigned long taxiStallHelicopterLastMoveMs = 0;

static bool taxiStallMonitorReady = false;

// 車子在不同方向行駛時的車道 X/Y 偏移量。
// 這些數值會影響車子畫在道路上的位置，也會影響 CAM 跟隨位置與排序位置。

// RIGHT / UP 方向使用的車道偏移。
// 主要用在往右上方向行駛時的視覺位置。
static const int TAXI_LANE_RIGHT_UP_X = 3;
static const int TAXI_LANE_RIGHT_UP_Y = 10;

// RIGHT / DOWN 方向使用的車道偏移。
// 主要用在往右下方向行駛時的視覺位置。
static const int TAXI_LANE_RIGHT_DOWN_X = -3;
static const int TAXI_LANE_RIGHT_DOWN_Y = 10;

// LEFT / UP 方向使用的車道偏移。
// 主要用在往左上方向行駛時的視覺位置。
static const int TAXI_LANE_LEFT_UP_X = 3;
static const int TAXI_LANE_LEFT_UP_Y = -5;

// LEFT / DOWN 方向使用的車道偏移。
// 主要用在往左下方向行駛時的視覺位置。
static const int TAXI_LANE_LEFT_DOWN_X = -3;
static const int TAXI_LANE_LEFT_DOWN_Y = -5;

// 特殊轉彎的提前判定 X 軸距離。
// 目前只用在 DOWN → LEFT、UP → RIGHT。
// 數值越大，越早進入轉彎點。
static const int TAXI_EARLY_TURN_X = 16;

// 特殊轉彎的提前判定 Y 軸距離。
// 需要和 TAXI_EARLY_TURN_X 維持 2:1，因為車子移動比例是 X 每次 2、Y 每次 1。
// 例如 X = 16 時，Y 建議是 8。
static const int TAXI_EARLY_TURN_Y = 8;

// 車輛保持距離參數。
// 用在所有車輛之間，避免同方向、同路線時互相重疊。
// X/Y 建議維持約 2:1，符合車子斜向移動比例。
static const int TAXI_VEHICLE_KEEP_DISTANCE_X = 38;
static const int TAXI_VEHICLE_KEEP_DISTANCE_Y = 20;

// 極近距離保護。
// 即使不是同方向，只要車輛中心太接近，也避免高槽位車輛繼續壓上去。
static const int TAXI_VEHICLE_OVERLAP_GUARD_X = 22;
static const int TAXI_VEHICLE_OVERLAP_GUARD_Y = 12;


enum TaxiRoadTileId {
  TAXI_ROAD_H = 0,
  TAXI_ROAD_V = 1,
  TAXI_ROAD_L = 2,
  TAXI_ROAD_R = 3,
  TAXI_ROAD_U = 4,
  TAXI_ROAD_D = 5,
  TAXI_ROAD_N = 6,
  TAXI_ROAD_E = 7,
  TAXI_ROAD_M = 8,
  TAXI_ROAD_S = 9,
  TAXI_ROAD_C = 10
};

struct TaxiRoadTile {
  uint8_t tileId;
  int16_t x;
  int16_t y;
};

enum TaxiCityObjectId {
  TAXI_OBJ_A = 0,
  TAXI_OBJ_B = 1,
  TAXI_OBJ_C = 2,
  TAXI_OBJ_D = 3,
  TAXI_OBJ_E = 4,
  TAXI_OBJ_F = 5,
  TAXI_OBJ_G = 6,
  TAXI_OBJ_H = 7,
  TAXI_OBJ_I = 8,
  TAXI_OBJ_J = 9,
  TAXI_OBJ_K = 10,
  TAXI_OBJ_L = 11,
  TAXI_OBJ_M = 12
};

struct TaxiCityObjectTile {
  uint8_t objId;
  int16_t x;
  int16_t y;
  bool flipX;
};

struct TaxiPassengerPoint {
  int16_t x;
  int16_t y;
  bool flipX;
};

struct TaxiPassenger {
  bool active;
  int16_t worldX;
  int16_t worldY;
  uint8_t frameIndex;
  bool flipX;
  int pointIndex;
};

static const TaxiRoadTile taxiRoadTiles[] = {
  {TAXI_ROAD_U, 320, 0},
  {TAXI_ROAD_H, 288, 16},
  {TAXI_ROAD_V, 352, 16},
  {TAXI_ROAD_H, 256, 32},
  {TAXI_ROAD_V, 384, 32},
  {TAXI_ROAD_H, 224, 48},
  {TAXI_ROAD_E, 416, 48},
  {TAXI_ROAD_N, 192, 64},
  {TAXI_ROAD_H, 384, 64},
  {TAXI_ROAD_V, 448, 64},
  {TAXI_ROAD_H, 160, 80},
  {TAXI_ROAD_V, 224, 80},
  {TAXI_ROAD_H, 352, 80},
  {TAXI_ROAD_V, 480, 80},
  {TAXI_ROAD_H, 128, 96},
  {TAXI_ROAD_V, 256, 96},
  {TAXI_ROAD_H, 320, 96},
  {TAXI_ROAD_E, 512, 96},
  {TAXI_ROAD_N, 96, 112},
  {TAXI_ROAD_C, 288, 112},
  {TAXI_ROAD_H, 480, 112},
  {TAXI_ROAD_V, 544, 112},
  {TAXI_ROAD_H, 64, 128},
  {TAXI_ROAD_V, 128, 128},
  {TAXI_ROAD_H, 256, 128},
  {TAXI_ROAD_V, 320, 128},
  {TAXI_ROAD_H, 448, 128},
  {TAXI_ROAD_V, 576, 128},
  {TAXI_ROAD_H, 32, 144},
  {TAXI_ROAD_V, 160, 144},
  {TAXI_ROAD_H, 224, 144},
  {TAXI_ROAD_V, 352, 144},
  {TAXI_ROAD_H, 416, 144},
  {TAXI_ROAD_R, 608, 144},
  {TAXI_ROAD_L, 0, 160},
  {TAXI_ROAD_C, 192, 160},
  {TAXI_ROAD_C, 384, 160},
  {TAXI_ROAD_H, 576, 160},
  {TAXI_ROAD_V, 32, 176},
  {TAXI_ROAD_H, 160, 176},
  {TAXI_ROAD_V, 224, 176},
  {TAXI_ROAD_H, 352, 176},
  {TAXI_ROAD_V, 416, 176},
  {TAXI_ROAD_H, 544, 176},
  {TAXI_ROAD_E, 64, 192},
  {TAXI_ROAD_H, 128, 192},
  {TAXI_ROAD_V, 256, 192},
  {TAXI_ROAD_H, 320, 192},
  {TAXI_ROAD_V, 448, 192},
  {TAXI_ROAD_H, 512, 192},
  {TAXI_ROAD_H, 32, 208},
  {TAXI_ROAD_M, 96, 208},
  {TAXI_ROAD_C, 288, 208},
  {TAXI_ROAD_S, 480, 208},
  {TAXI_ROAD_L, 0, 224},
  {TAXI_ROAD_V, 128, 224},
  {TAXI_ROAD_H, 256, 224},
  {TAXI_ROAD_V, 320, 224},
  {TAXI_ROAD_H, 448, 224},
  {TAXI_ROAD_V, 32, 240},
  {TAXI_ROAD_V, 160, 240},
  {TAXI_ROAD_H, 224, 240},
  {TAXI_ROAD_V, 352, 240},
  {TAXI_ROAD_H, 416, 240},
  {TAXI_ROAD_V, 64, 256},
  {TAXI_ROAD_S, 192, 256},
  {TAXI_ROAD_D, 384, 256},
  {TAXI_ROAD_V, 96, 272},
  {TAXI_ROAD_H, 160, 272},
  {TAXI_ROAD_D, 128, 288}
};

static const int TAXI_ROAD_COUNT =
  sizeof(taxiRoadTiles) / sizeof(taxiRoadTiles[0]);

static const TaxiPassengerPoint taxiPassengerPoints[] = {
  { 35, 127, false},
  {157,  70, false},
  {217,  35, false},
  {351,  65, false},
  {313,  86, false},
  {217, 133, false},
  {217, 230, false},
  {451, 111, false},
  {531, 170, false},
  { 29, 195, false},
  {400, 229, false},
  { 89, 167, true },
  {303, 176, true },
  {466,  33, true },
  {594, 100, true },
  {114, 243, true }
};

static const int TAXI_PASSENGER_POINT_COUNT =
  sizeof(taxiPassengerPoints) / sizeof(taxiPassengerPoints[0]);

static const TaxiCityObjectTile taxiCityObjects[] = {
  {TAXI_OBJ_A, 288, -16, false},
  {TAXI_OBJ_D, 352, -16, false},
  {TAXI_OBJ_A, 256,   0, false},
  {TAXI_OBJ_D, 384,   0, false},
  {TAXI_OBJ_A, 224,  16, false},
  {TAXI_OBJ_D, 416,  16, false},

  {TAXI_OBJ_J, 192,  0, false},
  {TAXI_OBJ_E, 192,  32, false},

  {TAXI_OBJ_M, 448,  0, true },
  {TAXI_OBJ_H, 448,  32, true },
  {TAXI_OBJ_B, 160,  48, false},
  {TAXI_OBJ_D, 480,  48, false},
  {TAXI_OBJ_K, 128,  32, false},
  {TAXI_OBJ_F, 128,  64, false},
  
  {TAXI_OBJ_J, 512,  32, true },
  {TAXI_OBJ_E, 512,  64, true },
  {TAXI_OBJ_C,  96,  80, false},
  {TAXI_OBJ_D, 544,  80, false},

  {TAXI_OBJ_L,  64,  64, false},
  {TAXI_OBJ_G,  64,  96, false},
  {TAXI_OBJ_K, 576,  64, true },
  {TAXI_OBJ_F, 576,  96, true },
  {TAXI_OBJ_L,  32,  80, false},
  {TAXI_OBJ_G,  32, 112, false},
  {TAXI_OBJ_D, 608, 112, false},
  {TAXI_OBJ_M,   0, 96, false},
  {TAXI_OBJ_H,   0, 128, false},
  {TAXI_OBJ_D, 640, 128, false},
  {TAXI_OBJ_B, -32, 144, false},

  {TAXI_OBJ_I, 0,  192, false},
  {TAXI_OBJ_I, -32,  176, false},
  {TAXI_OBJ_I, -32,  208, false},

  {TAXI_OBJ_I, -32,  240, false},

  {TAXI_OBJ_A, 0,  256, false},
  {TAXI_OBJ_I, 32,  272, false},
  {TAXI_OBJ_A, 64,  288, false},
  {TAXI_OBJ_I, 160,  304, false},

  {TAXI_OBJ_A, 640,  160, false},
  {TAXI_OBJ_I, 608,  176, false},
  {TAXI_OBJ_A, 576,  192, false},
  {TAXI_OBJ_I, 544,  208, false},
  {TAXI_OBJ_A, 512,  224, false},
  {TAXI_OBJ_I, 480,  240, false},

  {TAXI_OBJ_A, 448,  256, false},
  {TAXI_OBJ_I, 416,  272, false},
  {TAXI_OBJ_I, 352,  272, false},
  {TAXI_OBJ_A, 384,  288, false},

  {TAXI_OBJ_A, 288,  240, false},
  {TAXI_OBJ_A, 256,  256, false},
  {TAXI_OBJ_A, 320,  256, false},
  {TAXI_OBJ_I, 224,  272, false},
  {TAXI_OBJ_A, 192,  288, false},

  // 空地
  {TAXI_OBJ_I, 320,  32, false},
  {TAXI_OBJ_I, 288,  48, false},
  {TAXI_OBJ_I, 352,  48, false},
  {TAXI_OBJ_I, 256,  64, false},
  {TAXI_OBJ_M, 320,  32, false},
  {TAXI_OBJ_H, 320,  64, false},
  {TAXI_OBJ_J, 288,  48, false},
  {TAXI_OBJ_E, 288,  80, false},

  {TAXI_OBJ_A, 64,  224, false},
  {TAXI_OBJ_I, 96,  240, false},
  {TAXI_OBJ_A, 128, 256, false},

  {TAXI_OBJ_A, 416,  80, false},
  {TAXI_OBJ_A, 384,  96, false},
  {TAXI_OBJ_A, 448,  96, false},
  {TAXI_OBJ_A, 352, 112, false},
  {TAXI_OBJ_L, 416, 80, false},
  {TAXI_OBJ_G, 416, 112, false},
  {TAXI_OBJ_A, 384, 128, false},

  {TAXI_OBJ_A, 512,  128, false},
  {TAXI_OBJ_A, 480,  144, false},
  {TAXI_OBJ_D, 544,  144, true},
  {TAXI_OBJ_D, 488,  160, false},
  {TAXI_OBJ_L, 512,  128, false},
  {TAXI_OBJ_L, 480,  144, false},
  {TAXI_OBJ_G, 512,  160, false},
  {TAXI_OBJ_G, 480,  176, false},

  {TAXI_OBJ_A, 192,  96, false},
  {TAXI_OBJ_A, 160,  112, false},
  {TAXI_OBJ_A, 224,  112, false},
  {TAXI_OBJ_F, 192,  128, false},
  {TAXI_OBJ_K, 192,  96, false},

  {TAXI_OBJ_B, 288,  144, false},
  {TAXI_OBJ_B, 256,  160, false},
  {TAXI_OBJ_B, 320,  160, false},
  {TAXI_OBJ_J, 288,  144, true},
  {TAXI_OBJ_E, 288,  176, true},

  {TAXI_OBJ_A, 384,  192, false},
  {TAXI_OBJ_D, 352,  208, false},
  {TAXI_OBJ_D, 416,  208, true},
  {TAXI_OBJ_H, 384,  224, true},
  {TAXI_OBJ_M, 384,  192, true},

  {TAXI_OBJ_I, 96,  144, false},
  {TAXI_OBJ_B, 64,  160, false},
  {TAXI_OBJ_A, 128,  160, false},
  {TAXI_OBJ_G, 96,  176, false},
  {TAXI_OBJ_L, 96,  144, false},

  {TAXI_OBJ_A, 192,  192, false},
  {TAXI_OBJ_B, 160,  208, false},
  {TAXI_OBJ_B, 224,  208, false},
  {TAXI_OBJ_I, 192,  224, false},
};

static const int TAXI_CITY_OBJECT_COUNT =
  sizeof(taxiCityObjects) / sizeof(taxiCityObjects[0]);

enum TaxiPassengerServiceState {
  TAXI_SERVICE_SEARCHING = 0,

  // 上車演出
  TAXI_SERVICE_PICKUP_CAMERA_TO_CENTER = 1,
  TAXI_SERVICE_PICKUP_SHOW_WAIT = 2,
  TAXI_SERVICE_PICKUP_PASSENGER_GONE_WAIT = 3,
  TAXI_SERVICE_PICKUP_CAMERA_RETURN = 4,
  TAXI_SERVICE_PICKUP_READY_WAIT = 5,

  // 正常載客流程
  TAXI_SERVICE_RIDE_CRUISE = 6,
  TAXI_SERVICE_DRIVING_TO_DROPOFF = 7,

  // 下車演出
  TAXI_SERVICE_DROPOFF_CAMERA_TO_CENTER = 8,
  TAXI_SERVICE_DROPOFF_SHOW_WAIT = 9,
  TAXI_SERVICE_DROPOFF_PASSENGER_GONE_WAIT = 10,
  TAXI_SERVICE_DROPOFF_CAMERA_RETURN = 11,
  TAXI_SERVICE_DROPOFF_READY_WAIT = 12,

  // 下車後巡航
  TAXI_SERVICE_AFTER_DROPOFF_CRUISE = 13
};


enum TaxiGameMode {
  TAXI_GAME_MODE_PASSENGER = 0,
  TAXI_GAME_MODE_POLICE = 1,
  TAXI_GAME_MODE_HELICOPTER = 2
};

enum TaxiPoliceState {
  TAXI_POLICE_STATE_DISABLED = 0,

  // 開場：警車正常巡航 30 秒。
  TAXI_POLICE_STATE_INTRO_PATROL = 1,

  // CAM 滑到 BOY。
  TAXI_POLICE_STATE_CAMERA_TO_BOY = 2,

  // CAM 跟 BOY 3 秒，BOY 持續移動。
  TAXI_POLICE_STATE_BOY_FOCUS = 3,

  // CAM 滑回警車。
  TAXI_POLICE_STATE_CAMERA_TO_POLICE = 4,

  // 警車正常巡航，等待與 BOY 進入同一段道路。
  TAXI_POLICE_STATE_SEARCHING_BOY = 5,

  // 警車追蹤 BOY。
  TAXI_POLICE_STATE_CHASING_BOY = 6,

  // 逮捕：兩車先停止。
  TAXI_POLICE_STATE_ARREST_STOP = 7,

  // 逮捕：CAM 滑到兩者中間。
  TAXI_POLICE_STATE_ARREST_CAMERA_CENTER = 8,

  // 逮捕：CAM 停在兩者中間。
  TAXI_POLICE_STATE_ARREST_FOCUS_WAIT = 9,

  // 逮捕：BOY 閃爍。
  TAXI_POLICE_STATE_ARREST_BOY_BLINK = 10,

  // 逮捕：BOY 消失後等待。
  TAXI_POLICE_STATE_ARREST_AFTER_GONE = 11,

  // 逃走：CAM 先滑到 BOY。
  TAXI_POLICE_STATE_ESCAPE_CAMERA_TO_BOY = 12,

  // 逃走：CAM 跟著 BOY，看 BOY 逃跑。
  TAXI_POLICE_STATE_ESCAPE_BOY_FOCUS = 13,

  // 逃走：CAM 滑回警車。
  TAXI_POLICE_STATE_ESCAPE_CAMERA_TO_POLICE = 14,

  // 逃走：CAM 回到警車後等待，然後回到 LOOP。
  TAXI_POLICE_STATE_ESCAPE_COOLDOWN = 15

  
};


static bool taxiModeReady = false;
static int taxiCameraX = 0;
static int taxiCameraY = 0;
static int taxiCameraFollowVehicleIndex = 0;

// 目前模式。
static uint8_t taxiGameMode = TAXI_GAME_MODE_PASSENGER;

// =====================================================
// 直升機模式資料
// =====================================================

// 直升機的 worldX / worldY 使用圖資中心點。
static int taxiHelicopterWorldX = TAXI_WORLD_W / 2;
static int taxiHelicopterWorldY = TAXI_WORLD_H / 2;
static int taxiHelicopterTargetX = TAXI_WORLD_W / 2;
static int taxiHelicopterTargetY = TAXI_WORLD_H / 2;

// 方向沿用 TAXI_DIR_UP / DOWN / LEFT / RIGHT。
static uint8_t taxiHelicopterDir = TAXI_DIR_RIGHT;

// true 表示已抵達目標，正在原地等待 15 秒。
static bool taxiHelicopterWaiting = false;

// 0 / 1 交叉循環，搭配方向基底組成實際 frame。
static uint8_t taxiHelicopterAnimFrame = 0;

static unsigned long taxiHelicopterStateStartMs = 0;
static unsigned long taxiHelicopterLastMoveMs = 0;
static unsigned long taxiHelicopterLastAnimMs = 0;

static void resetTaxiStallMonitor(unsigned long nowMs) {
  for (int i = 0; i < TAXI_VEHICLE_COUNT; i++) {
    taxiStallLastVehicleX[i] = taxiVehicles[i].worldX;
    taxiStallLastVehicleY[i] = taxiVehicles[i].worldY;
    taxiStallLastMoveMs[i] = nowMs;
  }

  taxiStallLastHelicopterX = taxiHelicopterWorldX;
  taxiStallLastHelicopterY = taxiHelicopterWorldY;
  taxiStallHelicopterLastMoveMs = nowMs;

  taxiStallMonitorReady = true;
}


// POLICE 模式狀態。
static uint8_t taxiPoliceState = TAXI_POLICE_STATE_DISABLED;
static unsigned long taxiPoliceStateStartMs = 0;
static unsigned long taxiPoliceChaseStartMs = 0;

// BOY 顯示控制。
// 逮捕後 BOY 會閃爍並消失。
static bool taxiBoyVisible = true;

static TaxiPassenger taxiPassengers[TAXI_PASSENGER_ACTIVE_COUNT];
static uint8_t taxiPassengerServiceState = TAXI_SERVICE_SEARCHING;
static unsigned long taxiPassengerStateStartMs = 0;
static int taxiPassengerPickedPointIndex = -1;
static int taxiPassengerDestinationPointIndex = -1;
static int taxiPassengerDestinationRoadIndex = -1;


// 目前車上乘客使用的圖資 frame。
// 用來確保上車的是哪一位角色，下車時也是同一位角色。
static uint8_t taxiPassengerRidingFrameIndex = 0;
static bool taxiPassengerHasRider = false;

// 上車演出用：記住被載的乘客位置
static int taxiPickupPassengerX = 0;
static int taxiPickupPassengerY = 0;

// CAM 逐幀移動用
static int taxiCameraMoveStartX = 0;
static int taxiCameraMoveStartY = 0;
static int taxiCameraMoveTargetX = 0;
static int taxiCameraMoveTargetY = 0;
static unsigned long taxiCameraMoveStartMs = 0;
static unsigned long taxiCameraMoveDurationMs = 0;



static void drawTaxiIndexedFrame(
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

  if (x <= -frameW || x >= TAXI_SCR_W ||
      y <= -frameH || y >= TAXI_SCR_H) {
    return;
  }

  int frameStartX = frameIndex * frameW;

  for (int j = 0; j < frameH; j++) {
    int dy = y + j;
    if (dy < 0 || dy >= TAXI_SCR_H) continue;

    for (int i = 0; i < frameW; i++) {
      int dx = x + i;
      if (dx < 0 || dx >= TAXI_SCR_W) continue;

      int sx = flipX ? (frameW - 1 - i) : i;

      uint32_t pos =
        (uint32_t)j * (uint32_t)sheetW +
        (uint32_t)frameStartX +
        (uint32_t)sx;

      uint8_t colorIndex = pgm_read_byte(&(bitmap[pos]));

      // index 0 固定當透明色
      if (colorIndex == 0) continue;

      uint16_t color = pgm_read_word(&(palette[colorIndex]));

     

      display.drawPixel(dx, dy, color);
    }
  }
}

static int getTaxiRoadCenterX(int roadIndex) {
  return taxiRoadTiles[roadIndex].x + (TAXI_ROAD_TILE_W / 2);
}

static int getTaxiRoadCenterY(int roadIndex) {
  return taxiRoadTiles[roadIndex].y + (TAXI_ROAD_TILE_H / 2);
}

static int findTaxiRoadIndexByAnchor(int x, int y) {
  for (int i = 0; i < TAXI_ROAD_COUNT; i++) {
    if (taxiRoadTiles[i].x == x &&
        taxiRoadTiles[i].y == y) {
      return i;
    }
  }

  return 0;
}

static int findNearestTaxiRoadIndexByWorldPoint(int x, int y) {
  int bestIndex = 0;
  long bestDistance = 2147483647L;

  for (int i = 0; i < TAXI_ROAD_COUNT; i++) {
    long dx = (long)getTaxiRoadCenterX(i) - (long)x;
    long dy = (long)getTaxiRoadCenterY(i) - (long)y;
    long distance = dx * dx + dy * dy;

    if (distance < bestDistance) {
      bestDistance = distance;
      bestIndex = i;
    }
  }

  return bestIndex;
}

static bool isTaxiNeighborRoad(int a, int b) {
  if (a == b) return false;

  int dx = taxiRoadTiles[b].x - taxiRoadTiles[a].x;
  int dy = taxiRoadTiles[b].y - taxiRoadTiles[a].y;

  return abs(dx) == 32 && abs(dy) == 16;
}

static int chooseNextTaxiRoadIndex(int fromIndex, int previousIndex) {
  int candidates[8];
  int count = 0;

  for (int i = 0; i < TAXI_ROAD_COUNT; i++) {
    if (!isTaxiNeighborRoad(fromIndex, i)) continue;
    if (i == previousIndex) continue;

    if (count < 8) {
      candidates[count] = i;
      count++;
    }
  }

  if (count == 0) {
    for (int i = 0; i < TAXI_ROAD_COUNT; i++) {
      if (!isTaxiNeighborRoad(fromIndex, i)) continue;

      if (count < 8) {
        candidates[count] = i;
        count++;
      }
    }
  }

  if (count == 0) return fromIndex;

  return candidates[random(count)];
}

static int chooseNextTaxiRoadIndexTowardDestination(
  int fromIndex,
  int previousIndex,
  int destinationIndex
) {
  if (fromIndex == destinationIndex) {
  return chooseNextTaxiRoadIndex(fromIndex, previousIndex);
}

  int queue[TAXI_ROAD_COUNT];
  int parent[TAXI_ROAD_COUNT];
  bool visited[TAXI_ROAD_COUNT];

  for (int i = 0; i < TAXI_ROAD_COUNT; i++) {
    parent[i] = -1;
    visited[i] = false;
  }

  int head = 0;
  int tail = 0;

  queue[tail++] = fromIndex;
  visited[fromIndex] = true;

  while (head < tail) {
    int current = queue[head++];

    if (current == destinationIndex) break;

    for (int i = 0; i < TAXI_ROAD_COUNT; i++) {
      if (visited[i]) continue;
      if (!isTaxiNeighborRoad(current, i)) continue;

      visited[i] = true;
      parent[i] = current;
      queue[tail++] = i;

      if (i == destinationIndex) {
        head = tail;
        break;
      }
    }
  }

  if (!visited[destinationIndex]) {
    return chooseNextTaxiRoadIndex(fromIndex, previousIndex);
  }

  int step = destinationIndex;

  while (parent[step] != -1 && parent[step] != fromIndex) {
    step = parent[step];
  }

  if (parent[step] == fromIndex) {
    return step;
  }

  return chooseNextTaxiRoadIndex(fromIndex, previousIndex);
}

static uint8_t getTaxiDirectionByVector(int dx, int dy, uint8_t fallbackDir) {
  if (dx > 0 && dy > 0) {
    return TAXI_DIR_DOWN;
  } else if (dx < 0 && dy < 0) {
    return TAXI_DIR_UP;
  } else if (dx > 0 && dy < 0) {
    return TAXI_DIR_RIGHT;
  } else if (dx < 0 && dy > 0) {
    return TAXI_DIR_LEFT;
  }

  return fallbackDir;
}

static int getTaxiVehicleFrameIndex(uint8_t dir) {
  if (dir == TAXI_DIR_UP) return 0;
  if (dir == TAXI_DIR_DOWN) return 1;
  if (dir == TAXI_DIR_LEFT) return 1;
  if (dir == TAXI_DIR_RIGHT) return 0;

  return 0;
}

static bool getTaxiVehicleFlipX(uint8_t dir) {
  if (dir == TAXI_DIR_LEFT) return true;
  if (dir == TAXI_DIR_RIGHT) return true;

  return false;
}

static void getTaxiRightLaneOffset(uint8_t dir, int* offsetX, int* offsetY) {
  int ox = 0;
  int oy = 0;

  switch (dir) {
    case TAXI_DIR_UP:
      ox = TAXI_LANE_LEFT_UP_X;
      oy = TAXI_LANE_LEFT_UP_Y;
      break;

    case TAXI_DIR_DOWN:
      ox = TAXI_LANE_RIGHT_DOWN_X;
      oy = TAXI_LANE_RIGHT_DOWN_Y;
      break;

    case TAXI_DIR_LEFT:
      ox = TAXI_LANE_LEFT_DOWN_X;
      oy = TAXI_LANE_LEFT_DOWN_Y;
      break;

    case TAXI_DIR_RIGHT:
      ox = TAXI_LANE_RIGHT_UP_X;
      oy = TAXI_LANE_RIGHT_UP_Y;
      break;
  }

  *offsetX = ox;
  *offsetY = oy;
}

static void clampTaxiCameraTarget(int* cameraX, int* cameraY) {
  if (*cameraX < 0) *cameraX = 0;
  if (*cameraY < 0) *cameraY = 0;

  int maxCameraX = TAXI_WORLD_W - TAXI_SCR_W;
  int maxCameraY = TAXI_WORLD_H - TAXI_SCR_H;

  if (maxCameraX < 0) maxCameraX = 0;
  if (maxCameraY < 0) maxCameraY = 0;

  if (*cameraX > maxCameraX) *cameraX = maxCameraX;
  if (*cameraY > maxCameraY) *cameraY = maxCameraY;
}

static void getTaxiNormalCameraTarget(int* cameraX, int* cameraY) {
  int followIndex = taxiCameraFollowVehicleIndex;

  if (followIndex < 0) followIndex = 0;
  if (followIndex >= TAXI_VEHICLE_COUNT) followIndex = 0;

  int laneX = 0;
  int laneY = 0;

  getTaxiRightLaneOffset(
    taxiVehicles[followIndex].dir,
    &laneX,
    &laneY
  );

  int focusX = taxiVehicles[followIndex].worldX + laneX;
  int focusY = taxiVehicles[followIndex].worldY + laneY;

  *cameraX = focusX - (TAXI_SCR_W / 2);
  *cameraY = focusY - (TAXI_SCR_H / 2) - TAXI_CAMERA_FOLLOW_OFFSET_Y;

  clampTaxiCameraTarget(cameraX, cameraY);
}

static void getTaxiPickupCenterCameraTarget(int* cameraX, int* cameraY) {
  int laneX = 0;
  int laneY = 0;

  getTaxiRightLaneOffset(
    taxiVehicles[TAXI_MAIN_CAR_SLOT].dir,
    &laneX,
    &laneY
  );

  int taxiX = taxiVehicles[TAXI_MAIN_CAR_SLOT].worldX + laneX;
  int taxiY = taxiVehicles[TAXI_MAIN_CAR_SLOT].worldY + laneY;

  int focusX = (taxiX + taxiPickupPassengerX) / 2;
  int focusY = (taxiY + taxiPickupPassengerY) / 2;

  *cameraX = focusX - (TAXI_SCR_W / 2);
  *cameraY = focusY - (TAXI_SCR_H / 2) - TAXI_PICKUP_CAMERA_CENTER_OFFSET_Y;

  clampTaxiCameraTarget(cameraX, cameraY);
}

static void beginTaxiCameraMoveTo(
  int targetCameraX,
  int targetCameraY,
  unsigned long nowMs,
  unsigned long durationMs
) {
  clampTaxiCameraTarget(&targetCameraX, &targetCameraY);

  taxiCameraMoveStartX = taxiCameraX;
  taxiCameraMoveStartY = taxiCameraY;
  taxiCameraMoveTargetX = targetCameraX;
  taxiCameraMoveTargetY = targetCameraY;
  taxiCameraMoveStartMs = nowMs;
  taxiCameraMoveDurationMs = durationMs;
}

static int getTaxiCameraLerpValue(
  int startValue,
  int targetValue,
  unsigned long elapsedMs,
  unsigned long durationMs
) {
  if (durationMs == 0 || elapsedMs >= durationMs) {
    return targetValue;
  }

  long delta = (long)targetValue - (long)startValue;
  return startValue + (int)(delta * (long)elapsedMs / (long)durationMs);
}

static void updateTaxiCameraMove(unsigned long nowMs) {
  unsigned long elapsedMs = nowMs - taxiCameraMoveStartMs;

  taxiCameraX = getTaxiCameraLerpValue(
    taxiCameraMoveStartX,
    taxiCameraMoveTargetX,
    elapsedMs,
    taxiCameraMoveDurationMs
  );

  taxiCameraY = getTaxiCameraLerpValue(
    taxiCameraMoveStartY,
    taxiCameraMoveTargetY,
    elapsedMs,
    taxiCameraMoveDurationMs
  );

  clampTaxiCameraTarget(&taxiCameraX, &taxiCameraY);
}

static bool isTaxiDropoffCarWaiting() {
  if (taxiPassengerServiceState != TAXI_SERVICE_DROPOFF_SHOW_WAIT) {
    return false;
  }

  return millis() - taxiPassengerStateStartMs < TAXI_DROPOFF_CAR_WAIT_MS;
}

static bool isTaxiMainVehiclePaused() {
  return taxiPassengerServiceState == TAXI_SERVICE_PICKUP_CAMERA_TO_CENTER ||
         taxiPassengerServiceState == TAXI_SERVICE_PICKUP_SHOW_WAIT ||
         taxiPassengerServiceState == TAXI_SERVICE_PICKUP_PASSENGER_GONE_WAIT ||
         taxiPassengerServiceState == TAXI_SERVICE_PICKUP_CAMERA_RETURN ||
         taxiPassengerServiceState == TAXI_SERVICE_PICKUP_READY_WAIT ||

         // 下車鏡頭移動中，車子暫停。
         taxiPassengerServiceState == TAXI_SERVICE_DROPOFF_CAMERA_TO_CENTER ||

         // 乘客剛出現後，車子先等一下再開走。
         isTaxiDropoffCarWaiting();
}

static void clearTaxiPassengers() {
  for (int i = 0; i < TAXI_PASSENGER_ACTIVE_COUNT; i++) {
    taxiPassengers[i].active = false;
    taxiPassengers[i].worldX = 0;
    taxiPassengers[i].worldY = 0;
    taxiPassengers[i].frameIndex = 0;
    taxiPassengers[i].flipX = false;
    taxiPassengers[i].pointIndex = -1;
  }
}

static int chooseTaxiPassengerPointIndexAvoid(int avoidIndexA, int avoidIndexB) {
  if (TAXI_PASSENGER_POINT_COUNT <= 0) return 0;

  for (int tries = 0; tries < 16; tries++) {
    int index = random(TAXI_PASSENGER_POINT_COUNT);

    if (TAXI_PASSENGER_POINT_COUNT > 1 && index == avoidIndexA) continue;
    if (TAXI_PASSENGER_POINT_COUNT > 2 && index == avoidIndexB) continue;

    return index;
  }

  for (int i = 0; i < TAXI_PASSENGER_POINT_COUNT; i++) {
    if (TAXI_PASSENGER_POINT_COUNT > 1 && i == avoidIndexA) continue;
    if (TAXI_PASSENGER_POINT_COUNT > 2 && i == avoidIndexB) continue;
    return i;
  }

  return random(TAXI_PASSENGER_POINT_COUNT);
}


static int chooseRandomTaxiRoadIndexAvoid(
  int avoidRoadA,
  int avoidRoadB
) {
  if (TAXI_ROAD_COUNT <= 0) return 0;

  for (int tries = 0; tries < 32; tries++) {
    int roadIndex = random(TAXI_ROAD_COUNT);

    if (TAXI_ROAD_COUNT > 1 && roadIndex == avoidRoadA) continue;
    if (TAXI_ROAD_COUNT > 2 && roadIndex == avoidRoadB) continue;

    return roadIndex;
  }

  for (int i = 0; i < TAXI_ROAD_COUNT; i++) {
    if (TAXI_ROAD_COUNT > 1 && i == avoidRoadA) continue;
    if (TAXI_ROAD_COUNT > 2 && i == avoidRoadB) continue;

    return i;
  }

  return random(TAXI_ROAD_COUNT);
}


static void setTaxiPassengerSlotFromPoint(int slot, int pointIndex) {
  if (slot < 0 || slot >= TAXI_PASSENGER_ACTIVE_COUNT) return;
  if (pointIndex < 0 || pointIndex >= TAXI_PASSENGER_POINT_COUNT) return;

  taxiPassengers[slot].active = true;
  taxiPassengers[slot].worldX = taxiPassengerPoints[pointIndex].x;
  taxiPassengers[slot].worldY = taxiPassengerPoints[pointIndex].y;
  taxiPassengers[slot].frameIndex = random(TAXI_PASSENGER_FRAME_COUNT);
  taxiPassengers[slot].flipX = taxiPassengerPoints[pointIndex].flipX;
  taxiPassengers[slot].pointIndex = pointIndex;
}

static int getTaxiPassengerTouchX(int passengerIndex) {
  return taxiPassengers[passengerIndex].worldX + (TAXI_PASSENGER_FRAME_W / 2);
}

static int getTaxiPassengerTouchY(int passengerIndex) {
  return taxiPassengers[passengerIndex].worldY + TAXI_PASSENGER_FRAME_H - 1;
}


static void spawnTaxiWaitingPassengers() {
  clearTaxiPassengers();

  int usedA = -1;
  int usedB = -1;

  for (int i = 0; i < TAXI_PASSENGER_ACTIVE_COUNT; i++) {
    int pointIndex = chooseTaxiPassengerPointIndexAvoid(usedA, usedB);
    setTaxiPassengerSlotFromPoint(i, pointIndex);

    if (i == 0) {
      usedA = pointIndex;
    } else if (i == 1) {
      usedB = pointIndex;
    }
  }
}

static void setTaxiVehicleTargetRoad(
  int carSlot,
  int nextRoadIndex,
  uint8_t fallbackDir
) {
  if (carSlot < 0 || carSlot >= TAXI_VEHICLE_COUNT) return;
  if (nextRoadIndex < 0 || nextRoadIndex >= TAXI_ROAD_COUNT) return;

  taxiVehicles[carSlot].targetRoadIndex = nextRoadIndex;

  int nextX = getTaxiRoadCenterX(taxiVehicles[carSlot].targetRoadIndex);
  int nextY = getTaxiRoadCenterY(taxiVehicles[carSlot].targetRoadIndex);

  taxiVehicles[carSlot].dir = getTaxiDirectionByVector(
    nextX - taxiVehicles[carSlot].worldX,
    nextY - taxiVehicles[carSlot].worldY,
    fallbackDir
  );
}

static bool isTaxiPassengerInFrontOfTaxi(
  uint8_t taxiDir,
  int passengerX,
  int passengerY,
  int taxiX,
  int taxiY
) {
  int dx = passengerX - taxiX;
  int dy = passengerY - taxiY;

  switch (taxiDir) {
    case TAXI_DIR_UP:
      return dx <= 8 && dy <= 8;

    case TAXI_DIR_DOWN:
      return dx >= -8 && dy >= -8;

    case TAXI_DIR_LEFT:
      return dx <= 8 && dy >= -8;

    case TAXI_DIR_RIGHT:
      return dx >= -8 && dy <= 8;
  }

  return true;
}


static bool isTaxiCarPixelSolid(
  int spriteId,
  int frameIndex,
  bool flipX,
  int localX,
  int localY
) {
  if (spriteId < 0 || spriteId >= TAXI_CAR_SPRITE_DEF_COUNT) {
  return false;
}

  const TaxiCarSpriteDef* def = &taxiCarSpriteDefs[spriteId];

  if (localX < 0 || localX >= def->frameW) return false;
  if (localY < 0 || localY >= def->frameH) return false;

  int sx = flipX ? (def->frameW - 1 - localX) : localX;

  uint32_t pos =
    (uint32_t)localY * (uint32_t)def->sheetW +
    (uint32_t)frameIndex * (uint32_t)def->frameW +
    (uint32_t)sx;

uint8_t colorIndex = pgm_read_byte(&(def->bitmap[pos]));

return colorIndex != 0;


}

static bool isTaxiPassengerPixelSolid(
  int passengerIndex,
  int localX,
  int localY
) {
  if (passengerIndex < 0 ||
      passengerIndex >= TAXI_PASSENGER_ACTIVE_COUNT) {
    return false;
  }

  if (localX < 0 || localX >= TAXI_PASSENGER_FRAME_W) return false;
  if (localY < 0 || localY >= TAXI_PASSENGER_FRAME_H) return false;

  int frameIndex = taxiPassengers[passengerIndex].frameIndex;
  if (frameIndex < 0 || frameIndex >= TAXI_PASSENGER_FRAME_COUNT) {
    return false;
  }

  int sx = taxiPassengers[passengerIndex].flipX ?
    (TAXI_PASSENGER_FRAME_W - 1 - localX) :
    localX;

  uint32_t pos =
    (uint32_t)localY * (uint32_t)TAXI_PASSENGER_SHEET_W +
    (uint32_t)frameIndex * (uint32_t)TAXI_PASSENGER_FRAME_W +
    (uint32_t)sx;

uint8_t colorIndex = pgm_read_byte(&(CITY_P[pos]));

return colorIndex != 0;
}

static bool isTaxiPassengerPixelSolidWithExtraX(
  int passengerIndex,
  int localX,
  int localY
) {
  for (int ox = -TAXI_PASSENGER_COLLISION_EXTRA_X;
       ox <= TAXI_PASSENGER_COLLISION_EXTRA_X;
       ox++) {
    if (isTaxiPassengerPixelSolid(
          passengerIndex,
          localX + ox,
          localY
        )) {
      return true;
    }
  }

  return false;
}

static int findTaxiPassengerTouchingMainTaxi() {
  int laneX = 0;
  int laneY = 0;

  getTaxiRightLaneOffset(
    taxiVehicles[TAXI_MAIN_CAR_SLOT].dir,
    &laneX,
    &laneY
  );

  int spriteId = taxiVehicles[TAXI_MAIN_CAR_SLOT].spriteId;
  int carFrameIndex =
    getTaxiVehicleFrameIndex(taxiVehicles[TAXI_MAIN_CAR_SLOT].dir);

  bool carFlipX =
    getTaxiVehicleFlipX(taxiVehicles[TAXI_MAIN_CAR_SLOT].dir);

  const TaxiCarSpriteDef* carDef = &taxiCarSpriteDefs[spriteId];

  int carLeft =
    taxiVehicles[TAXI_MAIN_CAR_SLOT].worldX +
    laneX -
    carDef->anchorX;

  int carTop =
    taxiVehicles[TAXI_MAIN_CAR_SLOT].worldY +
    laneY -
    carDef->anchorY;

  int carRight =
    carLeft +
    carDef->frameW -
    1;

  int carBottom =
    carTop +
    carDef->frameH -
    1;

  for (int i = 0; i < TAXI_PASSENGER_ACTIVE_COUNT; i++) {
    if (!taxiPassengers[i].active) continue;

    int passengerLeft = taxiPassengers[i].worldX;
    int passengerTop = taxiPassengers[i].worldY;

    int passengerRight =
      passengerLeft +
      TAXI_PASSENGER_FRAME_W -
      1;

    int passengerBottom =
      passengerTop +
      TAXI_PASSENGER_FRAME_H -
      1;

    int checkPassengerLeft =
      passengerLeft - TAXI_PASSENGER_COLLISION_EXTRA_X;

    int checkPassengerRight =
      passengerRight + TAXI_PASSENGER_COLLISION_EXTRA_X;

    int overlapLeft =
      (carLeft > checkPassengerLeft) ? carLeft : checkPassengerLeft;

    int overlapRight =
      (carRight < checkPassengerRight) ? carRight : checkPassengerRight;

    int overlapTop =
      (carTop > passengerTop) ? carTop : passengerTop;

    int overlapBottom =
      (carBottom < passengerBottom) ? carBottom : passengerBottom;

    if (overlapLeft > overlapRight) continue;
    if (overlapTop > overlapBottom) continue;

    for (int wy = overlapTop; wy <= overlapBottom; wy++) {
      for (int wx = overlapLeft; wx <= overlapRight; wx++) {
        int carLocalX = wx - carLeft;
        int carLocalY = wy - carTop;

        int passengerLocalX = wx - passengerLeft;
        int passengerLocalY = wy - passengerTop;

        if (!isTaxiCarPixelSolid(
              spriteId,
              carFrameIndex,
              carFlipX,
              carLocalX,
              carLocalY
            )) {
          continue;
        }

        if (!isTaxiPassengerPixelSolidWithExtraX(
              i,
              passengerLocalX,
              passengerLocalY
            )) {
          continue;
        }

        return i;
      }
    }
  }

  return -1;
}


static bool isTaxiDestinationPassengerPixelSolid(
  int localX,
  int localY
) {
  if (taxiPassengerDestinationPointIndex < 0 ||
      taxiPassengerDestinationPointIndex >= TAXI_PASSENGER_POINT_COUNT) {
    return false;
  }

  if (localX < 0 || localX >= TAXI_PASSENGER_FRAME_W) return false;
  if (localY < 0 || localY >= TAXI_PASSENGER_FRAME_H) return false;

// 下車判定用的虛擬乘客圖資。
// 使用上車時記住的乘客 frame，確保上下車是同一個角色。
int frameIndex = taxiPassengerHasRider ?
  taxiPassengerRidingFrameIndex :
  0;

if (frameIndex < 0 || frameIndex >= TAXI_PASSENGER_FRAME_COUNT) {
  frameIndex = 0;
}

  bool flipX =
    taxiPassengerPoints[taxiPassengerDestinationPointIndex].flipX;

  int sx = flipX ?
    (TAXI_PASSENGER_FRAME_W - 1 - localX) :
    localX;

  uint32_t pos =
    (uint32_t)localY * (uint32_t)TAXI_PASSENGER_SHEET_W +
    (uint32_t)frameIndex * (uint32_t)TAXI_PASSENGER_FRAME_W +
    (uint32_t)sx;

uint8_t colorIndex = pgm_read_byte(&(CITY_P[pos]));

return colorIndex != 0;


}

static bool isTaxiDestinationPassengerPixelSolidWithExtraX(
  int localX,
  int localY
) {
  for (int ox = -TAXI_PASSENGER_COLLISION_EXTRA_X;
       ox <= TAXI_PASSENGER_COLLISION_EXTRA_X;
       ox++) {
    if (isTaxiDestinationPassengerPixelSolid(
          localX + ox,
          localY
        )) {
      return true;
    }
  }

  return false;
}

static bool isTaxiMainCarTouchingDestinationPassengerPoint() {
  if (taxiPassengerDestinationPointIndex < 0 ||
      taxiPassengerDestinationPointIndex >= TAXI_PASSENGER_POINT_COUNT) {
    return false;
  }

  int laneX = 0;
  int laneY = 0;

  getTaxiRightLaneOffset(
    taxiVehicles[TAXI_MAIN_CAR_SLOT].dir,
    &laneX,
    &laneY
  );

  int spriteId = taxiVehicles[TAXI_MAIN_CAR_SLOT].spriteId;

  int carFrameIndex =
    getTaxiVehicleFrameIndex(taxiVehicles[TAXI_MAIN_CAR_SLOT].dir);

  bool carFlipX =
    getTaxiVehicleFlipX(taxiVehicles[TAXI_MAIN_CAR_SLOT].dir);

  const TaxiCarSpriteDef* carDef = &taxiCarSpriteDefs[spriteId];

  int carLeft =
    taxiVehicles[TAXI_MAIN_CAR_SLOT].worldX +
    laneX -
    carDef->anchorX;

  int carTop =
    taxiVehicles[TAXI_MAIN_CAR_SLOT].worldY +
    laneY -
    carDef->anchorY;

  int carRight =
    carLeft +
    carDef->frameW -
    1;

  int carBottom =
    carTop +
    carDef->frameH -
    1;

  int passengerLeft =
    taxiPassengerPoints[taxiPassengerDestinationPointIndex].x;

  int passengerTop =
    taxiPassengerPoints[taxiPassengerDestinationPointIndex].y;

  int passengerRight =
    passengerLeft +
    TAXI_PASSENGER_FRAME_W -
    1;

  int passengerBottom =
    passengerTop +
    TAXI_PASSENGER_FRAME_H -
    1;

  int checkPassengerLeft =
    passengerLeft - TAXI_PASSENGER_COLLISION_EXTRA_X;

  int checkPassengerRight =
    passengerRight + TAXI_PASSENGER_COLLISION_EXTRA_X;

  int overlapLeft =
    (carLeft > checkPassengerLeft) ? carLeft : checkPassengerLeft;

  int overlapRight =
    (carRight < checkPassengerRight) ? carRight : checkPassengerRight;

  int overlapTop =
    (carTop > passengerTop) ? carTop : passengerTop;

  int overlapBottom =
    (carBottom < passengerBottom) ? carBottom : passengerBottom;

  if (overlapLeft > overlapRight) return false;
  if (overlapTop > overlapBottom) return false;

  for (int wy = overlapTop; wy <= overlapBottom; wy++) {
    for (int wx = overlapLeft; wx <= overlapRight; wx++) {
      int carLocalX = wx - carLeft;
      int carLocalY = wy - carTop;

      int passengerLocalX = wx - passengerLeft;
      int passengerLocalY = wy - passengerTop;

      if (!isTaxiCarPixelSolid(
            spriteId,
            carFrameIndex,
            carFlipX,
            carLocalX,
            carLocalY
          )) {
        continue;
      }

      if (!isTaxiDestinationPassengerPixelSolidWithExtraX(
            passengerLocalX,
            passengerLocalY
          )) {
        continue;
      }

      return true;
    }
  }

  return false;
}

static int getTaxiDestinationPassengerFootX() {
  if (taxiPassengerDestinationPointIndex < 0 ||
      taxiPassengerDestinationPointIndex >= TAXI_PASSENGER_POINT_COUNT) {
    return 0;
  }

  return taxiPassengerPoints[taxiPassengerDestinationPointIndex].x +
         (TAXI_PASSENGER_FRAME_W / 2);
}

static int getTaxiDestinationPassengerFootY() {
  if (taxiPassengerDestinationPointIndex < 0 ||
      taxiPassengerDestinationPointIndex >= TAXI_PASSENGER_POINT_COUNT) {
    return 0;
  }

  return taxiPassengerPoints[taxiPassengerDestinationPointIndex].y +
         TAXI_PASSENGER_FRAME_H -
         1;
}



static bool isTaxiMainCarNearDestinationPassengerPoint() {
  if (taxiPassengerDestinationPointIndex < 0 ||
      taxiPassengerDestinationPointIndex >= TAXI_PASSENGER_POINT_COUNT) {
    return false;
  }

  int laneX = 0;
  int laneY = 0;

  getTaxiRightLaneOffset(
    taxiVehicles[TAXI_MAIN_CAR_SLOT].dir,
    &laneX,
    &laneY
  );

  // 車子的行車點。
  // 這個點比整張車圖更適合拿來做下車保底判定，
  // 避免車圖透明邊緣造成太早觸發。
  int taxiX = taxiVehicles[TAXI_MAIN_CAR_SLOT].worldX + laneX;
  int taxiY = taxiVehicles[TAXI_MAIN_CAR_SLOT].worldY + laneY;

  int passengerLeft =
    taxiPassengerPoints[taxiPassengerDestinationPointIndex].x -
    TAXI_DROPOFF_FALLBACK_EXTRA_X;

  int passengerTop =
    taxiPassengerPoints[taxiPassengerDestinationPointIndex].y -
    TAXI_DROPOFF_FALLBACK_EXTRA_Y;

  int passengerRight =
    taxiPassengerPoints[taxiPassengerDestinationPointIndex].x +
    TAXI_PASSENGER_FRAME_W -
    1 +
    TAXI_DROPOFF_FALLBACK_EXTRA_X;

  int passengerBottom =
    taxiPassengerPoints[taxiPassengerDestinationPointIndex].y +
    TAXI_PASSENGER_FRAME_H -
    1 +
    TAXI_DROPOFF_FALLBACK_EXTRA_Y;

  if (taxiX < passengerLeft) return false;
  if (taxiX > passengerRight) return false;
  if (taxiY < passengerTop) return false;
  if (taxiY > passengerBottom) return false;

  return true;
}

static bool isTaxiDestinationPassengerPointVisibleOnScreen(int marginPx) {
  if (taxiPassengerDestinationPointIndex < 0 ||
      taxiPassengerDestinationPointIndex >= TAXI_PASSENGER_POINT_COUNT) {
    return false;
  }

  int passengerCenterX =
    taxiPassengerPoints[taxiPassengerDestinationPointIndex].x +
    (TAXI_PASSENGER_FRAME_W / 2);

  int passengerCenterY =
    taxiPassengerPoints[taxiPassengerDestinationPointIndex].y +
    (TAXI_PASSENGER_FRAME_H / 2);

  if (passengerCenterX < taxiCameraX - marginPx) return false;
  if (passengerCenterX >= taxiCameraX + TAXI_SCR_W + marginPx) return false;
  if (passengerCenterY < taxiCameraY - marginPx) return false;
  if (passengerCenterY >= taxiCameraY + TAXI_SCR_H + marginPx) return false;

  return true;
}

static bool isTaxiMainCarReadyForDropoff(unsigned long nowMs) {
  if (taxiPassengerDestinationPointIndex < 0 ||
      taxiPassengerDestinationPointIndex >= TAXI_PASSENGER_POINT_COUNT) {
    return false;
  }

  if (taxiPassengerDestinationRoadIndex < 0 ||
      taxiPassengerDestinationRoadIndex >= TAXI_ROAD_COUNT) {
    return false;
  }

  int laneX = 0;
  int laneY = 0;

  getTaxiRightLaneOffset(
    taxiVehicles[TAXI_MAIN_CAR_SLOT].dir,
    &laneX,
    &laneY
  );

  // 車子的行車點，也就是車子目前在道路上的位置。
  int taxiX = taxiVehicles[TAXI_MAIN_CAR_SLOT].worldX + laneX;
  int taxiY = taxiVehicles[TAXI_MAIN_CAR_SLOT].worldY + laneY;

  // 目的地乘客腳底中心。
  // 這個點最接近道路邊，所以拿來判斷下車比較合理。
  int passengerFootX = getTaxiDestinationPassengerFootX();
  int passengerFootY = getTaxiDestinationPassengerFootY();

  int dx = abs(taxiX - passengerFootX);
  int dy = abs(taxiY - passengerFootY);

  bool taxiIsNearPassengerFoot =
    dx <= TAXI_DROPOFF_READY_DISTANCE_X &&
    dy <= TAXI_DROPOFF_READY_DISTANCE_Y;

  bool taxiIsOnDestinationRoad =
    taxiVehicles[TAXI_MAIN_CAR_SLOT].currentRoadIndex ==
      taxiPassengerDestinationRoadIndex ||
    taxiVehicles[TAXI_MAIN_CAR_SLOT].targetRoadIndex ==
      taxiPassengerDestinationRoadIndex;

  // 正常下車條件：
  // 車子必須已經在目的地道路附近，而且行車點靠近乘客腳底。
  if (taxiIsOnDestinationRoad && taxiIsNearPassengerFoot) {
    return true;
  }

  // 保底條件：
  // 避免車子因為路線、轉彎或座標誤差，在附近一直徘徊。
  // 這個是防卡死，不是主要下車邏輯。
  if (nowMs - taxiPassengerStateStartMs >= TAXI_DROPOFF_FORCE_MS) {
    Serial.println("卡死狀態解除");
    return true;
  }

  return false;
}


static void showTaxiDropoffPassengerAtDestination() {
  clearTaxiPassengers();

  if (taxiPassengerDestinationPointIndex < 0 ||
      taxiPassengerDestinationPointIndex >= TAXI_PASSENGER_POINT_COUNT) {
    return;
  }

  taxiPassengers[0].active = true;

  taxiPassengers[0].worldX =
    taxiPassengerPoints[taxiPassengerDestinationPointIndex].x;

  taxiPassengers[0].worldY =
    taxiPassengerPoints[taxiPassengerDestinationPointIndex].y;

  // 下車顯示同一個上車乘客角色。
  taxiPassengers[0].frameIndex = taxiPassengerHasRider ?
    taxiPassengerRidingFrameIndex :
    0;

  if (taxiPassengers[0].frameIndex >= TAXI_PASSENGER_FRAME_COUNT) {
    taxiPassengers[0].frameIndex = 0;
  }

  taxiPassengers[0].flipX =
    taxiPassengerPoints[taxiPassengerDestinationPointIndex].flipX;

  taxiPassengers[0].pointIndex =
    taxiPassengerDestinationPointIndex;
}


static void beginTaxiPassengerBoarding(int passengerSlot, unsigned long nowMs) {
  if (passengerSlot < 0 || passengerSlot >= TAXI_PASSENGER_ACTIVE_COUNT) return;

TaxiPassenger pickedPassenger = taxiPassengers[passengerSlot];

taxiPassengerPickedPointIndex = pickedPassenger.pointIndex;
taxiPassengerDestinationPointIndex = -1;
taxiPassengerDestinationRoadIndex = -1;

// 記住這次上車的是哪一個乘客角色。
// 下車時會使用同一個 frameIndex。
taxiPassengerRidingFrameIndex = pickedPassenger.frameIndex;
taxiPassengerHasRider = true;

taxiPickupPassengerX =
  pickedPassenger.worldX + (TAXI_PASSENGER_FRAME_W / 2);

taxiPickupPassengerY =
  pickedPassenger.worldY + (TAXI_PASSENGER_FRAME_H / 2);

  // 遭遇後只留下被載到的這一位乘客，其他乘客消失。
  clearTaxiPassengers();

  taxiPassengers[0] = pickedPassenger;
  taxiPassengers[0].active = true;

  int pickupCameraX = 0;
  int pickupCameraY = 0;

  getTaxiPickupCenterCameraTarget(&pickupCameraX, &pickupCameraY);

  beginTaxiCameraMoveTo(
    pickupCameraX,
    pickupCameraY,
    nowMs,
    TAXI_PICKUP_CAMERA_TO_CENTER_MS
  );

  taxiPassengerServiceState = TAXI_SERVICE_PICKUP_CAMERA_TO_CENTER;
  taxiPassengerStateStartMs = nowMs;
}

static void beginTaxiPassengerDestinationRun(unsigned long nowMs) {
  taxiPassengerDestinationPointIndex =
    chooseTaxiPassengerPointIndexAvoid(taxiPassengerPickedPointIndex, -1);

  int destinationFootX =
    taxiPassengerPoints[taxiPassengerDestinationPointIndex].x +
    (TAXI_PASSENGER_FRAME_W / 2);

  int destinationFootY =
    taxiPassengerPoints[taxiPassengerDestinationPointIndex].y +
    TAXI_PASSENGER_FRAME_H -
    1;

  taxiPassengerDestinationRoadIndex = findNearestTaxiRoadIndexByWorldPoint(
    destinationFootX,
    destinationFootY
  );

  taxiPassengerServiceState = TAXI_SERVICE_DRIVING_TO_DROPOFF;
  taxiPassengerStateStartMs = nowMs;
}

static void beginTaxiPassengerDropoff(unsigned long nowMs) {
  // 先不要顯示乘客。
  // 下車乘客要等車子停住、鏡頭到位後才出現。
  clearTaxiPassengers();

  if (taxiPassengerDestinationPointIndex >= 0 &&
      taxiPassengerDestinationPointIndex < TAXI_PASSENGER_POINT_COUNT) {
    taxiPickupPassengerX =
      taxiPassengerPoints[taxiPassengerDestinationPointIndex].x +
      (TAXI_PASSENGER_FRAME_W / 2);

    taxiPickupPassengerY =
      taxiPassengerPoints[taxiPassengerDestinationPointIndex].y +
      (TAXI_PASSENGER_FRAME_H / 2);
  }

  // 讓車子先停在目前位置。
  // 後面乘客出現後，會重新給車子下一個路線，讓車子開走。
  taxiVehicles[TAXI_MAIN_CAR_SLOT].targetRoadIndex =
    taxiVehicles[TAXI_MAIN_CAR_SLOT].currentRoadIndex;

  int dropoffCameraX = 0;
  int dropoffCameraY = 0;

  getTaxiPickupCenterCameraTarget(&dropoffCameraX, &dropoffCameraY);

  beginTaxiCameraMoveTo(
    dropoffCameraX,
    dropoffCameraY,
    nowMs,
    TAXI_PICKUP_CAMERA_TO_CENTER_MS
  );

  taxiPassengerServiceState = TAXI_SERVICE_DROPOFF_CAMERA_TO_CENTER;
  taxiPassengerStateStartMs = nowMs;
}


static bool isTaxiPassengerModeActive() {
  return taxiGameMode == TAXI_GAME_MODE_PASSENGER;
}

static bool isTaxiPoliceModeActive() {
  return taxiGameMode == TAXI_GAME_MODE_POLICE;
}

static bool isTaxiHelicopterModeActive() {
  return taxiGameMode == TAXI_GAME_MODE_HELICOPTER;
}

// =====================================================
// 直升機模式
//
// 直升機不依附道路節點，而是在整個城市範圍內自由飛行。
// 抵達目標後停留 15 秒，再重新抽選下一個地點。
// =====================================================

static void chooseTaxiHelicopterTarget(unsigned long nowMs) {
  int minX = TAXI_AIR_FRAME_W / 2;
  int maxX = TAXI_WORLD_W - (TAXI_AIR_FRAME_W / 2);
  int minY = TAXI_AIR_FRAME_H / 2;
  int maxY = TAXI_WORLD_H - (TAXI_AIR_FRAME_H / 2);

  int newTargetX = taxiHelicopterWorldX;
  int newTargetY = taxiHelicopterWorldY;

  long minDistanceSquared =
    (long)TAXI_HELICOPTER_MIN_TARGET_DISTANCE *
    (long)TAXI_HELICOPTER_MIN_TARGET_DISTANCE;

  // 優先抽選距離目前位置較遠的地點，
  // 避免直升機剛結束等待後只移動一小段又停止。
  for (int tries = 0; tries < 24; tries++) {
    newTargetX = random(minX, maxX + 1);
    newTargetY = random(minY, maxY + 1);

    long dx = (long)newTargetX - (long)taxiHelicopterWorldX;
    long dy = (long)newTargetY - (long)taxiHelicopterWorldY;

    if (dx * dx + dy * dy >= minDistanceSquared) {
      break;
    }
  }

  taxiHelicopterTargetX = newTargetX;
  taxiHelicopterTargetY = newTargetY;

  taxiHelicopterDir = getTaxiDirectionByVector(
    taxiHelicopterTargetX - taxiHelicopterWorldX,
    taxiHelicopterTargetY - taxiHelicopterWorldY,
    taxiHelicopterDir
  );

  taxiHelicopterWaiting = false;
  taxiHelicopterStateStartMs = nowMs;
  taxiHelicopterLastMoveMs = nowMs;
}

static void initTaxiHelicopter(unsigned long nowMs) {
  int minX = TAXI_AIR_FRAME_W / 2;
  int maxX = TAXI_WORLD_W - (TAXI_AIR_FRAME_W / 2);
  int minY = TAXI_AIR_FRAME_H / 2;
  int maxY = TAXI_WORLD_H - (TAXI_AIR_FRAME_H / 2);

  taxiHelicopterWorldX = random(minX, maxX + 1);
  taxiHelicopterWorldY = random(minY, maxY + 1);

  taxiHelicopterTargetX = taxiHelicopterWorldX;
  taxiHelicopterTargetY = taxiHelicopterWorldY;
  taxiHelicopterDir = TAXI_DIR_RIGHT;
  taxiHelicopterWaiting = false;
  taxiHelicopterAnimFrame = 0;
  taxiHelicopterStateStartMs = nowMs;
  taxiHelicopterLastMoveMs = nowMs;
  taxiHelicopterLastAnimMs = nowMs;

  chooseTaxiHelicopterTarget(nowMs);
}

static void updateTaxiHelicopterAnimation(unsigned long nowMs) {
  if (!isTaxiHelicopterModeActive()) return;

  if (nowMs - taxiHelicopterLastAnimMs <
      TAXI_HELICOPTER_ANIM_INTERVAL_MS) {
    return;
  }

  taxiHelicopterLastAnimMs = nowMs;
  taxiHelicopterAnimFrame ^= 1;
}

static void updateTaxiHelicopter(unsigned long nowMs) {
  if (!isTaxiHelicopterModeActive()) return;

  // 螺旋槳動畫在飛行與等待期間都持續播放。
  updateTaxiHelicopterAnimation(nowMs);

  if (taxiHelicopterWaiting) {
    if (nowMs - taxiHelicopterStateStartMs >=
        TAXI_HELICOPTER_WAIT_MS) {
      chooseTaxiHelicopterTarget(nowMs);
    }

    return;
  }

  if (nowMs - taxiHelicopterLastMoveMs <
      TAXI_HELICOPTER_MOVE_INTERVAL_MS) {
    return;
  }

  taxiHelicopterLastMoveMs = nowMs;

  int dx = taxiHelicopterTargetX - taxiHelicopterWorldX;
  int dy = taxiHelicopterTargetY - taxiHelicopterWorldY;

  taxiHelicopterDir = getTaxiDirectionByVector(
    dx,
    dy,
    taxiHelicopterDir
  );

  if (dx != 0) {
    int moveX = dx > 0 ?
      TAXI_HELICOPTER_STEP_PX :
      -TAXI_HELICOPTER_STEP_PX;

    if (abs(dx) <= TAXI_HELICOPTER_STEP_PX) {
      taxiHelicopterWorldX = taxiHelicopterTargetX;
    } else {
      taxiHelicopterWorldX += moveX;
    }
  }

  if (dy != 0) {
    int moveY = dy > 0 ?
      TAXI_HELICOPTER_STEP_PX :
      -TAXI_HELICOPTER_STEP_PX;

    if (abs(dy) <= TAXI_HELICOPTER_STEP_PX) {
      taxiHelicopterWorldY = taxiHelicopterTargetY;
    } else {
      taxiHelicopterWorldY += moveY;
    }
  }

  if (taxiHelicopterWorldX == taxiHelicopterTargetX &&
      taxiHelicopterWorldY == taxiHelicopterTargetY) {
    taxiHelicopterWaiting = true;
    taxiHelicopterStateStartMs = nowMs;
  }
}

static void getTaxiHelicopterCameraTarget(
  int* cameraX,
  int* cameraY
) {
  *cameraX =
    taxiHelicopterWorldX -
    (TAXI_SCR_W / 2);

  *cameraY =
    taxiHelicopterWorldY -
    (TAXI_SCR_H / 2) -
    TAXI_HELICOPTER_CAMERA_OFFSET_Y;

  clampTaxiCameraTarget(cameraX, cameraY);
}

static int getTaxiHelicopterFrameIndex() {
  // 與其他交通工具相同：
  // UP / RIGHT 使用第一組方向，DOWN / LEFT 使用第二組方向。
  int directionBaseFrame =
    (taxiHelicopterDir == TAXI_DIR_DOWN ||
     taxiHelicopterDir == TAXI_DIR_LEFT) ?
    2 :
    0;

  int frameIndex =
    directionBaseFrame +
    taxiHelicopterAnimFrame;

  if (frameIndex < 0 || frameIndex >= TAXI_AIR_FRAME_COUNT) {
    return 0;
  }

  return frameIndex;
}

static void drawTaxiHelicopter() {
  if (!isTaxiHelicopterModeActive()) return;

  int drawX =
    taxiHelicopterWorldX -
    taxiCameraX -
    (TAXI_AIR_FRAME_W / 2);

  int drawY =
    taxiHelicopterWorldY -
    taxiCameraY -
    (TAXI_AIR_FRAME_H / 2);

  bool flipX =
    getTaxiVehicleFlipX(taxiHelicopterDir);

  drawTaxiIndexedFrame(
    CAR_AIR,
    CAR_AIR_PALETTE,
    TAXI_AIR_SHEET_W,
    TAXI_AIR_FRAME_W,
    TAXI_AIR_FRAME_H,
    getTaxiHelicopterFrameIndex(),
    flipX,
    drawX,
    drawY
  );
}

static bool isTaxiVehicleSlotActive(int carSlot) {
  if (carSlot < 0 || carSlot >= TAXI_VEHICLE_COUNT) return false;

  // 只有警車模式逮捕後，BOY 才暫時消失。
  // TAXI 模式時，BOY 也會在大地圖上正常巡航。
  if (isTaxiPoliceModeActive() &&
      carSlot == TAXI_BOY_CAR_SLOT &&
      !taxiBoyVisible) {
    return false;
  }

  return true;
}

static int getTaxiVehicleLaneWorldX(int carSlot) {
  int laneX = 0;
  int laneY = 0;

  getTaxiRightLaneOffset(
    taxiVehicles[carSlot].dir,
    &laneX,
    &laneY
  );

  return taxiVehicles[carSlot].worldX + laneX;
}

static int getTaxiVehicleLaneWorldY(int carSlot) {
  int laneX = 0;
  int laneY = 0;

  getTaxiRightLaneOffset(
    taxiVehicles[carSlot].dir,
    &laneX,
    &laneY
  );

  return taxiVehicles[carSlot].worldY + laneY;
}

static bool getTaxiDirectionUnit(
  uint8_t dir,
  int* dirX,
  int* dirY
) {
  switch (dir) {
    case TAXI_DIR_UP:
      *dirX = -1;
      *dirY = -1;
      return true;

    case TAXI_DIR_DOWN:
      *dirX = 1;
      *dirY = 1;
      return true;

    case TAXI_DIR_LEFT:
      *dirX = -1;
      *dirY = 1;
      return true;

    case TAXI_DIR_RIGHT:
      *dirX = 1;
      *dirY = -1;
      return true;
  }

  *dirX = 0;
  *dirY = 0;
  return false;
}

static bool isTaxiVehicleTooCloseToVehicle(
  int carSlot,
  int otherSlot
) {
  if (carSlot == otherSlot) return false;
  if (!isTaxiVehicleSlotActive(carSlot)) return false;
  if (!isTaxiVehicleSlotActive(otherSlot)) return false;

  // TAXI 載客模式時，如果主 TAXI 正在上下車等待，
  // 其他車輛不要因為靠近主 TAXI 而暫停。
  // 這樣主 TAXI 停著演出時，警車 / BOY 仍然會繼續巡航。
  if (isTaxiPassengerModeActive() &&
      otherSlot == TAXI_MAIN_CAR_SLOT &&
      isTaxiMainVehiclePaused()) {
    return false;
  }


  int carX = getTaxiVehicleLaneWorldX(carSlot);
  int carY = getTaxiVehicleLaneWorldY(carSlot);

  int otherX = getTaxiVehicleLaneWorldX(otherSlot);
  int otherY = getTaxiVehicleLaneWorldY(otherSlot);

  int dx = otherX - carX;
  int dy = otherY - carY;

  // 極近距離保護：
  // 避免兩車圖資已經快重疊時，後建立或高槽位車繼續壓上去。
  if (abs(dx) <= TAXI_VEHICLE_OVERLAP_GUARD_X &&
      abs(dy) <= TAXI_VEHICLE_OVERLAP_GUARD_Y &&
      otherSlot < carSlot) {
    return true;
  }

  // 不同方向時，只做極近距離保護，不做前後車距。
  if (taxiVehicles[carSlot].dir != taxiVehicles[otherSlot].dir) {
    return false;
  }

  int dirX = 0;
  int dirY = 0;

  if (!getTaxiDirectionUnit(
        taxiVehicles[carSlot].dir,
        &dirX,
        &dirY
      )) {
    return false;
  }

  int along = dx * dirX + dy * dirY;

  bool otherIsInFront =
    along > 0 ||
    (dx == 0 && dy == 0 && otherSlot < carSlot);

  if (!otherIsInFront) {
    return false;
  }

  if (abs(dx) > TAXI_VEHICLE_KEEP_DISTANCE_X) return false;
  if (abs(dy) > TAXI_VEHICLE_KEEP_DISTANCE_Y) return false;

  return true;
}

static bool isTaxiVehicleTooCloseToAnyOtherVehicle(int carSlot) {
  if (!isTaxiVehicleSlotActive(carSlot)) return false;

  for (int i = 0; i < TAXI_VEHICLE_COUNT; i++) {
    if (i == carSlot) continue;

    if (isTaxiVehicleTooCloseToVehicle(carSlot, i)) {
      return true;
    }
  }

  return false;
}


static bool isTaxiPoliceAndBoyOnSameRoadArea() {
  if (!isTaxiPoliceModeActive()) return false;
  if (!taxiBoyVisible) return false;

  int policeCurrent = taxiVehicles[TAXI_POLICE_CAR_SLOT].currentRoadIndex;
  int policeTarget = taxiVehicles[TAXI_POLICE_CAR_SLOT].targetRoadIndex;

  int boyCurrent = taxiVehicles[TAXI_BOY_CAR_SLOT].currentRoadIndex;
  int boyTarget = taxiVehicles[TAXI_BOY_CAR_SLOT].targetRoadIndex;

  return policeCurrent == boyCurrent ||
         policeCurrent == boyTarget ||
         policeTarget == boyCurrent ||
         policeTarget == boyTarget;
}

static int getTaxiPoliceChaseDestinationRoad() {
  if (!taxiBoyVisible) {
    return taxiVehicles[TAXI_POLICE_CAR_SLOT].targetRoadIndex;
  }

  int boyTarget = taxiVehicles[TAXI_BOY_CAR_SLOT].targetRoadIndex;

  if (boyTarget >= 0 && boyTarget < TAXI_ROAD_COUNT) {
    return boyTarget;
  }

  return taxiVehicles[TAXI_BOY_CAR_SLOT].currentRoadIndex;
}

static void getTaxiVehicleCameraTarget(
  int carSlot,
  int* cameraX,
  int* cameraY
) {
  int laneX = 0;
  int laneY = 0;

  getTaxiRightLaneOffset(
    taxiVehicles[carSlot].dir,
    &laneX,
    &laneY
  );

  int focusX = taxiVehicles[carSlot].worldX + laneX;
  int focusY = taxiVehicles[carSlot].worldY + laneY;

  *cameraX = focusX - (TAXI_SCR_W / 2);
  *cameraY = focusY - (TAXI_SCR_H / 2) - TAXI_CAMERA_FOLLOW_OFFSET_Y;

  clampTaxiCameraTarget(cameraX, cameraY);
}

static void beginTaxiCameraMoveToVehicle(
  int carSlot,
  unsigned long nowMs,
  unsigned long durationMs
) {
  int cameraX = 0;
  int cameraY = 0;

  getTaxiVehicleCameraTarget(
    carSlot,
    &cameraX,
    &cameraY
  );

  beginTaxiCameraMoveTo(
    cameraX,
    cameraY,
    nowMs,
    durationMs
  );
}

static void beginTaxiCameraMoveToVehiclePair(
  int carSlotA,
  int carSlotB,
  unsigned long nowMs,
  unsigned long durationMs
) {
  int ax = getTaxiVehicleLaneWorldX(carSlotA);
  int ay = getTaxiVehicleLaneWorldY(carSlotA);

  int bx = getTaxiVehicleLaneWorldX(carSlotB);
  int by = getTaxiVehicleLaneWorldY(carSlotB);

  int focusX = (ax + bx) / 2;
  int focusY = (ay + by) / 2;

  int cameraX = focusX - (TAXI_SCR_W / 2);
  int cameraY = focusY - (TAXI_SCR_H / 2) - TAXI_CAMERA_FOLLOW_OFFSET_Y;

  clampTaxiCameraTarget(&cameraX, &cameraY);

  beginTaxiCameraMoveTo(
    cameraX,
    cameraY,
    nowMs,
    durationMs
  );
}

static bool isTaxiPoliceCameraMovingState() {
  return taxiPoliceState == TAXI_POLICE_STATE_CAMERA_TO_BOY ||
         taxiPoliceState == TAXI_POLICE_STATE_CAMERA_TO_POLICE ||
         taxiPoliceState == TAXI_POLICE_STATE_ARREST_CAMERA_CENTER ||
         taxiPoliceState == TAXI_POLICE_STATE_ESCAPE_CAMERA_TO_BOY ||
         taxiPoliceState == TAXI_POLICE_STATE_ESCAPE_CAMERA_TO_POLICE;
}



static void restartTaxiPoliceLoop(unsigned long nowMs) {
  taxiBoyVisible = true;

  int policeStartRoadIndex =
    chooseRandomTaxiRoadIndexAvoid(-1, -1);

  int boyStartRoadIndex =
    chooseRandomTaxiRoadIndexAvoid(policeStartRoadIndex, -1);

   initTaxiVehicle(
    TAXI_POLICE_CAR_SLOT,
    TAXI_SPRITE_POLICE,
    policeStartRoadIndex,
    nowMs + 20
  );

  initTaxiVehicle(
    TAXI_BOY_CAR_SLOT,
    TAXI_SPRITE_BOY,
    boyStartRoadIndex,
    nowMs + 40
  );

  taxiPoliceState = TAXI_POLICE_STATE_INTRO_PATROL;
  taxiPoliceStateStartMs = nowMs;
  taxiPoliceChaseStartMs = 0;

  taxiCameraFollowVehicleIndex = TAXI_POLICE_CAR_SLOT;
}

static void updateTaxiPoliceScenario(unsigned long nowMs) {
  if (!isTaxiPoliceModeActive()) return;

  switch (taxiPoliceState) {
    case TAXI_POLICE_STATE_DISABLED:
      restartTaxiPoliceLoop(nowMs);
      break;

    case TAXI_POLICE_STATE_INTRO_PATROL:
      if (nowMs - taxiPoliceStateStartMs >=
          TAXI_POLICE_INTRO_PATROL_MS) {
        beginTaxiCameraMoveToVehicle(
          TAXI_BOY_CAR_SLOT,
          nowMs,
          TAXI_POLICE_CAMERA_MOVE_MS
        );

        taxiPoliceState = TAXI_POLICE_STATE_CAMERA_TO_BOY;
        taxiPoliceStateStartMs = nowMs;
      }
      break;

    case TAXI_POLICE_STATE_CAMERA_TO_BOY:
      if (nowMs - taxiPoliceStateStartMs >=
          TAXI_POLICE_CAMERA_MOVE_MS) {
        taxiCameraFollowVehicleIndex = TAXI_BOY_CAR_SLOT;

        taxiPoliceState = TAXI_POLICE_STATE_BOY_FOCUS;
        taxiPoliceStateStartMs = nowMs;
      }
      break;

    case TAXI_POLICE_STATE_BOY_FOCUS:
      if (nowMs - taxiPoliceStateStartMs >=
          TAXI_POLICE_BOY_FOCUS_MS) {
        beginTaxiCameraMoveToVehicle(
          TAXI_POLICE_CAR_SLOT,
          nowMs,
          TAXI_POLICE_CAMERA_MOVE_MS
        );

        taxiPoliceState = TAXI_POLICE_STATE_CAMERA_TO_POLICE;
        taxiPoliceStateStartMs = nowMs;
      }
      break;

    case TAXI_POLICE_STATE_CAMERA_TO_POLICE:
      if (nowMs - taxiPoliceStateStartMs >=
          TAXI_POLICE_CAMERA_MOVE_MS) {
        taxiCameraFollowVehicleIndex = TAXI_POLICE_CAR_SLOT;

        taxiPoliceState = TAXI_POLICE_STATE_SEARCHING_BOY;
        taxiPoliceStateStartMs = nowMs;
      }
      break;

    case TAXI_POLICE_STATE_SEARCHING_BOY:
      if (isTaxiPoliceAndBoyOnSameRoadArea()) {
        taxiPoliceState = TAXI_POLICE_STATE_CHASING_BOY;
        taxiPoliceStateStartMs = nowMs;
        taxiPoliceChaseStartMs = nowMs;
      }
      break;

    case TAXI_POLICE_STATE_CHASING_BOY:
      if (nowMs - taxiPoliceChaseStartMs >=
          TAXI_POLICE_CHASE_MS) {
        bool arrestBoy = random(2) == 0;

        if (arrestBoy) {
          taxiPoliceState = TAXI_POLICE_STATE_ARREST_STOP;
          taxiPoliceStateStartMs = nowMs;
        } else {
          beginTaxiCameraMoveToVehicle(
            TAXI_BOY_CAR_SLOT,
            nowMs,
            TAXI_POLICE_CAMERA_MOVE_MS
          );

          taxiPoliceState = TAXI_POLICE_STATE_ESCAPE_CAMERA_TO_BOY;
          taxiPoliceStateStartMs = nowMs;
        }
      }
      break;

    case TAXI_POLICE_STATE_ARREST_STOP:
      if (nowMs - taxiPoliceStateStartMs >=
          TAXI_POLICE_ARREST_STOP_MS) {
        beginTaxiCameraMoveToVehiclePair(
          TAXI_POLICE_CAR_SLOT,
          TAXI_BOY_CAR_SLOT,
          nowMs,
          TAXI_POLICE_CAMERA_MOVE_MS
        );

        taxiPoliceState = TAXI_POLICE_STATE_ARREST_CAMERA_CENTER;
        taxiPoliceStateStartMs = nowMs;
      }
      break;

    case TAXI_POLICE_STATE_ARREST_CAMERA_CENTER:
      if (nowMs - taxiPoliceStateStartMs >=
          TAXI_POLICE_CAMERA_MOVE_MS) {
        taxiPoliceState = TAXI_POLICE_STATE_ARREST_FOCUS_WAIT;
        taxiPoliceStateStartMs = nowMs;
      }
      break;

    case TAXI_POLICE_STATE_ARREST_FOCUS_WAIT:
      if (nowMs - taxiPoliceStateStartMs >=
          TAXI_POLICE_ARREST_FOCUS_MS) {
        taxiPoliceState = TAXI_POLICE_STATE_ARREST_BOY_BLINK;
        taxiPoliceStateStartMs = nowMs;
      }
      break;

    case TAXI_POLICE_STATE_ARREST_BOY_BLINK:
      if (nowMs - taxiPoliceStateStartMs >=
          TAXI_POLICE_ARREST_BLINK_MS) {
        taxiBoyVisible = false;

        taxiPoliceState = TAXI_POLICE_STATE_ARREST_AFTER_GONE;
        taxiPoliceStateStartMs = nowMs;
      }
      break;

    case TAXI_POLICE_STATE_ARREST_AFTER_GONE:
      if (nowMs - taxiPoliceStateStartMs >=
          TAXI_POLICE_ARREST_AFTER_GONE_MS) {
        restartTaxiPoliceLoop(nowMs);
      }
      break;

    case TAXI_POLICE_STATE_ESCAPE_CAMERA_TO_BOY:
      if (nowMs - taxiPoliceStateStartMs >=
          TAXI_POLICE_CAMERA_MOVE_MS) {
        taxiCameraFollowVehicleIndex = TAXI_BOY_CAR_SLOT;

        taxiPoliceState = TAXI_POLICE_STATE_ESCAPE_BOY_FOCUS;
        taxiPoliceStateStartMs = nowMs;
      }
      break;

    case TAXI_POLICE_STATE_ESCAPE_BOY_FOCUS:
      if (nowMs - taxiPoliceStateStartMs >=
          TAXI_POLICE_ESCAPE_BOY_FOCUS_MS) {
        beginTaxiCameraMoveToVehicle(
          TAXI_POLICE_CAR_SLOT,
          nowMs,
          TAXI_POLICE_CAMERA_MOVE_MS
        );

        taxiPoliceState = TAXI_POLICE_STATE_ESCAPE_CAMERA_TO_POLICE;
        taxiPoliceStateStartMs = nowMs;
      }
      break;

    case TAXI_POLICE_STATE_ESCAPE_CAMERA_TO_POLICE:
      if (nowMs - taxiPoliceStateStartMs >=
          TAXI_POLICE_CAMERA_MOVE_MS) {
        taxiCameraFollowVehicleIndex = TAXI_POLICE_CAR_SLOT;

        taxiPoliceState = TAXI_POLICE_STATE_ESCAPE_COOLDOWN;
        taxiPoliceStateStartMs = nowMs;
      }
      break;


    case TAXI_POLICE_STATE_ESCAPE_COOLDOWN:
      if (nowMs - taxiPoliceStateStartMs >=
          TAXI_POLICE_ESCAPE_COOLDOWN_MS) {
        restartTaxiPoliceLoop(nowMs);
      }
      break;
  }
}



static bool isTaxiVehiclePausedByPoliceMode(int carSlot) {
  if (!isTaxiPoliceModeActive()) return false;

  // BOY 被逮捕消失後，不再移動。
  if (carSlot == TAXI_BOY_CAR_SLOT && !taxiBoyVisible) {
    return true;
  }

  bool arrestPause =
    taxiPoliceState == TAXI_POLICE_STATE_ARREST_STOP ||
    taxiPoliceState == TAXI_POLICE_STATE_ARREST_CAMERA_CENTER ||
    taxiPoliceState == TAXI_POLICE_STATE_ARREST_FOCUS_WAIT ||
    taxiPoliceState == TAXI_POLICE_STATE_ARREST_BOY_BLINK ||
    taxiPoliceState == TAXI_POLICE_STATE_ARREST_AFTER_GONE;

  // 逮捕演出時，警車與 BOY 都停住。
  if (arrestPause &&
      (carSlot == TAXI_POLICE_CAR_SLOT ||
       carSlot == TAXI_BOY_CAR_SLOT)) {
    return true;
  }

  return false;
}


static bool shouldDrawTaxiBoyVehicle() {
  // TAXI 模式時，BOY 是一般路上角色，正常顯示。
  if (!isTaxiPoliceModeActive()) {
    return true;
  }

  if (!taxiBoyVisible) return false;

  // 只有逮捕閃爍狀態才做閃爍。
  if (taxiPoliceState != TAXI_POLICE_STATE_ARREST_BOY_BLINK) {
    return true;
  }

  unsigned long elapsedMs = millis() - taxiPoliceStateStartMs;
  unsigned long blinkIndex =
    elapsedMs / TAXI_POLICE_BOY_BLINK_INTERVAL_MS;

  return (blinkIndex % 2) == 0;
}


static void updateTaxiPoliceCamera(unsigned long nowMs) {
  if (!isTaxiPoliceModeActive()) return;

  // CAM 滑動狀態：
  // 滑到 BOY、滑回警車、滑到警車與 BOY 中間。
  if (isTaxiPoliceCameraMovingState()) {
    updateTaxiCameraMove(nowMs);
    return;
  }

  // 聚焦 BOY 時，CAM 會持續跟著 BOY。
  // BOY 在這 3 秒內仍然會移動。
  if (taxiPoliceState == TAXI_POLICE_STATE_BOY_FOCUS ||
      taxiPoliceState == TAXI_POLICE_STATE_ESCAPE_BOY_FOCUS) {
    int cameraX = 0;
    int cameraY = 0;

    getTaxiVehicleCameraTarget(
      TAXI_BOY_CAR_SLOT,
      &cameraX,
      &cameraY
    );

    taxiCameraX = cameraX;
    taxiCameraY = cameraY;
    return;
  }

  // 逮捕演出中，CAM 停在兩車中間。
  if (taxiPoliceState == TAXI_POLICE_STATE_ARREST_FOCUS_WAIT ||
      taxiPoliceState == TAXI_POLICE_STATE_ARREST_BOY_BLINK ||
      taxiPoliceState == TAXI_POLICE_STATE_ARREST_AFTER_GONE) {
    clampTaxiCameraTarget(&taxiCameraX, &taxiCameraY);
    return;
  }

// 追逐中不要只鎖警車，改看警車與 BOY 的中間。
if (taxiPoliceState == TAXI_POLICE_STATE_CHASING_BOY &&
    taxiBoyVisible) {
  int policeX = getTaxiVehicleLaneWorldX(TAXI_POLICE_CAR_SLOT);
  int policeY = getTaxiVehicleLaneWorldY(TAXI_POLICE_CAR_SLOT);

  int boyX = getTaxiVehicleLaneWorldX(TAXI_BOY_CAR_SLOT);
  int boyY = getTaxiVehicleLaneWorldY(TAXI_BOY_CAR_SLOT);

  int focusX = (policeX + boyX) / 2;
  int focusY = (policeY + boyY) / 2;

  taxiCameraX = focusX - (TAXI_SCR_W / 2);
  taxiCameraY = focusY - (TAXI_SCR_H / 2) - TAXI_CAMERA_FOLLOW_OFFSET_Y;

  clampTaxiCameraTarget(&taxiCameraX, &taxiCameraY);
  return;
}


  // 其他警車模式狀態，CAM 跟著目前指定的車。
  getTaxiNormalCameraTarget(&taxiCameraX, &taxiCameraY);
}



static int chooseNextTaxiRoadIndexForVehicleFrom(
  int carSlot,
  int fromRoadIndex,
  int previousRoadIndex
) {
  // POLICE 模式：警車追蹤 BOY。
  if (isTaxiPoliceModeActive() &&
      carSlot == TAXI_POLICE_CAR_SLOT &&
      taxiPoliceState == TAXI_POLICE_STATE_CHASING_BOY &&
      taxiBoyVisible) {
    int chaseRoad = getTaxiPoliceChaseDestinationRoad();

    if (chaseRoad >= 0 && chaseRoad < TAXI_ROAD_COUNT) {
      return chooseNextTaxiRoadIndexTowardDestination(
        fromRoadIndex,
        previousRoadIndex,
        chaseRoad
      );
    }
  }

  // TAXI 載客模式：主角計程車前往乘客目的地。
  if (isTaxiPassengerModeActive() &&
      carSlot == TAXI_MAIN_CAR_SLOT &&
      taxiPassengerServiceState == TAXI_SERVICE_DRIVING_TO_DROPOFF &&
      taxiPassengerDestinationRoadIndex >= 0) {
    return chooseNextTaxiRoadIndexTowardDestination(
      fromRoadIndex,
      previousRoadIndex,
      taxiPassengerDestinationRoadIndex
    );
  }

  // 其他狀態：正常巡航。
  return chooseNextTaxiRoadIndex(
    fromRoadIndex,
    previousRoadIndex
  );
}

static int chooseNextTaxiRoadIndexForVehicle(int carSlot, int oldCurrentRoadIndex) {
  return chooseNextTaxiRoadIndexForVehicleFrom(
    carSlot,
    taxiVehicles[carSlot].currentRoadIndex,
    oldCurrentRoadIndex
  );
}

static bool isTaxiSpecialEarlyTurn(uint8_t currentDir, uint8_t nextDir) {
  return (currentDir == TAXI_DIR_DOWN && nextDir == TAXI_DIR_LEFT) ||
         (currentDir == TAXI_DIR_UP && nextDir == TAXI_DIR_RIGHT);
}

static bool tryTaxiSpecialEarlyTurn(
  int carSlot,
  unsigned long nowMs,
  int dx,
  int dy
) {
  if (carSlot < 0 || carSlot >= TAXI_VEHICLE_COUNT) return false;

  if (abs(dx) > TAXI_EARLY_TURN_X ||
      abs(dy) > TAXI_EARLY_TURN_Y) {
    return false;
  }

  // 如果目前目標就是乘客目的地，不提前轉彎。
  // 避免還沒真的到站就觸發下車。
  if (carSlot == TAXI_MAIN_CAR_SLOT &&
      taxiPassengerServiceState == TAXI_SERVICE_DRIVING_TO_DROPOFF &&
      taxiVehicles[carSlot].targetRoadIndex == taxiPassengerDestinationRoadIndex) {
    return false;
  }

  int targetRoad = taxiVehicles[carSlot].targetRoadIndex;

  int nextRoad = chooseNextTaxiRoadIndexForVehicleFrom(
    carSlot,
    targetRoad,
    taxiVehicles[carSlot].currentRoadIndex
  );

  int targetX = getTaxiRoadCenterX(targetRoad);
  int targetY = getTaxiRoadCenterY(targetRoad);

  int nextX = getTaxiRoadCenterX(nextRoad);
  int nextY = getTaxiRoadCenterY(nextRoad);

  uint8_t nextDir = getTaxiDirectionByVector(
    nextX - targetX,
    nextY - targetY,
    taxiVehicles[carSlot].dir
  );

  if (!isTaxiSpecialEarlyTurn(taxiVehicles[carSlot].dir, nextDir)) {
    return false;
  }

  uint8_t oldDir = taxiVehicles[carSlot].dir;

  taxiVehicles[carSlot].previousRoadIndex =
    taxiVehicles[carSlot].currentRoadIndex;

  taxiVehicles[carSlot].currentRoadIndex =
    taxiVehicles[carSlot].targetRoadIndex;

  taxiVehicles[carSlot].targetRoadIndex = nextRoad;

  // DOWN → LEFT
  // 轉彎前的視覺 laneY 是 10，轉彎後是 -5，差 15px。
  // 所以這裡把 worldY 補到下一段路線上，避免轉彎後飄移。
  if (oldDir == TAXI_DIR_DOWN && nextDir == TAXI_DIR_LEFT) {
    taxiVehicles[carSlot].worldX =
      targetX - TAXI_EARLY_TURN_X;

    taxiVehicles[carSlot].worldY =
      targetY + TAXI_EARLY_TURN_Y;
  }

  // UP → RIGHT
  // 同理，提前切到下一段路線上的虛擬轉彎點。
  else if (oldDir == TAXI_DIR_UP && nextDir == TAXI_DIR_RIGHT) {
    taxiVehicles[carSlot].worldX =
      targetX + TAXI_EARLY_TURN_X;

    taxiVehicles[carSlot].worldY =
      targetY - TAXI_EARLY_TURN_Y;
  }

  taxiVehicles[carSlot].dir = nextDir;

  return true;
}

static void initTaxiVehicle(
  int carSlot,
  uint8_t spriteId,
  int startRoadIndex,
  unsigned long nowMs
) {
  taxiVehicles[carSlot].spriteId = spriteId;

  taxiVehicles[carSlot].currentRoadIndex = startRoadIndex;
  taxiVehicles[carSlot].previousRoadIndex = -1;
  taxiVehicles[carSlot].targetRoadIndex =
    chooseNextTaxiRoadIndex(taxiVehicles[carSlot].currentRoadIndex, -1);

  taxiVehicles[carSlot].worldX =
    getTaxiRoadCenterX(taxiVehicles[carSlot].currentRoadIndex);

  taxiVehicles[carSlot].worldY =
    getTaxiRoadCenterY(taxiVehicles[carSlot].currentRoadIndex);

  int targetX = getTaxiRoadCenterX(taxiVehicles[carSlot].targetRoadIndex);
  int targetY = getTaxiRoadCenterY(taxiVehicles[carSlot].targetRoadIndex);

  taxiVehicles[carSlot].dir = getTaxiDirectionByVector(
    targetX - taxiVehicles[carSlot].worldX,
    targetY - taxiVehicles[carSlot].worldY,
    TAXI_DIR_RIGHT
  );

  taxiVehicles[carSlot].lastMoveMs = nowMs;
  taxiVehicles[carSlot].moveIntervalMs = TAXI_VEHICLE_MOVE_INTERVAL_MS;
  taxiVehicles[carSlot].stepX = TAXI_VEHICLE_STEP_X;
  taxiVehicles[carSlot].stepY = TAXI_VEHICLE_STEP_Y;
}

static void updateTaxiVehicleMovement(int carSlot, unsigned long nowMs) {
  // 不啟用的車輛不移動。
  // 例如 TAXI 載客模式時，CITY_BOY 不參與。
  if (!isTaxiVehicleSlotActive(carSlot)) {
    taxiVehicles[carSlot].lastMoveMs = nowMs;
    return;
  }

  // TAXI 載客模式裡，主角車在上下車演出時會暫停。
  if (carSlot == TAXI_MAIN_CAR_SLOT && isTaxiMainVehiclePaused()) {
    taxiVehicles[carSlot].lastMoveMs = nowMs;
    return;
  }

  // POLICE 模式裡，逮捕演出時警車與 BOY 會暫停。
  if (isTaxiVehiclePausedByPoliceMode(carSlot)) {
    taxiVehicles[carSlot].lastMoveMs = nowMs;
    return;
  }

  // 全車輛保持距離。
  // 如果前方或附近有其他車，這台車先停住，避免疊圖。
  if (isTaxiVehicleTooCloseToAnyOtherVehicle(carSlot)) {
    taxiVehicles[carSlot].lastMoveMs = nowMs;
    return;
  }

  if (nowMs - taxiVehicles[carSlot].lastMoveMs <
      taxiVehicles[carSlot].moveIntervalMs) {
    return;
  }

  taxiVehicles[carSlot].lastMoveMs = nowMs;

  int targetRoad = taxiVehicles[carSlot].targetRoadIndex;

  if (targetRoad < 0 || targetRoad >= TAXI_ROAD_COUNT) {
    taxiVehicles[carSlot].targetRoadIndex =
      chooseNextTaxiRoadIndexForVehicle(
        carSlot,
        taxiVehicles[carSlot].previousRoadIndex
      );

    targetRoad = taxiVehicles[carSlot].targetRoadIndex;
  }

  int targetX = getTaxiRoadCenterX(targetRoad);
  int targetY = getTaxiRoadCenterY(targetRoad);

  int dx = targetX - taxiVehicles[carSlot].worldX;
  int dy = targetY - taxiVehicles[carSlot].worldY;

  if (tryTaxiSpecialEarlyTurn(carSlot, nowMs, dx, dy)) {
    return;
  }

  if (dx != 0) {
    int sx =
      (dx > 0) ?
      taxiVehicles[carSlot].stepX :
      -taxiVehicles[carSlot].stepX;

    if (abs(dx) <= taxiVehicles[carSlot].stepX) {
      taxiVehicles[carSlot].worldX = targetX;
    } else {
      taxiVehicles[carSlot].worldX += sx;
    }
  }

  if (dy != 0) {
    int sy =
      (dy > 0) ?
      taxiVehicles[carSlot].stepY :
      -taxiVehicles[carSlot].stepY;

    if (abs(dy) <= taxiVehicles[carSlot].stepY) {
      taxiVehicles[carSlot].worldY = targetY;
    } else {
      taxiVehicles[carSlot].worldY += sy;
    }
  }

  if (taxiVehicles[carSlot].worldX == targetX &&
      taxiVehicles[carSlot].worldY == targetY) {
    int oldCurrent = taxiVehicles[carSlot].currentRoadIndex;

    taxiVehicles[carSlot].previousRoadIndex =
      taxiVehicles[carSlot].currentRoadIndex;

    taxiVehicles[carSlot].currentRoadIndex =
      taxiVehicles[carSlot].targetRoadIndex;

    taxiVehicles[carSlot].targetRoadIndex =
      chooseNextTaxiRoadIndexForVehicle(carSlot, oldCurrent);

    int nextX = getTaxiRoadCenterX(taxiVehicles[carSlot].targetRoadIndex);
    int nextY = getTaxiRoadCenterY(taxiVehicles[carSlot].targetRoadIndex);

    taxiVehicles[carSlot].dir = getTaxiDirectionByVector(
      nextX - taxiVehicles[carSlot].worldX,
      nextY - taxiVehicles[carSlot].worldY,
      taxiVehicles[carSlot].dir
    );
  }
}



  

static void updateTaxiVehicles(unsigned long nowMs) {
  for (int i = 0; i < TAXI_VEHICLE_COUNT; i++) {
    updateTaxiVehicleMovement(i, nowMs);
  }
}


static void updateTaxiPassengerService(unsigned long nowMs) {
  // 只有 TAXI 載客模式會執行乘客上下車流程。
  // 警車模式與直升機模式都維持一般道路巡航。
  if (!isTaxiPassengerModeActive()) {
    return;
  }

  switch (taxiPassengerServiceState) {
    case TAXI_SERVICE_SEARCHING: {
      int passengerSlot = findTaxiPassengerTouchingMainTaxi();

      if (passengerSlot >= 0) {
        beginTaxiPassengerBoarding(passengerSlot, nowMs);
      }
      break;
    }

    case TAXI_SERVICE_PICKUP_CAMERA_TO_CENTER:
      if (nowMs - taxiPassengerStateStartMs >=
          TAXI_PICKUP_CAMERA_TO_CENTER_MS) {
        taxiCameraX = taxiCameraMoveTargetX;
        taxiCameraY = taxiCameraMoveTargetY;

        taxiPassengerServiceState = TAXI_SERVICE_PICKUP_SHOW_WAIT;
        taxiPassengerStateStartMs = nowMs;
      }
      break;

    case TAXI_SERVICE_PICKUP_SHOW_WAIT:
      if (nowMs - taxiPassengerStateStartMs >=
          TAXI_PASSENGER_BOARD_WAIT_MS) {
        clearTaxiPassengers();

        taxiPassengerServiceState = TAXI_SERVICE_PICKUP_PASSENGER_GONE_WAIT;
        taxiPassengerStateStartMs = nowMs;
      }
      break;

    case TAXI_SERVICE_PICKUP_PASSENGER_GONE_WAIT:
      if (nowMs - taxiPassengerStateStartMs >=
          TAXI_PICKUP_AFTER_GONE_WAIT_MS) {
        int normalCameraX = 0;
        int normalCameraY = 0;

        getTaxiNormalCameraTarget(&normalCameraX, &normalCameraY);

        beginTaxiCameraMoveTo(
          normalCameraX,
          normalCameraY,
          nowMs,
          TAXI_PICKUP_CAMERA_RETURN_MS
        );

        taxiPassengerServiceState = TAXI_SERVICE_PICKUP_CAMERA_RETURN;
        taxiPassengerStateStartMs = nowMs;
      }
      break;

    case TAXI_SERVICE_PICKUP_CAMERA_RETURN:
      if (nowMs - taxiPassengerStateStartMs >=
          TAXI_PICKUP_CAMERA_RETURN_MS) {
        taxiCameraX = taxiCameraMoveTargetX;
        taxiCameraY = taxiCameraMoveTargetY;

        taxiPassengerServiceState = TAXI_SERVICE_PICKUP_READY_WAIT;
        taxiPassengerStateStartMs = nowMs;
      }
      break;

    case TAXI_SERVICE_PICKUP_READY_WAIT:
      if (nowMs - taxiPassengerStateStartMs >=
          TAXI_PICKUP_READY_WAIT_MS) {
        taxiPassengerServiceState = TAXI_SERVICE_RIDE_CRUISE;
        taxiPassengerStateStartMs = nowMs;
      }
      break;

    case TAXI_SERVICE_RIDE_CRUISE:
      if (nowMs - taxiPassengerStateStartMs >=
          TAXI_PASSENGER_RIDE_CRUISE_MS) {
        beginTaxiPassengerDestinationRun(nowMs);
      }
      break;

case TAXI_SERVICE_DRIVING_TO_DROPOFF:
  if (isTaxiMainCarReadyForDropoff(nowMs)) {
    beginTaxiPassengerDropoff(nowMs);
  }
  break;
  
case TAXI_SERVICE_DROPOFF_CAMERA_TO_CENTER:
  if (nowMs - taxiPassengerStateStartMs >=
      TAXI_PICKUP_CAMERA_TO_CENTER_MS) {
    taxiCameraX = taxiCameraMoveTargetX;
    taxiCameraY = taxiCameraMoveTargetY;

    // 車子已經停住、鏡頭也到位，這時乘客才出現。
    showTaxiDropoffPassengerAtDestination();


// 乘客出現後，再讓 CAM 滑到乘客身上。
// 用來觀察車子與乘客下車位置的距離。
int passengerCameraX =
  taxiPickupPassengerX - (TAXI_SCR_W / 2);

int passengerCameraY =
  taxiPickupPassengerY - (TAXI_SCR_H / 2);

beginTaxiCameraMoveTo(
  passengerCameraX,
  passengerCameraY,
  nowMs,
  TAXI_DROPOFF_CAMERA_TO_PASSENGER_MS
);

    // 給車子新的目標，讓車子可以開走。
    if (taxiVehicles[TAXI_MAIN_CAR_SLOT].targetRoadIndex ==
        taxiVehicles[TAXI_MAIN_CAR_SLOT].currentRoadIndex) {
      int nextRoad = chooseNextTaxiRoadIndex(
        taxiVehicles[TAXI_MAIN_CAR_SLOT].currentRoadIndex,
        taxiVehicles[TAXI_MAIN_CAR_SLOT].previousRoadIndex
      );

      setTaxiVehicleTargetRoad(
        TAXI_MAIN_CAR_SLOT,
        nextRoad,
        taxiVehicles[TAXI_MAIN_CAR_SLOT].dir
      );
    }

    taxiPassengerServiceState = TAXI_SERVICE_DROPOFF_SHOW_WAIT;
    taxiPassengerStateStartMs = nowMs;
  }
  break;

case TAXI_SERVICE_DROPOFF_SHOW_WAIT:
  if (nowMs - taxiPassengerStateStartMs >=
      TAXI_PASSENGER_DROPOFF_WAIT_MS) {
    // 車子已經開走一段時間後，才清掉下車乘客。
    clearTaxiPassengers();

    taxiPassengerServiceState = TAXI_SERVICE_AFTER_DROPOFF_CRUISE;
    taxiPassengerStateStartMs = nowMs;
  }
  break;

    case TAXI_SERVICE_DROPOFF_PASSENGER_GONE_WAIT:
      if (nowMs - taxiPassengerStateStartMs >=
          TAXI_PICKUP_AFTER_GONE_WAIT_MS) {
        int normalCameraX = 0;
        int normalCameraY = 0;

        getTaxiNormalCameraTarget(&normalCameraX, &normalCameraY);

        beginTaxiCameraMoveTo(
          normalCameraX,
          normalCameraY,
          nowMs,
          TAXI_PICKUP_CAMERA_RETURN_MS
        );

        taxiPassengerServiceState = TAXI_SERVICE_DROPOFF_CAMERA_RETURN;
        taxiPassengerStateStartMs = nowMs;
      }
      break;

    case TAXI_SERVICE_DROPOFF_CAMERA_RETURN:
      if (nowMs - taxiPassengerStateStartMs >=
          TAXI_PICKUP_CAMERA_RETURN_MS) {
        taxiCameraX = taxiCameraMoveTargetX;
        taxiCameraY = taxiCameraMoveTargetY;

        taxiPassengerServiceState = TAXI_SERVICE_DROPOFF_READY_WAIT;
        taxiPassengerStateStartMs = nowMs;
      }
      break;

    case TAXI_SERVICE_DROPOFF_READY_WAIT:
      if (nowMs - taxiPassengerStateStartMs >=
          TAXI_PICKUP_READY_WAIT_MS) {
        taxiPassengerServiceState = TAXI_SERVICE_AFTER_DROPOFF_CRUISE;
        taxiPassengerStateStartMs = nowMs;

        if (taxiVehicles[TAXI_MAIN_CAR_SLOT].targetRoadIndex ==
            taxiVehicles[TAXI_MAIN_CAR_SLOT].currentRoadIndex) {
          int nextRoad = chooseNextTaxiRoadIndex(
            taxiVehicles[TAXI_MAIN_CAR_SLOT].currentRoadIndex,
            taxiVehicles[TAXI_MAIN_CAR_SLOT].previousRoadIndex
          );

          setTaxiVehicleTargetRoad(
            TAXI_MAIN_CAR_SLOT,
            nextRoad,
            taxiVehicles[TAXI_MAIN_CAR_SLOT].dir
          );
        }
      }
      break;

    case TAXI_SERVICE_AFTER_DROPOFF_CRUISE:
      if (nowMs - taxiPassengerStateStartMs >=
          TAXI_PASSENGER_AFTER_DROPOFF_CRUISE_MS) {
taxiPassengerPickedPointIndex = -1;
taxiPassengerDestinationPointIndex = -1;
taxiPassengerDestinationRoadIndex = -1;
taxiPassengerRidingFrameIndex = 0;
taxiPassengerHasRider = false;

spawnTaxiWaitingPassengers();

        taxiPassengerServiceState = TAXI_SERVICE_SEARCHING;
        taxiPassengerStateStartMs = nowMs;
      }
      break;
  }
}

static void updateTaxiCamera() {
  unsigned long nowMs = millis();

  // 直升機模式中，CAM 持續鎖定直升機中心。
  if (isTaxiHelicopterModeActive()) {
    getTaxiHelicopterCameraTarget(
      &taxiCameraX,
      &taxiCameraY
    );
    return;
  }

  if (isTaxiPoliceModeActive()) {
    updateTaxiPoliceCamera(nowMs);
    return;
  }

if (taxiPassengerServiceState == TAXI_SERVICE_PICKUP_CAMERA_TO_CENTER ||
    taxiPassengerServiceState == TAXI_SERVICE_PICKUP_CAMERA_RETURN ||
    taxiPassengerServiceState == TAXI_SERVICE_DROPOFF_CAMERA_TO_CENTER ||
    taxiPassengerServiceState == TAXI_SERVICE_DROPOFF_CAMERA_RETURN) {
  updateTaxiCameraMove(nowMs);
  return;
}

// 下車乘客剛出現後，短暫讓 CAM 滑到乘客身上。
// 滑完後就會繼續往下走，回到正常 TAXI 跟隨。
if (taxiPassengerServiceState == TAXI_SERVICE_DROPOFF_SHOW_WAIT &&
    nowMs - taxiPassengerStateStartMs <
    TAXI_DROPOFF_CAMERA_TO_PASSENGER_MS) {
  updateTaxiCameraMove(nowMs);
  return;
}

if (taxiPassengerServiceState == TAXI_SERVICE_PICKUP_SHOW_WAIT ||
    taxiPassengerServiceState == TAXI_SERVICE_PICKUP_PASSENGER_GONE_WAIT ||
    taxiPassengerServiceState == TAXI_SERVICE_PICKUP_READY_WAIT ||
    taxiPassengerServiceState == TAXI_SERVICE_DROPOFF_PASSENGER_GONE_WAIT ||
    taxiPassengerServiceState == TAXI_SERVICE_DROPOFF_READY_WAIT) {
  clampTaxiCameraTarget(&taxiCameraX, &taxiCameraY);
  return;
}

  getTaxiNormalCameraTarget(&taxiCameraX, &taxiCameraY);
}


static void drawTaxiCityRoads() {
  for (int i = 0; i < TAXI_ROAD_COUNT; i++) {
    int drawX = taxiRoadTiles[i].x - taxiCameraX;
    int drawY = taxiRoadTiles[i].y - taxiCameraY;

drawTaxiIndexedFrame(
  CITY_ROAD,
  CITY_ROAD_PALETTE,
  TAXI_ROAD_SHEET_W,
  TAXI_ROAD_TILE_W,
  TAXI_ROAD_TILE_H,
  taxiRoadTiles[i].tileId,
  false,
  drawX,
  drawY
);
  }
}

static void drawTaxiCityObjectByIndex(int objectIndex) {
  int drawX = taxiCityObjects[objectIndex].x - taxiCameraX;
  int drawY = taxiCityObjects[objectIndex].y - taxiCameraY;

drawTaxiIndexedFrame(
  CITY_OBJ,
  CITY_OBJ_PALETTE,
  TAXI_OBJ_SHEET_W,
  TAXI_OBJ_TILE_W,
  TAXI_OBJ_TILE_H,
  taxiCityObjects[objectIndex].objId,
  taxiCityObjects[objectIndex].flipX,
  drawX,
  drawY
);
}

static void drawTaxiCityObjects() {
  for (int i = 0; i < TAXI_CITY_OBJECT_COUNT; i++) {
    drawTaxiCityObjectByIndex(i);
  }
}

static void drawTaxiPassengerByIndex(int passengerIndex) {
  if (passengerIndex < 0 || passengerIndex >= TAXI_PASSENGER_ACTIVE_COUNT) return;
  if (!taxiPassengers[passengerIndex].active) return;

int drawX =
  taxiPassengers[passengerIndex].worldX -
  taxiCameraX;

int drawY =
  taxiPassengers[passengerIndex].worldY -
  taxiCameraY;

drawTaxiIndexedFrame(
  CITY_P,
  CITY_P_PALETTE,
  TAXI_PASSENGER_SHEET_W,
  TAXI_PASSENGER_FRAME_W,
  TAXI_PASSENGER_FRAME_H,
  taxiPassengers[passengerIndex].frameIndex,
  taxiPassengers[passengerIndex].flipX,
  drawX,
  drawY
);
}

static void drawTaxiPassengers() {
  for (int i = 0; i < TAXI_PASSENGER_ACTIVE_COUNT; i++) {
    drawTaxiPassengerByIndex(i);
  }
}

static int getTaxiVehicleSortY(int carSlot) {
  int laneX = 0;
  int laneY = 0;

  getTaxiRightLaneOffset(
    taxiVehicles[carSlot].dir,
    &laneX,
    &laneY
  );

  return taxiVehicles[carSlot].worldY + laneY;
}

static int getTaxiVehicleSortX(int carSlot) {
  int laneX = 0;
  int laneY = 0;

  getTaxiRightLaneOffset(
    taxiVehicles[carSlot].dir,
    &laneX,
    &laneY
  );

  return taxiVehicles[carSlot].worldX + laneX;
}

static bool shouldTaxiVehicleDrawBefore(int a, int b) {
  int ay = getTaxiVehicleSortY(a);
  int by = getTaxiVehicleSortY(b);

  if (ay != by) {
    return ay < by;
  }

  int ax = getTaxiVehicleSortX(a);
  int bx = getTaxiVehicleSortX(b);

  return ax < bx;
}

static void drawTaxiVehicleByIndex(int carSlot) {
  if (!isTaxiVehicleSlotActive(carSlot)) return;

  if (carSlot == TAXI_BOY_CAR_SLOT &&
      !shouldDrawTaxiBoyVehicle()) {
    return;
  }

  int spriteId = taxiVehicles[carSlot].spriteId;

const uint8_t* bitmap = taxiCarSpriteDefs[spriteId].bitmap;
const uint16_t* palette = taxiCarSpriteDefs[spriteId].palette;
  int sheetW = taxiCarSpriteDefs[spriteId].sheetW;
  int frameW = taxiCarSpriteDefs[spriteId].frameW;
  int frameH = taxiCarSpriteDefs[spriteId].frameH;
  int anchorX = taxiCarSpriteDefs[spriteId].anchorX;
  int anchorY = taxiCarSpriteDefs[spriteId].anchorY;

  int laneX = 0;
  int laneY = 0;

  getTaxiRightLaneOffset(
    taxiVehicles[carSlot].dir,
    &laneX,
    &laneY
  );

  int drawX =
    taxiVehicles[carSlot].worldX +
    laneX -
    taxiCameraX -
    anchorX;

  int drawY =
    taxiVehicles[carSlot].worldY +
    laneY -
    taxiCameraY -
    anchorY;

  int frameIndex = getTaxiVehicleFrameIndex(taxiVehicles[carSlot].dir);
  bool flipX = getTaxiVehicleFlipX(taxiVehicles[carSlot].dir);

drawTaxiIndexedFrame(
  bitmap,
  palette,
  sheetW,
  frameW,
  frameH,
  frameIndex,
  flipX,
  drawX,
  drawY
);
}

static void drawTaxiVehiclesSorted() {
  int order[TAXI_VEHICLE_COUNT];

  for (int i = 0; i < TAXI_VEHICLE_COUNT; i++) {
    order[i] = i;
  }

  for (int i = 0; i < TAXI_VEHICLE_COUNT - 1; i++) {
    for (int j = i + 1; j < TAXI_VEHICLE_COUNT; j++) {
      if (!shouldTaxiVehicleDrawBefore(order[i], order[j])) {
        int temp = order[i];
        order[i] = order[j];
        order[j] = temp;
      }
    }
  }

  for (int i = 0; i < TAXI_VEHICLE_COUNT; i++) {
    drawTaxiVehicleByIndex(order[i]);
  }
}

static void renderTaxiScene() {
  display.fillScreen(TAXI_BG_COLOR);

  drawTaxiCityRoads();       // 第一層：道路
  drawTaxiCityObjects();     // 第二層：CITY_OBJ
  drawTaxiPassengers();      // 第三層：乘客
  drawTaxiVehiclesSorted();  // 第四層：道路交通工具
  drawTaxiHelicopter();      // 第五層：空中的直升機
  
  drawThemeClockText();      // 最上層：時鐘文字
}

static void TaxiModeInit() {
  if (taxiModeReady) return;

  randomSeed(millis());

  unsigned long nowMs = millis();

if (TAXI_START_GAME_MODE == 3) {
  taxiGameMode = random(3);  // 0 = TAXI，1 = 警車，2 = 直升機
} else if (TAXI_START_GAME_MODE <= TAXI_GAME_MODE_HELICOPTER) {
  taxiGameMode = TAXI_START_GAME_MODE;
} else {
  // 設定值超出範圍時，回到三模式隨機。
  taxiGameMode = random(3);
}
  

int taxiStartRoadIndex =
  chooseRandomTaxiRoadIndexAvoid(-1, -1);

int policeStartRoadIndex =
  chooseRandomTaxiRoadIndexAvoid(-1, -1);

int boyStartRoadIndex =
  chooseRandomTaxiRoadIndexAvoid(policeStartRoadIndex, -1);

   initTaxiVehicle(
    TAXI_MAIN_CAR_SLOT,
    TAXI_SPRITE_TAXI,
    taxiStartRoadIndex,
    nowMs
  );

  initTaxiVehicle(
    TAXI_POLICE_CAR_SLOT,
    TAXI_SPRITE_POLICE,
    policeStartRoadIndex,
    nowMs + 20
  );

  initTaxiVehicle(
    TAXI_BOY_CAR_SLOT,
    TAXI_SPRITE_BOY,
    boyStartRoadIndex,
    nowMs + 40
  );

  clearTaxiPassengers();

  taxiPassengerServiceState = TAXI_SERVICE_SEARCHING;
  taxiPassengerStateStartMs = nowMs;
  taxiPassengerPickedPointIndex = -1;
  taxiPassengerDestinationPointIndex = -1;
  taxiPassengerDestinationRoadIndex = -1;
  taxiPassengerRidingFrameIndex = 0;
  taxiPassengerHasRider = false;

  taxiPickupPassengerX = 0;
  taxiPickupPassengerY = 0;

  taxiCameraMoveStartX = 0;
  taxiCameraMoveStartY = 0;
  taxiCameraMoveTargetX = 0;
  taxiCameraMoveTargetY = 0;
  taxiCameraMoveStartMs = nowMs;
  taxiCameraMoveDurationMs = 0;

  taxiPoliceState = TAXI_POLICE_STATE_INTRO_PATROL;
  taxiPoliceStateStartMs = nowMs;
  taxiPoliceChaseStartMs = 0;
  taxiBoyVisible = true;

  // 直升機使用獨立座標，不影響原本三台道路交通工具。
  initTaxiHelicopter(nowMs);

  if (isTaxiPoliceModeActive()) {
    taxiCameraFollowVehicleIndex = TAXI_POLICE_CAR_SLOT;
  } else if (isTaxiHelicopterModeActive()) {
    // 直升機模式由 updateTaxiCamera() 直接鎖定直升機。
    taxiCameraFollowVehicleIndex = TAXI_MAIN_CAR_SLOT;
  } else {
    taxiCameraFollowVehicleIndex = TAXI_MAIN_CAR_SLOT;
    spawnTaxiWaitingPassengers();
  }

  updateTaxiCamera();

  // 每次完整初始化後，同步目前車輛位置並重新開始卡死計時。
  resetTaxiStallMonitor(nowMs);

  taxiModeReady = true;
}

// =====================================================
// 卡死檢查與重新初始化
//
// 只有目前模式的主角會觸發重置：
// 1. TAXI 載客模式：主 TAXI。
// 2. 警車追逐模式：警車。
// 3. 直升機模式：直升機。
//
// 背景中的 TAXI、警車或 BOY 即使停止，也不會觸發重置。
// 直升機正常等待只有 15 秒，遠短於 2 分鐘，因此不會誤觸。
// =====================================================

static bool checkTaxiStallAndReinitialize(unsigned long nowMs) {
  if (!taxiStallMonitorReady) {
    resetTaxiStallMonitor(nowMs);
    return false;
  }

  // 保留所有道路交通工具的位置紀錄。
  // 目前只用該模式的主角判斷是否需要重新初始化。
  for (int i = 0; i < TAXI_VEHICLE_COUNT; i++) {
    if (taxiVehicles[i].worldX != taxiStallLastVehicleX[i] ||
        taxiVehicles[i].worldY != taxiStallLastVehicleY[i]) {
      taxiStallLastVehicleX[i] = taxiVehicles[i].worldX;
      taxiStallLastVehicleY[i] = taxiVehicles[i].worldY;
      taxiStallLastMoveMs[i] = nowMs;
    }
  }

  // 直升機使用獨立座標，因此另外更新它的最後移動時間。
  if (taxiHelicopterWorldX != taxiStallLastHelicopterX ||
      taxiHelicopterWorldY != taxiStallLastHelicopterY) {
    taxiStallLastHelicopterX = taxiHelicopterWorldX;
    taxiStallLastHelicopterY = taxiHelicopterWorldY;
    taxiStallHelicopterLastMoveMs = nowMs;
  }

  bool shouldReinitialize = false;

  if (isTaxiHelicopterModeActive()) {
    // 直升機模式只監控直升機。
    if (nowMs - taxiStallHelicopterLastMoveMs >=
        TAXI_STALL_REINIT_MS) {
      shouldReinitialize = true;
    }
  } else {
    // 載客模式只監控主 TAXI；警車模式只監控警車。
    int monitoredVehicleSlot = isTaxiPoliceModeActive() ?
      TAXI_POLICE_CAR_SLOT :
      TAXI_MAIN_CAR_SLOT;

    if (nowMs - taxiStallLastMoveMs[monitoredVehicleSlot] >=
        TAXI_STALL_REINIT_MS) {
      shouldReinitialize = true;
    }
  }

  if (!shouldReinitialize) {
    return false;
  }

  Serial.println("TaxiMode 主角卡死超過 2 分鐘，重新初始化");

  // 清除原本的一次性初始化旗標，讓 TaxiModeInit() 完整重跑。
  taxiModeReady = false;
  taxiStallMonitorReady = false;

  TaxiModeInit();

  return true;
}

void TaxiMode() {
  TaxiModeInit();

  unsigned long nowMs = millis();

  // 三個模式中，道路上的 TAXI、警車與 BOY 都維持原本巡航。
  updateTaxiVehicles(nowMs);

  // 直升機模式另外更新自由飛行與兩格動畫。
  updateTaxiHelicopter(nowMs);

  if (isTaxiPoliceModeActive()) {
    updateTaxiPoliceScenario(nowMs);
  } else if (isTaxiPassengerModeActive()) {
    updateTaxiPassengerService(nowMs);
  }

  // 主體車輛連續 2 分鐘沒有位移時，完整重新初始化。
  // 重新初始化完成後直接繪製新場景，避免本幀繼續使用舊狀態。
  if (checkTaxiStallAndReinitialize(nowMs)) {
    renderTaxiScene();
    wait_with_display(30);
    return;
  }

  updateTaxiCamera();

  renderTaxiScene();

  wait_with_display(30);
}
