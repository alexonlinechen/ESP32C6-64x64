#include "Space.h"

// =====================================================
// Mario Space Mode
// 火箭升空 -> 外太空 -> Mario / Luigi 出艙漂浮 -> 吃星星 -> 勝利
// =====================================================


// =====================================================
// 螢幕大小
// =====================================================
static const int SPACE_SCR_W = 64;
static const int SPACE_SCR_H = 64;


// =====================================================
// 透明色
// BMP 藍色背景 RGB(0,0,255) 轉 RGB565 = 0x001F
// 所有 sprite 繪製時都會跳過這個顏色
// =====================================================
static const uint16_t SPACE_TRANSPARENT = 0x001F;


// =====================================================
// 圖資尺寸
// =====================================================
static const int SPACE_BG_W = 16;
static const int SPACE_BG_H = 320;

static const int SPACE_ROCKET_W = 64;
static const int SPACE_ROCKET_H = 48;

static const int SPACE_FLAME_SHEET_W = 128;
static const int SPACE_FLAME_H = 16;
static const int SPACE_FLAME_FRAME_W = 64;
static const int SPACE_FLAME_FRAME_COUNT = 2;

static const int SPACE_ICON_SHEET_W = 24;
static const int SPACE_ICON_H = 12;
static const int SPACE_ICON_FRAME_W = 12;
static const int SPACE_ICON_FRAME_COUNT = 2;

static const int SPACE_HERO_SHEET_W = 54;
static const int SPACE_HERO_H = 29;
static const int SPACE_HERO_FRAME_W = 18;
static const int SPACE_HERO_FRAME_COUNT = 3;

static const int SPACE_STAR_SHEET_W = 16;
static const int SPACE_STAR_H = 8;
static const int SPACE_STAR_FRAME_W = 8;
static const int SPACE_STAR_FRAME_COUNT = 2;

static const int SPACE_WIN_SHEET_W = 48;
static const int SPACE_WIN_H = 24;
static const int SPACE_WIN_FRAME_W = 24;
static const int SPACE_WIN_FRAME_COUNT = 2;


// =====================================================
// 可調參數
// =====================================================

// 一開始停留時間
static const unsigned long SPACE_PARAM_A_MS = 5000UL;

// 外太空黑場停留時間
static const unsigned long SPACE_PARAM_B_MS = 3000UL;

// 主迴圈刷新速度
static const unsigned long SPACE_FRAME_DELAY_MS = 30UL;

// ICON 閃爍設定
static const uint8_t SPACE_ICON_BLINK_COUNT = 3;
static const unsigned long SPACE_ICON_BLINK_INTERVAL_MS = 500UL;

// 倒數時間：5、4、3、2、1、GO，共 6 秒
static const unsigned long SPACE_COUNTDOWN_MS = 6000UL;

// 火焰預備時間
static const unsigned long SPACE_PRELAUNCH_FLAME_MS = 1000UL;

// 升空總時間：背景從 source y=256 捲到 source y=0
static const unsigned long SPACE_TAKEOFF_TOTAL_MS = 24000UL;

// 背景到達最頂端後，整張背景往下滑出畫面，黑色從上方接續進來
static const unsigned long SPACE_BG_SLIDE_OUT_MS = 2500UL;

// 火箭下降回畫面時間
static const unsigned long SPACE_ROCKET_DESCEND_MS = 4000UL;

// 主角出艙前待機時間
static const unsigned long SPACE_HERO_IDLE_MS = 3000UL;

// 勝利畫面停留時間
static const unsigned long SPACE_WIN_HOLD_MS = 10000UL;

// 勝利後回到 init 畫面，等待多久再重新開始
static const unsigned long SPACE_RESTART_WAIT_MS = 300000UL;

// 星星產生間隔
static const unsigned long SPACE_STAR_SPAWN_MIN_MS = 10000UL;
static const unsigned long SPACE_STAR_SPAWN_MAX_MS = 15000UL;

// 動畫 frame 間隔
static const unsigned long SPACE_FLAME_FRAME_INTERVAL_MS = 100UL;
static const unsigned long SPACE_STAR_FRAME_INTERVAL_MS = 120UL;
static const unsigned long SPACE_EAT_FRAME_INTERVAL_MS = 120UL;

// 主角漂浮速度，數字越大越快
static const float SPACE_HERO_SPEED_X = 5.0f;
static const float SPACE_HERO_SPEED_Y = 3.0f;

// 火箭退出速度
static const float SPACE_ROCKET_EXIT_SPEED = 7.0f;

// 星星穿越速度
static const float SPACE_STAR_SPEED = 25.0f;

// 吃幾顆星星後勝利
static const uint8_t SPACE_STAR_WIN_COUNT = 10;

// 最後一顆星星吃完動畫後，停留多久才進入 WIN
static const unsigned long SPACE_FINAL_EAT_AFTER_DELAY_MS = 1500UL;


// =====================================================
// 星空閃爍背景設定
// SPACE_BG 完全滑出、黑色填滿 64x64 後才會開始顯示
// =====================================================
static const uint8_t SPACE_TWINKLE_STAR_COUNT = 26;
static const unsigned long SPACE_TWINKLE_INTERVAL_MS = 140UL;

static const uint16_t SPACE_COLOR_BLACK = 0x0000;
static const uint16_t SPACE_COLOR_STAR_WHITE = 0xFFFF;
static const uint16_t SPACE_COLOR_STAR_GRAY = 0x8410;


// =====================================================
// 角色定義
// =====================================================
static const uint8_t SPACE_HERO_MARIO = 0;
static const uint8_t SPACE_HERO_LUIGI = 1;

static uint8_t spaceHero = SPACE_HERO_MARIO;


