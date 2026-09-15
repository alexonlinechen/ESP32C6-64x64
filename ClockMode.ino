#include "Config.h"


//自訂義主題 每小時檢查函式
void checkCustomThemeSchedule() {
  if (!customThemeEnable) return;

  if (H < 0 || H > 23) return;

  if (lastCustomThemeHour == H) return;

  int targetMode = customThemeSchedule[H];

  Serial.print(F("自訂主題時間，目前 H = "));
  Serial.print(H);
  Serial.print(F("，目標 Mode = "));
  Serial.println(targetMode);

  lastCustomThemeHour = H;

  if ((targetMode >= 1 && targetMode <= THEME_MODE_MAX) || targetMode == 99) {
    display.clearDisplay();
    ModefirstRun = true;
    Mode = targetMode;
  }
}


// 模式切換邏輯
void SwitchMode() {

  checkCustomThemeSchedule();
  switch (Mode) {
    
    case 1: ClockMode1(); break;
    case 2: ClockMode2(); break;
    case 3: ClockMode3(); break;
    case 4: ClockMode4(); break;
    case 5: updateWeather(); break;    
    case 6: TetrisMode(); break;
    case 7: MarioClockMode(); break;
    case 8: MarioMode(); break;
    case 9: MarioSpaceMode(); break;
    case 10: KartMode(); break;
    case 11: MotoMode(); break;
    case 12: PacmanMode(); break;    
    case 13: SonicMode(); break;  
    case 14: ZeldaMode(); break;       
    case 15: BubbleMode(); break;
    case 16: BombMode(); break;  
    case 17: BirdMode(); break;  
    case 18: DrillMode(); break; 
    case 19: FarmMode(); break;       
    case 20: DuckMode(); break;
    case 21: DogMode(); break; 
    case 22: TrainMode(); break;  
    case 23: MarioTrainMode(); break;      
    case 24: MetroMode(); break; 
    case 25: TaxiMode(); break;    
    case 26: IslandMode(); break;  
    case 27: ZooMode(); break;  
    
    case 28:
      ClockMode5(); // 隨機模式
      break;


      
      
    case 50:
      playGif();  
      break; 
         
    case 99:
      display.clearDisplay();
      break;
  }
}


void ClockMode1(){        

  unsigned long currentMillis = millis();

  static int animH_Y = 20;            // 小時目前的滾動 Y 座標 (預設在定格位置 20)
  static int animM_Y = 20;            // 分鐘目前的滾動 Y 座標 (預設在定格位置 20)
  static unsigned long lastAnimH = 0; // 小時動畫計時器
  static unsigned long lastAnimM = 0; // 分鐘動畫計時器

  // =====================================================
  // 1. 小時滾動動畫邏輯 (非阻塞)
  // =====================================================
  // 偵測到小時改變，立即啟動動畫
  if (tempH != H) {
    animH_Y = -20; // 讓新的數字從上方外面 (-20) 開始進場
    tempH = H;     // 立即同步，防止重複觸發
  }

  // 每 20 毫秒讓數字向下滾動一格，直到抵達定格位置 (20)
  if (animH_Y < 20) {
    if (currentMillis - lastAnimH >= 20) {
      animH_Y++;
      lastAnimH = currentMillis;
      // 滾動時，清空螢幕左半邊（小時顯示區域），防止殘影
      display.fillRect(0, 0, 31, 64, display.color565(0, 0, 0)); 
    }
  }

  // 畫出小時數字（動畫中會變動 animH_Y，定格時 animH_Y 就是 20）
  showbigbitnumber(H, 12, 20, 2, animH_Y, hsv2rgb(hueh, saturation, value));


  // =====================================================
  // 2. 冒號閃爍邏輯 (維持原樣)
  // =====================================================
  if (currentMillis - colonTime > 1000) {
    colon = !colon;
    colonTime = currentMillis;
  }
  showbigColon(31, 25, colon, hsv2rgb(hue, saturation, value));
  showbigColon(31, 34, colon, hsv2rgb(hue, saturation, value));


  // =====================================================
  // 3. 分鐘滾動動畫邏輯 (非阻塞)
  // =====================================================
  // 偵測到分鐘改變，立即啟動動畫
  if (tempM != M) {
    animM_Y = -20; // 讓新的數字從上方外面 (-20) 開始進場
    tempM = M;     // 立即同步
  }

  // 每 20 毫秒讓數字向下滾動一格，直到抵達定格位置 (20)
  if (animM_Y < 20) {
    if (currentMillis - lastAnimM >= 20) {
      animM_Y++;
      lastAnimM = currentMillis;
      // 滾動時，清空螢幕右半邊（分鐘顯示區域），防止殘影
      display.fillRect(32, 0, 32, 64, display.color565(0, 0, 0));
    }
  }

  // 畫出分鐘數字（動畫中會變動 animM_Y，定格時 animM_Y 就是 20）
  showbigbitnumber(M, 12, 20, 36, animM_Y, hsv2rgb(huem, saturation, value));
  
}





