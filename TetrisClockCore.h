#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>

// =====================================================
// 1. 型別定義
// =====================================================
struct fall_instr {
  uint8_t blocktype;
  uint8_t color;
  uint8_t x_pos;
  uint8_t y_stop;
  uint8_t num_rot;
};

// =====================================================
// 2. 數字座標表 (PROGMEM)
// =====================================================
#define SIZE_NUM_0 12
static const fall_instr num_0[SIZE_NUM_0] PROGMEM = {
  {2, 5, 4, 16, 0},{4, 7, 2, 16, 1},{3, 4, 0, 16, 1},{6, 6, 1, 16, 1},
  {5, 1, 4, 14, 0},{6, 6, 0, 13, 3},{5, 1, 4, 12, 0},{5, 1, 0, 11, 0},
  {6, 6, 4, 10, 1},{6, 6, 0, 9, 1},{5, 1, 1, 8, 1},{2, 5, 3, 8, 3}
};

#define SIZE_NUM_1 5
static const fall_instr num_1[SIZE_NUM_1] PROGMEM = {
  {2, 5, 2, 16, 0},{3, 4, 2, 15, 1},{3, 4, 3, 13, 3},{2, 5, 2, 11, 2},{0, 0, 2, 8, 0}
};

#define SIZE_NUM_2 11
static const fall_instr num_2[SIZE_NUM_2] PROGMEM = {
  {0, 0, 4, 16, 0},{3, 4, 0, 16, 1},{1, 2, 1, 16, 3},{1, 2, 1, 15, 0},
  {3, 4, 1, 12, 2},{1, 2, 0, 12, 1},{2, 5, 3, 12, 3},{0, 0, 4, 10, 0},
  {3, 4, 1, 8, 0},{2, 5, 3, 8, 3},{1, 2, 0, 8, 1}
};

#define SIZE_NUM_3 11
static const fall_instr num_3[SIZE_NUM_3] PROGMEM = {
  {1, 2, 3, 16, 3},{2, 5, 0, 16, 1},{3, 4, 1, 15, 2},{0, 0, 4, 14, 0},
  {3, 4, 1, 12, 2},{1, 2, 0, 12, 1},{3, 4, 5, 12, 3},{2, 5, 3, 11, 0},
  {3, 4, 1, 8, 0},{1, 2, 0, 8, 1},{2, 5, 3, 8, 3}
};

#define SIZE_NUM_4 9
static const fall_instr num_4[SIZE_NUM_4] PROGMEM = {
  {0, 0, 4, 16, 0},{0, 0, 4, 14, 0},{3, 4, 1, 12, 0},{1, 2, 0, 12, 1},
  {2, 5, 0, 10, 0},{2, 5, 3, 12, 3},{3, 4, 4, 10, 3},{2, 5, 0, 9, 2},{3, 4, 5, 10, 1}
};

#define SIZE_NUM_5 11
static const fall_instr num_5[SIZE_NUM_5] PROGMEM = {
  {0, 0, 0, 16, 0},{2, 5, 2, 16, 1},{2, 5, 3, 15, 0},{3, 4, 5, 16, 1},
  {3, 4, 1, 12, 0},{1, 2, 0, 12, 1},{2, 5, 3, 12, 3},{0, 0, 0, 10, 0},
  {3, 4, 1, 8, 2},{1, 2, 0, 8, 1},{2, 5, 3, 8, 3}
};

#define SIZE_NUM_6 12
static const fall_instr num_6[SIZE_NUM_6] PROGMEM = {
  {2, 5, 0, 16, 1},{5, 1, 2, 16, 1},{6, 6, 0, 15, 3},{6, 6, 4, 16, 3},
  {5, 1, 4, 14, 0},{3, 4, 1, 12, 2},{2, 5, 0, 13, 2},{3, 4, 2, 11, 0},
  {0, 0, 0, 10, 0},{3, 4, 1, 8, 0},{1, 2, 0, 8, 1},{2, 5, 3, 8, 3}
};

#define SIZE_NUM_7 7
static const fall_instr num_7[SIZE_NUM_7] PROGMEM = {
  {0, 0, 4, 16, 0},{1, 2, 4, 14, 0},{3, 4, 5, 13, 1},{2, 5, 4, 11, 2},
  {3, 4, 1, 8, 2},{2, 5, 3, 8, 3},{1, 2, 0, 8, 1}
};

