#include "Mario.h"

// =====================================================
// 螢幕大小
// =====================================================
static const int SCR_W = 64;
static const int SCR_H = 64;

// =====================================================
// 透明色
// =====================================================
static const uint16_t SPRITE_TRANSPARENT = 0x07ff;

// =====================================================
// 馬力歐圖資設定
// 126x28，共 7 張，每張 18x28
// frame:
// 0,1,2 = 行走
// 3     = 待機
// =====================================================
static const int MARIO_SHEET_W = 126;
static const int MARIO_FRAME_W = 18;
static const int MARIO_FRAME_H = 28;
static const int MARIO_FRAMES  = 7;

struct PlayerDef {
  const uint8_t* sheet;
  const uint16_t* palette;
  int sheetW;
  int frameW;
  int frameH;
};

static const PlayerDef PLAYER_MARIO = {
  MARIO_MAIN,
  MARIO_MAIN_PALETTE,
  126,
  18,
  28
};

static const PlayerDef PLAYER_LUIGI = {
  LUIGI_MAIN,
  LUIGI_MAIN_PALETTE,
  126,
  18,
  31
};

static const PlayerDef PLAYER_YOSHI = {
  YOSHI_MAIN,
  YOSHI_MAIN_PALETTE,
  182,
  26,
  31
};


static const PlayerDef* const players[] = {
  &PLAYER_MARIO,
  &PLAYER_LUIGI,
  &PLAYER_YOSHI,
};

static const int PLAYER_COUNT = sizeof(players) / sizeof(players[0]);
static const PlayerDef* currentPlayerDef = &PLAYER_MARIO;



static int marioVictorySet = 0;   // 0 = 345, 1 = 346
static unsigned long marioRestAnimStartMs = 0;
static const unsigned long MARIO_REST_VICTORY_DURATION_MS = 2700;

// =====================================================
// 模式時間
// 跑 60 秒 -> 休息 5分鐘
// =====================================================
static const unsigned long RUN_DURATION_MS  = 60000UL;
static const unsigned long REST_DURATION_MS = 60000*5UL;



// =====================================================
// 怪物資料
// 未來要加更多怪物，再往下補即可
// =====================================================
struct MonsterDef {
  const uint8_t* sheet;
  const uint16_t* palette;
  int sheetW;
  int frameW;
  int frameH;
  int frames;
};

static const MonsterDef MONSTER_1 = {
  MARIO_MST,
  MARIO_MST_PALETTE,
  40,
  20,
  19,
  2
};

static const MonsterDef MONSTER_2 = {
  MARIO_MST2,
  MARIO_MST2_PALETTE,
  32,
  16,
  16,
  2
};

static const MonsterDef MONSTER_3 = {
  MARIO_MST3,
  MARIO_MST3_PALETTE,
  40,
  20,
  32,
  2
};

static const MonsterDef MONSTER_4 = {
  MARIO_MST4,
  MARIO_MST4_PALETTE,
  32,
  16,
  19,
  2
};

static const MonsterDef MONSTER_5 = {
  MARIO_MST5,
  MARIO_MST5_PALETTE,
  32,
  16,
  19,
  2
};

static const MonsterDef MONSTER_6 = {
  MARIO_MST6,
  MARIO_MST6_PALETTE,
  32,
  16,
  16,
  2
};

static const MonsterDef MONSTER_7 = {
  MARIO_MST7,
  MARIO_MST7_PALETTE,
  46,
  23,
  17,
  2
};

static const MonsterDef MONSTER_8 = {
  MARIO_MST8,
  MARIO_MST8_PALETTE,
  32,
  16,
  17,
  2
};


// 怪物清單
static const MonsterDef* const monsters[] = {
  &MONSTER_1,
  &MONSTER_2,
  &MONSTER_3,
  &MONSTER_4,
  &MONSTER_5,
  &MONSTER_6,
  &MONSTER_7,
  &MONSTER_8
};


static const int MONSTER_COUNT = sizeof(monsters) / sizeof(monsters[0]);
static const MonsterDef* currentMonster = nullptr;

// =====================================================
// 背景景物資料
// 加 type，讓休息時只保留雲
// =====================================================
static const int SCENERY_TYPE_GROUND = 0;   // 山、草叢
static const int SCENERY_TYPE_CLOUD  = 1;   // 雲