//时钟模式一  HH:mm + 日期 
void ClockMode2(){        


if ( millis() - last_H_Time > 60000){   //間隔20秒顯示一次 避免閃爍


//時間H   變化/動畫效果
   if( tempH != H){
   for(int i=-20;i<7;i++){
      display.fillRect(0,0,64,6,display.color565(0,0,0));
      showbigbitnumber(H,12,20,2,i,hsv2rgb(hueh, saturation, value)); 
      wait_with_display(20);
      tempH = H;    
         }
      }    
      
     showbigbitnumber(tempH,12,20,2,6,hsv2rgb(hueh, saturation, value));  
     last_H_Time = millis();
  } 

  
  if ( millis() - colonTime > 1000 )
  {
    colon = !colon;
    colonTime = millis();
  }
  showbigColon(31,11,colon,hsv2rgb(hue, saturation, value));
  showbigColon(31,20,colon,hsv2rgb(hue, saturation, value));
      


if ( millis() - last_M_Time > 25000){ 
  
//時間M   變化/動畫效果
 if( tempM != M){
   for(int i=-20;i<7;i++){
      display.fillRect(0,0,64,6,display.color565(0,0,0));
      showbigbitnumber(M,12,20,36,i,hsv2rgb(huem, saturation, value));  
      wait_with_display(20);
      tempM = M;
           }
          }
     showbigbitnumber(tempM,12,20,36,6,hsv2rgb(huem, saturation, value));
     last_M_Time = millis();
 


  //顯示 日期 星期
  display.fillRect(0,33,64,64,display.color565(0,0,0));

   showbit12number(currentMonth,7,12,2,35,hsv2rgb(hueh, saturation, value));  // 數字月
   showbitmapWeek(9,9,12,22,35,hsv2rgb(huew, saturation, value));   //月
   showbit12number(monthDay,7,12,35,35,hsv2rgb(huem, saturation, value));    // 數字日
   showbitmapWeek(0,9,12,54,35,hsv2rgb(huew, saturation, value));   //日

   showbitmapWeek(7,9,12,14,50,hsv2rgb(huew, saturation, value));   //星
   showbitmapWeek(8,9,12,26,50,hsv2rgb(huew, saturation, value));   //期
   
   if(D == 6) showbitmapWeek(D,9,12,40,50,display.color565(0,255,0));  
   else if (D == 0) showbitmapWeek(D,9,12,40,50,display.color565(255,0,0)); 
   else showbitmapWeek(D,9,12,40,50,hsv2rgb(hueh, saturation, value));  
  
   
 } 
          
}





//时钟模式  HH:mm  + GIF
void ClockMode3(){        


//間隔30秒顯示時間
if ( millis() - lastTime > 30000){  

     display.fillRect(0,0,64,14,display.color565(0,0,0));

    /*      
     showbit12number(H,7,12,14,1,hsv2rgb(hueh, saturation, value));   
     display.drawPixel(32,4,hsv2rgb(hue, saturation, value));
     display.drawPixel(32,8,hsv2rgb(hue, saturation, value));
     showbit12number(M,7,12,35,1,hsv2rgb(huem, saturation, value)); 
    */

  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", H, M);

  display.setTextWrap(false);
  display.setTextSize(2);
  display.setTextColor(hsv2rgb(huem, saturation, value));
  display.setCursor(4, 1);
  display.print(buf);

  display.setTextColor(hsv2rgb(hueh, saturation, value));
  display.setCursor(3, 0);
  display.print(buf);

        
         GifClock = true ;    //  啟用 讓Gif 配合時間  向下偏移 

          if(GifRandom)  randomGif();
          else playGif();  
          
         GifClock = false ;
         lastTime = millis();
       }   
}


