#include "MarioClock.h"

// =====================================================
// MarioMode.ino
// 全畫布重繪版本 + 雲朵持續移動版本
//
// 重點：
// 1. 每次更新直接重畫整個 64x64 場景
// 2. 雲朵在等待時間也會從右往左慢慢移動
// 3. 原本 CLOUD1 軌道維持 y=21，作為前景雲
// 4. 原本 CLOUD2 軌道維持 y=7，作為背景雲
// 5. 繪圖順序：CLOUD2 後、BLOCK 中、CLOUD1 前
// 6. 保留原本跳躍與磚塊撞擊邏輯
// =====================================================

// ===================== 內部狀態 =====================
static float prev_m_y = 40.0f;
static float m_y = 40.0f;
static float m_vy = 0.0f;
static bool  m_jumping = false;
static bool  block_move = true;

// 角色切換（0=Mario,1=Yoshi,2=Mushroom,3=Cloud,4=Ghost,5=Baby）
static int currentHero = 0;

static const int MC_HILL_W   = 20;
static const int MC_HILL_H   = 22;

static const int MC_CLOUD1_W = 17;
static const int MC_CLOUD1_H = 12;

static const int MC_CLOUD2_W = 17;
static const int MC_CLOUD2_H = 12;

static const int MC_BUSH_W   = 21;
static const int MC_BUSH_H   = 9;

static const int MC_GROUND_W = 8;
static const int MC_GROUND_H = 8;

static const int MC_BLOCK_W  = 19;
static const int MC_BLOCK_H  = 19;

// ===================== 移動雲朵設定 =====================
// 這裡的 Front / Back 指「圖層」：
// Front = 原本 CLOUD1 的位置，維持 y=21，畫在 BLOCK 前面
// Back  = 原本 CLOUD2 的位置，維持 y=7，畫在 BLOCK 後面
//
// 注意：貼圖本身會在 CLOUD1 / CLOUD2 之間隨機抽。
static float marioCloudFrontX = 0.0f;
static int   marioCloudFrontY = 21;
static float marioCloudFrontSpeed = 2.0f;   // 前景雲速度，數字越大越快，單位約 px/sec
static uint8_t marioCloudFrontSprite = 1;   // 1 = CLOUD1 貼圖，2 = CLOUD2 貼圖

static float marioCloudBackX = 51.0f;
static int   marioCloudBackY = 7;
static float marioCloudBackSpeed = 1.2f;    // 背景雲速度，比前景慢一點會比較有遠近感
static uint8_t marioCloudBackSprite = 2;    // 1 = CLOUD1 貼圖，2 = CLOUD2 貼圖

static uint32_t marioCloudLastMs = 0;

// 前置宣告：marioClockAnimatedWait() 會呼叫全畫面重繪
static void renderMarioFullFrame();


// =====================================================
// 基本繪圖：palette/index sprite
// 透明 = SKY_COLOR 不畫
// =====================================================
static inline uint16_t marioClockReadPaletteColor(
  const uint8_t* bitmap,
  const uint16_t* palette,
  uint32_t pixelPos
) {
  uint8_t colorIndex = pgm_read_byte(&(bitmap[pixelPos]));
  return pgm_read_word(&(palette[colorIndex]));
}



static void drawMarioClockSpriteIndexedLen(
  int x,
  int y,
  int w,
  int h,
  const uint8_t* data,
  const uint16_t* palette,
  uint32_t dataLen
) {
  if (!data || !palette) return;

  for (int i = 0; i < h; i++) {
    int drawY = y + i;
    if (drawY < 0 || drawY >= 64) continue;

    for (int j = 0; j < w; j++) {
      int drawX = x + j;
      if (drawX < 0 || drawX >= 64) continue;

      uint32_t pixelPos =
        (uint32_t)i * (uint32_t)w +
        (uint32_t)j;

      // 避免資料長度跟 w*h 不一致時讀爆
      if (pixelPos >= dataLen) continue;

      uint16_t color =
        marioClockReadPaletteColor(data, palette, pixelPos);

      if (color != SKY_COLOR) {
        display.drawPixel(drawX, drawY, color);
      }
    }
  }
}

// 給主角用：主角尺寸正確時可用這個
static void drawMarioClockSpriteIndexed(
  int x,
  int y,
  int w,
  int h,
  const uint8_t* data,
  const uint16_t* palette
) {
  drawMarioClockSpriteIndexedLen(
    x,
    y,
    w,
    h,
    data,
    palette,
    (uint32_t)w * (uint32_t)h
  );
}

// 給固定陣列用：自動帶入 sizeof(array)
#define DRAW_MARIO_CLOCK_ARRAY(x, y, w, h, data, palette) \
  drawMarioClockSpriteIndexedLen( \
    x, y, w, h, data, palette, sizeof(data) \
  )

// =====================================================
// 移動雲朵
// =====================================================
static uint8_t randomMarioCloudSprite() {
  return (random(0, 2) == 0) ? 1 : 2;
}