// =====================================================
// 狀態機
// =====================================================
static const uint8_t SPACE_STATE_WAIT_START        = 0;
static const uint8_t SPACE_STATE_ICON_FLASH        = 1;
static const uint8_t SPACE_STATE_COUNTDOWN         = 2;
static const uint8_t SPACE_STATE_PRELAUNCH_FLAME   = 3;
static const uint8_t SPACE_STATE_TAKEOFF           = 4;
static const uint8_t SPACE_STATE_BG_SLIDE_OUT      = 5;
static const uint8_t SPACE_STATE_SPACE_HOLD_BLACK  = 6;
static const uint8_t SPACE_STATE_ROCKET_DESCEND    = 7;
static const uint8_t SPACE_STATE_SPACE_HOLD_CLOCK  = 8;
static const uint8_t SPACE_STATE_HERO_IDLE         = 9;
static const uint8_t SPACE_STATE_ROAM              = 10;
static const uint8_t SPACE_STATE_FINAL_EAT         = 11;
static const uint8_t SPACE_STATE_WIN               = 12;
static const uint8_t SPACE_STATE_RESTART_WAIT      = 13;

static uint8_t spaceState = SPACE_STATE_WAIT_START;
static unsigned long spaceStateStartMs = 0;


// =====================================================
// 背景與物件狀態
// =====================================================

// SPACE_BG 取樣起點
// 256 = 地面 / 發射場
// 0   = 最上方外太空
static float spaceBgY = 256.0f;

// SPACE_BG 到達 y=0 後，這個值控制它往下滑出畫面
// 0  = 剛開始滑出
// 64 = 完全滑出，畫面全黑
static float spaceBgSlideOffsetY = 0.0f;

// 火箭座標
static float spaceRocketX = 0.0f;
static float spaceRocketY = 16.0f;

// 火焰動畫
static uint8_t spaceFlameFrame = 0;
static unsigned long spaceFlameLastFrameMs = 0;

// 時鐘顯示控制
static bool spaceClockVisible = true;


// =====================================================
// 星空閃爍背景狀態
// =====================================================
static uint8_t spaceTwinkleX[SPACE_TWINKLE_STAR_COUNT];
static uint8_t spaceTwinkleY[SPACE_TWINKLE_STAR_COUNT];
static uint8_t spaceTwinkleState[SPACE_TWINKLE_STAR_COUNT];

static bool spaceTwinkleInited = false;
static unsigned long spaceTwinkleLastMs = 0;


// =====================================================
// 主角狀態
// =====================================================
static float spaceHeroX = 23.0f;
static float spaceHeroY = 29.0f;
static float spaceHeroVX = SPACE_HERO_SPEED_X;
static float spaceHeroVY = -SPACE_HERO_SPEED_Y;

static uint8_t spaceHeroFrame = 0;
static bool spaceHeroFlipH = false;

static bool spaceHeroEating = false;
static unsigned long spaceHeroEatStartMs = 0;
static unsigned long spaceHeroEatLastFrameMs = 0;
static uint8_t spaceHeroEatStep = 0;


// =====================================================
// 星星狀態
// =====================================================
static bool spaceStarActive = false;
static float spaceStarX = 0.0f;
static float spaceStarY = 0.0f;
static float spaceStarVX = 0.0f;
static float spaceStarVY = 0.0f;

static uint8_t spaceStarFrame = 0;
static unsigned long spaceStarLastFrameMs = 0;
static unsigned long spaceNextStarSpawnMs = 0;

static uint8_t spaceStarEatCount = 0;


// =====================================================
// 勝利前後狀態
// =====================================================
static unsigned long spaceFinalEatDoneMs = 0;
static bool spaceWinScreenDrawn = false;


// =====================================================
// 時間控制
// =====================================================
static unsigned long spaceLastUpdateMs = 0;


// =====================================================
// 繪製 RGB565 sprite
// 支援：透明色、frame、水平翻轉
// =====================================================
static void drawSpaceSpriteFrame(
  int x,
  int y,
  int sheetW,
  int sheetH,
  const uint16_t* sprite,
  int srcX,
  int srcY,
  int frameW,
  int frameH,
  bool flipH,
  bool useTransparent
) {
  if (!sprite) return;

  for (int j = 0; j < frameH; j++) {
    int dy = y + j;
    if (dy < 0 || dy >= SPACE_SCR_H) continue;

    int sy = srcY + j;
    if (sy < 0 || sy >= sheetH) continue;

    for (int i = 0; i < frameW; i++) {
      int dx = x + i;
      if (dx < 0 || dx >= SPACE_SCR_W) continue;

      int sx;
      if (flipH) {
        sx = srcX + frameW - 1 - i;
      } else {
        sx = srcX + i;
      }

      if (sx < 0 || sx >= sheetW) continue;

      uint32_t pos =
        (uint32_t)sy * (uint32_t)sheetW +
        (uint32_t)sx;

      uint16_t color = pgm_read_word(&(sprite[pos]));

      if (useTransparent && color == SPACE_TRANSPARENT) {
        continue;
      }

      display.drawPixel(dx, dy, color);
    }
  }
}


// =====================================================
// 繪製單張 sprite
// =====================================================
static void drawSpaceSprite(
  int x,
  int y,
  int w,
  int h,
  const uint16_t* sprite,
  bool useTransparent
) {
  drawSpaceSpriteFrame(
    x,
    y,
    w,
    h,
    sprite,
    0,
    0,
    w,
    h,
    false,
    useTransparent
  );
}


// =====================================================
// 繪製 SPACE_BG 背景
// SPACE_BG 是 16x320，X 方向重複 4 次填滿 64 寬
// =====================================================
static void drawSpaceBackground() {
  int baseY = (int)spaceBgY;

  if (baseY < 0) {
    baseY = 0;
  }

  if (baseY > SPACE_BG_H - SPACE_SCR_H) {
    baseY = SPACE_BG_H - SPACE_SCR_H;
  }

  for (int y = 0; y < SPACE_SCR_H; y++) {
    int srcY = baseY + y;

    if (srcY < 0) {
      srcY = 0;
    }

    if (srcY >= SPACE_BG_H) {
      srcY = SPACE_BG_H - 1;
    }

    for (int x = 0; x < SPACE_SCR_W; x++) {
      int srcX = x % SPACE_BG_W;

      uint32_t pos =
        (uint32_t)srcY * (uint32_t)SPACE_BG_W +
        (uint32_t)srcX;

      uint16_t color = pgm_read_word(&(SPACE_BG[pos]));
      display.drawPixel(x, y, color);
    }
  }
}