static unsigned long sceneryLastMoveMs = 0;
static unsigned long sceneryMoveIntervalMs = 180;  //背景物件 移動速度 越大越慢


struct SceneryDef {
  const uint8_t* sprite;
  const uint16_t* palette;
  int w;
  int h;
  int y;
  int speed;
  int type;
};

static const SceneryDef SCENERY_HILL = {
  MARIO_HILL,
  MARIO_HILL_PALETTE,
  28,
  17,
  44,
  1,
  SCENERY_TYPE_GROUND
};

static const SceneryDef SCENERY_CLOUD1 = {
  MARIO_CLOUD1,
  MARIO_CLOUD1_PALETTE,
  17,
  12,
  12,
  1,
  SCENERY_TYPE_CLOUD
};

static const SceneryDef SCENERY_CLOUD2 = {
  MARIO_CLOUD2,
  MARIO_CLOUD2_PALETTE,
  20,
  16,
  10,
  1,
  SCENERY_TYPE_CLOUD
};

static const SceneryDef SCENERY_BUSH = {
  MARIO_BUSH,
  MARIO_BUSH_PALETTE,
  22,
  11,
  50,
  1,
  SCENERY_TYPE_GROUND
};

static const SceneryDef SCENERY_MUSHROOM = {
  MARIO_MUSHROOM,
  MARIO_MUSHROOM_PALETTE,
  38,
  41,
  20,
  1,
  SCENERY_TYPE_GROUND
};

static const SceneryDef SCENERY_BLOCK = {
  MARIO_BLOCK,
  MARIO_BLOCK_PALETTE,
  44,
  16,
  15,
  1,
  SCENERY_TYPE_GROUND
};

static const SceneryDef SCENERY_WAY = {
  MARIO_WAY,
  MARIO_WAY_PALETTE,
  24,
  24,
  37,
  1,
  SCENERY_TYPE_GROUND
};


static const SceneryDef SCENERY_GROUND_TILE = {
  GROUND,
  GROUND_PALETTE,
  8,
  8,
  61,
  1,
  SCENERY_TYPE_GROUND
};

static int groundX = 0;


static const SceneryDef* const sceneryDefs[] = {
  &SCENERY_HILL,
  &SCENERY_CLOUD1,
  &SCENERY_CLOUD2,
  &SCENERY_BUSH,
  &SCENERY_MUSHROOM,
  &SCENERY_BLOCK,
  &SCENERY_WAY,
};

static const int SCENERY_DEF_COUNT = sizeof(sceneryDefs) / sizeof(sceneryDefs[0]);

struct SceneryInstance {
  const SceneryDef* def;
  int x;
  bool active;
};

static const int MAX_SCENERY_INSTANCES = 8;
static SceneryInstance sceneryPool[MAX_SCENERY_INSTANCES];

// =====================================================
// 大模式狀態
// =====================================================
enum MarioModeState {
  MARIO_MODE_RUNNING = 0,
  MARIO_MODE_REST
};

static MarioModeState marioModeState = MARIO_MODE_RUNNING;
static unsigned long marioModeStateStartMs = 0;

// =====================================================
// 怪物演出狀態
// =====================================================
enum MarioSceneState {
  MARIO_SCENE_IDLE = 0,
  MARIO_SCENE_MONSTER_IN,
  MARIO_SCENE_JUMP_UP,
  MARIO_SCENE_STOMP_FLASH,
  MARIO_SCENE_JUMP_DOWN,
  MARIO_SCENE_LAND_HOLD,
  MARIO_SCENE_FINISH
};

static MarioSceneState marioSceneState = MARIO_SCENE_IDLE;

// =====================================================
// 角色位置
// =====================================================
static int marioBaseX = 10;
static int marioX = 10;
static int marioY = 33;
static int marioGroundY = 33;

static int monsterX = 64;
static int monsterY = 42;
static bool monsterVisible = false;

// =====================================================
// 跳躍參數
// =====================================================
static int jumpStartX = 0;
static int jumpStartY = 0;
static int jumpMidX = 0;
static int jumpMidY = 0;
static int jumpEndX = 0;
static int jumpEndY = 0;
static int jumpPeakY = 0;
static unsigned long jumpStartMs = 0;
static unsigned long jumpDurationMs = 320;