static void resetMarioFrontCloud() {
  // 從畫面右側外面重新出現，random(0, 24) 用來製造一點間隔變化
  marioCloudFrontX = 64.0f + (float)random(0, 24);

  // 每次重新出現時，隨機抽 CLOUD1 或 CLOUD2 貼圖
  marioCloudFrontSprite = randomMarioCloudSprite();
}

static void resetMarioBackCloud() {
  // 從畫面右側外面重新出現，random(0, 24) 用來製造一點間隔變化
  marioCloudBackX = 64.0f + (float)random(0, 24);

  // 每次重新出現時，隨機抽 CLOUD1 或 CLOUD2 貼圖
  marioCloudBackSprite = randomMarioCloudSprite();
}

static void updateMarioMovingClouds() {
  uint32_t now = millis();

  if (marioCloudLastMs == 0) {
    marioCloudLastMs = now;
    return;
  }

  float dt = (float)(now - marioCloudLastMs) / 1000.0f;
  marioCloudLastMs = now;

  // 避免切換模式或暫停後 dt 太大，造成雲朵瞬間跳很遠
  if (dt > 0.5f) {
    dt = 0.5f;
  }

  marioCloudFrontX -= marioCloudFrontSpeed * dt;
  marioCloudBackX  -= marioCloudBackSpeed  * dt;

  if (marioCloudFrontX < -MC_CLOUD1_W) {
    resetMarioFrontCloud();
  }

  if (marioCloudBackX < -MC_CLOUD2_W) {
    resetMarioBackCloud();
  }
}

static void drawMarioCloudAt(int x, int y, uint8_t spriteType) {
  if (spriteType == 1) {
    DRAW_MARIO_CLOCK_ARRAY(
      x,
      y,
      MC_CLOUD1_W,
      MC_CLOUD1_H,
      CLOUD1,
      CLOUD1_PALETTE
    );
  } else {
    DRAW_MARIO_CLOCK_ARRAY(
      x,
      y,
      MC_CLOUD2_W,
      MC_CLOUD2_H,
      CLOUD2,
      CLOUD2_PALETTE
    );
  }
}

// 等待時也持續更新雲朵，避免畫面完全靜止
static void marioClockAnimatedWait(uint32_t waitMs) {
  uint32_t startMs = millis();

  while ((uint32_t)(millis() - startMs) < waitMs) {
    wait_with_display(80);
    updateMarioMovingClouds();
    renderMarioFullFrame();
  }
}

// =====================================================
// 取得目前主角 sprite 資訊
// =====================================================
static void getMarioHeroInfo(
  int &heroX,
  int &heroY,
  int &heroW,
  int &heroH,
  const uint8_t* &heroSprite,
  const uint16_t* &heroPalette
) {
  heroX = 20;
  heroY = (int)m_y - 10;
  heroW = 25;
  heroH = 30;
  heroSprite = nullptr;
  heroPalette = nullptr;

  if (currentHero == 0) {
    heroX = 23;
    heroW = 16;
    heroH = 26;
    heroY = (int)m_y - 6;

    if (m_jumping) {
      heroSprite = MARIO_JUMP;
      heroPalette = MARIO_JUMP_PALETTE;
    } else {
      heroSprite = MARIO;
      heroPalette = MARIO_PALETTE;
    }
  }
  else if (currentHero == 1) {
    heroX = 20;
    heroW = 25;
    heroH = 30;
    heroY = (int)m_y - 10;

    if (m_jumping) {
      heroSprite = YOSHI_JUMP;
      heroPalette = YOSHI_JUMP_PALETTE;
    } else {
      heroSprite = YOSHI;
      heroPalette = YOSHI_PALETTE;
    }
  }
  else if (currentHero == 2) {
    heroX = 23;
    heroW = 20;
    heroH = 30;
    heroY = (int)m_y - 10;
    heroSprite = MUSHROOM;
    heroPalette = MUSHROOM_PALETTE;
  }
  else if (currentHero == 3) {
    heroX = 20;
    heroW = 25;
    heroH = 30;
    heroY = (int)m_y - 10;
    heroSprite = CLOUD;
    heroPalette = CLOUD_PALETTE;
  }
  else if (currentHero == 4) {
    heroX = 20;
    heroW = 25;
    heroH = 25;
    heroY = (int)m_y - 5;

    if (m_jumping) {
      heroSprite = GHOST_JUMP;
      heroPalette = GHOST_JUMP_PALETTE;
    } else {
      heroSprite = GHOST;
      heroPalette = GHOST_PALETTE;
    }
  }
  else if (currentHero == 5) {
    heroX = 20;
    heroW = 19;
    heroH = 26;
    heroY = (int)m_y - 6;

    if (m_jumping) {
      heroSprite = BABY_JUMP;
      heroPalette = BABY_JUMP_PALETTE;
    } else {
      heroSprite = BABY;
      heroPalette = BABY_PALETTE;
    }
  }
}

