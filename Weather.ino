#include "Weather.h"

// ============================================================
//  Weather.ino
//  功能：
//  1. 取得 OpenWeather 目前天氣
//  2. 取得未來 3 天預報
//  3. 顯示目前天氣畫面
//  4. 顯示未來 3 天預報畫面
//
//  注意：
//  - WEATHER_TEXT[] 是 176x16 橫向中文字圖資，每個字 16x16，共 11 個字。
//  - WEATHER_ICON[] 是 144x16 橫向天氣圖示圖資，每個圖示 16x16，共 9 張。
// ============================================================

#define WEATHER_TEXT_WIDTH        176
#define WEATHER_TEXT_HEIGHT       16
#define WEATHER_CHAR_WIDTH        16
#define WEATHER_CHAR_HEIGHT       16
#define WEATHER_CHAR_COUNT        11

#define WEATHER_ICON_WIDTH        144
#define WEATHER_ICON_HEIGHT       16
#define WEATHER_ICON_FRAME_WIDTH  16
#define WEATHER_ICON_FRAME_HEIGHT 16
#define WEATHER_ICON_FRAME_COUNT  9

// WEATHER_TEXT[] 字元索引
const int WTXT_TIAN    = 0;   // 天
const int WTXT_QI      = 1;   // 氣
const int WTXT_WEN     = 2;   // 溫
const int WTXT_DU      = 3;   // 度
const int WTXT_SHI     = 4;   // 濕
const int WTXT_YU      = 5;   // 預
const int WTXT_BAO     = 6;   // 報
const int WTXT_JIANG   = 7;   // 降
const int WTXT_YU2     = 8;   // 雨
const int WTXT_DEG     = 9;   // 度 / ° 符號
const int WTXT_PERCENT = 10;  // %

const char* openWeatherApiKey = "25cb51d9c95df685694efff635725243";
const char* weatherCity = "Kaohsiung,TW";

// 正式使用建議 10 分鐘以上
const unsigned long WEATHER_UPDATE_INTERVAL = 10UL * 60UL * 1000UL;

// 更新失敗時，1 分鐘後重試
const unsigned long WEATHER_RETRY_INTERVAL = 60UL * 1000UL;


// ============================================================
// 目前天氣資料
// ============================================================
struct CurrentWeather {
    bool valid = false;

    float temp = 0;
    float feelsLike = 0;
    int humidity = 0;
    int pressure = 0;
    float windSpeed = 0;

    String description = "";
    String icon = "";
};


// ============================================================
// 未來天氣資料
//
// forecastDays[0] = 明天
// forecastDays[1] = 後天
// forecastDays[2] = 大後天
// ============================================================
struct ForecastDay {
    bool valid = false;

    int month = 0;
    int day = 0;
    int weekDay = 0;       // 週日=0，週一=1，...，週六=6

    float tempMin = 999;
    float tempMax = -999;

    int humidity = 0;
    int pop = 0;           // 降雨機率 %
    float rainMm = 0;      // 預估雨量 mm

    String description = "";
    String icon = "";

    int displayScore = 99; // 越接近中午 12 點越小
};

CurrentWeather currentWeather;
ForecastDay forecastDays[3];

bool weatherReady = false;


// ============================================================
// 函式宣告
// ============================================================
void playWeatherPages();
void ShowWeather();
void Forcast0();
void Forcast1();
void Forcast2();


// ============================================================
// 清空目前天氣與預報資料
// ============================================================
void resetWeatherData() {
    currentWeather = CurrentWeather();

    for (int i = 0; i < 3; i++) {
        forecastDays[i] = ForecastDay();
    }
}


// ============================================================
// 組 OpenWeather URL
//
// endpoint:
// - "weather"  = 目前天氣
// - "forecast" = 5 天 / 每 3 小時預報
// ============================================================
String makeOpenWeatherUrl(const char* endpoint) {
    String url = "http://api.openweathermap.org/data/2.5/";
    url += endpoint;
    url += "?q=";
    url += weatherCity;
    url += "&appid=";
    url += openWeatherApiKey;
    url += "&units=metric";
    url += "&lang=zh_tw";
    return url;
}