#define SIZE_NUM_8 13
static const fall_instr num_8[SIZE_NUM_8] PROGMEM = {
  {3, 4, 1, 16, 0},{6, 6, 0, 16, 1},{3, 4, 5, 16, 1},{1, 2, 2, 15, 3},
  {4, 7, 0, 14, 0},{1, 2, 1, 12, 3},{6, 6, 4, 13, 1},{2, 5, 0, 11, 1},
  {4, 7, 0, 10, 0},{4, 7, 4, 11, 0},{5, 1, 0, 8, 1},{5, 1, 2, 8, 1},{1, 2, 4, 9, 2}
};

#define SIZE_NUM_9 12
static const fall_instr num_9[SIZE_NUM_9] PROGMEM = {
  {0, 0, 0, 16, 0},{3, 4, 2, 16, 0},{1, 2, 2, 15, 3},{1, 2, 4, 15, 2},
  {3, 4, 1, 12, 2},{3, 4, 5, 12, 3},{5, 1, 0, 12, 0},{1, 2, 2, 11, 3},
  {5, 1, 4, 9, 0},{6, 6, 0, 10, 1},{5, 1, 0, 8, 1},{6, 6, 2, 8, 2}
};

static const uint8_t blocksPerNumber[10] PROGMEM = {
  SIZE_NUM_0, SIZE_NUM_1, SIZE_NUM_2, SIZE_NUM_3, SIZE_NUM_4,
  SIZE_NUM_5, SIZE_NUM_6, SIZE_NUM_7, SIZE_NUM_8, SIZE_NUM_9
};

static inline const fall_instr* getNumTable(uint8_t num) {
  switch (num) {
    case 0: return num_0;
    case 1: return num_1;
    case 2: return num_2;
    case 3: return num_3;
    case 4: return num_4;
    case 5: return num_5;
    case 6: return num_6;
    case 7: return num_7;
    case 8: return num_8;
    default: return num_9;
  }
}

// =====================================================
// 3. Tetromino 形狀查表
// =====================================================
static const int8_t SHAPES[8][4][4][2] PROGMEM = {
  { {{0,0},{1,0},{0,1},{1,1}}, {{0,0},{1,0},{0,1},{1,1}}, {{0,0},{1,0},{0,1},{1,1}}, {{0,0},{1,0},{0,1},{1,1}} }, // O
  { {{0,0},{1,0},{0,1},{0,2}}, {{0,0},{0,1},{1,1},{2,1}}, {{1,0},{1,1},{1,2},{0,2}}, {{0,0},{1,0},{2,0},{2,1}} }, // J
  { {{0,0},{1,0},{1,1},{1,2}}, {{0,0},{1,0},{2,0},{0,1}}, {{0,0},{0,1},{0,2},{1,2}}, {{0,1},{1,1},{2,1},{2,0}} }, // L
  { {{0,0},{1,0},{2,0},{3,0}}, {{0,0},{0,1},{0,2},{0,3}}, {{0,0},{1,0},{2,0},{3,0}}, {{0,0},{0,1},{0,2},{0,3}} }, // I
  { {{1,0},{0,1},{1,1},{0,2}}, {{0,0},{1,0},{1,1},{2,1}}, {{1,0},{0,1},{1,1},{0,2}}, {{0,0},{1,0},{1,1},{2,1}} }, // S
  { {{0,0},{0,1},{1,1},{1,2}}, {{1,0},{2,0},{0,1},{1,1}}, {{0,0},{0,1},{1,1},{1,2}}, {{1,0},{2,0},{0,1},{1,1}} }, // Z
  { {{0,0},{1,0},{2,0},{1,1}}, {{0,0},{0,1},{0,2},{1,1}}, {{1,0},{0,1},{1,1},{2,1}}, {{1,0},{0,1},{1,1},{1,2}} }, // T
  { {{0,0},{1,0},{0,1},{127,127}}, {{0,0},{0,1},{1,1},{127,127}}, {{1,0},{1,1},{0,1},{127,127}}, {{0,0},{1,0},{1,1},{127,127}} }  // corner
};

// =====================================================
// 4. 狀態
// needsClear:
//   當某一位數字改變時，先清掉該位欄位，避免舊數字殘留
// =====================================================
struct NumState {
  int8_t num;
  uint8_t blockIndex;
  uint8_t fallIndex;
  uint8_t xShift;
  bool needsClear;
};

// =====================================================
// 5. Renderer 類別
// =====================================================
class TetrisClockRenderer {
public:
  explicit TetrisClockRenderer(Adafruit_GFX &g) : gfx(&g) {
    reset();
  }

  void reset() {
    for (uint8_t i = 0; i < 4; i++) {
      st[i].num = -1;
      st[i].blockIndex = 0;
      st[i].fallIndex = 0;
      st[i].xShift = 0;
      st[i].needsClear = true;
    }
    scale = 2;
  }