// =====================================================
// 繪製 SPACE_BG 滑出黑場
//
// 目的：
// SPACE_BG 播放到最頂端 y=0 後，不要突然切成黑底星空。
// 而是讓 source y=0~63 這張畫面繼續往下滑，
// 上方用黑色補進來，直到整個 64x64 都變黑。
// =====================================================
static void drawSpaceBackgroundSlideOut() {
  int offsetY = (int)spaceBgSlideOffsetY;

  // 先鋪黑色，讓背景滑出後露出的地方都是黑色
  display.fillScreen(SPACE_COLOR_BLACK);

  // 固定取 SPACE_BG 最頂端 y=0~63
  // 然後整張往下移動 offsetY
  for (int y = 0; y < SPACE_SCR_H; y++) {
    int dstY = y + offsetY;

    if (dstY < 0 || dstY >= SPACE_SCR_H) {
      continue;
    }

    int srcY = y;

    for (int x = 0; x < SPACE_SCR_W; x++) {
      int srcX = x % SPACE_BG_W;

      uint32_t pos =
        (uint32_t)srcY * (uint32_t)SPACE_BG_W +
        (uint32_t)srcX;

      uint16_t color = pgm_read_word(&(SPACE_BG[pos]));
      display.drawPixel(x, dstY, color);
    }
  }
}


// =====================================================
// 初始化星空閃爍背景
// =====================================================
static void initSpaceTwinkleBackground(unsigned long nowMs) {
  for (int i = 0; i < SPACE_TWINKLE_STAR_COUNT; i++) {
    spaceTwinkleX[i] = random(0, SPACE_SCR_W);
    spaceTwinkleY[i] = random(0, SPACE_SCR_H);

    // 0 = 不亮，1 = 灰白，2 = 純白
    spaceTwinkleState[i] = random(0, 3);
  }

  spaceTwinkleInited = true;
  spaceTwinkleLastMs = nowMs;
}


// =====================================================
// 更新星星閃爍狀態
// =====================================================
static void updateSpaceTwinkleBackground(unsigned long nowMs) {
  if (!spaceTwinkleInited) {
    initSpaceTwinkleBackground(nowMs);
  }

  if (nowMs - spaceTwinkleLastMs < SPACE_TWINKLE_INTERVAL_MS) {
    return;
  }

  spaceTwinkleLastMs = nowMs;

  for (int i = 0; i < SPACE_TWINKLE_STAR_COUNT; i++) {
    // 不是每顆每次都變，避免畫面太亂
    if (random(0, 100) < 35) {
      spaceTwinkleState[i] = random(0, 3);
    }

    // 偶爾讓少數星星換位置，增加一點生命感
    if (random(0, 100) < 4) {
      spaceTwinkleX[i] = random(0, SPACE_SCR_W);
      spaceTwinkleY[i] = random(0, SPACE_SCR_H);
    }
  }
}


// =====================================================
// 繪製全黑 + 隨機白點閃爍背景
// 注意：這個背景只會在 SPACE_BG 完全滑出後才啟用
// =====================================================
static void drawSpaceTwinkleBackground(unsigned long nowMs) {
  updateSpaceTwinkleBackground(nowMs);

  display.fillScreen(SPACE_COLOR_BLACK);

  for (int i = 0; i < SPACE_TWINKLE_STAR_COUNT; i++) {
    if (spaceTwinkleState[i] == 0) {
      continue;
    }

    uint16_t color =
      (spaceTwinkleState[i] == 2)
      ? SPACE_COLOR_STAR_WHITE
      : SPACE_COLOR_STAR_GRAY;

    display.drawPixel(
      spaceTwinkleX[i],
      spaceTwinkleY[i],
      color
    );
  }
}


// =====================================================
// 判斷是否使用星空閃爍背景
// BG_SLIDE_OUT 不在這裡，因為它要先滑出黑場
// =====================================================
static bool spaceShouldUseTwinkleBackground() {
  return (
    spaceState == SPACE_STATE_SPACE_HOLD_BLACK ||
    spaceState == SPACE_STATE_ROCKET_DESCEND ||
    spaceState == SPACE_STATE_SPACE_HOLD_CLOCK ||
    spaceState == SPACE_STATE_HERO_IDLE ||
    spaceState == SPACE_STATE_ROAM ||
    spaceState == SPACE_STATE_FINAL_EAT
  );
}


// =====================================================
// 更新火焰 frame
// =====================================================
static void updateSpaceFlameFrame(unsigned long nowMs) {
  if (nowMs - spaceFlameLastFrameMs >= SPACE_FLAME_FRAME_INTERVAL_MS) {
    spaceFlameLastFrameMs = nowMs;
    spaceFlameFrame++;

    if (spaceFlameFrame >= SPACE_FLAME_FRAME_COUNT) {
      spaceFlameFrame = 0;
    }
  }
}


// =====================================================
// 繪製火箭
// =====================================================
static void drawSpaceRocket() {
  drawSpaceSprite(
    (int)spaceRocketX,
    (int)spaceRocketY,
    SPACE_ROCKET_W,
    SPACE_ROCKET_H,
    SPACE_ROCKET,
    true
  );
}


// =====================================================
// 繪製火焰
// 火焰接在火箭後方，也就是 rocketY + rocketH
// =====================================================
static void drawSpaceFlame() {
  int flameY = (int)spaceRocketY + SPACE_ROCKET_H;

  // 火焰完全超出螢幕就不畫
  if (flameY >= SPACE_SCR_H) {
    return;
  }

  drawSpaceSpriteFrame(
    (int)spaceRocketX,
    flameY,
    SPACE_FLAME_SHEET_W,
    SPACE_FLAME_H,
    SPACE_FLAME,
    spaceFlameFrame * SPACE_FLAME_FRAME_W,
    0,
    SPACE_FLAME_FRAME_W,
    SPACE_FLAME_H,
    false,
    true
  );
}


