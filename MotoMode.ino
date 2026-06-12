#include "Moto.h"

// =====================================================
// MotoMode.ino
// 64x64 MOTO 橫向賽車模式
// 主函式：MotoMode()
// =====================================================

static const int MOTO_SCR_W = 64;
static const int MOTO_SCR_H = 64;

static const uint16_t MOTO_TRANSPARENT = 0x001f;

// =====================================================
// 圖資尺寸
// =====================================================
static const int MOTO_TRACK_W = 8;
static const int MOTO_TRACK_H = 64;

static const int MOTO_HILL_W = 23;
static const int MOTO_HILL_H = 60;

static const int MOTO_END_W = 8;
static const int MOTO_END_H = 48;

// =====================================================
// 角色圖資
// frame 0 = 停止
// frame 0~1 = 騎車動畫
// frame 2 = 飛躍
// frame 3 = 勝利
// =====================================================
struct MotoDriverDef {
  const uint8_t* sheet;
  const uint16_t* palette;
  int sheetW;
  int frameW;
  int frameH;
};

static const MotoDriverDef MOTO_DRIVER_M = { MOTO_M, MOTO_M_PALETTE, 120, 30, 32 };
static const MotoDriverDef MOTO_DRIVER_L = { MOTO_L, MOTO_L_PALETTE, 120, 30, 32 };
static const MotoDriverDef MOTO_DRIVER_Y = { MOTO_Y, MOTO_Y_PALETTE, 108, 27, 32 };
static const MotoDriverDef MOTO_DRIVER_P = { MOTO_P, MOTO_P_PALETTE, 120, 30, 32 };
static const MotoDriverDef MOTO_DRIVER_W = { MOTO_W, MOTO_W_PALETTE, 124, 31, 32 };
static const MotoDriverDef MOTO_DRIVER_T = { MOTO_T, MOTO_T_PALETTE, 120, 30, 32 };
static const MotoDriverDef MOTO_DRIVER_K = { MOTO_K, MOTO_K_PALETTE, 120, 30, 32 };

static const MotoDriverDef* const motoDrivers[] = {
  &MOTO_DRIVER_M,
  &MOTO_DRIVER_L,
  &MOTO_DRIVER_Y,
  &MOTO_DRIVER_P,
  &MOTO_DRIVER_W,
  &MOTO_DRIVER_T,
  &MOTO_DRIVER_K
};

static const int MOTO_DRIVER_COUNT = sizeof(motoDrivers) / sizeof(motoDrivers[0]);

// =====================================================
// 參數
// =====================================================

// 角色開場由左側騎入定位點的移動間隔，數值越小進場越快
static const unsigned long MOTO_ENTER_STEP_MS = 35;

// 角色進場後停在起跑線等待的時間，單位毫秒
static const unsigned long MOTO_GRID_HOLD_MS = 3000;

// 倒數計時每一個數字切換的間隔，單位毫秒
static const unsigned long MOTO_COUNTDOWN_INTERVAL_MS = 1000;

// 角色騎車動畫 frame0 / frame1 切換速度，數值越小動畫越快
static const unsigned long MOTO_FRAME_INTERVAL_MS = 80;

// 場景更新速度，控制賽道、土丘、終點線移動頻率，數值越小整體速度越快
static const unsigned long MOTO_WORLD_STEP_MS = 30;

// 排名與搶跑道更新頻率，數值越小角色換位越頻繁
static const unsigned long MOTO_RANK_STEP_MS = 90;

// 跳躍動畫每一步的更新間隔，數值越小跳躍越快
static const unsigned long MOTO_JUMP_STEP_MS = 35;

// 跳到最高點後停留的步數，數值越大角色在空中最高點停越久
static const int MOTO_JUMP_HOLD_STEPS = 10;

// 獲勝者往勝利定點移動的速度間隔，數值越小移動越快
static const unsigned long MOTO_WIN_STEP_MS = 45;

// 獲勝畫面停留時間，單位毫秒
static const unsigned long MOTO_VICTORY_HOLD_MS = 5*60*1000;  //5分鐘

