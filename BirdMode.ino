#include "Bird.h"

// =====================================================
// Angry Bird Mode
// 修正版：
// 1. init 選定主角後，飛行時不再重新抽鳥
// 2. 主角從 init 實際位置起飛，不再瞬移到 X=24
// 3. CAM 改成平滑追蹤，減少飛行與背景瞬移感
// 4. 拋物線飛更高，並支援 BG 高度超過 64px
// 5. 飛行途中逐幀偵測 BLOCK，第一個碰到的 BLOCK 才會受擊
// 6. BIRD_BOM 改為碰撞點置中顯示
// 7. CAM 回 init 改用 smoothstep 緩動，修正最後階段瞬移
// 8. 主角消失後，CAM 先移到 BLOCK 區塊置中觀察 3 秒，再回 init
// 9. 豬 BLOCK 改成每次 reset 時在 3x3 區域隨機 3 個不重複位置
// =====================================================


// =====================================================
// 螢幕大小
// =====================================================
static const int BIRD_SCR_W = 64;
static const int BIRD_SCR_H = 64;


// =====================================================
// 透明色
// BMP 背景透明色 RGB(0,91,91) 轉 RGB565 = 0x02CB
// =====================================================
static const uint16_t BIRD_TRANSPARENT = 0x02CB;


// =====================================================
// 圖資尺寸
// =====================================================
static const int BIRD_BG_W = 16;
static const int BIRD_BG_H = 92;
static const int BIRD_BG_VIEW_Y = BIRD_BG_H - BIRD_SCR_H;

static const int BIRD_A_W = 18;
static const int BIRD_A_H = 16;

static const int BIRD_B_W = 19;
static const int BIRD_B_H = 19;

static const int BIRD_C_W = 16;
static const int BIRD_C_H = 16;

static const int BIRD_D_W = 18;
static const int BIRD_D_H = 20;

static const int BIRD_E_W = 21;
static const int BIRD_E_H = 21;

static const int BIRD_F_W = 16;
static const int BIRD_F_H = 16;

static const int BIRD_G_W = 18;
static const int BIRD_G_H = 18;

static const int BIRD_H_W = 18;
static const int BIRD_H_H = 19;

static const int BIRD_I_W = 16;
static const int BIRD_I_H = 16;


static const int BIRD_BOM_W = 15;
static const int BIRD_BOM_H = 15;

static const int BIRD_BLOCK_SHEET_W = 48;
static const int BIRD_BLOCK_H = 16;
static const int BIRD_BLOCK_FRAME_W = 16;
static const int BIRD_BLOCK_FRAME_COUNT = 3;

static const int BIRD_SHOT_W = 15;
static const int BIRD_SHOT_H = 30;


// =====================================================
// 可調參數
// =====================================================

// 每幀更新間隔，數字越小動畫越順，但負擔越高。
// 建議範圍：20 ~ 40。
static const unsigned long BIRD_FRAME_DELAY_MS = 30UL;

// 進入 init 畫面後，等待多久發射第一隻鳥。
// 單位：毫秒。5000 = 5 秒。
static const unsigned long BIRD_FIRST_SHOT_WAIT_MS = 5000UL;

// 每次普通彈射結束並回到 init 位置後，等待多久發射下一隻鳥。
// 單位：毫秒。5000 = 5 秒。
static const unsigned long BIRD_NEXT_SHOT_WAIT_MS = 5000UL;

// 撞擊與爆炸效果結束後，主角停在撞擊點多久才消失。
static const unsigned long BIRD_POST_HIT_HOLD_MS = 500UL;

// 主角消失後，CAM 在原撞擊位置停留多久，接著才移到 BLOCK 區塊觀察。
static const unsigned long BIRD_HIDE_HOLD_MS = 1000UL;

// CAM 移動到 BLOCK 區塊置中的時間。
// 單位：毫秒。數字越大移動越慢。
static const unsigned long BIRD_BLOCK_FOCUS_MOVE_MS = 800UL;

// CAM 鎖定 BLOCK 區塊置中後，觀察多久才開始回 init。
static const unsigned long BIRD_BLOCK_FOCUS_HOLD_MS = 1000UL;

// CAM 從 BLOCK 區塊平滑移回 init 的時間。
// 單位：毫秒。數字越大回鏡越慢。
static const unsigned long BIRD_CAM_RETURN_MS = 2500UL;

// 三隻豬都消失後，等待多久顯示 WIN。
// 單位：毫秒。2000 = 2 秒。
static const unsigned long BIRD_WIN_DELAY_MS = 2000UL;

// WIN 畫面停留多久。
// 單位：毫秒。5000 = 5 秒。
static const unsigned long BIRD_WIN_HOLD_MS = 5000UL;

// WIN 後回到 init 畫面，等待多久重新開始整個 loop。
// 單位：毫秒。300000 = 5 分鐘。
static const unsigned long BIRD_RESTART_WAIT_MS = 300000UL;

// 爆炸圖 BIRD_BOM 閃爍次數。
// 3 = 閃爍 3 次。
static const uint8_t BIRD_BOM_BLINK_COUNT = 3;

// 爆炸圖每次亮 / 滅的間隔。
// 單位：毫秒。數字越小閃越快。
static const unsigned long BIRD_BOM_BLINK_INTERVAL_MS = 130UL;

// BLOCK 被打中後閃爍次數。
// 5 = 閃爍 5 次。
static const uint8_t BIRD_BLOCK_BLINK_COUNT = 5;

// BLOCK 每次亮 / 滅的間隔。
// 單位：毫秒。數字越小閃越快。
static const unsigned long BIRD_BLOCK_BLINK_INTERVAL_MS = 110UL;

// 飛行結果權重：落在 BLOCK 前方。
// 數字越大，越容易在目標前方落地。
static const int BIRD_CHANCE_BEFORE_BLOCK = 25;

// 飛行結果權重：朝 BLOCK 方向命中。
// 注意：真正受擊的 BLOCK 仍然由飛行途中第一個碰到的 BLOCK 決定。
static const int BIRD_CHANCE_HIT_BLOCK = 50;

// 飛行結果權重：飛過 BLOCK 後方。
// 數字越大，越容易越過目標區。
static const int BIRD_CHANCE_AFTER_BLOCK = 25;

// 拋物線最低飛行高度。
// 數字越大，鳥飛得越高。
static const int BIRD_ARC_HIGH_MIN = 42;

// 拋物線最高飛行高度。
// 數字越大，鳥飛得越高，但太高可能會飛出畫面。
static const int BIRD_ARC_HIGH_MAX = 60;

