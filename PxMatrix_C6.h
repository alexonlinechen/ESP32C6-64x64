/*********************************************************************
  PxMatrix_C6 - Arduino 版，改為接近 IDF 顯示邏輯
  測試修正版：
  - ESP32-C6
  - 64x64 HUB75
  - 1:32 scan
  - 針對「某一水平線瞬間特別亮 / 像往下掃」做時序修正

  修正重點：
  1. refreshSlice() 加 noInterrupts()/interrupts()
  2. OE 關閉後先清 RGB，避免換列殘光
  3. 拿掉原本多餘的 delayMicroseconds(1)
  4. 改成先 shift -> latch -> set address -> OE on
  5. latch 前再清一次 RGB，降低鬼影與亮線
*********************************************************************/

#ifndef _PxMATRIX_H
#define _PxMATRIX_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <string.h>
#include "soc/gpio_reg.h"
#include "soc/soc.h"

#ifndef PxMATRIX_COLOR_DEPTH
#define PxMATRIX_COLOR_DEPTH 3
#endif

#if PxMATRIX_COLOR_DEPTH != 3
#error "This PxMatrix_C6.h is designed for PxMATRIX_COLOR_DEPTH == 3"
#endif

class PxMATRIX : public Adafruit_GFX {
public:
  PxMATRIX(uint16_t width, uint16_t height,
           uint8_t LATCH, uint8_t OE,
           uint8_t A, uint8_t B, uint8_t C, uint8_t D, uint8_t E)
    : Adafruit_GFX(width, height) {
    _LATCH_PIN = LATCH;
    _OE_PIN    = OE;
    _A_PIN     = A;
    _B_PIN     = B;
    _C_PIN     = C;
    _D_PIN     = D;
    _E_PIN     = E;
    _width     = width;
    _height    = height;
  }

  ~PxMATRIX() {
    if (_fb) {
      delete[] _fb;
      _fb = nullptr;
    }
  }

void setRGBPins(uint8_t r1, uint8_t g1, uint8_t b1,
                uint8_t r2, uint8_t g2, uint8_t b2,
                uint8_t clk) {

  // 先算好新的 GPIO mask
  uint32_t r1Mask = bitMask(r1);
  uint32_t g1Mask = bitMask(g1);
  uint32_t b1Mask = bitMask(b1);

  uint32_t r2Mask = bitMask(r2);
  uint32_t g2Mask = bitMask(g2);
  uint32_t b2Mask = bitMask(b2);

  uint32_t clkMask = bitMask(clk);

  // 避免正在掃描時剛好切換 RGB mapping
  noInterrupts();

  _R1 = r1;
  _G1 = g1;
  _B1 = b1;

  _R2 = r2;
  _G2 = g2;
  _B2 = b2;

  _CLK_PIN = clk;

  // 立即更新高速 GPIO Mask
  _R1_MASK = r1Mask;
  _G1_MASK = g1Mask;
  _B1_MASK = b1Mask;

  _R2_MASK = r2Mask;
  _G2_MASK = g2Mask;
  _B2_MASK = b2Mask;

  _CLK_MASK = clkMask;

  _RGB_MASK_ALL =
      _R1_MASK | _G1_MASK | _B1_MASK |
      _R2_MASK | _G2_MASK | _B2_MASK;

  interrupts();
}

  void begin() {
    pinMode(_LATCH_PIN, OUTPUT);
    pinMode(_OE_PIN, OUTPUT);

    pinMode(_A_PIN, OUTPUT);
    pinMode(_B_PIN, OUTPUT);
    pinMode(_C_PIN, OUTPUT);
    pinMode(_D_PIN, OUTPUT);
    pinMode(_E_PIN, OUTPUT);

    pinMode(_CLK_PIN, OUTPUT);

    pinMode(_R1, OUTPUT);
    pinMode(_G1, OUTPUT);
    pinMode(_B1, OUTPUT);
    pinMode(_R2, OUTPUT);
    pinMode(_G2, OUTPUT);
    pinMode(_B2, OUTPUT);

    digitalWrite(_OE_PIN, HIGH);
    digitalWrite(_LATCH_PIN, LOW);
    digitalWrite(_CLK_PIN, LOW);

    if (_fb) {
      delete[] _fb;
      _fb = nullptr;
    }

    _fb_size = (uint32_t)_width * (uint32_t)_height * 3UL;
    _fb = new uint8_t[_fb_size];
    clearDisplay();

    _scan_plane = PxMATRIX_COLOR_DEPTH - 1;
    _scan_row = 0;

    _R1_MASK = bitMask(_R1);
    _G1_MASK = bitMask(_G1);
    _B1_MASK = bitMask(_B1);
    _R2_MASK = bitMask(_R2);
    _G2_MASK = bitMask(_G2);
    _B2_MASK = bitMask(_B2);
    _CLK_MASK = bitMask(_CLK_PIN);
    _LAT_MASK = bitMask(_LATCH_PIN);
    _OE_MASK  = bitMask(_OE_PIN);

    _ADDR_MASK =
      bitMask(_A_PIN) |
      bitMask(_B_PIN) |
      bitMask(_C_PIN) |
      bitMask(_D_PIN) |
      bitMask(_E_PIN);

    _RGB_MASK_ALL =
      _R1_MASK | _G1_MASK | _B1_MASK |
      _R2_MASK | _G2_MASK | _B2_MASK;

    fastSetHigh(_OE_MASK);
    fastSetLow(_LAT_MASK | _CLK_MASK);
    fastSetLow(_ADDR_MASK);
    fastSetLow(_RGB_MASK_ALL);
  }

