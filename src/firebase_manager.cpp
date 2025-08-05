#include "firebase_manager.h"

FirebaseManager::FirebaseManager() {
  timeClient = nullptr;
  isConnected = false;
  signupOK = false;
  lastConnectionCheck = 0;
  lastDataSend = 0;
}

FirebaseManager::~FirebaseManager() {
  if (timeClient) {
    delete timeClient;
  }
}

bool FirebaseManager::begin() {
  Serial.println("FirebaseManager: Initializing...");
  
  // Connect to WiFi first
  if (!connectToWiFi()) {
    Serial.println("FirebaseManager: WiFi connection failed");
    return false;
  }
  
  // Initialize NTP client
  timeClient = new NTPClient(ntpUDP, NTP_SERVER);
  timeClient->begin();
  timeClient->setTimeOffset(TIME_OFFSET);
  timeClient->update();
  
  Serial.println("FirebaseManager: NTP client initialized");
  
  // Initialize Firebase
  if (!initializeFirebase()) {
    Serial.println("FirebaseManager: Firebase initialization failed");
    return false;
  }
  
  Serial.println("FirebaseManager: Initialized successfully");
  return true;
}

bool FirebaseManager::connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("FirebaseManager: Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.printf("FirebaseManager: WiFi connected - IP: %s\n", WiFi.localIP().toString().c_str());
    return true;
  } else {
    Serial.println("\nFirebaseManager: WiFi connection failed");
    return false;
  }
}

bool FirebaseManager::initializeFirebase() {
  // Configure Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  
  // Attempt anonymous sign up
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("FirebaseManager: Firebase SignUp OK");
    signupOK = true;
  } else {
    Serial.printf("FirebaseManager: SignUp Error - %s\n", config.signer.signupError.message.c_str());
    signupOK = false;
  }
  
  // Set token status callback
  config.token_status_callback = tokenStatusCallback;
  
  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  // Test connection
  delay(1000);
  isConnected = Firebase.ready();
  
  return isConnected && signupOK;
}

void FirebaseManager::checkConnection() {
  unsigned long currentTime = millis();
  
  // Check connection every 30 seconds
  if (currentTime - lastConnectionCheck > 30000) {
    lastConnectionCheck = currentTime;
    
    bool wifiStatus = (WiFi.status() == WL_CONNECTED);
    bool firebaseStatus = Firebase.ready();
    
    if (wifiStatus != isWiFiConnected() || firebaseStatus != isConnected) {
      Serial.printf("FirebaseManager: Connection status changed - WiFi: %s, Firebase: %s\n",
                    wifiStatus ? "Connected" : "Disconnected",
                    firebaseStatus ? "Connected" : "Disconnected");
    }
    
    isConnected = firebaseStatus;
    
    // Attempt reconnection if needed
    if (!wifiStatus) {
      Serial.println("FirebaseManager: Attempting WiFi reconnection...");
      WiFi.reconnect();
    }
  }
}

bool FirebaseManager::isReady() const {
  return isConnected && signupOK && (WiFi.status() == WL_CONNECTED);
}

bool FirebaseManager::isWiFiConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

bool FirebaseManager::isFirebaseConnected() const {
  return isConnected && Firebase.ready();
}

String createPath(const char* suffix) {
    String path = String(FB_SENSORS_PATH);
    path += suffix;
    return path;
}

String createPath2(const String& suffix) {
    String path = String(FB_EMERGENCY_PATH);
    path += suffix;
    return path;
}

bool FirebaseManager::sendSensorData(const SensorData& data, int crashSeverity, bool crashDetected) {
  if (!isReady()) return false;
  
  bool success = true;
  
  success &= sendFloat(createPath("accelX"), data.accelX);
  success &= sendFloat(createPath("accelY"), data.accelY);
  success &= sendFloat(createPath("accelZ"), data.accelZ);
  success &= sendFloat(createPath("gyroX"), data.gyroX);
  success &= sendFloat(createPath("gyroY"), data.gyroY);
  success &= sendFloat(createPath("gyroZ"), data.gyroZ);
  success &= sendFloat(createPath("distance"), data.distance);
  success &= sendInt(createPath("vibration"), data.vibration);
  success &= sendFloat(createPath("latitude"), data.latitude);
  success &= sendFloat(createPath("longitude"), data.longitude);
  success &= sendInt(createPath("crashSeverity"), crashSeverity);
  success &= sendBool(createPath("crashDetected"), crashDetected);
  success &= sendInt(createPath("timestamp"), getCurrentTimestamp());
  
  if (success) {
    lastDataSend = millis();
  }
  
  return success;
}

