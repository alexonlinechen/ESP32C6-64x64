#include "Config.h"

#if FEATURE_HOME_TRASH

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>
#include "HomeTrashData.h"

// ============================================================================
// HomeTrash - 住家專用垃圾車資訊模組（正式版 / 全部資料固化於 Flash）
//
// 保留：
// - 高雄市官方垃圾車 API
// - 啟用/停用、時間排程、星期三排除
// - 指定車號搜尋
// - 排程期間暫時 Override 畫面，結束後回原本主模式
// - 地址比對與內建中文點陣顯示
// - 獨立 /trash 設定頁（HTML 內嵌於 HomeTrashData.h）
//
// 已移除：
// - LittleFS 的 trash.bin / trash_index.json / trash.html
// - 測試 JSON URL
// - 測試模式
// - 動態載入/重新載入地址庫
// ============================================================================

static const char* HOME_TRASH_OFFICIAL_API =
  "https://api.kcg.gov.tw/api/service/get/aaf4ce4b-4ca8-43de-bfaf-6dc97e89cac0";
  //"http://220.133.230.117:8080/trashcar/test.json";
  
static const char* HOME_TRASH_PREF_NS = "homeTrash";

struct HomeTrashSettings {
  bool enabled = false;
  bool skipWednesday = true;
  uint8_t startHour = 18;
  uint8_t startMinute = 25;
  uint8_t endHour = 19;
  uint8_t endMinute = 0;
  uint16_t updateSeconds = 20;
  String carId = "KEM-0621";
};

static HomeTrashSettings homeTrashSettings;
static Preferences homeTrashPrefs;

static bool homeTrashActiveState = false;
static bool homeTrashWasActive = false;
static bool homeTrashForceFetch = false;

static volatile bool homeTrashFetchInProgress = false;
static volatile bool homeTrashFetchResultReady = false;
static volatile bool homeTrashFetchResultSuccess = false;
static volatile int homeTrashPendingHttpCode = 0;
static char homeTrashPendingLocation[192] = {0};
static char homeTrashPendingError[160] = {0};

static unsigned long homeTrashLastFetchStartMs = 0;
static unsigned long homeTrashLastSuccessMs = 0;
static int homeTrashLastHttpCode = 0;
static String homeTrashLastError = "尚未抓取";
static String homeTrashApiLocation = "";
static int16_t homeTrashLocationIndex = -1;
static bool homeTrashBitmapReady = false;

static String homeTrashJsonEscape(const String &s) {
  String out;
  out.reserve(s.length() + 16);
  for (size_t i = 0; i < s.length(); i++) {
    char c = s.charAt(i);
    switch (c) {
      case '\\': out += "\\\\"; break;
      case '"':  out += "\\\""; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default: out += c; break;
    }
  }
  return out;
}

static String homeTrashNormalizeLocation(String s) {
  s.replace("\r", "");
  s.replace("\n", "");
  s.trim();
  while (s.indexOf("  ") >= 0) s.replace("  ", " ");
  return s;
}

static int16_t homeTrashFindLocation(const String &location) {
  for (uint8_t i = 0; i < HOME_TRASH_INDEX_COUNT; i++) {
    if (location == HOME_TRASH_INDEX[i].apiText) {
      return (int16_t)i;
    }
  }
  return -1;
}

static bool homeTrashValidateBitmap(int16_t index) {
  if (index < 0 || index >= HOME_TRASH_INDEX_COUNT) return false;

  const HomeTrashDataEntry &idx = HOME_TRASH_INDEX[index];
  if (idx.size == 0 || idx.width == 0 || idx.height != 15) return false;
  if ((idx.size % 30) != 0) return false;
  if ((uint32_t)idx.offset + idx.size > HOME_TRASH_BITMAP_DATA_SIZE) return false;

  uint16_t expectedWidth = (idx.size / 30) * 16;
  return idx.width == expectedWidth;
}