// after-block 的額外抬高高度。
// 用來讓飛過目標區的路線更容易越過 BLOCK。
static const int BIRD_AFTER_EXTRA_ARC = 32;

// 基礎飛行時間。
// 單位：毫秒。數字越大，整體飛行越慢。
static const unsigned long BIRD_FLIGHT_BASE_MS = 3600UL;

// 飛行距離造成的額外時間係數。
// 數字越大，飛越遠時速度越慢。
static const float BIRD_FLIGHT_DISTANCE_MS = 12.0f;

// 飛行時間上限。
// 單位：毫秒。避免飛太慢。
static const unsigned long BIRD_FLIGHT_MAX_MS = 5800UL;

// 飛行途中碰撞取樣密度。
// 數字越小越不容易穿透 BLOCK，但運算量較高。
// 建議範圍：2 ~ 4。
static const int BIRD_COLLISION_SAMPLE_PX = 2;

// CAM 追蹤平滑度。
// 0.10 = 很慢很柔順；0.35 = 跟很緊；1.00 = 立即鎖定。
// 建議範圍：0.18 ~ 0.30。
static const float BIRD_CAM_FOLLOW_LERP = 0.24f;


// =====================================================
// 場景座標
// =====================================================
static const int BIRD_INIT_HERO_X = 0;
static const int BIRD_INIT_HERO_Y = 43;

// 飛行中 CAM 想把主角維持在畫面 X=24 附近。
// 注意：不是一開始瞬移到 X=24，而是飛到中段後 CAM 才跟上。
static const int BIRD_HERO_LOCK_SCREEN_X = 24;

// 飛高時 CAM 想把主角維持在畫面 Y=22 附近，讓上方天空露出。
static const int BIRD_HERO_LOCK_SCREEN_Y_HIGH = 22;

static const int BIRD_SHOT_X = 49;
static const int BIRD_SHOT_Y = 34;

static const int BIRD_ROPE_1_X1 = 51;
static const int BIRD_ROPE_1_Y1 = 39;
static const int BIRD_ROPE_1_X2 = 9;
static const int BIRD_ROPE_1_Y2 = 52;

static const int BIRD_ROPE_2_X1 = 61;
static const int BIRD_ROPE_2_Y1 = 41;
static const int BIRD_ROPE_2_X2 = 9;
static const int BIRD_ROPE_2_Y2 = 60;

static const int BIRD_BLOCK_COLS = 3;
static const int BIRD_BLOCK_ROWS = 3;
static const int BIRD_BLOCK_TOTAL = BIRD_BLOCK_COLS * BIRD_BLOCK_ROWS;

static const int BIRD_TARGET_X = 112;
static const int BIRD_TARGET_Y = 16;

static const int BIRD_GROUND_Y = 64;


// =====================================================
// 顏色
// =====================================================
static const uint16_t BIRD_COLOR_BLACK = 0x0000;
static const uint16_t BIRD_COLOR_WHITE = 0xFFFF;
static const uint16_t BIRD_COLOR_RED = 0xF800;
static const uint16_t BIRD_COLOR_ROPE = 0x8A22;


// =====================================================
// 主角定義
// =====================================================
static const uint8_t BIRD_HERO_A = 0;
static const uint8_t BIRD_HERO_B = 1;
static const uint8_t BIRD_HERO_C = 2;
static const uint8_t BIRD_HERO_D = 3;
static const uint8_t BIRD_HERO_E = 4;
static const uint8_t BIRD_HERO_F = 5;
static const uint8_t BIRD_HERO_G = 6;
static const uint8_t BIRD_HERO_H = 7;
static const uint8_t BIRD_HERO_I = 8;



static uint8_t birdHero = BIRD_HERO_A;


// =====================================================
// BLOCK frame 定義
// frame0 = 空箱
// frame1 = 豬豬在箱
// frame2 = 豬豬
// =====================================================
static const uint8_t BIRD_BLOCK_FRAME_EMPTY_BOX = 0;
static const uint8_t BIRD_BLOCK_FRAME_PIG_BOX = 1;
static const uint8_t BIRD_BLOCK_FRAME_PIG = 2;

// 初始豬數量。
// 會在 3x3 BLOCK 裡隨機挑 3 個不重複位置放豬。
static const uint8_t BIRD_PIG_START_COUNT = 3;


// =====================================================
// 狀態機
// =====================================================
static const uint8_t BIRD_STATE_INIT_WAIT = 0;
static const uint8_t BIRD_STATE_FLY = 1;
static const uint8_t BIRD_STATE_IMPACT = 2;
static const uint8_t BIRD_STATE_POST_HIT_HOLD = 3;
static const uint8_t BIRD_STATE_HIDE_BIRD = 4;
static const uint8_t BIRD_STATE_BLOCK_FOCUS = 5;
static const uint8_t BIRD_STATE_CAM_RETURN = 6;
static const uint8_t BIRD_STATE_WAIT_NEXT = 7;
static const uint8_t BIRD_STATE_WIN_DELAY = 8;
static const uint8_t BIRD_STATE_WIN = 9;
static const uint8_t BIRD_STATE_RESTART_WAIT = 10;

static uint8_t birdState = BIRD_STATE_INIT_WAIT;
static unsigned long birdStateStartMs = 0;
static unsigned long birdLastUpdateMs = 0;


// =====================================================
// CAM 與主角
// birdCamY 可為負值：代表鏡頭往上看，顯示 BG 更高的位置
// =====================================================
static float birdCamX = 0.0f;
static float birdCamY = 0.0f;

static float birdReturnCamStartX = 0.0f;
static float birdReturnCamStartY = 0.0f;

static float birdBlockFocusCamStartX = 0.0f;
static float birdBlockFocusCamStartY = 0.0f;
static float birdBlockFocusCamTargetX = 0.0f;
static float birdBlockFocusCamTargetY = 0.0f;

static float birdHeroX = BIRD_INIT_HERO_X;
static float birdHeroY = BIRD_INIT_HERO_Y;

static bool birdHeroVisible = true;
static bool birdClockVisible = true;


// =====================================================
// 飛行路徑
// =====================================================
static float birdFlightStartX = BIRD_INIT_HERO_X;
static float birdFlightStartY = BIRD_INIT_HERO_Y;

static float birdFlightCtrlX = 70.0f;
static float birdFlightCtrlY = -25.0f;

static float birdFlightTargetX = 120.0f;
static float birdFlightTargetY = 40.0f;

static unsigned long birdFlightStartMs = 0;
static unsigned long birdFlightDurationMs = 3500UL;