bool FirebaseManager::sendEmergencyAlert(const SensorData& data, int severity) {
  if (!isReady()) return false;
  
  // Create emergency data structure
  FirebaseJson emergencyData;
  emergencyData.set("timestamp", getCurrentTimestamp());
  emergencyData.set("severity", severity);
  emergencyData.set("latitude", data.latitude);
  emergencyData.set("longitude", data.longitude);
  
  // Calculate magnitudes
  float accelMagnitude = sqrt(data.accelX*data.accelX + 
                             data.accelY*data.accelY + 
                             data.accelZ*data.accelZ);
  float gyroMagnitude = sqrt(data.gyroX*data.gyroX + 
                            data.gyroY*data.gyroY + 
                            data.gyroZ*data.gyroZ);
  
  emergencyData.set("accelMagnitude", accelMagnitude);
  emergencyData.set("gyroMagnitude", gyroMagnitude);
  emergencyData.set("distance", data.distance);
  emergencyData.set("vibration", data.vibration);
  
  String emergencyPath = createPath2(String(getCurrentTimestamp()));
  
  if (Firebase.RTDB.setJSON(&fbdo, emergencyPath, &emergencyData)) {
    Serial.println("FirebaseManager: Emergency alert sent successfully");
    return true;
  } else {
    Serial.printf("FirebaseManager: Emergency alert failed - %s\n", fbdo.errorReason().c_str());
    return false;
  }
}

bool FirebaseManager::updateCrashStatus(int severity, bool emergencyActive) {
  if (!isReady()) return false;
  
  bool success = true;
  success &= Firebase.RTDB.setInt(&fbdo, FB_CRASH_STATUS_PATH, severity);
  success &= Firebase.RTDB.setBool(&fbdo, FB_EMERGENCY_ACTIVE_PATH, emergencyActive);
  
  return success;
}

bool FirebaseManager::sendFloat(const String& path, float value) {
  if (!isReady()) return false;
  
  if (Firebase.RTDB.setFloat(&fbdo, path, value)) {
    return true;
  } else {
    Serial.printf("FirebaseManager: Failed to send float to %s - %s\n", 
                  path.c_str(), fbdo.errorReason().c_str());
    return false;
  }
}

bool FirebaseManager::sendInt(const String& path, int value) {
  if (!isReady()) return false;
  
  if (Firebase.RTDB.setInt(&fbdo, path, value)) {
    return true;
  } else {
    Serial.printf("FirebaseManager: Failed to send int to %s - %s\n", 
                  path.c_str(), fbdo.errorReason().c_str());
    return false;
  }
}

bool FirebaseManager::sendBool(const String& path, bool value) {
  if (!isReady()) return false;
  
  if (Firebase.RTDB.setBool(&fbdo, path, value)) {
    return true;
  } else {
    Serial.printf("FirebaseManager: Failed to send bool to %s - %s\n", 
                  path.c_str(), fbdo.errorReason().c_str());
    return false;
  }
}

bool FirebaseManager::sendString(const String& path, const String& value) {
  if (!isReady()) return false;
  
  if (Firebase.RTDB.setString(&fbdo, path, value)) {
    return true;
  } else {
    Serial.printf("FirebaseManager: Failed to send string to %s - %s\n", 
                  path.c_str(), fbdo.errorReason().c_str());
    return false;
  }
}

unsigned long FirebaseManager::getCurrentTimestamp() {
  if (timeClient) {
    return timeClient->getEpochTime();
  }
  return millis() / 1000; // Fallback to system time
}

void FirebaseManager::updateTime() {
  if (timeClient) {
    timeClient->update();
  }
}

void FirebaseManager::reconnect() {
  Serial.println("FirebaseManager: Attempting reconnection...");
  
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }
  
  if (WiFi.status() == WL_CONNECTED && !Firebase.ready()) {
    Firebase.begin(&config, &auth);
    delay(1000);
    isConnected = Firebase.ready();
  }
}

void FirebaseManager::handleConnection() {
  checkConnection();
  
  // Auto-reconnect if needed
  if (!isReady()) {
    static unsigned long lastReconnectAttempt = 0;
    if (millis() - lastReconnectAttempt > 60000) { // Try every minute
      lastReconnectAttempt = millis();
      reconnect();
    }
  }
}

String FirebaseManager::getConnectionInfo() {
  String info = "WiFi: ";
  info += isWiFiConnected() ? "Connected" : "Disconnected";
  info += " | Firebase: ";
  info += isFirebaseConnected() ? "Connected" : "Disconnected";
  info += " | Ready: ";
  info += isReady() ? "Yes" : "No";
  return info;
}

String FirebaseManager::getLastError() {
  return fbdo.errorReason();
}

bool FirebaseManager::testConnection() {
  if (!isReady()) return false;
  
  String testPath = createPath("test");
  bool result = Firebase.RTDB.setString(&fbdo, testPath, "connection_test");
  
  if (result) {
    Serial.println("FirebaseManager: Connection test successful");
  } else {
    Serial.printf("FirebaseManager: Connection test failed - %s\n", fbdo.errorReason().c_str());
  }
  
  return result;
}

bool FirebaseManager::testDataSend() {
  if (!isReady()) return false;
  
  SensorData testData;
  memset(&testData, 0, sizeof(testData));
  testData.accelX = 1.0;
  testData.accelY = 0.0;
  testData.accelZ = 0.0;
  testData.timestamp = millis();
  
  return sendSensorData(testData, NO_CRASH, false);
}