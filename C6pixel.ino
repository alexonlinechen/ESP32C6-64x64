#include "Config.h"
#include "PxMatrix_C6.h" // 請確保你之前修正過的 PxMatrix_C6.h 在同一個資料夾
#include "GifPlayer.h"
#include "Train.h"


// --- XIAO ESP32-C6 引腳定義 ---
#define P_LAT 21
#define P_OE  14
#define P_A   7
#define P_B   19
#define P_C   18
#define P_D   15
#define P_E   6
#define P_CLK 22
#define R1 0
#define G1 2
#define B1 1
#define R2 3
#define G2 5
#define B2 4

// 初始化顯示器
PxMATRIX display(64, 64, P_LAT, P_OE, P_A, P_B, P_C, P_D, P_E);

// 全域實體
AsyncWebServer server(80);

GifPlayer gifPlayer; 



String setssid, setpwd, input, readssid;
uint8_t brightnessNow = 10;
int Mode = 1;
int hue = 180, hueh = 180, huem = 180, hues = 180, huew = 180, hueb = 180;
int randomMode = 1, lastRandomMode = -1;
bool ModefirstRun = true;
int H, M, S, D, currentMonth, monthDay;
int start_H, start_M, end_H, end_M;
int read_start_H, read_start_M, read_end_H, read_end_M;


// 主題時鐘顏色定義
String themeA_str = "#FF0000"; 
String themeB_str = "#000000";
uint16_t colorA_565 = 0xf800; 
uint16_t colorB_565 = 0x0000;

bool CheckTime = true;
unsigned long lastSyncMillis = 0;
const unsigned long syncInterval = 10 * 60 * 1000;    // 10分鐘

// ===================================
//宣告函式  避免後續引用找不到
// ===================================
void FS_Init(); 
void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void handleFSUpdatePage(AsyncWebServerRequest *request);

// 非阻塞等待函數：在等待時持續刷新螢幕，消除閃爍  更新時間
void wait_with_display(int ms) { 
    unsigned long start = millis();
    while (millis() - start < (unsigned long)ms) {
        updateLocalTime();
        display.display();
        yield();
    }
}
void setup() {
  Serial.begin(115200);


// 初始化 LittleFS 檔案系統（加入 true 參數：掛載失敗時自動格式化）
if (!LittleFS.begin(true)) {
  Serial.println("An Error has occurred while mounting LittleFS");
  // 為了防止時鐘變磚，就算 FS 失敗也讓它繼續往下跑，不要 return
} else {
  Serial.println("LittleFS Mount Success!");
}
  
  
  // 顯示器硬體初始化
  display.setRGBPins(R1, G1, B1, R2, G2, B2, P_CLK);
  display.begin();
  display.clearDisplay();

  // EEPROM 初始化
  EEPROM.begin(EEPROM_SIZE);
  initDefaultEEPROMIfNeeded();
  
    brightnessNow = EEPROM.read(EEPROM_BRIGHTNESS);
    Mode = EEPROM.read(EEPROM_MODE);
    hue = int(EEPROM.read(EEPROM_COLOR))*2;
    hueh = int(EEPROM.read(EEPROM_COLOR_H))*2;
    huem = int(EEPROM.read(EEPROM_COLOR_M))*2;
    hues = int(EEPROM.read(EEPROM_COLOR_S))*2;
    huew = int(EEPROM.read(EEPROM_COLOR_W))*2;

    read_start_H = int(EEPROM.read(EEPROM_START_H));
    read_start_M = int(EEPROM.read(EEPROM_START_M));
    read_end_H = int(EEPROM.read(EEPROM_END_H));
    read_end_M = int(EEPROM.read(EEPROM_END_M));
    start_H = int(EEPROM.read(EEPROM_START_H));
    start_M = int(EEPROM.read(EEPROM_START_M));
    end_H = int(EEPROM.read(EEPROM_END_H));
    end_M = int(EEPROM.read(EEPROM_END_M));
    
    gifcount = int(EEPROM.read(EEPROM_GIF_COUNT));
    random_gif_no = int(EEPROM.read(EEPROM_GIF_NO));
    gifdelay = int(EEPROM.read(EEPROM_GIF_DELAY));
    random_min = int(EEPROM.read(EEPROM_RANDOM_MIN));

    EEPROM.get(EEPROM_THEME_A, colorA_565);
    EEPROM.get(EEPROM_THEME_B, colorB_565);
    
   customThemeEnable = EEPROM.read(EEPROM_CUSTOM_ENABLE) == 1;

     for (int i = 0; i < 24; i++) {
        int v = EEPROM.read(EEPROM_CUSTOM_TABLE + i);

        if ((v >= 1 && v <= 20) || v == 99) {
           customThemeSchedule[i] = v;
        } else {
           customThemeSchedule[i] = 99;
        }
      }

     lastCustomThemeHour = -1;
     Serial.println(F("EEPROM 載入自訂主題時間表："));
     Serial.print(F("customThemeEnable = "));
     Serial.println(customThemeEnable);

     for (int i = 0; i < 24; i++) {
         Serial.print(i);
         Serial.print(F(": "));
         Serial.println(customThemeSchedule[i]);
     }

    
  if (Mode == 255) Mode = 1; // 第一次燒錄時的初始值

  display.setBrightness(brightnessNow);


  // 網路初始化
   setup_wifi();
   ShowIP();
   Serial.println("System Initialized");
   launchWeb();
    // 檔案系統初始化 (需配合修改後的 FSbrowser.ino)
   FS_Init();
   
   server.begin();
   Serial.println("HTTP server started");
   
   lastSyncMillis = millis();
   delay(5000);
   display.clearDisplay();
}



void loop() {
  display.display();

  TimeONOFF();    
  SwitchMode();
  
  display.display();
  updateLocalTime();
}



/*


void loop() {
  // 執行目前的時鐘模式
     SwitchMode();


  display.display();

  // 更新時間
     updateLocalTime();


while (Serial.available()) {
  char cmd = Serial.read();
  if (cmd == 'R' || cmd == 'r') {
    Serial.println("Restarting ESP32-C6...");
    Serial.flush();
    delay(200);
    ESP.restart();
  }
}
    

}


*/