static void homeTrashApplyLocation(const String &rawLocation) {
  String location = homeTrashNormalizeLocation(rawLocation);
  int16_t index = homeTrashFindLocation(location);

  homeTrashApiLocation = location;
  homeTrashLocationIndex = index;
  homeTrashBitmapReady = homeTrashValidateBitmap(index);

  if (index < 0) {
    Serial.print(F("[HomeTrash] Unknown location: "));
    Serial.println(location);
    return;
  }

  Serial.print(F("[HomeTrash] matched: "));
  Serial.print(location);
  Serial.print(F(" -> "));
  Serial.println(HOME_TRASH_INDEX[index].displayText);

  if (!homeTrashBitmapReady) {
    homeTrashLastError = "內建點陣資料索引錯誤";
    Serial.println(F("[HomeTrash] embedded bitmap invalid"));
  }
}

static bool homeTrashIsScheduledNow() {
  if (!homeTrashSettings.enabled) return false;

  // D 沿用原專案星期值：Sun=0, Mon=1, Tue=2, Wed=3 ...
  if (homeTrashSettings.skipWednesday && D == 3) return false;

  int nowMinute = H * 60 + M;
  int startMinute = homeTrashSettings.startHour * 60 + homeTrashSettings.startMinute;
  int endMinute = homeTrashSettings.endHour * 60 + homeTrashSettings.endMinute;

  if (startMinute == endMinute) return true;
  if (startMinute < endMinute) {
    return nowMinute >= startMinute && nowMinute < endMinute;
  }
  return nowMinute >= startMinute || nowMinute < endMinute;
}

static void homeTrashSetPendingError(const String &message, int httpCode = 0) {
  homeTrashPendingHttpCode = httpCode;
  strlcpy(homeTrashPendingError, message.c_str(), sizeof(homeTrashPendingError));
  homeTrashPendingLocation[0] = '\0';
  homeTrashFetchResultSuccess = false;
}

static bool homeTrashParseJson(Stream &stream, String &locationOut, String &errorOut) {
  // 官方 JSON 很大，因此不要等待整份 data[] 解析完成。
  // 先定位到 data 陣列，再逐筆解析物件；找到指定車號立即返回。
  // 這樣即使官方回傳數百筆資料，也只保留目前正在看的那一筆。

  stream.setTimeout(30000);

  if (!stream.find("\"data\"")) {
    errorOut = "JSON 中找不到 data";
    return false;
  }

  if (!stream.find("[")) {
    errorOut = "JSON 中找不到 data[]";
    return false;
  }

  StaticJsonDocument<96> filter;
  filter["car"] = true;
  filter["location"] = true;

  uint16_t scanned = 0;

  while (true) {
    // 跳過陣列元素間的空白與逗號，但保留 '{' 給 ArduinoJson 解析。
    int c = stream.peek();

    while (c == ' ' || c == '\r' || c == '\n' || c == '\t' || c == ',') {
      stream.read();
      c = stream.peek();
    }

    if (c < 0) {
      errorOut = String("data[] 傳輸中斷，已掃描 ") + scanned + " 筆";
      return false;
    }

    if (c == ']') {
      errorOut = String("找不到車號: ") + homeTrashSettings.carId;
      return false;
    }

    // 若碰到非物件內容，略過到下一個可能的物件或陣列結尾。
    if (c != '{') {
      stream.read();
      continue;
    }

    StaticJsonDocument<256> item;
    DeserializationError error = deserializeJson(
      item,
      stream,
      DeserializationOption::Filter(filter)
    );

    if (error) {
      errorOut = String("JSON item parse failed: ") + error.c_str() +
                 String(" (scanned=") + scanned + ")";
      return false;
    }

    scanned++;

    const char* car = item["car"] | "";
    if (homeTrashSettings.carId == car) {
      const char* location = item["location"] | "";

      if (strlen(location) == 0) {
        errorOut = "找到車號但 location 為空";
        return false;
      }

      locationOut = location;

      Serial.print(F("[HomeTrash] found after scanning "));
      Serial.print(scanned);
      Serial.println(F(" items"));

      return true;
    }
  }
}

