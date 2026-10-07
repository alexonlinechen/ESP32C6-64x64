/*********************************************************************
  PxMatrix_C6 PARLIO BETA1 - ESP32-C6 HUB75 64x64 hardware-assisted refresh

  Purpose:
  - Preserve the existing Adafruit_GFX / PxMATRIX drawing API used by C6pixel.
  - Keep FRONT/BACK packed 3-bit RGB frame buffers.
  - Move HUB75 scan timing away from Arduino loop() and CPU GPIO bit-banging.
  - PARLIO + GDMA continuously sends a complete HUB75 waveform.
  - A high-priority producer task keeps the PARLIO transaction queue filled.
  - Two DMA waveform buffers are used. A waveform is never modified until all
    transactions that referenced it have completed.

  ESP32-C6 / 64x64 / 1:32 scan only.

  BETA1 timing (same timing proven by C6_Driver_Benchmark_PARLIO_B):
    PARLIO clock:      4 MHz
    scan slot:         260 clocks = 65 us
    slices/frame:      32 rows * 3 PWM planes = 96
    full refresh:      ~160.26 Hz
    waveform buffer:   49,920 bytes
    double waveform:   99,840 bytes

  Notes:
  - display() now means "commit BACK frame". It no longer performs scanning.
  - wait_with_display()/yield()/Wi-Fi can run normally; refresh is independent.
  - Legacy cooperative-refresh tuning methods remain as compatibility no-ops.
*********************************************************************/

#ifndef _PxMATRIX_H
#define _PxMATRIX_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_idf_version.h"
#include "soc/soc_caps.h"
#include "driver/parlio_tx.h"

#if !defined(CONFIG_IDF_TARGET_ESP32C6)
#error "PxMatrix_C6 PARLIO backend is intended for ESP32-C6 only."
#endif

#ifndef SOC_PARLIO_TX_UNIT_MAX_DATA_WIDTH
#error "PARLIO TX is not available in this ESP-IDF/Arduino core."
#endif

#if SOC_PARLIO_TX_UNIT_MAX_DATA_WIDTH < 16
#error "PxMatrix_C6 PARLIO backend requires 16-bit PARLIO TX support."
#endif

#ifndef PxMATRIX_COLOR_DEPTH
#define PxMATRIX_COLOR_DEPTH 3
#endif

#if PxMATRIX_COLOR_DEPTH != 3
#error "PxMatrix_C6 PARLIO backend requires PxMATRIX_COLOR_DEPTH == 3"
#endif

class PxMATRIX : public Adafruit_GFX {
public:
  PxMATRIX(uint16_t width, uint16_t height,
           uint8_t LATCH, uint8_t OE,
           uint8_t A, uint8_t B, uint8_t C, uint8_t D, uint8_t E)
    : Adafruit_GFX(width, height),
      _width(width), _height(height),
      _LATCH_PIN(LATCH), _OE_PIN(OE),
      _A_PIN(A), _B_PIN(B), _C_PIN(C), _D_PIN(D), _E_PIN(E) {}

  ~PxMATRIX() {
    stopParlio();
    freeFrameBuffers();
  }

  // The first call (before begin) defines the six physical PARLIO color slots.
  // Later calls may permute the same six pins (C6pixel RGB-order setting). In
  // that case only logical bit mapping changes; PARLIO does not need restart.
  void setRGBPins(uint8_t r1, uint8_t g1, uint8_t b1,
                  uint8_t r2, uint8_t g2, uint8_t b2,
                  uint8_t clk) {
    if (!_started) {
      _R1 = r1; _G1 = g1; _B1 = b1;
      _R2 = r2; _G2 = g2; _B2 = b2;
      _CLK_PIN = clk;
      return;
    }

    if (clk != _CLK_PIN) {
      Serial.println(F("PARLIO: runtime CLK pin change is not supported; restart required."));
      return;
    }

    if (_frameMutex) xSemaphoreTake(_frameMutex, portMAX_DELAY);

    _R1 = r1; _G1 = g1; _B1 = b1;
    _R2 = r2; _G2 = g2; _B2 = b2;

    bool ok = rebuildLogicalColorMasks();
    if (ok) requestWaveUpdateLocked();

    if (_frameMutex) xSemaphoreGive(_frameMutex);

    if (!ok) {
      Serial.println(F("PARLIO: RGB-order change used pins outside initial color-pin set; ignored."));
    }
  }