// =====================================================
// 怪物閃爍控制
// =====================================================
static unsigned long stompFlashStartMs = 0;
static int stompFlashToggleCount = 0;
static bool stompMonsterDrawVisible = true;

// =====================================================
// 計時
// =====================================================
static unsigned long marioLastAnimMs = 0;
static unsigned long monsterLastAnimMs = 0;
static unsigned long marioSceneStateMs = 0;
static unsigned long monsterNextSpawnMs = 0;
static unsigned long sceneryNextSpawnMs = 0;

static int marioWalkFrame = 0;
static int monsterWalkFrame = 0;


// =====================================================
// 繪製單一 frame sprite
// =====================================================

static inline uint16_t marioReadPaletteColor(
  const uint8_t* sheet,
  const uint16_t* palette,
  uint32_t pixelPos
) {
  uint8_t colorIndex = pgm_read_byte(&(sheet[pixelPos]));
  return pgm_read_word(&(palette[colorIndex]));
}

static void drawMarioBitmapIndexed(
  const uint8_t* bitmap,
  const uint16_t* palette,
  int bitmapW,
  int bitmapH,
  int xOnScreen,
  int yOnScreen
) {
  if (xOnScreen <= -bitmapW || xOnScreen >= SCR_W ||
      yOnScreen <= -bitmapH || yOnScreen >= SCR_H) {
    return;
  }

  for (int y = 0; y < bitmapH; y++) {
    int dy = yOnScreen + y;
    if (dy < 0 || dy >= SCR_H) continue;

    for (int x = 0; x < bitmapW; x++) {
      int dx = xOnScreen + x;
      if (dx < 0 || dx >= SCR_W) continue;

      uint32_t pixelPos =
        (uint32_t)y * (uint32_t)bitmapW +
        (uint32_t)x;

      uint16_t color =
        marioReadPaletteColor(bitmap, palette, pixelPos);

      if (color == SPRITE_TRANSPARENT) continue;

      display.drawPixel(dx, dy, color);
    }
  }
}


static void drawSpriteFrame(
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
    if (dy < 0 || dy >= SCR_H) continue;

    for (int i = 0; i < frameW; i++) {
      int sx = mirrorX ? (frameW - 1 - i) : i;
      int dx = x + i;

      if (dx < 0 || dx >= SCR_W) continue;

      uint32_t pos =
        (uint32_t)j * (uint32_t)sheetW +
        (uint32_t)frameStartX +
        (uint32_t)sx;

      uint16_t color =
        marioReadPaletteColor(sheet, palette, pos);

      if (color == SPRITE_TRANSPARENT) continue;

      display.drawPixel(dx, dy, color);
    }
  }
}



// =====================================================
// 景物池管理
// =====================================================
static void clearSceneryPool() {
  for (int i = 0; i < MAX_SCENERY_INSTANCES; i++) {
    sceneryPool[i].def = nullptr;
    sceneryPool[i].x = 0;
    sceneryPool[i].active = false;
  }
}

static int getRightmostSceneryX() {
  int rightmost = -9999;

  for (int i = 0; i < MAX_SCENERY_INSTANCES; i++) {
    if (!sceneryPool[i].active || sceneryPool[i].def == nullptr) continue;
    int r = sceneryPool[i].x + sceneryPool[i].def->w;
    if (r > rightmost) rightmost = r;
  }

  return rightmost;
}

static bool spawnRandomSceneryAt(int x) {
  int slot = -1;

  for (int i = 0; i < MAX_SCENERY_INSTANCES; i++) {
    if (!sceneryPool[i].active) {
      slot = i;
      break;
    }
  }

  if (slot < 0) return false;

  const SceneryDef* def = sceneryDefs[random(SCENERY_DEF_COUNT)];
  sceneryPool[slot].def = def;
  sceneryPool[slot].x = x;
  sceneryPool[slot].active = true;
  return true;
}

static bool spawnRandomScenery() {
  return spawnRandomSceneryAt(SCR_W);
}

static bool spawnRandomCloudAt(int x) {
  int slot = -1;

  for (int i = 0; i < MAX_SCENERY_INSTANCES; i++) {
    if (!sceneryPool[i].active) {
      slot = i;
      break;
    }
  }

  if (slot < 0) return false;

  const SceneryDef* def = (random(2) == 0) ? &SCENERY_CLOUD1 : &SCENERY_CLOUD2;
  sceneryPool[slot].def = def;
  sceneryPool[slot].x = x;
  sceneryPool[slot].active = true;
  return true;
}

