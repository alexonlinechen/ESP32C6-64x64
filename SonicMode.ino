#include "Sonic.h"

// =====================================================
// Sonic Mode
//
// 需要 Sonic.h 內有：
// SONIC              384x32，16 frames，每格 24x32
// SONIC_BG           16x96
// SONIC_GN1          16x6
// SONIC_GN2          16x5
// SONIC_GN0          16x3
// SONIC_TREE         30x60
// SONIC_RING         32x16，2 frames，每格 16x16
// SONIC_MONITOR      24x14，2 frames，每格 12x14
// SONIC_SPRING       32x16，2 frames，每格 16x16
// SONIC_SIGN_BASE1   6x2
// SONIC_SIGN_BASE2   6x16
// SONIC_SIGN1        48x30
// SONIC_SIGN2        32x30
// SONIC_SIGN3        8x31
// SONIC_SIGN4        48x30
// SONIC_SIGN5        48x30
// =====================================================


// =====================================================
// 螢幕大小
// =====================================================
static const int SONIC_SCR_W = 64;
static const int SONIC_SCR_H = 64;


// =====================================================
// 透明色
// RGB(0,204,238) -> RGB565 = 0x067D
// =====================================================
static const uint16_t SONIC_TRANSPARENT = 0x067D;


// =====================================================
// 圖資尺寸
// =====================================================
static const int SONIC_SHEET_W = 384;
static const int SONIC_SHEET_H = 32;
static const int SONIC_FRAME_W = 24;
static const int SONIC_FRAME_H = 32;
static const int SONIC_FRAME_COUNT = 16;

static const int SONIC_BG_W = 16;
static const int SONIC_BG_H = 96;
static const int SONIC_BG_VIEW_Y = SONIC_BG_H - SONIC_SCR_H;

static const int SONIC_GN1_W = 16;
static const int SONIC_GN1_H = 6;

static const int SONIC_GN2_W = 16;
static const int SONIC_GN2_H = 5;

static const int SONIC_GN0_W = 16;
static const int SONIC_GN0_H = 3;

static const int SONIC_TERRAIN_TILE_W = 16;

static const int SONIC_TREE_W = 30;
static const int SONIC_TREE_H = 60;

static const int SONIC_RING_SHEET_W = 32;
static const int SONIC_RING_SHEET_H = 16;
static const int SONIC_RING_FRAME_W = 16;
static const int SONIC_RING_FRAME_H = 16;
static const int SONIC_RING_FRAME_COUNT = 2;

static const int SONIC_MONITOR_SHEET_W = 24;
static const int SONIC_MONITOR_SHEET_H = 14;
static const int SONIC_MONITOR_FRAME_W = 12;
static const int SONIC_MONITOR_FRAME_H = 14;
static const int SONIC_MONITOR_FRAME_COUNT = 2;

static const int SONIC_SPRING_SHEET_W = 32;
static const int SONIC_SPRING_SHEET_H = 16;
static const int SONIC_SPRING_FRAME_W = 16;
static const int SONIC_SPRING_FRAME_H = 16;
static const int SONIC_SPRING_FRAME_COUNT = 2;

static const int SONIC_SIGN_BASE1_W = 6;
static const int SONIC_SIGN_BASE1_H = 2;

static const int SONIC_SIGN_BASE2_W = 6;
static const int SONIC_SIGN_BASE2_H = 16;

static const int SONIC_SIGN1_W = 48;
static const int SONIC_SIGN1_H = 30;

static const int SONIC_SIGN2_W = 32;
static const int SONIC_SIGN2_H = 30;

static const int SONIC_SIGN3_W = 8;
static const int SONIC_SIGN3_H = 31;

static const int SONIC_SIGN4_W = 48;
static const int SONIC_SIGN4_H = 30;

static const int SONIC_SIGN5_W = 48;
static const int SONIC_SIGN5_H = 30;


// =====================================================
// 可調參數
// =====================================================

// -----------------------------------------------------
// 整體畫面更新速度
// -----------------------------------------------------

// 每一幀畫面更新間隔。
// 數值越小，動畫越流暢，但負擔也越高。
static const unsigned long SONIC_FRAME_DELAY_MS = 30UL;


// -----------------------------------------------------
// 模式等待時間
// -----------------------------------------------------

// 第一次進入 SonicMode 後，等待多久才開始 Sonic 動畫。
static const unsigned long SONIC_FIRST_START_WAIT_MS = 5000UL;

// 一輪 Sonic 動畫結束後，等待多久才重新開始。
// 300000ms = 5 分鐘。
static const unsigned long SONIC_RESTART_WAIT_MS = 300000UL;


// -----------------------------------------------------
// Sonic 各狀態動畫速度
// -----------------------------------------------------

// Sonic 待機動畫 frame1 / frame2 的切換速度。
static const unsigned long SONIC_IDLE_FRAME_MS = 500UL;

// Sonic 起跑前 ready 動畫 frame0 / frame3 / frame4 的切換速度。
static const unsigned long SONIC_READY_FRAME_MS = 200UL;

// ready 動畫完成後，停留多久才開始跑步。
static const unsigned long SONIC_READY_HOLD_MS = 2000UL;

// Sonic 跑步動畫 frame5 / frame6 / frame7 / frame8 的切換速度。
static const unsigned long SONIC_RUN_FRAME_MS = 20UL;

// 結果為 WIN 時，Sonic 勝利動畫的切換速度。
static const unsigned long SONIC_WIN_FRAME_MS = 250UL;


// -----------------------------------------------------
// Sonic 跑步與背景速度
// -----------------------------------------------------

// Sonic 跑步時，地形與物件往左移動的速度。
// 數值越大，跑得越快。
static const float SONIC_RUN_SPEED_PX_PER_SEC = 42.0f;

// 背景捲動比例。
// 小於 1 會讓背景比地面慢，產生遠景效果。
static const float SONIC_BG_SCROLL_RATIO = 0.35f;

// Sonic 從待機位置移動到固定跑步位置的速度。
static const float SONIC_RUN_TO_X_SPEED = 8.0f;

// 結尾階段 Sonic 往右跑到終點牌前的速度。
static const float SONIC_END_MOVE_SPEED = 22.0f;


// -----------------------------------------------------
// 跑步總時間與 END 修正
// -----------------------------------------------------

// Sonic 跑步多久後準備進入 END 結尾流程。
// 60000ms = 60 秒。
static const unsigned long SONIC_RUN_BEFORE_END_MS = 60000UL;

// 修正 END 前遇到高地形瞬間變平地的問題。
// 到達結尾時間後，會等地形完整跑完，再額外跑一小段平地才進 END。
static const unsigned long SONIC_END_FLAT_AFTER_TERRAIN_MS = 1200UL;


// -----------------------------------------------------
// 山丘 / 地形生成參數
// -----------------------------------------------------

// 進入跑步後，第一次山丘出現前的等待時間。
static const unsigned long SONIC_HILL_FIRST_DELAY_MS = 5000UL;

// 兩個山丘之間的最短間隔。
static const unsigned long SONIC_HILL_GAP_MIN_MS = 2800UL;

// 兩個山丘之間的最長間隔。
static const unsigned long SONIC_HILL_GAP_MAX_MS = 5200UL;

// 地形3 / 地形4 / 地形5 的平台最短格數。
// 每一格是 16px。
static const uint8_t SONIC_HILL_PLATEAU_MIN_BLOCKS = 3;

// 地形3 / 地形4 / 地形5 的平台最長格數。
// 每一格是 16px。
static const uint8_t SONIC_HILL_PLATEAU_MAX_BLOCKS = 6;

// 山丘從畫面右側生成的位置。
static const int SONIC_HILL_SPAWN_X = 76;

// 每往右 16px，高度改變 2px。
static const int SONIC_HILL_STEP_Y = 2;

// 地形2 / 地形3 / 地形5 的基本高度階數。
// 從 y=56 升到 y=40，共 8 階。
static const int SONIC_HILL_BASE_STEP_COUNT = 8;

// 地形4 比地形3 再高 4 階。
// 地形4 最高點會從 y=40 再升到 y=32。
static const int SONIC_HILL4_EXTRA_STEP_COUNT = 4;

// 地形5 的懸崖缺口寬度。
// 2 格 = 32px。
// 目前建議用 2，Sonic 比較不會有踩空感。
// 如果想更刺激再改成 3，也就是 48px。
static const int SONIC_HILL5_GAP_BLOCKS = 2;


// Sonic 跳過地形5缺口時，額外往上拋的高度。
// 數值越大，拋物線越明顯。
// 缺口改成 32px 後，用 14 會比 12 更有跳過去的感覺。
static const int SONIC_HILL5_JUMP_EXTRA_LIFT_Y = 14;


// -----------------------------------------------------
// TREE 背景物件參數
// -----------------------------------------------------

// 進入跑步後，第一棵樹出現前的等待時間。
static const unsigned long SONIC_TREE_FIRST_DELAY_MS = 3500UL;

// 樹再次出現的最短間隔。
static const unsigned long SONIC_TREE_GAP_MIN_MS = 3000UL;

// 樹再次出現的最長間隔。
static const unsigned long SONIC_TREE_GAP_MAX_MS = 8500UL;

// 樹從畫面右側生成的位置。
static const int SONIC_TREE_SPAWN_X = 72;

// 樹用來判斷貼地高度的 X 錨點。
// 預設抓樹寬度的一半。
static const int SONIC_TREE_ANCHOR_X = SONIC_TREE_W / 2;


// TREE 遇到地形5懸崖缺口時的安全距離。
// TREE 本身寬度較大，如果太靠近缺口也會看起來像長在洞裡。
// 數值越大，TREE 越不容易出現在懸崖附近。
static const int SONIC_TREE_GAP_SAFE_MARGIN_X = 4;

// -----------------------------------------------------
// RING / MONITOR / SPRING 共用生成鎖
// -----------------------------------------------------

// 互動物件消失後，至少等待多久才允許下一個互動物件生成。
// 用來避免 RING、MONITOR、SPRING 互相重疊或連續貼太近。
static const unsigned long SONIC_OBJECT_CLEAR_AFTER_MS = 1500UL;


// -----------------------------------------------------
// RING 金環物件參數
// -----------------------------------------------------

// 進入跑步後，第一個金環出現前的等待時間。
static const unsigned long SONIC_RING_FIRST_DELAY_MS = 2500UL;

// 金環再次出現的最短間隔。
static const unsigned long SONIC_RING_GAP_MIN_MS = 2200UL;

// 金環再次出現的最長間隔。
static const unsigned long SONIC_RING_GAP_MAX_MS = 6200UL;

// 金環從畫面右側生成的位置。
static const int SONIC_RING_SPAWN_X = 74;

// 金環用來判斷地形高度的 X 錨點。
static const int SONIC_RING_ANCHOR_X = SONIC_RING_FRAME_W / 2;

// 金環距離地形表面的高度。
// 數值越大，金環越高。
static const int SONIC_RING_FLOAT_OFFSET_Y = 5;

// Sonic 碰到金環後，frame1 閃爍持續時間。
static const unsigned long SONIC_RING_COLLECT_BLINK_MS = 900UL;

// 金環碰撞後的閃爍速度。
static const unsigned long SONIC_RING_BLINK_INTERVAL_MS = 80UL;


// -----------------------------------------------------
// MONITOR 物件參數
// -----------------------------------------------------

// 進入跑步後，第一個 MONITOR 出現前的等待時間。
static const unsigned long SONIC_MONITOR_FIRST_DELAY_MS = 6500UL;

// MONITOR 再次出現的最短間隔。
static const unsigned long SONIC_MONITOR_GAP_MIN_MS = 7000UL;

