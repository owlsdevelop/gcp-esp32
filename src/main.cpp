#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "SPIFFS.h"

// --- Wi-Fi設定 ---
const char* ssid = "<YOUR_SSID>";
const char* password = "<YOUR_PASSWORD>";

// --- GCS設定 ---
// ステップ1で生成した署名付きURLを貼り付ける
const char* signedUrl = "<YOUR_SIGNED_URL>";

void setup() {
  Serial.begin(115200);
  delay(1000);

  // --- 1. SPIFFSの初期化とファイル作成 ---
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  Serial.println("SPIFFS mounted successfully.");

  // "a.txt" を書き込みモードで開く
  File file = SPIFFS.open("/a.txt", "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  
  // ファイルに "aaa" と書き込む
  if (file.print("aaa")) {
    Serial.println("File '/a.txt' was written with content 'aaa'");
  } else {
    Serial.println("File write failed");
  }
  file.close();


  // --- 2. Wi-Fi接続 ---
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  // --- 3. 署名付きURLへファイルをアップロード ---
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    Serial.print("Uploading file to GCS...");

    // SPIFFSからファイルを読み込みモードで再度開く
    File uploadFile = SPIFFS.open("/a.txt", "r");
    if(!uploadFile){
        Serial.println("Failed to open file for reading");
        return;
    }

    // HTTPリクエストを開始
    http.begin(signedUrl);
    // Content-Typeヘッダーを設定
    http.addHeader("Content-Type", "text/plain");

    // PUTリクエストを送信 (ファイルの内容を送信)
    int httpResponseCode = http.PUT(uploadFile.readString());

    if (httpResponseCode > 0) {
      Serial.printf(" [DONE]\n");
      Serial.printf("HTTP Response code: %d\n", httpResponseCode);
      if (httpResponseCode == 200) {
          Serial.println("File uploaded successfully!");
      }
    } else {
      Serial.printf(" [FAILED]\n");
      Serial.printf("HTTP Error code: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    // リソースを解放
    uploadFile.close();
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }

  // SPIFFSをアンマウント
  SPIFFS.end();
}

void loop() {
  // 何もしない
}