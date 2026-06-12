#include "Kart.h"

// =====================================================
// 螢幕大小
// =====================================================
static const int KART_SCR_W = 64;
static const int KART_SCR_H = 64;

// =====================================================
// 透明色
// =====================================================
static const uint16_t KART_TRANSPARENT = 0x1c27;

// =====================================================
// 遠景圖資
// KART_HILL[] : 單一圖片
// 單張 80x14
// =====================================================
static const int HILL_SHEET_W = 80;
static const int HILL_FRAME_W = 80;
static const int HILL_FRAME_H = 14;

// =====================================================
// 軌道圖資
// KART_TRACK[] : 12 張圖
// 總寬 960，高 41
// 單張 80x41
// frame 0~3   = 直線循環
// frame 4~7   = 右轉入彎，只播放一次
// frame 8~11  = 右轉持續循環
// 回直線時使用 frame 7~4 反向播放
// 左轉使用同一組右轉圖，繪圖時水平反轉
// =====================================================
static const int TRACK_SHEET_W = 960;
static const int TRACK_FRAME_W = 80;
static const int TRACK_FRAME_H = 41;
static const int TRACK_FRAMES  = 12;

// =====================================================
// 終點箭頭圖資
// KART_END[] : 總寬 256，高 16
// 單張 16x16，共 16 張
// =====================================================
static const int KART_END_SHEET_W  = 256;
static const int KART_END_FRAME_W  = 16;
static const int KART_END_FRAME_H  = 16;
static const int KART_END_FRAMES   = 16;

// =====================================================
// 賽車角色圖資
// frame 規則：
// 0      = 直行 / 待機
// 1~4    = 右轉
// 左轉    = 1~4 水平翻轉
// 0~10   = 旋轉一圈動畫
// 10~11  = 勝利手勢循環
// =====================================================
struct KartDriverDef {
  const uint8_t* sheet;
  const uint16_t* palette;
  int sheetW;
  int frameW;
  int frameH;
};

static const KartDriverDef DRIVER_MARIO = { KART_MARIO, KART_MARIO_PALETTE, 300, 25, 25 };
static const KartDriverDef DRIVER_LUIGI = { KART_LUIGI, KART_LUIGI_PALETTE, 300, 25, 26 };
static const KartDriverDef DRIVER_YOSHI = { KART_YOSHI, KART_YOSHI_PALETTE, 300, 25, 26 };
static const KartDriverDef DRIVER_TODE  = { KART_TODE,  KART_TODE_PALETTE,  300, 25, 24 };
static const KartDriverDef DRIVER_CUBA  = { KART_CUBA,  KART_CUBA_PALETTE,  300, 25, 27 };
static const KartDriverDef DRIVER_KONG  = { KART_KONG,  KART_KONG_PALETTE,  312, 26, 26 };

static const KartDriverDef* const kartDrivers[] = {
  &DRIVER_MARIO,
  &DRIVER_LUIGI,
  &DRIVER_YOSHI,
  &DRIVER_TODE,
  &DRIVER_CUBA,
  &DRIVER_KONG,
      
};

static const int KART_DRIVER_COUNT = sizeof(kartDrivers) / sizeof(kartDrivers[0]);

// =====================================================
// 三位選手狀態
// =====================================================
struct RaceCarState {
  const KartDriverDef* driver;
  int driverIndex;

  int x;
  int y;
  int targetX;
  int targetY;

  int progress;               // 比賽進度，數值越大越領先
  int baseSpeed;              // 基本速度
  int boostSpeed;             // 衝刺加成
  unsigned long boostEndMs;   // 衝刺結束時間
  unsigned long nextBoostMs;  // 下一次衝刺觸發時間

  int bobOffset;              // 起伏偏移
  int bobTarget;              // 目標起伏
  unsigned long nextBobMs;    // 下一次更新起伏目標時間

  int frameIndex;             // 目前 frame
  bool mirrorX;               // 是否水平翻轉

  // 另外兩位跟隨第一名動作的延遲資料
  unsigned long delayedMotionStartMs;
  int pendingFrameIndex;
  bool pendingMirrorX;
};

static RaceCarState raceCars[3];

// =====================================================
// 優勝者資料
// 後續終點、旋轉、勝利動作都套用這位
// =====================================================
static const KartDriverDef* currentWinnerDriver = &DRIVER_MARIO;
static int currentWinnerDriverIndex = 0;

// =====================================================
// 輸家索引
// 箭頭播放期間往下退場
// =====================================================
static int kartLoserAIdx = -1;
static int kartLoserBIdx = -1;

// =====================================================
// 領先者動作狀態
// 中間位置是目前第一名，負責表演轉彎
// =====================================================
enum KartHeroState {
  KART_HERO_STRAIGHT = 0,
  KART_HERO_TURN_RIGHT_IN,
  KART_HERO_TURN_RIGHT_HOLD,
  KART_HERO_TURN_RIGHT_OUT,
  KART_HERO_TURN_LEFT_IN,
  KART_HERO_TURN_LEFT_HOLD,
  KART_HERO_TURN_LEFT_OUT
};

static KartHeroState kartHeroState = KART_HERO_STRAIGHT;

// =====================================================
// KART_TRACK 賽道動畫狀態
// 直線：0~3 循環
// 右轉：4~7 一次，8~11 循環，回直線 7~4 一次
// 左轉：同右轉 frame，但水平反轉繪製
// =====================================================
enum KartTrackAnimMode {
  KART_TRACK_STRAIGHT_LOOP = 0,
  KART_TRACK_TURN_IN,
  KART_TRACK_TURN_HOLD,
  KART_TRACK_TURN_OUT
};

static KartTrackAnimMode kartTrackAnimMode = KART_TRACK_STRAIGHT_LOOP;
static int kartTrackAnimStep = 0;

// =====================================================
// 整體主題流程狀態
// ENTER_GRID        : 三位選手先排好位子，由下往上進場
// GRID_HOLD         : 定位後等待 2 秒
// COUNTDOWN         : 倒數 5 4 3 2 1 GO
// RUNNING           : 正常比賽 1 分鐘
// END_ARROW_PLAY    : 播放終點箭頭，輸家開始退場，軌道仍繼續跑
// END_ARROW_HOLD    : 箭頭停最後一張 2 秒，輸家繼續退場，軌道仍繼續跑
// END_TRACK_STOP    : 軌道停止，等待 2 秒，只剩優勝者
// END_ROTATE        : 優勝者旋轉 0~10
// END_ROTATE_WAIT   : 旋轉完等待 2 秒
// END_VICTORY       : 優勝者勝利手勢 10 / 11
// REST              : 等待期，維持 frame 10
// TRANSITION        : 黑白格由左向右清除畫面
// =====================================================
enum KartModePhase {
  KART_PHASE_ENTER_GRID = 0,
  KART_PHASE_GRID_HOLD,
  KART_PHASE_COUNTDOWN,
  KART_PHASE_RUNNING,
  KART_PHASE_END_ARROW_PLAY,
  KART_PHASE_END_ARROW_HOLD,
  KART_PHASE_END_TRACK_STOP,
  KART_PHASE_END_ROTATE,
  KART_PHASE_END_ROTATE_WAIT,
  KART_PHASE_END_VICTORY,
  KART_PHASE_REST,
  KART_PHASE_TRANSITION
};

static KartModePhase kartModePhase = KART_PHASE_ENTER_GRID;