// =====================================================
// 全畫面重繪
// =====================================================
static void renderMarioFullFrame() {
  // 1. 背景
  display.fillScreen(SKY_COLOR);

  // 2. 背景物件
  DRAW_MARIO_CLOCK_ARRAY(
    0,
    38,
    MC_HILL_W,
    MC_HILL_H,
    HILL,
    HILL_PALETTE
  );

  // CLOUD2 後景：先畫，所以會在 BLOCK 後面
  drawMarioCloudAt(
    (int)marioCloudBackX,
    marioCloudBackY,
    marioCloudBackSprite
  );

  DRAW_MARIO_CLOCK_ARRAY(
    43,
    51,
    MC_BUSH_W,
    MC_BUSH_H,
    BUSH,
    BUSH_PALETTE
  );

  // 3. 地板
  for (int x = 0; x < 64; x += MC_GROUND_W) {
    DRAW_MARIO_CLOCK_ARRAY(
      x,
      60,
      MC_GROUND_W,
      MC_GROUND_H,
      GROUND,
      GROUND_PALETTE
    );
  }

  // 4. 時鐘磚塊位置
  int blockY = block_move ? 4 : -4;

  DRAW_MARIO_CLOCK_ARRAY(
    13,
    blockY,
    MC_BLOCK_W,
    MC_BLOCK_H,
    BLOCK,
    BLOCK_PALETTE
  );

  DRAW_MARIO_CLOCK_ARRAY(
    32,
    blockY,
    MC_BLOCK_W,
    MC_BLOCK_H,
    BLOCK,
    BLOCK_PALETTE
  );

  // 5. 時間數字
  int clockY = blockY + 2;
  show_mario_number(H, 7, 12, 14, clockY, 0xF800);
  show_mario_number(M, 7, 12, 33, clockY, 0xF800);

  // CLOUD1 前景：後畫，所以會在 BLOCK 前面
  drawMarioCloudAt(
    (int)marioCloudFrontX,
    marioCloudFrontY,
    marioCloudFrontSprite
  );

  // 6. 主角
  int heroX, heroY, heroW, heroH;
  const uint8_t* heroSprite = nullptr;
  const uint16_t* heroPalette = nullptr;

  getMarioHeroInfo(
    heroX,
    heroY,
    heroW,
    heroH,
    heroSprite,
    heroPalette
  );

  drawMarioClockSpriteIndexed(
    heroX,
    heroY,
    heroW,
    heroH,
    heroSprite,
    heroPalette
  );
}

// =====================================================
// 初始化
// =====================================================
void MarioClockInit() {
  if (ModefirstRun) {
    Serial.println("Mario 初始（全畫布 + 移動雲朵版）");

    m_y = 40.0f;
    prev_m_y = 40.0f;
    m_vy = 0.0f;
    m_jumping = false;
    block_move = true;
    currentHero = 0;

    // 雲朵一開始維持原本位置，之後離開左側才會從右側重新進場
    marioCloudFrontX = 0.0f;
    marioCloudFrontY = 21;
    marioCloudFrontSpeed = 2.0f;
    marioCloudFrontSprite = 1;

    marioCloudBackX = 51.0f;
    marioCloudBackY = 7;
    marioCloudBackSpeed = 1.2f;
    marioCloudBackSprite = 2;

    marioCloudLastMs = millis();

    last_M_Time = M;

    renderMarioFullFrame();
    ModefirstRun = false;
  }
}

// =====================================================
// 主模式
// =====================================================
void MarioClockMode() {
  MarioClockInit();

  wait_with_display(80);

  // 不管有沒有跳躍，每次循環都更新雲朵
  updateMarioMovingClouds();

  // 分鐘變化時切角色並起跳
  if (M != last_M_Time && !m_jumping) {
    currentHero = random(0, 6);

    // 先顯示新角色站立一段時間
    // 原本這裡 wait_with_display(10000) 會讓畫面靜止
    // 改成 marioClockAnimatedWait(10000)，等待時雲朵仍會動
    m_y = 40.0f;
    m_vy = 0.0f;
    m_jumping = false;
    block_move = true;

    renderMarioFullFrame();
    marioClockAnimatedWait(10000);

    m_jumping = true;
    m_vy = -3.5f;
  }

  // 跳躍物理
  if (m_jumping) {
    m_y += m_vy;
    m_vy += 0.45f;

    // 撞擊磚塊
    if (block_move && m_y < 30.0f) {
      block_move = false;
    }

    // 落地
    if (m_y >= 40.0f) {
      m_y = 40.0f;
      m_jumping = false;
      block_move = true;
      Serial.println("落地");
    }
  }

  // 每次循環都重畫，所以等待狀態也會看到雲朵移動
  renderMarioFullFrame();

  last_M_Time = M;
}