// MONITOR 再次出現的最長間隔。
static const unsigned long SONIC_MONITOR_GAP_MAX_MS = 13000UL;

// MONITOR 從畫面右側生成的位置。
static const int SONIC_MONITOR_SPAWN_X = 74;

// Sonic 距離 MONITOR 多近時，開始執行 frame10~14 球化動畫。
// 數值越大，Sonic 越早開始變球。
static const int SONIC_MONITOR_ATTACK_TRIGGER_DISTANCE = 34;

// Sonic frame10~14 球化動畫的逐幀速度。
static const unsigned long SONIC_MONITOR_BALL_FRAME_MS = 80UL;

// MONITOR 被撞後閃爍消失的總時間。
static const unsigned long SONIC_MONITOR_BLINK_MS = 700UL;

// MONITOR 被撞後的閃爍速度。
static const unsigned long SONIC_MONITOR_BLINK_INTERVAL_MS = 70UL;

// Sonic 撞完 MONITOR 後落地所需時間。
static const unsigned long SONIC_MONITOR_LAND_MS = 360UL;

// Sonic 落地後，frame14~10 反動畫速度。
static const unsigned long SONIC_MONITOR_RECOVER_FRAME_MS = 70UL;

// Sonic 變球撞 MONITOR 時，最高上升量。
// 數值越大，球化撞擊時跳得越高。
static const int SONIC_MONITOR_BALL_LIFT_Y = 12;


// -----------------------------------------------------
// SPRING 彈簧物件參數
// -----------------------------------------------------

// 進入跑步後，第一個彈簧出現前的等待時間。
static const unsigned long SONIC_SPRING_FIRST_DELAY_MS = 9000UL;

// 彈簧再次出現的最短間隔。
static const unsigned long SONIC_SPRING_GAP_MIN_MS = 8500UL;

// 彈簧再次出現的最長間隔。
static const unsigned long SONIC_SPRING_GAP_MAX_MS = 16000UL;

// 彈簧從畫面右側生成的位置。
static const int SONIC_SPRING_SPAWN_X = 74;

// Sonic 距離彈簧多近時，開始執行 frame10~14 球化動畫。
// 數值越大，Sonic 越早開始變球。
static const int SONIC_SPRING_ATTACK_TRIGGER_DISTANCE = 34;

// Sonic frame10~14 球化動畫的逐幀速度。
static const unsigned long SONIC_SPRING_BALL_FRAME_MS = 80UL;

// Sonic 變球碰彈簧前，最高上升量。
static const int SONIC_SPRING_BALL_LIFT_Y = 8;

// 彈簧往上彈起時間。
// 這段會快速把場景往下拉，營造 Sonic 往上飛。
static const unsigned long SONIC_SPRING_JUMP_UP_MS = 560UL;

// 彈簧到最高點後，短暫停留多久。
// 加這段可以避免最高點一到就立刻切回落下，讓大跳比較有重量。
static const unsigned long SONIC_SPRING_PEAK_HOLD_MS = 240UL;

// 彈簧落下時間。
// 數值越大，落地越慢、越有重力感。
static const unsigned long SONIC_SPRING_FALL_MS = 2000UL;

// 彈簧大跳最高點時，場景往下偏移的最大高度。
// 數值越大，Sonic 看起來跳越高。
static const int SONIC_SPRING_JUMP_MAX_SCENE_OFFSET_Y = 44;

// 彈簧大跳期間，場景往左捲動速度倍率。
// 數值越大，越像往前飛。
// 不建議太高，避免彈簧太快離開畫面導致動作不完整。
static const float SONIC_SPRING_JUMP_SCROLL_MULTIPLIER = 1.75f;


// -----------------------------------------------------
// SIGN 結尾牌參數
// -----------------------------------------------------

// 結尾牌從右側進場的速度。
static const float SONIC_SIGN_MOVE_SPEED = 38.0f;

// 結尾牌旋轉動畫的逐幀速度。
static const unsigned long SONIC_SIGN_SPIN_FRAME_MS = 120UL;

// 結尾牌旋轉總時間。
static const unsigned long SONIC_SIGN_SPIN_TOTAL_MS = 5200UL;

// Sonic 到達結尾牌前後，等待多久才轉身。
static const unsigned long SONIC_TURN_WAIT_MS = 1000UL;

// 結果牌出現後，等待多久才進入 Sonic 結果動畫。
static const unsigned long SONIC_RESULT_SIGN_HOLD_MS = 1000UL;

// Sonic 結果動畫停留時間。
static const unsigned long SONIC_RESULT_HOLD_MS = 6000UL;


// -----------------------------------------------------
// 結果與文字
// -----------------------------------------------------

// Sonic 最後出現 WIN 結果的機率。
// 70 = 70%。
static const uint8_t SONIC_WIN_CHANCE_PERCENT = 70;

// 是否顯示時鐘文字。
// true = 顯示，false = 不顯示。
static const bool SONIC_DRAW_CLOCK_TEXT = true;


// =====================================================
// 場景座標
// =====================================================
static const int SONIC_IDLE_X = 6;
static const int SONIC_IDLE_Y = 30;

static const int SONIC_RUN_LOCK_X = 14;
static const int SONIC_END_X = 41;

static const int SONIC_BASE_Y = 30;

static const int SONIC_FLAT_GRASS_TOP_Y = 58;

static const int SONIC_HILL_START_TOP_Y = 56;
static const int SONIC_HILL_GN0_START_OFFSET_Y = SONIC_GN2_H;

static const int SONIC_HILL3_MAX_TOP_Y = 40;
static const int SONIC_HILL4_MAX_TOP_Y = 32;
static const int SONIC_HILL_MIN_TOP_Y = SONIC_HILL4_MAX_TOP_Y;

// 地形5 缺口用，不是真正座標。
static const int SONIC_TERRAIN_GAP_TOP_Y = 999;

static const int SONIC_SIGN_BASE1_OFFSET_X = 21;
static const int SONIC_SIGN_BASE1_Y = 14;

static const int SONIC_SIGN_MAIN_OFFSET_X = 0;
static const int SONIC_SIGN_MAIN_Y = 16;

static const int SONIC_SIGN_BASE2_OFFSET_X = 21;
static const int SONIC_SIGN_BASE2_Y = 46;


// =====================================================
// 狀態機
// =====================================================
static const uint8_t SONIC_STATE_INIT_WAIT = 0;
static const uint8_t SONIC_STATE_READY_ANIM = 1;
static const uint8_t SONIC_STATE_READY_HOLD = 2;
static const uint8_t SONIC_STATE_RUN = 3;
static const uint8_t SONIC_STATE_END_SIGN_IN = 4;
static const uint8_t SONIC_STATE_END_FINISH = 5;
static const uint8_t SONIC_STATE_RESULT_SIGN_WAIT = 6;
static const uint8_t SONIC_STATE_RESULT_SONIC_ANIM = 7;
static const uint8_t SONIC_STATE_RESTART_WAIT = 8;

static uint8_t sonicState = SONIC_STATE_INIT_WAIT;
static unsigned long sonicStateStartMs = 0;
static unsigned long sonicLastUpdateMs = 0;


// =====================================================
// Sonic 狀態
// =====================================================
static float sonicX = SONIC_IDLE_X;
static float sonicY = SONIC_IDLE_Y;

static bool sonicFlipX = false;

static float sonicBgScrollX = 0.0f;
static float sonicGroundScrollX = 0.0f;

// 彈簧大跳時，背景與 GN 往下偏移，Sonic 本身不真的往上移。
static int sonicSceneOffsetY = 0;

static unsigned long sonicRunStartMs = 0;
static unsigned long sonicNextHillMs = 0;

static bool sonicEndRequested = false;
static unsigned long sonicEndFlatStartMs = 0;

// RING / MONITOR / SPRING 共用生成鎖。
// 任一互動物件消失後，會延後一小段時間才允許下一個互動物件出現。
static unsigned long sonicNextObjectAllowedMs = 0;

static bool sonicResultWin = true;
static bool sonicResultDecided = false;

static unsigned long sonicEndGoalReachMs = 0;


// =====================================================
// 小山丘狀態
// =====================================================
static const uint8_t SONIC_HILL_NONE = 0;
static const uint8_t SONIC_HILL_TYPE_2 = 2;
static const uint8_t SONIC_HILL_TYPE_3 = 3;
static const uint8_t SONIC_HILL_TYPE_4 = 4;
static const uint8_t SONIC_HILL_TYPE_5 = 5;

static bool sonicHillActive = false;
static uint8_t sonicHillType = SONIC_HILL_NONE;
static float sonicHillX = 0.0f;
static int sonicHillWidth = 0;
static uint8_t sonicHillPlateauBlocks = 0;


// =====================================================
// TREE 狀態
// =====================================================
static bool sonicTreeActive = false;
static float sonicTreeX = 0.0f;
static unsigned long sonicNextTreeMs = 0;


// =====================================================
// RING 狀態
// =====================================================
static bool sonicRingActive = false;
static bool sonicRingCollected = false;
static float sonicRingX = 0.0f;
static unsigned long sonicNextRingMs = 0;
static unsigned long sonicRingCollectedMs = 0;

static void scheduleNextSonicRing(unsigned long nowMs);


// =====================================================
// MONITOR 狀態
// =====================================================
static const uint8_t SONIC_MONITOR_ACTION_NONE = 0;
static const uint8_t SONIC_MONITOR_ACTION_BALL_UP = 1;
static const uint8_t SONIC_MONITOR_ACTION_ATTACK = 2;
static const uint8_t SONIC_MONITOR_ACTION_LAND = 3;
static const uint8_t SONIC_MONITOR_ACTION_RECOVER = 4;

static bool sonicMonitorActive = false;
static bool sonicMonitorCollected = false;
static float sonicMonitorX = 0.0f;
static uint8_t sonicMonitorFrame = 0;
static unsigned long sonicNextMonitorMs = 0;
static unsigned long sonicMonitorCollectedMs = 0;

static uint8_t sonicMonitorAction = SONIC_MONITOR_ACTION_NONE;
static unsigned long sonicMonitorActionStartMs = 0;

static void scheduleNextSonicMonitor(unsigned long nowMs);


// =====================================================
// SPRING 狀態
// =====================================================
static const uint8_t SONIC_SPRING_ACTION_NONE = 0;
static const uint8_t SONIC_SPRING_ACTION_BALL_UP = 1;
static const uint8_t SONIC_SPRING_ACTION_ATTACK = 2;
static const uint8_t SONIC_SPRING_ACTION_JUMP = 3;
static const uint8_t SONIC_SPRING_ACTION_FALL = 4;

static bool sonicSpringActive = false;
static bool sonicSpringPressed = false;
static float sonicSpringX = 0.0f;
static unsigned long sonicNextSpringMs = 0;

static uint8_t sonicSpringAction = SONIC_SPRING_ACTION_NONE;
static unsigned long sonicSpringActionStartMs = 0;

static void scheduleNextSonicSpring(unsigned long nowMs);


// =====================================================
// SIGN 狀態
// =====================================================
static float sonicSignX = SONIC_SCR_W;


// =====================================================
// 小工具
// =====================================================
static long sonicWrapIndex(long value, int maxValue) {
  if (maxValue <= 0) return 0;

  long result = value % maxValue;
  if (result < 0) result += maxValue;
  return result;
}


static int sonicClampInt(int value, int minValue, int maxValue) {
  if (value < minValue) return minValue;
  if (value > maxValue) return maxValue;
  return value;
}


static bool sonicAabbOverlap(
  int ax,
  int ay,
  int aw,
  int ah,
  int bx,
  int by,
  int bw,
  int bh
) {
  if (ax + aw <= bx) return false;
  if (bx + bw <= ax) return false;
  if (ay + ah <= by) return false;
  if (by + bh <= ay) return false;
  return true;
}


