#define MAX_AIRS 10

// =====================================================
// 氣泡圖案 (0=黑, 1=藍/粉 2=白)
// =====================================================
const uint8_t PROGMEM AIR[7][7] = {
  0, 0, 1, 1, 1, 0, 0,
  0, 1, 2, 2, 2, 1, 0,
  1, 2, 2, 2, 2, 2, 1,
  1, 2, 2, 2, 2, 2, 1,
  1, 2, 2, 2, 2, 2, 1,
  0, 1, 2, 2, 2, 1, 0,
  0, 0, 1, 1, 1, 0, 0
};

// =====================================================
// 泡泡龍圖案 1
// 0=透明 1=橘黃 2=綠 3=白  4= 
// =====================================================
const uint8_t PROGMEM Bubble[16][16] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 
  0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 0, 0, 0, 0, 
  0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 
  0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 2, 3, 2, 0, 0, 
  0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 4, 2, 4, 3, 0, 0, 
  0, 0, 0, 1, 2, 2, 2, 2, 3, 3, 4, 2, 4, 3, 0, 0, 
  0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 2, 4, 3, 2, 0, 
  0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 4, 2, 4, 3, 2, 0, 
  0, 0, 0, 1, 2, 2, 2, 2, 2, 3, 3, 2, 3, 2, 2, 0, 
  0, 0, 0, 0, 2, 1, 2, 2, 4, 4, 4, 3, 4, 4, 0, 0, 
  0, 0, 0, 2, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 0, 0, 
  0, 0, 0, 2, 1, 1, 1, 2, 2, 3, 3, 3, 3, 0, 0, 0, 
  0, 0, 1, 2, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3, 0, 0, 
  0, 1, 2, 2, 2, 2, 2, 1, 1, 1, 3, 3, 3, 3, 0, 0, 
  2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 3, 3, 1, 1, 1
};

// =====================================================
// 泡泡龍圖案 2
// =====================================================
const uint8_t PROGMEM Bubble2[16][16] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 
  0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 0, 0, 0, 0, 
  0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 
  0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 2, 3, 2, 0, 0, 
  0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 4, 2, 4, 3, 0, 0, 
  0, 0, 0, 1, 2, 2, 2, 2, 3, 3, 4, 2, 4, 3, 0, 0, 
  0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 2, 4, 3, 2, 0, 
  0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 4, 2, 4, 3, 2, 0, 
  0, 0, 0, 1, 2, 2, 2, 2, 2, 3, 3, 2, 3, 2, 2, 0, 
  0, 0, 0, 0, 2, 1, 2, 2, 4, 4, 4, 4, 4, 4, 0, 0, 
  0, 0, 0, 2, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 0, 0, 
  2, 0, 0, 2, 1, 1, 1, 2, 2, 3, 3, 3, 3, 0, 0, 0, 
  2, 1, 1, 2, 1, 1, 2, 2, 3, 1, 1, 3, 3, 3, 0, 0, 
  0, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 3, 3, 0, 0, 
  0, 0, 2, 2, 2, 2, 1, 1, 1, 1, 1, 3, 3, 1, 1, 0
};

struct AirBubble {
  int x;
  int y;
  bool active;
  bool isPink;
  unsigned long lastMoveTime;
};

AirBubble airs[MAX_AIRS];

// =====================================================
// 背景色：深夜藍
// RGB: 21,36,71
// HEX: #152447
// =====================================================
static uint16_t BUBBLE_BG_COLOR;

// =====================================================
// 動畫 / 生成計時
// =====================================================
unsigned long lastAnimTime = 0;
unsigned long nextAirSpawnTime = 0;

static const unsigned long BUBBLE_ANIM_INTERVAL_MS = 220;
static const unsigned long AIR_MOVE_INTERVAL_MS    = 70;

bool useBubble2 = false;

// =====================================================
// 工具：安排下一次氣泡生成時間
// =====================================================
void scheduleNextBubbleAirSpawn(unsigned long now) {
  nextAirSpawnTime = now + (unsigned long)random(1200, 3200);
}

// =====================================================
// 工具：清空所有漂浮氣泡
// =====================================================
void clearBubbleAirs() {
  for (int i = 0; i < MAX_AIRS; i++) {
    airs[i].active = false;
    airs[i].x = 0;
    airs[i].y = 0;
    airs[i].isPink = false;
    airs[i].lastMoveTime = 0;
  }
}

// =====================================================
// 初始化
// =====================================================
void initBubble() {
  if (ModefirstRun) {
    BUBBLE_BG_COLOR = display.color565(21, 36, 71);

    clearBubbleAirs();

    unsigned long now = millis();
    lastAnimTime = now;
    useBubble2 = false;
    scheduleNextBubbleAirSpawn(now);

    ModefirstRun = false;
  }
}

// =====================================================
// 畫 1P
// 0=透明 1=橘黃 2=綠 3=白 4=黑
// =====================================================
void drawBubble(int x, int y, const uint8_t art[][16], int rows, int cols) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      uint8_t colorIdx = pgm_read_byte(&(art[i][j]));
      if (colorIdx == 0) continue;

      uint16_t color = 0;
      if (colorIdx == 1) color = display.color565(255, 126, 0);   // 橘黃
      if (colorIdx == 2) color = display.color565(34, 144, 76);   // 綠
      if (colorIdx == 3) color = display.color565(255, 255, 255); // 白
      if (colorIdx == 4) color = display.color565(0, 0, 0);       // 黑

      display.drawPixel(x + j, y + i, color);
    }
  }
}

