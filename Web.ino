#include "Config.h"

// 設定 Web 各項功能
void launchWeb() {

server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists("/index.html")) {
        request->send(LittleFS, "/index.html", "text/html");
    } else {
        String html;
        html += "<!DOCTYPE html><html><head>";
        html += "<meta charset='UTF-8'>";
        html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
        html += "<title>C6Pixel First Setup</title>";
        html += "<style>";
        html += "body{font-family:Arial;background:#111;color:#eee;padding:20px;}";
        html += ".box{max-width:480px;margin:auto;background:#222;padding:20px;border-radius:12px;}";
        html += "a{display:block;background:#2196f3;color:white;padding:12px;margin:12px 0;text-align:center;text-decoration:none;border-radius:8px;}";
        html += "</style></head><body><div class='box'>";
        html += "<h2>C6Pixel First Setup</h2>";
        html += "<p>LittleFS 尚未上傳 index.html。</p>";
        html += "<a href='/upload'>上傳 index.html / fs.html</a>";
        html += "<a href='/status'>查看 LittleFS 狀態</a>";
        html += "</div></body></html>";

        request->send(200, "text/html", html);
    }
});


    // 1. 檔案系統更新頁面 (注意參數)
// 💡 【全新補上】：對接網頁 Update 按鈕的 POST 路由，把傳進來的檔案存入 LittleFS
    server.on("/update", HTTP_POST, 
        [](AsyncWebServerRequest *request) {
            // 檔案傳完後的網頁回應
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Upload Success! Please restart ESP32.");
            response->addHeader("Connection", "close");
            request->send(response);
        }, 
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            static File tempFile;
            if (!index) {
                if (!filename.startsWith("/")) filename = "/" + filename;
                // 以寫入模式打開檔案
                tempFile = LittleFS.open(filename, "w");
            }
            if (len && tempFile) {
                tempFile.write(data, len); // 寫入資料片段
            }
            if (final && tempFile) {
                tempFile.close(); // 傳輸完成，關閉檔案
            }
        }
    );

    // 2. 模式切換路由 - 時間模式
    server.on("/bigclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
        display.clearDisplay();   
        customThemeEnable = false;
        Mode = 1;
        Serial.println(F("時間模式")); 
        request->send(200, "text/plain", "OK");
    });  

    // 3. 模式切換路由 - 時間 + 日期模式
    server.on("/weekclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
        display.clearDisplay();   
        customThemeEnable = false;
        Mode = 2;
        Serial.println(F("時間 + 日期模式")); 
        request->send(200, "text/plain", "OK");
    });  


 server.on("/gifclock", HTTP_POST, [](AsyncWebServerRequest *request) {   
    display.clearDisplay();  
    customThemeEnable = false; 
    Mode = 3;
    Serial.println(F("時間 + Gif模式")); 
     });

    server.on("/gifbigclock", HTTP_POST, [](AsyncWebServerRequest *request) {   
    display.clearDisplay();   
    customThemeEnable = false;
    Mode = 4;
    Serial.println(F("大時間 + Gif模式")); 
     });



    server.on("/Weatherclock", HTTP_POST, [](AsyncWebServerRequest *request) {   
    display.clearDisplay();  
    customThemeEnable = false;
    Mode = 5; 
    Serial.println(F("天氣 模式")); 
     });


    server.on("/Tetrisclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay();  
    customThemeEnable = false; 
    ModefirstRun = true;  
    Mode = 6;
    Serial.println(F("俄羅斯方塊模式")); 

    });

    server.on("/Marioclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay(); 
    customThemeEnable = false;  
    ModefirstRun = true;  
    Mode = 7;
    Serial.println(F("瑪利歐模式")); 
    });  

    server.on("/Marioclock2", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay();   
    customThemeEnable = false;
    ModefirstRun = true;  
    Mode = 8;
    Serial.println(F("瑪利歐模式2")); 
    }); 

    server.on("/Spaceclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay();   
    customThemeEnable = false;
    ModefirstRun = true;  
    Mode = 9;
    Serial.println(F("瑪利歐太空模式")); 
    });


    server.on("/Kartclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay();
    customThemeEnable = false;   
    ModefirstRun = true;  
    Mode = 10;
    Serial.println(F("瑪利歐賽車模式")); 

    }); 


    server.on("/Motoclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay();  
    customThemeEnable = false; 
    ModefirstRun = true;  
    Mode = 11;
    Serial.println(F("瑪利歐 摩托模式")); 

    }); 
    

    server.on("/pacmanmode", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay(); 
    customThemeEnable = false;  
    ModefirstRun = true;  
    Mode = 12;
    Serial.println(F("Pacman 小精靈模式")); 
    });


    server.on("/Sonicclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay();  
    customThemeEnable = false; 
    ModefirstRun = true;  
    Mode = 13;
    Serial.println(F("音速小子 模式")); 
    });


    server.on("/Zeldaclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay();  
    customThemeEnable = false; 
    ModefirstRun = true;  
    Mode = 14;
    Serial.println(F("賽爾達˙ 模式")); 
    }); 

    server.on("/Bubbleclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay();
    customThemeEnable = false;   
    ModefirstRun = true;  
    Mode = 15;
    Serial.println(F("泡泡龍模式")); 
    }); 


    server.on("/Bombclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay();  
    customThemeEnable = false; 
    ModefirstRun = true;  
    Mode = 16;
    Serial.println(F("轟炸超人˙ 模式")); 
    }); 

    server.on("/Birdclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay();  
    customThemeEnable = false; 
    ModefirstRun = true;  
    Mode = 17;
    Serial.println(F("憤怒鳥 模式")); 
    });


    server.on("/Drillclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay();  
    customThemeEnable = false; 
    ModefirstRun = true;  
    Mode = 18;
    Serial.println(F("鑽子先生 模式")); 
    }); 


    server.on("/Farmclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay();  
    customThemeEnable = false; 
    ModefirstRun = true;  
    Mode = 19;
    Serial.println(F("牧場物語 模式")); 
    });


    server.on("/Duckclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay(); 
    customThemeEnable = false;  
    ModefirstRun = true;  
    Mode = 20;
    Serial.println(F(" 小鴨 模式")); 
    }); 

    server.on("/Petclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay();
    customThemeEnable = false;   
    ModefirstRun = true;  
    Mode = 21;
    Serial.println(F(" 寵物模式")); 
    }); 


    server.on("/Trainclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay();   
    customThemeEnable = false;
    ModefirstRun = true;  
    Mode = 22;
    Serial.println(F("台鐵˙ 模式")); 
    }); 


    server.on("/MarioTrainclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay();
    customThemeEnable = false;   
    ModefirstRun = true;  
    Mode = 23;
    Serial.println(F("馬力歐 鐵路˙ 模式")); 
    }); 


    server.on("/Metroclock", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay();  
    customThemeEnable = false; 
    ModefirstRun = true;  
    Mode = 24;
    Serial.println(F("高雄捷運 模式")); 
    });


    server.on("/randommode", HTTP_POST, [](AsyncWebServerRequest *request) {   
    display.clearDisplay();  
    customThemeEnable = false;
    Mode = 25; 
    randomMode = random(1, THEME_MODE_MAX + 1);
    Serial.println(F("隨機模式")); 
    }); 

    server.on("/CustTheme", HTTP_POST, [](AsyncWebServerRequest *request) {
      customThemeEnable = true;
      // 重要：讓目前這個小時可以立刻重新判斷
      lastCustomThemeHour = -1;
      Serial.println(F("啟用自訂每小時主題模式"));
      request->send(200, "text/plain", "OK");
      });


    server.on("/customThemeSchedule", HTTP_POST, [](AsyncWebServerRequest *request) {

    if (request->hasArg("schedule")) {
        String scheduleText = request->arg("schedule");

        Serial.print(F("收到自訂主題時間表："));
        Serial.println(scheduleText);

        int hourIndex = 0;
        int startIndex = 0;

        for (int i = 0; i <= scheduleText.length(); i++) {
            if (scheduleText.charAt(i) == ',' || i == scheduleText.length()) {
                if (hourIndex < 24) {
                    String valueText = scheduleText.substring(startIndex, i);
                    int modeValue = valueText.toInt();


                    // 主題數量24
                    if ((modeValue >= 1 && modeValue <= 24) || modeValue == 99) {
                        customThemeSchedule[hourIndex] = modeValue;
                    } else {
                        customThemeSchedule[hourIndex] = 99;
                    }

                    hourIndex++;
                    startIndex = i + 1;
                }
            }
        }

        customThemeEnable = true;
        lastCustomThemeHour = -1;
        
        Serial.println(F("解析後的 24 小時主題表："));
        for (int h = 0; h < 24; h++) {
            Serial.print(h);
            Serial.print(F(": "));
            Serial.println(customThemeSchedule[h]);
        }

EEPROM.write(EEPROM_CUSTOM_ENABLE, customThemeEnable ? 1 : 0);

for (int i = 0; i < 24; i++) {
    EEPROM.write(EEPROM_CUSTOM_TABLE + i, customThemeSchedule[i]);
}

EEPROM.commit();

Serial.println(F("自訂主題時間表已儲存 EEPROM"));

        request->send(200, "text/plain", "OK");
    } else {
        request->send(400, "text/plain", "No schedule data");
    }
   });



    server.on("/clear", HTTP_POST, [](AsyncWebServerRequest *request) {   
    display.clearDisplay();   
    }); 


    server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request) {   
    ESP.restart(); 
    }); 


    server.on("/savedata", HTTP_POST, [](AsyncWebServerRequest *request) {   

         EEPROM.write(EEPROM_MODE, Mode);
         EEPROM.write(EEPROM_COLOR, hue/2);
         EEPROM.write(EEPROM_COLOR_H, hueh/2);
         EEPROM.write(EEPROM_COLOR_M, huem/2);
         EEPROM.write(EEPROM_COLOR_S, hues/2);
         EEPROM.write(EEPROM_COLOR_W, huew/2);
         EEPROM.write(EEPROM_BRIGHTNESS, brightnessNow);
         
         EEPROM.write(EEPROM_START_H, start_H);
         EEPROM.write(EEPROM_START_M, start_M);
         EEPROM.write(EEPROM_END_H, end_H);
         EEPROM.write(EEPROM_END_M, end_M);
         
         EEPROM.write(EEPROM_GIF_NO, random_gif_no);
         EEPROM.write(EEPROM_GIF_COUNT, gifcount);
         EEPROM.write(EEPROM_GIF_DELAY, gifdelay);
         EEPROM.write(EEPROM_RANDOM_MIN, random_min);

         
         EEPROM.put(EEPROM_THEME_A, colorA_565); 
         EEPROM.put(EEPROM_THEME_B, colorB_565);

         EEPROM.write(EEPROM_CUSTOM_ENABLE, customThemeEnable ? 1 : 0);

         for (int i = 0; i < 24; i++) {
         EEPROM.write(EEPROM_CUSTOM_TABLE + i, customThemeSchedule[i]);
         }         
         EEPROM.commit();
         Serial.println(F("儲存資料完成")); 
         savedata();
    }); 



      //解析顏色數值
     server.on("/getColor", HTTP_POST, [](AsyncWebServerRequest *request) {   
     
     //str轉char
     input = request->arg("value");
     strcpy(socketMessage,input.c_str());
    
         
      if(socketMessage[0] == 'h'){//颜色设置
                int hh= ((int)socketMessage[2]-48)*100+((int)socketMessage[3]-48)*10+(int)socketMessage[4]-48;
                Serial.print(F("傳入數值："));
                Serial.println(hh);
                if(hh>=0&&hh<=360){  
                    switch (socketMessage[1])
                    {
                     
                    case 'a':
                        hue = hh;
                        hueh = hh;
                        huem = hh;
                        hues = hh;
                        huew = hh;
                        hueb = hh;
                        break;
                        
                    case 'b':
                        brightnessNow = hh ;
                        display.setBrightness(brightnessNow);
                        break; 
                                                
                    case 'd':
                        hue = hh;
                        break;
                    case 'h':
                        hueh = hh;
                        break;
                    case 'm':
                        huem = hh;
                        break;
                    case 's':
                        hues = hh;
                        break;   
                    case 'w':
                        huew = hh;
                        break;  
                                                                        
                    case 'o':   
                        start_H = hh;
                        Serial.println(start_H);
                        break;
                        
                    case 'p':
                        start_M = hh;
                          Serial.println(start_M);
                        break;
                        
                    case 'q':
                        end_H = hh;
                          Serial.println(end_H);
                        break;
                        
                    case 'r':
                        end_M = hh;
                         Serial.println(end_M);
                        break;

                    case 'g':
                        gifcount = hh;
                        Serial.print(F("設定Gif播放次數: ")); 
                        Serial.println(gifcount);
                        break; 

                    case 'n':
                        random_gif_no = hh;
                        Serial.print(F("設定隨機Gif數量: ")); 
                        Serial.println(random_gif_no);
                        break;                         

                    case 'u':
                        random_min = hh;
                        Serial.print(F("隨機模式切換時間: "));
                        Serial.println(random_min);
                        break;
                           
                    case 'y':
                        gifdelay = hh;
                        Serial.print(F("gif延遲時間: "));
                        Serial.println(gifdelay);                        
                        break;                             
                                                                                      
                    default:
                        break;
                    }
                }
          }
     });  