// =====================================================
// 互動物件共用生成鎖
// =====================================================
static void markSonicInteractiveObjectClearance(unsigned long nowMs) {
  sonicNextObjectAllowedMs = nowMs + SONIC_OBJECT_CLEAR_AFTER_MS;
}


static bool isSonicInteractiveObjectBusy() {
  if (sonicRingActive) {
    return true;
  }

  if (sonicMonitorActive || sonicMonitorAction != SONIC_MONITOR_ACTION_NONE) {
    return true;
  }

  if (sonicSpringActive || sonicSpringAction != SONIC_SPRING_ACTION_NONE) {
    return true;
  }

  return false;
}


static bool canSpawnSonicInteractiveObject(unsigned long nowMs) {
  if (nowMs < sonicNextObjectAllowedMs) {
    return false;
  }

  if (isSonicInteractiveObjectBusy()) {
    return false;
  }

  return true;
}


// =====================================================
// RGB565 sprite 繪製
// =====================================================
static void drawSonicSpriteFrameScreen(
  int x,
  int y,
  int sheetW,
  int sheetH,
  const uint16_t* sprite,
  int srcX,
  int srcY,
  int frameW,
  int frameH,
  bool flipX,
  bool useTransparent
) {
  if (!sprite) return;

  for (int j = 0; j < frameH; j++) {
    int dy = y + j;
    if (dy < 0 || dy >= SONIC_SCR_H) continue;

    int sy = srcY + j;
    if (sy < 0 || sy >= sheetH) continue;

    for (int i = 0; i < frameW; i++) {
      int dx = x + i;
      if (dx < 0 || dx >= SONIC_SCR_W) continue;

      int localSrcX = i;
      if (flipX) localSrcX = frameW - 1 - i;

      int sx = srcX + localSrcX;
      if (sx < 0 || sx >= sheetW) continue;

      uint32_t pos =
        (uint32_t)sy * (uint32_t)sheetW +
        (uint32_t)sx;

      uint16_t color = pgm_read_word(&(sprite[pos]));

      if (useTransparent && color == SONIC_TRANSPARENT) continue;

      display.drawPixel(dx, dy, color);
    }
  }
}


static void drawSonicImageScreen(
  int x,
  int y,
  int w,
  int h,
  const uint16_t* sprite,
  bool flipX,
  bool useTransparent
) {
  drawSonicSpriteFrameScreen(
    x,
    y,
    w,
    h,
    sprite,
    0,
    0,
    w,
    h,
    flipX,
    useTransparent
  );
}


static void drawSonicFrame(int frameIndex, int x, int y, bool flipX) {
  frameIndex = sonicClampInt(frameIndex, 0, SONIC_FRAME_COUNT - 1);

  drawSonicSpriteFrameScreen(
    x,
    y,
    SONIC_SHEET_W,
    SONIC_SHEET_H,
    SONIC,
    frameIndex * SONIC_FRAME_W,
    0,
    SONIC_FRAME_W,
    SONIC_FRAME_H,
    flipX,
    true
  );
}


// =====================================================
// 背景繪製
// 背景繪製
// =====================================================
static void drawSonicBackground() {
  long scrollX = (long)sonicBgScrollX;

  for (int y = 0; y < SONIC_SCR_H; y++) {
    int srcY = SONIC_BG_VIEW_Y + y - sonicSceneOffsetY;

    if (srcY < 0) srcY = 0;
    if (srcY >= SONIC_BG_H) srcY = SONIC_BG_H - 1;

    for (int x = 0; x < SONIC_SCR_W; x++) {
      int srcX = (int)sonicWrapIndex((long)x + scrollX, SONIC_BG_W);

      uint32_t pos =
        (uint32_t)srcY * (uint32_t)SONIC_BG_W +
        (uint32_t)srcX;

      uint16_t color = pgm_read_word(&(SONIC_BG[pos]));
      display.drawPixel(x, y, color);
    }
  }
}


static void drawSonicBackgroundRect(int x, int y, int w, int h) {
  long scrollX = (long)sonicBgScrollX;

  int startX = sonicClampInt(x, 0, SONIC_SCR_W);
  int endX = sonicClampInt(x + w, 0, SONIC_SCR_W);
  int startY = sonicClampInt(y, 0, SONIC_SCR_H);
  int endY = sonicClampInt(y + h, 0, SONIC_SCR_H);

  for (int dy = startY; dy < endY; dy++) {
    int srcY = SONIC_BG_VIEW_Y + dy - sonicSceneOffsetY;

    if (srcY < 0) srcY = 0;
    if (srcY >= SONIC_BG_H) srcY = SONIC_BG_H - 1;

    for (int dx = startX; dx < endX; dx++) {
      int srcX = (int)sonicWrapIndex((long)dx + scrollX, SONIC_BG_W);

      uint32_t pos =
        (uint32_t)srcY * (uint32_t)SONIC_BG_W +
        (uint32_t)srcX;

      uint16_t color = pgm_read_word(&(SONIC_BG[pos]));
      display.drawPixel(dx, dy, color);
    }
  }
}


// =====================================================
// 地形繪製
// =====================================================
static void drawSonicFlatGroundTile(int x) {
  drawSonicImageScreen(
    x,
    SONIC_FLAT_GRASS_TOP_Y + sonicSceneOffsetY,
    SONIC_GN1_W,
    SONIC_GN1_H,
    SONIC_GN1,
    false,
    true
  );
}


static void drawSonicHillTerrainColumn(int x, int topY) {
  topY = sonicClampInt(topY, SONIC_HILL_MIN_TOP_Y, SONIC_HILL_START_TOP_Y);

  drawSonicImageScreen(
    x,
    topY + sonicSceneOffsetY,
    SONIC_GN2_W,
    SONIC_GN2_H,
    SONIC_GN2,
    false,
    true
  );

  for (
    int y = topY + SONIC_HILL_GN0_START_OFFSET_Y;
    y < SONIC_SCR_H;
    y += SONIC_GN0_H
  ) {
    drawSonicImageScreen(
      x,
      y + sonicSceneOffsetY,
      SONIC_GN0_W,
      SONIC_GN0_H,
      SONIC_GN0,
      false,
      false
    );
  }
}


static void drawSonicGroundBase() {
  int scrollX = (int)sonicGroundScrollX;
  int startX = -(scrollX % SONIC_TERRAIN_TILE_W);

  for (
    int x = startX - SONIC_TERRAIN_TILE_W;
    x < SONIC_SCR_W + SONIC_TERRAIN_TILE_W;
    x += SONIC_TERRAIN_TILE_W
  ) {
    drawSonicFlatGroundTile(x);
  }
}


static int getSonicHillStepCountForType(uint8_t hillType) {
  if (hillType == SONIC_HILL_TYPE_4) {
    return SONIC_HILL_BASE_STEP_COUNT + SONIC_HILL4_EXTRA_STEP_COUNT;
  }

  return SONIC_HILL_BASE_STEP_COUNT;
}


static int getSonicHillStepCountForCurrentType() {
  return getSonicHillStepCountForType(sonicHillType);
}


static int getSonicHillMaxTopYForType(uint8_t hillType) {
  int stepCount = getSonicHillStepCountForType(hillType);
  int topY = SONIC_HILL_START_TOP_Y - (stepCount * SONIC_HILL_STEP_Y);

  return sonicClampInt(topY, SONIC_HILL_MIN_TOP_Y, SONIC_HILL_START_TOP_Y);
}


static int getSonicHillMaxTopYForCurrentType() {
  return getSonicHillMaxTopYForType(sonicHillType);
}


static int getSonicHillPlateauBlocksForCurrentType() {
  if (
    sonicHillType == SONIC_HILL_TYPE_3 ||
    sonicHillType == SONIC_HILL_TYPE_4 ||
    sonicHillType == SONIC_HILL_TYPE_5
  ) {
    return (int)sonicHillPlateauBlocks;
  }

  return 0;
}


static int getSonicHillGapBlocksForType(uint8_t hillType) {
  if (hillType == SONIC_HILL_TYPE_5) {
    return SONIC_HILL5_GAP_BLOCKS;
  }

  return 0;
}


static int getSonicHillGapBlocksForCurrentType() {
  return getSonicHillGapBlocksForType(sonicHillType);
}


static bool isSonicHillGapSegment(int segmentIndex) {
  if (sonicHillType != SONIC_HILL_TYPE_5) {
    return false;
  }

  int stepCount = getSonicHillStepCountForCurrentType();
  int climbBlocks = stepCount + 1;
  int plateauBlocks = getSonicHillPlateauBlocksForCurrentType();

  int gapStart = climbBlocks + plateauBlocks;
  int gapEnd = gapStart + getSonicHillGapBlocksForCurrentType();

  return segmentIndex >= gapStart && segmentIndex < gapEnd;
}


static bool getSonicHillGapProgressAtScreenX(float sampleX, float* outProgress) {
  if (!sonicHillActive) {
    return false;
  }

  if (sonicHillType != SONIC_HILL_TYPE_5) {
    return false;
  }

  int stepCount = getSonicHillStepCountForCurrentType();
  int climbBlocks = stepCount + 1;
  int plateauBlocks = getSonicHillPlateauBlocksForCurrentType();
  int gapBlocks = getSonicHillGapBlocksForCurrentType();

  if (gapBlocks <= 0) {
    return false;
  }

  int gapStartSegment = climbBlocks + plateauBlocks;

  float gapStartX =
    sonicHillX +
    ((float)gapStartSegment * (float)SONIC_TERRAIN_TILE_W);

  float gapWidth =
    (float)gapBlocks *
    (float)SONIC_TERRAIN_TILE_W;

  float gapEndX = gapStartX + gapWidth;

  if (sampleX < gapStartX || sampleX >= gapEndX) {
    return false;
  }

  float progress = (sampleX - gapStartX) / gapWidth;

  if (progress < 0.0f) {
    progress = 0.0f;
  }

  if (progress > 1.0f) {
    progress = 1.0f;
  }

  if (outProgress) {
    *outProgress = progress;
  }

  return true;
}


static bool getSonicHill5GapScreenRange(float* outGapStartX, float* outGapEndX) {
  if (!sonicHillActive) {
    return false;
  }

  if (sonicHillType != SONIC_HILL_TYPE_5) {
    return false;
  }

  int stepCount = getSonicHillStepCountForCurrentType();
  int climbBlocks = stepCount + 1;
  int plateauBlocks = getSonicHillPlateauBlocksForCurrentType();
  int gapBlocks = getSonicHillGapBlocksForCurrentType();

  if (gapBlocks <= 0) {
    return false;
  }

  int gapStartSegment = climbBlocks + plateauBlocks;

  float gapStartX =
    sonicHillX +
    ((float)gapStartSegment * (float)SONIC_TERRAIN_TILE_W);

  float gapEndX =
    gapStartX +
    ((float)gapBlocks * (float)SONIC_TERRAIN_TILE_W);

  if (outGapStartX) {
    *outGapStartX = gapStartX;
  }

  if (outGapEndX) {
    *outGapEndX = gapEndX;
  }

  return true;
}


static bool isSonicObjectRangeOverHill5Gap(
  float objectX,
  int objectW,
  int marginX
) {
  float gapStartX = 0.0f;
  float gapEndX = 0.0f;

  if (!getSonicHill5GapScreenRange(&gapStartX, &gapEndX)) {
    return false;
  }

  float objectLeft = objectX;
  float objectRight = objectX + (float)objectW;

  gapStartX -= (float)marginX;
  gapEndX += (float)marginX;

  if (objectRight <= gapStartX) {
    return false;
  }

  if (objectLeft >= gapEndX) {
    return false;
  }

  return true;
}