static bool spawnRandomCloud() {
  return spawnRandomCloudAt(SCR_W);
}

static void seedInitialScenery() {
  clearSceneryPool();

  // 跑動一開始先給一些景物，避免畫面太空
  spawnRandomSceneryAt(0);
  spawnRandomSceneryAt(22);
  spawnRandomSceneryAt(46);
}

static void updateScenery(unsigned long nowMs) {

  if (nowMs - sceneryLastMoveMs < sceneryMoveIntervalMs) {
    return;
  }
  sceneryLastMoveMs = nowMs;

//地板移動
if (marioModeState == MARIO_MODE_RUNNING) {
  groundX -= SCENERY_GROUND_TILE.speed;

  if (groundX <= -SCENERY_GROUND_TILE.w) {
    groundX += SCENERY_GROUND_TILE.w;
  }
}

  
  // 先更新目前已存在的景物
  for (int i = 0; i < MAX_SCENERY_INSTANCES; i++) {
    if (!sceneryPool[i].active || sceneryPool[i].def == nullptr) continue;

    const SceneryDef* def = sceneryPool[i].def;

    if (marioModeState == MARIO_MODE_RUNNING) {
      // 跑動模式：全部景物都移動
      sceneryPool[i].x -= def->speed;
    } else {
      // 休息模式：只有雲慢慢飄
      if (def->type == SCENERY_TYPE_CLOUD) {
        sceneryPool[i].x -= 1;
      }
      // 山、草叢不動
    }

    if (sceneryPool[i].x + def->w < 0) {
      sceneryPool[i].active = false;
      sceneryPool[i].def = nullptr;
    }
  }

  // 再決定是否生成新景物
  if (nowMs >= sceneryNextSpawnMs) {
    int rightmost = getRightmostSceneryX();

    if (rightmost <= (SCR_W - random(32, 64))) {     //可生成背景物件的間距
      if (marioModeState == MARIO_MODE_RUNNING) {
        // 跑動模式：所有景物都可生成
        if (spawnRandomScenery()) {
          sceneryNextSpawnMs = nowMs + random(600, 1400);  //背景物件的間距
        } else {
          sceneryNextSpawnMs = nowMs + 120;
        }
      } else {
        // 休息模式：只生成雲，而且頻率低很多
        if (spawnRandomCloud()) {
          sceneryNextSpawnMs = nowMs + random(4000, 7000);   //雲的間距
        } else {
          sceneryNextSpawnMs = nowMs + 300;
        }
      }
    } else {
      sceneryNextSpawnMs = nowMs + 100;
    }
  }
}

// =====================================================
// 畫背景
// =====================================================
static void drawMarioSceneryLayer(bool drawCloudLayer) {
  for (int i = 0; i < MAX_SCENERY_INSTANCES; i++) {
    if (!sceneryPool[i].active || sceneryPool[i].def == nullptr) continue;

    bool isCloud =
      sceneryPool[i].def->type == SCENERY_TYPE_CLOUD;

    if (isCloud != drawCloudLayer) continue;

    drawMarioBitmapIndexed(
      sceneryPool[i].def->sprite,
      sceneryPool[i].def->palette,
      sceneryPool[i].def->w,
      sceneryPool[i].def->h,
      sceneryPool[i].x,
      sceneryPool[i].def->y
    );
  }
}

static void renderMarioSceneBackground() {
  display.fillScreen(SKY_COLOR);

  // 1. 先畫 CLOUD
  // 先畫的會在後面，所以 CLOUD 會被 BLOCK 蓋住
  drawMarioSceneryLayer(true);

  // 2. 再畫其他景物
  // BLOCK 屬於非 CLOUD，所以會在 CLOUD 後面畫，也就是顯示在 CLOUD 前面
  drawMarioSceneryLayer(false);

  // 3. 最後畫地板
  for (int x = groundX; x < SCR_W; x += SCENERY_GROUND_TILE.w) {
    drawMarioBitmapIndexed(
      SCENERY_GROUND_TILE.sprite,
      SCENERY_GROUND_TILE.palette,
      SCENERY_GROUND_TILE.w,
      SCENERY_GROUND_TILE.h,
      x,
      SCENERY_GROUND_TILE.y
    );
  }
}