// ============================================================
// 取得目前即時天氣
// ============================================================
bool fetchCurrentWeather() {
    WiFiClient client;
    HTTPClient http;

    String url = makeOpenWeatherUrl("weather");

    Serial.println();
    Serial.println("Request current weather:");
    Serial.println(url);

    if (!http.begin(client, url)) {
        Serial.println("Current weather http.begin failed");
        return false;
    }

    http.setTimeout(10000);

    int httpCode = http.GET();

    Serial.print("Current weather HTTP code: ");
    Serial.println(httpCode);

    if (httpCode != 200) {
        Serial.println("Current weather failed:");
        Serial.println(http.getString());
        http.end();
        return false;
    }

    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, http.getStream());

    http.end();

    if (error) {
        Serial.print("Current weather JSON parse failed: ");
        Serial.println(error.c_str());
        return false;
    }

    currentWeather.valid = true;
    currentWeather.temp = doc["main"]["temp"] | 0.0;
    currentWeather.feelsLike = doc["main"]["feels_like"] | 0.0;
    currentWeather.humidity = doc["main"]["humidity"] | 0;
    currentWeather.pressure = doc["main"]["pressure"] | 0;
    currentWeather.windSpeed = doc["wind"]["speed"] | 0.0;
  
    const char* desc = doc["weather"][0]["description"] | "";
    const char* icon = doc["weather"][0]["icon"] | "";
    currentWeather.description = String(desc);
    currentWeather.icon = String(icon);

    Serial.printf("目前天氣: %.1fC 體感 %.1fC 濕度 %d%% %s icon:%s\n",
                  currentWeather.temp,
                  currentWeather.feelsLike,
                  currentWeather.humidity,
                  currentWeather.description.c_str(),
                  currentWeather.icon.c_str());

    return true;
}


// ============================================================
// 將 forecast 其中一筆資料加入某一天的統計資料
//
// 功能：
// - 統計最低溫
// - 統計最高溫
// - 統計最大降雨機率
// - 累積雨量
// - 選最接近中午 12 點的天氣描述與 icon
// ============================================================
void addForecastItemToDayIndex(int dayIndex, JsonObject item, int hour) {
    if (dayIndex < 0 || dayIndex >= 3) {
        return;
    }

    float tempMin = item["main"]["temp_min"] | 999.0;
    float tempMax = item["main"]["temp_max"] | -999.0;
    int humidity = item["main"]["humidity"] | 0;

    float popValue = item["pop"] | 0.0;
    int popPercent = (int)(popValue * 100.0 + 0.5);

    float rain3h = item["rain"]["3h"] | 0.0;

    const char* desc = item["weather"][0]["description"] | "";
    const char* icon = item["weather"][0]["icon"] | "";

    if (tempMin < forecastDays[dayIndex].tempMin) {
        forecastDays[dayIndex].tempMin = tempMin;
    }

    if (tempMax > forecastDays[dayIndex].tempMax) {
        forecastDays[dayIndex].tempMax = tempMax;
    }

    if (popPercent > forecastDays[dayIndex].pop) {
        forecastDays[dayIndex].pop = popPercent;
    }

    forecastDays[dayIndex].rainMm += rain3h;

    int score = abs(hour - 12);

    if (forecastDays[dayIndex].description == "" ||
        score < forecastDays[dayIndex].displayScore) {

        forecastDays[dayIndex].humidity = humidity;
        forecastDays[dayIndex].description = String(desc);
        forecastDays[dayIndex].icon = String(icon);
        forecastDays[dayIndex].displayScore = score;
    }
}