// =====================================================
// 繪製角色 ICON
// SPACE_ICON 24x12
// Mario = 左邊 12x12
// Luigi = 右邊 12x12
// =====================================================
static void drawSpaceIcon() {
  int srcX = spaceHero * SPACE_ICON_FRAME_W;

  drawSpaceSpriteFrame(
    26,
    30,
    SPACE_ICON_SHEET_W,
    SPACE_ICON_H,
    SPACE_ICON,
    srcX,
    0,
    SPACE_ICON_FRAME_W,
    SPACE_ICON_H,
    false,
    true
  );
}


// =====================================================
// 繪製倒數文字
// 0~1 秒：5
// 1~2 秒：4
// 2~3 秒：3
// 3~4 秒：2
// 4~5 秒：1
// 5~6 秒：GO
// =====================================================
static void drawSpaceCountdownText() {
  unsigned long elapsed = millis() - spaceStateStartMs;
  int step = elapsed / 1000;

  display.setTextWrap(false);
  display.setTextSize(2);

  int textY = 25;

  if (step <= 4) {
    int number = 5 - step;
    int textX = 28;

    display.setTextColor(0x0000);
    display.setCursor(textX + 1, textY + 1);
    display.print(number);

    display.setTextColor(0xF800);
    display.setCursor(textX, textY);
    display.print(number);
  }
  else if (step == 5) {
    int textX = 18;

    display.setTextColor(0x0000);
    display.setCursor(textX + 1, textY + 1);
    display.print("GO");

    display.setTextColor(0xF800);
    display.setCursor(textX, textY);
    display.print("GO");
  }
}


// =====================================================
// 取得目前主角圖資
// SPACE_M / SPACE_L：54x29
// 單 frame：18x29
// frame0 = 待機
// frame1 = 右移動
// frame2 = 吃星星動作
// =====================================================
static const uint16_t* getSpaceHeroSprite() {
  if (spaceHero == SPACE_HERO_LUIGI) {
    return SPACE_L;
  }

  return SPACE_M;
}


// =====================================================
// 繪製主角
// 左移時使用 frame1 水平翻轉
// =====================================================
static void drawSpaceHero() {
  const uint16_t* heroSprite = getSpaceHeroSprite();

  drawSpaceSpriteFrame(
    (int)spaceHeroX,
    (int)spaceHeroY,
    SPACE_HERO_SHEET_W,
    SPACE_HERO_H,
    heroSprite,
    spaceHeroFrame * SPACE_HERO_FRAME_W,
    0,
    SPACE_HERO_FRAME_W,
    SPACE_HERO_H,
    spaceHeroFlipH,
    true
  );
}


// =====================================================
// 開始吃星星動畫
// =====================================================
static void startSpaceHeroEat(unsigned long nowMs) {
  spaceHeroEating = true;
  spaceHeroEatStartMs = nowMs;
  spaceHeroEatLastFrameMs = nowMs;
  spaceHeroEatStep = 0;
  spaceHeroFrame = 1;
}


// =====================================================
// 更新吃星星動畫
// frame1 / frame2 交替 3 次
// =====================================================
static void updateSpaceHeroEat(unsigned long nowMs) {
  if (!spaceHeroEating) {
    return;
  }

  if (nowMs - spaceHeroEatLastFrameMs >= SPACE_EAT_FRAME_INTERVAL_MS) {
    spaceHeroEatLastFrameMs = nowMs;
    spaceHeroEatStep++;

    if ((spaceHeroEatStep % 2) == 0) {
      spaceHeroFrame = 1;
    } else {
      spaceHeroFrame = 2;
    }

    // frame1 / frame2 循環 3 次 = 6 步
    if (spaceHeroEatStep >= 6) {
      spaceHeroEating = false;

      if (spaceHeroVX >= 0) {
        spaceHeroFrame = 1;
        spaceHeroFlipH = false;
      } else {
        spaceHeroFrame = 1;
        spaceHeroFlipH = true;
      }
    }
  }
}


// =====================================================
// 繪製星星
// SPACE_STAR 32x16
// frame0 / frame1 閃爍
// =====================================================
static void drawSpaceStar() {
  if (!spaceStarActive) {
    return;
  }

  drawSpaceSpriteFrame(
    (int)spaceStarX,
    (int)spaceStarY,
    SPACE_STAR_SHEET_W,
    SPACE_STAR_H,
    SPACE_STAR,
    spaceStarFrame * SPACE_STAR_FRAME_W,
    0,
    SPACE_STAR_FRAME_W,
    SPACE_STAR_H,
    false,
    true
  );
}


// =====================================================
// 安排下一顆星星產生時間
// =====================================================
static void scheduleNextSpaceStar(unsigned long nowMs) {
  spaceNextStarSpawnMs =
    nowMs +
    random(SPACE_STAR_SPAWN_MIN_MS, SPACE_STAR_SPAWN_MAX_MS + 1);
}