// =====================================================
// 固定位置
// =====================================================
static int kartHillBaseX = 0;
static int kartHillX = 0;
static int kartHillY = 0;

static int kartTrackFrameIndex = 0;
static bool kartTrackMirrorX = false;
static int kartTrackBaseX = 0;
static int kartTrackX = 0;
static int kartTrackY = 0;

// =====================================================
// 優勝者單人演出位置
// =====================================================
static int kartWinnerBaseX = 0;
static int kartWinnerBaseY = 38;
static int kartWinnerX = 0;
static int kartWinnerY = 38;
static int kartWinnerFrame = 0;
static bool kartWinnerMirrorX = false;

// =====================================================
// 偏移量
// =====================================================
static int kartHillOffsetX = 0;
static int kartTrackOffsetX = 0;
static int kartWinnerOffsetX = 0;

// =====================================================
// 動畫狀態
// =====================================================
static int kartTurnStep = 0;
static int kartEndArrowFrame = 0;
static int kartRotateFrame = 0;
static int kartVictoryFrame = 10;
static int kartVictoryLoopCount = 0;
static int kartTransitionColumn = 0;

// =====================================================
// 計時
// =====================================================
static unsigned long kartLastHeroAnimMs = 0;
static unsigned long kartLastTrackAnimMs = 0;
static unsigned long kartNextStateChangeMs = 0;
static unsigned long kartPhaseStartMs = 0;
static unsigned long kartLastEndAnimMs = 0;
static unsigned long kartLastArrowAnimMs = 0;
static unsigned long kartLastRaceUpdateMs = 0;
static unsigned long kartLastTransitionMs = 0;

// =====================================================
// 節奏參數
// =====================================================
static const unsigned long KART_ENTER_STEP_MS             = 30;      // 進場移動速度
static const unsigned long KART_GRID_HOLD_MS              = 2000;    // 三車定位後等待 2 秒
static const unsigned long KART_COUNTDOWN_INTERVAL_MS     = 1000;    // 倒數每秒一拍
static const unsigned long KART_RUN_DURATION_MS           = 60000;   // 正式比賽 1 分鐘

static const unsigned long KART_TRACK_ANIM_INTERVAL_MS    = 120;     // 軌道 frame 播放速度
static const unsigned long KART_RACE_UPDATE_MS            = 80;      // 中段比賽更新節奏
static const unsigned long KART_HERO_STEP_INTERVAL_MS     = 150;     // 領先者轉向表演速度
static const unsigned long KART_STRAIGHT_MIN_MS           = 4000;    // 直線最短持續時間
static const unsigned long KART_STRAIGHT_MAX_MS           = 8000;    // 直線最長持續時間
static const unsigned long KART_TURN_HOLD_MIN_MS          = 4000;    // 持續轉彎最短停留
static const unsigned long KART_TURN_HOLD_MAX_MS          = 6000;    // 持續轉彎最長停留

static const unsigned long KART_END_ARROW_INTERVAL_MS     = 200;     // 箭頭更新率
static const unsigned long KART_END_ARROW_HOLD_MS         = 2000;    // 箭頭停最後一張
static const unsigned long KART_END_TRACK_STOP_WAIT_MS    = 2000;    // 軌道停止等待
static const unsigned long KART_LOSERS_EXIT_STEP_MS       = 40;      // 輸家離場速度
static const unsigned long KART_ROTATE_INTERVAL_MS        = 100;     // 旋轉一圈每格速度
static const unsigned long KART_ROTATE_WAIT_MS            = 2000;    // 旋轉完等待
static const unsigned long KART_VICTORY_INTERVAL_MS       = 180;     // 勝利手勢切換速度
static const unsigned long KART_REST_DURATION_MS          = 300000;  // 休息 5 分鐘

static const unsigned long KART_TRANSITION_INTERVAL_MS    = 90;      // 黑白動畫清除的更新間隔

// =====================================================
// 機率參數
// =====================================================
static const int KART_STRAIGHT_CHANCE = 75;

// =====================================================
// 最大偏移量參數
// =====================================================
static const int KART_HILL_MAX_SHIFT_RIGHT  =  2;
static const int KART_HILL_MAX_SHIFT_LEFT   = -2;
static const int KART_TRACK_MAX_SHIFT_RIGHT =  2;
static const int KART_TRACK_MAX_SHIFT_LEFT  = -2;
static const int KART_CAR_MAX_SHIFT_RIGHT   = -2;
static const int KART_CAR_MAX_SHIFT_LEFT    =  2;

// =====================================================
// 車位與排名視覺參數
// 第一名站中間，第二名左側，第三名右側
// y 越小越靠前
// =====================================================
static const int KART_CENTER_Y = 38;
static const int KART_LEFT_Y   = 43;
static const int KART_RIGHT_Y  = 47;
static const int KART_LANE_GAP = 2;      // 中間與左右車位間隔 2px

// =====================================================
// 黑白格過渡動畫格子大小
// =====================================================
static const int KART_TRANSITION_BLOCK_SIZE = 4;

// =====================================================
// 右轉 / 左轉入彎角色 frame 序列
// =====================================================
static const int KART_TURN_IN_SEQ[]  = {1, 2, 3, 4};
static const int KART_TURN_OUT_SEQ[] = {4, 3, 2, 1};

static const int KART_TURN_IN_LEN  = sizeof(KART_TURN_IN_SEQ)  / sizeof(KART_TURN_IN_SEQ[0]);
static const int KART_TURN_OUT_LEN = sizeof(KART_TURN_OUT_SEQ) / sizeof(KART_TURN_OUT_SEQ[0]);

// =====================================================
// KART_TRACK frame 序列
// =====================================================
static const int KART_TRACK_STRAIGHT_SEQ[] = {0, 1, 2, 3};
static const int KART_TRACK_TURN_IN_SEQ[]  = {4, 5, 6, 7};
static const int KART_TRACK_TURN_HOLD_SEQ[] = {8, 9, 10, 11};
static const int KART_TRACK_TURN_OUT_SEQ[] = {7, 6, 5, 4};

static const int KART_TRACK_STRAIGHT_LEN = sizeof(KART_TRACK_STRAIGHT_SEQ) / sizeof(KART_TRACK_STRAIGHT_SEQ[0]);
static const int KART_TRACK_TURN_IN_LEN  = sizeof(KART_TRACK_TURN_IN_SEQ)  / sizeof(KART_TRACK_TURN_IN_SEQ[0]);
static const int KART_TRACK_TURN_HOLD_LEN = sizeof(KART_TRACK_TURN_HOLD_SEQ) / sizeof(KART_TRACK_TURN_HOLD_SEQ[0]);
static const int KART_TRACK_TURN_OUT_LEN = sizeof(KART_TRACK_TURN_OUT_SEQ) / sizeof(KART_TRACK_TURN_OUT_SEQ[0]);