// 轉場動畫更新間隔，數值越小轉場越快
static const unsigned long MOTO_TRANSITION_INTERVAL_MS = 90;

// 賽道背景捲動速度，每次更新移動幾個像素
static const int MOTO_TRACK_SPEED = 2;

// 土丘與終點線移動速度，每次更新移動幾個像素
static const int MOTO_OBJECT_SPEED = 2;

// 比賽總距離，達到此距離後開始生成終點線
static const int MOTO_FINISH_DISTANCE = 2500;

// 距離終點前多少距離停止生成土丘，避免終點前還出現障礙
static const int MOTO_NO_HILL_BEFORE_END = 150;

// 土丘生成的最短間隔距離
static const int MOTO_HILL_GAP_MIN = 150;

// 土丘生成的最長間隔距離
static const int MOTO_HILL_GAP_MAX = 200;

// 角色比賽中可橫向移動的最小 X 座標
static const int MOTO_X_MIN = 0;

// 角色比賽中可橫向移動的最大 X 座標
static const int MOTO_X_MAX = 16;

// 第 1 名所在跑道的 Y 座標
static const int MOTO_LANE_FIRST_Y = 14;

// 第 2 名所在跑道的 Y 座標
static const int MOTO_LANE_SECOND_Y = 27;

// 第 3 名所在跑道的 Y 座標
static const int MOTO_LANE_THIRD_Y = 3;

// 轉場方塊大小，數值越大方塊越大
static const int MOTO_TRANSITION_BLOCK_SIZE = 4;

// =====================================================
// 流程
// =====================================================
enum MotoModePhase {
  MOTO_PHASE_ENTER = 0,
  MOTO_PHASE_HOLD,
  MOTO_PHASE_COUNTDOWN,
  MOTO_PHASE_RUNNING,
  MOTO_PHASE_FINISH_IN,
  MOTO_PHASE_WINNER_RIDE,
  MOTO_PHASE_VICTORY,
  MOTO_PHASE_TRANSITION
};

static MotoModePhase motoPhase = MOTO_PHASE_ENTER;

// =====================================================
// 角色狀態
// =====================================================
struct MotoRiderState {
  const MotoDriverDef* driver;
  int driverIndex;

  int x;
  int y;
  int baseY;
  int targetX;
  int targetY;

  int progress;
  int rank;

  int frame;
  int rideFrame;

  bool jumping;
  bool hillTouched;
  int jumpStep;
  int jumpOffset;
};

static MotoRiderState motoRiders[3];

// =====================================================
// 場景狀態
// =====================================================
static int motoTrackOffset = 0;

static bool motoHillActive = false;
static int motoHillX = MOTO_SCR_W;
static int motoNextHillDistance = 160;
static bool motoFirstHillGone = false;

static bool motoEndActive = false;
static int motoEndX = MOTO_SCR_W;

static int motoDistance = 0;
static int motoWinnerIndex = 0;

static int motoTransitionColumn = 0;

// =====================================================
// 計時
// =====================================================
static unsigned long motoPhaseStartMs = 0;
static unsigned long motoLastEnterMs = 0;
static unsigned long motoLastFrameMs = 0;
static unsigned long motoLastWorldMs = 0;
static unsigned long motoLastRankMs = 0;
static unsigned long motoLastJumpMs = 0;
static unsigned long motoLastWinMs = 0;
static unsigned long motoLastTransitionMs = 0;

// =====================================================
// 基礎繪圖
// =====================================================
//palette 讀色小工具

static inline uint16_t motoReadPaletteColor(
  const uint8_t* bitmap,
  const uint16_t* palette,
  uint32_t pixelPos
) {
  uint8_t colorIndex = pgm_read_byte(&(bitmap[pixelPos]));
  return pgm_read_word(&(palette[colorIndex]));
}

