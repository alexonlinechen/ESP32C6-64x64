
// =====================================================
// DogMode.ino - 多狗通用版
//
// 地圖：沿用 Zelda_map / Zelda_BG_MAP
// 狗圖資：Dog_A[]、Dog_B[]、Dog_C[]
//
// 動作定義（全部狗共用）
// 0~5   往下
// 6~9   往上
// 10~15 往左
// 往右 = 翻轉往左
// 16~19 等待（靜態隨機抽一張，不連續動畫）
// 20~22 睡覺（靜態隨機抽一張，不連續動畫）
//
// 規則：
// 1. 預設 Camera 鎖定 Dog_A
// 2. 所有狗會在大地圖隨機活動
// 3. 目前被 Camera 鎖定的狗，碰到其他狗時，Camera 會切到那隻狗
// 4. Camera 切換冷卻 1 分鐘
// 5. 行為機率：睡覺 20%、待機 40%、行走 40%
// =====================================================


#include "Zelda.h"
#include "Dog.h"
extern int cameraX;
extern int cameraY;

// =====================================================
// DogMode.ino - 多狗通用版
//
// 地圖：沿用 Zelda_map / Zelda_BG_MAP
// 狗圖資：Dog_A[]、Dog_B[]、Dog_C[]
//
// 動作定義（全部狗共用）
// 0~5   往下
// 6~9   往上
// 10~15 往左
// 往右 = 翻轉往左
// 16~19 等待（靜態隨機抽一張，不連續動畫）
// 20~22 睡覺（靜態隨機抽一張，不連續動畫）
//
// 規則：
// 1. 預設 Camera 鎖定 Dog_A
// 2. 所有狗會在大地圖隨機活動
// 3. 目前被 Camera 鎖定的狗，碰到其他狗時，Camera 會切到那隻狗
// 4. Camera 切換冷卻 1 分鐘
// 5. 行為機率：睡覺 20%、待機 40%、行走 40%
// =====================================================


// =====================================================
// 世界設定
// =====================================================
static const int DOG_TILE_SIZE = 16;
static const int DOG_MAP_COLS  = 38;
static const int DOG_MAP_ROWS  = 25;

static const int DOG_WORLD_W = DOG_MAP_COLS * DOG_TILE_SIZE;   // 608
static const int DOG_WORLD_H = DOG_MAP_ROWS * DOG_TILE_SIZE;   // 400

static const int DOG_SCREEN_W = 64;
static const int DOG_SCREEN_H = 64;


// =====================================================
// 狗 Sprite 設定
// 依你目前 Dog_A / Dog_B / Dog_C 規格
// =====================================================
static const int DOG_SPRITE_W = 34;
static const int DOG_SPRITE_H = 31;
static const int DOG_TOTAL_FRAMES = 23;
static const int DOG_SHEET_W = 782;


// =====================================================
// 狗碰撞盒
// =====================================================
static const int DOG_HITBOX_W = 16;
static const int DOG_HITBOX_H = 10;
static const int DOG_HITBOX_OFFSET_X = 10;
static const int DOG_HITBOX_OFFSET_Y = 25;


// =====================================================
// 狗移動與動畫參數
// =====================================================
static const float DOG_SPEED = 1.2f;
static const unsigned long DOG_ANIM_INTERVAL = 150;
static const unsigned long DOG_AI_MIN_MS = 5000;
static const unsigned long DOG_AI_MAX_MS = 10000;
static const unsigned long DOG_IDLE_PICK_INTERVAL  = 10000;
static const unsigned long DOG_SLEEP_PICK_INTERVAL = 10000;


// =====================================================
// Camera 切換冷卻
// =====================================================
static const unsigned long DOG_CAM_SWITCH_COOLDOWN_MS = 60000;   // 1 分鐘


// =====================================================
// 多狗數量
// 之後你要加狗，改這裡即可
// =====================================================
static const int DOG_COUNT = 4;