// ============================================================
// 取得未來 3 天預報
//
// forecastDays[0] = 明天
// forecastDays[1] = 後天
// forecastDays[2] = 大後天
// ============================================================
bool fetchForecast3Days() {
    WiFiClient client;
    HTTPClient http;

    String url = makeOpenWeatherUrl("forecast");

    Serial.println();
    Serial.println("Request forecast:");
    Serial.println(url);

    if (!http.begin(client, url)) {
        Serial.println("Forecast http.begin failed");
        return false;
    }

    http.setTimeout(15000);

    int httpCode = http.GET();

    Serial.print("Forecast HTTP code: ");
    Serial.println(httpCode);

    if (httpCode != 200) {
        Serial.println("Forecast failed:");
        Serial.println(http.getString());
        http.end();
        return false;
    }

    DynamicJsonDocument doc(32768);
    DeserializationError error = deserializeJson(doc, http.getStream());

    http.end();

    if (error) {
        Serial.print("Forecast JSON parse failed: ");
        Serial.println(error.c_str());
        return false;
    }

    for (int i = 0; i < 3; i++) {
        forecastDays[i] = ForecastDay();
    }

    int timezoneOffset = doc["city"]["timezone"] | 28800;
    JsonArray list = doc["list"];

    // 用 NTP 目前時間判斷今天。
    // 若 NTP 尚未成功，則跳過 forecast 第一個日期。
    time_t nowUtc = time(nullptr);
    bool hasValidTime = nowUtc > 1700000000;

    int todayYear = -1;
    int todayYDay = -1;

    if (hasValidTime) {
        time_t localNow = nowUtc + timezoneOffset;
        struct tm nowInfo;
        gmtime_r(&localNow, &nowInfo);

        todayYear = nowInfo.tm_year + 1900;
        todayYDay = nowInfo.tm_yday;
    }

    int currentDayIndex = -1;
    int lastYear = -1;
    int lastYDay = -1;
    bool skippedFirstDateWithoutTime = false;

    for (JsonObject item : list) {
        long utcTime = item["dt"] | 0;

        time_t localTime = utcTime + timezoneOffset;
        struct tm timeinfo;
        gmtime_r(&localTime, &timeinfo);

        int year = timeinfo.tm_year + 1900;
        int month = timeinfo.tm_mon + 1;
        int day = timeinfo.tm_mday;
        int yday = timeinfo.tm_yday;
        int weekDay = timeinfo.tm_wday;
        int hour = timeinfo.tm_hour;

        bool isNewDate = (year != lastYear || yday != lastYDay);

        if (isNewDate) {
            lastYear = year;
            lastYDay = yday;

            // NTP 正常時，明確跳過今天
            if (hasValidTime && year == todayYear && yday == todayYDay) {
                continue;
            }

            // NTP 還沒成功時，跳過 forecast 第一個日期
            if (!hasValidTime && !skippedFirstDateWithoutTime) {
                skippedFirstDateWithoutTime = true;
                continue;
            }

            currentDayIndex++;

            if (currentDayIndex >= 3) {
                break;
            }

            forecastDays[currentDayIndex].valid = true;
            forecastDays[currentDayIndex].month = month;
            forecastDays[currentDayIndex].day = day;
            forecastDays[currentDayIndex].weekDay = weekDay;
        }

        if (currentDayIndex < 0 || currentDayIndex >= 3) {
            continue;
        }

        addForecastItemToDayIndex(currentDayIndex, item, hour);
    }

    Serial.println();
    Serial.println("===== 未來3天天氣預報 =====");

    for (int i = 0; i < 3; i++) {
        if (!forecastDays[i].valid) {
            continue;
        }

        Serial.printf("Day %d: %d/%d 週%d %.1f~%.1fC 濕度:%d%% 降雨:%d%% 雨量:%.1fmm %s icon:%s\n",
                      i + 1,
                      forecastDays[i].month,
                      forecastDays[i].day,
                      forecastDays[i].weekDay,
                      forecastDays[i].tempMin,
                      forecastDays[i].tempMax,
                      forecastDays[i].humidity,
                      forecastDays[i].pop,
                      forecastDays[i].rainMm,
                      forecastDays[i].description.c_str(),
                      forecastDays[i].icon.c_str());
    }

    Serial.println("==========================");

    return true;
}