static bool homeTrashFetchBlocking(
  String &locationOut,
  int &httpCodeOut,
  String &errorOut)
{
  if (WiFi.status() != WL_CONNECTED) {
    errorOut = "Wi-Fi 未連線";
    return false;
  }

  String url = HOME_TRASH_OFFICIAL_API;
  url.trim();

  HTTPClient http;

  http.setConnectTimeout(5000);
  http.setTimeout(30000);
  http.useHTTP10(true);

  bool beginOK = false;

  WiFiClient plainClient;
  WiFiClientSecure secureClient;

  // ==========================================
  // HTTPS
  // ==========================================
  if (url.startsWith("https://")) {

    secureClient.setInsecure();

    beginOK = http.begin(
      secureClient,
      url
    );
  }

  // ==========================================
  // HTTP
  // ==========================================
  else if (url.startsWith("http://")) {

    beginOK = http.begin(
      plainClient,
      url
    );
  }

  else {
    errorOut = "URL 必須是 http:// 或 https://";
    return false;
  }


  if (!beginOK) {
    errorOut = "http.begin failed";
    return false;
  }


  http.addHeader(
    "Accept",
    "application/json"
  );

  http.addHeader(
    "Accept-Encoding",
    "identity"
  );

  http.addHeader(
    "User-Agent",
    "C6Pixel-HomeTrash/2.2-stream"
  );


  int code = http.GET();

  httpCodeOut = code;


  // 額外印出 ESP32 HTTP 錯誤文字
  if (code <= 0) {

    errorOut =
      String("HTTP error: ") +
      code +
      " " +
      HTTPClient::errorToString(code);

    http.end();

    return false;
  }


  if (code != HTTP_CODE_OK) {

    errorOut =
      String("HTTP error: ") +
      code;

    http.end();

    return false;
  }


  String parseError;

  bool ok = homeTrashParseJson(
    http.getStream(),
    locationOut,
    parseError
  );


  http.end();


  if (!ok) {
    errorOut = parseError;
    return false;
  }


  return true;
}

static void homeTrashFetchTask(void *parameter) {
  (void)parameter;

  String location;
  String error;
  int httpCode = 0;

  Serial.print(F("[HomeTrash] GET "));
  Serial.println(HOME_TRASH_OFFICIAL_API);

  bool ok = homeTrashFetchBlocking(location, httpCode, error);

  homeTrashPendingHttpCode = httpCode;
  if (ok) {
    strlcpy(homeTrashPendingLocation, location.c_str(), sizeof(homeTrashPendingLocation));
    homeTrashPendingError[0] = '\0';
    homeTrashFetchResultSuccess = true;
  } else {
    homeTrashSetPendingError(error, httpCode);
  }

  homeTrashFetchResultReady = true;
  homeTrashFetchInProgress = false;
  vTaskDelete(nullptr);
}

static void homeTrashStartFetch() {
  if (homeTrashFetchInProgress) return;

  homeTrashFetchInProgress = true;
  homeTrashLastFetchStartMs = millis();

  BaseType_t created = xTaskCreate(
    homeTrashFetchTask,
    "HomeTrashHTTP",
    8192,
    nullptr,
    1,
    nullptr
  );

  if (created != pdPASS) {
    homeTrashFetchInProgress = false;
    homeTrashLastError = "無法建立 HomeTrash HTTP task";
    Serial.println(F("[HomeTrash] xTaskCreate failed"));
  }
}

static void homeTrashConsumeFetchResult() {
  if (!homeTrashFetchResultReady) return;

  homeTrashFetchResultReady = false;
  homeTrashLastHttpCode = homeTrashPendingHttpCode;

  if (homeTrashFetchResultSuccess) {
    homeTrashLastError = "";
    homeTrashLastSuccessMs = millis();
    homeTrashApplyLocation(String(homeTrashPendingLocation));
  } else {
    homeTrashLastError = String(homeTrashPendingError);
    Serial.print(F("[HomeTrash] fetch failed: "));
    Serial.println(homeTrashLastError);
    // 失敗時保留上一個成功位置。
  }
}

