#include "Zelda.h"

// =====================================================
// ZeldaMode.ino
// 背景改為使用 ZELDA_BG_Map[25][38] tile 顯示
//
// 主角移動：
// 0~5   往下
// 6~11  往上
// 12~17 往左
// 右向 = 翻轉往左
// 18~19 待機
//
// 攻擊動畫：
// ZELDA_ATTACK[] 大小 315*45
// 每幀 35*45，共 9 幀
// 0~2   攻擊往下
// 3~5   攻擊往上
// 6~8   攻擊往左
// 右向 = 翻轉往左
//
// 怪物：ZELDA_OCT[]
// 大小 240*16
// 每幀 20*16，共 12 幀
// 0~3   往下
// 4~7   往上
// 8~11  往左
// 右向 = 翻轉往左
//
// 規則：
// 1. 大地圖最多 25 隻怪物，初始化時隨機分布
// 2. 怪物會隨機移動
// 3. 主角碰到怪物時，主角播放攻擊動畫
// 4. 怪物閃爍後消失
// 5. 怪物死亡後 3~7 秒隨機重生，維持最大數量
//
// 重要：
// - ZELDA_BG_Map[][] 只負責畫圖
// - ZELDA_Map[][]    只負責阻擋判斷
// =====================================================


// =====================================================
// 世界設定
// =====================================================
static const int ZELDA_TILE_SIZE = 16;
static const int ZELDA_MAP_COLS  = 38;
static const int ZELDA_MAP_ROWS  = 25;

static const int ZELDA_WORLD_W = ZELDA_MAP_COLS * ZELDA_TILE_SIZE;   // 608
static const int ZELDA_WORLD_H = ZELDA_MAP_ROWS * ZELDA_TILE_SIZE;   // 400

static const int ZELDA_SCREEN_W = 64;
static const int ZELDA_SCREEN_H = 64;


// =====================================================
// 主角一般移動 Sprite 設定
// =====================================================
static const int ZELDA_SPRITE_W = 25;
static const int ZELDA_SPRITE_H = 30;
static const int ZELDA_TOTAL_FRAMES = 20;
static const int ZELDA_SHEET_W = ZELDA_SPRITE_W * ZELDA_TOTAL_FRAMES;


// =====================================================
// 主角攻擊 Sprite 設定
// =====================================================
static const int ZELDA_ATTACK_SPRITE_W = 35;
static const int ZELDA_ATTACK_SPRITE_H = 45;
static const int ZELDA_ATTACK_TOTAL_FRAMES = 9;
static const int ZELDA_ATTACK_SHEET_W = 315;  // 35 * 9


// =====================================================
// 怪物 Sprite 設定
// =====================================================
static const int OCT_SPRITE_W = 20;
static const int OCT_SPRITE_H = 16;
static const int OCT_TOTAL_FRAMES = 12;
static const int OCT_SHEET_W = 240;   // 20 * 12
static const int OCT_MAX = 25;


// =====================================================
// 背景 tile sprite 設定
// =====================================================
static const int BG_TILE_W = 16;
static const int BG_TILE_H = 16;

static const int ZELDA_WALL_SHEET_W = 320;   // 16 * 6
static const int ZELDA_TREE_SHEET_W = 688;  // 16 * 12


// =====================================================
// 主角碰撞盒
// =====================================================
static const int LINK_HITBOX_W = 10;
static const int LINK_HITBOX_H = 8;
static const int LINK_HITBOX_OFFSET_X = 11;
static const int LINK_HITBOX_OFFSET_Y = 22;


// =====================================================
// 怪物碰撞盒
// =====================================================
static const int OCT_HITBOX_W = 12;
static const int OCT_HITBOX_H = 10;
static const int OCT_HITBOX_OFFSET_X = 4;
static const int OCT_HITBOX_OFFSET_Y = 5;


// =====================================================
// 腳部基準點
// 讓一般圖和攻擊圖腳底對齊
// =====================================================
static const int LINK_FOOT_ANCHOR_X   = 16;
static const int LINK_FOOT_ANCHOR_Y   = 27;

static const int ATTACK_FOOT_ANCHOR_X = 27;
static const int ATTACK_FOOT_ANCHOR_Y = 32;


// =====================================================
// 主角移動與動畫參數
// =====================================================
static const float LINK_SPEED = 1.4f;
static const unsigned long LINK_ANIM_INTERVAL = 120;
static const unsigned long LINK_AI_MIN_MS = 1800;
static const unsigned long LINK_AI_MAX_MS = 4200;

static const unsigned long LINK_ATTACK_FRAME_INTERVAL = 90;
static const unsigned long LINK_AFTER_ATTACK_IDLE_MS  = 250;


// =====================================================
// 怪物移動與動畫參數
// =====================================================
static const float OCT_SPEED = 0.6f;
static const unsigned long OCT_ANIM_INTERVAL = 180;
static const unsigned long OCT_AI_MIN_MS = 700;
static const unsigned long OCT_AI_MAX_MS = 1800;
static const unsigned long OCT_BLINK_INTERVAL = 300;
static const int OCT_BLINK_TOGGLES = 10;

static const unsigned long OCT_RESPAWN_MIN_MS = 3000;
static const unsigned long OCT_RESPAWN_MAX_MS = 7000;


// =====================================================
// 主角狀態
// =====================================================
 float linkX = 160.0f;
 float linkY = 120.0f;