// ============================================================
// 主要天氣更新函式
//
// 建議在 loop() 中一直呼叫：
//     updateWeather();
//
// 行為：
// - 開機後第一次會立即抓天氣
// - 抓成功後，依 WEATHER_UPDATE_INTERVAL 重新更新
// - 抓失敗後，依 WEATHER_RETRY_INTERVAL 重試
// ============================================================
bool updateWeather() {
    // 每次進 loop 先刷新上方時間區。
    // 若主程式已經負責畫時鐘，可移除此段。
    display.fillRect(0, 0, 64, 16, 0x0000);
    drawThemeClockText();

    static unsigned long lastWeatherUpdate = 0;
    static unsigned long lastWeatherAttempt = 0;

    unsigned long nowMs = millis();

    // 已成功取得資料，且尚未到更新時間：不重新打 API
    if (weatherReady &&
        lastWeatherUpdate != 0 &&
        nowMs - lastWeatherUpdate < WEATHER_UPDATE_INTERVAL) {
        return true;
    }

    // 還沒成功時，失敗後才等待重試時間。
    // lastWeatherAttempt != 0 可避免開機第一次被擋住。
    if (!weatherReady &&
        lastWeatherAttempt != 0 &&
        nowMs - lastWeatherAttempt < WEATHER_RETRY_INTERVAL) {
        return false;
    }

    lastWeatherAttempt = nowMs;

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected, weather update skipped");
        weatherReady = false;
        return false;
    }

    Serial.println();
    Serial.println("開始更新天氣資料...");

    resetWeatherData();

    bool currentOK = fetchCurrentWeather();

    delay(300);

    bool forecastOK = fetchForecast3Days();

    weatherReady = currentOK && forecastOK;

    if (!weatherReady) {
        Serial.println("天氣資料更新不完整");
        return false;
    }

    lastWeatherUpdate = millis();

    Serial.println("天氣資料更新完成");

    playWeatherPages();

    return true;
}


// ============================================================
// 繪製 WEATHER_TEXT[] 中的單一中文字
//
// WEATHER_TEXT[] 是 176x16 橫向圖資。
// 每個字 16x16。
//
// 圖資值：
// 0 = 不畫
// 1 = color1
// 2 = color2
// ============================================================
void drawWeatherTextChar(
    int charIndex,
    int x,
    int y,
    uint16_t color1,
    uint16_t color2
) {
    if (charIndex < 0 || charIndex >= WEATHER_CHAR_COUNT) {
        return;
    }

    for (int row = 0; row < WEATHER_CHAR_HEIGHT; row++) {
        for (int col = 0; col < WEATHER_CHAR_WIDTH; col++) {
            int sourceX = charIndex * WEATHER_CHAR_WIDTH + col;
            int dataIndex = row * WEATHER_TEXT_WIDTH + sourceX;

            uint16_t v = pgm_read_word(&WEATHER_TEXT[dataIndex]);

            if (v == 1) {
                display.drawPixel(x + col, y + row, color1);
            } else if (v == 2) {
                display.drawPixel(x + col, y + row, color2);
            }
        }
    }
}


// ============================================================
// 從 startCharIndex 開始連續畫 length 個中文字
//
// 例：
// drawWeatherTextChars(WTXT_TIAN, 2, ...) 會畫「天氣」
// drawWeatherTextChars(WTXT_WEN, 2, ...) 會畫「溫度」
// ============================================================
void drawWeatherTextChars(
    int startCharIndex,
    int length,
    int x,
    int y,
    uint16_t color1,
    uint16_t color2,
    int spacing = 0
) {
    for (int i = 0; i < length; i++) {
        int charIndex = startCharIndex + i;

        if (charIndex >= WEATHER_CHAR_COUNT) {
            break;
        }

        int drawX = x + i * (WEATHER_CHAR_WIDTH + spacing);
        drawWeatherTextChar(charIndex, drawX, y, color1, color2);
    }
}


