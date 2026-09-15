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

// =====================================================
// 模式時間
// =====================================================
static const unsigned long RUN_DURATION_MS              = 60000UL;        // 正常跑動 60 秒
static const unsigned long REST_DURATION_MS             = 60000UL * 5UL;  // 休息 5 分鐘
static const unsigned long MARIO_VICTORY_DURATION_MS    = 2700UL;         // 勝利動畫時間
static const unsigned long VICTORY_AFTER_WAIT_MS        = 2000UL;         // 勝利結束後等待 2 秒
static const unsigned long CHARACTER_SWAP_WAIT_MS       = 1000UL;         // 舊角色跑出後等待 1 秒

// =====================================================
// 角色進出場參數
// =====================================================
// HERO_ENTER_STEP_PX：角色從左側跑進來的速度
// HERO_EXIT_STEP_PX ：角色往右側跑出螢幕的速度
// 64x64 螢幕建議 2，想慢一點改 1，想快一點改 3。
static const int HERO_ENTER_STEP_PX = 1;
static const int HERO_EXIT_STEP_PX  = 1;

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
static unsigned long sceneryMoveIntervalMs = 120;  // 背景物件移動速度，越大越慢

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
// 新流程：
// ENTERING_RUN  ：一開始角色從左側跑進來
// RUNNING       ：正常跑動，怪物出現，踩怪
// VICTORY       ：跑滿時間後播放勝利動畫
// VICTORY_WAIT  ：勝利動畫結束後等待 2 秒
// EXITING       ：舊角色往右跑出螢幕
// SWAP_WAIT     ：舊角色離開後等待 1 秒
// ENTERING_REST ：下一位隨機角色從左側跑進來
// REST          ：新角色站著等待，休息 5 分鐘
// 休息結束後，這位新角色直接進入 RUNNING
// =====================================================
enum MarioModeState {
  MARIO_MODE_ENTERING_RUN = 0,
  MARIO_MODE_RUNNING,
  MARIO_MODE_VICTORY,
  MARIO_MODE_VICTORY_WAIT,
  MARIO_MODE_EXITING,
  MARIO_MODE_SWAP_WAIT,
  MARIO_MODE_ENTERING_REST,
  MARIO_MODE_REST
};

static MarioModeState marioModeState = MARIO_MODE_ENTERING_RUN;
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
// 踩怪後反彈落地參數
// =====================================================
// 不再讓主角落地後滑回原位。
// 改成踩怪後在 JUMP_DOWN 下落弧線中反彈回原位。
// 這樣 64x64 小螢幕比較不會有倒退跑或輸送帶感。
static const int HERO_LAND_X_OFFSET_FROM_BASE = 0;   // 0 = 落回原位；2~4 = 落在原位右邊一點
static const unsigned long LAND_HOLD_MS = 120;       // 落地後停留時間，越大停越久

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
// 小工具函式
// =====================================================
static bool isSceneryRunningMode() {
  return marioModeState == MARIO_MODE_ENTERING_RUN ||
         marioModeState == MARIO_MODE_RUNNING ||
         marioModeState == MARIO_MODE_EXITING ||
         marioModeState == MARIO_MODE_ENTERING_REST;
}

static bool isHeroRunAnimMode() {
  return marioModeState == MARIO_MODE_ENTERING_RUN ||
         marioModeState == MARIO_MODE_RUNNING ||
         marioModeState == MARIO_MODE_EXITING ||
         marioModeState == MARIO_MODE_ENTERING_REST;
}

static void resetMonsterState() {
  monsterVisible = false;
  currentMonster = nullptr;
  monsterX = SCR_W;
  monsterY = marioGroundY;

  stompFlashStartMs = 0;
  stompFlashToggleCount = 0;
  stompMonsterDrawVisible = true;

  monsterWalkFrame = 0;
  monsterLastAnimMs = 0;
}

static void chooseRandomPlayer(bool avoidSamePlayer) {
  if (PLAYER_COUNT <= 0) return;

  const PlayerDef* oldPlayer = currentPlayerDef;
  const PlayerDef* nextPlayer = players[random(PLAYER_COUNT)];

  if (avoidSamePlayer && PLAYER_COUNT > 1) {
    int guard = 0;

    while (nextPlayer == oldPlayer && guard < 10) {
      nextPlayer = players[random(PLAYER_COUNT)];
      guard++;
    }
  }

  currentPlayerDef = nextPlayer;
}

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