static int getSonicTerrain5JumpExtraLiftAtScreenX(float sampleX) {
  float progress = 0.0f;

  if (!getSonicHillGapProgressAtScreenX(sampleX, &progress)) {
    return 0;
  }

  // 拋物線：
  // progress = 0   起跳
  // progress = 0.5 最高點
  // progress = 1   落地
  float arc = 4.0f * progress * (1.0f - progress);

  int lift = (int)((float)SONIC_HILL5_JUMP_EXTRA_LIFT_Y * arc + 0.5f);

  if (lift < 0) {
    lift = 0;
  }

  return lift;
}


static int getSonicHillTopYForSegment(int segmentIndex) {
  if (segmentIndex < 0) {
    return SONIC_FLAT_GRASS_TOP_Y;
  }

  int stepCount = getSonicHillStepCountForCurrentType();
  int maxTopY = getSonicHillMaxTopYForCurrentType();

  int climbBlocks = stepCount + 1;

  if (segmentIndex < climbBlocks) {
    int topY = SONIC_HILL_START_TOP_Y - (segmentIndex * SONIC_HILL_STEP_Y);
    return sonicClampInt(topY, maxTopY, SONIC_HILL_START_TOP_Y);
  }

  int plateauBlocks = getSonicHillPlateauBlocksForCurrentType();
  int plateauStart = climbBlocks;
  int plateauEnd = plateauStart + plateauBlocks;

  if (segmentIndex < plateauEnd) {
    return maxTopY;
  }

  int gapBlocks = getSonicHillGapBlocksForCurrentType();
  int gapStart = plateauEnd;
  int gapEnd = gapStart + gapBlocks;

  if (segmentIndex >= gapStart && segmentIndex < gapEnd) {
    return SONIC_TERRAIN_GAP_TOP_Y;
  }

  int downIndex = segmentIndex - gapEnd;

  if (downIndex < stepCount) {
    int topY = maxTopY + ((downIndex + 1) * SONIC_HILL_STEP_Y);
    return sonicClampInt(topY, maxTopY, SONIC_HILL_START_TOP_Y);
  }

  return SONIC_FLAT_GRASS_TOP_Y;
}


static int getSonicHillLiftForSegment(int segmentIndex) {
  int topY = getSonicHillTopYForSegment(segmentIndex);

  if (topY == SONIC_TERRAIN_GAP_TOP_Y) {
    int maxTopY = getSonicHillMaxTopYForCurrentType();
    return SONIC_FLAT_GRASS_TOP_Y - maxTopY;
  }

  if (topY >= SONIC_FLAT_GRASS_TOP_Y) {
    return 0;
  }

  return SONIC_FLAT_GRASS_TOP_Y - topY;
}


static int getSonicHillLiftAtScreenX(float sampleX) {
  if (!sonicHillActive) {
    return 0;
  }

  float relX = sampleX - sonicHillX;

  if (relX < 0.0f) {
    return 0;
  }

  if (relX >= (float)sonicHillWidth) {
    return 0;
  }

  int segmentIndex = (int)relX / SONIC_TERRAIN_TILE_W;
  int lift = getSonicHillLiftForSegment(segmentIndex);

  // 地形5 懸崖跳躍：
  // 原本只是 frame15 水平滑過，這裡加上拋物線高度。
  if (
    sonicHillType == SONIC_HILL_TYPE_5 &&
    isSonicHillGapSegment(segmentIndex)
  ) {
    lift += getSonicTerrain5JumpExtraLiftAtScreenX(sampleX);
  }

  return lift;
}


static int getSonicTerrainTopYAtScreenX(float sampleX) {
  if (!sonicHillActive) {
    return SONIC_FLAT_GRASS_TOP_Y;
  }

  float relX = sampleX - sonicHillX;

  if (relX < 0.0f || relX >= (float)sonicHillWidth) {
    return SONIC_FLAT_GRASS_TOP_Y;
  }

  int segmentIndex = (int)relX / SONIC_TERRAIN_TILE_W;
  int topY = getSonicHillTopYForSegment(segmentIndex);

  if (topY == SONIC_TERRAIN_GAP_TOP_Y) {
    return getSonicHillMaxTopYForCurrentType();
  }

  return topY;
}


static void drawSonicHill() {
  if (!sonicHillActive) {
    return;
  }

  int firstX = (int)sonicHillX;
  int totalSegments =
    (sonicHillWidth + SONIC_TERRAIN_TILE_W - 1) /
    SONIC_TERRAIN_TILE_W;

  for (int i = 0; i < totalSegments; i++) {
    int x = firstX + (i * SONIC_TERRAIN_TILE_W);

    if (x <= -SONIC_TERRAIN_TILE_W || x >= SONIC_SCR_W) {
      continue;
    }

    int topY = getSonicHillTopYForSegment(i);

    if (topY == SONIC_TERRAIN_GAP_TOP_Y) {
      int gapEraseY = getSonicHillMaxTopYForCurrentType() + sonicSceneOffsetY;

      drawSonicBackgroundRect(
        x,
        gapEraseY,
        SONIC_TERRAIN_TILE_W,
        SONIC_SCR_H - gapEraseY
      );

      continue;
    }

    drawSonicHillTerrainColumn(x, topY);
  }
}


// =====================================================
// TREE 繪製
// =====================================================
static void drawSonicTree() {
  if (!sonicTreeActive) {
    return;
  }

  // TREE 是背景大物件，不能長在地形5懸崖缺口上。
  // 如果 TREE 的寬度範圍碰到缺口，就暫時不畫它。
  if (
    isSonicObjectRangeOverHill5Gap(
      sonicTreeX,
      SONIC_TREE_W,
      SONIC_TREE_GAP_SAFE_MARGIN_X
    )
  ) {
    return;
  }

  int anchorX = (int)sonicTreeX + SONIC_TREE_ANCHOR_X;
  int terrainTopY = getSonicTerrainTopYAtScreenX((float)anchorX);
  int treeY = terrainTopY - SONIC_TREE_H;

  drawSonicImageScreen(
    (int)sonicTreeX,
    treeY + sonicSceneOffsetY,
    SONIC_TREE_W,
    SONIC_TREE_H,
    SONIC_TREE,
    false,
    true
  );
}


// =====================================================
// RING 繪製 / 碰撞
// =====================================================
static int getSonicRingY() {
  int anchorX = (int)sonicRingX + SONIC_RING_ANCHOR_X;
  int terrainTopY = getSonicTerrainTopYAtScreenX((float)anchorX);

  return terrainTopY - SONIC_RING_FRAME_H - SONIC_RING_FLOAT_OFFSET_Y;
}


static void drawSonicRingFrame(int frameIndex, int x, int y) {
  frameIndex = sonicClampInt(frameIndex, 0, SONIC_RING_FRAME_COUNT - 1);

  drawSonicSpriteFrameScreen(
    x,
    y,
    SONIC_RING_SHEET_W,
    SONIC_RING_SHEET_H,
    SONIC_RING,
    frameIndex * SONIC_RING_FRAME_W,
    0,
    SONIC_RING_FRAME_W,
    SONIC_RING_FRAME_H,
    false,
    true
  );
}


static void drawSonicRing(unsigned long nowMs) {
  if (!sonicRingActive) {
    return;
  }

  int ringY = getSonicRingY() + sonicSceneOffsetY;

  if (!sonicRingCollected) {
    drawSonicRingFrame(0, (int)sonicRingX, ringY);
    return;
  }

  unsigned long collectedElapsed = nowMs - sonicRingCollectedMs;

  if (collectedElapsed >= SONIC_RING_COLLECT_BLINK_MS) {
    return;
  }

  unsigned long blinkPhase = collectedElapsed / SONIC_RING_BLINK_INTERVAL_MS;

  if ((blinkPhase % 2) == 0) {
    drawSonicRingFrame(1, (int)sonicRingX, ringY);
  }
}


static void updateSonicRingCollision(unsigned long nowMs) {
  if (!sonicRingActive) {
    return;
  }

  if (sonicRingCollected) {
    if (nowMs - sonicRingCollectedMs >= SONIC_RING_COLLECT_BLINK_MS) {
      sonicRingActive = false;
      sonicRingCollected = false;
      sonicRingCollectedMs = 0;
      markSonicInteractiveObjectClearance(nowMs);
      scheduleNextSonicRing(nowMs);
    }

    return;
  }

  int ringY = getSonicRingY();

  int sonicHitX = (int)sonicX + 5;
  int sonicHitY = (int)sonicY + 6;
  int sonicHitW = 14;
  int sonicHitH = 22;

  int ringHitX = (int)sonicRingX + 3;
  int ringHitY = ringY + 3;
  int ringHitW = 10;
  int ringHitH = 10;

  if (
    sonicAabbOverlap(
      sonicHitX,
      sonicHitY,
      sonicHitW,
      sonicHitH,
      ringHitX,
      ringHitY,
      ringHitW,
      ringHitH
    )
  ) {
    sonicRingCollected = true;
    sonicRingCollectedMs = nowMs;
  }
}


// =====================================================
// MONITOR 繪製 / 動作
// =====================================================
static int getSonicMonitorY() {
  return SONIC_FLAT_GRASS_TOP_Y - SONIC_MONITOR_FRAME_H;
}


static void drawSonicMonitorFrame(uint8_t frameIndex, int x, int y) {
  frameIndex = sonicClampInt(frameIndex, 0, SONIC_MONITOR_FRAME_COUNT - 1);

  drawSonicSpriteFrameScreen(
    x,
    y,
    SONIC_MONITOR_SHEET_W,
    SONIC_MONITOR_SHEET_H,
    SONIC_MONITOR,
    frameIndex * SONIC_MONITOR_FRAME_W,
    0,
    SONIC_MONITOR_FRAME_W,
    SONIC_MONITOR_FRAME_H,
    false,
    true
  );
}


static void drawSonicMonitor(unsigned long nowMs) {
  if (!sonicMonitorActive) {
    return;
  }

  int monitorY = getSonicMonitorY() + sonicSceneOffsetY;

  if (!sonicMonitorCollected) {
    drawSonicMonitorFrame(
      sonicMonitorFrame,
      (int)sonicMonitorX,
      monitorY
    );

    return;
  }

  unsigned long collectedElapsed = nowMs - sonicMonitorCollectedMs;

  if (collectedElapsed >= SONIC_MONITOR_BLINK_MS) {
    return;
  }

  unsigned long blinkPhase = collectedElapsed / SONIC_MONITOR_BLINK_INTERVAL_MS;

  if ((blinkPhase % 2) == 0) {
    drawSonicMonitorFrame(
      sonicMonitorFrame,
      (int)sonicMonitorX,
      monitorY
    );
  }
}


static void startSonicMonitorAction(unsigned long nowMs) {
  if (sonicMonitorAction != SONIC_MONITOR_ACTION_NONE) {
    return;
  }

  sonicMonitorAction = SONIC_MONITOR_ACTION_BALL_UP;
  sonicMonitorActionStartMs = nowMs;
}


static int getSonicMonitorActionFrame(unsigned long nowMs) {
  if (sonicMonitorAction == SONIC_MONITOR_ACTION_NONE) {
    return -1;
  }

  if (sonicMonitorAction == SONIC_MONITOR_ACTION_BALL_UP) {
    unsigned long elapsed = nowMs - sonicMonitorActionStartMs;
    int index = elapsed / SONIC_MONITOR_BALL_FRAME_MS;

    if (index < 0) index = 0;
    if (index > 4) index = 4;

    return 10 + index;
  }

  if (
    sonicMonitorAction == SONIC_MONITOR_ACTION_ATTACK ||
    sonicMonitorAction == SONIC_MONITOR_ACTION_LAND
  ) {
    return 14;
  }

  if (sonicMonitorAction == SONIC_MONITOR_ACTION_RECOVER) {
    unsigned long elapsed = nowMs - sonicMonitorActionStartMs;
    int index = elapsed / SONIC_MONITOR_RECOVER_FRAME_MS;

    if (index < 0) index = 0;
    if (index > 4) index = 4;

    return 14 - index;
  }

  return -1;
}