// =====================================================
// 撞擊狀態
// =====================================================
static const uint8_t BIRD_OUTCOME_BEFORE = 0;
static const uint8_t BIRD_OUTCOME_HIT = 1;
static const uint8_t BIRD_OUTCOME_AFTER = 2;

static uint8_t birdShotOutcome = BIRD_OUTCOME_BEFORE;

static float birdBoomX = 0.0f;
static float birdBoomY = 0.0f;

// 真正碰撞點。
// BIRD_BOM 會用這個點當作爆炸圖中心點。
static float birdImpactX = 0.0f;
static float birdImpactY = 0.0f;

static int birdHitBlockIndex = -1;
static bool birdBlockDamageApplied = false;


// =====================================================
// BLOCK 狀態
// =====================================================
static bool birdBlockVisible[BIRD_BLOCK_TOTAL];
static uint8_t birdBlockFrame[BIRD_BLOCK_TOTAL];
static uint8_t birdPigsRemaining = BIRD_PIG_START_COUNT;


// =====================================================
// WIN 狀態
// =====================================================
static bool birdWinScreenDrawn = false;


// =====================================================
// 小工具
// =====================================================
static long birdWrapIndex(long value, int maxValue) {
  if (maxValue <= 0) {
    return 0;
  }

  long result = value % maxValue;

  if (result < 0) {
    result += maxValue;
  }

  return result;
}


static float birdClampFloat(float value, float minValue, float maxValue) {
  if (value < minValue) {
    return minValue;
  }

  if (value > maxValue) {
    return maxValue;
  }

  return value;
}


static float birdAbsFloat(float value) {
  if (value < 0.0f) {
    return -value;
  }

  return value;
}


static float birdMaxFloat(float a, float b) {
  if (a > b) {
    return a;
  }

  return b;
}


static float birdMinFloat(float a, float b) {
  if (a < b) {
    return a;
  }

  return b;
}


static const uint16_t* getBirdHeroSprite() {
  if (birdHero == BIRD_HERO_B) {
    return BIRD_B;
  }

  if (birdHero == BIRD_HERO_C) {
    return BIRD_C;
  }

  if (birdHero == BIRD_HERO_D) {
    return BIRD_D;
  }

  if (birdHero == BIRD_HERO_E) {
    return BIRD_E;
  }

  if (birdHero == BIRD_HERO_F) {
    return BIRD_F;
  }

  if (birdHero == BIRD_HERO_G) {
    return BIRD_G;
  }

  if (birdHero == BIRD_HERO_H) {
    return BIRD_H;
  }

  if (birdHero == BIRD_HERO_I) {
    return BIRD_I;
  }

  return BIRD_A;
}


static int getBirdHeroW() {
  if (birdHero == BIRD_HERO_B) {
    return BIRD_B_W;
  }

  if (birdHero == BIRD_HERO_C) {
    return BIRD_C_W;
  }

  if (birdHero == BIRD_HERO_D) {
    return BIRD_D_W;
  }

  if (birdHero == BIRD_HERO_E) {
    return BIRD_E_W;
  }

  if (birdHero == BIRD_HERO_F) {
    return BIRD_F_W;
  }

  if (birdHero == BIRD_HERO_G) {
    return BIRD_G_W;
  }

  if (birdHero == BIRD_HERO_H) {
    return BIRD_H_W;
  }

  if (birdHero == BIRD_HERO_I) {
    return BIRD_I_W;
  }
  

  return BIRD_A_W;
}


static int getBirdHeroH() {
  if (birdHero == BIRD_HERO_B) {
    return BIRD_B_H;
  }

  if (birdHero == BIRD_HERO_C) {
    return BIRD_C_H;
  }

  if (birdHero == BIRD_HERO_D) {
    return BIRD_D_H;
  }

  if (birdHero == BIRD_HERO_E) {
    return BIRD_E_H;
  }

  if (birdHero == BIRD_HERO_F) {
    return BIRD_F_H;
  }

  if (birdHero == BIRD_HERO_G) {
    return BIRD_G_H;
  }

  if (birdHero == BIRD_HERO_H) {
    return BIRD_H_H;
  }

  if (birdHero == BIRD_HERO_I) {
    return BIRD_I_H;
  }

  return BIRD_A_H;
}


static void selectRandomBirdHero() {
  birdHero = random(0, 9);
}


// =====================================================
// RGB565 sprite 繪製
// =====================================================
static void drawBirdSpriteFrameScreen(
  int x,
  int y,
  int sheetW,
  int sheetH,
  const uint16_t* sprite,
  int srcX,
  int srcY,
  int frameW,
  int frameH,
  bool useTransparent
) {
  if (!sprite) {
    return;
  }

  for (int j = 0; j < frameH; j++) {
    int dy = y + j;

    if (dy < 0 || dy >= BIRD_SCR_H) {
      continue;
    }

    int sy = srcY + j;

    if (sy < 0 || sy >= sheetH) {
      continue;
    }

    for (int i = 0; i < frameW; i++) {
      int dx = x + i;

      if (dx < 0 || dx >= BIRD_SCR_W) {
        continue;
      }

      int sx = srcX + i;

      if (sx < 0 || sx >= sheetW) {
        continue;
      }

      uint32_t pos =
        (uint32_t)sy * (uint32_t)sheetW +
        (uint32_t)sx;

      uint16_t color = pgm_read_word(&(sprite[pos]));

      if (useTransparent && color == BIRD_TRANSPARENT) {
        continue;
      }

      display.drawPixel(dx, dy, color);
    }
  }
}


static void drawBirdSpriteScreen(
  int x,
  int y,
  int w,
  int h,
  const uint16_t* sprite,
  bool useTransparent
) {
  drawBirdSpriteFrameScreen(
    x,
    y,
    w,
    h,
    sprite,
    0,
    0,
    w,
    h,
    useTransparent
  );
}


static void drawBirdSpriteFrameWorld(
  float worldX,
  float worldY,
  int sheetW,
  int sheetH,
  const uint16_t* sprite,
  int srcX,
  int srcY,
  int frameW,
  int frameH,
  bool useTransparent
) {
  int screenX = (int)(worldX - birdCamX);
  int screenY = (int)(worldY - birdCamY);

  drawBirdSpriteFrameScreen(
    screenX,
    screenY,
    sheetW,
    sheetH,
    sprite,
    srcX,
    srcY,
    frameW,
    frameH,
    useTransparent
  );
}


static void drawBirdSpriteWorld(
  float worldX,
  float worldY,
  int w,
  int h,
  const uint16_t* sprite,
  bool useTransparent
) {
  drawBirdSpriteFrameWorld(
    worldX,
    worldY,
    w,
    h,
    sprite,
    0,
    0,
    w,
    h,
    useTransparent
  );
}


