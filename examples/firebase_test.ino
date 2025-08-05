/*
 * Firebase Connection Test
 * 
 * This example tests Firebase connectivity without the full system.
 * Update your WiFi and Firebase credentials before running.
 */

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// Update these with your credentials
#define WIFI_SSID "Your_WiFi_SSID"
#define WIFI_PASSWORD "Your_WiFi_Password"
#define API_KEY "Your_Firebase_API_Key"
#define DATABASE_URL "Your_Firebase_Database_URL"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Firebase Connection Test");
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  
  // Configure Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  
  // Sign up
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase signup OK");
    signupOK = true;
  } else {
    Serial.printf("Firebase signup failed: %s\n", config.signer.signupError.message.c_str());
  }
  
  config.token_status_callback = tokenStatusCallback;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Firebase.ready() && signupOK) {
    Serial.println("Testing Firebase write...");
    
    // Test writing data
    if (Firebase.RTDB.setFloat(&fbdo, "test/temperature", 25.6)) {
      Serial.println("Temperature write successful");
    } else {
      Serial.println("Temperature write failed");
      Serial.println(fbdo.errorReason());
    }
    
    if (Firebase.RTDB.setInt(&fbdo, "test/humidity", 60)) {
      Serial.println("Humidity write successful");
    } else {
      Serial.println("Humidity write failed");
      Serial.println(fbdo.errorReason());
    }
    
    if (Firebase.RTDB.setString(&fbdo, "test/status", "online")) {
      Serial.println("Status write successful");
    } else {
      Serial.println("Status write failed");
      Serial.println(fbdo.errorReason());
    }
    
    // Test reading data
    if (Firebase.RTDB.getFloat(&fbdo, "test/temperature")) {
      if (fbdo.dataType() == "float") {
        float temp = fbdo.floatData();
        Serial.printf("Read temperature: %.2f\n", temp);
      }
    } else {
      Serial.println("Temperature read failed");
      Serial.println(fbdo.errorReason());
    }
    
    Serial.println("Test complete. Waiting 10 seconds...\n");
    delay(10000);
  } else {
    Serial.println("Firebase not ready or signup failed");
    delay(1000);
  }
}