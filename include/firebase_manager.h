#ifndef FIREBASE_MANAGER_H
#define FIREBASE_MANAGER_H

#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

class FirebaseManager {
private:
  FirebaseData fbdo;
  FirebaseAuth auth;
  FirebaseConfig config;
  WiFiUDP ntpUDP;
  NTPClient* timeClient;
  
  bool isConnected;
  bool signupOK;
  unsigned long lastConnectionCheck;
  unsigned long lastDataSend;

  // Helper functions
  bool connectToWiFi();
  bool initializeFirebase();
  void checkConnection();

public:
  FirebaseManager();
  ~FirebaseManager();
  
  // Initialize WiFi and Firebase
  bool begin();
  
  // Connection status
  bool isReady() const;
  bool isWiFiConnected() const;
  bool isFirebaseConnected() const;
  
  // Send sensor data
  bool sendSensorData(const SensorData& data, int crashSeverity, bool crashDetected);
  
  // Send emergency alert
  bool sendEmergencyAlert(const SensorData& data, int severity);
  
  // Update crash status
  bool updateCrashStatus(int severity, bool emergencyActive);
  
  // Send individual sensor values
  bool sendFloat(const String& path, float value);
  bool sendInt(const String& path, int value);
  bool sendBool(const String& path, bool value);
  bool sendString(const String& path, const String& value);
  
  // Get current timestamp
  unsigned long getCurrentTimestamp();
  
  // Update NTP time
  void updateTime();
  
  // Connection management
  void reconnect();
  void handleConnection();
  
  // Get connection info
  String getConnectionInfo();
  String getLastError();
  
  // Test functions
  bool testConnection();
  bool testDataSend();
};

#endif // FIREBASE_MANAGER_H