// =====================================================
// 背景
// X：依照 CAM X 持續重複
// Y：依照 CAM Y 顯示 BG 上方或下方
// =====================================================
static void drawBirdBackground() {
  long camXInt = (long)birdCamX;
  long camYInt = (long)birdCamY;

  for (int y = 0; y < BIRD_SCR_H; y++) {
    int srcY = BIRD_BG_VIEW_Y + y + camYInt;

    if (srcY < 0) {
      srcY = 0;
    }

    if (srcY >= BIRD_BG_H) {
      srcY = BIRD_BG_H - 1;
    }

    for (int x = 0; x < BIRD_SCR_W; x++) {
      int srcX = (int)birdWrapIndex((long)x + camXInt, BIRD_BG_W);

      uint32_t pos =
        (uint32_t)srcY * (uint32_t)BIRD_BG_W +
        (uint32_t)srcX;

      uint16_t color = pgm_read_word(&(BIRD_BG[pos]));
      display.drawPixel(x, y, color);
    }
  }
}


// =====================================================
// CAM 更新
// instant = true  立即定位，用在 init / reset
// instant = false 平滑追蹤，用在飛行中
// =====================================================
static void updateBirdCameraForHero(bool instant) {
  float desiredCamX = birdHeroX - (float)BIRD_HERO_LOCK_SCREEN_X;

  if (desiredCamX < 0.0f) {
    desiredCamX = 0.0f;
  }

  float desiredCamY = 0.0f;

  if (birdHeroY < (float)BIRD_HERO_LOCK_SCREEN_Y_HIGH) {
    desiredCamY = birdHeroY - (float)BIRD_HERO_LOCK_SCREEN_Y_HIGH;
  }

  float minCamY = -(float)BIRD_BG_VIEW_Y;

  desiredCamY = birdClampFloat(desiredCamY, minCamY, 0.0f);

  if (instant) {
    birdCamX = desiredCamX;
    birdCamY = desiredCamY;
    return;
  }

  birdCamX = birdCamX + (desiredCamX - birdCamX) * BIRD_CAM_FOLLOW_LERP;
  birdCamY = birdCamY + (desiredCamY - birdCamY) * BIRD_CAM_FOLLOW_LERP;

  if (birdAbsFloat(desiredCamX - birdCamX) < 0.05f) {
    birdCamX = desiredCamX;
  }

  if (birdAbsFloat(desiredCamY - birdCamY) < 0.05f) {
    birdCamY = desiredCamY;
  }
}


// =====================================================
// CAM 鎖定 BLOCK 區塊
// 讓 3x3 BLOCK 區域盡量置中。
// Y 軸限制在背景圖可顯示範圍內，避免畫面底部被拉出背景。
// =====================================================
static float getBirdBlockFocusCamX() {
  float blockCenterX =
    (float)BIRD_TARGET_X +
    ((float)(BIRD_BLOCK_COLS * BIRD_BLOCK_FRAME_W) * 0.50f);

  float desiredCamX = blockCenterX - ((float)BIRD_SCR_W * 0.50f);

  if (desiredCamX < 0.0f) {
    desiredCamX = 0.0f;
  }

  return desiredCamX;
}


static float getBirdBlockFocusCamY() {
  float blockCenterY =
    (float)BIRD_TARGET_Y +
    ((float)(BIRD_BLOCK_ROWS * BIRD_BLOCK_H) * 0.50f);

  float desiredCamY = blockCenterY - ((float)BIRD_SCR_H * 0.50f);

  float minCamY = -(float)BIRD_BG_VIEW_Y;
  float maxCamY = 0.0f;

  return birdClampFloat(desiredCamY, minCamY, maxCamY);
}


static void updateBirdBlockFocusCamera(unsigned long elapsedMs) {
  if (BIRD_BLOCK_FOCUS_MOVE_MS == 0UL) {
    birdCamX = birdBlockFocusCamTargetX;
    birdCamY = birdBlockFocusCamTargetY;
    return;
  }

  float t = (float)elapsedMs / (float)BIRD_BLOCK_FOCUS_MOVE_MS;
  t = birdClampFloat(t, 0.0f, 1.0f);

  // smoothstep 緩動：讓 CAM 移到 BLOCK 時不會突然跳動。
  float easeT = t * t * (3.0f - 2.0f * t);

  birdCamX =
    birdBlockFocusCamStartX +
    (birdBlockFocusCamTargetX - birdBlockFocusCamStartX) * easeT;

  birdCamY =
    birdBlockFocusCamStartY +
    (birdBlockFocusCamTargetY - birdBlockFocusCamStartY) * easeT;
}


// =====================================================
// BLOCK 初始化與座標
// 3x3 九宮格全部先生成空箱。
// 每次 reset 時，隨機挑 3 個不重複位置放豬。
// 可能結果範例：
// [P][0][0]
// [0][P][0]
// [0][0][P]
// =====================================================
static void resetBirdBlocks() {
  for (int i = 0; i < BIRD_BLOCK_TOTAL; i++) {
    birdBlockVisible[i] = true;
    birdBlockFrame[i] = BIRD_BLOCK_FRAME_EMPTY_BOX;
  }

  uint8_t pigsPlaced = 0;

  while (pigsPlaced < BIRD_PIG_START_COUNT) {
    int index = random(0, BIRD_BLOCK_TOTAL);

    if (birdBlockFrame[index] != BIRD_BLOCK_FRAME_EMPTY_BOX) {
      continue;
    }

    birdBlockFrame[index] = BIRD_BLOCK_FRAME_PIG_BOX;
    pigsPlaced++;
  }

  birdPigsRemaining = BIRD_PIG_START_COUNT;
}


static int birdBlockWorldX(int index) {
  int col = index % BIRD_BLOCK_COLS;
  return BIRD_TARGET_X + col * BIRD_BLOCK_FRAME_W;
}


static int birdBlockWorldY(int index) {
  int row = index / BIRD_BLOCK_COLS;
  return BIRD_TARGET_Y + row * BIRD_BLOCK_H;
}


// =====================================================
// BLOCK 繪製與閃爍
// =====================================================
static bool shouldDrawBlinkingBirdBlock(int index, unsigned long nowMs) {
  if (birdState != BIRD_STATE_IMPACT) {
    return true;
  }

  if (index != birdHitBlockIndex) {
    return true;
  }

  if (birdShotOutcome != BIRD_OUTCOME_HIT) {
    return true;
  }

  unsigned long elapsed = nowMs - birdStateStartMs;
  unsigned long blinkTotal =
    (unsigned long)BIRD_BLOCK_BLINK_COUNT *
    2UL *
    BIRD_BLOCK_BLINK_INTERVAL_MS;

  if (elapsed >= blinkTotal) {
    return true;
  }

  unsigned long phase = elapsed / BIRD_BLOCK_BLINK_INTERVAL_MS;

  return ((phase % 2UL) == 0UL);
}


