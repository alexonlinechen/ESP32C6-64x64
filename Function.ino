#include "Config.h"
#include <esp_wifi.h>    // 控制 WiFi 協定 (b/g/n)
#include <HTTPClient.h>


bool getTimeFromNTP() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi 未連線，無法 NTP 同步");
        return false;
    }

    static bool ntpConfigured = false;

    // 台灣時區 GMT+8，無日光節約時間
    const long gmtOffset_sec = 8 * 3600;
    const int daylightOffset_sec = 0;

    // NTP 只需要設定一次
    if (!ntpConfigured) {
        Serial.println("設定 NTP 伺服器...");

        configTime(
            gmtOffset_sec,
            daylightOffset_sec,
            "pool.ntp.org",
            "time.google.com",
            "time.cloudflare.com"
        );

        ntpConfigured = true;
        delay(500);
    }

    struct tm timeinfo;

    Serial.println("正在使用 NTP 同步時間...");

    // 最多等 10 秒
    if (!getLocalTime(&timeinfo, 10000)) {
        Serial.println("NTP 同步失敗");
        return false;
    }

    // 寫回你的全域時間變數
    H = timeinfo.tm_hour;
    M = timeinfo.tm_min;
    S = timeinfo.tm_sec;

    monthDay = timeinfo.tm_mday;
    currentMonth = timeinfo.tm_mon + 1;   // tm_mon 是 0~11，所以要 +1
    D = timeinfo.tm_wday;                 // 週日=0，週一=1，...週六=6

    Serial.printf("NTP 同步成功: %d月%d日 (週%d) %02d:%02d:%02d\n",
                  currentMonth, monthDay, D, H, M, S);

    return true;
}





// HSV 轉 RGB 函數，針對我們的 1-bit PxMatrix_C6 優化
uint16_t hsv2rgb(uint16_t h, uint8_t s, uint8_t v) {
    float r = 0, g = 0, b = 0;

    // 確保 Hue 落在 0-359，避免 360 變成黃色的偏移
    float h_float = (h % 360) / 60.0;
    float s_float = s / 255.0;
    float v_float = v / 255.0;

    int i = floor(h_float);
    float f = h_float - i;
    float p = v_float * (1.0 - s_float);
    float q = v_float * (1.0 - s_float * f);
    float t = v_float * (1.0 - s_float * (1.0 - f));

    switch (i) {
        case 0: r = v_float; g = t;       b = p;       break; // 紅 -> 黃
        case 1: r = q;       g = v_float; b = p;       break; // 黃 -> 綠
        case 2: r = p;       g = v_float; b = t;       break; // 綠 -> 青
        case 3: r = p;       g = q;       b = v_float; break; // 青 -> 藍
        case 4: r = t;       g = p;       b = v_float; break; // 藍 -> 紫
        default: r = v_float; g = p;      b = q;       break; // 紫 -> 紅
    }

    // 轉換為標準 565 格式
    return display.color565((uint8_t)(r * 255), (uint8_t)(g * 255), (uint8_t)(b * 255));
}

    // 定時開啟/關閉顯示
void TimeONOFF() {

    // 定時開啟/關閉顯示
    if (CheckTime) {
        if (H == end_H && M == end_M) {
            Mode = 99; // 熄屏模式
            display.clearDisplay();
            CheckTime = false;
            Serial.print("熄屏");
        }
    } else {
        if (H == start_H && M == start_M) {
            Mode = EEPROM.read(EEPROM_MODE);
            CheckTime = true;
            Serial.print("開屏");
        }
    }
}

void ShowIP() {
    String myip = WiFi.localIP().toString();
    display.clearDisplay();
    display.setTextColor(display.color565(255, 0, 0));
    display.setCursor(2, 28);
    display.print(myip);
    wait_with_display(3000); // 顯示三秒並保持掃描
}

