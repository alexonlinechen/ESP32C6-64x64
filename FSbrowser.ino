#include "config.h"
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

// 確保編譯器在 FS_Init 之前看過這個函式
void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);


File uploadFile; // 用於檔案上傳的 File 物件 (應宣告為全域變數或靜態變數)



// 遞迴建立目錄輔助函式
void _createDirRecursive(String path) {
    if (path == "/" || path == "" || path == ".") return;
    if (!path.startsWith("/")) path = "/" + path;
    
    // 如果路徑結尾是 / 則移除，避免分割出空字串
    if (path.endsWith("/")) path = path.substring(0, path.length() - 1);

    int start = 1;
    while (true) {
        int end = path.indexOf('/', start);
        if (end == -1) {
            // 到達最後一層路徑
            if (!LittleFS.exists(path)) LittleFS.mkdir(path);
            break;
        }
        String sub = path.substring(0, end);
        if (sub != "" && !LittleFS.exists(sub)) {
            LittleFS.mkdir(sub);
        }
        start = end + 1;
    }
}


// ===================================
// MARK: - 檔案管理員輔助函式 (File Manager Helpers)
// ===================================

// 獲取檔案系統狀態並回傳 JSON 格式 (已修正為 'total' 和 'used')
String handleFsStatus() {
  String json = "{";
  json += "\"total\":" + String(LittleFS.totalBytes());
  json += ",\"used\":" + String(LittleFS.usedBytes());
  json += "}";
  return json;
}

// 獲取檔案列表並回傳 JSON 格式
String handleFileList(const char* path) {
    String output = "[";
    File root = LittleFS.open(path);
    if (!root) {
        return "[]"; // 目錄不存在或無法開啟
    }

    File file = root.openNextFile();

    while (file) {
        if (output != "[") output += ",";
        output += "{";
        output += "\"type\":\"" + String(file.isDirectory() ? "dir" : "file") + "\"";
        
        String filename = file.name();
        // 移除路徑前綴，僅保留檔案名或相對路徑
        if (filename.startsWith(path)) {
            filename.remove(0, String(path).length());
        }
        
        // 確保根目錄列出時，名稱不會是空字串
        if (filename.length() == 0 && String(path) == "/") {
            filename = file.isDirectory() ? "/" : file.name();
        }
        
        output += ",\"name\":\"" + filename + "\"";
        output += ",\"size\":" + String(file.size());
        output += "}";
        file = root.openNextFile();
    }
    output += "]";
    return output;
}

// 檔案上傳事件處理函式 (用於 POST /edit)
void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
        // 上傳開始
        Serial.printf("Upload Start: %s\n", filename.c_str());
        // 確保路徑以 / 開頭
        if (!filename.startsWith("/")) {
            filename = "/" + filename;
        }
        // 使用 'w' 模式開啟檔案寫入
        uploadFile = LittleFS.open(filename, "w");
        if (!uploadFile) {
            Serial.println("Upload failed to open file");
        }
    }
    if (len && uploadFile) {
        uploadFile.write(data, len);
    }
    if (final) {
        if (uploadFile) {
            uploadFile.close();
            Serial.printf("Upload End: %s, Size: %u\n", filename.c_str(), index + len);
            request->send(200, "text/plain", "Upload Successful");
        } else {
             request->send(500, "text/plain", "Upload Failed (File Error)");
        }
    }
}




