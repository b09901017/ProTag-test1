#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// 1. Wi-Fi 設定
#define WIFI_SSID "HTC U23 pro"
#define WIFI_PASSWORD "htcu23pro"

// 2. Firebase 設定
#define API_KEY "AIzaSyATnKODPUhALae4W2eKS87DnGYoWp1qfTQ" // 其實這個庫用 Database Secret 比較簡單，下面用 Secret 示範
#define DATABASE_URL "protag-5888d-default-rtdb.firebaseio.com" //"你的資料庫網址 (不含 https:// 和結尾的 /)" 
#define DATABASE_SECRET "77VkPq0aUzwE8TF8zg4Hivsz1i97LvuA4TgLKPEH"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

const int hallSensorPin = 34;
int safeMin = 1600;
int safeMax = 2200;
int lastStatus = -1; // 記錄狀態避免重複傳送

void setup() {
  Serial.begin(115200);

  // 連接 Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  // 連接 Firebase
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // 簡單的霍爾感測邏輯
  int rawValue = analogRead(hallSensorPin);
  int currentStatus = 0; // 0: 安全, 1: 警報

  if (rawValue > safeMin && rawValue < safeMax) {
    currentStatus = 1; // 磁鐵分離 (警報)
  } else {
    currentStatus = 0; // 磁鐵吸住 (安全)
  }

  // 只有狀態改變時才上傳到資料庫
  if (currentStatus != lastStatus) {
    lastStatus = currentStatus;
    
    // 上傳到 Firebase 的 "/alarm_status" 路徑
    if (Firebase.RTDB.setInt(&fbdo, "/alarm_status", currentStatus)) {
      Serial.print("狀態已更新為: ");
      Serial.println(currentStatus);
    } else {
      Serial.print("上傳失敗: ");
      Serial.println(fbdo.errorReason());
    }
    
    delay(200); // 防彈跳
  }
  
  delay(100);
}