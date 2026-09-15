#ifndef GIFPLAYER_H
#define GIFPLAYER_H

#include <Arduino.h>
#include <FS.h>
#include <string.h>
#include "PxMatrix_C6.h"

// 你的面板 & GIF 尺寸
#define MATRIX_WIDTH  64
#define MATRIX_HEIGHT 50
#define FRAME_PIXELS  (MATRIX_WIDTH * MATRIX_HEIGHT)   // 3200

// === RAM 優化選項 ===
// 1) LZW bits：11 相容最多（不易破圖）；10 省 RAM 但少數 GIF 會解錯
#ifndef GIF_LZW_MAXBITS
#define GIF_LZW_MAXBITS 11
#endif

// 2) 是否支援 DISPOSAL_RESTORE（會用到一份備份 buffer，吃 RAM）
//    若你的 GIF 背景黑、沒有複雜 restore，可關掉省 RAM
#ifndef GIF_SUPPORT_DISPOSAL_RESTORE
#define GIF_SUPPORT_DISPOSAL_RESTORE 0
#endif

// === 你專案外部全域 ===
extern PxMATRIX display;
extern bool GifClock;

// ==== 避免與其他函式庫/核心的同名巨集衝突 ====
#ifdef ERROR_NONE
#undef ERROR_NONE
#endif
#ifdef ERROR_FILEOPEN
#undef ERROR_FILEOPEN
#endif
#ifdef ERROR_FILENOTGIF
#undef ERROR_FILENOTGIF
#endif
#ifdef ERROR_BADGIFFORMAT
#undef ERROR_BADGIFFORMAT
#endif
#ifdef ERROR_UNKNOWNCONTROLEXT
#undef ERROR_UNKNOWNCONTROLEXT
#endif
#ifdef ERROR_FINISHED
#undef ERROR_FINISHED
#endif

// === Error codes（保持 old 介面，讓 Gif.ino 不用改）===
#define ERROR_NONE              0
#define ERROR_FILEOPEN          1
#define ERROR_FILENOTGIF        2
#define ERROR_BADGIFFORMAT      3
#define ERROR_UNKNOWNCONTROLEXT 4
#define ERROR_FINISHED          5

// GIF flags
#define COLORTBLFLAG    0x80
#define INTERLACEFLAG   0x40
#define TRANSPARENTFLAG 0x01

#define NO_TRANSPARENT_INDEX -1

// Disposal methods
#define DISPOSAL_NONE       0
#define DISPOSAL_LEAVE      1
#define DISPOSAL_BACKGROUND 2
#define DISPOSAL_RESTORE    3

class GifPlayer {
private:
  // Header tags
  static const int GIFHDRSIZE = 6;
  const char* GIFHDRTAGNORM  = "GIF87a";
  const char* GIFHDRTAGNORM1 = "GIF89a";

  typedef struct {
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
  } _RGB;

  // Logical screen descriptor
  int lsdWidth;
  int lsdHeight;
  int lsdPackedField;
  int lsdAspectRatio;
  int lsdBackgroundIndex;

  // Image descriptor
  int tbiImageX;
  int tbiImageY;
  int tbiWidth;
  int tbiHeight;
  int tbiPackedBits;
  bool tbiInterlaced;

  // Frame control
  int frameDelay;
  bool frameDrawn;
  int transparentColorIndex;
  int prevBackgroundIndex;
  int prevDisposalMethod;
  int disposalMethod;
  int lzwCodeSize;
  bool keyFrame;
  int rectX, rectY, rectWidth, rectHeight;

  int colorCount;
  _RGB gifPalette[256];

  // 解碼/畫面緩衝
  uint8_t lzwImageData[FRAME_PIXELS];
  uint8_t imageData[FRAME_PIXELS];

#if GIF_SUPPORT_DISPOSAL_RESTORE
  uint8_t imageDataBU[FRAME_PIXELS];
#endif

  // GIF sub-block 最大 255
  char tempBuffer[256];

  File file;

  // ---- IO helpers ----
  int readByte() {
    return file.read();
  }

  int readWord() {
    int b0 = file.read();
    int b1 = file.read();
    if (b0 < 0 || b1 < 0) return -1;
    return (b1 << 8) | b0;
  }

  int readShort() {
    int b0 = file.read();
    int b1 = file.read();
    if (b0 < 0 || b1 < 0) return -1;
    return (b1 << 8) | b0;
  }

