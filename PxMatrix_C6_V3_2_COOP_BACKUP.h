/*********************************************************************
  PxMatrix_C6 V3.2 FINAL - DIRECT2 + double buffer + paced cooperative refresh

  Design goals:
  - Keep the normal Adafruit_GFX/PxMatrix drawing API.
  - Refresh always reads a stable FRONT packed bitplane buffer.
  - Drawing always writes a BACK packed bitplane buffer.
  - drawPixel()/fillRect() automatically inject short refresh bursts so
    a heavy renderer does not leave HUB75 dark for several milliseconds.
  - The next public display() call commits the completed BACK frame at a
    complete 96-slice PWM-cycle boundary, avoiding half-rendered frames.
  - After swap, FRONT is copied to BACK so partial-update themes preserve
    the same semantics as the original single framebuffer.

  V3.2 pacing policy:
    target refresh is paced to a fixed full-cycle rate (default 160 Hz)
    drawing checks the slice deadline frequently and services at most one
    due slice at a time. This avoids alternating between ~240 Hz idle scan
    and a much slower scan while rendering, which can create low-amplitude
    frame-rate brightness wobble.

  Buffer RAM:
    front: 3 * 32 * 64 = 6144 bytes
    back : 3 * 32 * 64 = 6144 bytes
    total                   12288 bytes
  This matches the original RGB888 single-buffer RAM size.
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
    freeBuffers();
  }

  void setRGBPins(uint8_t r1, uint8_t g1, uint8_t b1,
                  uint8_t r2, uint8_t g2, uint8_t b2,
                  uint8_t clk) {
    noInterrupts();

    _R1 = r1; _G1 = g1; _B1 = b1;
    _R2 = r2; _G2 = g2; _B2 = b2;
    _CLK_PIN = clk;

    _R1_MASK = bitMask(r1);
    _G1_MASK = bitMask(g1);
    _B1_MASK = bitMask(b1);
    _R2_MASK = bitMask(r2);
    _G2_MASK = bitMask(g2);
    _B2_MASK = bitMask(b2);
    _CLK_MASK = bitMask(clk);

    _RGB_MASK_ALL =
      _R1_MASK | _G1_MASK | _B1_MASK |
      _R2_MASK | _G2_MASK | _B2_MASK;
    _RGB_CLK_MASK = _RGB_MASK_ALL | _CLK_MASK;

    rebuildGpioLUT();
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

    freeBuffers();

    _buf_size = (uint32_t)PxMATRIX_COLOR_DEPTH * 32UL * 64UL;
    _bufferA = new uint8_t[_buf_size];
    _bufferB = new uint8_t[_buf_size];
    _frontbuf = _bufferA;
    _backbuf  = _bufferB;

    if (!_bufferA || !_bufferB) {
      freeBuffers();
      return;
    }

    memset(_frontbuf, 0, _buf_size);
    memset(_backbuf, 0, _buf_size);

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
    _RGB_CLK_MASK = _RGB_MASK_ALL | _CLK_MASK;

    rebuildGpioLUT();
    rebuildRowAddressLUT();
    rebuildColorPlaneLUT();
    rebuildPlaneHoldLUT();

    _backDirty = false;
    _swapPending = false;
    _backNeedsSync = false;
    _writesSinceService = 0;
    _serviceCheckCounter = 0;
    _lastAutoServiceUs = micros();
    _nextSliceDueUs = micros();
    _lastSliceStartUs = 0;
    resetStats();

    fastSetHigh(_OE_MASK);
    fastSetLow(_LAT_MASK | _CLK_MASK);
    fastSetLow(_ADDR_MASK);
    fastSetLow(_RGB_MASK_ALL);
  }

  // Drawing-side clear: clear BACK only, then commit on next public display().
  void clearDisplay() {
    if (!_backbuf) return;
    // Full overwrite: no need to synchronize old FRONT into BACK first.
    _backNeedsSync = false;
    memset(_backbuf, 0, _buf_size);
    markBackDirty();
  }

  void clearDisplay(bool) { clearDisplay(); }
  void flushDisplay() { clearDisplay(); }
  void showBuffer() { display(); }

  uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  }

  void setBrightness(uint8_t brightness) {
    _brightness = brightness;
    rebuildPlaneHoldLUT();
  }

  // Public display() is a frame-boundary hint. In paced mode it services
  // only slices whose deadline has arrived, keeping scan cadence nearly
  // constant during both idle and rendering periods.
  void display() {
    if (!_frontbuf || !_backbuf) return;
    if (_backDirty) _swapPending = true;

    if (_pacedRefresh) {
      servicePacedSlice();
      return;
    }

    displayRaw(_slices_per_call);
  }

  // Explicit-slice API remains available for raw benchmarks.
  void display(uint16_t slices) {
    if (_pacedRefresh) {
      if (_backDirty) _swapPending = true;
      servicePacedSlice();
      return;
    }
    displayRaw(slices);
  }

  void setPacedRefresh(bool enabled) {
    _pacedRefresh = enabled;
    _nextSliceDueUs = micros();
  }
  bool getPacedRefresh() const { return _pacedRefresh; }

  void setTargetRefreshHz(uint16_t hz) {
    if (hz < 80) hz = 80;
    if (hz > 220) hz = 220;
    _targetRefreshHz = hz;
    uint32_t denom = (uint32_t)hz * 96UL;
    _slicePeriodUs = (1000000UL + denom / 2UL) / denom;
    if (_slicePeriodUs < 40) _slicePeriodUs = 40;
    _nextSliceDueUs = micros();
  }
  uint16_t getTargetRefreshHz() const { return _targetRefreshHz; }
  uint32_t getSlicePeriodUs() const { return _slicePeriodUs; }

  // 0 = SAFE3 (3 MMIO writes/column), 1 = DIRECT2 (2 direct writes/column).
  void setScanMode(uint8_t mode) { _scanMode = mode ? 1 : 0; }
  uint8_t getScanMode() const { return _scanMode; }

  // Cooperative-refresh tuning. Defaults are deliberately conservative.
  void setAutoRefresh(bool enabled) { _autoRefresh = enabled; }
  bool getAutoRefresh() const { return _autoRefresh; }

  void setAutoRefreshPolicy(uint16_t pixelWritesPerBurst,
                            uint8_t slicesPerBurst,
                            uint32_t maxGapUs) {
    if (pixelWritesPerBurst < 16) pixelWritesPerBurst = 16;
    if (pixelWritesPerBurst > 2048) pixelWritesPerBurst = 2048;
    if (slicesPerBurst < 1) slicesPerBurst = 1;
    if (slicesPerBurst > 24) slicesPerBurst = 24;
    if (maxGapUs < 100) maxGapUs = 100;
    if (maxGapUs > 5000) maxGapUs = 5000;

    _autoPixelPeriod = pixelWritesPerBurst;
    _autoSlices = slicesPerBurst;
    _autoMaxGapUs = maxGapUs;
  }

  void getAutoRefreshPolicy(uint16_t &pixelWritesPerBurst,
                            uint8_t &slicesPerBurst,
                            uint32_t &maxGapUs) const {
    pixelWritesPerBurst = _autoPixelPeriod;
    slicesPerBurst = _autoSlices;
    maxGapUs = _autoMaxGapUs;
  }

  void resetStats() {
    _statAutoBursts = 0;
    _statAutoSlices = 0;
    _statSwaps = 0;
    _statSyncCopies = 0;
    _maxSliceGapUs = 0;
  }

  uint32_t getAutoBurstCount() const { return _statAutoBursts; }
  uint32_t getAutoSliceCount() const { return _statAutoSlices; }
  uint32_t getSwapCount() const { return _statSwaps; }
  uint32_t getSyncCopyCount() const { return _statSyncCopies; }
  uint32_t getMaxSliceGapUs() const { return _maxSliceGapUs; }

  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
    if (!_backbuf) return;
    if (x < 0 || x >= _width || y < 0 || y >= _height) return;
    ensureBackSynced();

    uint16_t key = (uint16_t)(((color >> 13) & 0x07) << 6) |
                   (uint16_t)(((color >>  8) & 0x07) << 3) |
                   (uint16_t)(((color >>  2) & 0x07));
    setPixelPlanes(x, y, _colorPlaneLUT[key]);
    markBackDirty();
    serviceAfterPixelWrite();
  }

  void fillScreen(uint16_t color) override {
    if (!_backbuf) return;
    // Full overwrite: old BACK contents do not need FRONT synchronization.
    _backNeedsSync = false;

    uint16_t key = (uint16_t)(((color >> 13) & 0x07) << 6) |
                   (uint16_t)(((color >>  8) & 0x07) << 3) |
                   (uint16_t)(((color >>  2) & 0x07));
    uint32_t planes = _colorPlaneLUT[key];

    for (uint8_t plane = 0; plane < 3; plane++) {
      uint8_t top = (uint8_t)((planes >> (plane * 8)) & 0x07);
      uint8_t both = (uint8_t)(top | (top << 3));
      memset(_backbuf + planeOffset(plane), both, 32UL * 64UL);
    }

    markBackDirty();
    // fillScreen is only ~tens of microseconds with packed buffers, so no
    // burst is forced here. The following drawing calls will service scan.
  }

  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override {
    if (!_backbuf) return;
    if (w <= 0 || h <= 0) return;

    int16_t x2 = x + w - 1;
    int16_t y2 = y + h - 1;

    if (x >= _width || y >= _height) return;
    if (x2 < 0 || y2 < 0) return;

    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x2 >= _width)  x2 = _width - 1;
    if (y2 >= _height) y2 = _height - 1;

    bool fullCover = (x == 0 && y == 0 && x2 == (_width - 1) && y2 == (_height - 1));
    if (fullCover) _backNeedsSync = false;
    else ensureBackSynced();

    uint16_t key = (uint16_t)(((color >> 13) & 0x07) << 6) |
                   (uint16_t)(((color >>  8) & 0x07) << 3) |
                   (uint16_t)(((color >>  2) & 0x07));
    uint32_t planes = _colorPlaneLUT[key];

    for (int16_t yy = y; yy <= y2; yy++) {
      for (int16_t xx = x; xx <= x2; xx++) {
        setPixelPlanes(xx, yy, planes);
        markBackDirty();
        serviceAfterPixelWrite();
      }
    }
  }

private:
  uint8_t *_bufferA = nullptr;
  uint8_t *_bufferB = nullptr;
  uint8_t *_frontbuf = nullptr;
  uint8_t *_backbuf = nullptr;
  uint32_t _buf_size = 0;

  uint32_t _gpioLUT[64] = {0};
  uint32_t _rowAddressLUT[32] = {0};
  uint32_t _colorPlaneLUT[512] = {0};
  uint16_t _planeHoldUs[3] = {0, 0, 0};

  uint8_t _scanMode = 1; // V3 defaults to measured-fast DIRECT2.
  uint32_t _RGB_CLK_MASK = 0;

  bool _backDirty = false;
  bool _swapPending = false;
  bool _backNeedsSync = false;

  bool _autoRefresh = true;
  bool _pacedRefresh = true;
  uint16_t _targetRefreshHz = 160;
  uint32_t _slicePeriodUs = 65;
  uint32_t _nextSliceDueUs = 0;
  uint32_t _lastSliceStartUs = 0;
  uint32_t _maxSliceGapUs = 0;
  uint16_t _autoPixelPeriod = 128;
  uint8_t _autoSlices = 2;
  uint32_t _autoMaxGapUs = 800;
  uint16_t _writesSinceService = 0;
  uint8_t _serviceCheckCounter = 0;
  uint32_t _lastAutoServiceUs = 0;

  uint32_t _statAutoBursts = 0;
  uint32_t _statAutoSlices = 0;
  uint32_t _statSwaps = 0;
  uint32_t _statSyncCopies = 0;

  static inline uint32_t bitMask(uint8_t pin) {
    return (1UL << pin);
  }

  static inline void fastSetHigh(uint32_t mask) {
    REG_WRITE(GPIO_OUT_W1TS_REG, mask);
  }

  static inline void fastSetLow(uint32_t mask) {
    REG_WRITE(GPIO_OUT_W1TC_REG, mask);
  }

  static inline uint32_t planeOffset(uint8_t plane) {
    return (uint32_t)plane * 32UL * 64UL;
  }

  static inline uint32_t scanIndex(uint8_t plane, uint8_t row, uint8_t col) {
    return planeOffset(plane) + (uint32_t)row * 64UL + col;
  }

  inline bool atFrameBoundary() const {
    return (_scan_row == 0 && _scan_plane == (PxMATRIX_COLOR_DEPTH - 1));
  }

  void freeBuffers() {
    if (_bufferA) {
      delete[] _bufferA;
      _bufferA = nullptr;
    }
    if (_bufferB) {
      delete[] _bufferB;
      _bufferB = nullptr;
    }
    _frontbuf = nullptr;
    _backbuf = nullptr;
  }

  inline void markBackDirty() {
    _backDirty = true;
  }

  inline void setPixelPlanes(int16_t x, int16_t y, uint32_t planes) {
    uint32_t idx = ((uint32_t)((uint8_t)y & 0x1F) << 6) + (uint8_t)x;
    uint8_t *q0 = _backbuf + idx;
    uint8_t *q1 = q0 + (32UL * 64UL);
    uint8_t *q2 = q1 + (32UL * 64UL);

    uint8_t p0 = (uint8_t)( planes        & 0x07);
    uint8_t p1 = (uint8_t)((planes >> 8)  & 0x07);
    uint8_t p2 = (uint8_t)((planes >> 16) & 0x07);

    if (y < 32) {
      *q0 = (uint8_t)((*q0 & 0xF8) | p0);
      *q1 = (uint8_t)((*q1 & 0xF8) | p1);
      *q2 = (uint8_t)((*q2 & 0xF8) | p2);
    } else {
      *q0 = (uint8_t)((*q0 & 0x07) | (p0 << 3));
      *q1 = (uint8_t)((*q1 & 0x07) | (p1 << 3));
      *q2 = (uint8_t)((*q2 & 0x07) | (p2 << 3));
    }
  }

  inline void ensureBackSynced() {
    if (!_backNeedsSync || !_frontbuf || !_backbuf) return;
    memcpy(_backbuf, _frontbuf, _buf_size);
    _backNeedsSync = false;
    _statSyncCopies++;
  }

  void displayRaw(uint16_t slices) {
    if (!_frontbuf || !_backbuf) return;
    if (slices == 0) slices = 1;
    if (_backDirty) _swapPending = true;

    uint16_t done = 0;
    uint16_t guard = 0;
    bool mustCommit = _swapPending;
    while (done < slices || mustCommit) {
      maybeSwapAtFrameBoundary();
      if (!_swapPending) mustCommit = false;
      refreshSlice();
      done++;
      if (++guard > (uint16_t)(slices + 96U)) break;
    }
    _lastAutoServiceUs = micros();
    _writesSinceService = 0;
  }

  inline bool servicePacedSlice() {
    uint32_t now = micros();
    if ((int32_t)(now - _nextSliceDueUs) < 0) return false;

    maybeSwapAtFrameBoundary();
    refreshSlice();
    _statAutoBursts++;
    _statAutoSlices++;

    uint32_t after = micros();
    int32_t late = (int32_t)(after - _nextSliceDueUs);
    if (late > (int32_t)(_slicePeriodUs * 2UL)) {
      // Do not burst-catch-up after a long foreground stall; restart cadence.
      _nextSliceDueUs = after + _slicePeriodUs;
    } else {
      _nextSliceDueUs += _slicePeriodUs;
    }
    _lastAutoServiceUs = after;
    _writesSinceService = 0;
    return true;
  }

  inline void serviceAfterPixelWrite() {
    if (!_autoRefresh || !_frontbuf) return;

    _writesSinceService++;
    _serviceCheckCounter++;

    if (_pacedRefresh) {
      // A deadline check every 8 writes is cheap enough and keeps the longest
      // foreground-only gap well below the visible 5-8 ms threshold.
      if ((_serviceCheckCounter & 0x07) == 0) {
        servicePacedSlice();
      }
      return;
    }

    bool dueByPixels = (_writesSinceService >= _autoPixelPeriod);
    bool dueByTime = false;
    if ((_serviceCheckCounter & 0x1F) == 0) {
      uint32_t now = micros();
      dueByTime = ((uint32_t)(now - _lastAutoServiceUs) >= _autoMaxGapUs);
    }
    if (!dueByPixels && !dueByTime) return;

    for (uint8_t i = 0; i < _autoSlices; i++) refreshSlice();
    _statAutoBursts++;
    _statAutoSlices += _autoSlices;
    _writesSinceService = 0;
    _lastAutoServiceUs = micros();
  }

  void maybeSwapAtFrameBoundary() {
    if (!_swapPending || !atFrameBoundary()) return;

    uint8_t *tmp = _frontbuf;
    _frontbuf = _backbuf;
    _backbuf = tmp;

    // Defer FRONT->BACK synchronization. Full-frame themes typically call
    // fillScreen() next, in which case the copy is skipped entirely. Partial
    // update themes trigger the copy lazily on their first drawPixel/fillRect.
    _backNeedsSync = true;

    _backDirty = false;
    _swapPending = false;
    _statSwaps++;
  }

  void rebuildColorPlaneLUT() {
    for (uint16_t key = 0; key < 512; key++) {
      uint8_t r3 = (uint8_t)((key >> 6) & 0x07);
      uint8_t g3 = (uint8_t)((key >> 3) & 0x07);
      uint8_t b3 = (uint8_t)( key       & 0x07);
      uint8_t p0 = (uint8_t)((r3 & 1) | ((g3 & 1) << 1) | ((b3 & 1) << 2));
      uint8_t p1 = (uint8_t)(((r3 >> 1) & 1) | (((g3 >> 1) & 1) << 1) | (((b3 >> 1) & 1) << 2));
      uint8_t p2 = (uint8_t)(((r3 >> 2) & 1) | (((g3 >> 2) & 1) << 1) | (((b3 >> 2) & 1) << 2));
      _colorPlaneLUT[key] = (uint32_t)p0 | ((uint32_t)p1 << 8) | ((uint32_t)p2 << 16);
    }
  }

  void rebuildGpioLUT() {
    for (uint8_t v = 0; v < 64; v++) {
      uint32_t mask = 0;
      if (v & 0x01) mask |= _R1_MASK;
      if (v & 0x02) mask |= _G1_MASK;
      if (v & 0x04) mask |= _B1_MASK;
      if (v & 0x08) mask |= _R2_MASK;
      if (v & 0x10) mask |= _G2_MASK;
      if (v & 0x20) mask |= _B2_MASK;
      _gpioLUT[v] = mask;
    }
  }

  void rebuildRowAddressLUT() {
    for (uint8_t row = 0; row < 32; row++) {
      uint32_t mask = 0;
      if (row & 0x01) mask |= bitMask(_A_PIN);
      if (row & 0x02) mask |= bitMask(_B_PIN);
      if (row & 0x04) mask |= bitMask(_C_PIN);
      if (row & 0x08) mask |= bitMask(_D_PIN);
      if (row & 0x10) mask |= bitMask(_E_PIN);
      _rowAddressLUT[row] = mask;
    }
  }

  void rebuildPlaneHoldLUT() {
    static const uint16_t base[3] = {20, 40, 80};

    for (uint8_t plane = 0; plane < 3; plane++) {
      if (_brightness == 0) {
        _planeHoldUs[plane] = 0;
        continue;
      }

      uint32_t hold = ((uint32_t)base[plane] * (uint32_t)_brightness) / 255UL;
      if (hold < 2) hold = 2;
      if (hold > 200) hold = 200;
      _planeHoldUs[plane] = (uint16_t)hold;
    }
  }

  inline void setAddressFastRaw(uint8_t row) {
    fastSetLow(_ADDR_MASK);
    fastSetHigh(_rowAddressLUT[row]);
  }

  inline void pulseLatch() {
    fastSetHigh(_LAT_MASK);
    fastSetLow(_LAT_MASK);
  }

  void refreshSlice() {
    if (!_frontbuf) return;
    if (_width != 64 || _height != 64) return;

    uint32_t sliceStart = micros();
    if (_lastSliceStartUs != 0) {
      uint32_t gap = (uint32_t)(sliceStart - _lastSliceStartUs);
      if (gap > _maxSliceGapUs) _maxSliceGapUs = gap;
    }
    _lastSliceStartUs = sliceStart;

    uint8_t plane = _scan_plane;
    uint8_t row   = _scan_row;
    uint16_t hold = _planeHoldUs[plane];
    const uint8_t *src = _frontbuf + scanIndex(plane, row, 0);
    const uint32_t *lut = _gpioLUT;

    noInterrupts();

    fastSetHigh(_OE_MASK);
    fastSetLow(_LAT_MASK);
    fastSetLow(_RGB_CLK_MASK);

    if (_scanMode == 0) {
      #define SHIFT_SAFE_ONE() do { \
        uint32_t d = lut[*src++]; \
        fastSetHigh(d); \
        fastSetHigh(_CLK_MASK); \
        fastSetLow(_RGB_CLK_MASK); \
      } while (0)
      for (uint8_t block = 0; block < 8; block++) {
        SHIFT_SAFE_ONE(); SHIFT_SAFE_ONE(); SHIFT_SAFE_ONE(); SHIFT_SAFE_ONE();
        SHIFT_SAFE_ONE(); SHIFT_SAFE_ONE(); SHIFT_SAFE_ONE(); SHIFT_SAFE_ONE();
      }
      #undef SHIFT_SAFE_ONE
    } else {
      uint32_t base = REG_READ(GPIO_OUT_REG) & ~_RGB_CLK_MASK;
      #define SHIFT_DIRECT_ONE() do { \
        uint32_t out = base | lut[*src++]; \
        REG_WRITE(GPIO_OUT_REG, out); \
        REG_WRITE(GPIO_OUT_REG, out | _CLK_MASK); \
      } while (0)
      for (uint8_t block = 0; block < 8; block++) {
        SHIFT_DIRECT_ONE(); SHIFT_DIRECT_ONE(); SHIFT_DIRECT_ONE(); SHIFT_DIRECT_ONE();
        SHIFT_DIRECT_ONE(); SHIFT_DIRECT_ONE(); SHIFT_DIRECT_ONE(); SHIFT_DIRECT_ONE();
      }
      #undef SHIFT_DIRECT_ONE
      REG_WRITE(GPIO_OUT_REG, base);
    }

    pulseLatch();
    setAddressFastRaw(row);

    fastSetLow(_OE_MASK);
    if (hold > 0) delayMicroseconds(hold);
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