// ============================================================
// OpenWeather icon code 轉 WEATHER_ICON[] frame
//
// WEATHER_ICON[] frame 對應：
// frame 0 = 01d
// frame 1 = 01n
// frame 2 = 02d
// frame 3 = 02n
// frame 4 = 03d / 03n
// frame 5 = 04d / 04n
// frame 6 = 09d / 09n
// frame 7 = 10d / 10n
// frame 8 = 11d / 11n
// ============================================================
int getWeatherIconFrame(const String& iconCode) {
    if (iconCode == "01d") return 0;
    if (iconCode == "01n") return 1;

    if (iconCode == "02d") return 2;
    if (iconCode == "02n") return 3;

    if (iconCode == "03d" || iconCode == "03n") return 4;
    if (iconCode == "04d" || iconCode == "04n") return 5;

    if (iconCode == "09d" || iconCode == "09n") return 6;
    if (iconCode == "10d" || iconCode == "10n") return 7;
    if (iconCode == "11d" || iconCode == "11n") return 8;

    // 沒有雪 / 霧圖時，用多雲代替
    if (iconCode == "13d" || iconCode == "13n") return 5;
    if (iconCode == "50d" || iconCode == "50n") return 5;

    return 5;
}


// ============================================================
// 繪製 WEATHER_ICON[] 中的指定 frame
//
// WEATHER_ICON[] 是 144x16 橫向圖資。
// 每個 icon 16x16。
// 內容已經是 RGB565，所以直接 drawPixel。
//
// 若 drawTransparent = true，0x0000 視為透明不畫。
// ============================================================
void drawWeatherIconFrame(
    int frameIndex,
    int x,
    int y,
    bool drawTransparent = true
) {
    if (frameIndex < 0 || frameIndex >= WEATHER_ICON_FRAME_COUNT) {
        return;
    }

    for (int row = 0; row < WEATHER_ICON_FRAME_HEIGHT; row++) {
        for (int col = 0; col < WEATHER_ICON_FRAME_WIDTH; col++) {
            int sourceX = frameIndex * WEATHER_ICON_FRAME_WIDTH + col;
            int dataIndex = row * WEATHER_ICON_WIDTH + sourceX;

            uint16_t color = pgm_read_word(&WEATHER_ICON[dataIndex]);

            if (drawTransparent && color == 0x0000) {
                continue;
            }

            display.drawPixel(x + col, y + row, color);
        }
    }
}


// ============================================================
// 繪製目前天氣 icon
// ============================================================
void weathericon(int x, int y) {
    if (!currentWeather.valid) {
        return;
    }

    int frameIndex = getWeatherIconFrame(currentWeather.icon);
    drawWeatherIconFrame(frameIndex, x, y, true);
}


// ============================================================
// 繪製指定預報日 icon
//
// forecastIndex:
// 0 = 明天
// 1 = 後天
// 2 = 大後天
// ============================================================
void forecasticon(int forecastIndex, int x, int y) {
    if (forecastIndex < 0 || forecastIndex >= 3) {
        return;
    }

    if (!forecastDays[forecastIndex].valid) {
        return;
    }

    int frameIndex = getWeatherIconFrame(forecastDays[forecastIndex].icon);
    drawWeatherIconFrame(frameIndex, x, y, true);
}


// ============================================================
// 顯示預報日期，例如 5/20
// ============================================================
void drawForecastDate(int forecastIndex, int x, int y, uint16_t color) {
    if (forecastIndex < 0 || forecastIndex >= 3) {
        return;
    }

    if (!forecastDays[forecastIndex].valid) {
        return;
    }

    display.setTextSize(1);
    display.setTextColor(color);
    display.setCursor(x, y);

    display.print(forecastDays[forecastIndex].month);
    display.print("/");
    display.print(forecastDays[forecastIndex].day);
}