// =====================================================
// 畫 2P（左右翻轉 + 綠改粉）
// 0=透明 1=橘黃 2=綠/粉 3=白 4=黑
// =====================================================
void drawBubbleFlip(int x, int y, const uint8_t art[][16], int rows, int cols, bool is2P) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      uint8_t colorIdx = pgm_read_byte(&(art[i][cols - 1 - j]));
      if (colorIdx == 0) continue;

      uint16_t color = 0;
      if (colorIdx == 1) color = display.color565(255, 126, 0);   // 橘黃
      if (colorIdx == 2) color = is2P ? display.color565(255, 105, 180)
                                      : display.color565(34, 144, 76);
      if (colorIdx == 3) color = display.color565(255, 255, 255); // 白
      if (colorIdx == 4) color = display.color565(0, 0, 0);       // 黑

      display.drawPixel(x + j, y + i, color);
    }
  }
}

// =====================================================
// 畫單顆漂浮氣泡
// =====================================================
void drawSingleAirBubble(int x, int y, bool isPink) {
  uint16_t borderColor = isPink ? display.color565(255, 105, 180)
                                : display.color565(0, 255, 255);
  uint16_t whiteColor = display.color565(255, 255, 255);

  for (int r = 0; r < 7; r++) {
    for (int c = 0; c < 7; c++) {
      uint8_t colorIdx = pgm_read_byte(&(AIR[r][c]));
      if (colorIdx == 1) display.drawPixel(x + c, y + r, borderColor);
      if (colorIdx == 2) display.drawPixel(x + c, y + r, whiteColor);
    }
  }
}

// =====================================================
// 畫上方時鐘
// =====================================================
void drawBubbleClock() {
  display.drawRect(12, 5, 40, 18, display.color565(255, 255, 255));
  display.fillRect(14, 7, 36, 14, display.color565(0, 0, 0));
  
  showbit12number(H, 7, 12, 14, 8, hsv2rgb(hueh, saturation, value));
  showbit12number(M, 7, 12, 34, 8, hsv2rgb(huem, saturation, value));

  if (S % 2 == 0) {
    display.drawPixel(32, 12, hsv2rgb(hue, saturation, value));
    display.drawPixel(32, 16, hsv2rgb(hue, saturation, value));
  }
}

// =====================================================
// 更新主角動畫
// =====================================================
void updateBubbleHeroAnim(unsigned long now) {
  if (now - lastAnimTime >= BUBBLE_ANIM_INTERVAL_MS) {
    useBubble2 = !useBubble2;
    lastAnimTime = now;
  }
}

// =====================================================
// 生成氣泡
// =====================================================
void spawnBubbleAir(unsigned long now) {
  if (now < nextAirSpawnTime) return;

  for (int i = 0; i < MAX_AIRS; i++) {
    if (!airs[i].active) {
      bool fromRight = random(2);

      airs[i].x = fromRight ? 32 : 27;
      airs[i].y = 55;
      airs[i].isPink = fromRight;
      airs[i].active = true;
      airs[i].lastMoveTime = now;

      scheduleNextBubbleAirSpawn(now);
      return;
    }
  }

  scheduleNextBubbleAirSpawn(now + 400);
}

// =====================================================
// 更新氣泡位置
// =====================================================
void updateBubbleAirs(unsigned long now) {
  for (int i = 0; i < MAX_AIRS; i++) {
    if (!airs[i].active) continue;

    if (now - airs[i].lastMoveTime >= AIR_MOVE_INTERVAL_MS) {

      bool hitBottom = (
        airs[i].y == 23 &&
        airs[i].x > random(0, 6) &&
        airs[i].x < random(52, 57)
      );

      if (hitBottom) {
        // 依照目前在中線左右，決定往左或往右滑
        if (airs[i].x + 3 < 32) {
          airs[i].x--;
        } else {
          airs[i].x++;
        }
      } else {
        // 正常往上飄
        airs[i].y--;

        // 中上段加入左右浮動，但避開框底那段特殊區域
        if (airs[i].y <= 40 && !(airs[i].y < 23 && airs[i].y > -2)) {
          airs[i].x += (random(0, 3) - 1);   // -1 / 0 / +1
        }
      }

      // 邊界保護，避免泡泡飄出太多
      if (airs[i].x < 0) airs[i].x = 0;
      if (airs[i].x > 57) airs[i].x = 57;

      airs[i].lastMoveTime = now;
    }

    // 超出畫面就關閉
    if (airs[i].y < -7) {
      airs[i].active = false;
    }
  }
}

// =====================================================
// 畫全部氣泡
// =====================================================
void drawBubbleAirs() {
  for (int i = 0; i < MAX_AIRS; i++) {
    if (!airs[i].active) continue;
    drawSingleAirBubble(airs[i].x, airs[i].y, airs[i].isPink);
  }
}

// =====================================================
// 主模式
// =====================================================
void BubbleMode() {
  initBubble();

  unsigned long now = millis();

  updateBubbleHeroAnim(now);
  spawnBubbleAir(now);
  updateBubbleAirs(now);

  display.fillScreen(BUBBLE_BG_COLOR);

  drawBubbleClock();

  const uint8_t (*currentArt)[16] = useBubble2 ? Bubble2 : Bubble;

  drawBubble(10, 48, currentArt, 16, 16);
  drawBubbleFlip(39, 48, currentArt, 16, 16, true);

  drawBubbleAirs();

  wait_with_display(30);
}