server.on("/themeColor", HTTP_POST, [](AsyncWebServerRequest *request) {
    // 接收前景顏色 A
    if (request->hasArg("a_val")) {
        colorA_565 = request->arg("a_val").toInt(); // 網頁算好的數字，直接轉 int
        themeA_str = request->arg("a_str");         // 存下 #RRGGBB 字串
    }
    
    // 接收背景顏色 B
    if (request->hasArg("b_val")) {
        colorB_565 = request->arg("b_val").toInt(); // 網頁算好的數字
        themeB_str = request->arg("b_str");         // 存下 #RRGGBB 字串
    }
    
    Serial.printf("顏色更新！A: %d, B: %d\n", colorA_565, colorB_565);
    request->send(200, "text/plain", "OK");
});




    //設定播放GIF 檔名
    server.on("/gifname", HTTP_POST, [](AsyncWebServerRequest *request) {  
    display.clearDisplay();   
    Mode = 50;        
    GIFname = request->arg("gifname");
    //playGif();
     }); 


    //Gif 固定播放
    server.on("/fixedgif", HTTP_POST, [](AsyncWebServerRequest *request) {        
 
    Serial.println(F("固定播放-GIF"));
    GifRandom = false ;
     }); 


    //Gif 隨機播放
    server.on("/randomgif", HTTP_POST, [](AsyncWebServerRequest *request) {        
   
    Serial.println(F("GIF隨機播放"));
    GifRandom = true ;
     }); 


    // 4. WiFi 設定功能
    server.on("/setwifi", HTTP_POST, [](AsyncWebServerRequest *request) {   
        if(request->hasArg("hh2")) setssid = request->arg("hh2");
        if(request->hasArg("mm2")) setpwd = request->arg("mm2");    
        
        save_ssid(); // 確保 Function.ino 裡有此函式
        
        request->send(200, "application/json", "{\"result\":\"ok\"}");
        
        display.clearDisplay();
        display.setCursor(20, 8);
        display.print(F("Saved"));
        // 注意：在非同步回呼中延遲重啟建議使用定時器，這裡先維持你的邏輯
        wait_with_display(1000); 
        display.setCursor(14, 32);
        display.print(F("RE-BOOT"));
        wait_with_display(1000);
        ESP.restart();
    });