  void setDigits(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, bool force) {
    setOne(0, d0, 0,  force);   // 小時十位
    setOne(1, d1, 15, force);   // 小時個位
    setOne(2, d2, 34, force);   // 分鐘十位
    setOne(3, d3, 49, force);   // 分鐘個位
  }

  bool drawFrame(int xStart, int yFinish, bool showColon) {
    bool allDone = true;
    const int yStart = yFinish - (16 * scale);

    for (uint8_t pos = 0; pos < 4; pos++) {
      if (st[pos].num < 0) continue;

      // -------------------------------------------------
      // 數字剛切換時，先把這一位數字的欄位清空
      // 這樣 0 -> 1、8 -> 3 等情況就不會殘留舊方塊
      // -------------------------------------------------
      if (st[pos].needsClear) {
        gfx->fillRect(
          xStart + st[pos].xShift,
          yStart - scale,
          13 * scale,
          (17 * scale) + scale,
          0x0000
        );
        st[pos].needsClear = false;
      }

      uint8_t num = (uint8_t)st[pos].num;
      uint8_t total = pgm_read_byte(&blocksPerNumber[num]);

      // -------------------------------------------------
      // 還有方塊在掉落中
      // -------------------------------------------------
      if (st[pos].blockIndex < total) {
        allDone = false;

        fall_instr cur;
        memcpy_P(&cur, getNumTable(num) + st[pos].blockIndex, sizeof(fall_instr));

        // 清掉上一幀掉落中的位置
        if (st[pos].fallIndex > 0) {
          drawShape(
            cur.blocktype,
            cur.num_rot,
            0x0000,
            xStart + st[pos].xShift + (cur.x_pos * scale),
            yStart + ((st[pos].fallIndex - 1) * scale)
          );
        }

        // 畫出這一幀掉落中的位置
        uint16_t col = colors[cur.color % 9];
        drawShape(
          cur.blocktype,
          cur.num_rot,
          col,
          xStart + st[pos].xShift + (cur.x_pos * scale),
          yStart + (st[pos].fallIndex * scale)
        );

        st[pos].fallIndex++;

        if (st[pos].fallIndex > cur.y_stop) {
          st[pos].fallIndex = 0;
          st[pos].blockIndex++;
        }
      }

      // -------------------------------------------------
      // 畫已經固定到底部的方塊
      // -------------------------------------------------
      for (uint8_t i = 0; i < st[pos].blockIndex; i++) {
        fall_instr f;
        memcpy_P(&f, getNumTable(num) + i, sizeof(fall_instr));

        uint16_t col = colors[f.color % 9];
        drawShape(
          f.blocktype,
          f.num_rot,
          col,
          xStart + st[pos].xShift + (f.x_pos * scale),
          yStart + (f.y_stop * scale)
        );
      }
    }

    if (showColon) drawColon(xStart, yFinish);
    return allDone;
  }

  uint8_t scale = 2;

private:
  Adafruit_GFX *gfx;
  NumState st[4];

  const uint16_t colors[9] = {
    0xF800, // red
    0x07E0, // green
    0x325F, // blue-ish
    0xFFFF, // white
    0xFFE0, // yellow
    0x07FF, // cyan
    0xF81F, // magenta
    0xFB00, // orange
    0x0000  // black
  };

  void setOne(uint8_t idx, uint8_t num, uint8_t xShift, bool force) {
    if (!force && st[idx].num == (int8_t)num) return;

    st[idx].needsClear = true;
    st[idx].num = (int8_t)num;
    st[idx].xShift = xShift;
    st[idx].blockIndex = 0;
    st[idx].fallIndex = 0;
  }

  void drawShape(uint8_t type, uint8_t rot, uint16_t c, int x, int yBottom) {
    for (uint8_t i = 0; i < 4; i++) {
      int8_t dx = (int8_t)pgm_read_byte(&SHAPES[type][rot & 3][i][0]);
      int8_t dy = (int8_t)pgm_read_byte(&SHAPES[type][rot & 3][i][1]);
      if (dx == 127) continue;

      gfx->fillRect(
        x + dx * scale,
        yBottom - dy * scale,
        scale,
        scale,
        c
      );
    }
  }

  void drawColon(int xStart, int yFinish) {
    int xPos = xStart + 30;
    gfx->fillRect(xPos, yFinish - 3,  scale, scale, 0xFFFF);
    gfx->fillRect(xPos, yFinish - 15, scale, scale, 0xFFFF);
  }
};