static void collectSonicMonitor(unsigned long nowMs) {
  if (!sonicMonitorActive || sonicMonitorCollected) {
    return;
  }

  sonicMonitorCollected = true;
  sonicMonitorCollectedMs = nowMs;

  sonicMonitorAction = SONIC_MONITOR_ACTION_LAND;
  sonicMonitorActionStartMs = nowMs;
}


static bool isSonicMonitorCollision() {
  if (!sonicMonitorActive || sonicMonitorCollected) {
    return false;
  }

  int monitorY = getSonicMonitorY();

  int sonicHitX = (int)sonicX + 5;
  int sonicHitY = (int)sonicY + 6;
  int sonicHitW = 14;
  int sonicHitH = 22;

  int monitorHitX = (int)sonicMonitorX + 1;
  int monitorHitY = monitorY + 1;
  int monitorHitW = SONIC_MONITOR_FRAME_W - 2;
  int monitorHitH = SONIC_MONITOR_FRAME_H - 2;

  return sonicAabbOverlap(
    sonicHitX,
    sonicHitY,
    sonicHitW,
    sonicHitH,
    monitorHitX,
    monitorHitY,
    monitorHitW,
    monitorHitH
  );
}


static void updateSonicMonitorAction(unsigned long nowMs) {
  if (!sonicMonitorActive && sonicMonitorAction == SONIC_MONITOR_ACTION_NONE) {
    return;
  }

  if (
    sonicMonitorActive &&
    !sonicMonitorCollected &&
    sonicMonitorAction == SONIC_MONITOR_ACTION_NONE
  ) {
    int triggerX = (int)sonicX + SONIC_MONITOR_ATTACK_TRIGGER_DISTANCE;

    if (
      sonicMonitorX <= (float)triggerX &&
      sonicMonitorX >= sonicX
    ) {
      startSonicMonitorAction(nowMs);
    }
  }

  if (sonicMonitorAction == SONIC_MONITOR_ACTION_BALL_UP) {
    unsigned long elapsed = nowMs - sonicMonitorActionStartMs;
    int index = elapsed / SONIC_MONITOR_BALL_FRAME_MS;

    if (index > 4) index = 4;

    int lift = (SONIC_MONITOR_BALL_LIFT_Y * index) / 4;
    sonicY = (float)(SONIC_BASE_Y - lift);

    unsigned long totalMs = SONIC_MONITOR_BALL_FRAME_MS * 5UL;

    if (elapsed >= totalMs) {
      sonicMonitorAction = SONIC_MONITOR_ACTION_ATTACK;
      sonicMonitorActionStartMs = nowMs;
    }
  }
  else if (sonicMonitorAction == SONIC_MONITOR_ACTION_ATTACK) {
    sonicY = (float)(SONIC_BASE_Y - SONIC_MONITOR_BALL_LIFT_Y);
  }
  else if (sonicMonitorAction == SONIC_MONITOR_ACTION_LAND) {
    unsigned long elapsed = nowMs - sonicMonitorActionStartMs;

    if (elapsed >= SONIC_MONITOR_LAND_MS) {
      sonicY = (float)SONIC_BASE_Y;
      sonicMonitorAction = SONIC_MONITOR_ACTION_RECOVER;
      sonicMonitorActionStartMs = nowMs;
    }
    else {
      int lift = SONIC_MONITOR_BALL_LIFT_Y -
        ((SONIC_MONITOR_BALL_LIFT_Y * elapsed) / SONIC_MONITOR_LAND_MS);

      sonicY = (float)(SONIC_BASE_Y - lift);
    }
  }
  else if (sonicMonitorAction == SONIC_MONITOR_ACTION_RECOVER) {
    sonicY = (float)SONIC_BASE_Y;

    unsigned long elapsed = nowMs - sonicMonitorActionStartMs;
    unsigned long totalMs = SONIC_MONITOR_RECOVER_FRAME_MS * 5UL;

    if (elapsed >= totalMs) {
      sonicMonitorAction = SONIC_MONITOR_ACTION_NONE;
      sonicMonitorActionStartMs = 0;
    }
  }

  if (
    sonicMonitorAction == SONIC_MONITOR_ACTION_BALL_UP ||
    sonicMonitorAction == SONIC_MONITOR_ACTION_ATTACK
  ) {
    if (isSonicMonitorCollision()) {
      collectSonicMonitor(nowMs);
    }
  }

  if (sonicMonitorCollected) {
    if (nowMs - sonicMonitorCollectedMs >= SONIC_MONITOR_BLINK_MS) {
      sonicMonitorActive = false;
      sonicMonitorCollected = false;
      sonicMonitorCollectedMs = 0;
      markSonicInteractiveObjectClearance(nowMs);
      scheduleNextSonicMonitor(nowMs);
    }
  }
}


// =====================================================
// SPRING 繪製 / 動作
// =====================================================
static int getSonicSpringY() {
  return SONIC_FLAT_GRASS_TOP_Y - SONIC_SPRING_FRAME_H;
}


static void drawSonicSpringFrame(uint8_t frameIndex, int x, int y) {
  frameIndex = sonicClampInt(frameIndex, 0, SONIC_SPRING_FRAME_COUNT - 1);

  drawSonicSpriteFrameScreen(
    x,
    y,
    SONIC_SPRING_SHEET_W,
    SONIC_SPRING_SHEET_H,
    SONIC_SPRING,
    frameIndex * SONIC_SPRING_FRAME_W,
    0,
    SONIC_SPRING_FRAME_W,
    SONIC_SPRING_FRAME_H,
    false,
    true
  );
}


static void drawSonicSpring(unsigned long nowMs) {
  if (!sonicSpringActive) {
    return;
  }

  int frame = 0;

  if (sonicSpringPressed) {
    frame = 1;
  }

  drawSonicSpringFrame(
    frame,
    (int)sonicSpringX,
    getSonicSpringY() + sonicSceneOffsetY
  );
}


static void startSonicSpringAction(unsigned long nowMs) {
  if (sonicSpringAction != SONIC_SPRING_ACTION_NONE) {
    return;
  }

  sonicSpringAction = SONIC_SPRING_ACTION_BALL_UP;
  sonicSpringActionStartMs = nowMs;
}


static int getSonicSpringActionFrame(unsigned long nowMs) {
  if (sonicSpringAction == SONIC_SPRING_ACTION_NONE) {
    return -1;
  }

  if (sonicSpringAction == SONIC_SPRING_ACTION_BALL_UP) {
    unsigned long elapsed = nowMs - sonicSpringActionStartMs;
    int index = elapsed / SONIC_SPRING_BALL_FRAME_MS;

    if (index < 0) {
      index = 0;
    }

    if (index > 4) {
      index = 4;
    }

    return 10 + index;
  }

  if (sonicSpringAction == SONIC_SPRING_ACTION_ATTACK) {
    return 14;
  }

  if (
    sonicSpringAction == SONIC_SPRING_ACTION_JUMP ||
    sonicSpringAction == SONIC_SPRING_ACTION_FALL
  ) {
    return 15;
  }

  return -1;
}


static bool isSonicSpringCollision() {
  if (!sonicSpringActive || sonicSpringPressed) {
    return false;
  }

  int springY = getSonicSpringY();

  int sonicHitX = (int)sonicX + 5;
  int sonicHitY = (int)sonicY + 6;
  int sonicHitW = 14;
  int sonicHitH = 22;

  int springHitX = (int)sonicSpringX + 2;
  int springHitY = springY + 2;
  int springHitW = SONIC_SPRING_FRAME_W - 4;
  int springHitH = SONIC_SPRING_FRAME_H - 3;

  return sonicAabbOverlap(
    sonicHitX,
    sonicHitY,
    sonicHitW,
    sonicHitH,
    springHitX,
    springHitY,
    springHitW,
    springHitH
  );
}


static void collectSonicSpring(unsigned long nowMs) {
  if (!sonicSpringActive || sonicSpringPressed) {
    return;
  }

  sonicSpringPressed = true;
  sonicSpringAction = SONIC_SPRING_ACTION_JUMP;
  sonicSpringActionStartMs = nowMs;
}


static void updateSonicSpringAction(unsigned long nowMs) {
  if (sonicSpringAction == SONIC_SPRING_ACTION_NONE) {
    sonicSceneOffsetY = 0;

    if (
      sonicSpringActive &&
      !sonicSpringPressed
    ) {
      int triggerX = (int)sonicX + SONIC_SPRING_ATTACK_TRIGGER_DISTANCE;

      if (
        sonicSpringX <= (float)triggerX &&
        sonicSpringX >= sonicX
      ) {
        startSonicSpringAction(nowMs);
      }
    }

    return;
  }

  if (sonicSpringAction == SONIC_SPRING_ACTION_BALL_UP) {
    unsigned long elapsed = nowMs - sonicSpringActionStartMs;
    int index = elapsed / SONIC_SPRING_BALL_FRAME_MS;

    if (index > 4) {
      index = 4;
    }

    int lift = (SONIC_SPRING_BALL_LIFT_Y * index) / 4;
    sonicY = (float)(SONIC_BASE_Y - lift);

    unsigned long totalMs = SONIC_SPRING_BALL_FRAME_MS * 5UL;

    if (elapsed >= totalMs) {
      sonicSpringAction = SONIC_SPRING_ACTION_ATTACK;
      sonicSpringActionStartMs = nowMs;
    }
  }
  else if (sonicSpringAction == SONIC_SPRING_ACTION_ATTACK) {
    sonicY = (float)(SONIC_BASE_Y - SONIC_SPRING_BALL_LIFT_Y);
  }
  else if (sonicSpringAction == SONIC_SPRING_ACTION_JUMP) {
    unsigned long elapsed = nowMs - sonicSpringActionStartMs;

    if (elapsed >= SONIC_SPRING_JUMP_UP_MS) {
      sonicSceneOffsetY = SONIC_SPRING_JUMP_MAX_SCENE_OFFSET_Y;
      sonicY = (float)SONIC_BASE_Y;

      sonicSpringAction = SONIC_SPRING_ACTION_FALL;
      sonicSpringActionStartMs = nowMs;
      return;
    }

    // ease-out 上升：
    // 一開始很快，越接近最高點越慢。
    float progress = (float)elapsed / (float)SONIC_SPRING_JUMP_UP_MS;

    if (progress < 0.0f) {
      progress = 0.0f;
    }

    if (progress > 1.0f) {
      progress = 1.0f;
    }

    float eased = 1.0f - ((1.0f - progress) * (1.0f - progress));

    sonicSceneOffsetY =
      (int)((float)SONIC_SPRING_JUMP_MAX_SCENE_OFFSET_Y * eased + 0.5f);

    sonicY = (float)SONIC_BASE_Y;
  }
  else if (sonicSpringAction == SONIC_SPRING_ACTION_FALL) {
    unsigned long elapsed = nowMs - sonicSpringActionStartMs;

    if (elapsed < SONIC_SPRING_PEAK_HOLD_MS) {
      sonicSceneOffsetY = SONIC_SPRING_JUMP_MAX_SCENE_OFFSET_Y;
      sonicY = (float)SONIC_BASE_Y;
      return;
    }

    unsigned long fallElapsed = elapsed - SONIC_SPRING_PEAK_HOLD_MS;

    if (fallElapsed >= SONIC_SPRING_FALL_MS) {
      sonicSceneOffsetY = 0;
      sonicY = (float)SONIC_BASE_Y;

      sonicSpringActive = false;
      sonicSpringPressed = false;
      sonicSpringAction = SONIC_SPRING_ACTION_NONE;
      sonicSpringActionStartMs = 0;

      markSonicInteractiveObjectClearance(nowMs);
      scheduleNextSonicSpring(nowMs);
      return;
    }

    // ease-in 落下：
    // 剛開始慢，越接近地面越快，比較像重力。
    float progress = (float)fallElapsed / (float)SONIC_SPRING_FALL_MS;

    if (progress < 0.0f) {
      progress = 0.0f;
    }

    if (progress > 1.0f) {
      progress = 1.0f;
    }

    float fallCurve = progress * progress;

    sonicSceneOffsetY =
      SONIC_SPRING_JUMP_MAX_SCENE_OFFSET_Y -
      (int)((float)SONIC_SPRING_JUMP_MAX_SCENE_OFFSET_Y * fallCurve + 0.5f);

    if (sonicSceneOffsetY < 0) {
      sonicSceneOffsetY = 0;
    }

    sonicY = (float)SONIC_BASE_Y;
  }

  if (
    sonicSpringAction == SONIC_SPRING_ACTION_BALL_UP ||
    sonicSpringAction == SONIC_SPRING_ACTION_ATTACK
  ) {
    if (isSonicSpringCollision()) {
      collectSonicSpring(nowMs);
    }
  }
}