// =====================================================
// 畫寬圖裁切
// =====================================================
static void drawKartCroppedFrame(
  const uint8_t* sheet,
  const uint16_t* palette,
  int sheetW,
  int frameW,
  int frameH,
  int frameIndex,
  int xOnScreen,
  int yOnScreen,
  bool mirrorX = false
) {
  int frameStartX = frameIndex * frameW;

  for (int y = 0; y < frameH; y++) {
    int dy = yOnScreen + y;
    if (dy < 0 || dy >= KART_SCR_H) continue;

    for (int x = 0; x < frameW; x++) {
      int sx = mirrorX ? (frameW - 1 - x) : x;
      int dx = xOnScreen + x;
      if (dx < 0 || dx >= KART_SCR_W) continue;

      uint32_t pos =
        (uint32_t)y * (uint32_t)sheetW +
        (uint32_t)(frameStartX + sx);

      uint8_t colorIndex = pgm_read_byte(&(sheet[pos]));
      uint16_t color = pgm_read_word(&(palette[colorIndex]));

      if (color == KART_TRANSPARENT) continue;

      display.drawPixel(dx, dy, color);
    }
  }
}

// =====================================================
// 畫單一角色 frame
// =====================================================
static void drawKartSpriteFrame(
  const uint8_t* sheet,
  const uint16_t* palette,
  int sheetW,
  int frameW,
  int frameH,
  int frameIndex,
  int x,
  int y,
  bool mirrorX = false
) {
  if (frameIndex < 0) return;

  int frameStartX = frameIndex * frameW;

  for (int j = 0; j < frameH; j++) {
    int dy = y + j;
    if (dy < 0 || dy >= KART_SCR_H) continue;

    for (int i = 0; i < frameW; i++) {
      int sx = mirrorX ? (frameW - 1 - i) : i;
      int dx = x + i;

      if (dx < 0 || dx >= KART_SCR_W) continue;

      uint32_t pos =
        (uint32_t)j * (uint32_t)sheetW +
        (uint32_t)frameStartX +
        (uint32_t)sx;

      uint8_t colorIndex = pgm_read_byte(&(sheet[pos]));
      uint16_t color = pgm_read_word(&(palette[colorIndex]));

      if (color == KART_TRANSPARENT) continue;

      display.drawPixel(dx, dy, color);
    }
  }
}



// =====================================================
// 畫倒數 5 4 3 2 1 GO
// GO 為紅色
// =====================================================
static void drawKartCountdownText() {
  if (kartModePhase != KART_PHASE_COUNTDOWN) return;

  unsigned long elapsed = millis() - kartPhaseStartMs;
  int step = elapsed / KART_COUNTDOWN_INTERVAL_MS;

  display.setTextWrap(false);
  display.setTextSize(2);

  int textY = KART_CENTER_Y - 18;

  if (step <= 4) {
    int number = 5 - step;
    int textX = 28;

    display.setTextColor(0x0000);
    display.setCursor(textX + 1, textY + 1);
    display.print(number);

    display.setTextColor(0xffff);
    display.setCursor(textX, textY);
    display.print(number);
  } else if (step == 5) {
    int textX = 18;

    display.setTextColor(0x0000);
    display.setCursor(textX + 1, textY + 1);
    display.print("GO");

    display.setTextColor(0xf800);
    display.setCursor(textX, textY);
    display.print("GO");
  }
}

// =====================================================
// 根據 step 計算漸進偏移量
// =====================================================
static int calcProgressiveOffset(int maxShift, int stepIndex, int totalSteps) {
  int absMax = maxShift >= 0 ? maxShift : -maxShift;
  int value = ((stepIndex + 1) * absMax) / totalSteps;

  if (value < 1) value = 1;
  if (value > absMax) value = absMax;

  return (maxShift >= 0) ? value : -value;
}

// =====================================================
// 排序出目前名次
// 回傳第一、第二、第三名的 raceCars index
// =====================================================
static void getRaceOrder(int& firstIdx, int& secondIdx, int& thirdIdx) {
  int idx[3] = {0, 1, 2};

  for (int i = 0; i < 2; i++) {
    for (int j = i + 1; j < 3; j++) {
      if (raceCars[idx[j]].progress > raceCars[idx[i]].progress) {
        int t = idx[i];
        idx[i] = idx[j];
        idx[j] = t;
      }
    }
  }

  firstIdx = idx[0];
  secondIdx = idx[1];
  thirdIdx = idx[2];
}

// =====================================================
// 選三位不重複角色
// raceCars[0] 視為初始主角
// =====================================================
static void chooseThreeUniqueDrivers() {
  int used[3];

  used[0] = random(KART_DRIVER_COUNT);

  do {
    used[1] = random(KART_DRIVER_COUNT);
  } while (used[1] == used[0]);

  do {
    used[2] = random(KART_DRIVER_COUNT);
  } while (used[2] == used[0] || used[2] == used[1]);

  for (int i = 0; i < 3; i++) {
    raceCars[i].driverIndex = used[i];
    raceCars[i].driver = kartDrivers[used[i]];
  }
}

// =====================================================
// 依目前名次設定目標車位
// 第一名中間，第二名左，第三名右
// =====================================================
static void computeRaceTargetSlots(int firstIdx, int secondIdx, int thirdIdx) {
  RaceCarState& first  = raceCars[firstIdx];
  RaceCarState& second = raceCars[secondIdx];
  RaceCarState& third  = raceCars[thirdIdx];

  int centerX = (KART_SCR_W - first.driver->frameW) / 2;
  int leftX   = centerX - second.driver->frameW - KART_LANE_GAP;
  int rightX  = centerX + first.driver->frameW + KART_LANE_GAP;

  if (leftX < 0) leftX = 0;
  if (rightX > (KART_SCR_W - third.driver->frameW)) {
    rightX = KART_SCR_W - third.driver->frameW;
  }

  first.targetX = centerX;
  first.targetY = KART_CENTER_Y + first.bobOffset;

  second.targetX = leftX;
  second.targetY = KART_LEFT_Y + second.bobOffset;

  third.targetX = rightX;
  third.targetY = KART_RIGHT_Y + third.bobOffset;
}

// =====================================================
// 讓三台車往 target 平滑移動
// =====================================================
static void moveRaceCarsTowardTarget() {
  for (int i = 0; i < 3; i++) {
    if (raceCars[i].x < raceCars[i].targetX) raceCars[i].x++;
    if (raceCars[i].x > raceCars[i].targetX) raceCars[i].x--;

    if (raceCars[i].y < raceCars[i].targetY) raceCars[i].y++;
    if (raceCars[i].y > raceCars[i].targetY) raceCars[i].y--;
  }
}

// =====================================================
// 初始化三台車
// 重點：先排好位子，再一起由下往上進場
// =====================================================
static void initRaceCarsForNewRound() {
  chooseThreeUniqueDrivers();

  int centerX = (KART_SCR_W - raceCars[0].driver->frameW) / 2;
  int leftX   = centerX - raceCars[1].driver->frameW - KART_LANE_GAP;
  int rightX  = centerX + raceCars[0].driver->frameW + KART_LANE_GAP;

  if (leftX < 0) leftX = 0;
  if (rightX > (KART_SCR_W - raceCars[2].driver->frameW)) {
    rightX = KART_SCR_W - raceCars[2].driver->frameW;
  }

  for (int i = 0; i < 3; i++) {
    raceCars[i].progress = 0;
    raceCars[i].baseSpeed = random(4, 8);
    raceCars[i].boostSpeed = 0;
    raceCars[i].boostEndMs = 0;
    raceCars[i].nextBoostMs = millis() + random(2000UL, 6000UL);

    raceCars[i].bobOffset = 0;
    raceCars[i].bobTarget = 0;
    raceCars[i].nextBobMs = millis() + random(500UL, 1500UL);

    raceCars[i].frameIndex = 0;
    raceCars[i].mirrorX = false;

    raceCars[i].delayedMotionStartMs = 0;
    raceCars[i].pendingFrameIndex = 0;
    raceCars[i].pendingMirrorX = false;
  }

  // 先排好賽道位置，再一起由下往上進場
  raceCars[0].x = centerX;
  raceCars[0].y = KART_SCR_H + 8;
  raceCars[0].targetX = centerX;
  raceCars[0].targetY = KART_CENTER_Y;

  raceCars[1].x = leftX;
  raceCars[1].y = KART_SCR_H + 8;
  raceCars[1].targetX = leftX;
  raceCars[1].targetY = KART_LEFT_Y;

  raceCars[2].x = rightX;
  raceCars[2].y = KART_SCR_H + 8;
  raceCars[2].targetX = rightX;
  raceCars[2].targetY = KART_RIGHT_Y;

  currentWinnerDriver = raceCars[0].driver;
  currentWinnerDriverIndex = raceCars[0].driverIndex;
}