  void clearDisplay() {
    if (_fb) {
      memset(_fb, 0, _fb_size);
    }
  }

  void clearDisplay(bool) {
    clearDisplay();
  }

  void flushDisplay() {
    clearDisplay();
  }

  void showBuffer() {
    display();
  }

  uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  }

void setBrightness(uint8_t brightness) {
  _brightness = brightness;
}

  void display() {
    display(_slices_per_call);
  }

  void display(uint16_t slices) {
    if (slices == 0) slices = 1;
    while (slices--) {
      refreshSlice();
    }
  }

  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
    if (!_fb) return;
    if (x < 0 || x >= _width || y < 0 || y >= _height) return;

    uint8_t r5 = (uint8_t)((color >> 11) & 0x1F);
    uint8_t g6 = (uint8_t)((color >> 5)  & 0x3F);
    uint8_t b5 = (uint8_t)(color & 0x1F);

    uint8_t r8 = (uint8_t)((r5 << 3) | (r5 >> 2));
    uint8_t g8 = (uint8_t)((g6 << 2) | (g6 >> 4));
    uint8_t b8 = (uint8_t)((b5 << 3) | (b5 >> 2));

    setPixelRGB888(x, y, r8, g8, b8);
  }

  void fillScreen(uint16_t color) override {
    if (!_fb) return;

    uint8_t r5 = (uint8_t)((color >> 11) & 0x1F);
    uint8_t g6 = (uint8_t)((color >> 5)  & 0x3F);
    uint8_t b5 = (uint8_t)(color & 0x1F);

    uint8_t r8 = (uint8_t)((r5 << 3) | (r5 >> 2));
    uint8_t g8 = (uint8_t)((g6 << 2) | (g6 >> 4));
    uint8_t b8 = (uint8_t)((b5 << 3) | (b5 >> 2));

    uint32_t pixels = (uint32_t)_width * (uint32_t)_height;
    uint8_t *p = _fb;

    while (pixels--) {
      *p++ = r8;
      *p++ = g8;
      *p++ = b8;
    }
  }

  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override {
    if (!_fb) return;
    if (w <= 0 || h <= 0) return;

    int16_t x2 = x + w - 1;
    int16_t y2 = y + h - 1;

    if (x >= _width || y >= _height) return;
    if (x2 < 0 || y2 < 0) return;

    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x2 >= _width)  x2 = _width - 1;
    if (y2 >= _height) y2 = _height - 1;

    uint8_t r5 = (uint8_t)((color >> 11) & 0x1F);
    uint8_t g6 = (uint8_t)((color >> 5)  & 0x3F);
    uint8_t b5 = (uint8_t)(color & 0x1F);

    uint8_t r8 = (uint8_t)((r5 << 3) | (r5 >> 2));
    uint8_t g8 = (uint8_t)((g6 << 2) | (g6 >> 4));
    uint8_t b8 = (uint8_t)((b5 << 3) | (b5 >> 2));

    for (int16_t yy = y; yy <= y2; yy++) {
      uint8_t *row = _fb + (((uint32_t)yy * _width + x) * 3UL);
      for (int16_t xx = x; xx <= x2; xx++) {
        *row++ = r8;
        *row++ = g8;
        *row++ = b8;
      }
    }
  }

private:
  uint8_t *_fb = nullptr;
  uint32_t _fb_size = 0;

  static inline uint32_t bitMask(uint8_t pin) {
    return (1UL << pin);
  }

  static inline void fastSetHigh(uint32_t mask) {
    REG_WRITE(GPIO_OUT_W1TS_REG, mask);
  }

  static inline void fastSetLow(uint32_t mask) {
    REG_WRITE(GPIO_OUT_W1TC_REG, mask);
  }

  inline void setPixelRGB888(int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t index = (((uint32_t)y * _width) + x) * 3UL;
    _fb[index + 0] = r;
    _fb[index + 1] = g;
    _fb[index + 2] = b;
  }

  inline void getPixelRGB888(int16_t x, int16_t y, uint8_t &r, uint8_t &g, uint8_t &b) const {
    uint32_t index = (((uint32_t)y * _width) + x) * 3UL;
    r = _fb[index + 0];
    g = _fb[index + 1];
    b = _fb[index + 2];
  }

  inline void setAddressFastRaw(uint8_t row) {
    uint32_t setMask = 0;
    if (row & 0x01) setMask |= bitMask(_A_PIN);
    if (row & 0x02) setMask |= bitMask(_B_PIN);
    if (row & 0x04) setMask |= bitMask(_C_PIN);
    if (row & 0x08) setMask |= bitMask(_D_PIN);
    if (row & 0x10) setMask |= bitMask(_E_PIN);

    fastSetLow(_ADDR_MASK);
    fastSetHigh(setMask);
  }

  inline void pulseLatch() {
    fastSetHigh(_LAT_MASK);
    fastSetLow(_LAT_MASK);
  }

  inline void pulseClock() {
    fastSetHigh(_CLK_MASK);
    fastSetLow(_CLK_MASK);
  }

  inline uint8_t scaleByBrightness(uint8_t v) const {
    return (uint8_t)(((uint16_t)v * (uint16_t)_brightness) / 255U);
  }

  static inline uint8_t to3Bit(uint8_t v) {
    return v >> 5;
  }