// ============================================================
// 顯示目前天氣畫面
//
// 第 0~15 列：時間
// 第 16~31 列：天氣 + icon
// 第 32~47 列：溫度
// 第 48~63 列：濕度
// ============================================================
void ShowWeather() {
    drawThemeClockText();

    uint16_t yellow1 = display.color565(255, 255, 0);
    uint16_t yellow2 = display.color565(255, 80, 0);

    uint16_t blue1 = display.color565(0, 180, 255);
    uint16_t blue2 = display.color565(0, 80, 255);

    // 天氣
    drawWeatherTextChars(WTXT_TIAN, 2, 0, 16, yellow1, yellow2);

    // 目前天氣 icon
    weathericon(40, 16);

    // 溫度
    drawWeatherTextChars(WTXT_WEN, 2, 0, 32, yellow1, yellow2);

    int tempInt = (int)currentWeather.temp;
    display.setTextColor(hsv2rgb(hueh, saturation, value));
    display.setTextSize(2);
    display.setCursor(32, 33);
    display.print(tempInt);

    // 度
    drawWeatherTextChar(WTXT_DEG, 56, 32, yellow1, yellow2);

    // 濕度
    drawWeatherTextChar(WTXT_SHI, 0, 48, blue1, blue2);
    drawWeatherTextChar(WTXT_DU, 16, 48, blue1, blue2);

    int humInt = currentWeather.humidity;
    display.setTextColor(hsv2rgb(huem, saturation, value));
    display.setTextSize(2);
    display.setCursor(32, 49);
    display.print(humInt);

    // %
    if(humInt<100)drawWeatherTextChar(WTXT_PERCENT, 54, 48, blue1, blue2);
}


// ============================================================
// 顯示指定一天的天氣預報畫面
//
// forecastIndex:
// 0 = 明天
// 1 = 後天
// 2 = 大後天
//
// 第 0~15 列：天氣預報
// 第 16~31 列：日期 + 星期
// 第 32~47 列：天氣 + icon
// 第 48~63 列：降雨機率
// ============================================================
void ShowForecastPage(int forecastIndex) {
    if (forecastIndex < 0 || forecastIndex >= 3) {
        return;
    }

    if (!forecastDays[forecastIndex].valid) {
        return;
    }

    uint16_t yellow1 = display.color565(255, 255, 0);
    uint16_t yellow2 = display.color565(255, 80, 0);

    uint16_t blue1 = display.color565(0, 180, 255);
    uint16_t blue2 = display.color565(0, 80, 255);

    // 天氣預報
    drawWeatherTextChars(WTXT_TIAN, 2, 0, 0, yellow1, yellow2);
    drawWeatherTextChars(WTXT_YU, 2, 32, 0, yellow1, yellow2);

    // 日期
    drawForecastDate(forecastIndex, 2, 22, hsv2rgb(huew, saturation, value));

    // 星期
    showbitmapWeek(7, 9, 12, 30, 17, hsv2rgb(huew, saturation, value)); // 星
    showbitmapWeek(8, 9, 12, 40, 17, hsv2rgb(huew, saturation, value)); // 期

    int weekDay = forecastDays[forecastIndex].weekDay;
    showbitmapWeek(weekDay, 9, 12, 53, 17, hsv2rgb(huew, saturation, value));

    // 天氣 icon
    drawWeatherTextChars(WTXT_TIAN, 2, 0, 32, yellow1, yellow2);
    forecasticon(forecastIndex, 40, 32);

    // 降雨
    drawWeatherTextChars(WTXT_JIANG, 2, 0, 48, blue1, blue2);

    int popInt = forecastDays[forecastIndex].pop;
    display.setTextColor(hsv2rgb(huem, saturation, value));
    display.setTextSize(2);
    display.setCursor(32, 49);
    display.print(popInt);

    // %
    if(popInt<100)drawWeatherTextChar(WTXT_PERCENT, 54, 48, blue1, blue2);
}


// ============================================================
// 保留你原本的函式名稱
// ============================================================
void Forcast0() {
    ShowForecastPage(0);
}

void Forcast1() {
    ShowForecastPage(1);
}

void Forcast2() {
    ShowForecastPage(2);
}


// ============================================================
// 播放天氣頁面
//
// 流程：
// 目前天氣 20 秒
// 明天預報 20 秒
// 後天預報 20 秒
// 大後天預報 20 秒
// 回到目前天氣
// ============================================================
void playWeatherPages() {
    ClearAll();
    ShowWeather();
    wait_with_display(20000);

    ClearAll();
    Forcast0();
    wait_with_display(20000);

    ClearAll();
    Forcast1();
    wait_with_display(20000);

    ClearAll();
    Forcast2();
    wait_with_display(20000);

    ClearAll();
    ShowWeather();
}