// =====================================================
// 畫角色
// =====================================================
// =====================================================
// 畫角色
// =====================================================
static void drawMarioSceneActors() {
  bool drawMonsterNow = monsterVisible;

  if (marioSceneState == MARIO_SCENE_STOMP_FLASH) {
    drawMonsterNow = monsterVisible && stompMonsterDrawVisible;
  }

  // 1. 先畫怪物
  if (drawMonsterNow && currentMonster != nullptr) {
    int frameIndex = monsterWalkFrame;

    if (currentMonster->frames <= 1) {
      frameIndex = 0;
    } else {
      frameIndex %= currentMonster->frames;
    }

    drawSpriteFrame(
      currentMonster->sheet,
      currentMonster->palette,
      currentMonster->sheetW,
      currentMonster->frameW,
      currentMonster->frameH,
      frameIndex,
      monsterX,
      monsterY,
      false
    );
  }

  // 2. 計算主角 frame
  int marioFrameIndex = 3;

  if (marioModeState == MARIO_MODE_RUNNING) {
    marioFrameIndex = marioWalkFrame % 3;
  }

  if (marioSceneState == MARIO_SCENE_JUMP_UP ||
      marioSceneState == MARIO_SCENE_STOMP_FLASH ||
      marioSceneState == MARIO_SCENE_JUMP_DOWN ||
      marioSceneState == MARIO_SCENE_LAND_HOLD) {
    marioFrameIndex = 2;
  }

  if (marioModeState == MARIO_MODE_REST &&
      marioSceneState == MARIO_SCENE_IDLE) {
    if (millis() - marioRestAnimStartMs < MARIO_REST_VICTORY_DURATION_MS) {
      int animIndex = ((millis() - marioRestAnimStartMs) / 150) % 3;

      if (marioVictorySet == 0) {
        int frames[3] = {3, 4, 5};
        marioFrameIndex = frames[animIndex];
      } else {
        int frames[3] = {3, 4, 6};
        marioFrameIndex = frames[animIndex];
      }
    } else {
      marioFrameIndex = 3;
    }
  }

  // 3. 畫主角
  int heroDrawY = marioY + (MARIO_FRAME_H - currentPlayerDef->frameH);

  drawSpriteFrame(
    currentPlayerDef->sheet,
    currentPlayerDef->palette,
    currentPlayerDef->sheetW,
    currentPlayerDef->frameW,
    currentPlayerDef->frameH,
    marioFrameIndex,
    marioX,
    heroDrawY,
    false
  );
}

// =====================================================
// 整體重繪
// =====================================================
static void renderMarioScene() {
  renderMarioSceneBackground();
  drawMarioSceneActors();
  drawThemeClockText();
}

// =====================================================
// 馬力歐跑步動畫
// =====================================================
static void updateMarioWalkAnim(unsigned long nowMs) {
  if (marioModeState != MARIO_MODE_RUNNING) return;

  if (nowMs - marioLastAnimMs >= 120) {
    marioLastAnimMs = nowMs;
    marioWalkFrame++;
    if (marioWalkFrame >= 3) marioWalkFrame = 0;
  }
}

// =====================================================
// 怪物走路動畫
// =====================================================
static void updateMonsterWalkAnim(unsigned long nowMs) {
  if (currentMonster == nullptr) return;

  if (nowMs - monsterLastAnimMs >= 180) {
    monsterLastAnimMs = nowMs;

    if (currentMonster->frames <= 1) {
      monsterWalkFrame = 0;
    } else {
      monsterWalkFrame++;
      if (monsterWalkFrame >= currentMonster->frames) {
        monsterWalkFrame = 0;
      }
    }
  }
}

// =====================================================
// 安排下一隻怪物進場
// =====================================================
static void scheduleNextMonsterSpawn(unsigned long nowMs) {
  monsterNextSpawnMs = nowMs + random(1500, 6500);     //怪物進場間格時間
}