  void begin() {
    if (_started) return;

    if (_width != 64 || _height != 64) {
      Serial.println(F("PARLIO: only 64x64 panels are supported by this backend."));
      return;
    }
    if (_CLK_PIN == 255 || _R1 == 255 || _G1 == 255 || _B1 == 255 ||
        _R2 == 255 || _G2 == 255 || _B2 == 255) {
      Serial.println(F("PARLIO: setRGBPins() must be called before begin()."));
      return;
    }

    _frameMutex = xSemaphoreCreateMutex();
    if (!_frameMutex) {
      Serial.println(F("PARLIO: failed to create frame mutex."));
      return;
    }

    _buf_size = (uint32_t)PxMATRIX_COLOR_DEPTH * 32UL * 64UL;
    _bufferA = new uint8_t[_buf_size];
    _bufferB = new uint8_t[_buf_size];
    if (!_bufferA || !_bufferB) {
      Serial.println(F("PARLIO: packed framebuffer allocation failed."));
      freeFrameBuffers();
      vSemaphoreDelete(_frameMutex);
      _frameMutex = nullptr;
      return;
    }

    _frontbuf = _bufferA;
    _backbuf  = _bufferB;
    memset(_frontbuf, 0, _buf_size);
    memset(_backbuf, 0, _buf_size);

    // Capture the physical pins assigned to the six fixed PARLIO color slots.
    _colorSlotPins[0] = _R1;
    _colorSlotPins[1] = _G1;
    _colorSlotPins[2] = _B1;
    _colorSlotPins[3] = _R2;
    _colorSlotPins[4] = _G2;
    _colorSlotPins[5] = _B2;
    if (!rebuildLogicalColorMasks()) {
      Serial.println(F("PARLIO: invalid initial RGB pin mapping."));
      return;
    }

    rebuildColorPlaneLUT();

    _wave[0] = (uint16_t *)heap_caps_aligned_alloc(
      32, WAVE_BYTES, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
    _wave[1] = (uint16_t *)heap_caps_aligned_alloc(
      32, WAVE_BYTES, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA | MALLOC_CAP_8BIT);

    if (!_wave[0] || !_wave[1]) {
      Serial.printf("PARLIO: DMA waveform allocation failed (need %u bytes x2).\n",
                    (unsigned)WAVE_BYTES);
      freeWaveBuffers();
      return;
    }

    memset(_wave[0], 0, WAVE_BYTES);
    memset(_wave[1], 0, WAVE_BYTES);

    // Build the initial black frame synchronously before PARLIO starts.
    buildWaveform(_wave[0], _frontbuf, _brightness);
    _activeWave = 0;
    _requestedVersion = 1;
    _builtVersion = 1;
    _lastSubmittedWave[0] = 0;
    _lastSubmittedWave[1] = 0;
    _txSubmitted = 0;
    _txCompleted = 0;

    if (!startParlio()) {
      Serial.println(F("PARLIO: startup failed; panel refresh not started."));
      return;
    }

    _backDirty = false;
    _backNeedsSync = false;
    _statSwaps = 0;
    _statSyncCopies = 0;
    _statsFrameBase = 0;
    _started = true;

    Serial.printf("PARLIO HUB75: started, waveform=%u bytes x2, target=%.2f Hz\n",
                  (unsigned)WAVE_BYTES,
                  (double)PARLIO_CLK_HZ / (double)FRAME_SAMPLES);
  }

  // Drawing-side clear: BACK only. display() commits it.
  void clearDisplay() {
    if (!_backbuf) return;
    _backNeedsSync = false;
    memset(_backbuf, 0, _buf_size);
    _backDirty = true;
  }

  void clearDisplay(bool) { clearDisplay(); }
  void flushDisplay() { clearDisplay(); }
  void showBuffer() { display(); }

  uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
  }

  void setBrightness(uint8_t brightness) {
    if (!_started || !_frameMutex) {
      _brightness = brightness;
      return;
    }
    xSemaphoreTake(_frameMutex, portMAX_DELAY);
    if (_brightness != brightness) {
      _brightness = brightness;
      requestWaveUpdateLocked();
    }
    xSemaphoreGive(_frameMutex);
  }