  void readString(char* str, int len) {
    file.readBytes(str, len);
  }

  // ---- Image buffer helpers ----
  void fillImageDataRect(int colorIndex, int x, int y, int width, int height) {
    for (int iy = y; iy < y + height; iy++) {
      for (int ix = x; ix < x + width; ix++) {
        if (ix < 0 || ix >= MATRIX_WIDTH || iy < 0 || iy >= MATRIX_HEIGHT) continue;
        imageData[iy * MATRIX_WIDTH + ix] = (uint8_t)colorIndex;
      }
    }
  }

  void copyImageDataRect(uint8_t* src, uint8_t* dst, int x, int y, int width, int height) {
    for (int iy = y; iy < y + height; iy++) {
      for (int ix = x; ix < x + width; ix++) {
        if (ix < 0 || ix >= MATRIX_WIDTH || iy < 0 || iy >= MATRIX_HEIGHT) continue;
        dst[iy * MATRIX_WIDTH + ix] = src[iy * MATRIX_WIDTH + ix];
      }
    }
  }

  void drawPixelToImageData(int x, int y, uint8_t colorIndex) {
    if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT) return;
    imageData[y * MATRIX_WIDTH + x] = colorIndex;
  }

  // 一次性把整幀送到面板，再刷新
  void flushToDisplayAndShow() {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
      const int screenY = GifClock ? (y + 14) : y;

      for (int x = 0; x < MATRIX_WIDTH; x++) {
        int offset = y * MATRIX_WIDTH + x;
        uint8_t idx = imageData[offset];
        _RGB rgb = gifPalette[idx];
        uint16_t color = display.color565(rgb.Red, rgb.Green, rgb.Blue);
        display.drawPixel(x, screenY, color);
      }

      yield();
    }

    // PxMatrix_C6 版本使用 display()，不是 showBuffer()
    display.display();
  }

  // ---- LZW ----
#define LZW_MAXBITS   GIF_LZW_MAXBITS
#define LZW_SIZTABLE  (1 << LZW_MAXBITS)

  unsigned int mask[17] = {
    0x0000, 0x0001, 0x0003, 0x0007,
    0x000F, 0x001F, 0x003F, 0x007F,
    0x00FF, 0x01FF, 0x03FF, 0x07FF,
    0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF,
    0xFFFF
  };

  uint8_t stack[LZW_SIZTABLE];
  uint8_t suffix[LZW_SIZTABLE];
  unsigned int prefix[LZW_SIZTABLE];

  int codeSize, clearCode, endCode, maxCodeSize, maxCode, firstCode, oldCode;
  int code, inCode;
  int stackPtr;
  int bits, datum;
  int blockSize;
  int blockPos;
  uint8_t block[256];

  void lzw_init(int codeSizeIn) {
    codeSize = codeSizeIn;
    clearCode = 1 << codeSize;
    endCode = clearCode + 1;
    maxCodeSize = clearCode << 1;
    maxCode = clearCode + 2;
    oldCode = -1;

    for (int i = 0; i < clearCode; i++) {
      prefix[i] = 0;
      suffix[i] = (uint8_t)i;
    }

    stackPtr = 0;
    bits = 0;
    datum = 0;
    blockSize = 0;
    blockPos = 0;
  }

  int lzw_get_code() {
    while (bits < codeSize + 1) {
      if (blockPos >= blockSize) {
        blockSize = readByte();
        if (blockSize <= 0) return -1;
        file.read(block, blockSize);
        blockPos = 0;
      }
      datum += ((int)block[blockPos]) << bits;
      bits += 8;
      blockPos++;
    }

    int out = datum & mask[codeSize + 1];
    datum >>= (codeSize + 1);
    bits -= (codeSize + 1);
    return out;
  }

  int lzw_decode(uint8_t* dest, int width, int height) {
    int count = 0;
    int max = width * height;

    while (count < max) {
      if (stackPtr == 0) {
        code = lzw_get_code();
        if (code == -1) return ERROR_BADGIFFORMAT;

        if (code == clearCode) {
          codeSize = lzwCodeSize;
          maxCodeSize = clearCode << 1;
          maxCode = clearCode + 2;
          oldCode = -1;
          continue;
        } else if (code == endCode) {
          break;
        }

        if (oldCode == -1) {
          dest[count++] = suffix[code];
          oldCode = code;
          firstCode = code;
          continue;
        }

        inCode = code;

        if (code >= maxCode) {
          stack[stackPtr++] = (uint8_t)firstCode;
          code = oldCode;
        }

        while (code >= clearCode) {
          stack[stackPtr++] = suffix[code];
          code = prefix[code];
        }

        firstCode = suffix[code];
        stack[stackPtr++] = (uint8_t)firstCode;

        if (maxCode < LZW_SIZTABLE) {
          prefix[maxCode] = (unsigned int)oldCode;
          suffix[maxCode] = (uint8_t)firstCode;
          maxCode++;

          if ((maxCode >= maxCodeSize) && (maxCodeSize < LZW_SIZTABLE)) {
            maxCodeSize <<= 1;
            codeSize++;
          }
        }

        oldCode = inCode;
      }

      stackPtr--;
      dest[count++] = stack[stackPtr];
    }

    return ERROR_NONE;
  }

  // 安全跳過 image data sub-block 到 terminator(0)
  void skipImageDataToTerminatorSafe() {
    while (true) {
      int v = readByte();
      if (v < 0) return;
      if (v == 0) return;

      if (v == 0x21 || v == 0x2C || v == 0x3B) {
        file.seek(file.position() - 1);
        return;
      }

      file.seek(file.position() + v);
    }
  }