static void drawBirdBlocks(unsigned long nowMs) {
  for (int i = 0; i < BIRD_BLOCK_TOTAL; i++) {
    if (!birdBlockVisible[i]) {
      continue;
    }

    if (!shouldDrawBlinkingBirdBlock(i, nowMs)) {
      continue;
    }

    int frame = birdBlockFrame[i];

    if (frame < 0 || frame >= BIRD_BLOCK_FRAME_COUNT) {
      frame = BIRD_BLOCK_FRAME_EMPTY_BOX;
    }

    drawBirdSpriteFrameWorld(
      birdBlockWorldX(i),
      birdBlockWorldY(i),
      BIRD_BLOCK_SHEET_W,
      BIRD_BLOCK_H,
      BIRD_BLOCK,
      frame * BIRD_BLOCK_FRAME_W,
      0,
      BIRD_BLOCK_FRAME_W,
      BIRD_BLOCK_H,
      true
    );
  }
}


// =====================================================
// 碰撞偵測
// =====================================================
static bool birdRectIntersect(
  float ax,
  float ay,
  int aw,
  int ah,
  float bx,
  float by,
  int bw,
  int bh
) {
  if (ax >= bx + bw) {
    return false;
  }

  if (ax + aw <= bx) {
    return false;
  }

  if (ay >= by + bh) {
    return false;
  }

  if (ay + ah <= by) {
    return false;
  }

  return true;
}


static int findFirstBirdBlockCollision(float heroX, float heroY) {
  int heroW = getBirdHeroW();
  int heroH = getBirdHeroH();

  int bestIndex = -1;
  int bestX = 9999;

  for (int i = 0; i < BIRD_BLOCK_TOTAL; i++) {
    if (!birdBlockVisible[i]) {
      continue;
    }

    int bx = birdBlockWorldX(i);
    int by = birdBlockWorldY(i);

    bool hit = birdRectIntersect(
      heroX,
      heroY,
      heroW,
      heroH,
      bx,
      by,
      BIRD_BLOCK_FRAME_W,
      BIRD_BLOCK_H
    );

    if (!hit) {
      continue;
    }

    if (bx < bestX) {
      bestX = bx;
      bestIndex = i;
    }
  }

  return bestIndex;
}


// =====================================================
// 彈弓
// =====================================================
static bool birdShouldDrawReadySlingshot() {
  return (
    birdState == BIRD_STATE_INIT_WAIT ||
    birdState == BIRD_STATE_WAIT_NEXT ||
    birdState == BIRD_STATE_RESTART_WAIT
  );
}


static void drawBirdReadySlingshot() {
  display.drawLine(
    BIRD_ROPE_1_X1,
    BIRD_ROPE_1_Y1,
    BIRD_ROPE_1_X2,
    BIRD_ROPE_1_Y2,
    BIRD_COLOR_ROPE
  );

  display.drawLine(
    BIRD_ROPE_2_X1,
    BIRD_ROPE_2_Y1,
    BIRD_ROPE_2_X2,
    BIRD_ROPE_2_Y2,
    BIRD_COLOR_ROPE
  );

  drawBirdSpriteScreen(
    BIRD_SHOT_X,
    BIRD_SHOT_Y,
    BIRD_SHOT_W,
    BIRD_SHOT_H,
    BIRD_SHOT,
    true
  );
}


// =====================================================
// 主角與爆炸
// =====================================================
static void drawBirdHero() {
  if (!birdHeroVisible) {
    return;
  }

  const uint16_t* sprite = getBirdHeroSprite();
  int heroW = getBirdHeroW();
  int heroH = getBirdHeroH();

  if (
    birdState == BIRD_STATE_INIT_WAIT ||
    birdState == BIRD_STATE_WAIT_NEXT ||
    birdState == BIRD_STATE_RESTART_WAIT
  ) {
    drawBirdSpriteScreen(
      BIRD_INIT_HERO_X,
      BIRD_INIT_HERO_Y,
      heroW,
      heroH,
      sprite,
      true
    );

    return;
  }

  drawBirdSpriteWorld(
    birdHeroX,
    birdHeroY,
    heroW,
    heroH,
    sprite,
    true
  );
}


static bool birdShouldDrawBoom(unsigned long nowMs) {
  if (birdState != BIRD_STATE_IMPACT) {
    return false;
  }

  unsigned long elapsed = nowMs - birdStateStartMs;
  unsigned long blinkTotal =
    (unsigned long)BIRD_BOM_BLINK_COUNT *
    2UL *
    BIRD_BOM_BLINK_INTERVAL_MS;

  if (elapsed >= blinkTotal) {
    return false;
  }

  unsigned long phase = elapsed / BIRD_BOM_BLINK_INTERVAL_MS;

  return ((phase % 2UL) == 0UL);
}


static void drawBirdBoom(unsigned long nowMs) {
  if (!birdShouldDrawBoom(nowMs)) {
    return;
  }

  drawBirdSpriteWorld(
    birdBoomX,
    birdBoomY,
    BIRD_BOM_W,
    BIRD_BOM_H,
    BIRD_BOM,
    true
  );
}


// =====================================================
// 目標選取
// 只用來決定拋物線大方向，不直接決定哪個 BLOCK 消失
// 真正受擊 BLOCK 由飛行途中第一個碰撞 BLOCK 決定
// =====================================================
static int chooseBirdAimBlock() {
  int pigIndexes[BIRD_BLOCK_TOTAL];
  int pigCount = 0;

  for (int i = 0; i < BIRD_BLOCK_TOTAL; i++) {
    if (!birdBlockVisible[i]) {
      continue;
    }

    if (
      birdBlockFrame[i] == BIRD_BLOCK_FRAME_PIG_BOX ||
      birdBlockFrame[i] == BIRD_BLOCK_FRAME_PIG
    ) {
      pigIndexes[pigCount] = i;
      pigCount++;
    }
  }

  if (pigCount > 0) {
    return pigIndexes[random(0, pigCount)];
  }

  int blockIndexes[BIRD_BLOCK_TOTAL];
  int blockCount = 0;

  for (int i = 0; i < BIRD_BLOCK_TOTAL; i++) {
    if (birdBlockVisible[i]) {
      blockIndexes[blockCount] = i;
      blockCount++;
    }
  }

  if (blockCount > 0) {
    return blockIndexes[random(0, blockCount)];
  }

  return -1;
}