void FS_Init(){

 // F. 靜態檔案服務 (處理所有未被上述 API 捕捉到的 GET 請求，用於下載或顯示文件內容)
    server.serveStatic("/", LittleFS, "/");

    // ===================================
    // 檔案管理器 API 路由設定
    // ===================================
    
    // A. 服務檔案管理器 HTML 頁面 (假設 fs.html 在 LittleFS 根目錄)
server.on("/fs", HTTP_GET, [](AsyncWebServerRequest *request){
    if (LittleFS.exists("/fs.html")) {
        request->send(LittleFS, "/fs.html", "text/html");
    } else {
        String html;
        html += "<!DOCTYPE html><html><head>";
        html += "<meta charset='UTF-8'>";
        html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
        html += "<title>fs.html Missing</title>";
        html += "<style>";
        html += "body{font-family:Arial;background:#111;color:#eee;padding:20px;}";
        html += ".box{max-width:480px;margin:auto;background:#222;padding:20px;border-radius:12px;}";
        html += "a{display:block;background:#2196f3;color:white;padding:12px;margin:12px 0;text-align:center;text-decoration:none;border-radius:8px;}";
        html += "</style></head><body><div class='box'>";
        html += "<h2>fs.html 尚未上傳</h2>";
        html += "<p>請先到 /upload 上傳 fs.html。</p>";
        html += "<a href='/upload'>前往上傳頁</a>";
        html += "<a href='/'>回首頁</a>";
        html += "</div></body></html>";

        request->send(200, "text/html", html);
    }
});
    
    // B. 取得檔案系統狀態 (/status)
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "application/json", handleFsStatus());
    });
    
    // C. 取得檔案列表 (/list?dir=/path)
    server.on("/list", HTTP_GET, [](AsyncWebServerRequest *request){
        String path = "/";
        if (request->hasArg("dir")) {
            path = request->arg("dir");
        }
        request->send(200, "application/json", handleFileList(path.c_str()));
    });
    
    // D. 檔案操作路由 (POST: 上傳, DELETE: 刪除, PUT: 創建/重命名/移動)
    
    // 1. POST 處理 (檔案上傳)
    server.on("/edit", HTTP_POST, 
        // 1. 請求完成處理
        [](AsyncWebServerRequest *request){
            request->send(200, "text/plain", "Upload Success");
        }, 
        // 2. 檔案上傳事件處理 (核心上傳邏輯)
        handleFileUpload);
        
    // 2. DELETE 處理 (刪除檔案/目錄)
    server.on("/edit", HTTP_DELETE, [](AsyncWebServerRequest *request){
        String path = request->arg("path"); // 假設路徑由 path 參數傳入
        
        if (!request->hasArg("path")) {
             return request->send(400, "text/plain", "Missing 'path' argument for DELETE.");
        }
        
        if (LittleFS.remove(path)) {
            request->send(200, "text/plain", "Delete Success");
        } else {
            request->send(500, "text/plain", "Delete Failed");
        }
    });

    // 3. PUT 處理 (建立檔案 / 建立資料夾 / 重新命名 / 移動)
    server.on("/edit", HTTP_PUT, [](AsyncWebServerRequest *request) {

        if (!request->hasArg("path")) {
            return request->send(400, "text/plain", "Missing path argument");
        }

        String action = request->hasArg("action") ? request->arg("action") : "create";
        String path   = request->arg("path");

        path.trim();
        if (!path.startsWith("/")) path = "/" + path;

        // -------------------------------------------------
        // RENAME / MOVE
        // fs.html 會傳：action=rename, path=舊路徑, dest=新路徑
        // -------------------------------------------------
        if (action == "rename") {

            if (!request->hasArg("dest")) {
                return request->send(400, "text/plain", "Missing dest argument");
            }

            String dest = request->arg("dest");
            dest.trim();

            if (!dest.startsWith("/")) dest = "/" + dest;

            // 根目錄禁止重新命名
            if (path == "/" || dest == "/") {
                return request->send(400, "text/plain", "Cannot rename root directory");
            }

            if (path == dest) {
                return request->send(400, "text/plain", "Source and destination are the same");
            }

            if (!LittleFS.exists(path)) {
                return request->send(404, "text/plain", "Source not found: " + path);
            }

            // 避免意外覆蓋既有檔案
            if (LittleFS.exists(dest)) {
                return request->send(409, "text/plain", "Destination already exists: " + dest);
            }

            // 如果是移到其他資料夾，先確保目的地父目錄存在
            int lastSlash = dest.lastIndexOf('/');
            if (lastSlash > 0) {
                _createDirRecursive(dest.substring(0, lastSlash));
            }

            Serial.printf("Rename: %s -> %s\n", path.c_str(), dest.c_str());

            if (LittleFS.rename(path, dest)) {
                // fs.html 的 onOperationComplete 會利用 responseText
                // 更新來源資料夾，因此回傳舊檔案的父路徑
                String oldParent = path.substring(0, path.lastIndexOf('/'));
                if (oldParent.length() == 0) oldParent = "/";

                return request->send(200, "text/plain", oldParent);
            }

            return request->send(500, "text/plain", "Rename Failed");
        }

        // -------------------------------------------------
        // CREATE
        // 沒有 action 或 action=create 時維持原本新增功能
        // -------------------------------------------------

        // 用結尾 / 判斷資料夾，比用「有沒有 .」更可靠
        bool createDirectory = path.endsWith("/");

        if (createDirectory) {
            // 去掉尾端 /，LittleFS mkdir 使用標準目錄路徑
            while (path.length() > 1 && path.endsWith("/")) {
                path.remove(path.length() - 1);
            }

            _createDirRecursive(path);

            if (LittleFS.exists(path)) {
                return request->send(200, "text/plain", "Directory Created");
            }

            return request->send(500, "text/plain", "Directory Creation Failed");
        }

        // 建立檔案
        int lastSlash = path.lastIndexOf('/');
        if (lastSlash > 0) {
            _createDirRecursive(path.substring(0, lastSlash));
        }

        // 不要用 w 去碰已存在的檔案，避免誤清成 0 KB
        if (LittleFS.exists(path)) {
            return request->send(409, "text/plain", "File already exists: " + path);
        }

        File file = LittleFS.open(path, "w");
        if (!file) {
            return request->send(500, "text/plain", "File Creation Failed");
        }

        file.close();
        return request->send(200, "text/plain", "File Created");
    });

    // 【新增】E. 檔案下載路由 (/download?path=...)
// 【修正後的檔案下載路由】E. 檔案下載路由 (/download?path=...)
server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
    String path = request->arg("path");
    
    if (!request->hasArg("path")) {
        return request->send(400, "text/plain", "Missing 'path' argument for download.");
    }

    if (!LittleFS.exists(path)) {
        return request->send(404, "text/plain", "File Not Found: " + path);
    }

    // 【修正點：改用 request->beginResponse】
    // 步驟 1: 使用 beginResponseFile 建立 response object (這會回傳一個指標)
    // 參數 3: MIME type (設為空字串讓 send 函式自動判斷)
    // 參數 4: 是否強制下載 (設為 true)
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, path, String(), true);
    
    // 步驟 2: 設置 Content-Disposition 標頭以強制瀏覽器下載
    String filename = path.substring(path.lastIndexOf('/') + 1); 
    String header = "attachment; filename=" + filename;
    response->addHeader("Content-Disposition", header);
    
    // 步驟 3: 傳送 response 並使用 return 確保立即結束
    return request->send(response);
});

}