// =====================================================
// 套用優勝者直線畫面
// =====================================================
static void applyWinnerStraightVisual() {
  kartWinnerFrame = 0;
  kartWinnerMirrorX = false;

  kartHillOffsetX = 0;
  kartTrackOffsetX = 0;
  kartWinnerOffsetX = 0;

  kartHillX = kartHillBaseX + kartHillOffsetX;
  kartTrackX = kartTrackBaseX + kartTrackOffsetX;
  kartWinnerX = kartWinnerBaseX + kartWinnerOffsetX;
  kartWinnerY = kartWinnerBaseY;
}

// =====================================================
// 套用優勝者右轉畫面
// =====================================================
static void applyWinnerRightTurnVisual(int frameIndex, bool progressive, int stepIndex) {
  kartWinnerFrame = frameIndex;
  kartWinnerMirrorX = false;

  if (progressive) {
    kartHillOffsetX   = calcProgressiveOffset(KART_HILL_MAX_SHIFT_RIGHT,  stepIndex, KART_TURN_IN_LEN);
    kartTrackOffsetX  = calcProgressiveOffset(KART_TRACK_MAX_SHIFT_RIGHT, stepIndex, KART_TURN_IN_LEN);
    kartWinnerOffsetX = calcProgressiveOffset(KART_CAR_MAX_SHIFT_RIGHT,   stepIndex, KART_TURN_IN_LEN);
  } else {
    kartHillOffsetX   = KART_HILL_MAX_SHIFT_RIGHT;
    kartTrackOffsetX  = KART_TRACK_MAX_SHIFT_RIGHT;
    kartWinnerOffsetX = KART_CAR_MAX_SHIFT_RIGHT;
  }

  kartHillX = kartHillBaseX + kartHillOffsetX;
  kartTrackX = kartTrackBaseX + kartTrackOffsetX;
  kartWinnerX = kartWinnerBaseX + kartWinnerOffsetX;
  kartWinnerY = kartWinnerBaseY;
}

// =====================================================
// 套用優勝者左轉畫面
// =====================================================
static void applyWinnerLeftTurnVisual(int frameIndex, bool progressive, int stepIndex) {
  kartWinnerFrame = frameIndex;
  kartWinnerMirrorX = true;

  if (progressive) {
    kartHillOffsetX   = calcProgressiveOffset(KART_HILL_MAX_SHIFT_LEFT,  stepIndex, KART_TURN_IN_LEN);
    kartTrackOffsetX  = calcProgressiveOffset(KART_TRACK_MAX_SHIFT_LEFT, stepIndex, KART_TURN_IN_LEN);
    kartWinnerOffsetX = calcProgressiveOffset(KART_CAR_MAX_SHIFT_LEFT,   stepIndex, KART_TURN_IN_LEN);
  } else {
    kartHillOffsetX   = KART_HILL_MAX_SHIFT_LEFT;
    kartTrackOffsetX  = KART_TRACK_MAX_SHIFT_LEFT;
    kartWinnerOffsetX = KART_CAR_MAX_SHIFT_LEFT;
  }

  kartHillX = kartHillBaseX + kartHillOffsetX;
  kartTrackX = kartTrackBaseX + kartTrackOffsetX;
  kartWinnerX = kartWinnerBaseX + kartWinnerOffsetX;
  kartWinnerY = kartWinnerBaseY;
}

// =====================================================
// 設定 KART_TRACK 動畫
// =====================================================
static void startKartTrackStraightLoop() {
  kartTrackAnimMode = KART_TRACK_STRAIGHT_LOOP;
  kartTrackAnimStep = 0;
  kartTrackFrameIndex = KART_TRACK_STRAIGHT_SEQ[0];
  kartTrackMirrorX = false;
}

static void startKartTrackTurnIn(bool mirrorX) {
  kartTrackAnimMode = KART_TRACK_TURN_IN;
  kartTrackAnimStep = 0;
  kartTrackFrameIndex = KART_TRACK_TURN_IN_SEQ[0];
  kartTrackMirrorX = mirrorX;
}

static void startKartTrackTurnHold(bool mirrorX) {
  kartTrackAnimMode = KART_TRACK_TURN_HOLD;
  kartTrackAnimStep = 0;
  kartTrackFrameIndex = KART_TRACK_TURN_HOLD_SEQ[0];
  kartTrackMirrorX = mirrorX;
}

static void startKartTrackTurnOut(bool mirrorX) {
  kartTrackAnimMode = KART_TRACK_TURN_OUT;
  kartTrackAnimStep = 0;
  kartTrackFrameIndex = KART_TRACK_TURN_OUT_SEQ[0];
  kartTrackMirrorX = mirrorX;
}

// =====================================================
// 進入直線狀態
// =====================================================
static void enterStraightState(unsigned long now) {
  kartHeroState = KART_HERO_STRAIGHT;
  kartTurnStep = 0;
  startKartTrackStraightLoop();
  applyWinnerStraightVisual();

  kartNextStateChangeMs = now + random(KART_STRAIGHT_MIN_MS, KART_STRAIGHT_MAX_MS);
}

// =====================================================
// 進入右轉入彎
// =====================================================
static void enterRightTurnInState(unsigned long now) {
  kartHeroState = KART_HERO_TURN_RIGHT_IN;
  kartTurnStep = 0;
  kartLastHeroAnimMs = now;
  startKartTrackTurnIn(false);
  applyWinnerRightTurnVisual(KART_TURN_IN_SEQ[kartTurnStep], true, kartTurnStep);
}

// =====================================================
// 進入左轉入彎
// =====================================================
static void enterLeftTurnInState(unsigned long now) {
  kartHeroState = KART_HERO_TURN_LEFT_IN;
  kartTurnStep = 0;
  kartLastHeroAnimMs = now;
  startKartTrackTurnIn(true);
  applyWinnerLeftTurnVisual(KART_TURN_IN_SEQ[kartTurnStep], true, kartTurnStep);
}

// =====================================================
// 隨機決定下一個大動作
// 這個只用在比賽中「目前領先者」的表演
// =====================================================
static void chooseNextMainAction(unsigned long now) {
  int r = random(100);

  if (r < KART_STRAIGHT_CHANCE) {
    enterStraightState(now);
  } else if (r < (KART_STRAIGHT_CHANCE + (100 - KART_STRAIGHT_CHANCE) / 2)) {
    enterRightTurnInState(now);
  } else {
    enterLeftTurnInState(now);
  }
}

