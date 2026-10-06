HomeTrash 測試版說明
====================

這一版已加入「住家專用垃圾車」Optional Module。
它不是 Mode，不會改掉目前 Mode；只在啟用時暫時覆蓋 64x64 畫面，結束後自然回原 Mode。

新增檔案
--------
HomeTrash.ino           垃圾車排程 / HTTP / JSON / 顯示 / Web API
HomeTrashData.h         API 地址 -> 固定點陣 ID 的對照表
data/trash.html         獨立垃圾車設定頁
data/trash.bin          已先產生的地址中文字點陣
                        （神農路、水管路、美庄路、成功路、後庄里、成功路209巷、
                         民成街、民忠街、民忠一街、成功路196號、民成街8號）
data/trash_test.json    可放到你自己的伺服器當假 JSON 範例

原專案只改了兩個地方
--------------------
config.h
  新增 FEATURE_HOME_TRASH = 1

C6pixel.ino
  setup() 呼叫 homeTrashSetup()
  loop() 在 SwitchMode() 前檢查 HomeTrash override

因此要做「不含垃圾車的通用版」時：
1. config.h 把 FEATURE_HOME_TRASH 改成 0
2. 可直接刪除 HomeTrash.ino、HomeTrashData.h、data/trash.html、data/trash.bin、data/trash_test.json
   主模式編號與原本 ClockMode 都不必改。

目前如何測試
------------
1. 編譯/燒錄 C6pixel。
2. 記得把 data 目錄重新上傳到 LittleFS，至少要包含：
   trash.html
   trash.bin
3. 把 data/trash_test.json 放到你的 HTTP/HTTPS 伺服器。
4. 瀏覽器開：http://ESP32的IP/trash
5. 勾選：
   - 啟用垃圾車資訊
   - 強制測試模式
   - 使用測試 JSON URL
6. 填入你的假 JSON 網址。
7. 車號保持 KEM-0621。
8. 按「儲存設定」或「立即抓取一次」。

測試 JSON 建議格式
------------------
{
  "data": [
    {
      "car": "KEM-0621",
      "location": "高雄市大寮區 高61 成功路"
    }
  ]
}

只要改 location，就能測其他固定地址，例如：
高雄市鳥松區 神農路
高雄市鳥松區 水管路
高雄市鳥松區 高61 美庄路
高雄市大寮區 高61 成功路
高雄市大寮區 後庄里
後庄里 成功路209巷2-6號前
高雄市大寮區 民成街
高雄市大寮區 民忠街
高雄市大寮區 民忠一街
後庄里 成功路196號
後庄里 民成街8號（8-147號）
高雄市大寮區 美庄路

64x64 畫面
-----------
上方目前先放程式畫出的垃圾車 placeholder。
你之後可直接修改 HomeTrash.ino 裡的 homeTrashDrawTruckPlaceholder()，換成自己的點陣。

下方已先做一套測試中文字點陣，可直接驗證地址比對與跑馬燈。
短地址會置中；寬度超過 64px 會自動跑馬燈。

正式上線
--------
在 /trash 頁面：
1. 取消「強制測試模式」
2. 取消「使用測試 JSON URL」
3. 設定正式時間，例如 18:25 ~ 19:00
4. 保留「星期三不顯示」
5. 啟用垃圾車資訊

此時只有排程時間內會抓垃圾車 API 並覆蓋畫面；結束後自動回原本 Mode。

注意
----
- HTTPS 目前採 setInsecure()，等同你舊 PHP 關閉 SSL 驗證的概念；功能確認後可改 CA 驗證。
- HTTP 抓取放在獨立 FreeRTOS task，避免等待 API 時主 loop 整段卡住。
- API 失敗時會保留最後一次成功的位置。
- 遇到沒有收錄的新地址，Serial 會印：[HomeTrash] Unknown location: ...
  之後只需要更新 HomeTrashData.h + trash.bin，不用改抓 API 的程式。