// =====================================================
// Sonic 高度計算
// =====================================================
static int getSonicHillLift() {
  // 用 Sonic 中心偏右的位置判斷地形。
  // 比只用最右腳更穩，避免地形5落地時太早切回跑步造成踩空感。
  float sonicSampleX = sonicX + ((float)SONIC_FRAME_W * 0.65f);
  return getSonicHillLiftAtScreenX(sonicSampleX);
}


static void updateSonicYByHill() {
  int lift = getSonicHillLift();
  sonicY = (float)(SONIC_BASE_Y - lift);
}


// =====================================================
// 小山丘生成
// =====================================================
static int getSonicHillWidth(uint8_t hillType, uint8_t plateauBlocks) {
  int plateau = 0;

  if (
    hillType == SONIC_HILL_TYPE_3 ||
    hillType == SONIC_HILL_TYPE_4 ||
    hillType == SONIC_HILL_TYPE_5
  ) {
    plateau = (int)plateauBlocks;
  }

  int stepCount = getSonicHillStepCountForType(hillType);
  int climbBlocks = stepCount + 1;
  int downBlocks = stepCount;
  int gapBlocks = getSonicHillGapBlocksForType(hillType);

  int totalBlocks = climbBlocks + plateau + gapBlocks + downBlocks;

  return totalBlocks * SONIC_TERRAIN_TILE_W;
}


static void scheduleNextSonicHill(unsigned long nowMs) {
  unsigned long gap = random(SONIC_HILL_GAP_MIN_MS, SONIC_HILL_GAP_MAX_MS + 1);
  sonicNextHillMs = nowMs + gap;
}


static void spawnSonicHill() {
  int typeRoll = random(0, 4);

  if (typeRoll == 0) {
    sonicHillType = SONIC_HILL_TYPE_2;
    sonicHillPlateauBlocks = 0;
  }
  else if (typeRoll == 1) {
    sonicHillType = SONIC_HILL_TYPE_3;
    sonicHillPlateauBlocks = random(
      SONIC_HILL_PLATEAU_MIN_BLOCKS,
      SONIC_HILL_PLATEAU_MAX_BLOCKS + 1
    );
  }
  else if (typeRoll == 2) {
    sonicHillType = SONIC_HILL_TYPE_4;
    sonicHillPlateauBlocks = random(
      SONIC_HILL_PLATEAU_MIN_BLOCKS,
      SONIC_HILL_PLATEAU_MAX_BLOCKS + 1
    );
  }
  else {
    sonicHillType = SONIC_HILL_TYPE_5;
    sonicHillPlateauBlocks = random(
      SONIC_HILL_PLATEAU_MIN_BLOCKS,
      SONIC_HILL_PLATEAU_MAX_BLOCKS + 1
    );
  }

  sonicHillX = (float)SONIC_HILL_SPAWN_X;
  sonicHillWidth = getSonicHillWidth(sonicHillType, sonicHillPlateauBlocks);
  sonicHillActive = true;
}


static void updateSonicHillSpawner(unsigned long nowMs) {
  if (sonicState != SONIC_STATE_RUN) {
    return;
  }

  if (sonicHillActive) {
    if (sonicHillX + (float)sonicHillWidth < -24.0f) {
      sonicHillActive = false;
      scheduleNextSonicHill(nowMs);
    }

    return;
  }

  if (sonicEndRequested) {
    return;
  }

  if (
    sonicMonitorActive ||
    sonicMonitorAction != SONIC_MONITOR_ACTION_NONE ||
    sonicSpringActive ||
    sonicSpringAction != SONIC_SPRING_ACTION_NONE
  ) {
    return;
  }

  unsigned long runElapsed = nowMs - sonicRunStartMs;

  if (runElapsed < SONIC_HILL_FIRST_DELAY_MS) {
    return;
  }

  if (runElapsed + 4000UL >= SONIC_RUN_BEFORE_END_MS) {
    return;
  }

  if (nowMs >= sonicNextHillMs) {
    spawnSonicHill();
  }
}


// =====================================================
// TREE 生成
// =====================================================
static void scheduleNextSonicTree(unsigned long nowMs) {
  unsigned long gap = random(SONIC_TREE_GAP_MIN_MS, SONIC_TREE_GAP_MAX_MS + 1);
  sonicNextTreeMs = nowMs + gap;
}


static void spawnSonicTree() {
  sonicTreeX = (float)SONIC_TREE_SPAWN_X;
  sonicTreeActive = true;
}


static void updateSonicTreeSpawner(unsigned long nowMs) {
  if (sonicState != SONIC_STATE_RUN) {
    return;
  }

  if (sonicTreeActive) {
    if (sonicTreeX + (float)SONIC_TREE_W < -4.0f) {
      sonicTreeActive = false;
      scheduleNextSonicTree(nowMs);
    }

    return;
  }

  if (sonicEndRequested) {
    return;
  }

// 地形存在時，不生成新的 TREE。
// 避免 TREE 剛好生成在地形5懸崖缺口附近。
if (sonicHillActive) {
  return;
}


  unsigned long runElapsed = nowMs - sonicRunStartMs;

  if (runElapsed < SONIC_TREE_FIRST_DELAY_MS) {
    return;
  }

  if (runElapsed + 4000UL >= SONIC_RUN_BEFORE_END_MS) {
    return;
  }

  if (nowMs >= sonicNextTreeMs) {
    spawnSonicTree();
  }
}


// =====================================================
// RING 生成
// =====================================================
static void scheduleNextSonicRing(unsigned long nowMs) {
  unsigned long gap = random(SONIC_RING_GAP_MIN_MS, SONIC_RING_GAP_MAX_MS + 1);
  sonicNextRingMs = nowMs + gap;
}


static void spawnSonicRing() {
  sonicRingX = (float)SONIC_RING_SPAWN_X;
  sonicRingActive = true;
  sonicRingCollected = false;
  sonicRingCollectedMs = 0;
}


static void updateSonicRingSpawner(unsigned long nowMs) {
  if (sonicState != SONIC_STATE_RUN) {
    return;
  }

  if (sonicRingActive) {
    if (sonicRingX + (float)SONIC_RING_FRAME_W < -4.0f) {
      sonicRingActive = false;
      sonicRingCollected = false;
      sonicRingCollectedMs = 0;
      markSonicInteractiveObjectClearance(nowMs);
      scheduleNextSonicRing(nowMs);
    }

    return;
  }

  if (sonicEndRequested) {
    return;
  }

  if (!canSpawnSonicInteractiveObject(nowMs)) {
    return;
  }

  unsigned long runElapsed = nowMs - sonicRunStartMs;

  if (runElapsed < SONIC_RING_FIRST_DELAY_MS) {
    return;
  }

  if (runElapsed + 4000UL >= SONIC_RUN_BEFORE_END_MS) {
    return;
  }

  if (nowMs >= sonicNextRingMs) {
    spawnSonicRing();
  }
}


// =====================================================
// MONITOR 生成
// =====================================================
static void scheduleNextSonicMonitor(unsigned long nowMs) {
  unsigned long gap = random(
    SONIC_MONITOR_GAP_MIN_MS,
    SONIC_MONITOR_GAP_MAX_MS + 1
  );

  sonicNextMonitorMs = nowMs + gap;
}


static void spawnSonicMonitor() {
  sonicMonitorX = (float)SONIC_MONITOR_SPAWN_X;
  sonicMonitorFrame = random(0, SONIC_MONITOR_FRAME_COUNT);
  sonicMonitorActive = true;
  sonicMonitorCollected = false;
  sonicMonitorCollectedMs = 0;
}


static void updateSonicMonitorSpawner(unsigned long nowMs) {
  if (sonicState != SONIC_STATE_RUN) {
    return;
  }

  if (sonicMonitorActive) {
    if (sonicMonitorX + (float)SONIC_MONITOR_FRAME_W < -4.0f) {
      sonicMonitorActive = false;
      sonicMonitorCollected = false;
      sonicMonitorCollectedMs = 0;
      sonicMonitorAction = SONIC_MONITOR_ACTION_NONE;
      sonicMonitorActionStartMs = 0;
      markSonicInteractiveObjectClearance(nowMs);
      scheduleNextSonicMonitor(nowMs);
    }

    return;
  }

  if (sonicEndRequested) {
    return;
  }

  if (!canSpawnSonicInteractiveObject(nowMs)) {
    return;
  }

  unsigned long runElapsed = nowMs - sonicRunStartMs;

  if (runElapsed < SONIC_MONITOR_FIRST_DELAY_MS) {
    return;
  }

  if (runElapsed + 5000UL >= SONIC_RUN_BEFORE_END_MS) {
    return;
  }

  if (sonicHillActive) {
    return;
  }

  if (nowMs >= sonicNextMonitorMs) {
    spawnSonicMonitor();
  }
}


// =====================================================
// SPRING 生成
// =====================================================
static void scheduleNextSonicSpring(unsigned long nowMs) {
  unsigned long gap = random(
    SONIC_SPRING_GAP_MIN_MS,
    SONIC_SPRING_GAP_MAX_MS + 1
  );

  sonicNextSpringMs = nowMs + gap;
}


static void spawnSonicSpring() {
  sonicSpringX = (float)SONIC_SPRING_SPAWN_X;
  sonicSpringActive = true;
  sonicSpringPressed = false;
}


static void updateSonicSpringSpawner(unsigned long nowMs) {
  if (sonicState != SONIC_STATE_RUN) {
    return;
  }

  if (sonicSpringActive) {
    if (
      sonicSpringAction == SONIC_SPRING_ACTION_NONE &&
      sonicSpringX + (float)SONIC_SPRING_FRAME_W < -4.0f
    ) {
      sonicSpringActive = false;
      sonicSpringPressed = false;
      markSonicInteractiveObjectClearance(nowMs);
      scheduleNextSonicSpring(nowMs);
    }

    return;
  }

  if (sonicEndRequested) {
    return;
  }

  if (!canSpawnSonicInteractiveObject(nowMs)) {
    return;
  }

  unsigned long runElapsed = nowMs - sonicRunStartMs;

  if (runElapsed < SONIC_SPRING_FIRST_DELAY_MS) {
    return;
  }

  if (runElapsed + 5000UL >= SONIC_RUN_BEFORE_END_MS) {
    return;
  }

  if (sonicHillActive) {
    return;
  }

  if (nowMs >= sonicNextSpringMs) {
    spawnSonicSpring();
  }
}