// =====================================================
// BLOCK 傷害套用
// =====================================================
static void applyBirdBlockDamage() {
  if (birdBlockDamageApplied) {
    return;
  }

  birdBlockDamageApplied = true;

  if (birdShotOutcome != BIRD_OUTCOME_HIT) {
    return;
  }

  if (birdHitBlockIndex < 0 || birdHitBlockIndex >= BIRD_BLOCK_TOTAL) {
    return;
  }

  if (!birdBlockVisible[birdHitBlockIndex]) {
    return;
  }

  uint8_t frame = birdBlockFrame[birdHitBlockIndex];

  if (frame == BIRD_BLOCK_FRAME_PIG_BOX) {
    birdBlockFrame[birdHitBlockIndex] = BIRD_BLOCK_FRAME_PIG;
    return;
  }

  if (frame == BIRD_BLOCK_FRAME_PIG) {
    birdBlockVisible[birdHitBlockIndex] = false;
    birdBlockFrame[birdHitBlockIndex] = BIRD_BLOCK_FRAME_EMPTY_BOX;

    if (birdPigsRemaining > 0) {
      birdPigsRemaining--;
    }

    return;
  }

  birdBlockVisible[birdHitBlockIndex] = false;
}


// =====================================================
// 狀態切換
// =====================================================
static void setBirdState(uint8_t newState, unsigned long nowMs) {
  birdState = newState;
  birdStateStartMs = nowMs;

  birdClockVisible = true;

  if (newState == BIRD_STATE_INIT_WAIT) {
    birdCamX = 0.0f;
    birdCamY = 0.0f;

    birdHeroX = BIRD_INIT_HERO_X;
    birdHeroY = BIRD_INIT_HERO_Y;

    birdHeroVisible = true;
    birdClockVisible = true;

    selectRandomBirdHero();
  }

  if (newState == BIRD_STATE_FLY) {
    birdClockVisible = false;
    birdHeroVisible = true;
  }

  if (newState == BIRD_STATE_IMPACT) {
    birdClockVisible = true;
    birdHeroVisible = true;
    birdBlockDamageApplied = false;
  }

  if (newState == BIRD_STATE_HIDE_BIRD) {
    birdClockVisible = true;
    birdHeroVisible = false;
  }

  if (newState == BIRD_STATE_BLOCK_FOCUS) {
    birdClockVisible = true;
    birdHeroVisible = false;

    birdBlockFocusCamStartX = birdCamX;
    birdBlockFocusCamStartY = birdCamY;
    birdBlockFocusCamTargetX = getBirdBlockFocusCamX();
    birdBlockFocusCamTargetY = getBirdBlockFocusCamY();
  }

  if (newState == BIRD_STATE_CAM_RETURN) {
    birdClockVisible = true;
    birdHeroVisible = false;

    birdReturnCamStartX = birdCamX;
    birdReturnCamStartY = birdCamY;
  }

  if (newState == BIRD_STATE_WAIT_NEXT) {
    birdCamX = 0.0f;
    birdCamY = 0.0f;

    birdHeroX = BIRD_INIT_HERO_X;
    birdHeroY = BIRD_INIT_HERO_Y;

    birdHeroVisible = true;
    birdClockVisible = true;

    selectRandomBirdHero();
  }

  if (newState == BIRD_STATE_WIN_DELAY) {
    birdClockVisible = true;
    birdHeroVisible = true;
  }

  if (newState == BIRD_STATE_WIN) {
    birdClockVisible = false;
    birdWinScreenDrawn = false;
  }

  if (newState == BIRD_STATE_RESTART_WAIT) {
    birdCamX = 0.0f;
    birdCamY = 0.0f;

    birdHeroX = BIRD_INIT_HERO_X;
    birdHeroY = BIRD_INIT_HERO_Y;

    birdHeroVisible = true;
    birdClockVisible = true;
    birdWinScreenDrawn = false;

    resetBirdBlocks();
    selectRandomBirdHero();
  }
}


// =====================================================
// 開始撞擊
// blockIndex >= 0：撞到 BLOCK
// blockIndex < 0：碰地或沒撞到 BLOCK
//
// BIRD_BOM 顯示邏輯：
// 碰撞點 = BIRD_BOM 的繪圖中心點
// =====================================================
static void beginBirdImpact(unsigned long nowMs, int blockIndex) {
  int heroW = getBirdHeroW();
  int heroH = getBirdHeroH();

  birdHitBlockIndex = blockIndex;

  if (blockIndex >= 0) {
    birdShotOutcome = BIRD_OUTCOME_HIT;

    int bx = birdBlockWorldX(blockIndex);
    int by = birdBlockWorldY(blockIndex);

    // 鳥由左往右飛，碰撞點 X 抓 BLOCK 左側。
    // 如果覺得爆炸太靠左，可把 + 0.0f 改成 + 2.0f 或 + 3.0f。
    birdImpactX = (float)bx + 0.0f;

    // 碰撞點 Y 抓「鳥與 BLOCK 重疊區域」的中心。
    // 比單純抓鳥中心或 BLOCK 中心更準。
    float overlapTop = birdMaxFloat(birdHeroY, (float)by);
    float overlapBottom = birdMinFloat(
      birdHeroY + (float)heroH,
      (float)by + (float)BIRD_BLOCK_H
    );

    if (overlapBottom > overlapTop) {
      birdImpactY = (overlapTop + overlapBottom) * 0.50f;
    }
    else {
      birdImpactY = (float)by + ((float)BIRD_BLOCK_H * 0.50f);
    }
  }
  else {
    // 沒撞到 BLOCK 時，碰撞點抓鳥底部中央，也就是落地點。
    birdImpactX = birdHeroX + ((float)heroW * 0.50f);
    birdImpactY = birdHeroY + (float)heroH;
  }

  // 讓 BIRD_BOM 的中心點對準碰撞點。
  birdBoomX =
    birdImpactX -
    ((float)BIRD_BOM_W * 0.50f);

  birdBoomY =
    birdImpactY -
    ((float)BIRD_BOM_H * 0.50f);

  birdBoomY = birdClampFloat(
    birdBoomY,
    birdCamY,
    birdCamY + (float)(BIRD_SCR_H - BIRD_BOM_H)
  );

  updateBirdCameraForHero(false);

  setBirdState(BIRD_STATE_IMPACT, nowMs);
}