  // Commit a completed BACK frame. Hardware refresh continues independently.
  void display() {
    if (!_started || !_frontbuf || !_backbuf || !_backDirty) return;

    xSemaphoreTake(_frameMutex, portMAX_DELAY);

    uint8_t *tmp = _frontbuf;
    _frontbuf = _backbuf;
    _backbuf = tmp;

    _backNeedsSync = true;
    _backDirty = false;
    _statSwaps++;
    requestWaveUpdateLocked();

    xSemaphoreGive(_frameMutex);
  }

  void display(uint16_t) { display(); }

  // Compatibility API from V3.2. PARLIO refresh is always hardware paced.
  void setPacedRefresh(bool enabled) { _pacedRefreshCompat = enabled; }
  bool getPacedRefresh() const { return _pacedRefreshCompat; }
  void setTargetRefreshHz(uint16_t) {}
  uint16_t getTargetRefreshHz() const { return 160; }
  uint32_t getSlicePeriodUs() const { return 65; }
  void setScanMode(uint8_t mode) { _scanModeCompat = mode ? 1 : 0; }
  uint8_t getScanMode() const { return _scanModeCompat; }
  void setAutoRefresh(bool enabled) { _autoRefreshCompat = enabled; }
  bool getAutoRefresh() const { return _autoRefreshCompat; }
  void setAutoRefreshPolicy(uint16_t, uint8_t, uint32_t) {}
  void getAutoRefreshPolicy(uint16_t &pixelWritesPerBurst,
                            uint8_t &slicesPerBurst,
                            uint32_t &maxGapUs) const {
    pixelWritesPerBurst = 0;
    slicesPerBurst = 0;
    maxGapUs = 0;
  }

  void resetStats() {
    _statSwaps = 0;
    _statSyncCopies = 0;
    _statsFrameBase = _txCompleted;
  }

  uint32_t getAutoBurstCount() const { return _txCompleted - _statsFrameBase; }
  uint32_t getAutoSliceCount() const { return (_txCompleted - _statsFrameBase) * 96UL; }
  uint32_t getSwapCount() const { return _statSwaps; }
  uint32_t getSyncCopyCount() const { return _statSyncCopies; }
  uint32_t getMaxSliceGapUs() const { return 0; }

  uint32_t getParlioFramesDone() const { return _txCompleted; }
  int32_t getParlioLastError() const { return _lastTxError; }
  bool parlioReady() const { return _started && _tx != nullptr; }

  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
    if (!_backbuf) return;
    if (x < 0 || x >= _width || y < 0 || y >= _height) return;
    ensureBackSynced();

    uint16_t key = (uint16_t)(((color >> 13) & 0x07) << 6) |
                   (uint16_t)(((color >>  8) & 0x07) << 3) |
                   (uint16_t)(((color >>  2) & 0x07));
    setPixelPlanes(x, y, _colorPlaneLUT[key]);
    _backDirty = true;
  }

  void fillScreen(uint16_t color) override {
    if (!_backbuf) return;
    _backNeedsSync = false;

    uint16_t key = (uint16_t)(((color >> 13) & 0x07) << 6) |
                   (uint16_t)(((color >>  8) & 0x07) << 3) |
                   (uint16_t)(((color >>  2) & 0x07));
    uint32_t planes = _colorPlaneLUT[key];

    for (uint8_t plane = 0; plane < 3; ++plane) {
      uint8_t top = (uint8_t)((planes >> (plane * 8)) & 0x07);
      uint8_t both = (uint8_t)(top | (top << 3));
      memset(_backbuf + planeOffset(plane), both, 32UL * 64UL);
    }
    _backDirty = true;
  }

  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override {
    if (!_backbuf || w <= 0 || h <= 0) return;

    int16_t x2 = x + w - 1;
    int16_t y2 = y + h - 1;
    if (x >= _width || y >= _height || x2 < 0 || y2 < 0) return;
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

    for (int16_t yy = y; yy <= y2; ++yy) {
      for (int16_t xx = x; xx <= x2; ++xx) {
        setPixelPlanes(xx, yy, planes);
      }
    }
    _backDirty = true;
  }