// =====================================================
// 進入跑動模式
// =====================================================
static void enterRunningMode() {
  unsigned long nowMs = millis();
  currentPlayerDef = players[random(PLAYER_COUNT)];
  
  marioModeState = MARIO_MODE_RUNNING;
  marioModeStateStartMs = nowMs;

  marioBaseX = 10;
  marioGroundY = 33;
  marioX = marioBaseX;
  marioY = marioGroundY;

  marioSceneState = MARIO_SCENE_IDLE;

  monsterVisible = false;
  currentMonster = nullptr;
  monsterX = SCR_W;
  monsterY = marioGroundY;

  stompFlashStartMs = 0;
  stompFlashToggleCount = 0;
  stompMonsterDrawVisible = true;

  marioWalkFrame = 0;
  monsterWalkFrame = 0;
  marioLastAnimMs = 0;
  monsterLastAnimMs = 0;

  seedInitialScenery();

  sceneryNextSpawnMs = nowMs + random(250, 700);
  scheduleNextMonsterSpawn(nowMs);
}

// =====================================================
// 進入休息模式
// - 馬力歐待機
// - 怪物停止
// - 清掉非雲景物
// - 只讓雲偶爾慢慢飄過
// =====================================================
static void enterRestMode() {
  unsigned long nowMs = millis();

  marioModeState = MARIO_MODE_REST;
  marioModeStateStartMs = nowMs;

  marioVictorySet = random(2);
  marioRestAnimStartMs = nowMs;
  
  marioSceneState = MARIO_SCENE_IDLE;

  marioX = marioBaseX;
  marioY = marioGroundY;

  monsterVisible = false;
  currentMonster = nullptr;
  stompMonsterDrawVisible = true;

  // 清掉非雲景物，讓休息畫面更乾淨
  for (int i = 0; i < MAX_SCENERY_INSTANCES; i++) {
    if (!sceneryPool[i].active || sceneryPool[i].def == nullptr) continue;

    if (sceneryPool[i].def->type != SCENERY_TYPE_CLOUD) {
      sceneryPool[i].active = false;
      sceneryPool[i].def = nullptr;
    }
  }

  // 安排休息時下一朵雲
  sceneryNextSpawnMs = nowMs + random(1000, 2500);
}

// =====================================================
// 啟動一隻怪物演出
// =====================================================
static void startMonsterEncounter(unsigned long nowMs) {
  currentMonster = monsters[random(MONSTER_COUNT)];

  monsterX = SCR_W;
  monsterY = marioGroundY + (MARIO_FRAME_H - currentMonster->frameH);
  monsterVisible = true;

  monsterWalkFrame = 0;
  monsterLastAnimMs = 0;

  stompFlashStartMs = 0;
  stompFlashToggleCount = 0;
  stompMonsterDrawVisible = true;

  marioSceneState = MARIO_SCENE_MONSTER_IN;
  marioSceneStateMs = nowMs;
}