// =====================================================
// 更新中間領先者的動作
// =====================================================
static void updateLeaderShowMotion(unsigned long now) {
  switch (kartHeroState) {

    case KART_HERO_STRAIGHT: {
      applyWinnerStraightVisual();

      if (now >= kartNextStateChangeMs) {
        chooseNextMainAction(now);
      }
      break;
    }

    case KART_HERO_TURN_RIGHT_IN: {
      if (now - kartLastHeroAnimMs >= KART_HERO_STEP_INTERVAL_MS) {
        kartLastHeroAnimMs = now;
        kartTurnStep++;

        if (kartTurnStep >= KART_TURN_IN_LEN) {
          kartHeroState = KART_HERO_TURN_RIGHT_HOLD;
          kartTurnStep = KART_TURN_IN_LEN - 1;
          startKartTrackTurnHold(false);
          applyWinnerRightTurnVisual(4, false, kartTurnStep);
          kartNextStateChangeMs = now + random(KART_TURN_HOLD_MIN_MS, KART_TURN_HOLD_MAX_MS);
        } else {
          applyWinnerRightTurnVisual(KART_TURN_IN_SEQ[kartTurnStep], true, kartTurnStep);
        }
      }
      break;
    }

    case KART_HERO_TURN_RIGHT_HOLD: {
      applyWinnerRightTurnVisual(4, false, kartTurnStep);

      if (now >= kartNextStateChangeMs) {
        kartHeroState = KART_HERO_TURN_RIGHT_OUT;
        kartTurnStep = 0;
        kartLastHeroAnimMs = now;
        startKartTrackTurnOut(false);
        applyWinnerRightTurnVisual(KART_TURN_OUT_SEQ[kartTurnStep], false, kartTurnStep);
      }
      break;
    }

    case KART_HERO_TURN_RIGHT_OUT: {
      if (now - kartLastHeroAnimMs >= KART_HERO_STEP_INTERVAL_MS) {
        kartLastHeroAnimMs = now;
        kartTurnStep++;

        if (kartTurnStep >= KART_TURN_OUT_LEN) {
          enterStraightState(now);
        } else {
          int reverseStep = (KART_TURN_OUT_LEN - 1) - kartTurnStep;
          applyWinnerRightTurnVisual(KART_TURN_OUT_SEQ[kartTurnStep], true, reverseStep);
        }
      }
      break;
    }

    case KART_HERO_TURN_LEFT_IN: {
      if (now - kartLastHeroAnimMs >= KART_HERO_STEP_INTERVAL_MS) {
        kartLastHeroAnimMs = now;
        kartTurnStep++;

        if (kartTurnStep >= KART_TURN_IN_LEN) {
          kartHeroState = KART_HERO_TURN_LEFT_HOLD;
          kartTurnStep = KART_TURN_IN_LEN - 1;
          startKartTrackTurnHold(true);
          applyWinnerLeftTurnVisual(4, false, kartTurnStep);
          kartNextStateChangeMs = now + random(KART_TURN_HOLD_MIN_MS, KART_TURN_HOLD_MAX_MS);
        } else {
          applyWinnerLeftTurnVisual(KART_TURN_IN_SEQ[kartTurnStep], true, kartTurnStep);
        }
      }
      break;
    }

    case KART_HERO_TURN_LEFT_HOLD: {
      applyWinnerLeftTurnVisual(4, false, kartTurnStep);

      if (now >= kartNextStateChangeMs) {
        kartHeroState = KART_HERO_TURN_LEFT_OUT;
        kartTurnStep = 0;
        kartLastHeroAnimMs = now;
        startKartTrackTurnOut(true);
        applyWinnerLeftTurnVisual(KART_TURN_OUT_SEQ[kartTurnStep], false, kartTurnStep);
      }
      break;
    }

    case KART_HERO_TURN_LEFT_OUT: {
      if (now - kartLastHeroAnimMs >= KART_HERO_STEP_INTERVAL_MS) {
        kartLastHeroAnimMs = now;
        kartTurnStep++;

        if (kartTurnStep >= KART_TURN_OUT_LEN) {
          enterStraightState(now);
        } else {
          int reverseStep = (KART_TURN_OUT_LEN - 1) - kartTurnStep;
          applyWinnerLeftTurnVisual(KART_TURN_OUT_SEQ[kartTurnStep], true, reverseStep);
        }
      }
      break;
    }

    default:
      break;
  }
}

// =====================================================
// 設定目前優勝者（或目前領先者）
// 中間位置就是給第一名表演的
// =====================================================
static void setCurrentWinnerByRaceIndex(int raceIdx) {
  currentWinnerDriver = raceCars[raceIdx].driver;
  currentWinnerDriverIndex = raceCars[raceIdx].driverIndex;

  kartWinnerBaseX = (KART_SCR_W - currentWinnerDriver->frameW) / 2;
  kartWinnerX = kartWinnerBaseX;
  kartWinnerY = kartWinnerBaseY;
}

// =====================================================
// 更新中段比賽
// 1. 三台車各自有基本速度
// 2. 不定時衝刺，讓名次互換
// 3. 每台車有起伏 bob，讓畫面更像追逐
// 4. 第一名站中間，另外兩台左右跟隨
// 5. 三台都會做同樣轉彎，另外兩台延遲 1 秒
// 6. 增加落後補償，避免後半段名次完全鎖死
// =====================================================
static void updateRaceRunning(unsigned long now) {
  if (now - kartLastRaceUpdateMs < KART_RACE_UPDATE_MS) return;
  kartLastRaceUpdateMs = now;

  int firstIdx, secondIdx, thirdIdx;
  getRaceOrder(firstIdx, secondIdx, thirdIdx);
  int leaderProgress = raceCars[firstIdx].progress;

  for (int i = 0; i < 3; i++) {
    if (now >= raceCars[i].nextBobMs) {
      raceCars[i].nextBobMs = now + random(500UL, 1300UL);
      raceCars[i].bobTarget = random(-4, 6);
    }

    if (raceCars[i].bobOffset < raceCars[i].bobTarget) raceCars[i].bobOffset++;
    if (raceCars[i].bobOffset > raceCars[i].bobTarget) raceCars[i].bobOffset--;

    if (now >= raceCars[i].nextBoostMs) {
      raceCars[i].nextBoostMs = now + random(2500UL, 7000UL);
      raceCars[i].boostEndMs = now + random(700UL, 1800UL);
      raceCars[i].boostSpeed = random(2, 6);
    }

    if (now >= raceCars[i].boostEndMs) {
      raceCars[i].boostSpeed = 0;
    }

    int gapToLeader = leaderProgress - raceCars[i].progress;
    int catchUpBonus = gapToLeader / 20;

    if (catchUpBonus > 4) catchUpBonus = 4;
    if (catchUpBonus < 0) catchUpBonus = 0;

    raceCars[i].progress += raceCars[i].baseSpeed + raceCars[i].boostSpeed + catchUpBonus + random(0, 2);
 
  
  
  }

  getRaceOrder(firstIdx, secondIdx, thirdIdx);
  computeRaceTargetSlots(firstIdx, secondIdx, thirdIdx);

  setCurrentWinnerByRaceIndex(firstIdx);
  updateLeaderShowMotion(now);

  // 第一名立即套用
  raceCars[firstIdx].frameIndex = kartWinnerFrame;
  raceCars[firstIdx].mirrorX = kartWinnerMirrorX;

  // 第二名記錄待套用動作
  if (raceCars[secondIdx].pendingFrameIndex != kartWinnerFrame ||
      raceCars[secondIdx].pendingMirrorX != kartWinnerMirrorX) {
    raceCars[secondIdx].pendingFrameIndex = kartWinnerFrame;
    raceCars[secondIdx].pendingMirrorX = kartWinnerMirrorX;
    raceCars[secondIdx].delayedMotionStartMs = now;
  }

  // 第三名記錄待套用動作
  if (raceCars[thirdIdx].pendingFrameIndex != kartWinnerFrame ||
      raceCars[thirdIdx].pendingMirrorX != kartWinnerMirrorX) {
    raceCars[thirdIdx].pendingFrameIndex = kartWinnerFrame;
    raceCars[thirdIdx].pendingMirrorX = kartWinnerMirrorX;
    raceCars[thirdIdx].delayedMotionStartMs = now;
  }

  // 延遲 0.4 秒後跟上
  if (now - raceCars[secondIdx].delayedMotionStartMs >= 400) {
    raceCars[secondIdx].frameIndex = raceCars[secondIdx].pendingFrameIndex;
    raceCars[secondIdx].mirrorX = raceCars[secondIdx].pendingMirrorX;
  }

  if (now - raceCars[thirdIdx].delayedMotionStartMs >= 800) {
    raceCars[thirdIdx].frameIndex = raceCars[thirdIdx].pendingFrameIndex;
    raceCars[thirdIdx].mirrorX = raceCars[thirdIdx].pendingMirrorX;
  }

  moveRaceCarsTowardTarget();
}