// =====================================================
// 產生一顆從任意方向直線穿越的星星
// =====================================================
static void spawnSpaceStar(unsigned long nowMs) {
  uint8_t side = random(0, 4);

  float startX = 0.0f;
  float startY = 0.0f;
  float endX = 0.0f;
  float endY = 0.0f;

  // 0 = 左進右出
  // 1 = 右進左出
  // 2 = 上進下出
  // 3 = 下進上出
  if (side == 0) {
    startX = -SPACE_STAR_FRAME_W;
    startY = random(0, SPACE_SCR_H - SPACE_STAR_H + 1);
    endX = SPACE_SCR_W + SPACE_STAR_FRAME_W;
    endY = random(0, SPACE_SCR_H - SPACE_STAR_H + 1);
  }
  else if (side == 1) {
    startX = SPACE_SCR_W + SPACE_STAR_FRAME_W;
    startY = random(0, SPACE_SCR_H - SPACE_STAR_H + 1);
    endX = -SPACE_STAR_FRAME_W;
    endY = random(0, SPACE_SCR_H - SPACE_STAR_H + 1);
  }
  else if (side == 2) {
    startX = random(0, SPACE_SCR_W - SPACE_STAR_FRAME_W + 1);
    startY = -SPACE_STAR_H;
    endX = random(0, SPACE_SCR_W - SPACE_STAR_FRAME_W + 1);
    endY = SPACE_SCR_H + SPACE_STAR_H;
  }
  else {
    startX = random(0, SPACE_SCR_W - SPACE_STAR_FRAME_W + 1);
    startY = SPACE_SCR_H + SPACE_STAR_H;
    endX = random(0, SPACE_SCR_W - SPACE_STAR_FRAME_W + 1);
    endY = -SPACE_STAR_H;
  }

  float dx = endX - startX;
  float dy = endY - startY;
  float length = sqrt(dx * dx + dy * dy);

  if (length < 1.0f) {
    length = 1.0f;
  }

  spaceStarX = startX;
  spaceStarY = startY;
  spaceStarVX = (dx / length) * SPACE_STAR_SPEED;
  spaceStarVY = (dy / length) * SPACE_STAR_SPEED;

  spaceStarFrame = 0;
  spaceStarLastFrameMs = nowMs;
  spaceStarActive = true;
}


// =====================================================
// 更新星星移動與閃爍
// =====================================================
static void updateSpaceStar(unsigned long nowMs, float dt) {
  if (!spaceStarActive) {
    if (nowMs >= spaceNextStarSpawnMs) {
      spawnSpaceStar(nowMs);
    }

    return;
  }

  if (nowMs - spaceStarLastFrameMs >= SPACE_STAR_FRAME_INTERVAL_MS) {
    spaceStarLastFrameMs = nowMs;
    spaceStarFrame++;

    if (spaceStarFrame >= SPACE_STAR_FRAME_COUNT) {
      spaceStarFrame = 0;
    }
  }

  spaceStarX += spaceStarVX * dt;
  spaceStarY += spaceStarVY * dt;

  // 星星完全離開畫面後，排下一顆
  if (
    spaceStarX < -32 ||
    spaceStarX > SPACE_SCR_W + 32 ||
    spaceStarY < -32 ||
    spaceStarY > SPACE_SCR_H + 32
  ) {
    spaceStarActive = false;
    scheduleNextSpaceStar(nowMs);
  }
}


// =====================================================
// 主角與星星碰撞判斷
// =====================================================
static bool checkSpaceHeroStarCollision() {
  if (!spaceStarActive) {
    return false;
  }

  int hx1 = (int)spaceHeroX + 4;
  int hy1 = (int)spaceHeroY + 2;
  int hx2 = (int)spaceHeroX + SPACE_HERO_FRAME_W - 4;
  int hy2 = (int)spaceHeroY + SPACE_HERO_H / 2;

  int sx1 = (int)spaceStarX + 2;
  int sy1 = (int)spaceStarY + 2;
  int sx2 = (int)spaceStarX + SPACE_STAR_FRAME_W - 2;
  int sy2 = (int)spaceStarY + SPACE_STAR_H - 2;

  if (hx1 > sx2) return false;
  if (hx2 < sx1) return false;
  if (hy1 > sy2) return false;
  if (hy2 < sy1) return false;

  return true;
}


// =====================================================
// 繪製 WIN 圖
// SPACE_WIN 48x24
// Mario frame0，Luigi frame1
// 每個 frame 是 24x24
// =====================================================
static void drawSpaceWin() {
  int srcX = spaceHero * SPACE_WIN_FRAME_W;

  drawSpaceSpriteFrame(
    20,
    20,
    SPACE_WIN_SHEET_W,
    SPACE_WIN_H,
    SPACE_WIN,
    srcX,
    0,
    SPACE_WIN_FRAME_W,
    SPACE_WIN_H,
    false,
    true
  );
}