static float lastLinkX = 160.0f;
static float lastLinkY = 120.0f;

 int cameraX = 0;
 int cameraY = 0;

// 0 = 下
// 1 = 上
// 2 = 左
// 3 = 右
// 4 = 待機
static int linkState = 4;

static int linkMoveFrame = 0;    // 0~5
static int linkIdleFrame = 18;   // 18~19

static unsigned long linkLastAnimMs = 0;
static unsigned long linkNextActionMs = 0;
static unsigned long linkAttackUntilIdleMs = 0;


// =====================================================
// 主角攻擊狀態
// =====================================================
static bool linkAttackActive = false;
static int linkAttackDir = 0;          // 0下 1上 2左 3右
static int linkAttackFrame = 0;        // 0~2
static unsigned long linkLastAttackMs = 0;


// =====================================================
// 怪物資料結構
// =====================================================
struct OctMonster {
  bool active;
  bool dying;
  bool visible;

  float x;
  float y;

  int dir;                    // 0下 1上 2左 3右 4待機
  int animFrame;              // 0~3

  unsigned long lastAnimMs;
  unsigned long nextActionMs;

  unsigned long blinkLastMs;
  int blinkToggleCount;

  unsigned long respawnAt;
};

static OctMonster octs[OCT_MAX];


// =====================================================
// Debug
// =====================================================
static unsigned long zeldaDebugLastMs = 0;