// =====================================================
// KART_TRACK 動畫
// 直線：0~3 循環
// 入彎：4~7 一次
// 持續轉彎：8~11 循環
// 出彎回直線：7~4 一次，完成後由角色狀態切回直線 0~3
// =====================================================
static void updateKartTrackAnim(unsigned long now) {
  if (now - kartLastTrackAnimMs < KART_TRACK_ANIM_INTERVAL_MS) return;
  kartLastTrackAnimMs = now;

  switch (kartTrackAnimMode) {

    case KART_TRACK_STRAIGHT_LOOP: {
      kartTrackAnimStep++;
      if (kartTrackAnimStep >= KART_TRACK_STRAIGHT_LEN) {
        kartTrackAnimStep = 0;
      }
      kartTrackFrameIndex = KART_TRACK_STRAIGHT_SEQ[kartTrackAnimStep];
      kartTrackMirrorX = false;
      break;
    }

    case KART_TRACK_TURN_IN: {
      if (kartTrackAnimStep < KART_TRACK_TURN_IN_LEN - 1) {
        kartTrackAnimStep++;
      }
      kartTrackFrameIndex = KART_TRACK_TURN_IN_SEQ[kartTrackAnimStep];
      break;
    }

    case KART_TRACK_TURN_HOLD: {
      kartTrackAnimStep++;
      if (kartTrackAnimStep >= KART_TRACK_TURN_HOLD_LEN) {
        kartTrackAnimStep = 0;
      }
      kartTrackFrameIndex = KART_TRACK_TURN_HOLD_SEQ[kartTrackAnimStep];
      break;
    }

    case KART_TRACK_TURN_OUT: {
      if (kartTrackAnimStep < KART_TRACK_TURN_OUT_LEN - 1) {
        kartTrackAnimStep++;
      }
      kartTrackFrameIndex = KART_TRACK_TURN_OUT_SEQ[kartTrackAnimStep];
      break;
    }

    default:
      break;
  }
}

// =====================================================
// 箭頭期間讓兩位輸家往下退場
// =====================================================
static void updateKartLosersExitDuringArrow(unsigned long now) {
  if (now - kartLastEndAnimMs < KART_LOSERS_EXIT_STEP_MS) return;
  kartLastEndAnimMs = now;

  if (kartLoserAIdx >= 0 && raceCars[kartLoserAIdx].y < KART_SCR_H + 20) {
    raceCars[kartLoserAIdx].y += 2;
  }

  if (kartLoserBIdx >= 0 && raceCars[kartLoserBIdx].y < KART_SCR_H + 20) {
    raceCars[kartLoserBIdx].y += 2;
  }
}

// =====================================================
// 開始新一輪
// =====================================================
static void startKartNewRound(unsigned long now) {
  initRaceCarsForNewRound();

  kartModePhase = KART_PHASE_ENTER_GRID;
  kartPhaseStartMs = now;
  kartLastRaceUpdateMs = now;
  kartLastTrackAnimMs = now;
  kartLastHeroAnimMs = now;
  kartLastEndAnimMs = now;
  kartLastArrowAnimMs = now;

  kartTrackFrameIndex = 0;
  kartTrackMirrorX = false;
  kartTrackAnimMode = KART_TRACK_STRAIGHT_LOOP;
  kartTrackAnimStep = 0;
  kartEndArrowFrame = 0;
  kartRotateFrame = 0;
  kartVictoryFrame = 10;
  kartVictoryLoopCount = 0;
  kartTransitionColumn = 0;

  kartLoserAIdx = -1;
  kartLoserBIdx = -1;

  setCurrentWinnerByRaceIndex(0);
  enterStraightState(now);
}

// =====================================================
// 進入定位後等待階段
// =====================================================
static void startKartGridHoldPhase(unsigned long now) {
  kartModePhase = KART_PHASE_GRID_HOLD;
  kartPhaseStartMs = now;
}

// =====================================================
// 進入倒數階段
// =====================================================
static void startKartCountdownPhase(unsigned long now) {
  kartModePhase = KART_PHASE_COUNTDOWN;
  kartPhaseStartMs = now;
}

// =====================================================
// 進入正式比賽
// =====================================================
static void startKartRunningPhase(unsigned long now) {
  kartModePhase = KART_PHASE_RUNNING;
  kartPhaseStartMs = now;
  kartLastRaceUpdateMs = now;
  kartLastTrackAnimMs = now;
  kartLastHeroAnimMs = now;
  enterStraightState(now);
}

// =====================================================
// 進入終點箭頭播放
// 並鎖定真正優勝者與兩位輸家
// =====================================================
static void startKartEndArrowPlayPhase(unsigned long now) {
  int firstIdx, secondIdx, thirdIdx;
  getRaceOrder(firstIdx, secondIdx, thirdIdx);

  setCurrentWinnerByRaceIndex(firstIdx);
  kartLoserAIdx = secondIdx;
  kartLoserBIdx = thirdIdx;

  // 鎖定終點前站位：優勝者固定中間，輸家保留當下位置
  raceCars[firstIdx].x = (KART_SCR_W - raceCars[firstIdx].driver->frameW) / 2;
  raceCars[firstIdx].y = KART_CENTER_Y;
  raceCars[firstIdx].targetX = raceCars[firstIdx].x;
  raceCars[firstIdx].targetY = raceCars[firstIdx].y;
  raceCars[firstIdx].frameIndex = 0;
  raceCars[firstIdx].mirrorX = false;

  raceCars[secondIdx].targetX = raceCars[secondIdx].x;
  raceCars[secondIdx].targetY = raceCars[secondIdx].y;
  raceCars[secondIdx].frameIndex = 0;
  raceCars[secondIdx].mirrorX = false;

  raceCars[thirdIdx].targetX = raceCars[thirdIdx].x;
  raceCars[thirdIdx].targetY = raceCars[thirdIdx].y;
  raceCars[thirdIdx].frameIndex = 0;
  raceCars[thirdIdx].mirrorX = false;

  kartModePhase = KART_PHASE_END_ARROW_PLAY;
  kartPhaseStartMs = now;
  kartLastEndAnimMs = now;
  kartLastArrowAnimMs = now;
  kartEndArrowFrame = 0;

  startKartTrackStraightLoop();
  applyWinnerStraightVisual();
}

