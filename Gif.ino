#include "config.h"
#include "GifPlayer.h"

//GifPlayer gifPlayer;

void playGif() {
  String fileName = GIFname;

  if (!fileName.startsWith("/")) {
    if (!fileName.startsWith("gifs/")) {
      fileName = "/gifs/" + fileName;
    } else {
      fileName = "/" + fileName;
    }
  }

  Serial.print("Attempting to play: ");
  Serial.println(fileName);

  File imageFile = LittleFS.open(fileName, "r");
  if (!imageFile) {
    Serial.println("Critical Error: File NOT found in LittleFS!");
    return;
  }

  for (int c = 0; c < gifcount; c++) {
    imageFile.seek(0);
    gifPlayer.setFile(imageFile);

    if (!gifPlayer.parseGifHeader()) {
      Serial.println("Error: Not a valid GIF header");
      break;
    }

    gifPlayer.parseLogicalScreenDescriptor();

    int rc = gifPlayer.parseGlobalColorTable();
    if (rc != ERROR_NONE) {
      Serial.print("Error: parseGlobalColorTable failed, rc=");
      Serial.println(rc);
      break;
    }

    int result = ERROR_NONE;
    int frameCount = 0;

    while (true) {
      result = gifPlayer.drawFrame();

      if (result == ERROR_FINISHED) {
        break;
      }

      if (result != ERROR_NONE) {
        Serial.print("GIF drawFrame error: ");
        Serial.println(result);
        break;
      }

// drawFrame() 有些時候只是解析 GIF metadata，並沒有真正畫圖
if (!gifPlayer.wasFrameDrawn()) {
  continue;
}

frameCount++;

// WEB 的 gifdelay 直接就是毫秒
int frameDelay = gifdelay;

// 防止設定異常
if (frameDelay < 10) {
  frameDelay = 10;
}

if (frameDelay > 200) {
  frameDelay = 200;
}

wait_with_display(frameDelay);

      if (frameCount > 1000) {
        Serial.println("Safety break: too many frames");
        break;
      }
    }

    Serial.print("Played frames: ");
    Serial.println(frameCount);
  }

  imageFile.close();
}

void randomGif() {
  if (random_gif_no > 0) {
    int r = random(1, random_gif_no + 1);
    GIFname = String(r) + ".gif";
    playGif();
  }
}