// =====================================================
// SIGN 繪製
// =====================================================
static void drawSonicSignMainFrame(int groupX, uint8_t signFrame, bool flipX) {
  if (signFrame == 1) {
    drawSonicImageScreen(
      groupX + SONIC_SIGN_MAIN_OFFSET_X,
      SONIC_SIGN_MAIN_Y,
      SONIC_SIGN1_W,
      SONIC_SIGN1_H,
      SONIC_SIGN1,
      false,
      true
    );

    return;
  }

  if (signFrame == 2) {
    drawSonicImageScreen(
      groupX + 8,
      SONIC_SIGN_MAIN_Y,
      SONIC_SIGN2_W,
      SONIC_SIGN2_H,
      SONIC_SIGN2,
      flipX,
      true
    );

    return;
  }

  if (signFrame == 3) {
    drawSonicImageScreen(
      groupX + 20,
      SONIC_SIGN_MAIN_Y,
      SONIC_SIGN3_W,
      SONIC_SIGN3_H,
      SONIC_SIGN3,
      false,
      true
    );

    return;
  }

  if (signFrame == 4) {
    drawSonicImageScreen(
      groupX + SONIC_SIGN_MAIN_OFFSET_X,
      SONIC_SIGN_MAIN_Y,
      SONIC_SIGN4_W,
      SONIC_SIGN4_H,
      SONIC_SIGN4,
      false,
      true
    );

    return;
  }

  if (signFrame == 5) {
    drawSonicImageScreen(
      groupX + SONIC_SIGN_MAIN_OFFSET_X,
      SONIC_SIGN_MAIN_Y,
      SONIC_SIGN5_W,
      SONIC_SIGN5_H,
      SONIC_SIGN5,
      false,
      true
    );

    return;
  }
}


static void drawSonicSignGroup(int groupX, uint8_t signFrame, bool flipMainFrame) {
  drawSonicImageScreen(
    groupX + SONIC_SIGN_BASE1_OFFSET_X,
    SONIC_SIGN_BASE1_Y,
    SONIC_SIGN_BASE1_W,
    SONIC_SIGN_BASE1_H,
    SONIC_SIGN_BASE1,
    false,
    false
  );

  drawSonicSignMainFrame(groupX, signFrame, flipMainFrame);

  drawSonicImageScreen(
    groupX + SONIC_SIGN_BASE2_OFFSET_X,
    SONIC_SIGN_BASE2_Y,
    SONIC_SIGN_BASE2_W,
    SONIC_SIGN_BASE2_H,
    SONIC_SIGN_BASE2,
    false,
    false
  );
}


// =====================================================
// SIGN 旋轉動畫
// =====================================================
static void getSonicSpinSignFrame(
  unsigned long nowMs,
  uint8_t* outFrame,
  bool* outFlip
) {
  unsigned long elapsed = nowMs - sonicStateStartMs;
  unsigned long phase = elapsed / SONIC_SIGN_SPIN_FRAME_MS;

  uint8_t pos = phase % 4;
  uint8_t group = (phase / 4) % 2;

  *outFrame = 2;
  *outFlip = false;

  if (pos == 0) {
    *outFrame = 2;
    *outFlip = false;
    return;
  }

  if (pos == 1) {
    *outFrame = 3;
    *outFlip = false;
    return;
  }

  if (pos == 2) {
    *outFrame = 2;
    *outFlip = true;
    return;
  }

  if (group == 0) {
    *outFrame = 4;
  }
  else {
    *outFrame = 5;
  }

  *outFlip = false;
}


// =====================================================
// Sonic frame 選擇
// =====================================================
static int getSonicIdleFrame(unsigned long nowMs) {
  uint8_t frames[2] = {1, 2};
  unsigned long phase = (nowMs - sonicStateStartMs) / SONIC_IDLE_FRAME_MS;
  return frames[phase % 2];
}


static int getSonicReadyFrame(unsigned long nowMs) {
  uint8_t frames[3] = {0, 3, 4};
  unsigned long elapsed = nowMs - sonicStateStartMs;
  int index = (int)(elapsed / SONIC_READY_FRAME_MS);

  if (index < 0) {
    index = 0;
  }

  if (index > 2) {
    index = 2;
  }

  return frames[index];
}


static int getSonicRunFrame(unsigned long nowMs) {
  uint8_t frames[4] = {5, 6, 7, 8};
  unsigned long phase = nowMs / SONIC_RUN_FRAME_MS;
  return frames[phase % 4];
}


static int getSonicWinFrame(unsigned long nowMs) {
  uint8_t frames[2] = {1, 2};
  unsigned long phase = (nowMs - sonicStateStartMs) / SONIC_WIN_FRAME_MS;
  return frames[phase % 2];
}


static bool isSonicTerrain5JumpFrame() {
  if (!sonicHillActive) {
    return false;
  }

  if (sonicHillType != SONIC_HILL_TYPE_5) {
    return false;
  }

  // 右腳進入缺口時先切 frame15。
  // 中心點還在缺口內時也維持 frame15，避免太早變回跑步造成踩空感。
  float sonicRightX = sonicX + (float)SONIC_FRAME_W - 2.0f;
  float sonicCenterX = sonicX + ((float)SONIC_FRAME_W * 0.5f);

  if (getSonicHillGapProgressAtScreenX(sonicRightX, NULL)) {
    return true;
  }

  if (getSonicHillGapProgressAtScreenX(sonicCenterX, NULL)) {
    return true;
  }

  return false;
}


static int getCurrentSonicFrame(unsigned long nowMs) {
  if (
    sonicState == SONIC_STATE_INIT_WAIT ||
    sonicState == SONIC_STATE_RESTART_WAIT
  ) {
    return getSonicIdleFrame(nowMs);
  }

  if (sonicState == SONIC_STATE_READY_ANIM) {
    return getSonicReadyFrame(nowMs);
  }

  if (sonicState == SONIC_STATE_READY_HOLD) {
    return 4;
  }

  if (sonicState == SONIC_STATE_RUN) {
    int springFrame = getSonicSpringActionFrame(nowMs);

    if (springFrame >= 0) {
      return springFrame;
    }

    int monitorFrame = getSonicMonitorActionFrame(nowMs);

    if (monitorFrame >= 0) {
      return monitorFrame;
    }

    if (isSonicTerrain5JumpFrame()) {
      return 15;
    }
  }

  if (
    sonicState == SONIC_STATE_RUN ||
    sonicState == SONIC_STATE_END_SIGN_IN
  ) {
    return getSonicRunFrame(nowMs);
  }

  if (sonicState == SONIC_STATE_END_FINISH) {
    if (sonicX < (float)SONIC_END_X) {
      return getSonicRunFrame(nowMs);
    }

    return 0;
  }

  if (sonicState == SONIC_STATE_RESULT_SIGN_WAIT) {
    return 0;
  }

  if (sonicState == SONIC_STATE_RESULT_SONIC_ANIM) {
    if (sonicResultWin) {
      return getSonicWinFrame(nowMs);
    }

    return 9;
  }

  return 0;
}


// =====================================================
// 狀態切換
// =====================================================
static void resetSonicSceneToInit() {
  sonicX = (float)SONIC_IDLE_X;
  sonicY = (float)SONIC_IDLE_Y;
  sonicFlipX = false;

  sonicBgScrollX = 0.0f;
  sonicGroundScrollX = 0.0f;
  sonicSceneOffsetY = 0;

  sonicRunStartMs = 0;
  sonicNextHillMs = 0;
  sonicEndRequested = false;
  sonicEndFlatStartMs = 0;
  sonicNextObjectAllowedMs = 0;

  sonicHillActive = false;
  sonicHillType = SONIC_HILL_NONE;
  sonicHillX = 0.0f;
  sonicHillWidth = 0;
  sonicHillPlateauBlocks = 0;

  sonicTreeActive = false;
  sonicTreeX = 0.0f;
  sonicNextTreeMs = 0;

  sonicRingActive = false;
  sonicRingCollected = false;
  sonicRingX = 0.0f;
  sonicNextRingMs = 0;
  sonicRingCollectedMs = 0;

  sonicMonitorActive = false;
  sonicMonitorCollected = false;
  sonicMonitorX = 0.0f;
  sonicMonitorFrame = 0;
  sonicNextMonitorMs = 0;
  sonicMonitorCollectedMs = 0;
  sonicMonitorAction = SONIC_MONITOR_ACTION_NONE;
  sonicMonitorActionStartMs = 0;

  sonicSpringActive = false;
  sonicSpringPressed = false;
  sonicSpringX = 0.0f;
  sonicNextSpringMs = 0;
  sonicSpringAction = SONIC_SPRING_ACTION_NONE;
  sonicSpringActionStartMs = 0;

  sonicSignX = (float)SONIC_SCR_W;

  sonicResultWin = true;
  sonicResultDecided = false;
  sonicEndGoalReachMs = 0;
}


static void setSonicState(uint8_t newState, unsigned long nowMs) {
  sonicState = newState;
  sonicStateStartMs = nowMs;

  if (newState == SONIC_STATE_INIT_WAIT) {
    resetSonicSceneToInit();
  }

  if (newState == SONIC_STATE_READY_ANIM) {
    sonicX = (float)SONIC_IDLE_X;
    sonicY = (float)SONIC_IDLE_Y;
    sonicFlipX = false;
    sonicSceneOffsetY = 0;
  }

  if (newState == SONIC_STATE_READY_HOLD) {
    sonicX = (float)SONIC_IDLE_X;
    sonicY = (float)SONIC_IDLE_Y;
    sonicFlipX = false;
    sonicSceneOffsetY = 0;
  }

  if (newState == SONIC_STATE_RUN) {
    sonicRunStartMs = nowMs;
    sonicNextHillMs = nowMs + SONIC_HILL_FIRST_DELAY_MS;
    sonicNextTreeMs = nowMs + SONIC_TREE_FIRST_DELAY_MS;
    sonicNextRingMs = nowMs + SONIC_RING_FIRST_DELAY_MS;
    sonicNextMonitorMs = nowMs + SONIC_MONITOR_FIRST_DELAY_MS;
    sonicNextSpringMs = nowMs + SONIC_SPRING_FIRST_DELAY_MS;

    sonicEndRequested = false;
    sonicEndFlatStartMs = 0;
    sonicNextObjectAllowedMs = 0;

    sonicTreeActive = false;

    sonicRingActive = false;
    sonicRingCollected = false;
    sonicRingCollectedMs = 0;

    sonicMonitorActive = false;
    sonicMonitorCollected = false;
    sonicMonitorCollectedMs = 0;
    sonicMonitorAction = SONIC_MONITOR_ACTION_NONE;
    sonicMonitorActionStartMs = 0;

    sonicSpringActive = false;
    sonicSpringPressed = false;
    sonicSpringAction = SONIC_SPRING_ACTION_NONE;
    sonicSpringActionStartMs = 0;

    sonicFlipX = false;
    sonicSceneOffsetY = 0;
  }

  if (newState == SONIC_STATE_END_SIGN_IN) {
    sonicSignX = (float)SONIC_SCR_W;

    sonicHillActive = false;
    sonicTreeActive = false;

    sonicRingActive = false;
    sonicRingCollected = false;
    sonicRingCollectedMs = 0;

    sonicMonitorActive = false;
    sonicMonitorCollected = false;
    sonicMonitorCollectedMs = 0;
    sonicMonitorAction = SONIC_MONITOR_ACTION_NONE;
    sonicMonitorActionStartMs = 0;

    sonicSpringActive = false;
    sonicSpringPressed = false;
    sonicSpringAction = SONIC_SPRING_ACTION_NONE;
    sonicSpringActionStartMs = 0;

    sonicSceneOffsetY = 0;
    sonicY = (float)SONIC_BASE_Y;
  }

  if (newState == SONIC_STATE_END_FINISH) {
    sonicSignX = 0.0f;
    sonicHillActive = false;
    sonicTreeActive = false;

    sonicRingActive = false;
    sonicRingCollected = false;
    sonicRingCollectedMs = 0;

    sonicMonitorActive = false;
    sonicMonitorCollected = false;
    sonicMonitorCollectedMs = 0;
    sonicMonitorAction = SONIC_MONITOR_ACTION_NONE;
    sonicMonitorActionStartMs = 0;

    sonicSpringActive = false;
    sonicSpringPressed = false;
    sonicSpringAction = SONIC_SPRING_ACTION_NONE;
    sonicSpringActionStartMs = 0;

    sonicSceneOffsetY = 0;
    sonicY = (float)SONIC_BASE_Y;
    sonicEndGoalReachMs = 0;
    sonicResultDecided = false;
  }

  if (newState == SONIC_STATE_RESULT_SIGN_WAIT) {
    sonicResultWin = (random(0, 100) < SONIC_WIN_CHANCE_PERCENT);
    sonicResultDecided = true;
    sonicSignX = 0.0f;
    sonicX = (float)SONIC_END_X;
    sonicY = (float)SONIC_BASE_Y;
    sonicSceneOffsetY = 0;
  }

  if (newState == SONIC_STATE_RESULT_SONIC_ANIM) {
    sonicX = (float)SONIC_END_X;
    sonicY = (float)SONIC_BASE_Y;
    sonicSceneOffsetY = 0;
  }

  if (newState == SONIC_STATE_RESTART_WAIT) {
    ClearAll();
    resetSonicSceneToInit();
  }
}