//大时钟模式  HH:mm  + GIF
void ClockMode4(){        


//間隔30秒顯示時間
if ( millis() - lastTime > 30000){  
     //清空時鐘顯示區域
     display.fillRect(0,0,64,14,display.color565(0,0,0));

     showbit12number(H,7,12,14,1,hsv2rgb(hueh, saturation, value));   
     display.drawPixel(32,4,hsv2rgb(hue, saturation, value));
     display.drawPixel(32,8,hsv2rgb(hue, saturation, value));
     showbit12number(M,7,12,35,1,hsv2rgb(huem, saturation, value)); 
 
        
         GifClock = true ;    //  啟用 讓Gif 配合時間  向下偏移 

          if(GifRandom)  randomGif();
          else playGif();  
          
         GifClock = false ;

      wait_with_display(6000);
      display.clearDisplay();
      showbigbitnumber(H,12,20,2,20,hsv2rgb(hueh, saturation, value)); 
      showbigbitnumber(M,12,20,36,20,hsv2rgb(huem, saturation, value));
      drawFastXLine(31,25,2,hsv2rgb(hue, saturation, value));
      drawFastXLine(31,34,2,hsv2rgb(hue, saturation, value));

       wait_with_display(10000);
      
   
         lastTime = millis();
       }   
}





// 隨機模式
void ClockMode5() {
  if (randomMode != lastRandomMode) {
    if (randomMode == 5 || randomMode == 7) {
      ModefirstRun = true; // 只要切換到模式 5，就強制重置初始化標記
    }
    lastRandomMode = randomMode; // 更新紀錄
  }

  switch(randomMode) {
    case 1: ClockMode1(); break;
    case 2: ClockMode2(); break;
    case 3: ClockMode3(); break;
    case 4: ClockMode4(); break;
    case 5: updateWeather(); break;    
    case 6: TetrisMode(); break;
    case 7: MarioClockMode(); break;
    case 8: MarioMode(); break;
    case 9: MarioSpaceMode(); break;
    case 10: KartMode(); break;
    case 11: MotoMode(); break;
    case 12: PacmanMode(); break;    
    case 13: SonicMode(); break;  
    case 14: ZeldaMode(); break;       
    case 15: BubbleMode(); break;
    case 16: BombMode(); break;  
    case 17: BirdMode(); break;  
    case 18: DrillMode(); break; 
    case 19: FarmMode(); break;       
    case 20: DuckMode(); break;
    case 21: DogMode(); break; 
    case 22: TrainMode(); break;  
    case 23: MarioTrainMode(); break;      
    case 24: MetroMode(); break;     
    case 25: TaxiMode(); break;
    case 26: IslandMode(); break;    
    case 27: ZooMode(); break; 
  }

if ( millis() - randomTime > random_min*60000){    //間隔 - 分鐘顯示時間

   randomMode = random(1, THEME_MODE_MAX + 1);
   Serial.print(F("切換隨機模式："));
   Serial.println(randomMode);
   display.clearDisplay();
   randomTime = millis();
   ModefirstRun = true; 
 } 
  
}






void updateLocalTime() {
    static unsigned long lastTick = 0;
    unsigned long now = millis();

    // 第一次進來時初始化基準，避免開機初期出現大跳秒
    if (lastTick == 0) {
        lastTick = now;
    }

    // --- 部分 A: 本地秒數累加 ---
    // 用 while 補秒，避免 loop 卡住時漏秒
    while (now - lastTick >= 1000) {
        lastTick += 1000;

        S++;
        if (S >= 60) {
            S = 0;
            M++;
        }

        if (M >= 60) {
            M = 0;
            H++;
        }

        if (H >= 24) {
            H = 0;
        }
    }

    // --- 部分 B: 定時網路校準 ---
    // 每 30 分鐘檢查一次，WiFi 連線時才發 HTTP
    if (now - lastSyncMillis >= syncInterval) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nScheduled Time Sync...");

            if (getTimeFromNTP()) {
                // 同步成功：重設同步計時器與本地秒基準
                lastSyncMillis = now;
                lastTick = now;
            } else {
                // 同步失敗：5 分鐘後再試一次
                lastSyncMillis = now - (syncInterval - 5UL * 60UL * 1000UL);
            }
        }
    }
}