uint16_t planeHoldUs(uint8_t plane) const {
  static const uint16_t base[3] = {20, 40, 80};

  if (_brightness == 0) return 0;

  uint32_t hold = ((uint32_t)base[plane] * (uint32_t)_brightness) / 255UL;

  if (hold < 2) hold = 2;
  if (hold > 200) hold = 200;

  return (uint16_t)hold;
}


  void refreshSlice() {
    if (!_fb) return;
    if (_width != 64 || _height != 64) return;

    uint8_t plane = _scan_plane;
    uint8_t row   = _scan_row;
    uint16_t hold = planeHoldUs(plane);

    noInterrupts();

    // 1) 關輸出，避免切換期間漏光
    fastSetHigh(_OE_MASK);
    fastSetLow(_LAT_MASK);
    fastSetLow(_RGB_MASK_ALL);

    // 2) 先把當前 row 的 64 欄資料 shift 進去
    for (uint8_t col = 0; col < 64; col++) {
      uint8_t up_r, up_g, up_b;
      uint8_t dn_r, dn_g, dn_b;

      getPixelRGB888(col, row,      up_r, up_g, up_b);
      getPixelRGB888(col, row + 32, dn_r, dn_g, dn_b);

      //調亮度
uint8_t up_r3 = to3Bit(up_r);
uint8_t up_g3 = to3Bit(up_g);
uint8_t up_b3 = to3Bit(up_b);

uint8_t dn_r3 = to3Bit(dn_r);
uint8_t dn_g3 = to3Bit(dn_g);
uint8_t dn_b3 = to3Bit(dn_b);

      uint32_t setMask = 0;

      if ((up_r3 >> plane) & 0x01) setMask |= _R1_MASK;
      if ((up_g3 >> plane) & 0x01) setMask |= _G1_MASK;
      if ((up_b3 >> plane) & 0x01) setMask |= _B1_MASK;

      if ((dn_r3 >> plane) & 0x01) setMask |= _R2_MASK;
      if ((dn_g3 >> plane) & 0x01) setMask |= _G2_MASK;
      if ((dn_b3 >> plane) & 0x01) setMask |= _B2_MASK;

      fastSetLow(_RGB_MASK_ALL);
      fastSetHigh(setMask);
      pulseClock();
    }

    // 3) latch 前再清一次 RGB，避免殘光
    fastSetLow(_RGB_MASK_ALL);

    // 4) 鎖存
    pulseLatch();

    // 5) 設 row address
    setAddressFastRaw(row);

    // 6) 開輸出顯示
fastSetLow(_OE_MASK);
if (hold > 0) {
  delayMicroseconds(hold);
}
fastSetHigh(_OE_MASK);

    // 7) 再關輸出
    fastSetHigh(_OE_MASK);

    advanceScan();

    interrupts();
  }

  void advanceScan() {
    _scan_row++;

    if (_scan_row >= 32) {
      _scan_row = 0;

      if (_scan_plane == 0) {
        _scan_plane = PxMATRIX_COLOR_DEPTH - 1;
      } else {
        _scan_plane--;
      }
    }
  }

  uint16_t _width = 64;
  uint16_t _height = 64;

  uint8_t _LATCH_PIN = 255;
  uint8_t _OE_PIN    = 255;
  uint8_t _A_PIN     = 255;
  uint8_t _B_PIN     = 255;
  uint8_t _C_PIN     = 255;
  uint8_t _D_PIN     = 255;
  uint8_t _E_PIN     = 255;

  uint8_t _R1 = 255;
  uint8_t _G1 = 255;
  uint8_t _B1 = 255;
  uint8_t _R2 = 255;
  uint8_t _G2 = 255;
  uint8_t _B2 = 255;
  uint8_t _CLK_PIN = 255;

  uint8_t _brightness = 220;
  uint8_t _slices_per_call = 12;

  uint8_t _scan_plane = PxMATRIX_COLOR_DEPTH - 1;
  uint8_t _scan_row = 0;

  uint32_t _R1_MASK = 0;
  uint32_t _G1_MASK = 0;
  uint32_t _B1_MASK = 0;
  uint32_t _R2_MASK = 0;
  uint32_t _G2_MASK = 0;
  uint32_t _B2_MASK = 0;
  uint32_t _CLK_MASK = 0;
  uint32_t _LAT_MASK = 0;
  uint32_t _OE_MASK  = 0;
  uint32_t _ADDR_MASK = 0;
  uint32_t _RGB_MASK_ALL = 0;
};

#endif