// =====================================================
// 狀態切換
// =====================================================
static void setSpaceState(uint8_t newState, unsigned long nowMs) {
  spaceState = newState;
  spaceStateStartMs = nowMs;

  // 預設大部分狀態都顯示時鐘
  // 只有準備起飛 / 升空 / 黑場等待 / WIN 會另外關閉
  spaceClockVisible = true;

  if (newState == SPACE_STATE_WAIT_START) {
    // 初始畫面：背景從 y=256，火箭在 y=16
    spaceBgY = 256.0f;
    spaceBgSlideOffsetY = 0.0f;

    spaceRocketX = 0.0f;
    spaceRocketY = 16.0f;

    spaceTwinkleInited = false;
    spaceTwinkleLastMs = 0;

    spaceStarActive = false;
    spaceStarEatCount = 0;

    spaceHeroX = 23.0f;
    spaceHeroY = 29.0f;
    spaceHeroVX = SPACE_HERO_SPEED_X;
    spaceHeroVY = -SPACE_HERO_SPEED_Y;
    spaceHeroFrame = 0;
    spaceHeroFlipH = false;
    spaceHeroEating = false;
  }

  if (newState == SPACE_STATE_PRELAUNCH_FLAME) {
    // 火箭準備起飛：取消時鐘，只看火箭與火焰
    spaceClockVisible = false;
    spaceFlameFrame = 0;
    spaceFlameLastFrameMs = nowMs;
  }

  if (newState == SPACE_STATE_TAKEOFF) {
    // 火箭升空：取消時鐘
    spaceClockVisible = false;
    spaceFlameFrame = 0;
    spaceFlameLastFrameMs = nowMs;
  }

  if (newState == SPACE_STATE_BG_SLIDE_OUT) {
    // SPACE_BG 已到最頂端，開始整張背景往下滑出
    // 黑色從上方接續進來
    spaceClockVisible = false;

    spaceBgY = 0.0f;
    spaceBgSlideOffsetY = 0.0f;

    spaceRocketX = 0.0f;
    spaceRocketY = 0.0f;
  }

  if (newState == SPACE_STATE_SPACE_HOLD_BLACK) {
    // 黑色已經完全佈滿畫面，這時才開始使用星空閃爍背景
    spaceClockVisible = false;

    spaceBgY = 0.0f;
    spaceBgSlideOffsetY = 64.0f;

    spaceRocketX = 0.0f;
    spaceRocketY = 0.0f;

    spaceTwinkleInited = false;
    spaceTwinkleLastMs = 0;
  }

  if (newState == SPACE_STATE_ROCKET_DESCEND) {
    // 火箭下降開始後恢復時鐘
    spaceClockVisible = true;

    spaceRocketX = 0.0f;
    spaceRocketY = 0.0f;
  }

  if (newState == SPACE_STATE_SPACE_HOLD_CLOCK) {
    // 火箭下降到 y=16，停留並顯示時鐘
    spaceClockVisible = true;

    spaceBgY = 0.0f;
    spaceBgSlideOffsetY = 64.0f;

    spaceRocketX = 0.0f;
    spaceRocketY = 16.0f;
  }

  if (newState == SPACE_STATE_HERO_IDLE) {
    // 主角出艙前待機
    spaceClockVisible = true;

    spaceHeroX = 23.0f;
    spaceHeroY = 29.0f;
    spaceHeroFrame = 0;
    spaceHeroFlipH = false;
    spaceHeroEating = false;
  }

  if (newState == SPACE_STATE_ROAM) {
    // 主角開始太空漫遊，火箭也開始慢慢往左退出
    spaceClockVisible = true;

    spaceRocketX = 0.0f;
    spaceRocketY = 16.0f;

    spaceHeroX = 23.0f;
    spaceHeroY = 29.0f;
    spaceHeroVX = SPACE_HERO_SPEED_X;
    spaceHeroVY =
      (random(0, 2) == 0)
      ? -SPACE_HERO_SPEED_Y
      : SPACE_HERO_SPEED_Y;

    spaceHeroFrame = 1;
    spaceHeroFlipH = false;
    spaceHeroEating = false;

    spaceStarActive = false;
    spaceStarEatCount = 0;
    scheduleNextSpaceStar(nowMs);
  }

  if (newState == SPACE_STATE_FINAL_EAT) {
    // 吃到最後一顆星星後，不立刻進 WIN
    // 先讓吃星星動畫播完
    spaceClockVisible = true;
    spaceStarActive = false;
    spaceFinalEatDoneMs = 0;
  }

  if (newState == SPACE_STATE_WIN) {
    // WIN 畫面不顯示時鐘，而且只清除與繪製一次
    spaceClockVisible = false;
    spaceWinScreenDrawn = false;
  }

  if (newState == SPACE_STATE_RESTART_WAIT) {
    // WIN 清掉後回到 init 畫面，等待 SPACE_RESTART_WAIT_MS
    spaceClockVisible = true;

    spaceBgY = 256.0f;
    spaceBgSlideOffsetY = 0.0f;

    spaceRocketX = 0.0f;
    spaceRocketY = 16.0f;

    spaceTwinkleInited = false;
    spaceTwinkleLastMs = 0;

    spaceStarActive = false;
    spaceStarEatCount = 0;

    spaceHeroX = 23.0f;
    spaceHeroY = 29.0f;
    spaceHeroVX = SPACE_HERO_SPEED_X;
    spaceHeroVY = -SPACE_HERO_SPEED_Y;
    spaceHeroFrame = 0;
    spaceHeroFlipH = false;
    spaceHeroEating = false;
  }
}


// =====================================================
// 初始化
// =====================================================
static void MarioSpaceModeInit() {
  if (!ModefirstRun) {
    return;
  }

  randomSeed(millis());

  spaceHero = random(0, 2);
  spaceLastUpdateMs = millis();

  setSpaceState(SPACE_STATE_WAIT_START, millis());

  ModefirstRun = false;
}


// =====================================================
// 更新火箭升空
// 背景從 source y=256 捲到 source y=0
// 火箭從 y=16 移到 y=0
// =====================================================
static void updateSpaceTakeoff(unsigned long nowMs) {
  unsigned long elapsed = nowMs - spaceStateStartMs;

  if (elapsed >= SPACE_TAKEOFF_TOTAL_MS) {
    spaceBgY = 0.0f;
    spaceRocketY = 0.0f;
    spaceBgSlideOffsetY = 0.0f;

    setSpaceState(SPACE_STATE_BG_SLIDE_OUT, nowMs);
    return;
  }

  float t = (float)elapsed / (float)SPACE_TAKEOFF_TOTAL_MS;

  spaceBgY = 256.0f - (256.0f * t);
  spaceRocketY = 16.0f - (16.0f * t);
  spaceRocketX = 0.0f;
}


// =====================================================
// 更新背景滑出黑場
// SPACE_BG 最頂端 y=0~63 往下滑到 y=64
// 畫面最後完全變黑，再切換成星空閃爍背景
// =====================================================
static void updateSpaceBgSlideOut(unsigned long nowMs) {
  unsigned long elapsed = nowMs - spaceStateStartMs;

  if (elapsed >= SPACE_BG_SLIDE_OUT_MS) {
    spaceBgSlideOffsetY = 64.0f;
    setSpaceState(SPACE_STATE_SPACE_HOLD_BLACK, nowMs);
    return;
  }

  float t = (float)elapsed / (float)SPACE_BG_SLIDE_OUT_MS;
  spaceBgSlideOffsetY = 64.0f * t;
}