static void drawMotoBitmap(
  const uint8_t* bitmap,
  const uint16_t* palette,
  int bitmapW,
  int bitmapH,
  int xOnScreen,
  int yOnScreen
) {
  for (int y = 0; y < bitmapH; y++) {
    int dy = yOnScreen + y;
    if (dy < 0 || dy >= MOTO_SCR_H) continue;

    for (int x = 0; x < bitmapW; x++) {
      int dx = xOnScreen + x;
      if (dx < 0 || dx >= MOTO_SCR_W) continue;

      uint32_t pixelPos =
        (uint32_t)y * (uint32_t)bitmapW +
        (uint32_t)x;

      uint16_t color =
        motoReadPaletteColor(bitmap, palette, pixelPos);

      if (color == MOTO_TRANSPARENT) continue;

      display.drawPixel(dx, dy, color);
    }
  }
}

static void drawMotoSpriteFrame(
  const uint8_t* sheet,
  const uint16_t* palette,
  int sheetW,
  int frameW,
  int frameH,
  int frameIndex,
  int xOnScreen,
  int yOnScreen
) {
  if (frameIndex < 0) return;

  int frameStartX = frameIndex * frameW;

  for (int y = 0; y < frameH; y++) {
    int dy = yOnScreen + y;
    if (dy < 0 || dy >= MOTO_SCR_H) continue;

    for (int x = 0; x < frameW; x++) {
      int dx = xOnScreen + x;
      if (dx < 0 || dx >= MOTO_SCR_W) continue;

      uint32_t pos =
        (uint32_t)y * (uint32_t)sheetW +
        (uint32_t)frameStartX +
        (uint32_t)x;

      uint16_t color =
        motoReadPaletteColor(sheet, palette, pos);

      if (color == MOTO_TRANSPARENT) continue;

      display.drawPixel(dx, dy, color);
    }
  }
}

// =====================================================
// 軌道：MOTO_TRACK 8x64，橫向填滿 64x64
// 比賽開始後慢速往左捲動
// =====================================================
static void drawMotoTrack() {
  for (int x = -motoTrackOffset; x < MOTO_SCR_W; x += MOTO_TRACK_W) {
    drawMotoBitmap(
      MOTO_TRACK,
      MOTO_TRACK_PALETTE,
      MOTO_TRACK_W,
      MOTO_TRACK_H,
      x,
      0
    );
  }
}

// =====================================================
// 倒數文字，中心點靠近指定座標 32,24
// =====================================================
static void drawMotoCountdown() {
  if (motoPhase != MOTO_PHASE_COUNTDOWN) return;

  unsigned long elapsed = millis() - motoPhaseStartMs;
  int step = elapsed / MOTO_COUNTDOWN_INTERVAL_MS;

  display.setTextWrap(false);
  display.setTextSize(2);

  if (step <= 2) {
    int number = 3 - step;

    display.setTextColor(0x0000);
    display.setCursor(40, 25);
    display.print(number);

    display.setTextColor(0xffff);
    display.setCursor(39, 24);
    display.print(number);
  } else {
    display.setTextColor(0x0000);
    display.setCursor(35, 25);
    display.print("GO");

    display.setTextColor(0xf800);
    display.setCursor(34, 24);
    display.print("GO");
  }
}

// =====================================================
// 黑白轉場
// =====================================================
static void drawMotoTransitionOverlay() {
  if (motoPhase != MOTO_PHASE_TRANSITION) return;

  for (int x = 0; x < motoTransitionColumn; x += MOTO_TRANSITION_BLOCK_SIZE) {
    for (int y = 0; y < MOTO_SCR_H; y += MOTO_TRANSITION_BLOCK_SIZE) {
      bool whiteBlock =
        (((x / MOTO_TRANSITION_BLOCK_SIZE) + (y / MOTO_TRANSITION_BLOCK_SIZE)) % 2) == 0;

      display.fillRect(
        x,
        y,
        MOTO_TRANSITION_BLOCK_SIZE,
        MOTO_TRANSITION_BLOCK_SIZE,
        whiteBlock ? 0xffff : 0x0000
      );
    }
  }
}