// =====================================================
// 準備一次彈射
// 不再把主角瞬移到 X=24。
// 主角會從 init 畫面看到的 BIRD_INIT_HERO_X / Y 實際起飛。
// =====================================================
static void prepareBirdShot(unsigned long nowMs) {
  if (birdPigsRemaining == 0) {
    setBirdState(BIRD_STATE_WIN_DELAY, nowMs);
    return;
  }

  int heroW = getBirdHeroW();
  int heroH = getBirdHeroH();

  birdHeroX = (float)BIRD_INIT_HERO_X;
  birdHeroY = (float)BIRD_INIT_HERO_Y;

  birdFlightStartX = birdHeroX;
  birdFlightStartY = birdHeroY;

  birdHitBlockIndex = -1;
  birdBlockDamageApplied = false;

  int totalChance =
    BIRD_CHANCE_BEFORE_BLOCK +
    BIRD_CHANCE_HIT_BLOCK +
    BIRD_CHANCE_AFTER_BLOCK;

  if (totalChance <= 0) {
    totalChance = 1;
  }

  int roll = random(0, totalChance);

  if (roll < BIRD_CHANCE_BEFORE_BLOCK) {
    birdShotOutcome = BIRD_OUTCOME_BEFORE;
  }
  else if (roll < BIRD_CHANCE_BEFORE_BLOCK + BIRD_CHANCE_HIT_BLOCK) {
    birdShotOutcome = BIRD_OUTCOME_HIT;
  }
  else {
    birdShotOutcome = BIRD_OUTCOME_AFTER;
  }

  int aimBlockIndex = -1;

  if (birdShotOutcome == BIRD_OUTCOME_HIT) {
    aimBlockIndex = chooseBirdAimBlock();

    if (aimBlockIndex < 0) {
      birdShotOutcome = BIRD_OUTCOME_AFTER;
    }
  }

  if (birdShotOutcome == BIRD_OUTCOME_HIT) {
    int blockX = birdBlockWorldX(aimBlockIndex);
    int blockY = birdBlockWorldY(aimBlockIndex);

    birdFlightTargetX =
      (float)blockX +
      ((float)BIRD_BLOCK_FRAME_W * 0.50f) -
      ((float)heroW * 0.50f);

    birdFlightTargetY =
      (float)blockY +
      ((float)BIRD_BLOCK_H * 0.50f) -
      ((float)heroH * 0.50f);
  }
  else if (birdShotOutcome == BIRD_OUTCOME_BEFORE) {
    birdFlightTargetX = (float)(BIRD_TARGET_X - random(16, 34));
    birdFlightTargetY = (float)(BIRD_GROUND_Y - heroH);
  }
  else {
    birdFlightTargetX =
      (float)(BIRD_TARGET_X + BIRD_BLOCK_COLS * BIRD_BLOCK_FRAME_W + random(48, 80));

    birdFlightTargetY = (float)(BIRD_GROUND_Y - heroH);
  }

  birdFlightCtrlX = (birdFlightStartX + birdFlightTargetX) * 0.50f;

  int arcHigh = random(BIRD_ARC_HIGH_MIN, BIRD_ARC_HIGH_MAX + 1);

  birdFlightCtrlY =
    ((birdFlightStartY < birdFlightTargetY) ? birdFlightStartY : birdFlightTargetY) -
    (float)arcHigh;

  if (birdShotOutcome == BIRD_OUTCOME_AFTER) {
    birdFlightCtrlY -= (float)BIRD_AFTER_EXTRA_ARC;
  }

  float distance = birdFlightTargetX - birdFlightStartX;

  if (distance < 0.0f) {
    distance = 0.0f;
  }

  birdFlightDurationMs =
    BIRD_FLIGHT_BASE_MS +
    (unsigned long)(distance * BIRD_FLIGHT_DISTANCE_MS);

  if (birdFlightDurationMs > BIRD_FLIGHT_MAX_MS) {
    birdFlightDurationMs = BIRD_FLIGHT_MAX_MS;
  }

  birdFlightStartMs = nowMs;

  birdCamX = 0.0f;
  birdCamY = 0.0f;

  setBirdState(BIRD_STATE_FLY, nowMs);
}


// =====================================================
// 更新飛行
// 使用二次 Bezier 拋物線
// 每幀沿線段取樣，避免主角高速穿透 BLOCK
// =====================================================
static void updateBirdFlight(unsigned long nowMs) {
  unsigned long elapsed = nowMs - birdFlightStartMs;

  float t = (float)elapsed / (float)birdFlightDurationMs;

  if (t > 1.0f) {
    t = 1.0f;
  }

  if (t < 0.0f) {
    t = 0.0f;
  }

  float u = 1.0f - t;

  float nextX =
    (u * u * birdFlightStartX) +
    (2.0f * u * t * birdFlightCtrlX) +
    (t * t * birdFlightTargetX);

  float nextY =
    (u * u * birdFlightStartY) +
    (2.0f * u * t * birdFlightCtrlY) +
    (t * t * birdFlightTargetY);

  float prevX = birdHeroX;
  float prevY = birdHeroY;

  float dx = nextX - prevX;
  float dy = nextY - prevY;

  float maxDelta = birdAbsFloat(dx);

  if (birdAbsFloat(dy) > maxDelta) {
    maxDelta = birdAbsFloat(dy);
  }

  int sampleCount = (int)(maxDelta / (float)BIRD_COLLISION_SAMPLE_PX) + 1;

  if (sampleCount < 1) {
    sampleCount = 1;
  }

  int heroH = getBirdHeroH();

  for (int s = 1; s <= sampleCount; s++) {
    float k = (float)s / (float)sampleCount;

    float sampleX = prevX + dx * k;
    float sampleY = prevY + dy * k;

    int hitBlock = findFirstBirdBlockCollision(sampleX, sampleY);

    if (hitBlock >= 0) {
      birdHeroX = sampleX;
      birdHeroY = sampleY;

      updateBirdCameraForHero(false);
      beginBirdImpact(nowMs, hitBlock);
      return;
    }

    if (sampleY + (float)heroH >= (float)BIRD_GROUND_Y) {
      birdHeroX = sampleX;
      birdHeroY = (float)(BIRD_GROUND_Y - heroH);

      updateBirdCameraForHero(false);
      beginBirdImpact(nowMs, -1);
      return;
    }
  }

  birdHeroX = nextX;
  birdHeroY = nextY;

  updateBirdCameraForHero(false);

  if (t >= 1.0f) {
    beginBirdImpact(nowMs, -1);
  }
}