server.on("/datasync", HTTP_POST, [](AsyncWebServerRequest *request) {
  String json = "{";
  json += "\"startH\":"  + String(read_start_H)  + ",";
  json += "\"startM\":"  + String(read_start_M)  + ",";
  json += "\"endH\":"  + String(read_end_H)  + ",";
  json += "\"endM\":"  + String(read_end_M)  + ",";

  json += "\"readssid\":\"" + String(readssid) + "\",";
  json += "\"gifcount\":"  + String(gifcount)  + ",";
  json += "\"random_gif_no\":"  + String(random_gif_no)  + ",";
  json += "\"random_min\":"  + String(random_min)  + ",";
  json += "\"gifdelay\":"  + String(gifdelay)  + ",";
  json += "\"gifdelay\":"  + String(gifdelay)  + ",";      

  json += "\"themeA\":\"" + themeA_str + "\","; 
  json += "\"themeB\":\"" + themeB_str + "\",";
  json += "\"customThemeEnable\":" + String(customThemeEnable ? 1 : 0) + ",";
  json += "\"customThemeSchedule\":\"";
  for (int i = 0; i < 24; i++) {
     json += String(customThemeSchedule[i]);
     if (i < 23) json += ",";
     }
  json += "\",";               
  json += "\"hue\":"  + String(hue)  + ",";
  json += "\"hueh\":" + String(hueh) + ",";
  json += "\"huem\":" + String(huem) + ",";
  json += "\"hues\":" + String(hues) + ",";
  json += "\"huew\":" + String(huew) + ",";
  json += "\"hueb\":" + String(hueb);
  json += "}";

  request->send(200, "application/json", json);
}); 