// =====================================================
// 選 3 位不重複角色
// =====================================================
static void chooseMotoRiders() {
  int used[3];

  used[0] = random(MOTO_DRIVER_COUNT);

  do {
    used[1] = random(MOTO_DRIVER_COUNT);
  } while (used[1] == used[0]);

  do {
    used[2] = random(MOTO_DRIVER_COUNT);
  } while (used[2] == used[0] || used[2] == used[1]);

  for (int i = 0; i < 3; i++) {
    motoRiders[i].driverIndex = used[i];
    motoRiders[i].driver = motoDrivers[used[i]];
  }
}

// =====================================================
// 排名
// =====================================================
static void getMotoOrder(int& firstIdx, int& secondIdx, int& thirdIdx) {
  int idx[3] = {0, 1, 2};

  for (int i = 0; i < 2; i++) {
    for (int j = i + 1; j < 3; j++) {
      if (motoRiders[idx[j]].progress > motoRiders[idx[i]].progress) {
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

static void applyMotoRankTargets() {
  int firstIdx, secondIdx, thirdIdx;
  getMotoOrder(firstIdx, secondIdx, thirdIdx);

  motoRiders[firstIdx].rank = 1;
  motoRiders[secondIdx].rank = 2;
  motoRiders[thirdIdx].rank = 3;

  motoRiders[firstIdx].targetY = MOTO_LANE_FIRST_Y;
  motoRiders[secondIdx].targetY = MOTO_LANE_SECOND_Y;
  motoRiders[thirdIdx].targetY = MOTO_LANE_THIRD_Y;

  motoRiders[firstIdx].targetX = random(14, 21);
  motoRiders[secondIdx].targetX = random(7, 15);
  motoRiders[thirdIdx].targetX = random(0, 8);
}

static void moveMotoRidersTowardTarget() {
  for (int i = 0; i < 3; i++) {
    if (motoRiders[i].jumping) continue;

    if (motoRiders[i].x < motoRiders[i].targetX) motoRiders[i].x++;
    if (motoRiders[i].x > motoRiders[i].targetX) motoRiders[i].x--;

    if (motoRiders[i].baseY < motoRiders[i].targetY) motoRiders[i].baseY++;
    if (motoRiders[i].baseY > motoRiders[i].targetY) motoRiders[i].baseY--;

    motoRiders[i].y = motoRiders[i].baseY;
  }
}

// =====================================================
// 跳躍
// 碰到土丘後 frame2，先上移 9px，再落回原軌道
// =====================================================
static void startMotoJump(int idx) {
  if (motoRiders[idx].jumping) return;

  motoRiders[idx].jumping = true;
  motoRiders[idx].jumpStep = 0;
  motoRiders[idx].jumpOffset = 0;
  motoRiders[idx].frame = 2;
}

static void updateMotoJumps(unsigned long now) {
  if (now - motoLastJumpMs < MOTO_JUMP_STEP_MS) return;
  motoLastJumpMs = now;

  for (int i = 0; i < 3; i++) {
    if (!motoRiders[i].jumping) continue;

    motoRiders[i].jumpStep++;

if (motoRiders[i].jumpStep <= 9) {
  motoRiders[i].jumpOffset = -motoRiders[i].jumpStep;
} else if (motoRiders[i].jumpStep <= 9 + MOTO_JUMP_HOLD_STEPS) {
  motoRiders[i].jumpOffset = -9;
} else if (motoRiders[i].jumpStep <= 18 + MOTO_JUMP_HOLD_STEPS) {
  motoRiders[i].jumpOffset = -(18 + MOTO_JUMP_HOLD_STEPS - motoRiders[i].jumpStep);
} else {
      motoRiders[i].jumping = false;
      motoRiders[i].jumpStep = 0;
      motoRiders[i].jumpOffset = 0;
      motoRiders[i].frame = motoRiders[i].rideFrame;
    }

    motoRiders[i].y = motoRiders[i].baseY + motoRiders[i].jumpOffset;
  }
}

// =====================================================
// 土丘碰撞
// =====================================================
static void checkMotoHillCollision() {
  if (!motoHillActive) return;

  for (int i = 0; i < 3; i++) {
    int riderFront = motoRiders[i].x + motoRiders[i].driver->frameW - 6;
    int hillFront = motoHillX + 4;
    int hillBack = motoHillX + MOTO_HILL_W - 4;

if (!motoRiders[i].hillTouched &&
    riderFront >= hillFront &&
    riderFront <= hillBack) {
  motoRiders[i].hillTouched = true;
  startMotoJump(i);
}
  }
}

// =====================================================
// 初始化新回合
// =====================================================
static void startMotoNewRound(unsigned long now) {
  chooseMotoRiders();

  motoPhase = MOTO_PHASE_ENTER;
  motoPhaseStartMs = now;

  motoLastEnterMs = now;
  motoLastFrameMs = now;
  motoLastWorldMs = now;
  motoLastRankMs = now;
  motoLastJumpMs = now;
  motoLastWinMs = now;
  motoLastTransitionMs = now;

  motoTrackOffset = 0;

  motoHillActive = false;
  motoHillX = MOTO_SCR_W;
  motoNextHillDistance = random(MOTO_HILL_GAP_MIN, MOTO_HILL_GAP_MAX + 1);
  motoFirstHillGone = false;

  motoEndActive = false;
  motoEndX = MOTO_SCR_W;

  motoDistance = 0;
  motoWinnerIndex = 0;
  motoTransitionColumn = 0;

  int startY[3] = {3, 14, 27};
  int targetX[3] = {-11, -1, -11};

  for (int i = 0; i < 3; i++) {
    motoRiders[i].x = -42 - random(0, 12);
    motoRiders[i].y = startY[i];
    motoRiders[i].baseY = startY[i];

    motoRiders[i].targetX = targetX[i];
    motoRiders[i].targetY = startY[i];

    motoRiders[i].progress = random(0, 20);
    motoRiders[i].rank = i + 1;

    motoRiders[i].frame = 0;
    motoRiders[i].rideFrame = 0;

    motoRiders[i].jumping = false;
    motoRiders[i].hillTouched = false;
    motoRiders[i].jumpStep = 0;
    motoRiders[i].jumpOffset = 0;
  }
}

// =====================================================
// 進入正式比賽
// =====================================================
static void startMotoRunning(unsigned long now) {
  motoPhase = MOTO_PHASE_RUNNING;
  motoPhaseStartMs = now;

  for (int i = 0; i < 3; i++) {
    motoRiders[i].targetX = random(MOTO_X_MIN, MOTO_X_MAX + 1);
    motoRiders[i].frame = 0;
    motoRiders[i].rideFrame = 0;
  }
}

// =====================================================
// 更新流程
// =====================================================
static void updateMotoPhase(unsigned long now) {
  switch (motoPhase) {
    case MOTO_PHASE_ENTER: {
      bool arrived = true;

      if (now - motoLastEnterMs >= MOTO_ENTER_STEP_MS) {
        motoLastEnterMs = now;

        for (int i = 0; i < 3; i++) {
          motoRiders[i].rideFrame = 1 - motoRiders[i].rideFrame;
          motoRiders[i].frame = motoRiders[i].rideFrame;

          if (motoRiders[i].x < motoRiders[i].targetX) {
            motoRiders[i].x++;
            arrived = false;
          }
        }
      }

      for (int i = 0; i < 3; i++) {
        if (motoRiders[i].x < motoRiders[i].targetX) arrived = false;
      }

      if (arrived) {
        for (int i = 0; i < 3; i++) motoRiders[i].frame = 0;
        motoPhase = MOTO_PHASE_HOLD;
        motoPhaseStartMs = now;
      }

      break;
    }

    case MOTO_PHASE_HOLD: {
      for (int i = 0; i < 3; i++) motoRiders[i].frame = 0;

      if (now - motoPhaseStartMs >= MOTO_GRID_HOLD_MS) {
        motoPhase = MOTO_PHASE_COUNTDOWN;
        motoPhaseStartMs = now;
      }

      break;
    }

    case MOTO_PHASE_COUNTDOWN: {
      for (int i = 0; i < 3; i++) motoRiders[i].frame = 0;

      if (now - motoPhaseStartMs >= MOTO_COUNTDOWN_INTERVAL_MS * 4) {
        startMotoRunning(now);
      }

      break;
    }

    case MOTO_PHASE_RUNNING: {
      if (now - motoLastFrameMs >= MOTO_FRAME_INTERVAL_MS) {
        motoLastFrameMs = now;

        for (int i = 0; i < 3; i++) {
          motoRiders[i].rideFrame = 1 - motoRiders[i].rideFrame;
          if (!motoRiders[i].jumping) motoRiders[i].frame = motoRiders[i].rideFrame;
        }
      }

      if (now - motoLastWorldMs >= MOTO_WORLD_STEP_MS) {
        motoLastWorldMs = now;

        motoTrackOffset += MOTO_TRACK_SPEED;
        if (motoTrackOffset >= MOTO_TRACK_W) motoTrackOffset = 0;

        motoDistance += MOTO_TRACK_SPEED;

        for (int i = 0; i < 3; i++) {
          motoRiders[i].progress += random(1, 5);
        }

        if (!motoHillActive &&
            !motoEndActive &&
            motoDistance >= motoNextHillDistance &&
            motoDistance < MOTO_FINISH_DISTANCE - MOTO_NO_HILL_BEFORE_END) {
          motoHillActive = true;
          motoHillX = MOTO_SCR_W;
          for (int i = 0; i < 3; i++) {
  motoRiders[i].hillTouched = false;
}
        }

        if (motoHillActive) {
          motoHillX -= MOTO_OBJECT_SPEED;

          if (motoHillX < -MOTO_HILL_W) {
            motoHillActive = false;
            motoFirstHillGone = true;
            motoNextHillDistance =
              motoDistance + random(MOTO_HILL_GAP_MIN, MOTO_HILL_GAP_MAX + 1);
          }
        }

        if (!motoEndActive && motoDistance >= MOTO_FINISH_DISTANCE) {
          motoEndActive = true;
          motoEndX = MOTO_SCR_W;
        }

        if (motoEndActive) {
          motoEndX -= MOTO_OBJECT_SPEED;

          if (motoEndX <= 0) {
            int firstIdx, secondIdx, thirdIdx;
            getMotoOrder(firstIdx, secondIdx, thirdIdx);

            motoWinnerIndex = firstIdx;
            motoRiders[motoWinnerIndex].targetX = 24;
            motoRiders[motoWinnerIndex].targetY = MOTO_LANE_FIRST_Y;
            motoRiders[motoWinnerIndex].baseY = MOTO_LANE_FIRST_Y;
            motoRiders[motoWinnerIndex].y = MOTO_LANE_FIRST_Y;

            motoPhase = MOTO_PHASE_WINNER_RIDE;
            motoPhaseStartMs = now;
          }
        }

        checkMotoHillCollision();
      }

      updateMotoJumps(now);

      if (motoFirstHillGone && now - motoLastRankMs >= MOTO_RANK_STEP_MS) {
        motoLastRankMs = now;
        applyMotoRankTargets();
        moveMotoRidersTowardTarget();
      }

      break;
    }

    case MOTO_PHASE_WINNER_RIDE: {
      if (now - motoLastFrameMs >= MOTO_FRAME_INTERVAL_MS) {
        motoLastFrameMs = now;
        motoRiders[motoWinnerIndex].rideFrame = 1 - motoRiders[motoWinnerIndex].rideFrame;
        motoRiders[motoWinnerIndex].frame = motoRiders[motoWinnerIndex].rideFrame;
      }

      if (now - motoLastWorldMs >= MOTO_WORLD_STEP_MS) {
        motoLastWorldMs = now;

        motoTrackOffset += MOTO_TRACK_SPEED;
        if (motoTrackOffset >= MOTO_TRACK_W) motoTrackOffset = 0;

        if (motoEndX < MOTO_SCR_W) motoEndX -= MOTO_OBJECT_SPEED;
      }

      if (now - motoLastWinMs >= MOTO_WIN_STEP_MS) {
        motoLastWinMs = now;

        if (motoRiders[motoWinnerIndex].x < 24) {
          motoRiders[motoWinnerIndex].x++;
        } else {
          wait_with_display(2000);
          motoRiders[motoWinnerIndex].frame = 3;
          motoPhase = MOTO_PHASE_VICTORY;
          motoPhaseStartMs = now;
        }
      }

      break;
    }

    case MOTO_PHASE_VICTORY: {
      motoRiders[motoWinnerIndex].frame = 3;

      if (now - motoPhaseStartMs >= MOTO_VICTORY_HOLD_MS) {
        motoPhase = MOTO_PHASE_TRANSITION;
        motoPhaseStartMs = now;
        motoLastTransitionMs = now;
        motoTransitionColumn = 0;
      }

      break;
    }

    case MOTO_PHASE_TRANSITION: {
      if (now - motoLastTransitionMs >= MOTO_TRANSITION_INTERVAL_MS) {
        motoLastTransitionMs = now;
        motoTransitionColumn += MOTO_TRANSITION_BLOCK_SIZE;

        if (motoTransitionColumn > MOTO_SCR_W + MOTO_TRANSITION_BLOCK_SIZE) {
          startMotoNewRound(now);
        }
      }

      break;
    }

    default:
      break;
  }
}

// =====================================================
// 繪製角色
// =====================================================
static void drawMotoRiders() {
  int order[3] = {0, 1, 2};

  for (int i = 0; i < 2; i++) {
    for (int j = i + 1; j < 3; j++) {
      int footA = motoRiders[order[i]].y + motoRiders[order[i]].driver->frameH;
      int footB = motoRiders[order[j]].y + motoRiders[order[j]].driver->frameH;

      if (footA > footB) {
        int t = order[i];
        order[i] = order[j];
        order[j] = t;
      }
    }
  }

  for (int k = 0; k < 3; k++) {
    int idx = order[k];

    if (motoPhase == MOTO_PHASE_WINNER_RIDE ||
        motoPhase == MOTO_PHASE_VICTORY ||
        motoPhase == MOTO_PHASE_TRANSITION) {
      if (idx != motoWinnerIndex) continue;
    }

drawMotoSpriteFrame(
  motoRiders[idx].driver->sheet,
  motoRiders[idx].driver->palette,
  motoRiders[idx].driver->sheetW,
  motoRiders[idx].driver->frameW,
  motoRiders[idx].driver->frameH,
  motoRiders[idx].frame,
  motoRiders[idx].x,
  motoRiders[idx].y
);
  }
}

// =====================================================
// 重繪場景
// =====================================================
static void renderMotoScene() {
  display.fillScreen(0x0000);

  drawMotoTrack();

  if (motoHillActive) {
    drawMotoBitmap(
  MOTO_HILL,
  MOTO_HILL_PALETTE,
  MOTO_HILL_W,
  MOTO_HILL_H,
  motoHillX,
  0
);
  }

  if (motoEndActive ||
      motoPhase == MOTO_PHASE_WINNER_RIDE ||
      motoPhase == MOTO_PHASE_VICTORY ||
      motoPhase == MOTO_PHASE_TRANSITION) {
    drawMotoBitmap(
  MOTO_END,
  MOTO_END_PALETTE,
  MOTO_END_W,
  MOTO_END_H,
  motoEndX,
  12
);
  }

  drawMotoRiders();
  drawMotoCountdown();

  drawThemeClockText();
  drawMotoTransitionOverlay();
}

// =====================================================
// 主函式
// =====================================================
void MotoMode() {
  if (ModefirstRun) {
    randomSeed(millis());
    startMotoNewRound(millis());
    ModefirstRun = false;
  }

  unsigned long now = millis();

  updateMotoPhase(now);
  renderMotoScene();

  wait_with_display(30);
}