static void clearNonCloudScenery() {
  for (int i = 0; i < MAX_SCENERY_INSTANCES; i++) {
    if (!sceneryPool[i].active || sceneryPool[i].def == nullptr) continue;

    if (sceneryPool[i].def->type != SCENERY_TYPE_CLOUD) {
      sceneryPool[i].active = false;
      sceneryPool[i].def = nullptr;
    }
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

  bool runningScenery = isSceneryRunningMode();

  // 地板移動
  if (runningScenery) {
    groundX -= SCENERY_GROUND_TILE.speed;

    if (groundX <= -SCENERY_GROUND_TILE.w) {
      groundX += SCENERY_GROUND_TILE.w;
    }
  }

  // 先更新目前已存在的景物
  for (int i = 0; i < MAX_SCENERY_INSTANCES; i++) {
    if (!sceneryPool[i].active || sceneryPool[i].def == nullptr) continue;

    const SceneryDef* def = sceneryPool[i].def;

    if (runningScenery) {
      // 進場、跑動、勝利、出場時：全部景物都移動
      sceneryPool[i].x -= def->speed;
    } else {
      // 休息模式：只有雲慢慢飄
      if (def->type == SCENERY_TYPE_CLOUD) {
        sceneryPool[i].x -= 1;
      }
    }

    if (sceneryPool[i].x + def->w < 0) {
      sceneryPool[i].active = false;
      sceneryPool[i].def = nullptr;
    }
  }

  // 再決定是否生成新景物
  if (nowMs >= sceneryNextSpawnMs) {
    int rightmost = getRightmostSceneryX();

    if (rightmost <= (SCR_W - random(32, 64))) {
      if (runningScenery) {
        // 非休息模式：所有景物都可生成
        if (spawnRandomScenery()) {
          sceneryNextSpawnMs = nowMs + random(600, 1400);
        } else {
          sceneryNextSpawnMs = nowMs + 120;
        }
      } else {
        // 休息模式：只生成雲
        if (spawnRandomCloud()) {
          sceneryNextSpawnMs = nowMs + random(4000, 7000);
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
  drawMarioSceneryLayer(true);

  // 2. 再畫其他景物
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

  // 進場、跑動、出場都使用跑步動畫
  if (isHeroRunAnimMode()) {
    marioFrameIndex = marioWalkFrame % 3;
  }

  // 跳躍、踩怪、下落、落地時固定使用跳躍/攻擊姿勢
  if (marioModeState == MARIO_MODE_RUNNING &&
      (marioSceneState == MARIO_SCENE_JUMP_UP ||
       marioSceneState == MARIO_SCENE_STOMP_FLASH ||
       marioSceneState == MARIO_SCENE_JUMP_DOWN ||
       marioSceneState == MARIO_SCENE_LAND_HOLD)) {
    marioFrameIndex = 2;
  }

  // 勝利動畫
  if (marioModeState == MARIO_MODE_VICTORY) {
    int animIndex = ((millis() - marioRestAnimStartMs) / 150) % 3;

    if (marioVictorySet == 0) {
      int frames[3] = {3, 4, 5};
      marioFrameIndex = frames[animIndex];
    } else {
      int frames[3] = {3, 4, 6};
      marioFrameIndex = frames[animIndex];
    }
  }

  // 勝利後等待、休息時，使用待機姿勢
  if (marioModeState == MARIO_MODE_VICTORY_WAIT ||
      marioModeState == MARIO_MODE_REST ||
      marioModeState == MARIO_MODE_SWAP_WAIT) {
    marioFrameIndex = 3;
  }

  // 3. 畫主角
  int heroDrawY = marioY + (MARIO_FRAME_H - currentPlayerDef->frameH);

  // 踩怪後往左反彈落地時，讓主角面向移動方向。
  // 這樣視覺上比較像「反彈回來」，不是倒著移動。
  bool heroMirrorX = false;

  if (marioSceneState == MARIO_SCENE_JUMP_DOWN && marioX > jumpEndX) {
    heroMirrorX = true;
  }

  drawSpriteFrame(
    currentPlayerDef->sheet,
    currentPlayerDef->palette,
    currentPlayerDef->sheetW,
    currentPlayerDef->frameW,
    currentPlayerDef->frameH,
    marioFrameIndex,
    marioX,
    heroDrawY,
    heroMirrorX
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
  if (!isHeroRunAnimMode()) return;

  if (nowMs - marioLastAnimMs >= 100) {
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
  monsterNextSpawnMs = nowMs + random(1500, 6500);
}

// =====================================================
// 進入正常跑動模式
// 注意：這裡不再隨機換角色。
// 角色是在 init 或舊角色跑出後才決定。
// =====================================================
static void enterRunningMode() {
  unsigned long nowMs = millis();

  marioModeState = MARIO_MODE_RUNNING;
  marioModeStateStartMs = nowMs;

  marioX = marioBaseX;
  marioY = marioGroundY;

  marioSceneState = MARIO_SCENE_IDLE;

  resetMonsterState();

  marioWalkFrame = 0;
  monsterWalkFrame = 0;
  marioLastAnimMs = nowMs;
  monsterLastAnimMs = nowMs;

  seedInitialScenery();

  sceneryNextSpawnMs = nowMs + random(250, 700);
  scheduleNextMonsterSpawn(nowMs);
}

// =====================================================
// 進入勝利模式
// 跑滿 RUN_DURATION_MS 後觸發
// =====================================================
static void enterVictoryMode(unsigned long nowMs) {
  marioModeState = MARIO_MODE_VICTORY;
  marioModeStateStartMs = nowMs;

  marioVictorySet = random(2);
  marioRestAnimStartMs = nowMs;

  marioSceneState = MARIO_SCENE_IDLE;

  marioX = marioBaseX;
  marioY = marioGroundY;

  resetMonsterState();
}

// =====================================================
// 進入休息模式
// 新角色已經先跑進來，才會進入這裡。
// 休息時只保留雲，角色在原地待機。
// =====================================================
static void enterRestMode() {
  unsigned long nowMs = millis();

  marioModeState = MARIO_MODE_REST;
  marioModeStateStartMs = nowMs;

  marioSceneState = MARIO_SCENE_IDLE;

  marioX = marioBaseX;
  marioY = marioGroundY;

  resetMonsterState();

  // 清掉非雲景物，讓休息畫面更乾淨
  clearNonCloudScenery();

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

      if (monsterX <= 34 && currentMonster != nullptr) {
        marioSceneState = MARIO_SCENE_JUMP_UP;
        marioSceneStateMs = nowMs;

        jumpStartX = marioX;
        jumpStartY = marioGroundY;

        // 踩怪位置：改用目前角色寬度，讓 Mario / Luigi / Yoshi 都比較準
        jumpMidX = monsterX + (currentMonster->frameW / 2) - (currentPlayerDef->frameW / 2) - 3;
        jumpMidY = monsterY - MARIO_FRAME_H + 6;

        // 踩怪後直接反彈回原本位置落地
        jumpEndX = marioBaseX + HERO_LAND_X_OFFSET_FROM_BASE;
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

        // 反彈回原位的時間。
        // 越大越慢，越小越快。
        jumpDurationMs = 300;
      }
      break;
    }

    case MARIO_SCENE_JUMP_DOWN: {
      float t = (float)(nowMs - jumpStartMs) / (float)jumpDurationMs;
      if (t > 1.0f) t = 1.0f;

      // 從踩怪位置反彈回 marioBaseX
      marioX = jumpStartX + (int)((jumpEndX - jumpStartX) * t);

      int downPeakY = jumpStartY - 6;
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
      // 現在主角已經在下落弧線中回到原位了，
      // 所以落地後不再移動，只短暫停一下就恢復跑步。
      if (nowMs - marioSceneStateMs >= LAND_HOLD_MS) {
        marioX = marioBaseX;
        marioY = marioGroundY;

        marioSceneState = MARIO_SCENE_IDLE;
        marioSceneStateMs = nowMs;

        scheduleNextMonsterSpawn(nowMs);
      }
      break;
    }

    case MARIO_SCENE_FINISH: {
      // 安全備用狀態。
      marioX = marioBaseX;
      marioY = marioGroundY;

      marioSceneState = MARIO_SCENE_IDLE;
      marioSceneStateMs = nowMs;

      scheduleNextMonsterSpawn(nowMs);
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
  switch (marioModeState) {

    case MARIO_MODE_ENTERING_RUN: {
      // 一開始角色從左側跑進來，到達 marioBaseX 後才正式開始 60 秒跑動。
      marioY = marioGroundY;

      if (marioX < marioBaseX) {
        marioX += HERO_ENTER_STEP_PX;
        if (marioX > marioBaseX) marioX = marioBaseX;
      }

      if (marioX == marioBaseX) {
        marioModeState = MARIO_MODE_RUNNING;
        marioModeStateStartMs = nowMs;

        marioSceneState = MARIO_SCENE_IDLE;

        resetMonsterState();
        scheduleNextMonsterSpawn(nowMs);
      }
      break;
    }

    case MARIO_MODE_RUNNING: {
      // 跑滿 60 秒後，等目前踩怪流程結束，再進入勝利動畫。
      // 這樣不會踩怪踩到一半突然切狀態。
      if (nowMs - marioModeStateStartMs >= RUN_DURATION_MS &&
          marioSceneState == MARIO_SCENE_IDLE &&
          !monsterVisible) {
        enterVictoryMode(nowMs);
      }
      break;
    }

    case MARIO_MODE_VICTORY: {
      // 勝利動畫播放完，進入 2 秒等待。
      if (nowMs - marioRestAnimStartMs >= MARIO_VICTORY_DURATION_MS) {
        marioModeState = MARIO_MODE_VICTORY_WAIT;
        marioModeStateStartMs = nowMs;

        marioX = marioBaseX;
        marioY = marioGroundY;
      }
      break;
    }

    case MARIO_MODE_VICTORY_WAIT: {
      // 勝利後等待 2 秒，再往右跑出螢幕。
      if (nowMs - marioModeStateStartMs >= VICTORY_AFTER_WAIT_MS) {
        marioModeState = MARIO_MODE_EXITING;
        marioModeStateStartMs = nowMs;

        marioX = marioBaseX;
        marioY = marioGroundY;

        marioWalkFrame = 0;
        marioLastAnimMs = nowMs;
      }
      break;
    }

    case MARIO_MODE_EXITING: {
      // 舊角色往右跑出螢幕外。
      marioY = marioGroundY;
      marioX += HERO_EXIT_STEP_PX;

      if (marioX >= SCR_W) {
        marioModeState = MARIO_MODE_SWAP_WAIT;
        marioModeStateStartMs = nowMs;

        marioX = SCR_W;
        marioY = marioGroundY;

        resetMonsterState();
      }
      break;
    }

    case MARIO_MODE_SWAP_WAIT: {
      // 舊角色已經離開，等待 1 秒，再選下一位角色跑進來。
      if (nowMs - marioModeStateStartMs >= CHARACTER_SWAP_WAIT_MS) {
        chooseRandomPlayer(true);

        marioX = -currentPlayerDef->frameW;
        marioY = marioGroundY;

        marioWalkFrame = 0;
        marioLastAnimMs = nowMs;

        resetMonsterState();

        marioModeState = MARIO_MODE_ENTERING_REST;
        marioModeStateStartMs = nowMs;
      }
      break;
    }

    case MARIO_MODE_ENTERING_REST: {
      // 下一位角色從左側跑進來，到定位後才正式開始休息 5 分鐘。
      marioY = marioGroundY;

      if (marioX < marioBaseX) {
        marioX += HERO_ENTER_STEP_PX;
        if (marioX > marioBaseX) marioX = marioBaseX;
      }

      if (marioX == marioBaseX) {
        enterRestMode();
      }
      break;
    }

    case MARIO_MODE_REST: {
      // 休息 5 分鐘結束後，同一位已經在畫面中的角色直接開始跑動。
      // 不重新 random，因為角色已經在休息前換好了。
      if (nowMs - marioModeStateStartMs >= REST_DURATION_MS) {
        enterRunningMode();
      }
      break;
    }

    default:
      break;
  }
}

// =====================================================
// 初始化
// =====================================================
static void MarioModeInit() {
  if (!ModefirstRun) return;

  randomSeed(millis());
  unsigned long nowMs = millis();

  marioBaseX = 10;
  marioGroundY = 33;

  // 一開始就隨機選角色，但不瞬間出現。
  // 角色會從左側跑進畫面。
  chooseRandomPlayer(false);

  marioX = -currentPlayerDef->frameW;
  marioY = marioGroundY;

  marioSceneState = MARIO_SCENE_IDLE;
  marioModeState = MARIO_MODE_ENTERING_RUN;
  marioModeStateStartMs = nowMs;

  resetMonsterState();

  marioWalkFrame = 0;
  monsterWalkFrame = 0;
  marioLastAnimMs = nowMs;
  monsterLastAnimMs = nowMs;

  seedInitialScenery();

  sceneryNextSpawnMs = nowMs + random(250, 700);

  ModefirstRun = false;
}

// =====================================================
// 主函式
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
