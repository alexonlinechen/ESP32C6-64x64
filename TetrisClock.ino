#include "TetrisClockCore.h"

// 建立俄羅斯方塊渲染器實例，直接連結到你的 PxMATRIX display
static TetrisClockRenderer tetrisRenderer(display);

static uint8_t tetrisLastD[4] = {255, 255, 255, 255};
static uint32_t tetrisLastFrameMs = 0;

void TetrisMode() {
  // 1. 初始化檢查：使用你現有的 ModefirstRun 變數 
  if (ModefirstRun) {
    display.fillScreen(0);
    tetrisRenderer.reset();
    tetrisRenderer.scale = 2; // 64x64 建議使用 scale 2
    for(int i=0; i<4; i++) tetrisLastD[i] = 255;
    ModefirstRun = false; 
  }


  uint8_t d[4] = { (uint8_t)(H/10), (uint8_t)(H%10), (uint8_t)(M/10), (uint8_t)(M%10) };

  // 3. 檢查分鐘是否變動，若變動則觸發方塊重新掉落
  bool timeChanged = false;
  for(int i=0; i<4; i++) {
    if(d[i] != tetrisLastD[i]) timeChanged = true;
  }

  if (timeChanged) {
    // 呼叫 .h 內的設定數字功能
    tetrisRenderer.setDigits(d[0], d[1], d[2], d[3], false);
    for(int i=0; i<4; i++) tetrisLastD[i] = d[i];
  }

  // 4. 控制動畫幀率 (例如 70ms 更新一次)
  uint32_t now = millis();
  if (now - tetrisLastFrameMs >= 70) {
    tetrisLastFrameMs = now;
    // 在 64x64 螢幕上，將時鐘放在中間位置 (X=2, Y=45)
    tetrisRenderer.drawFrame(2, 40, true); 
  }
}