static void homeTrashLoadSettings() {
  homeTrashPrefs.begin(HOME_TRASH_PREF_NS, true);
  homeTrashSettings.enabled = homeTrashPrefs.getBool("enabled", false);
  homeTrashSettings.skipWednesday = homeTrashPrefs.getBool("skipWed", true);
  homeTrashSettings.startHour = homeTrashPrefs.getUChar("startH", 18);
  homeTrashSettings.startMinute = homeTrashPrefs.getUChar("startM", 25);
  homeTrashSettings.endHour = homeTrashPrefs.getUChar("endH", 19);
  homeTrashSettings.endMinute = homeTrashPrefs.getUChar("endM", 0);
  homeTrashSettings.updateSeconds = homeTrashPrefs.getUShort("interval", 20);
  homeTrashSettings.carId = homeTrashPrefs.getString("carId", "KEM-0621");
  homeTrashPrefs.end();

  if (homeTrashSettings.updateSeconds < 5) homeTrashSettings.updateSeconds = 5;
  if (homeTrashSettings.updateSeconds > 300) homeTrashSettings.updateSeconds = 300;
}

static void homeTrashSaveSettings() {
  homeTrashPrefs.begin(HOME_TRASH_PREF_NS, false);
  homeTrashPrefs.putBool("enabled", homeTrashSettings.enabled);
  homeTrashPrefs.putBool("skipWed", homeTrashSettings.skipWednesday);
  homeTrashPrefs.putUChar("startH", homeTrashSettings.startHour);
  homeTrashPrefs.putUChar("startM", homeTrashSettings.startMinute);
  homeTrashPrefs.putUChar("endH", homeTrashSettings.endHour);
  homeTrashPrefs.putUChar("endM", homeTrashSettings.endMinute);
  homeTrashPrefs.putUShort("interval", homeTrashSettings.updateSeconds);
  homeTrashPrefs.putString("carId", homeTrashSettings.carId);
  homeTrashPrefs.end();
}

static bool homeTrashArgBool(AsyncWebServerRequest *request, const char* name, bool fallback) {
  if (!request->hasArg(name)) return fallback;
  String value = request->arg(name);
  value.toLowerCase();
  return value == "1" || value == "true" || value == "on" || value == "yes";
}

static int homeTrashArgInt(AsyncWebServerRequest *request, const char* name, int fallback, int minV, int maxV) {
  if (!request->hasArg(name)) return fallback;
  int v = request->arg(name).toInt();
  if (v < minV) v = minV;
  if (v > maxV) v = maxV;
  return v;
}

static void homeTrashRegisterWebRoutes() {
  server.on("/trash", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html; charset=utf-8", HOME_TRASH_HTML);
  });

  server.on("/trash.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html; charset=utf-8", HOME_TRASH_HTML);
  });

  server.on("/api/hometrash/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{";
    json += "\"enabled\":" + String(homeTrashSettings.enabled ? "true" : "false") + ",";
    json += "\"skipWednesday\":" + String(homeTrashSettings.skipWednesday ? "true" : "false") + ",";
    json += "\"startHour\":" + String(homeTrashSettings.startHour) + ",";
    json += "\"startMinute\":" + String(homeTrashSettings.startMinute) + ",";
    json += "\"endHour\":" + String(homeTrashSettings.endHour) + ",";
    json += "\"endMinute\":" + String(homeTrashSettings.endMinute) + ",";
    json += "\"updateSeconds\":" + String(homeTrashSettings.updateSeconds) + ",";
    json += "\"carId\":\"" + homeTrashJsonEscape(homeTrashSettings.carId) + "\"";
    json += "}";
    request->send(200, "application/json; charset=utf-8", json);
  });

  server.on("/api/hometrash/settings", HTTP_POST, [](AsyncWebServerRequest *request) {
    homeTrashSettings.enabled = homeTrashArgBool(request, "enabled", homeTrashSettings.enabled);
    homeTrashSettings.skipWednesday = homeTrashArgBool(request, "skipWednesday", homeTrashSettings.skipWednesday);
    homeTrashSettings.startHour = homeTrashArgInt(request, "startHour", homeTrashSettings.startHour, 0, 23);
    homeTrashSettings.startMinute = homeTrashArgInt(request, "startMinute", homeTrashSettings.startMinute, 0, 59);
    homeTrashSettings.endHour = homeTrashArgInt(request, "endHour", homeTrashSettings.endHour, 0, 23);
    homeTrashSettings.endMinute = homeTrashArgInt(request, "endMinute", homeTrashSettings.endMinute, 0, 59);
    homeTrashSettings.updateSeconds = homeTrashArgInt(request, "updateSeconds", homeTrashSettings.updateSeconds, 5, 300);

    if (request->hasArg("carId")) {
      String v = request->arg("carId");
      v.trim();
      if (v.length() > 0 && v.length() <= 24) homeTrashSettings.carId = v;
    }

    homeTrashSaveSettings();
    homeTrashForceFetch = true;
    request->send(200, "application/json; charset=utf-8", "{\"ok\":true}");
  });

  server.on("/api/hometrash/fetch", HTTP_POST, [](AsyncWebServerRequest *request) {
    homeTrashForceFetch = true;
    request->send(202, "application/json; charset=utf-8", "{\"ok\":true,\"message\":\"fetch queued\"}");
  });

  server.on("/api/hometrash/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String label = "未知位置";
    if (homeTrashLocationIndex >= 0 && homeTrashLocationIndex < HOME_TRASH_INDEX_COUNT) {
      label = HOME_TRASH_INDEX[homeTrashLocationIndex].displayText;
    }

    long successAgo = -1;
    if (homeTrashLastSuccessMs > 0) successAgo = (long)((millis() - homeTrashLastSuccessMs) / 1000UL);

    String json = "{";
    json += "\"active\":" + String(homeTrashActiveState ? "true" : "false") + ",";
    json += "\"fetching\":" + String(homeTrashFetchInProgress ? "true" : "false") + ",";
    json += "\"httpCode\":" + String(homeTrashLastHttpCode) + ",";
    json += "\"location\":\"" + homeTrashJsonEscape(homeTrashApiLocation) + "\",";
    json += "\"displayLabel\":\"" + homeTrashJsonEscape(label) + "\",";
    json += "\"locationIndex\":" + String((int)homeTrashLocationIndex) + ",";
    json += "\"bitmapReady\":" + String(homeTrashBitmapReady ? "true" : "false") + ",";
    json += "\"lastSuccessAgo\":" + String(successAgo) + ",";
    json += "\"lastError\":\"" + homeTrashJsonEscape(homeTrashLastError) + "\"";
    json += "}";
    request->send(200, "application/json; charset=utf-8", json);
  });
}