void save_ssid() {
  dataType d;
  memset(&d, 0, sizeof(d));
  setssid.toCharArray(d.testssid, 32);
  setpwd.toCharArray(d.testpass, 32);
 
  int addr = EEPROM_WIFI_ADDR;
  uint8_t* p = (uint8_t*)(void*)&d;
  for (int i = 0; i < sizeof(d); i++) {
    EEPROM.write(addr++, *p++);
  }
  EEPROM.commit(); // ESP32 必須執行 commit 才會真正寫入
  Serial.println("WiFi Settings Saved to EEPROM");
}


// 定義讀取 EEPROM 的輔助模板 (替代原本可能缺失的 EEPROM_readAnything)
template <class T> int EEPROM_readAnything(int ee, T& value) {
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
        *p++ = EEPROM.read(ee++);
    return i;
}



void setup_wifi() {
    esp_netif_init(); 

    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);

      dataType d;
      EEPROM_readAnything(EEPROM_WIFI_ADDR, d);
      readssid = d.testssid ;
    
    // C6 穩定性設定
    esp_wifi_set_ps(WIFI_PS_NONE); 
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);

    Serial.println("Connecting to WiFi...");
    WiFi.begin(d.testssid, d.testpass);

    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 40) {
        wait_with_display(500); 
        Serial.print(".");
        timeout++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ WiFi Connected!");
        Serial.print(F("IP: "));
        Serial.println(WiFi.localIP());
        // 因為 NTP 封包在你的 C6 上會被底層鎖死，我們改走網頁請求
        getTimeFromNTP(); 

        
    } else {
        setupAP();
    }
}



//設定WIFI為AP模式
  void setupAP() {
    
  //WiFi.disconnect();
  delay(1000);
  const char* APssid = "64x64數字時鐘192.168.4.1";        
  const char* APpassword = "";  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(APssid, APpassword);    // IP is usually 192.168.4.1
  Serial.println();
  Serial.print(F("SoftAP IP: "));
  Serial.println(WiFi.softAPIP());
 // launchWeb();
  Serial.print(F("SoftAP IP: "));
  Serial.println(WiFi.softAPIP());
 }


    //畫X線 
void drawFastXLine(int16_t x, int16_t y, int16_t h, uint16_t c){
  for(int i=x;i<x+h;i++){
    display.drawPixel(i,y,c);
  }
}

void drawFastYLine(int16_t x, int16_t y, int16_t h, int16_t c){
  for(int i=y;i<y+h;i++){
    display.drawPixel(x,i,c);
  }
}


 void showbigbitnumber(int number, int xlength, int ylength, int x, int y, uint16_t colorxy){
  String numStr = String(number);
  if(number<10){
    showbitmap(bitdata30[(int)(0)],xlength,ylength, x, y, colorxy);
    showbitmap(bitdata30[(int)((String(numStr.charAt(0)).toInt() + 1) - 1)],xlength,ylength, x+14, y, colorxy);
  }else if(number<100){
    showbitmap(bitdata30[(int)((String(numStr.charAt(0)).toInt() + 1) - 1)],xlength,ylength, x, y, colorxy);
    showbitmap(bitdata30[(int)((String(numStr.charAt(1)).toInt() + 1) - 1)],xlength,ylength, x+14, y, colorxy);
  }
}


//7x12
void showbit12number(int number, int xlength, int ylength, int x, int y, uint16_t colorxy){
  String numStr = String(number);
  if(number<10){
    showbitmap(bitdata12[(int)(0)],xlength,ylength, x, y, colorxy);
    showbitmap(bitdata12[(int)((String(numStr.charAt(0)).toInt() + 1) - 1)],xlength,ylength, x+9, y, colorxy);
  }else if(number<100){
    showbitmap(bitdata12[(int)((String(numStr.charAt(0)).toInt() + 1) - 1)],xlength,ylength, x, y, colorxy);
    showbitmap(bitdata12[(int)((String(numStr.charAt(1)).toInt() + 1) - 1)],xlength,ylength, x+9, y, colorxy);
  }
}