// =====================================================
// 更新撞擊
// =====================================================
static void updateBirdImpact(unsigned long nowMs) {
  unsigned long elapsed = nowMs - birdStateStartMs;

  unsigned long boomTotal =
    (unsigned long)BIRD_BOM_BLINK_COUNT *
    2UL *
    BIRD_BOM_BLINK_INTERVAL_MS;

  unsigned long blockTotal =
    (unsigned long)BIRD_BLOCK_BLINK_COUNT *
    2UL *
    BIRD_BLOCK_BLINK_INTERVAL_MS;

  unsigned long total = boomTotal;

  if (birdShotOutcome == BIRD_OUTCOME_HIT) {
    total = blockTotal;

    if (boomTotal > total) {
      total = boomTotal;
    }

    if (!birdBlockDamageApplied && elapsed >= blockTotal) {
      applyBirdBlockDamage();
    }
  }

  if (elapsed >= total) {
    if (birdShotOutcome == BIRD_OUTCOME_HIT && !birdBlockDamageApplied) {
      applyBirdBlockDamage();
    }

    if (birdPigsRemaining == 0) {
      setBirdState(BIRD_STATE_WIN_DELAY, nowMs);
      return;
    }

    setBirdState(BIRD_STATE_POST_HIT_HOLD, nowMs);
  }
}


// =====================================================
// 更新狀態機
// =====================================================
static void updateBirdState(unsigned long nowMs) {
  unsigned long elapsed = nowMs - birdStateStartMs;

  birdLastUpdateMs = nowMs;

  switch (birdState) {
    case BIRD_STATE_INIT_WAIT: {
      if (elapsed >= BIRD_FIRST_SHOT_WAIT_MS) {
        prepareBirdShot(nowMs);
      }

      break;
    }

    case BIRD_STATE_FLY: {
      updateBirdFlight(nowMs);
      break;
    }

    case BIRD_STATE_IMPACT: {
      updateBirdImpact(nowMs);
      break;
    }

    case BIRD_STATE_POST_HIT_HOLD: {
      if (elapsed >= BIRD_POST_HIT_HOLD_MS) {
        setBirdState(BIRD_STATE_HIDE_BIRD, nowMs);
      }

      break;
    }

    case BIRD_STATE_HIDE_BIRD: {
      if (elapsed >= BIRD_HIDE_HOLD_MS) {
        setBirdState(BIRD_STATE_BLOCK_FOCUS, nowMs);
      }

      break;
    }

    case BIRD_STATE_BLOCK_FOCUS: {
      updateBirdBlockFocusCamera(elapsed);

      if (elapsed >= BIRD_BLOCK_FOCUS_MOVE_MS) {
        birdCamX = birdBlockFocusCamTargetX;
        birdCamY = birdBlockFocusCamTargetY;

        unsigned long focusHoldElapsed = elapsed - BIRD_BLOCK_FOCUS_MOVE_MS;

        if (focusHoldElapsed >= BIRD_BLOCK_FOCUS_HOLD_MS) {
          setBirdState(BIRD_STATE_CAM_RETURN, nowMs);
          return;
        }
      }

      break;
    }

    case BIRD_STATE_CAM_RETURN: {
      float t = (float)elapsed / (float)BIRD_CAM_RETURN_MS;
      t = birdClampFloat(t, 0.0f, 1.0f);

      // smoothstep 緩動：
      // 起點慢、中間快、終點慢。
      // 修正 CAM 回 init 最後一小段突然跳回 0 的瞬移感。
      float easeT = t * t * (3.0f - 2.0f * t);

      birdCamX = birdReturnCamStartX * (1.0f - easeT);
      birdCamY = birdReturnCamStartY * (1.0f - easeT);

      if (elapsed >= BIRD_CAM_RETURN_MS) {
        birdCamX = 0.0f;
        birdCamY = 0.0f;

        setBirdState(BIRD_STATE_WAIT_NEXT, nowMs);
        return;
      }

      break;
    }

    case BIRD_STATE_WAIT_NEXT: {
      if (elapsed >= BIRD_NEXT_SHOT_WAIT_MS) {
        prepareBirdShot(nowMs);
      }

      break;
    }

    case BIRD_STATE_WIN_DELAY: {
      if (elapsed >= BIRD_WIN_DELAY_MS) {
        setBirdState(BIRD_STATE_WIN, nowMs);
      }

      break;
    }

    case BIRD_STATE_WIN: {
      if (elapsed >= BIRD_WIN_HOLD_MS) {
        ClearAll();
        setBirdState(BIRD_STATE_RESTART_WAIT, nowMs);
      }

      break;
    }

    case BIRD_STATE_RESTART_WAIT: {
      if (elapsed >= BIRD_RESTART_WAIT_MS) {
        prepareBirdShot(nowMs);
      }

      break;
    }

    default: {
      setBirdState(BIRD_STATE_INIT_WAIT, nowMs);
      break;
    }
  }
}


// =====================================================
// WIN 畫面
// =====================================================
static void renderBirdWinScene() {
  if (birdWinScreenDrawn) {
    return;
  }

  ClearAll();

  display.setTextWrap(false);

  display.setTextSize(2);
  display.setTextColor(BIRD_COLOR_BLACK);
  display.setCursor(16, 22);
  display.print("WIN");

  display.setTextColor(BIRD_COLOR_RED);
  display.setCursor(15, 21);
  display.print("WIN");

  display.setTextSize(1);
  display.setTextColor(BIRD_COLOR_BLACK);
  display.setCursor(12, 47);
  display.print("YOU WIN");

  display.setTextColor(BIRD_COLOR_WHITE);
  display.setCursor(11, 46);
  display.print("YOU WIN");

  birdWinScreenDrawn = true;
}


// =====================================================
// 繪製整體場景
// =====================================================
static void renderBirdScene(unsigned long nowMs) {
  if (birdState == BIRD_STATE_WIN) {
    renderBirdWinScene();
    return;
  }

  drawBirdBackground();

  drawBirdBlocks(nowMs);

  if (birdShouldDrawReadySlingshot()) {
    drawBirdReadySlingshot();
  }

  drawBirdHero();

  drawBirdBoom(nowMs);

 // if (birdClockVisible) {
    drawThemeClockText();
  //}
}


// =====================================================
// 初始化
// =====================================================
static void BirdModeInit() {
  if (!ModefirstRun) {
    return;
  }

  randomSeed(millis());

  birdLastUpdateMs = millis();

  resetBirdBlocks();

  setBirdState(BIRD_STATE_INIT_WAIT, millis());

  ModefirstRun = false;
}


// =====================================================
// 主函式
// =====================================================
void BirdMode() {
  BirdModeInit();

  unsigned long nowMs = millis();

  updateBirdState(nowMs);
  renderBirdScene(nowMs);

  wait_with_display(BIRD_FRAME_DELAY_MS);
}