// =====================================================
// 箭頭停留最後一張
// =====================================================
static void startKartEndArrowHoldPhase(unsigned long now) {
  kartModePhase = KART_PHASE_END_ARROW_HOLD;
  kartPhaseStartMs = now;
  kartEndArrowFrame = KART_END_FRAMES - 1;
  applyWinnerStraightVisual();
}

// =====================================================
// 軌道停止等待
// =====================================================
static void startKartEndTrackStopPhase(unsigned long now) {
  kartModePhase = KART_PHASE_END_TRACK_STOP;
  kartPhaseStartMs = now;
  applyWinnerStraightVisual();
}

// =====================================================
// 旋轉一圈
// =====================================================
static void startKartRotatePhase(unsigned long now) {
  kartModePhase = KART_PHASE_END_ROTATE;
  kartPhaseStartMs = now;
  kartLastEndAnimMs = now;
  kartRotateFrame = 0;
  applyWinnerStraightVisual();
  kartWinnerFrame = kartRotateFrame;
}

// =====================================================
// 旋轉後等待
// =====================================================
static void startKartRotateWaitPhase(unsigned long now) {
  kartModePhase = KART_PHASE_END_ROTATE_WAIT;
  kartPhaseStartMs = now;
  kartWinnerFrame = 10;
  kartWinnerMirrorX = false;
}

// =====================================================
// 勝利手勢
// =====================================================
static void startKartVictoryPhase(unsigned long now) {
  kartModePhase = KART_PHASE_END_VICTORY;
  kartPhaseStartMs = now;
  kartLastEndAnimMs = now;
  kartVictoryFrame = 10;
  kartVictoryLoopCount = 0;
  kartWinnerFrame = 10;
  kartWinnerMirrorX = false;
}

// =====================================================
// 休息
// =====================================================
static void startKartRestPhase(unsigned long now) {
  kartModePhase = KART_PHASE_REST;
  kartPhaseStartMs = now;
  applyWinnerStraightVisual();
  kartWinnerFrame = 10;
}

// =====================================================
// 黑白格清除過渡動畫
// =====================================================
static void startKartTransitionPhase(unsigned long now) {
  kartModePhase = KART_PHASE_TRANSITION;
  kartPhaseStartMs = now;
  kartLastTransitionMs = now;
  kartTransitionColumn = 0;
  applyWinnerStraightVisual();
  kartWinnerFrame = 10;
}

// =====================================================
// 更新整體流程
// =====================================================
static void updateKartModePhase(unsigned long now) {
  switch (kartModePhase) {

    case KART_PHASE_ENTER_GRID: {
      bool allArrived = true;

      for (int i = 0; i < 3; i++) {
        if (raceCars[i].y > raceCars[i].targetY) {
          allArrived = false;

          if (now - kartLastEndAnimMs >= KART_ENTER_STEP_MS) {
            raceCars[i].y -= 1;
          }

          if (raceCars[i].y < raceCars[i].targetY) {
            raceCars[i].y = raceCars[i].targetY;
          }
        }
      }

      if (now - kartLastEndAnimMs >= KART_ENTER_STEP_MS) {
        kartLastEndAnimMs = now;
      }

      if (allArrived) {
        startKartGridHoldPhase(now);
      }
      break;
    }

    case KART_PHASE_GRID_HOLD: {
      if (now - kartPhaseStartMs >= KART_GRID_HOLD_MS) {
        startKartCountdownPhase(now);
      }
      break;
    }

    case KART_PHASE_COUNTDOWN: {
      kartTrackFrameIndex = 0;
      kartTrackMirrorX = false;

      if (now - kartPhaseStartMs >= KART_COUNTDOWN_INTERVAL_MS * 6) {
        startKartRunningPhase(now);
      }
      break;
    }

    case KART_PHASE_RUNNING: {
      updateKartTrackAnim(now);
      updateRaceRunning(now);

      if (now - kartPhaseStartMs >= KART_RUN_DURATION_MS) {
        startKartEndArrowPlayPhase(now);
      }
      break;
    }

    case KART_PHASE_END_ARROW_PLAY: {
      updateKartTrackAnim(now);
      applyWinnerStraightVisual();

      // 箭頭播放期間，輸家開始退場
      updateKartLosersExitDuringArrow(now);

      if (now - kartLastArrowAnimMs >= KART_END_ARROW_INTERVAL_MS) {
        kartLastArrowAnimMs = now;
        kartEndArrowFrame++;

        if (kartEndArrowFrame >= KART_END_FRAMES) {
          startKartEndArrowHoldPhase(now);
        }
      }
      break;
    }

    case KART_PHASE_END_ARROW_HOLD: {
      updateKartTrackAnim(now);
      applyWinnerStraightVisual();
      kartWinnerFrame = 0;

      // 箭頭停最後一張時，輸家繼續退場
      updateKartLosersExitDuringArrow(now);

      if (now - kartPhaseStartMs >= KART_END_ARROW_HOLD_MS) {
        startKartEndTrackStopPhase(now);
      }
      break;
    }

    case KART_PHASE_END_TRACK_STOP: {
      applyWinnerStraightVisual();
      kartWinnerFrame = 0;

      if (now - kartPhaseStartMs >= KART_END_TRACK_STOP_WAIT_MS) {
        startKartRotatePhase(now);
      }
      break;
    }

    case KART_PHASE_END_ROTATE: {
      applyWinnerStraightVisual();
      kartWinnerFrame = kartRotateFrame;
      kartWinnerMirrorX = false;

      if (now - kartLastEndAnimMs >= KART_ROTATE_INTERVAL_MS) {
        kartLastEndAnimMs = now;
        kartRotateFrame++;

        if (kartRotateFrame > 10) {
          startKartRotateWaitPhase(now);
        } else {
          kartWinnerFrame = kartRotateFrame;
        }
      }
      break;
    }

    case KART_PHASE_END_ROTATE_WAIT: {
      applyWinnerStraightVisual();
      kartWinnerFrame = 10;
      kartWinnerMirrorX = false;

      if (now - kartPhaseStartMs >= KART_ROTATE_WAIT_MS) {
        startKartVictoryPhase(now);
      }
      break;
    }

    case KART_PHASE_END_VICTORY: {
      applyWinnerStraightVisual();
      kartWinnerFrame = kartVictoryFrame;
      kartWinnerMirrorX = false;

      if (now - kartLastEndAnimMs >= KART_VICTORY_INTERVAL_MS) {
        kartLastEndAnimMs = now;

        if (kartVictoryFrame == 10) {
          kartVictoryFrame = 11;
        } else {
          kartVictoryFrame = 10;
        }

        kartWinnerFrame = kartVictoryFrame;
        kartVictoryLoopCount++;

        if (kartVictoryLoopCount >= 12) {
          startKartRestPhase(now);
        }
      }
      break;
    }

    case KART_PHASE_REST: {
      applyWinnerStraightVisual();
      kartWinnerFrame = 10;
      kartWinnerMirrorX = false;

      if (now - kartPhaseStartMs >= KART_REST_DURATION_MS) {
        startKartTransitionPhase(now);
      }
      break;
    }

    case KART_PHASE_TRANSITION: {
      applyWinnerStraightVisual();
      kartWinnerFrame = 10;
      kartWinnerMirrorX = false;

      if (now - kartLastTransitionMs >= KART_TRANSITION_INTERVAL_MS) {
        kartLastTransitionMs = now;
        kartTransitionColumn += KART_TRANSITION_BLOCK_SIZE;

        if (kartTransitionColumn > KART_SCR_W + KART_TRANSITION_BLOCK_SIZE) {
          startKartNewRound(now);
        }
      }
      break;
    }

    default:
      break;
  }
}