// =====================================================
// 跑步場景更新
// =====================================================
static void updateSonicScrolling(float dtSec) {
  float speed = SONIC_RUN_SPEED_PX_PER_SEC;

  if (
    sonicSpringAction == SONIC_SPRING_ACTION_JUMP ||
    sonicSpringAction == SONIC_SPRING_ACTION_FALL
  ) {
    speed *= SONIC_SPRING_JUMP_SCROLL_MULTIPLIER;
  }

  float movePx = speed * dtSec;

  sonicGroundScrollX += movePx;
  sonicBgScrollX += movePx * SONIC_BG_SCROLL_RATIO;

  if (sonicGroundScrollX > 100000.0f) {
    sonicGroundScrollX = 0.0f;
  }

  if (sonicBgScrollX > 100000.0f) {
    sonicBgScrollX = 0.0f;
  }

  if (sonicHillActive) {
    sonicHillX -= movePx;
  }

  if (sonicTreeActive) {
    sonicTreeX -= movePx;
  }

  if (sonicRingActive) {
    sonicRingX -= movePx;
  }

  if (sonicMonitorActive) {
    sonicMonitorX -= movePx;
  }

  if (sonicSpringActive) {
    sonicSpringX -= movePx;
  }
}


static void updateSonicRunPosition(float dtSec) {
  if (sonicX < (float)SONIC_RUN_LOCK_X) {
    sonicX += SONIC_RUN_TO_X_SPEED * dtSec;

    if (sonicX > (float)SONIC_RUN_LOCK_X) {
      sonicX = (float)SONIC_RUN_LOCK_X;
    }
  }
}


static void updateSonicEndMove(float dtSec, unsigned long nowMs) {
  if (sonicX < (float)SONIC_END_X) {
    sonicX += SONIC_END_MOVE_SPEED * dtSec;

    if (sonicX >= (float)SONIC_END_X) {
      sonicX = (float)SONIC_END_X;
      sonicEndGoalReachMs = nowMs;
    }
  }
  else {
    sonicX = (float)SONIC_END_X;

    if (sonicEndGoalReachMs == 0) {
      sonicEndGoalReachMs = nowMs;
    }
  }

  if (
    sonicEndGoalReachMs > 0 &&
    nowMs - sonicEndGoalReachMs >= SONIC_TURN_WAIT_MS
  ) {
    sonicFlipX = true;
  }
}


static void updateSonicEndRequest(unsigned long nowMs) {
  if (sonicState != SONIC_STATE_RUN) {
    return;
  }

  unsigned long runElapsed = nowMs - sonicRunStartMs;

  if (!sonicEndRequested && runElapsed >= SONIC_RUN_BEFORE_END_MS) {
    sonicEndRequested = true;
    sonicEndFlatStartMs = 0;
  }

  if (!sonicEndRequested) {
    return;
  }

  if (
    sonicHillActive ||
    sonicMonitorAction != SONIC_MONITOR_ACTION_NONE ||
    sonicSpringAction != SONIC_SPRING_ACTION_NONE
  ) {
    sonicEndFlatStartMs = 0;
    return;
  }

  if (sonicEndFlatStartMs == 0) {
    sonicEndFlatStartMs = nowMs;
    return;
  }

  if (nowMs - sonicEndFlatStartMs >= SONIC_END_FLAT_AFTER_TERRAIN_MS) {
    setSonicState(SONIC_STATE_END_SIGN_IN, nowMs);
  }
}


// =====================================================
// 狀態機更新
// =====================================================
static void updateSonicState(unsigned long nowMs) {
  unsigned long rawDt = nowMs - sonicLastUpdateMs;
  sonicLastUpdateMs = nowMs;

  if (rawDt > 200UL) {
    rawDt = 200UL;
  }

  float dtSec = (float)rawDt / 1000.0f;
  unsigned long elapsed = nowMs - sonicStateStartMs;

  switch (sonicState) {
    case SONIC_STATE_INIT_WAIT: {
      if (elapsed >= SONIC_FIRST_START_WAIT_MS) {
        setSonicState(SONIC_STATE_READY_ANIM, nowMs);
      }

      break;
    }

    case SONIC_STATE_READY_ANIM: {
      unsigned long readyTotal = SONIC_READY_FRAME_MS * 3UL;

      if (elapsed >= readyTotal) {
        setSonicState(SONIC_STATE_READY_HOLD, nowMs);
      }

      break;
    }

    case SONIC_STATE_READY_HOLD: {
      if (elapsed >= SONIC_READY_HOLD_MS) {
        setSonicState(SONIC_STATE_RUN, nowMs);
      }

      break;
    }

    case SONIC_STATE_RUN: {
      updateSonicScrolling(dtSec);
      updateSonicRunPosition(dtSec);

      updateSonicHillSpawner(nowMs);
      updateSonicTreeSpawner(nowMs);
      updateSonicRingSpawner(nowMs);
      updateSonicMonitorSpawner(nowMs);
      updateSonicSpringSpawner(nowMs);

      updateSonicYByHill();
      updateSonicRingCollision(nowMs);
      updateSonicMonitorAction(nowMs);
      updateSonicSpringAction(nowMs);

      updateSonicEndRequest(nowMs);

      break;
    }

    case SONIC_STATE_END_SIGN_IN: {
      updateSonicScrolling(dtSec);
      updateSonicRunPosition(dtSec);

      sonicSceneOffsetY = 0;
      sonicY = (float)SONIC_BASE_Y;

      sonicSignX -= SONIC_SIGN_MOVE_SPEED * dtSec;

      if (sonicSignX <= 0.0f) {
        sonicSignX = 0.0f;
        setSonicState(SONIC_STATE_END_FINISH, nowMs);
      }

      break;
    }

    case SONIC_STATE_END_FINISH: {
      sonicSceneOffsetY = 0;
      sonicY = (float)SONIC_BASE_Y;
      updateSonicEndMove(dtSec, nowMs);

      bool spinDone = (elapsed >= SONIC_SIGN_SPIN_TOTAL_MS);
      bool sonicTurnDone = (
        sonicEndGoalReachMs > 0 &&
        nowMs - sonicEndGoalReachMs >= SONIC_TURN_WAIT_MS
      );

      if (spinDone && sonicTurnDone) {
        setSonicState(SONIC_STATE_RESULT_SIGN_WAIT, nowMs);
      }

      break;
    }

    case SONIC_STATE_RESULT_SIGN_WAIT: {
      sonicSceneOffsetY = 0;
      sonicX = (float)SONIC_END_X;
      sonicY = (float)SONIC_BASE_Y;

      if (elapsed >= SONIC_RESULT_SIGN_HOLD_MS) {
        setSonicState(SONIC_STATE_RESULT_SONIC_ANIM, nowMs);
      }

      break;
    }

    case SONIC_STATE_RESULT_SONIC_ANIM: {
      sonicSceneOffsetY = 0;
      sonicX = (float)SONIC_END_X;
      sonicY = (float)SONIC_BASE_Y;

      if (elapsed >= SONIC_RESULT_HOLD_MS) {
        setSonicState(SONIC_STATE_RESTART_WAIT, nowMs);
      }

      break;
    }

    case SONIC_STATE_RESTART_WAIT: {
      if (elapsed >= SONIC_RESTART_WAIT_MS) {
        setSonicState(SONIC_STATE_READY_ANIM, nowMs);
      }

      break;
    }

    default: {
      setSonicState(SONIC_STATE_INIT_WAIT, nowMs);
      break;
    }
  }
}


// =====================================================
// 繪製 SIGN
// =====================================================
static void renderSonicSign(unsigned long nowMs) {
  if (sonicState == SONIC_STATE_END_SIGN_IN) {
    drawSonicSignGroup((int)sonicSignX, 1, false);
    return;
  }

  if (sonicState == SONIC_STATE_END_FINISH) {
    uint8_t frame = 2;
    bool flip = false;

    getSonicSpinSignFrame(nowMs, &frame, &flip);
    drawSonicSignGroup(0, frame, flip);
    return;
  }

  if (
    sonicState == SONIC_STATE_RESULT_SIGN_WAIT ||
    sonicState == SONIC_STATE_RESULT_SONIC_ANIM
  ) {
    if (sonicResultWin) {
      drawSonicSignGroup(0, 5, false);
    }
    else {
      drawSonicSignGroup(0, 4, false);
    }

    return;
  }
}


// =====================================================
// 繪製整體場景
// =====================================================
static void renderSonicScene(unsigned long nowMs) {
  drawSonicBackground();

  drawSonicTree();
  drawSonicGroundBase();
  drawSonicHill();

  drawSonicMonitor(nowMs);
  drawSonicSpring(nowMs);
  drawSonicRing(nowMs);

  renderSonicSign(nowMs);

  int sonicFrame = getCurrentSonicFrame(nowMs);

  drawSonicFrame(
    sonicFrame,
    (int)sonicX,
    (int)sonicY,
    sonicFlipX
  );

  if (SONIC_DRAW_CLOCK_TEXT) {
    drawThemeClockText();
  }
}


// =====================================================
// 初始化
// =====================================================
static void SonicModeInit() {
  if (!ModefirstRun) {
    return;
  }

  randomSeed(millis());

  sonicLastUpdateMs = millis();

  setSonicState(SONIC_STATE_INIT_WAIT, millis());

  ModefirstRun = false;
}


// =====================================================
// 主函式
// =====================================================
void SonicMode() {
  SonicModeInit();

  unsigned long nowMs = millis();

  updateSonicState(nowMs);
  renderSonicScene(nowMs);

  wait_with_display(SONIC_FRAME_DELAY_MS);
}