// =====================================================
// 更新火箭下降
// 火箭從 y=0 下降到 y=16
// =====================================================
static void updateSpaceRocketDescend(unsigned long nowMs) {
  unsigned long elapsed = nowMs - spaceStateStartMs;

  if (elapsed >= SPACE_ROCKET_DESCEND_MS) {
    spaceRocketY = 16.0f;
    setSpaceState(SPACE_STATE_SPACE_HOLD_CLOCK, nowMs);
    return;
  }

  float t = (float)elapsed / (float)SPACE_ROCKET_DESCEND_MS;

  spaceRocketY = 16.0f * t;
  spaceRocketX = 0.0f;
}


// =====================================================
// 更新主角漂浮
// 類似 DVD 螢幕保護程式，碰到邊緣就反彈
// =====================================================
static void updateSpaceHeroRoam(unsigned long nowMs, float dt) {
  updateSpaceHeroEat(nowMs);

  spaceHeroX += spaceHeroVX * dt;
  spaceHeroY += spaceHeroVY * dt;

  if (spaceHeroX <= 0.0f) {
    spaceHeroX = 0.0f;
    spaceHeroVX = SPACE_HERO_SPEED_X;
    spaceHeroFlipH = false;

    if (!spaceHeroEating) {
      spaceHeroFrame = 1;
    }
  }

  if (spaceHeroX >= SPACE_SCR_W - SPACE_HERO_FRAME_W) {
    spaceHeroX = SPACE_SCR_W - SPACE_HERO_FRAME_W;
    spaceHeroVX = -SPACE_HERO_SPEED_X;
    spaceHeroFlipH = true;

    if (!spaceHeroEating) {
      spaceHeroFrame = 1;
    }
  }

  if (spaceHeroY <= 0.0f) {
    spaceHeroY = 0.0f;
    spaceHeroVY = SPACE_HERO_SPEED_Y;
  }

  if (spaceHeroY >= SPACE_SCR_H - SPACE_HERO_H) {
    spaceHeroY = SPACE_SCR_H - SPACE_HERO_H;
    spaceHeroVY = -SPACE_HERO_SPEED_Y;
  }
}


// =====================================================
// 更新太空漫遊
// 火箭往左退出，主角漂浮，星星穿越畫面
// =====================================================
static void updateSpaceRoam(unsigned long nowMs, float dt) {
  // 火箭慢慢往左退出場景
  if (spaceRocketX + SPACE_ROCKET_W > 0) {
    spaceRocketX -= SPACE_ROCKET_EXIT_SPEED * dt;
  }

  updateSpaceHeroRoam(nowMs, dt);
  updateSpaceStar(nowMs, dt);

  if (checkSpaceHeroStarCollision()) {
    spaceStarActive = false;
    spaceStarEatCount++;

    startSpaceHeroEat(nowMs);

    // 吃到第 5 顆後，先播放吃星星動畫，不立刻進 WIN
    if (spaceStarEatCount >= SPACE_STAR_WIN_COUNT) {
      setSpaceState(SPACE_STATE_FINAL_EAT, nowMs);
      return;
    }

    scheduleNextSpaceStar(nowMs);
  }
}


// =====================================================
// 更新最後吃星星狀態
// 等吃星星動畫播完，再延遲一小段時間，最後進 WIN
// =====================================================
static void updateSpaceFinalEat(unsigned long nowMs) {
  updateSpaceHeroEat(nowMs);

  if (!spaceHeroEating) {
    if (spaceFinalEatDoneMs == 0) {
      spaceFinalEatDoneMs = nowMs;
    }

    if (nowMs - spaceFinalEatDoneMs >= SPACE_FINAL_EAT_AFTER_DELAY_MS) {
      setSpaceState(SPACE_STATE_WIN, nowMs);
    }
  }
}


// =====================================================
// 更新狀態機
// =====================================================
static void updateSpaceState(unsigned long nowMs) {
  unsigned long elapsed = nowMs - spaceStateStartMs;

  unsigned long deltaMs = nowMs - spaceLastUpdateMs;
  spaceLastUpdateMs = nowMs;

  float dt = (float)deltaMs / 1000.0f;

  // 避免暫停或切換模式後，dt 太大造成物件瞬移
  if (dt > 0.2f) {
    dt = 0.2f;
  }

  switch (spaceState) {
    case SPACE_STATE_WAIT_START: {
      if (elapsed >= SPACE_PARAM_A_MS) {
        setSpaceState(SPACE_STATE_ICON_FLASH, nowMs);
      }
      break;
    }

    case SPACE_STATE_ICON_FLASH: {
      unsigned long totalBlinkMs =
        (unsigned long)SPACE_ICON_BLINK_COUNT *
        2UL *
        SPACE_ICON_BLINK_INTERVAL_MS;

      if (elapsed >= totalBlinkMs) {
        setSpaceState(SPACE_STATE_COUNTDOWN, nowMs);
      }
      break;
    }

    case SPACE_STATE_COUNTDOWN: {
      if (elapsed >= SPACE_COUNTDOWN_MS) {
        setSpaceState(SPACE_STATE_PRELAUNCH_FLAME, nowMs);
      }
      break;
    }

    case SPACE_STATE_PRELAUNCH_FLAME: {
      updateSpaceFlameFrame(nowMs);

      if (elapsed >= SPACE_PRELAUNCH_FLAME_MS) {
        setSpaceState(SPACE_STATE_TAKEOFF, nowMs);
      }
      break;
    }

    case SPACE_STATE_TAKEOFF: {
      updateSpaceFlameFrame(nowMs);
      updateSpaceTakeoff(nowMs);
      break;
    }

    case SPACE_STATE_BG_SLIDE_OUT: {
      updateSpaceFlameFrame(nowMs);
      updateSpaceBgSlideOut(nowMs);
      break;
    }

    case SPACE_STATE_SPACE_HOLD_BLACK: {
      updateSpaceFlameFrame(nowMs);

      if (elapsed >= SPACE_PARAM_B_MS) {
        setSpaceState(SPACE_STATE_ROCKET_DESCEND, nowMs);
      }
      break;
    }

    case SPACE_STATE_ROCKET_DESCEND: {
      updateSpaceRocketDescend(nowMs);
      break;
    }

    case SPACE_STATE_SPACE_HOLD_CLOCK: {
      if (elapsed >= SPACE_PARAM_B_MS) {
        setSpaceState(SPACE_STATE_HERO_IDLE, nowMs);
      }
      break;
    }

    case SPACE_STATE_HERO_IDLE: {
      if (elapsed >= SPACE_HERO_IDLE_MS) {
        setSpaceState(SPACE_STATE_ROAM, nowMs);
      }
      break;
    }

    case SPACE_STATE_ROAM: {
      updateSpaceRoam(nowMs, dt);
      break;
    }

    case SPACE_STATE_FINAL_EAT: {
      updateSpaceFinalEat(nowMs);
      break;
    }

    case SPACE_STATE_WIN: {
      if (elapsed >= SPACE_WIN_HOLD_MS) {
        ClearAll();
        setSpaceState(SPACE_STATE_RESTART_WAIT, nowMs);
      }
      break;
    }

    case SPACE_STATE_RESTART_WAIT: {
      if (elapsed >= SPACE_RESTART_WAIT_MS) {
        spaceHero = random(0, 2);
        setSpaceState(SPACE_STATE_WAIT_START, nowMs);
      }
      break;
    }

    default: {
      setSpaceState(SPACE_STATE_WAIT_START, nowMs);
      break;
    }
  }
}