// =====================================================
// 畫遠景
// =====================================================
static void drawKartHill() {
  drawKartCroppedFrame(
    KART_HILL,
    KART_HILL_PALETTE,
    HILL_SHEET_W,
    HILL_FRAME_W,
    HILL_FRAME_H,
    0,
    kartHillX,
    kartHillY
  );
}

// =====================================================
// 畫軌道
// =====================================================
static void drawKartTrack() {
  drawKartCroppedFrame(
    KART_TRACK,
    KART_TRACK_PALETTE,
    TRACK_SHEET_W,
    TRACK_FRAME_W,
    TRACK_FRAME_H,
    kartTrackFrameIndex,
    kartTrackX,
    kartTrackY,
    kartTrackMirrorX
  );
}

// =====================================================
// 畫三位選手
// 比賽前、比賽中、箭頭期間都使用
// 遮罩順序依腳底位置決定，避免名次高的人硬蓋住其他人
// =====================================================
static void drawRaceCars() {
  int drawOrder[3] = {0, 1, 2};

  for (int i = 0; i < 2; i++) {
    for (int j = i + 1; j < 3; j++) {
      int footA = raceCars[drawOrder[i]].y + raceCars[drawOrder[i]].driver->frameH;
      int footB = raceCars[drawOrder[j]].y + raceCars[drawOrder[j]].driver->frameH;

      if (footA > footB) {
        int t = drawOrder[i];
        drawOrder[i] = drawOrder[j];
        drawOrder[j] = t;
      }
    }
  }

  for (int k = 0; k < 3; k++) {
    int idx = drawOrder[k];

drawKartSpriteFrame(
  raceCars[idx].driver->sheet,
  raceCars[idx].driver->palette,
  raceCars[idx].driver->sheetW,
  raceCars[idx].driver->frameW,
  raceCars[idx].driver->frameH,
  raceCars[idx].frameIndex,
  raceCars[idx].x,
  raceCars[idx].y,
  raceCars[idx].mirrorX
);
  }
}

// =====================================================
// 畫優勝者
// 終點後的單人表演使用
// =====================================================
static void drawWinnerDriver() {
  drawKartSpriteFrame(
    currentWinnerDriver->sheet,
    currentWinnerDriver->palette,
    currentWinnerDriver->sheetW,
    currentWinnerDriver->frameW,
    currentWinnerDriver->frameH,
    kartWinnerFrame,
    kartWinnerX,
    kartWinnerY,
    kartWinnerMirrorX
  );
}

// =====================================================
// 畫終點箭頭
// 左邊 (0,26)
// 右邊 (48,26)
// 箭頭在終點之後到清除動畫結束前都保留
// =====================================================
static void drawKartEndArrow() {
  bool keepArrowVisible =
    (kartModePhase == KART_PHASE_END_ARROW_PLAY) ||
    (kartModePhase == KART_PHASE_END_ARROW_HOLD) ||
    (kartModePhase == KART_PHASE_END_TRACK_STOP) ||
    (kartModePhase == KART_PHASE_END_ROTATE) ||
    (kartModePhase == KART_PHASE_END_ROTATE_WAIT) ||
    (kartModePhase == KART_PHASE_END_VICTORY) ||
    (kartModePhase == KART_PHASE_REST) ||
    (kartModePhase == KART_PHASE_TRANSITION);

  if (!keepArrowVisible) return;

  int frameToDraw = kartEndArrowFrame;
  if (frameToDraw < 0) frameToDraw = 0;
  if (frameToDraw >= KART_END_FRAMES) frameToDraw = KART_END_FRAMES - 1;

drawKartSpriteFrame(
  KART_END,
  KART_END_PALETTE,
  KART_END_SHEET_W,
  KART_END_FRAME_W,
  KART_END_FRAME_H,
  frameToDraw,
  0,
  26,
  false
);

drawKartSpriteFrame(
  KART_END,
  KART_END_PALETTE,
  KART_END_SHEET_W,
  KART_END_FRAME_W,
  KART_END_FRAME_H,
  frameToDraw,
  48,
  26,
  true
);
}

// =====================================================
// 畫黑白格清除動畫
// 由左向右清掉畫面
// =====================================================
static void drawKartTransitionOverlay() {
  if (kartModePhase != KART_PHASE_TRANSITION) return;

  for (int x = 0; x < kartTransitionColumn; x += KART_TRANSITION_BLOCK_SIZE) {
    for (int y = 0; y < KART_SCR_H; y += KART_TRANSITION_BLOCK_SIZE) {
      bool whiteBlock = (((x / KART_TRANSITION_BLOCK_SIZE) + (y / KART_TRANSITION_BLOCK_SIZE)) % 2) == 0;
      uint16_t color = whiteBlock ? 0xffff : 0x0000;
      display.fillRect(x, y, KART_TRANSITION_BLOCK_SIZE, KART_TRANSITION_BLOCK_SIZE, color);
    }
  }
}

// =====================================================
// 重繪畫面
// =====================================================
static void renderKartScene() {
  //display.fillScreen(0x1c27);
    display.fillScreen(0x000E);

  drawKartHill();
  drawKartTrack();

  // 比賽前、比賽中、箭頭期間都畫三台車
  if (kartModePhase == KART_PHASE_ENTER_GRID ||
      kartModePhase == KART_PHASE_GRID_HOLD ||
      kartModePhase == KART_PHASE_COUNTDOWN ||
      kartModePhase == KART_PHASE_RUNNING ||
      kartModePhase == KART_PHASE_END_ARROW_PLAY ||
      kartModePhase == KART_PHASE_END_ARROW_HOLD) {
    drawRaceCars();
  } else {
    drawWinnerDriver();
  }

  drawKartEndArrow();
  drawKartCountdownText();
  drawThemeClockText();
  drawKartTransitionOverlay();
}

// =====================================================
// 主函式
// =====================================================
void KartMode() {
  if (ModefirstRun) {
    randomSeed(millis());

    kartHillBaseX = (KART_SCR_W - HILL_FRAME_W) / 2;
    kartHillY     = 15;
    kartHillX     = kartHillBaseX;

    kartTrackBaseX = (KART_SCR_W - TRACK_FRAME_W) / 2;
    kartTrackY     = 26;
    kartTrackX     = kartTrackBaseX;
    kartTrackFrameIndex = 0;
    kartTrackMirrorX = false;

    startKartNewRound(millis());

    ModefirstRun = false;
  }

  unsigned long now = millis();

  updateKartModePhase(now);
  renderKartScene();

  wait_with_display(30);
}