// =====================================================
// 狗狀態
// 0 = 下
// 1 = 上
// 2 = 左
// 3 = 右
// 4 = 待機
// 5 = 睡覺
// =====================================================
struct DogActor {
  const uint8_t* sprite;
  const uint16_t* palette;
  const char* name;

  float x;
  float y;
  float lastX;
  float lastY;

  int state;
  int moveFrame;
  int stillFrame;

  int facingDir;
  bool stillMirrorX;

  unsigned long lastAnimMs;
  unsigned long nextActionMs;
  unsigned long lastStillPickMs;
};


// =====================================================
// 圖資列表
// 之後加 Dog_D / Dog_E 時往下加
// =====================================================
static const uint8_t* dogSpriteSheets[DOG_COUNT] = {
  Dog_A,
  Dog_B,
  Dog_C,
  Dog_D
};

static const uint16_t* dogPalettes[DOG_COUNT] = {
  Dog_A_PALETTE,
  Dog_B_PALETTE,
  Dog_C_PALETTE,
  Dog_D_PALETTE
};

static const char* dogNames[DOG_COUNT] = {
  "A",
  "B",
  "C",
  "D"
};

static DogActor dogs[DOG_COUNT];


// =====================================================
// Camera 狀態
// =====================================================
static int dogCameraTarget = 0;   // 預設鎖定 Dog_A
static int dogCameraX = 0;
static int dogCameraY = 0;
static unsigned long dogCameraLastSwitchMs = 0;

// 新增：記錄這一輪碰撞後要切到哪隻狗
static int dogPendingCameraTarget = -1;


// =====================================================
// Debug
// =====================================================
static unsigned long dogDebugLastMs = 0;