// =====================================================
// 是否顯示 ICON
// =====================================================
static bool spaceShouldDrawIcon(unsigned long nowMs) {
  if (spaceState != SPACE_STATE_ICON_FLASH) {
    return false;
  }

  unsigned long elapsed = nowMs - spaceStateStartMs;
  unsigned long phase = elapsed / SPACE_ICON_BLINK_INTERVAL_MS;

  // 偶數 phase 顯示，奇數 phase 隱藏
  return ((phase % 2UL) == 0UL);
}


// =====================================================
// 是否顯示火焰
// =====================================================
static bool spaceShouldDrawFlame() {
  return (
    spaceState == SPACE_STATE_PRELAUNCH_FLAME ||
    spaceState == SPACE_STATE_TAKEOFF ||
    spaceState == SPACE_STATE_BG_SLIDE_OUT ||
    spaceState == SPACE_STATE_SPACE_HOLD_BLACK ||
    spaceState == SPACE_STATE_ROCKET_DESCEND
  );
}


// =====================================================
// 是否顯示火箭
// =====================================================
static bool spaceShouldDrawRocket() {
  return (
    spaceState == SPACE_STATE_WAIT_START ||
    spaceState == SPACE_STATE_ICON_FLASH ||
    spaceState == SPACE_STATE_COUNTDOWN ||
    spaceState == SPACE_STATE_PRELAUNCH_FLAME ||
    spaceState == SPACE_STATE_TAKEOFF ||
    spaceState == SPACE_STATE_BG_SLIDE_OUT ||
    spaceState == SPACE_STATE_SPACE_HOLD_BLACK ||
    spaceState == SPACE_STATE_ROCKET_DESCEND ||
    spaceState == SPACE_STATE_SPACE_HOLD_CLOCK ||
    spaceState == SPACE_STATE_HERO_IDLE ||
    spaceState == SPACE_STATE_ROAM ||
    spaceState == SPACE_STATE_FINAL_EAT ||
    spaceState == SPACE_STATE_RESTART_WAIT
  );
}


// =====================================================
// 是否顯示主角
// =====================================================
static bool spaceShouldDrawHero() {
  return (
    spaceState == SPACE_STATE_HERO_IDLE ||
    spaceState == SPACE_STATE_ROAM ||
    spaceState == SPACE_STATE_FINAL_EAT
  );
}


// =====================================================
// 繪製 WIN 畫面
// 注意：WIN 畫面只清除與繪製一次，不每幀重畫
// =====================================================
static void renderSpaceWinScene() {
  if (!spaceWinScreenDrawn) {
    ClearAll();

    drawSpaceWin();

    display.setTextWrap(false);
    display.setTextSize(1);

    display.setTextColor(0x0000);
    display.setCursor(12, 51);
    display.print("YOU WIN");

    display.setTextColor(0xFFFF);
    display.setCursor(11, 50);
    display.print("YOU WIN");

    spaceWinScreenDrawn = true;
  }
}


// =====================================================
// 繪製整體場景
// =====================================================
static void renderSpaceScene(unsigned long nowMs) {
  if (spaceState == SPACE_STATE_WIN) {
    renderSpaceWinScene();
    return;
  }

  // 背景繪製順序：
  // 1. 起飛前 / 起飛中：SPACE_BG
  // 2. SPACE_BG 到最頂端後：SPACE_BG 往下滑出，黑色從上方接續
  // 3. 完全黑色後：黑底星空閃爍
  if (spaceState == SPACE_STATE_BG_SLIDE_OUT) {
    drawSpaceBackgroundSlideOut();
  }
  else if (spaceShouldUseTwinkleBackground()) {
    drawSpaceTwinkleBackground(nowMs);
  }
  else {
    drawSpaceBackground();
  }

  if (spaceShouldDrawRocket()) {
    drawSpaceRocket();
  }

  if (spaceShouldDrawFlame()) {
    drawSpaceFlame();
  }

  if (spaceShouldDrawHero()) {
    drawSpaceHero();
  }

  drawSpaceStar();

  if (spaceShouldDrawIcon(nowMs)) {
    drawSpaceIcon();
  }

  if (spaceState == SPACE_STATE_COUNTDOWN) {
    drawSpaceCountdownText();
  }

  if (spaceClockVisible) {
    drawThemeClockText();
  }
}


// =====================================================
// 主函式
// =====================================================
void MarioSpaceMode() {
  MarioSpaceModeInit();

  unsigned long nowMs = millis();

  updateSpaceState(nowMs);
  renderSpaceScene(nowMs);

  wait_with_display(SPACE_FRAME_DELAY_MS);
}