void showbitmapWeek(int number, int xlength, int ylength, int x, int y, uint16_t colorxy){
  String numStr = String(number);
    //showbitmap(weekdata[(int)(0)],xlength,ylength, x, y, colorxy);
    showbitmap(weekdata[(int)((String(numStr.charAt(0)).toInt() + 1) - 1)],xlength,ylength, x, y, colorxy);
}


void showbitmap(String bitrgbstr, int xlength, int ylength, int x, int y, uint16_t colorxy) {
  //Serial.println("bitrgbstr = " + bitrgbstr);
  for (int i = x; i < x+(xlength); i = i + (1)) {
    for(int j = y; j < y+(ylength); j = j + (1)){
      if (String(bitrgbstr.charAt(((j-y)*xlength+i-x))).toInt() != 0) {
        display.drawPixel(i,j,colorxy);
      } else {
        display.drawPixel(i,j,display.color565( 0, 0, 0));
      }
    }
  }
  }


  //大冒號
void showbigColon(int x,int y,bool l,uint16_t colorxy){
    if(l){
      drawFastXLine(x,y,2,colorxy);
      drawFastXLine(x,y+1,2,colorxy);
     // display.drawPixel(x,y,colorxy);
    }else{
      //display.drawPixel(x,y,display.color565(0, 0, 0));
      drawFastXLine(x,y,2,display.color565(0, 0, 0));
      drawFastXLine(x,y+1,2,display.color565(0, 0, 0));
    }
}


void savedata() {
  
    Serial.print(F("時鐘模式: ")); 
    Serial.println(Mode); 
    Serial.print(F("亮度: ")); 
    Serial.println(brightnessNow); 
    
    Serial.print(F("隨機時鐘模式-間隔(分): "));
    Serial.println(random_min); 
          
    Serial.print(F("GIF重複次數: ")); 
    Serial.println(gifcount); 
    
    Serial.print(F("GIF隨機播放數量: ")); 
    Serial.println(random_gif_no); 

    Serial.print(F("GIF延遲: ")); 
    Serial.println(gifdelay);     
    
}


// =====================================================
// 畫時鐘
// =====================================================
static void drawThemeClockText() {
  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", H, M);

  display.setTextWrap(false);
  display.setTextSize(2);
  display.setTextColor(colorB_565);
  display.setCursor(4, 2);
  display.print(buf);

  display.setTextColor(colorA_565);
  display.setCursor(3, 1);
  display.print(buf);
}



// =====================================================
// 畫黑白格清除動畫
// 由左向右清掉畫面
// =====================================================
void ClearEffect() {
  for (int x = 0; x < 64; x += 8) {
    for (int y = 0; y < 64; y += 8) {
      bool whiteBlock = (((x / 8) + (y / 8)) % 2) == 0;
      uint16_t color = whiteBlock ? 0xffff : 0x0000;

      display.fillRect(x, y, 8, 8, color);
    }

    wait_with_display(120);  // 每一欄進場速度，可調整
  }
}


void ClearAll() {
  for (int i = 0; i < 64; i++) {
      drawFastYLine(i,0,64,0);
      wait_with_display(30);  
    } 
}




// =====================================================
// 畫勝利文字
// =====================================================
static void WinText() {

  display.fillRect(10, 17, 43, 11, 0x0000);
  display.setTextWrap(false);
  display.setTextSize(1);
  display.setTextColor(0x0000);
  display.setCursor(12, 20);
  display.print("YOU WIN");
  display.setTextColor(0xffff);
  display.setCursor(11, 19);
  display.print("YOU WIN");
}


// =====================================================
// 畫 GAME OVER 文字
// =====================================================
static void GameOverText() {
  display.fillRect(4, 17, 58, 11, 0x0000);
  display.setTextWrap(false);
  display.setTextSize(1);
  display.setTextColor(0x0000);
  display.setCursor(8, 20);
  display.print("GAME OVER");
  display.setTextColor(0xf800);
  display.setCursor(7, 19);
  display.print("GAME OVER");
}