// =====================================================
// 小工具
// =====================================================
static inline int clamp_i(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static inline float clamp_f(float v, float lo, float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static inline bool rectOverlap(
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
// 讀背景 tile：只負責畫圖
// =====================================================
static uint8_t getZeldaBgTile(int tileX, int tileY) {
  if (tileX < 0 || tileX >= ZELDA_MAP_COLS || tileY < 0 || tileY >= ZELDA_MAP_ROWS) {
    return G__;
  }
  return pgm_read_byte(&(ZELDA_BG_Map[tileY][tileX]));
}


// =====================================================
// 讀碰撞 tile：只負責阻擋
// =====================================================
static uint8_t getZeldaCollisionTile(int tileX, int tileY) {
  if (tileX < 0 || tileX >= ZELDA_MAP_COLS || tileY < 0 || tileY >= ZELDA_MAP_ROWS) {
    return 1;
  }
  return pgm_read_byte(&(ZELDA_Map[tileY][tileX]));
}


// =====================================================
// 主角碰撞判斷：只看 ZELDA_Map[][]
// =====================================================
static bool checkZeldaCollision(float spriteX, float spriteY) {
  int hitX = (int)spriteX + LINK_HITBOX_OFFSET_X;
  int hitY = (int)spriteY + LINK_HITBOX_OFFSET_Y;
  int hitW = LINK_HITBOX_W;
  int hitH = LINK_HITBOX_H;

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
    int tileX = px[i] / ZELDA_TILE_SIZE;
    int tileY = py[i] / ZELDA_TILE_SIZE;

    if (getZeldaCollisionTile(tileX, tileY) == 1) {
      return true;
    }
  }

  return false;
}


// =====================================================
// 怪物碰撞判斷：只看 ZELDA_Map[][]
// =====================================================
static bool checkOctCollision(float spriteX, float spriteY) {
  int hitX = (int)spriteX + OCT_HITBOX_OFFSET_X;
  int hitY = (int)spriteY + OCT_HITBOX_OFFSET_Y;
  int hitW = OCT_HITBOX_W;
  int hitH = OCT_HITBOX_H;

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
    int tileX = px[i] / ZELDA_TILE_SIZE;
    int tileY = py[i] / ZELDA_TILE_SIZE;

    if (getZeldaCollisionTile(tileX, tileY) == 1) {
      return true;
    }
  }

  return false;
}


// =====================================================
// 畫 16x16 tile frame
// =====================================================
static void draw16x16TileFrame(
  const uint8_t *sheet,
  const uint16_t *palette,
  int sheetW,
  int frameIndex,
  int screenX,
  int screenY,
  bool mirrorX,
  bool mirrorY
) {
  if (screenX <= -BG_TILE_W || screenX >= ZELDA_SCREEN_W ||
      screenY <= -BG_TILE_H || screenY >= ZELDA_SCREEN_H) {
    return;
  }

  int frameStartX = frameIndex * BG_TILE_W;

  for (int j = 0; j < BG_TILE_H; j++) {
    int drawY = screenY + j;
    if (drawY < 0 || drawY >= ZELDA_SCREEN_H) continue;

    int srcY = mirrorY ? (BG_TILE_H - 1 - j) : j;

    for (int i = 0; i < BG_TILE_W; i++) {
      int drawX = screenX + i;
      if (drawX < 0 || drawX >= ZELDA_SCREEN_W) continue;

      int srcX = mirrorX ? (BG_TILE_W - 1 - i) : i;

      uint32_t pixelPos =
        (uint32_t)srcY * (uint32_t)sheetW +
        (uint32_t)frameStartX +
        (uint32_t)srcX;

      uint8_t colorIndex = pgm_read_byte(&(sheet[pixelPos]));
      uint16_t color = pgm_read_word(&(palette[colorIndex]));

      display.drawPixel(drawX, drawY, color);
    }
  }
}


// =====================================================
// 畫單一背景 tile
// 不用 struct，直接判斷 tile code
// =====================================================
static void drawZeldaBgTile(uint8_t tileCode, int screenX, int screenY) {
  if (tileCode == G__) return;

  // W11~W13 -> ZELDA_WALL frame 0~2
  if (tileCode >= W11 && tileCode <= W13) {
    int frame = tileCode - W11;
    draw16x16TileFrame(ZELDA_WALL, ZELDA_WALL_PALETTE, ZELDA_WALL_SHEET_W, frame, screenX, screenY, false, false);
    return;
  }

  // W14~W16 -> ZELDA_WALL frame 0~2 垂直反轉
  if (tileCode >= W14 && tileCode <= W16) {
    int frame = tileCode - W14;
    draw16x16TileFrame(ZELDA_WALL, ZELDA_WALL_PALETTE, ZELDA_WALL_SHEET_W, frame, screenX, screenY, false, true);
    return;
  }

  // W17~W19 -> ZELDA_WALL frame 3~5
  if (tileCode >= W17 && tileCode <= W19) {
    int frame = 3 + (tileCode - W17);
    draw16x16TileFrame(ZELDA_WALL, ZELDA_WALL_PALETTE, ZELDA_WALL_SHEET_W, frame, screenX, screenY, false, false);
    return;
  }

  // W20~W22 -> ZELDA_WALL frame 3~5 水平反轉
  if (tileCode >= W20 && tileCode <= W22) {
    int frame = 3 + (tileCode - W20);
    draw16x16TileFrame(ZELDA_WALL, ZELDA_WALL_PALETTE, ZELDA_WALL_SHEET_W, frame, screenX, screenY, true, false);
    return;
  }

  // W23~W34 -> ZELDA_WALL frame 6~17
  if (tileCode >= W23 && tileCode <= W34) {
    int frame = 6 + (tileCode - W23);
    draw16x16TileFrame(ZELDA_WALL, ZELDA_WALL_PALETTE, ZELDA_WALL_SHEET_W, frame, screenX, screenY, false, false);
    return;
  }

  // W35~W46 -> ZELDA_WALL frame 6~17 水平反轉
  if (tileCode >= W35 && tileCode <= W46) {
    int frame = 6 + (tileCode - W35);
    draw16x16TileFrame(ZELDA_WALL, ZELDA_WALL_PALETTE, ZELDA_WALL_SHEET_W, frame, screenX, screenY, true, false);
    return;
  }

  // TUL TUR TDL TDR -> ZELDA_TREE 0 1 2 3
  if (tileCode == TUL) {
    draw16x16TileFrame(ZELDA_TREE, ZELDA_TREE_PALETTE, ZELDA_TREE_SHEET_W, 0, screenX, screenY, false, false);
    return;
  }

  if (tileCode == TUR) {
    draw16x16TileFrame(ZELDA_TREE, ZELDA_TREE_PALETTE, ZELDA_TREE_SHEET_W, 1, screenX, screenY, false, false);
    return;
  }

  if (tileCode == TDL) {
    draw16x16TileFrame(ZELDA_TREE, ZELDA_TREE_PALETTE, ZELDA_TREE_SHEET_W, 2, screenX, screenY, false, false);
    return;
  }

  if (tileCode == TDR) {
    draw16x16TileFrame(ZELDA_TREE, ZELDA_TREE_PALETTE, ZELDA_TREE_SHEET_W, 3, screenX, screenY, false, false);
    return;
  }

  // TM1~HMB -> ZELDA_TREE frame 4 起
  if (tileCode >= TM1 && tileCode <= HMB) {
    int frame = 4 + (tileCode - TM1);
    draw16x16TileFrame(ZELDA_TREE, ZELDA_TREE_PALETTE, ZELDA_TREE_SHEET_W, frame, screenX, screenY, false, false);
    return;
  }
}


// =====================================================
// 背景整幀重繪：改成用 BG_Map tile 顯示
// =====================================================
static void drawZeldaBackgroundCrop() {
  display.fillScreen(0x7732);

  int startTileX = cameraX / ZELDA_TILE_SIZE;
  int startTileY = cameraY / ZELDA_TILE_SIZE;
  int endTileX   = (cameraX + ZELDA_SCREEN_W - 1) / ZELDA_TILE_SIZE;
  int endTileY   = (cameraY + ZELDA_SCREEN_H - 1) / ZELDA_TILE_SIZE;

  for (int ty = startTileY; ty <= endTileY; ty++) {
    for (int tx = startTileX; tx <= endTileX; tx++) {
      uint8_t tileCode = getZeldaBgTile(tx, ty);

      int worldX = tx * ZELDA_TILE_SIZE;
      int worldY = ty * ZELDA_TILE_SIZE;

      int screenX = worldX - cameraX;
      int screenY = worldY - cameraY;

      drawZeldaBgTile(tileCode, screenX, screenY);
    }
  }
}


// =====================================================
// 取得主角一般移動 frame
// =====================================================
static int getLinkFrame(bool &mirrorX) {
  mirrorX = false;

  if (linkState == 4) {
    return linkIdleFrame;
  }

  if (linkState == 0) {
    return 0 + linkMoveFrame;
  } else if (linkState == 1) {
    return 6 + linkMoveFrame;
  } else if (linkState == 2) {
    return 12 + linkMoveFrame;
  } else if (linkState == 3) {
    mirrorX = true;
    return 12 + linkMoveFrame;
  }

  return 18;
}


// =====================================================
// 取得主角攻擊 frame
// =====================================================
static int getLinkAttackFrame(bool &mirrorX) {
  mirrorX = false;

  if (linkAttackDir == 0) {
    return 0 + linkAttackFrame;
  } else if (linkAttackDir == 1) {
    return 3 + linkAttackFrame;
  } else if (linkAttackDir == 2) {
    return 6 + linkAttackFrame;
  } else if (linkAttackDir == 3) {
    mirrorX = true;
    return 6 + linkAttackFrame;
  }

  return 0;
}


// =====================================================
// 取得怪物 frame
// =====================================================
static int getOctFrame(int dir, int animFrame, bool &mirrorX) {
  mirrorX = false;

  if (dir == 0) {
    return 0 + animFrame;
  } else if (dir == 1) {
    return 4 + animFrame;
  } else if (dir == 2) {
    return 8 + animFrame;
  } else if (dir == 3) {
    mirrorX = true;
    return 8 + animFrame;
  }

  return 0;
}


// =====================================================
// 畫主角一般 sprite
// =====================================================
static void drawZeldaSprite(int screenX, int screenY, int frameIndex, bool mirrorX) {
  if (frameIndex < 0 || frameIndex >= ZELDA_TOTAL_FRAMES) return;

  int frameStartX = frameIndex * ZELDA_SPRITE_W;
  uint8_t transparentIndex = pgm_read_byte(&(ZELDA[frameStartX]));

  for (int j = 0; j < ZELDA_SPRITE_H; j++) {
    int drawY = screenY + j;
    if (drawY < 0 || drawY >= ZELDA_SCREEN_H) continue;

    for (int i = 0; i < ZELDA_SPRITE_W; i++) {
      int srcX = mirrorX ? (ZELDA_SPRITE_W - 1 - i) : i;
      int drawX = screenX + i;
      if (drawX < 0 || drawX >= ZELDA_SCREEN_W) continue;

      uint32_t pixelPos =
        (uint32_t)j * (uint32_t)ZELDA_SHEET_W +
        (uint32_t)frameStartX +
        (uint32_t)srcX;

      uint8_t colorIndex = pgm_read_byte(&(ZELDA[pixelPos]));

      if (colorIndex == transparentIndex) continue;

      uint16_t color = pgm_read_word(&(ZELDA_PALETTE[colorIndex]));
      display.drawPixel(drawX, drawY, color);
    }
  }
}

// =====================================================
// 畫主角攻擊 sprite
// =====================================================
static void drawZeldaAttackSprite(int screenX, int screenY, int frameIndex, bool mirrorX) {
  if (frameIndex < 0 || frameIndex >= ZELDA_ATTACK_TOTAL_FRAMES) return;

  int frameStartX = frameIndex * ZELDA_ATTACK_SPRITE_W;
  uint8_t transparentIndex = pgm_read_byte(&(ZELDA_ATTACK[frameStartX]));

  for (int j = 0; j < ZELDA_ATTACK_SPRITE_H; j++) {
    int drawY = screenY + j;
    if (drawY < 0 || drawY >= ZELDA_SCREEN_H) continue;

    for (int i = 0; i < ZELDA_ATTACK_SPRITE_W; i++) {
      int srcX = mirrorX ? (ZELDA_ATTACK_SPRITE_W - 1 - i) : i;
      int drawX = screenX + i;
      if (drawX < 0 || drawX >= ZELDA_SCREEN_W) continue;

      uint32_t pixelPos =
        (uint32_t)j * (uint32_t)ZELDA_ATTACK_SHEET_W +
        (uint32_t)frameStartX +
        (uint32_t)srcX;

      uint8_t colorIndex = pgm_read_byte(&(ZELDA_ATTACK[pixelPos]));

      if (colorIndex == transparentIndex) continue;

      uint16_t color = pgm_read_word(&(ZELDA_ATTACK_PALETTE[colorIndex]));
      display.drawPixel(drawX, drawY, color);
    }
  }
}


// =====================================================
// 畫怪物 sprite
// =====================================================
static void drawOctSprite(int screenX, int screenY, int frameIndex, bool mirrorX) {
  if (frameIndex < 0 || frameIndex >= OCT_TOTAL_FRAMES) return;

  int frameStartX = frameIndex * OCT_SPRITE_W;
  uint8_t transparentIndex = pgm_read_byte(&(ZELDA_OCT[frameStartX]));

  for (int j = 0; j < OCT_SPRITE_H; j++) {
    int drawY = screenY + j;
    if (drawY < 0 || drawY >= ZELDA_SCREEN_H) continue;

    for (int i = 0; i < OCT_SPRITE_W; i++) {
      int srcX = mirrorX ? (OCT_SPRITE_W - 1 - i) : i;
      int drawX = screenX + i;
      if (drawX < 0 || drawX >= ZELDA_SCREEN_W) continue;

      uint32_t pixelPos =
        (uint32_t)j * (uint32_t)OCT_SHEET_W +
        (uint32_t)frameStartX +
        (uint32_t)srcX;

      uint8_t colorIndex = pgm_read_byte(&(ZELDA_OCT[pixelPos]));

      if (colorIndex == transparentIndex) continue;

      uint16_t color = pgm_read_word(&(ZELDA_OCT_PALETTE[colorIndex]));
      display.drawPixel(drawX, drawY, color);
    }
  }
}





// =====================================================
// 啟動攻擊動畫
// =====================================================
static void startLinkAttack(int attackDir) {
  linkAttackActive = true;
  linkAttackDir = attackDir;
  linkAttackFrame = 0;
  linkLastAttackMs = millis();

  linkState = 4;
  linkAttackUntilIdleMs = 0;
}


// =====================================================
// 更新主角攻擊動畫
// =====================================================
static void updateLinkAttackAnimation(unsigned long nowMs) {
  if (!linkAttackActive) return;

  if (nowMs - linkLastAttackMs < LINK_ATTACK_FRAME_INTERVAL) return;
  linkLastAttackMs = nowMs;

  linkAttackFrame++;

  if (linkAttackFrame >= 3) {
    linkAttackFrame = 2;
    linkAttackActive = false;
    linkAttackUntilIdleMs = nowMs + LINK_AFTER_ATTACK_IDLE_MS;
    linkState = 4;
  }
}


// =====================================================
// 怪物出生位置是否合法
// =====================================================
static bool isOctSpawnValid(float x, float y) {
  if (x < 0 || y < 0) return false;
  if (x > (ZELDA_WORLD_W - OCT_SPRITE_W)) return false;
  if (y > (ZELDA_WORLD_H - OCT_SPRITE_H)) return false;

  if (checkOctCollision(x, y)) return false;

  int octHitX = (int)x + OCT_HITBOX_OFFSET_X;
  int octHitY = (int)y + OCT_HITBOX_OFFSET_Y;

  int linkHitX = (int)linkX + LINK_HITBOX_OFFSET_X;
  int linkHitY = (int)linkY + LINK_HITBOX_OFFSET_Y;

  if (rectOverlap(
    octHitX, octHitY, OCT_HITBOX_W, OCT_HITBOX_H,
    linkHitX, linkHitY, LINK_HITBOX_W, LINK_HITBOX_H
  )) {
    return false;
  }

  for (int i = 0; i < OCT_MAX; i++) {
    if (!octs[i].active) continue;

    int ox = (int)octs[i].x + OCT_HITBOX_OFFSET_X;
    int oy = (int)octs[i].y + OCT_HITBOX_OFFSET_Y;

    if (rectOverlap(
      octHitX, octHitY, OCT_HITBOX_W, OCT_HITBOX_H,
      ox, oy, OCT_HITBOX_W, OCT_HITBOX_H
    )) {
      return false;
    }
  }

  return true;
}


// =====================================================
// 初始化單隻怪物
// =====================================================
static void spawnOneOct(int idx) {
  octs[idx].active = false;
  octs[idx].dying = false;
  octs[idx].visible = true;
  octs[idx].x = 0;
  octs[idx].y = 0;
  octs[idx].dir = 4;
  octs[idx].animFrame = 0;
  octs[idx].lastAnimMs = 0;
  octs[idx].nextActionMs = 0;
  octs[idx].blinkLastMs = 0;
  octs[idx].blinkToggleCount = 0;
  octs[idx].respawnAt = 0;

  for (int tries = 0; tries < 200; tries++) {
    int tx = random(0, ZELDA_WORLD_W - OCT_SPRITE_W);
    int ty = random(0, ZELDA_WORLD_H - OCT_SPRITE_H);

    if (isOctSpawnValid((float)tx, (float)ty)) {
      octs[idx].active = true;
      octs[idx].dying = false;
      octs[idx].visible = true;
      octs[idx].x = (float)tx;
      octs[idx].y = (float)ty;
      octs[idx].dir = random(0, 5);
      octs[idx].animFrame = 0;
      octs[idx].lastAnimMs = millis();
      octs[idx].nextActionMs = millis() + random(OCT_AI_MIN_MS, OCT_AI_MAX_MS + 1);
      octs[idx].blinkLastMs = 0;
      octs[idx].blinkToggleCount = 0;
      octs[idx].respawnAt = 0;
      return;
    }
  }
}


// =====================================================
// 初始化全部怪物
// =====================================================
static void initAllOcts() {
  for (int i = 0; i < OCT_MAX; i++) {
    spawnOneOct(i);
  }
}


// =====================================================
// 讓怪物進入閃爍死亡
// =====================================================
static void killOct(int idx) {
  if (idx < 0 || idx >= OCT_MAX) return;
  if (!octs[idx].active) return;
  if (octs[idx].dying) return;

  octs[idx].dying = true;
  octs[idx].visible = false;
  octs[idx].blinkLastMs = millis();
  octs[idx].blinkToggleCount = 1;
  octs[idx].dir = 4;
}


// =====================================================
// 更新怪物閃爍死亡
// =====================================================
static void updateOctBlink(unsigned long nowMs) {
  for (int i = 0; i < OCT_MAX; i++) {
    if (!octs[i].active) continue;
    if (!octs[i].dying) continue;

    if (nowMs - octs[i].blinkLastMs < OCT_BLINK_INTERVAL) continue;
    octs[i].blinkLastMs = nowMs;

    octs[i].visible = !octs[i].visible;
    octs[i].blinkToggleCount++;

    if (octs[i].blinkToggleCount >= OCT_BLINK_TOGGLES) {
      octs[i].active = false;
      octs[i].dying = false;
      octs[i].visible = false;
      octs[i].respawnAt = nowMs + random(OCT_RESPAWN_MIN_MS, OCT_RESPAWN_MAX_MS + 1);
    }
  }
}


// =====================================================
// 更新怪物重生
// =====================================================
static void updateOctRespawn(unsigned long nowMs) {
  for (int i = 0; i < OCT_MAX; i++) {
    if (octs[i].active) continue;
    if (octs[i].dying) continue;
    if (octs[i].respawnAt == 0) continue;

    if (nowMs >= octs[i].respawnAt) {
      spawnOneOct(i);
    }
  }
}


// =====================================================
// 更新怪物動畫
// =====================================================
static void updateOctAnimation(unsigned long nowMs) {
  for (int i = 0; i < OCT_MAX; i++) {
    if (!octs[i].active) continue;
    if (octs[i].dying) continue;

    if (nowMs - octs[i].lastAnimMs < OCT_ANIM_INTERVAL) continue;
    octs[i].lastAnimMs = nowMs;

    if (octs[i].dir == 4) {
      octs[i].animFrame = 0;
    } else {
      octs[i].animFrame++;
      if (octs[i].animFrame >= 4) octs[i].animFrame = 0;
    }
  }
}


// =====================================================
// 更新怪物 AI
// =====================================================
static void updateOctAI(unsigned long nowMs) {
  for (int i = 0; i < OCT_MAX; i++) {
    if (!octs[i].active) continue;
    if (octs[i].dying) continue;

    if (nowMs < octs[i].nextActionMs) continue;

    int dice = random(0, 100);

    if (dice < 20) {
      octs[i].dir = 4;
    } else {
      octs[i].dir = random(0, 4);
    }

    octs[i].nextActionMs = nowMs + random(OCT_AI_MIN_MS, OCT_AI_MAX_MS + 1);
  }
}


// =====================================================
// 更新怪物位移
// =====================================================
static void updateOctMovement() {
  for (int i = 0; i < OCT_MAX; i++) {
    if (!octs[i].active) continue;
    if (octs[i].dying) continue;
    if (octs[i].dir == 4) continue;

    float nx = octs[i].x;
    float ny = octs[i].y;

    if (octs[i].dir == 0) {
      ny += OCT_SPEED;
    } else if (octs[i].dir == 1) {
      ny -= OCT_SPEED;
    } else if (octs[i].dir == 2) {
      nx -= OCT_SPEED;
    } else if (octs[i].dir == 3) {
      nx += OCT_SPEED;
    }

    nx = clamp_f(nx, 0.0f, (float)(ZELDA_WORLD_W - OCT_SPRITE_W));
    ny = clamp_f(ny, 0.0f, (float)(ZELDA_WORLD_H - OCT_SPRITE_H));

    bool blocked = checkOctCollision(nx, ny);

    if (!blocked) {
      for (int j = 0; j < OCT_MAX; j++) {
        if (j == i) continue;
        if (!octs[j].active) continue;

        int ax = (int)nx + OCT_HITBOX_OFFSET_X;
        int ay = (int)ny + OCT_HITBOX_OFFSET_Y;
        int bx = (int)octs[j].x + OCT_HITBOX_OFFSET_X;
        int by = (int)octs[j].y + OCT_HITBOX_OFFSET_Y;

        if (rectOverlap(ax, ay, OCT_HITBOX_W, OCT_HITBOX_H, bx, by, OCT_HITBOX_W, OCT_HITBOX_H)) {
          blocked = true;
          break;
        }
      }
    }

    if (!blocked) {
      octs[i].x = nx;
      octs[i].y = ny;
    } else {
      octs[i].dir = random(0, 5);
      octs[i].nextActionMs = millis() + random(300, 900);
    }
  }
}


// =====================================================
// 檢查主角是否碰到怪物
// 若碰到：主角攻擊、怪物閃爍後消失
// =====================================================
static void checkLinkVsOctCollision(int moveDirForAttack) {
  if (linkAttackActive) return;

  int linkHitX = (int)linkX + LINK_HITBOX_OFFSET_X;
  int linkHitY = (int)linkY + LINK_HITBOX_OFFSET_Y;
  int linkCenterX = linkHitX + (LINK_HITBOX_W / 2);
  int linkCenterY = linkHitY + (LINK_HITBOX_H / 2);

  for (int i = 0; i < OCT_MAX; i++) {
    if (!octs[i].active) continue;
    if (octs[i].dying) continue;

    int octHitX = (int)octs[i].x + OCT_HITBOX_OFFSET_X;
    int octHitY = (int)octs[i].y + OCT_HITBOX_OFFSET_Y;

    if (rectOverlap(
      linkHitX, linkHitY, LINK_HITBOX_W, LINK_HITBOX_H,
      octHitX, octHitY, OCT_HITBOX_W, OCT_HITBOX_H
    )) {
      int octCenterX = octHitX + (OCT_HITBOX_W / 2);
      int octCenterY = octHitY + (OCT_HITBOX_H / 2);

      int dx = octCenterX - linkCenterX;
      int dy = octCenterY - linkCenterY;

      int attackDir = moveDirForAttack;

      if (abs(dx) > abs(dy)) {
        attackDir = (dx >= 0) ? 3 : 2;
      } else {
        attackDir = (dy >= 0) ? 0 : 1;
      }

      linkState = attackDir;
      startLinkAttack(attackDir);
      killOct(i);

      linkNextActionMs = millis() + random(700, 1400);
      return;
    }
  }
}


// =====================================================
// 整幀重繪
// =====================================================
static void renderZeldaFullFrame() {
  drawZeldaBackgroundCrop();

  // 先畫怪物
  for (int i = 0; i < OCT_MAX; i++) {
    if (!octs[i].active) continue;
    if (!octs[i].visible) continue;

    bool mirrorX = false;
    int frameIndex = getOctFrame(octs[i].dir, octs[i].animFrame, mirrorX);

    int screenX = (int)octs[i].x - cameraX;
    int screenY = (int)octs[i].y - cameraY;

    drawOctSprite(screenX, screenY, frameIndex, mirrorX);
  }

  // 再畫主角
  if (linkAttackActive) {
    int worldFootX = (int)linkX + LINK_FOOT_ANCHOR_X;
    int worldFootY = (int)linkY + LINK_FOOT_ANCHOR_Y;

    int attackWorldX = worldFootX - ATTACK_FOOT_ANCHOR_X;
    int attackWorldY = worldFootY - ATTACK_FOOT_ANCHOR_Y;

    int screenX = attackWorldX - cameraX;
    int screenY = attackWorldY - cameraY;

    bool mirrorX = false;
    int frameIndex = getLinkAttackFrame(mirrorX);
    drawZeldaAttackSprite(screenX, screenY, frameIndex, mirrorX);
  } else {
    bool mirrorX = false;
    int frameIndex = getLinkFrame(mirrorX);

    int screenX = (int)linkX - cameraX;
    int screenY = (int)linkY - cameraY;

    drawZeldaSprite(screenX, screenY, frameIndex, mirrorX);
  }

  drawThemeClockText();
}


// =====================================================
// 更新主角一般移動動畫
// =====================================================
static void updateZeldaAnimation(unsigned long nowMs) {
  if (linkAttackActive) return;

  if (nowMs - linkLastAnimMs < LINK_ANIM_INTERVAL) return;
  linkLastAnimMs = nowMs;

  if (linkState == 4) {
    linkIdleFrame = (linkIdleFrame == 18) ? 19 : 18;
  } else {
    linkMoveFrame++;
    if (linkMoveFrame >= 6) linkMoveFrame = 0;
  }
}


// =====================================================
// 更新主角 AI
// =====================================================
static void updateZeldaAI(unsigned long nowMs) {
  if (linkAttackActive) return;

  if (linkAttackUntilIdleMs > nowMs) {
    linkState = 4;
    return;
  }

  if (nowMs < linkNextActionMs) return;

  int dice = random(0, 100);

  if (dice < 28) {
    linkState = 4;
  } else {
    linkState = random(0, 4);
  }

  linkNextActionMs = nowMs + random(LINK_AI_MIN_MS, LINK_AI_MAX_MS + 1);
}


// =====================================================
// 更新主角位移
// 碰到怪物時也播放攻擊動畫
// =====================================================
static void updateZeldaMovement() {
  if (linkAttackActive) return;
  if (linkState == 4) return;

  float nx = linkX;
  float ny = linkY;

  if (linkState == 0) ny += LINK_SPEED;
  else if (linkState == 1) ny -= LINK_SPEED;
  else if (linkState == 2) nx -= LINK_SPEED;
  else if (linkState == 3) nx += LINK_SPEED;

  nx = clamp_f(nx, 0.0f, (float)(ZELDA_WORLD_W - ZELDA_SPRITE_W));
  ny = clamp_f(ny, 0.0f, (float)(ZELDA_WORLD_H - ZELDA_SPRITE_H));

  if (!checkZeldaCollision(nx, ny)) {
    linkX = nx;
    linkY = ny;
    checkLinkVsOctCollision(linkState);
  }
}


// =====================================================
// 更新 Camera
// =====================================================
static void updateZeldaCamera() {
  int linkCenterX = (int)linkX + (ZELDA_SPRITE_W / 2);
  int linkCenterY = (int)linkY + (ZELDA_SPRITE_H / 2)-8;

  cameraX = clamp_i(linkCenterX - (ZELDA_SCREEN_W / 2), 0, ZELDA_WORLD_W - ZELDA_SCREEN_W);
  cameraY = clamp_i(linkCenterY - (ZELDA_SCREEN_H / 2), 0, ZELDA_WORLD_H - ZELDA_SCREEN_H);
}


// =====================================================
// Debug 輸出
// =====================================================
static void debugZeldaPosition() {
  int hitLeft   = (int)linkX + LINK_HITBOX_OFFSET_X;
  int hitTop    = (int)linkY + LINK_HITBOX_OFFSET_Y;
  int hitRight  = hitLeft + LINK_HITBOX_W - 1;
  int hitBottom = hitTop + LINK_HITBOX_H - 1;

  int tileX = hitLeft / ZELDA_TILE_SIZE;
  int tileY = hitTop / ZELDA_TILE_SIZE;

  float tx, ty;
  bool canD = false, canU = false, canL = false, canR = false;

  tx = linkX; ty = linkY + LINK_SPEED;
  canD = !checkZeldaCollision(tx, ty);

  tx = linkX; ty = linkY - LINK_SPEED;
  canU = !checkZeldaCollision(tx, ty);

  tx = linkX - LINK_SPEED; ty = linkY;
  canL = !checkZeldaCollision(tx, ty);

  tx = linkX + LINK_SPEED; ty = linkY;
  canR = !checkZeldaCollision(tx, ty);

  int aliveCount = 0;
  for (int i = 0; i < OCT_MAX; i++) {
    if (octs[i].active) aliveCount++;
  }

  Serial.print("[ZELDA] ");
  Serial.print("link=(");
  Serial.print((int)linkX);
  Serial.print(",");
  Serial.print((int)linkY);
  Serial.print(") ");

  Serial.print("hitBox=(");
  Serial.print(hitLeft);
  Serial.print(",");
  Serial.print(hitTop);
  Serial.print(")-(");
  Serial.print(hitRight);
  Serial.print(",");
  Serial.print(hitBottom);
  Serial.print(") ");

  Serial.print("tile=(");
  Serial.print(tileX);
  Serial.print(",");
  Serial.print(tileY);
  Serial.print(") ");

  Serial.print("colTile=");
  Serial.print(getZeldaCollisionTile(tileX, tileY));

  Serial.print(" state=");
  Serial.print(linkState);

  Serial.print(" attack=");
  Serial.print(linkAttackActive ? 1 : 0);

  Serial.print(" attackDir=");
  Serial.print(linkAttackDir);

  Serial.print(" attackFrame=");
  Serial.print(linkAttackFrame);

  Serial.print(" cam=(");
  Serial.print(cameraX);
  Serial.print(",");
  Serial.print(cameraY);
  Serial.print(") ");

  Serial.print(" can(D,U,L,R)=(");
  Serial.print(canD ? 1 : 0);
  Serial.print(",");
  Serial.print(canU ? 1 : 0);
  Serial.print(",");
  Serial.print(canL ? 1 : 0);
  Serial.print(",");
  Serial.print(canR ? 1 : 0);
  Serial.print(") ");

  Serial.print(" octAlive=");
  Serial.println(aliveCount);
}


// =====================================================
// 初始化 ZeldaMode
// =====================================================
static void ZeldaModeInit() {
  if (!ModefirstRun) return;

  randomSeed(millis());

  linkX = 300.0f;
  linkY = 130.0f;

  if (checkZeldaCollision(linkX, linkY)) {
    linkX = 64.0f;
    linkY = 64.0f;
  }

  lastLinkX = linkX;
  lastLinkY = linkY;

  cameraX = 0;
  cameraY = 0;

  linkState = 4;
  linkMoveFrame = 0;
  linkIdleFrame = 18;

  linkLastAnimMs = 0;
  linkNextActionMs = millis() + random(1000, 2500);
  linkAttackUntilIdleMs = 0;

  linkAttackActive = false;
  linkAttackDir = 0;
  linkAttackFrame = 0;
  linkLastAttackMs = 0;

  initAllOcts();

  zeldaDebugLastMs = 0;

  updateZeldaCamera();
  renderZeldaFullFrame();

  ModefirstRun = false;
}


// =====================================================
// 主模式函式
// =====================================================
void ZeldaMode() {

  TEST();
  ZeldaModeInit();


  unsigned long nowMs = millis();

  updateZeldaAI(nowMs);
  updateZeldaAnimation(nowMs);
  updateLinkAttackAnimation(nowMs);

  updateOctAI(nowMs);
  updateOctAnimation(nowMs);
  updateOctBlink(nowMs);
  updateOctRespawn(nowMs);

  lastLinkX = linkX;
  lastLinkY = linkY;

  updateZeldaMovement();
  updateOctMovement();
  updateZeldaCamera();

/*  if (nowMs - zeldaDebugLastMs >= 10000) {
    zeldaDebugLastMs = nowMs;
    debugZeldaPosition();
  }
*/

  renderZeldaFullFrame();

  wait_with_display(40);
}

void TEST(){


if (Serial.available() > 0) {
    char firstChar = Serial.peek(); // 偷看第一個字元，但不取走

    if (firstChar == 'R' || firstChar == 'r') {
      // 處理重啟指令
      Serial.read(); // 取走 'R'
      Serial.println(F("Restarting ESP32-C6..."));
      Serial.flush();
      delay(200);
      ESP.restart();
    } 
    else if (isDigit(firstChar)) {
      // 如果是數字，代表是座標指令 (例如 200,94)
      int tx = Serial.parseInt();
      int ty = Serial.parseInt();
      
      // 更新全域變數 linkX, linkY (確保 ZeldaMode 抓得到)
      linkX = clamp_f((float)tx, 0.0f, (float)(ZELDA_WORLD_W - ZELDA_SPRITE_W));
      linkY = clamp_f((float)ty, 0.0f, (float)(ZELDA_WORLD_H - ZELDA_SPRITE_H));

      // 如果當前正在 Zelda 模式，立刻更新鏡頭
      // updateZeldaCamera(); // 如果這個函式在 loop 可見的話
      
      Serial.print(F("[JUMP] X:")); Serial.print(linkX);
      Serial.print(F(" Y:")); Serial.println(linkY);
      
      // 清掉剩餘換行符
      while(Serial.available() > 0) { Serial.read(); }
    }
    else {
      // 其他無用字元清掉，避免卡死
      Serial.read();
    }
  }

  
}