// =====================================================
// 怪物演出狀態機
// =====================================================
static void updateMonsterScene(unsigned long nowMs) {
  if (marioModeState != MARIO_MODE_RUNNING) return;

  switch (marioSceneState) {

    case MARIO_SCENE_IDLE: {
      marioX = marioBaseX;
      marioY = marioGroundY;

      if (!monsterVisible && nowMs >= monsterNextSpawnMs) {
        startMonsterEncounter(nowMs);
      }
      break;
    }

    case MARIO_SCENE_MONSTER_IN: {
      updateMonsterWalkAnim(nowMs);
      monsterX -= 1;

      if (monsterX <= 28 && currentMonster != nullptr) {
        marioSceneState = MARIO_SCENE_JUMP_UP;
        marioSceneStateMs = nowMs;

        jumpStartX = marioX;
        jumpStartY = marioGroundY;

        jumpMidX = monsterX + (currentMonster->frameW / 2) - (MARIO_FRAME_W / 2) + 1;
        jumpMidY = monsterY - MARIO_FRAME_H + 6;

        jumpEndX = monsterX + currentMonster->frameW - 6;
        jumpEndY = marioGroundY;

        jumpPeakY = marioGroundY - 18;

        jumpStartMs = nowMs;
        jumpDurationMs = 320;
      }
      break;
    }

    case MARIO_SCENE_JUMP_UP: {
      updateMonsterWalkAnim(nowMs);

      if (monsterVisible) {
        monsterX -= 1;
      }

      float t = (float)(nowMs - jumpStartMs) / (float)jumpDurationMs;
      if (t > 1.0f) t = 1.0f;

      marioX = jumpStartX + (int)((jumpMidX - jumpStartX) * t);

      float arc = 4.0f * t * (1.0f - t);
      int baseY = jumpStartY + (int)((jumpMidY - jumpStartY) * t);
      marioY = baseY - (int)(arc * (jumpStartY - jumpPeakY));

      if (t >= 1.0f) {
        marioX = jumpMidX;
        marioY = jumpMidY;

        marioSceneState = MARIO_SCENE_STOMP_FLASH;
        marioSceneStateMs = nowMs;

        stompFlashStartMs = nowMs;
        stompFlashToggleCount = 0;
        stompMonsterDrawVisible = false;
      }
      break;
    }

    case MARIO_SCENE_STOMP_FLASH: {
      marioX = jumpMidX;
      marioY = jumpMidY;

      if (nowMs - stompFlashStartMs >= 70) {
        stompFlashStartMs = nowMs;
        stompMonsterDrawVisible = !stompMonsterDrawVisible;
        stompFlashToggleCount++;
      }

      if (stompFlashToggleCount >= 6) {
        monsterVisible = false;

        marioSceneState = MARIO_SCENE_JUMP_DOWN;
        marioSceneStateMs = nowMs;

        jumpStartX = marioX;
        jumpStartY = marioY;
        jumpStartMs = nowMs;
        jumpDurationMs = 260;
      }
      break;
    }

    case MARIO_SCENE_JUMP_DOWN: {
      float t = (float)(nowMs - jumpStartMs) / (float)jumpDurationMs;
      if (t > 1.0f) t = 1.0f;

      marioX = jumpStartX + (int)((jumpEndX - jumpStartX) * t);

      int downPeakY = jumpStartY - 4;
      float arc = 4.0f * t * (1.0f - t);
      int baseY = jumpStartY + (int)((jumpEndY - jumpStartY) * t);
      marioY = baseY - (int)(arc * (jumpStartY - downPeakY));

      if (t >= 1.0f) {
        marioX = jumpEndX;
        marioY = jumpEndY;

        marioSceneState = MARIO_SCENE_LAND_HOLD;
        marioSceneStateMs = nowMs;
      }
      break;
    }

    case MARIO_SCENE_LAND_HOLD: {
      if (nowMs - marioSceneStateMs >= 250) {
        marioSceneState = MARIO_SCENE_FINISH;
        marioSceneStateMs = nowMs;
      }
      break;
    }

    case MARIO_SCENE_FINISH: {
      if (marioX > marioBaseX) {
        marioX -= 1;
        if (marioX < marioBaseX) marioX = marioBaseX;
      } else if (marioX < marioBaseX) {
        marioX += 1;
        if (marioX > marioBaseX) marioX = marioBaseX;
      }

      if (marioX == marioBaseX) {
        marioY = marioGroundY;
        marioSceneState = MARIO_SCENE_IDLE;
        scheduleNextMonsterSpawn(nowMs);
      }
      break;
    }

    default:
      break;
  }
}

// =====================================================
// 更新大模式循環
// =====================================================
static void updateMarioModeCycle(unsigned long nowMs) {
  if (marioModeState == MARIO_MODE_RUNNING) {
    if (nowMs - marioModeStateStartMs >= RUN_DURATION_MS) {
      enterRestMode();
    }
  } else {
    if (nowMs - marioModeStateStartMs >= REST_DURATION_MS) {
      enterRunningMode();
    }
  }
}

// =====================================================
// 初始化
// =====================================================
static void MarioModeInit() {
  if (!ModefirstRun) return;
  randomSeed(millis());
  currentPlayerDef = &PLAYER_MARIO;
  
  marioBaseX = 10;
  marioGroundY = 33;
  marioX = marioBaseX;
  marioY = marioGroundY;

  seedInitialScenery();
  enterRunningMode();
  ModefirstRun = false;
}

// =====================================================
// 主函式
// 跑 60 秒 -> 休息 15 秒 -> 再跑
// =====================================================
void MarioMode() {
  MarioModeInit();

  unsigned long nowMs = millis();

  updateMarioModeCycle(nowMs);
  updateMarioWalkAnim(nowMs);
  updateScenery(nowMs);
  updateMonsterScene(nowMs);

  renderMarioScene();

  wait_with_display(30);
}