public:
  GifPlayer() {
    prevDisposalMethod = DISPOSAL_NONE;
    disposalMethod = DISPOSAL_NONE;
    transparentColorIndex = NO_TRANSPARENT_INDEX;
    prevBackgroundIndex = 0;
    frameDelay = 0;
    keyFrame = true;
    frameDrawn = false;
    rectX = rectY = rectWidth = rectHeight = 0;
    colorCount = 0;

    memset(imageData, 0, sizeof(imageData));
    memset(lzwImageData, 0, sizeof(lzwImageData));
  }

  void setFile(File imageFile) {
    file = imageFile;
  }

  int getFrameDelay() const {
    return frameDelay;
  }

  bool wasFrameDrawn() const {
    return frameDrawn;
  }

  bool parseGifHeader() {
    char hdr[GIFHDRSIZE];
    readString(hdr, GIFHDRSIZE);

    if (strncmp(hdr, GIFHDRTAGNORM, GIFHDRSIZE) != 0 &&
        strncmp(hdr, GIFHDRTAGNORM1, GIFHDRSIZE) != 0) {
      return false;
    }

    // reset state
    prevDisposalMethod = DISPOSAL_NONE;
    disposalMethod = DISPOSAL_NONE;
    transparentColorIndex = NO_TRANSPARENT_INDEX;
    prevBackgroundIndex = 0;
    frameDelay = 0;
    keyFrame = true;
    rectX = rectY = rectWidth = rectHeight = 0;
    return true;
  }

  void parseLogicalScreenDescriptor() {
    lsdWidth = readWord();
    lsdHeight = readWord();
    lsdPackedField = readByte();
    lsdBackgroundIndex = readByte();
    lsdAspectRatio = readByte();
  }

  int parseGlobalColorTable() {
    bool gctFlag = (lsdPackedField & COLORTBLFLAG) != 0;
    int gctSize = (lsdPackedField & 0x07);
    if (!gctFlag) return ERROR_NONE;

    colorCount = 1 << (gctSize + 1);

    for (int i = 0; i < colorCount; i++) {
      gifPalette[i].Red   = (uint8_t)readByte();
      gifPalette[i].Green = (uint8_t)readByte();
      gifPalette[i].Blue  = (uint8_t)readByte();
    }

    return ERROR_NONE;
  }

  int drawFrame() {
    frameDrawn = false;
    int b = readByte();
    if (b == -1) return ERROR_FINISHED;

    // trailer
    if (b == 0x3B) return ERROR_FINISHED;

    // extension
    if (b == 0x21) {
      int label = readByte();

      // Graphic Control Extension
      if (label == 0xF9) {
        (void)readByte(); // block size
        int packed = readByte();
        int delay = readShort();
        int transIndex = readByte();
        (void)readByte(); // terminator

        disposalMethod = (packed >> 2) & 0x07;
        frameDelay = delay;
        transparentColorIndex = (packed & TRANSPARENTFLAG) ? transIndex : NO_TRANSPARENT_INDEX;
        return ERROR_NONE;
      }

      // other extensions: skip sub-blocks
      int blockSize2 = readByte();
      if (blockSize2 > 0) file.readBytes(tempBuffer, blockSize2);

      int sub = 0;
      do {
        sub = readByte();
        if (sub > 0) file.readBytes(tempBuffer, sub);
      } while (sub > 0);

      return ERROR_NONE;
    }

    // image separator
    if (b != 0x2C) return ERROR_BADGIFFORMAT;

    // image descriptor
    tbiImageX = readWord();
    tbiImageY = readWord();
    tbiWidth  = readWord();
    tbiHeight = readWord();
    tbiPackedBits = readByte();
    tbiInterlaced = (tbiPackedBits & INTERLACEFLAG) != 0;

    // local color table?
    bool lctFlag = (tbiPackedBits & COLORTBLFLAG) != 0;
    int lctSize = (tbiPackedBits & 0x07);

    if (lctFlag) {
      int localColors = 1 << (lctSize + 1);
      for (int i = 0; i < localColors; i++) {
        gifPalette[i].Red   = (uint8_t)readByte();
        gifPalette[i].Green = (uint8_t)readByte();
        gifPalette[i].Blue  = (uint8_t)readByte();
      }
      colorCount = localColors;
    }

    // dispose previous frame
    if (!keyFrame) {
      if (prevDisposalMethod == DISPOSAL_BACKGROUND) {
        fillImageDataRect(prevBackgroundIndex, rectX, rectY, rectWidth, rectHeight);
      } else if (prevDisposalMethod == DISPOSAL_RESTORE) {
#if GIF_SUPPORT_DISPOSAL_RESTORE
        copyImageDataRect(imageDataBU, imageData, rectX, rectY, rectWidth, rectHeight);
#else
        fillImageDataRect(prevBackgroundIndex, rectX, rectY, rectWidth, rectHeight);
#endif
      }
    }

    prevDisposalMethod = disposalMethod;

    if (disposalMethod != DISPOSAL_NONE) {
      rectX = tbiImageX;
      rectY = tbiImageY;
      rectWidth = tbiWidth;
      rectHeight = tbiHeight;

      if (disposalMethod == DISPOSAL_BACKGROUND) {
        prevBackgroundIndex = (transparentColorIndex != NO_TRANSPARENT_INDEX)
                                ? transparentColorIndex
                                : lsdBackgroundIndex;
      } else if (disposalMethod == DISPOSAL_RESTORE) {
#if GIF_SUPPORT_DISPOSAL_RESTORE
        copyImageDataRect(imageData, imageDataBU, rectX, rectY, rectWidth, rectHeight);
#endif
      }
    }

    // min LZW code size
    lzwCodeSize = readByte();

    // decode to lzwImageData
    lzw_init(lzwCodeSize);
    int r = lzw_decode(lzwImageData, tbiWidth, tbiHeight);
    if (r != ERROR_NONE) return r;

    // align stream safely
    skipImageDataToTerminatorSafe();

    // blit into imageData（透明色不覆蓋）
    if (!tbiInterlaced) {
      int idx = 0;
      for (int y = 0; y < tbiHeight; y++) {
        int drawY = tbiImageY + y;
        if (drawY < 0 || drawY >= MATRIX_HEIGHT) {
          idx += tbiWidth;
          continue;
        }

        for (int x = 0; x < tbiWidth; x++) {
          int drawX = tbiImageX + x;
          uint8_t c = lzwImageData[idx++];
          if (c == transparentColorIndex) continue;
          drawPixelToImageData(drawX, drawY, c);
        }
      }
    } else {
      int inc[]   = {8, 8, 4, 2};
      int start[] = {0, 4, 2, 1};

      int idx = 0;
      for (int p = 0; p < 4; p++) {
        for (int y = start[p]; y < tbiHeight; y += inc[p]) {
          int drawY = tbiImageY + y;
          if (drawY < 0 || drawY >= MATRIX_HEIGHT) {
            idx += tbiWidth;
            continue;
          }

          for (int x = 0; x < tbiWidth; x++) {
            int drawX = tbiImageX + x;
            uint8_t c = lzwImageData[idx++];
            if (c == transparentColorIndex) continue;
            drawPixelToImageData(drawX, drawY, c);
          }
        }
      }
    }

    keyFrame = false;

    // 整幀畫完後再刷新
    flushToDisplayAndShow();
    frameDrawn = true;
    return ERROR_NONE;
  }
};

#endif // GIFPLAYER_H