// =====================================================
// LittleFS 手動上傳頁
// 新 MCU 第一次使用：
// http://192.168.4.1/upload
// =====================================================
server.on("/upload", HTTP_GET, handleFSUpdatePage);

server.on(
    "/upload",
    HTTP_POST,
    [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response =
            request->beginResponse(
                200,
                "text/html",
                "<!DOCTYPE html>"
                "<html><head>"
                "<meta charset='UTF-8'>"
                "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                "<title>Upload Success</title>"
                "<style>"
                "body{font-family:Arial;background:#111;color:#eee;padding:20px;}"
                ".box{max-width:480px;margin:auto;background:#222;padding:20px;border-radius:12px;}"
                "a{display:block;background:#2196f3;color:white;padding:12px;margin:12px 0;text-align:center;text-decoration:none;border-radius:8px;}"
                "</style>"
                "</head><body><div class='box'>"
                "<h2>Upload Success</h2>"
                "<p>檔案已上傳到 LittleFS。</p>"
                "<a href='/upload'>繼續上傳其他檔案</a>"
                "<a href='/fs'>開啟檔案管理頁</a>"
                "<a href='/'>回首頁</a>"
                "</div></body></html>"
            );

        response->addHeader("Connection", "close");
        request->send(response);
    },
    [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        static File uploadFile;

        if (!index) {
            if (!filename.startsWith("/")) {
                filename = "/" + filename;
            }

            Serial.print("LittleFS upload start: ");
            Serial.println(filename);

            uploadFile = LittleFS.open(filename, "w");

            if (!uploadFile) {
                Serial.println("LittleFS upload open failed");
                return;
            }
        }

        if (len && uploadFile) {
            uploadFile.write(data, len);
        }

        if (final) {
            if (uploadFile) {
                uploadFile.close();
            }

            Serial.print("LittleFS upload done: ");
            Serial.print(filename);
            Serial.print(" size=");
            Serial.println(index + len);
        }
    }
);
  

    
}