// ----------------------------------------------------------------------------
// 上方垃圾車圖：24x16 RGB565，置中顯示。
// 0x0000 當作透明，不覆蓋背景。
// ----------------------------------------------------------------------------
static void homeTrashDrawTruckBitmap() {
  const int startX = (64 - HOME_TRASH_TRUCK_WIDTH) / 2;
  const int startY = 4;

  for (uint8_t y = 0; y < HOME_TRASH_TRUCK_HEIGHT; y++) {
    for (uint8_t x = 0; x < HOME_TRASH_TRUCK_WIDTH; x++) {
      uint16_t color = pgm_read_word(
        &HOME_TRASH_TRUCK_BITMAP[(uint16_t)y * HOME_TRASH_TRUCK_WIDTH + x]
      );

      if (color != 0x0000) {
        display.drawPixel(startX + x, startY + y, color);
      }
    }
  }
}

// 畫指定範圍的 16x15 字元點陣，整行自動置中。
static void homeTrashDrawCharRange(
  int16_t locationIndex,
  uint8_t firstChar,
  uint8_t charCount,
  int drawY
) {
  if (charCount == 0) return;
  if (locationIndex < 0 || locationIndex >= HOME_TRASH_INDEX_COUNT) return;

  // 自訂 struct 僅在函式內使用，避免 Arduino 1.8.x 自動 prototype
  // 在 HomeTrashData.h 被 include 前先看到 HomeTrashDataEntry 而編譯失敗。
  const HomeTrashDataEntry &idx = HOME_TRASH_INDEX[locationIndex];

  const uint8_t CHAR_WIDTH = 16;
  const uint8_t CHAR_HEIGHT = 15;
  const uint8_t BYTES_PER_CHAR = 30;
  const uint16_t lineWidth = (uint16_t)charCount * CHAR_WIDTH;
  const int drawX = (64 - (int)lineWidth) / 2;
  const uint16_t color = display.color565(255, 255, 255);

  for (uint8_t c = 0; c < charCount; c++) {
    uint8_t charIndex = firstChar + c;

    for (uint8_t y = 0; y < CHAR_HEIGHT; y++) {
      for (uint8_t charX = 0; charX < CHAR_WIDTH; charX++) {
        size_t byteIndex =
          (size_t)idx.offset +
          (size_t)charIndex * BYTES_PER_CHAR +
          (size_t)y * 2 +
          (charX >> 3);

        if (byteIndex >= HOME_TRASH_BITMAP_DATA_SIZE) continue;

        uint8_t value = pgm_read_byte(HOME_TRASH_BITMAP_DATA + byteIndex);
        uint8_t mask = 0x80 >> (charX & 7);

        if (value & mask) {
          display.drawPixel(drawX + (int)c * CHAR_WIDTH + charX, drawY + y, color);
        }
      }
    }
  }
}