private:
  // PARLIO data-bit layout. P_CLK is generated by the PARLIO clock output.
  static constexpr uint8_t IDX_B2  = 0;
  static constexpr uint8_t IDX_B1  = 1;
  static constexpr uint8_t IDX_OE  = 2;
  static constexpr uint8_t IDX_G2  = 3;
  static constexpr uint8_t IDX_G1  = 4;
  static constexpr uint8_t IDX_LAT = 5;
  static constexpr uint8_t IDX_R2  = 6;
  static constexpr uint8_t IDX_R1  = 7;
  static constexpr uint8_t IDX_A   = 8;
  static constexpr uint8_t IDX_B   = 9;
  static constexpr uint8_t IDX_C   = 10;
  static constexpr uint8_t IDX_D   = 11;
  static constexpr uint8_t IDX_E   = 12;

  static constexpr uint16_t BIT_OE  = (1U << IDX_OE);
  static constexpr uint16_t BIT_LAT = (1U << IDX_LAT);
  static constexpr uint16_t BIT_A   = (1U << IDX_A);
  static constexpr uint16_t BIT_B   = (1U << IDX_B);
  static constexpr uint16_t BIT_C   = (1U << IDX_C);
  static constexpr uint16_t BIT_D   = (1U << IDX_D);
  static constexpr uint16_t BIT_E   = (1U << IDX_E);

  static constexpr uint32_t PARLIO_CLK_HZ = 4000000UL;
  static constexpr uint16_t SLOT_CYCLES = 260;
  static constexpr uint16_t SHIFT_CYCLES = 64;
  static constexpr uint16_t LATCH_SETTLE_CYCLES = 2;
  static constexpr uint16_t FRAME_SLICES = 32 * 3;
  static constexpr uint32_t FRAME_SAMPLES = (uint32_t)FRAME_SLICES * SLOT_CYCLES;
  static constexpr size_t WAVE_BYTES = FRAME_SAMPLES * sizeof(uint16_t);
  static constexpr uint8_t TX_QUEUE_DEPTH = 4;

  uint16_t _width = 64;
  uint16_t _height = 64;

  uint8_t _LATCH_PIN = 255;
  uint8_t _OE_PIN = 255;
  uint8_t _A_PIN = 255;
  uint8_t _B_PIN = 255;
  uint8_t _C_PIN = 255;
  uint8_t _D_PIN = 255;
  uint8_t _E_PIN = 255;

  uint8_t _R1 = 255;
  uint8_t _G1 = 255;
  uint8_t _B1 = 255;
  uint8_t _R2 = 255;
  uint8_t _G2 = 255;
  uint8_t _B2 = 255;
  uint8_t _CLK_PIN = 255;

  // Six fixed physical PARLIO color slots captured at begin().
  uint8_t _colorSlotPins[6] = {255, 255, 255, 255, 255, 255};
  uint16_t _bitR1 = (1U << IDX_R1);
  uint16_t _bitG1 = (1U << IDX_G1);
  uint16_t _bitB1 = (1U << IDX_B1);
  uint16_t _bitR2 = (1U << IDX_R2);
  uint16_t _bitG2 = (1U << IDX_G2);
  uint16_t _bitB2 = (1U << IDX_B2);

  uint8_t *_bufferA = nullptr;
  uint8_t *_bufferB = nullptr;
  uint8_t *_frontbuf = nullptr;
  uint8_t *_backbuf = nullptr;
  uint32_t _buf_size = 0;

  uint32_t _colorPlaneLUT[512] = {0};

  volatile bool _backDirty = false;
  bool _backNeedsSync = false;
  uint8_t _brightness = 220;

  SemaphoreHandle_t _frameMutex = nullptr;

  parlio_tx_unit_handle_t _tx = nullptr;
  TaskHandle_t _refreshTask = nullptr;
  uint16_t *_wave[2] = {nullptr, nullptr};
  uint8_t _activeWave = 0;

  volatile uint32_t _txSubmitted = 0;
  volatile uint32_t _txCompleted = 0;
  volatile int32_t _lastTxError = ESP_OK;
  volatile uint32_t _lastSubmittedWave[2] = {0, 0};

  volatile uint32_t _requestedVersion = 0;
  volatile uint32_t _builtVersion = 0;

  volatile uint32_t _statSwaps = 0;
  volatile uint32_t _statSyncCopies = 0;
  volatile uint32_t _statsFrameBase = 0;

  bool _started = false;
  bool _pacedRefreshCompat = true;
  bool _autoRefreshCompat = true;
  uint8_t _scanModeCompat = 1;

  static inline uint32_t planeOffset(uint8_t plane) {
    return (uint32_t)plane * 32UL * 64UL;
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
    // FRONT is immutable between display() commits. Producer only reads it.
    memcpy(_backbuf, _frontbuf, _buf_size);
    _backNeedsSync = false;
    _statSyncCopies++;
  }

  void rebuildColorPlaneLUT() {
    for (uint16_t key = 0; key < 512; ++key) {
      uint8_t r3 = (uint8_t)((key >> 6) & 0x07);
      uint8_t g3 = (uint8_t)((key >> 3) & 0x07);
      uint8_t b3 = (uint8_t)( key       & 0x07);
      uint8_t p0 = (uint8_t)((r3 & 1) | ((g3 & 1) << 1) | ((b3 & 1) << 2));
      uint8_t p1 = (uint8_t)(((r3 >> 1) & 1) | (((g3 >> 1) & 1) << 1) | (((b3 >> 1) & 1) << 2));
      uint8_t p2 = (uint8_t)(((r3 >> 2) & 1) | (((g3 >> 2) & 1) << 1) | (((b3 >> 2) & 1) << 2));
      _colorPlaneLUT[key] = (uint32_t)p0 | ((uint32_t)p1 << 8) | ((uint32_t)p2 << 16);
    }
  }

  static constexpr uint8_t slotBitIndex(uint8_t slot) {
    return slot == 0 ? IDX_R1 :
           slot == 1 ? IDX_G1 :
           slot == 2 ? IDX_B1 :
           slot == 3 ? IDX_R2 :
           slot == 4 ? IDX_G2 : IDX_B2;
  }

  bool bitForPhysicalPin(uint8_t pin, uint16_t &bit) const {
    for (uint8_t slot = 0; slot < 6; ++slot) {
      if (_colorSlotPins[slot] == pin) {
        bit = (uint16_t)(1U << slotBitIndex(slot));
        return true;
      }
    }
    return false;
  }

  bool rebuildLogicalColorMasks() {
    uint16_t r1, g1, b1, r2, g2, b2;
    if (!bitForPhysicalPin(_R1, r1) || !bitForPhysicalPin(_G1, g1) ||
        !bitForPhysicalPin(_B1, b1) || !bitForPhysicalPin(_R2, r2) ||
        !bitForPhysicalPin(_G2, g2) || !bitForPhysicalPin(_B2, b2)) {
      return false;
    }
    _bitR1 = r1; _bitG1 = g1; _bitB1 = b1;
    _bitR2 = r2; _bitG2 = g2; _bitB2 = b2;
    return true;
  }

  inline uint16_t addressBits(uint8_t row) const {
    uint16_t v = 0;
    if (row & 0x01) v |= BIT_A;
    if (row & 0x02) v |= BIT_B;
    if (row & 0x04) v |= BIT_C;
    if (row & 0x08) v |= BIT_D;
    if (row & 0x10) v |= BIT_E;
    return v;
  }

  uint16_t holdCyclesForPlane(uint8_t plane, uint8_t brightness) const {
    static const uint16_t baseUs[3] = {20, 40, 80};
    uint32_t us = ((uint32_t)baseUs[plane] * (uint32_t)brightness) / 255UL;
    if (brightness == 0) us = 0;
    else if (us < 2) us = 2;

    uint32_t cycles = (us * PARLIO_CLK_HZ + 999999UL) / 1000000UL;
    const uint32_t maxHold = SLOT_CYCLES - SHIFT_CYCLES - LATCH_SETTLE_CYCLES - 1;
    if (cycles > maxHold) cycles = maxHold;
    return (uint16_t)cycles;
  }

  void buildWaveform(uint16_t *dst, const uint8_t *frame, uint8_t brightness) {
    uint32_t out = 0;

    for (int plane = 2; plane >= 0; --plane) {
      const uint16_t holdCycles = holdCyclesForPlane((uint8_t)plane, brightness);

      for (uint8_t row = 0; row < 32; ++row) {
        const uint16_t addr = addressBits(row);
        const uint32_t slotStart = out;
        const uint8_t *src = frame + planeOffset((uint8_t)plane) + (uint32_t)row * 64UL;

        for (uint8_t x = 0; x < 64; ++x) {
          uint8_t p = *src++;
          uint16_t v = (uint16_t)(addr | BIT_OE);

          if (p & 0x01) v |= _bitR1;
          if (p & 0x02) v |= _bitG1;
          if (p & 0x04) v |= _bitB1;
          if (p & 0x08) v |= _bitR2;
          if (p & 0x10) v |= _bitG2;
          if (p & 0x20) v |= _bitB2;
          if (x == 63) v |= BIT_LAT;

          dst[out++] = v;
        }

        for (uint16_t i = 0; i < LATCH_SETTLE_CYCLES; ++i) {
          dst[out++] = (uint16_t)(addr | BIT_OE);
        }

        for (uint16_t i = 0; i < holdCycles; ++i) {
          dst[out++] = addr;  // OE=0 -> visible
        }

        while ((out - slotStart) < SLOT_CYCLES) {
          dst[out++] = (uint16_t)(addr | BIT_OE);
        }
      }
    }
  }

  inline void requestWaveUpdateLocked() {
    ++_requestedVersion;
    if (_requestedVersion == 0) ++_requestedVersion; // avoid zero after wrap
  }

  bool tryBuildLatestWave() {
    uint32_t wanted = _requestedVersion;
    if (_builtVersion == wanted) return false;

    uint8_t candidate = (uint8_t)(1U - _activeWave);

    // Never touch a buffer that can still be referenced by queued/in-flight DMA.
    if (_txCompleted < _lastSubmittedWave[candidate]) return false;

    xSemaphoreTake(_frameMutex, portMAX_DELAY);

    // Re-check after obtaining the mutex because display()/brightness/RGB order
    // may have changed the requested version while we were waiting.
    wanted = _requestedVersion;
    if (_builtVersion == wanted) {
      xSemaphoreGive(_frameMutex);
      return false;
    }

    candidate = (uint8_t)(1U - _activeWave);
    if (_txCompleted < _lastSubmittedWave[candidate]) {
      xSemaphoreGive(_frameMutex);
      return false;
    }

    buildWaveform(_wave[candidate], _frontbuf, _brightness);
    _activeWave = candidate;
    _builtVersion = wanted;

    xSemaphoreGive(_frameMutex);
    return true;
  }

  static IRAM_ATTR bool txDoneCallback(parlio_tx_unit_handle_t,
                                       const parlio_tx_done_event_data_t *,
                                       void *user_data) {
    PxMATRIX *self = static_cast<PxMATRIX *>(user_data);
    self->_txCompleted++;
    return false;
  }

  static void refreshTaskTrampoline(void *arg) {
    static_cast<PxMATRIX *>(arg)->refreshTaskLoop();
  }

  void refreshTaskLoop() {
    parlio_transmit_config_t txConfig = {};
    txConfig.idle_value = BIT_OE;  // if queue ever drains, blank panel safely
    txConfig.flags.queue_nonblocking = false;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
    txConfig.flags.loop_transmission = false;
#endif

    for (;;) {
      // Build a pending frame while already queued DMA frames continue playing.
      tryBuildLatestWave();

      uint8_t waveIndex = _activeWave;
      esp_err_t err = parlio_tx_unit_transmit(
        _tx,
        _wave[waveIndex],
        WAVE_BYTES * 8UL,
        &txConfig
      );

      if (err == ESP_OK) {
        uint32_t seq = ++_txSubmitted;
        _lastSubmittedWave[waveIndex] = seq;
      } else {
        _lastTxError = err;
        vTaskDelay(pdMS_TO_TICKS(1));
      }
    }
  }

  bool startParlio() {
    parlio_tx_unit_config_t cfg = {};
    cfg.clk_src = PARLIO_CLK_SRC_DEFAULT;
    cfg.clk_in_gpio_num = (gpio_num_t)-1;
    cfg.input_clk_src_freq_hz = 0;
    cfg.output_clk_freq_hz = PARLIO_CLK_HZ;
    cfg.data_width = 16;

    for (size_t i = 0; i < PARLIO_TX_UNIT_MAX_DATA_WIDTH; ++i) {
      cfg.data_gpio_nums[i] = (gpio_num_t)-1;
    }

    // Fixed PARLIO color slots. Logical RGB order can later be remapped in
    // software without changing these GPIO routes.
    cfg.data_gpio_nums[IDX_R1] = (gpio_num_t)_colorSlotPins[0];
    cfg.data_gpio_nums[IDX_G1] = (gpio_num_t)_colorSlotPins[1];
    cfg.data_gpio_nums[IDX_B1] = (gpio_num_t)_colorSlotPins[2];
    cfg.data_gpio_nums[IDX_R2] = (gpio_num_t)_colorSlotPins[3];
    cfg.data_gpio_nums[IDX_G2] = (gpio_num_t)_colorSlotPins[4];
    cfg.data_gpio_nums[IDX_B2] = (gpio_num_t)_colorSlotPins[5];

    cfg.data_gpio_nums[IDX_OE]  = (gpio_num_t)_OE_PIN;
    cfg.data_gpio_nums[IDX_LAT] = (gpio_num_t)_LATCH_PIN;
    cfg.data_gpio_nums[IDX_A]   = (gpio_num_t)_A_PIN;
    cfg.data_gpio_nums[IDX_B]   = (gpio_num_t)_B_PIN;
    cfg.data_gpio_nums[IDX_C]   = (gpio_num_t)_C_PIN;
    cfg.data_gpio_nums[IDX_D]   = (gpio_num_t)_D_PIN;
    cfg.data_gpio_nums[IDX_E]   = (gpio_num_t)_E_PIN;

    cfg.clk_out_gpio_num = (gpio_num_t)_CLK_PIN;
    cfg.valid_gpio_num = (gpio_num_t)-1;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
    cfg.valid_start_delay = 0;
    cfg.valid_stop_delay = 0;
#endif
    cfg.trans_queue_depth = TX_QUEUE_DEPTH;
    cfg.max_transfer_size = WAVE_BYTES;
    cfg.dma_burst_size = 0;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
    cfg.shift_edge = PARLIO_SHIFT_EDGE_NEG;
#else
    cfg.sample_edge = PARLIO_SAMPLE_EDGE_NEG;
#endif
    cfg.bit_pack_order = PARLIO_BIT_PACK_ORDER_MSB;
    cfg.flags.clk_gate_en = false;
    cfg.flags.io_loop_back = false;
    cfg.flags.allow_pd = false;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
    cfg.flags.invert_valid_out = false;
#endif

    esp_err_t err = parlio_new_tx_unit(&cfg, &_tx);
    if (err != ESP_OK) {
      _lastTxError = err;
      Serial.printf("PARLIO: parlio_new_tx_unit failed: %s (%d)\n",
                    esp_err_to_name(err), (int)err);
      _tx = nullptr;
      return false;
    }

    parlio_tx_event_callbacks_t callbacks = {};
    callbacks.on_trans_done = txDoneCallback;
    err = parlio_tx_unit_register_event_callbacks(_tx, &callbacks, this);
    if (err != ESP_OK) {
      _lastTxError = err;
      Serial.printf("PARLIO: callback registration failed: %s (%d)\n",
                    esp_err_to_name(err), (int)err);
      return false;
    }

    err = parlio_tx_unit_enable(_tx);
    if (err != ESP_OK) {
      _lastTxError = err;
      Serial.printf("PARLIO: enable failed: %s (%d)\n",
                    esp_err_to_name(err), (int)err);
      return false;
    }

    BaseType_t ok = xTaskCreate(
      refreshTaskTrampoline,
      "hub75_parlio",
      4096,
      this,
      configMAX_PRIORITIES - 1,
      &_refreshTask
    );
    if (ok != pdPASS) {
      Serial.println(F("PARLIO: failed to create refresh producer task."));
      _refreshTask = nullptr;
      return false;
    }

    return true;
  }

  void stopParlio() {
    if (_refreshTask) {
      vTaskDelete(_refreshTask);
      _refreshTask = nullptr;
    }

    if (_tx) {
      parlio_tx_unit_disable(_tx);
      parlio_del_tx_unit(_tx);
      _tx = nullptr;
    }

    freeWaveBuffers();

    if (_frameMutex) {
      vSemaphoreDelete(_frameMutex);
      _frameMutex = nullptr;
    }

    _started = false;
  }

  void freeWaveBuffers() {
    if (_wave[0]) {
      free(_wave[0]);
      _wave[0] = nullptr;
    }
    if (_wave[1]) {
      free(_wave[1]);
      _wave[1] = nullptr;
    }
  }

  void freeFrameBuffers() {
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
};

#endif