void handleFSUpdatePage(AsyncWebServerRequest *request) {
    String html;

    html += "<!DOCTYPE html>";
    html += "<html lang='zh-TW'>";
    html += "<head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>LittleFS Upload</title>";
    html += "<style>";
    html += "body{font-family:Arial;background:#111;color:#eee;padding:20px;}";
    html += ".box{max-width:480px;margin:auto;background:#222;padding:20px;border-radius:12px;}";
    html += "input,button{width:100%;padding:12px;margin-top:12px;font-size:16px;box-sizing:border-box;}";
    html += "button{background:#2196f3;color:white;border:0;border-radius:8px;}";
    html += "a{color:#8cc8ff;display:block;margin-top:12px;}";
    html += "</style>";
    html += "</head>";
    html += "<body>";
    html += "<div class='box'>";
    html += "<h2>LittleFS 檔案上傳</h2>";
    html += "<p>第一次使用請先上傳：</p>";
    html += "<ol>";
    html += "<li>fs.html</li>";
    html += "<li>index.html</li>";
    html += "</ol>";
    html += "<form method='POST' action='/upload' enctype='multipart/form-data'>";
    html += "<input type='file' name='file'>";
    html += "<button type='submit'>上傳到 LittleFS</button>";
    html += "</form>";
    html += "<a href='/'>回首頁</a>";
    html += "<a href='/fs'>開啟檔案管理頁 /fs</a>";
    html += "<a href='/status'>查看 LittleFS 狀態 /status</a>";
    html += "</div>";
    html += "</body>";
    html += "</html>";

    request->send(200, "text/html", html);
}