static void homeTrashDrawEmbeddedBitmap() {
  if (!homeTrashBitmapReady ||
      homeTrashLocationIndex < 0 ||
      homeTrashLocationIndex >= HOME_TRASH_INDEX_COUNT) {
    display.setTextWrap(false);
    display.setTextSize(1);
    display.setTextColor(display.color565(255, 255, 255));
    display.setCursor(11, 44);
    display.print(homeTrashApiLocation.length() ? "NO MAP" : "WAITING");
    return;
  }

  const HomeTrashDataEntry &idx = HOME_TRASH_INDEX[homeTrashLocationIndex];
  const uint8_t totalChars = idx.size / 30;

  // 固定地址庫可針對語意指定換行位置：
  // 成功路209巷 -> 成功路 / 209巷
  // 成功路196號 -> 成功路 / 196號
  // 民成街8號   -> 民成街 / 8號
  if (idx.lineBreakAfter > 0 && idx.lineBreakAfter < totalChars) {
    const uint8_t firstCount = idx.lineBreakAfter;
    const uint8_t secondCount = totalChars - firstCount;
    homeTrashDrawCharRange(homeTrashLocationIndex, 0, firstCount, 31);
    homeTrashDrawCharRange(homeTrashLocationIndex, firstCount, secondCount, 48);
  } else {
    // 1~4 字一行置中。民忠一街 4 字剛好 64px。
    homeTrashDrawCharRange(homeTrashLocationIndex, 0, totalChars, 40);
  }
}

void homeTrashSetup() {
  homeTrashLoadSettings();
  homeTrashRegisterWebRoutes();

  Serial.println(F("[HomeTrash] production module ready"));
  Serial.print(F("[HomeTrash] embedded locations="));
  Serial.print(HOME_TRASH_INDEX_COUNT);
  Serial.print(F(" bitmap bytes="));
  Serial.println(HOME_TRASH_BITMAP_DATA_SIZE);
}

void homeTrashUpdate() {
  homeTrashConsumeFetchResult();
  homeTrashActiveState = homeTrashIsScheduledNow();

  if (homeTrashActiveState && !homeTrashWasActive) {
    Serial.println(F("[HomeTrash] override START"));
    display.clearDisplay();
    homeTrashForceFetch = true;
  }

  if (!homeTrashActiveState && homeTrashWasActive) {
    Serial.println(F("[HomeTrash] override END -> return normal Mode"));
    display.clearDisplay();
  }

  homeTrashWasActive = homeTrashActiveState;

  unsigned long nowMs = millis();
  unsigned long intervalMs = (unsigned long)homeTrashSettings.updateSeconds * 1000UL;
  bool intervalDue = homeTrashActiveState &&
                     !homeTrashFetchInProgress &&
                     (homeTrashLastFetchStartMs == 0 || nowMs - homeTrashLastFetchStartMs >= intervalMs);

  // 網頁「立即抓取」可在排程外確認官方 API 狀態，但不會啟用 Override。
  if ((homeTrashForceFetch || intervalDue) && !homeTrashFetchInProgress) {
    homeTrashForceFetch = false;
    homeTrashStartFetch();
  }
}

bool homeTrashActive() {
  return homeTrashActiveState;
}

void homeTrashDraw() {
  display.clearDisplay();
  homeTrashDrawTruckBitmap();
  display.drawFastHLine(0, 26, 64, display.color565(45, 45, 45));
  homeTrashDrawEmbeddedBitmap();
}

#endif // FEATURE_HOME_TRASH