// =====================================================
// 小工具
// =====================================================
static inline int dog_clamp_i(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static inline float dog_clamp_f(float v, float lo, float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static inline uint32_t dog_bgIndex(int x, int y) {
  return (uint32_t)y * (uint32_t)DOG_WORLD_W + (uint32_t)x;
}

static inline bool dogRectOverlap(
  int ax, int ay, int aw, int ah,
  int bx, int by, int bw, int bh
) {
  if (ax + aw <= bx) return false;
  if (bx + bw <= ax) return false;
  if (ay + ah <= by) return false;
  if (by + bh <= ay) return false;
  return true;
}


// =====================================================
// tile 讀取
// =====================================================
static uint8_t getDogTile(int tileX, int tileY) {
  if (tileX < 0 || tileX >= DOG_MAP_COLS || tileY < 0 || tileY >= DOG_MAP_ROWS) {
    return 1;
  }
  return pgm_read_byte(&(ZELDA_Map[tileY][tileX]));
}


// =====================================================
// 地圖碰撞
// =====================================================
static bool checkDogCollision(float spriteX, float spriteY) {
  int hitX = (int)spriteX + DOG_HITBOX_OFFSET_X;
  int hitY = (int)spriteY + DOG_HITBOX_OFFSET_Y;
  int hitW = DOG_HITBOX_W;
  int hitH = DOG_HITBOX_H;

  int px[4] = {
    hitX,
    hitX + hitW - 1,
    hitX,
    hitX + hitW - 1
  };

  int py[4] = {
    hitY,
    hitY,
    hitY + hitH - 1,
    hitY + hitH - 1
  };

  for (int i = 0; i < 4; i++) {
    int tileX = px[i] / DOG_TILE_SIZE;
    int tileY = py[i] / DOG_TILE_SIZE;

    if (getDogTile(tileX, tileY) == 1) {
      return true;
    }
  }

  return false;
}


// =====================================================
// 狗與狗碰撞
// =====================================================
static bool checkDogVsDogCollision(float ax, float ay, float bx, float by) {
  int aHitX = (int)ax + DOG_HITBOX_OFFSET_X;
  int aHitY = (int)ay + DOG_HITBOX_OFFSET_Y;
  int bHitX = (int)bx + DOG_HITBOX_OFFSET_X;
  int bHitY = (int)by + DOG_HITBOX_OFFSET_Y;

  return dogRectOverlap(
    aHitX, aHitY, DOG_HITBOX_W, DOG_HITBOX_H,
    bHitX, bHitY, DOG_HITBOX_W, DOG_HITBOX_H
  );
}


// =====================================================
// 隨機抽待機 / 睡覺圖
// =====================================================
static void pickDogStillFrameByIndex(int dogIndex) {
  DogActor &dog = dogs[dogIndex];

  if (dog.state == 4) {
    dog.stillFrame = random(16, 20);   // 16~19
  } else if (dog.state == 5) {
    dog.stillFrame = random(20, 23);   // 20~22
  }
}


// =====================================================
// 依照原本朝向，決定待機 / 睡覺圖是否水平翻轉
// 原圖資面向左：
// facingDir == 左 -> 不翻
// facingDir == 右 -> 翻轉
// facingDir == 上或下 -> 隨機翻轉
// =====================================================
static void updateDogStillMirrorByIndex(int dogIndex) {
  DogActor &dog = dogs[dogIndex];

  if (dog.facingDir == 2) {
    dog.stillMirrorX = false;
  } else if (dog.facingDir == 3) {
    dog.stillMirrorX = true;
  } else {
    dog.stillMirrorX = (random(0, 2) == 1);
  }
}


// =====================================================
// 取得 frame
// =====================================================
static int getDogFrameByIndex(int dogIndex, bool &mirrorX) {
  DogActor &dog = dogs[dogIndex];
  mirrorX = false;

  if (dog.state == 0) {
    return 0 + dog.moveFrame;
  } else if (dog.state == 1) {
    return 6 + dog.moveFrame;
  } else if (dog.state == 2) {
    return 10 + dog.moveFrame;
  } else if (dog.state == 3) {
    mirrorX = true;
    return 10 + dog.moveFrame;
  } else if (dog.state == 4 || dog.state == 5) {
    mirrorX = dog.stillMirrorX;
    return dog.stillFrame;
  }

  return 16;
}


// =====================================================
// 畫單隻狗 sprite
// =====================================================
static void drawDogSprite(
  const uint8_t* spriteSheet,
  const uint16_t* palette,
  int screenX,
  int screenY,
  int frameIndex,
  bool mirrorX
) {
  if (frameIndex < 0 || frameIndex >= DOG_TOTAL_FRAMES) return;

  int frameStartX = frameIndex * DOG_SPRITE_W;

  // 使用每一幀左上角像素當透明色索引
  uint8_t transparentIndex = pgm_read_byte(&(spriteSheet[frameStartX]));

  for (int j = 0; j < DOG_SPRITE_H; j++) {
    int drawY = screenY + j;
    if (drawY < 0 || drawY >= DOG_SCREEN_H) continue;

    for (int i = 0; i < DOG_SPRITE_W; i++) {
      int srcX = mirrorX ? (DOG_SPRITE_W - 1 - i) : i;
      int drawX = screenX + i;
      if (drawX < 0 || drawX >= DOG_SCREEN_W) continue;

      uint32_t pixelPos =
        (uint32_t)j * (uint32_t)DOG_SHEET_W +
        (uint32_t)frameStartX +
        (uint32_t)srcX;

      uint8_t colorIndex = pgm_read_byte(&(spriteSheet[pixelPos]));

      if (colorIndex == transparentIndex) continue;

      uint16_t color = pgm_read_word(&(palette[colorIndex]));
      display.drawPixel(drawX, drawY, color);
    }
  }
}




// =====================================================
// 畫單隻狗
// =====================================================
static void drawOneDogByIndex(int dogIndex) {
  bool mirrorX = false;
  int frameIndex = getDogFrameByIndex(dogIndex, mirrorX);

  int screenX = (int)dogs[dogIndex].x - dogCameraX;
  int screenY = (int)dogs[dogIndex].y - dogCameraY;

drawDogSprite(
  dogs[dogIndex].sprite,
  dogs[dogIndex].palette,
  screenX,
  screenY,
  frameIndex,
  mirrorX
);
}


// =====================================================
// 整幀重繪
// =====================================================
static void renderDogFullFrame() {

  // 將 dogCamera 切換給 ZeldaCam
  cameraX = dogCameraX;
  cameraY = dogCameraY;

  drawZeldaBackgroundCrop();  // 套用 ZELDA 的地圖

  for (int i = 0; i < DOG_COUNT; i++) {
    drawOneDogByIndex(i);
  }

  drawThemeClockText();
}


// =====================================================
// 更新走路動畫 / 靜態圖抽選
// =====================================================
static void updateDogAnimationByIndex(int dogIndex, unsigned long nowMs) {
  DogActor &dog = dogs[dogIndex];

  if (dog.state >= 0 && dog.state <= 3) {
    if (nowMs - dog.lastAnimMs < DOG_ANIM_INTERVAL) return;
    dog.lastAnimMs = nowMs;

    if (dog.state == 0) {
      dog.moveFrame++;
      if (dog.moveFrame >= 6) dog.moveFrame = 0;
    } else if (dog.state == 1) {
      dog.moveFrame++;
      if (dog.moveFrame >= 4) dog.moveFrame = 0;
    } else if (dog.state == 2 || dog.state == 3) {
      dog.moveFrame++;
      if (dog.moveFrame >= 6) dog.moveFrame = 0;
    }
    return;
  }

  if (dog.state == 4) {
    if (nowMs - dog.lastStillPickMs >= DOG_IDLE_PICK_INTERVAL) {
      dog.lastStillPickMs = nowMs;
      pickDogStillFrameByIndex(dogIndex);
    }
  } else if (dog.state == 5) {
    if (nowMs - dog.lastStillPickMs >= DOG_SLEEP_PICK_INTERVAL) {
      dog.lastStillPickMs = nowMs;
      pickDogStillFrameByIndex(dogIndex);
    }
  }
}


// =====================================================
// 更新 AI
// 機率：行走 40%、待機 40%、睡覺 20%
// =====================================================
static void updateDogAIByIndex(int dogIndex, unsigned long nowMs) {
  DogActor &dog = dogs[dogIndex];

  if (nowMs < dog.nextActionMs) return;

  int dice = random(0, 100);

  if (dice < 40) {
    // 行走 40%
    dog.state = random(0, 4);
    dog.moveFrame = 0;
  } else if (dice < 80) {
    // 待機 40%
    dog.state = 4;
    pickDogStillFrameByIndex(dogIndex);
    updateDogStillMirrorByIndex(dogIndex);
    dog.lastStillPickMs = nowMs;
  } else {
    // 睡覺 20%
    dog.state = 5;
    pickDogStillFrameByIndex(dogIndex);
    updateDogStillMirrorByIndex(dogIndex);
    dog.lastStillPickMs = nowMs;
  }

  dog.nextActionMs = nowMs + random(DOG_AI_MIN_MS, DOG_AI_MAX_MS + 1);
}


// =====================================================
// 找出這一步會撞到哪隻狗
// =====================================================
static int findCollidingDogIndex(int ignoreIndex, float nx, float ny) {
  for (int i = 0; i < DOG_COUNT; i++) {
    if (i == ignoreIndex) continue;

    if (checkDogVsDogCollision(nx, ny, dogs[i].x, dogs[i].y)) {
      return i;
    }
  }
  return -1;
}


// =====================================================
// 更新位移
// =====================================================
static void updateDogMovementByIndex(int dogIndex) {
  DogActor &dog = dogs[dogIndex];

  if (dog.state < 0 || dog.state > 3) return;

  float nx = dog.x;
  float ny = dog.y;

  if (dog.state == 0) {
    dog.facingDir = 0;
    ny += DOG_SPEED;
  } else if (dog.state == 1) {
    dog.facingDir = 1;
    ny -= DOG_SPEED;
  } else if (dog.state == 2) {
    dog.facingDir = 2;
    nx -= DOG_SPEED;
  } else if (dog.state == 3) {
    dog.facingDir = 3;
    nx += DOG_SPEED;
  }

  nx = dog_clamp_f(nx, 0.0f, (float)(DOG_WORLD_W - DOG_SPRITE_W));
  ny = dog_clamp_f(ny, 0.0f, (float)(DOG_WORLD_H - DOG_SPRITE_H));

  bool blocked = false;

  if (checkDogCollision(nx, ny)) {
    blocked = true;
  }

  int hitDogIndex = -1;
  if (!blocked) {
    hitDogIndex = findCollidingDogIndex(dogIndex, nx, ny);
    if (hitDogIndex >= 0) {
      blocked = true;

      // 只要這一步會撞到別隻狗，就視為「遇到」
      // 若目前鎖定的是自己，切到對方
      if (dogIndex == dogCameraTarget) {
        dogPendingCameraTarget = hitDogIndex;
      }
      // 若目前鎖定的是對方，切到自己
      else if (hitDogIndex == dogCameraTarget) {
        dogPendingCameraTarget = dogIndex;
      }
    }
  }

  if (!blocked) {
    dog.x = nx;
    dog.y = ny;
  } else {
    if (random(0, 100) < 70) {
      dog.state = 4;
    } else {
      dog.state = 5;
    }
    pickDogStillFrameByIndex(dogIndex);
    updateDogStillMirrorByIndex(dogIndex);
    dog.lastStillPickMs = millis();
    dog.nextActionMs = millis() + random(800, 1800);
  }
}


// =====================================================
// 更新 Camera
// =====================================================
static void updateDogCamera() {
  DogActor &camDog = dogs[dogCameraTarget];

  int dogCenterX = (int)camDog.x + (DOG_SPRITE_W / 2);
  int dogCenterY = (int)camDog.y + (DOG_SPRITE_H / 2) -8;

  dogCameraX = dog_clamp_i(dogCenterX - (DOG_SCREEN_W / 2), 0, DOG_WORLD_W - DOG_SCREEN_W);
  dogCameraY = dog_clamp_i(dogCenterY - (DOG_SCREEN_H / 2), 0, DOG_WORLD_H - DOG_SCREEN_H);
}


// =====================================================
// 多狗通用 Camera 切換
// =====================================================
static void updateDogCameraSwitch(unsigned long nowMs) {
  if (nowMs - dogCameraLastSwitchMs < DOG_CAM_SWITCH_COOLDOWN_MS) {
    dogPendingCameraTarget = -1;
    return;
  }

  // 優先用「本輪即將碰撞」事件來切換
  if (dogPendingCameraTarget >= 0 && dogPendingCameraTarget < DOG_COUNT) {
    if (dogPendingCameraTarget != dogCameraTarget) {
      dogCameraTarget = dogPendingCameraTarget;
      dogCameraLastSwitchMs = nowMs;
    }
    dogPendingCameraTarget = -1;
    return;
  }

  // 保底：若真的重疊也可切
  for (int i = 0; i < DOG_COUNT; i++) {
    if (i == dogCameraTarget) continue;

    if (checkDogVsDogCollision(dogs[dogCameraTarget].x, dogs[dogCameraTarget].y, dogs[i].x, dogs[i].y)) {
      dogCameraTarget = i;
      dogCameraLastSwitchMs = nowMs;
      dogPendingCameraTarget = -1;
      return;
    }
  }

  dogPendingCameraTarget = -1;
}


// =====================================================
// Debug
// =====================================================
static void debugDogPosition() {
  Serial.print("[DOG] camTarget=");
  Serial.print(dogs[dogCameraTarget].name);
  Serial.print(" cam=(");
  Serial.print(dogCameraX);
  Serial.print(",");
  Serial.print(dogCameraY);
  Serial.print(") ");

  for (int i = 0; i < DOG_COUNT; i++) {
    Serial.print(dogs[i].name);
    Serial.print("=(");
    Serial.print((int)dogs[i].x);
    Serial.print(",");
    Serial.print((int)dogs[i].y);
    Serial.print(") ");
  }

  Serial.println();
}


// =====================================================
// 初始化單隻狗
// =====================================================
static void initOneDogByIndex(int dogIndex, float startX, float startY) {
  DogActor &dog = dogs[dogIndex];

dog.sprite = dogSpriteSheets[dogIndex];
dog.palette = dogPalettes[dogIndex];
dog.name = dogNames[dogIndex];

  dog.x = startX;
  dog.y = startY;
  dog.lastX = startX;
  dog.lastY = startY;

  if (checkDogCollision(dog.x, dog.y)) {
    dog.x = 64.0f;
    dog.y = 64.0f;
  }

  dog.state = 4;
  dog.moveFrame = 0;
  dog.stillFrame = 16;

  dog.facingDir = 2;      // 預設面向左
  dog.stillMirrorX = false;

  dog.lastAnimMs = 0;
  dog.nextActionMs = millis() + random(1000, 2500);
  dog.lastStillPickMs = millis();

  pickDogStillFrameByIndex(dogIndex);
  updateDogStillMirrorByIndex(dogIndex);
}


// =====================================================
// 初始化全部狗
// 你現在有 4 隻，後面要加狗就在這裡補位置
// =====================================================
static void initAllDogs() {
  initOneDogByIndex(0, 300.0f, 140.0f);   // A
  initOneDogByIndex(1, 300.0f, 50.0f);    // B
  initOneDogByIndex(2, 65.0f, 50.0f);     // C
  initOneDogByIndex(3, 480.0f, 340.0f);   // D

  // 若一開始重疊，簡單往其他地方移
  for (int i = 0; i < DOG_COUNT; i++) {
    for (int j = i + 1; j < DOG_COUNT; j++) {
      if (checkDogVsDogCollision(dogs[i].x, dogs[i].y, dogs[j].x, dogs[j].y)) {
        dogs[j].x = 64.0f + 40.0f * j;
        dogs[j].y = 120.0f + 20.0f * j;

        if (checkDogCollision(dogs[j].x, dogs[j].y)) {
          dogs[j].x = 64.0f;
          dogs[j].y = 64.0f;
        }
      }
    }
  }
}


// =====================================================
// 初始化 DogMode
// =====================================================
static void DogModeInit() {
  if (!ModefirstRun) return;

  randomSeed(millis());

  initAllDogs();

  dogCameraTarget = random(0, DOG_COUNT);
  dogCameraX = 0;
  dogCameraY = 0;
  dogCameraLastSwitchMs = millis() - DOG_CAM_SWITCH_COOLDOWN_MS;
  dogPendingCameraTarget = -1;

  dogDebugLastMs = 0;

  updateDogCamera();
  renderDogFullFrame();

  ModefirstRun = false;
}


// =====================================================
// DogMode 主函式
// =====================================================
void DogMode() {
  DogModeInit();

  unsigned long nowMs = millis();

  for (int i = 0; i < DOG_COUNT; i++) {
    updateDogAIByIndex(i, nowMs);
  }

  for (int i = 0; i < DOG_COUNT; i++) {
    updateDogAnimationByIndex(i, nowMs);
  }

  for (int i = 0; i < DOG_COUNT; i++) {
    dogs[i].lastX = dogs[i].x;
    dogs[i].lastY = dogs[i].y;
  }

  for (int i = 0; i < DOG_COUNT; i++) {
    updateDogMovementByIndex(i);
  }

  updateDogCameraSwitch(nowMs);
  updateDogCamera();

  if (nowMs - dogDebugLastMs >= 10000) {
    dogDebugLastMs = nowMs;
    debugDogPosition();
  }

  renderDogFullFrame();

  wait_with_display(40);
}
