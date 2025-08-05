#include <Arduino.h>
#include "config.h"
#include "sensor_manager.h"
#include "crash_detector.h"
#include "firebase_manager.h"

// Global objects
SensorManager sensors;
CrashDetector crashDetector;
FirebaseManager firebase;

// Global variables
SensorData currentData;
CrashDetectionConfig crashConfig;
unsigned long lastSensorRead = 0;
unsigned long lastFirebaseSend = 0;
unsigned long lastDebugPrint = 0;
int currentCrashSeverity = NO_CRASH;
bool systemInitialized = false;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("\n=== ESP32 Crash Detection System ===");
  Serial.println("Initializing...");
  
  // Initialize sensors
  Serial.println("Initializing sensors...");
  if (!sensors.begin()) {
    Serial.println("ERROR: Failed to initialize sensors!");
    while (1) {
      delay(1000);
      Serial.println("System halted due to sensor initialization failure");
    }
  }
  Serial.println("✓ Sensors initialized successfully");
  
  // Initialize crash detector
  Serial.println("Initializing crash detector...");
  crashDetector.begin(crashConfig);
  Serial.println("✓ Crash detector initialized");
  
  // Initialize Firebase connection
  Serial.println("Initializing Firebase connection...");
  if (!firebase.begin()) {
    Serial.println("WARNING: Firebase initialization failed!");
    Serial.println("System will continue without cloud connectivity");
  } else {
    Serial.println("✓ Firebase connected successfully");
  }
  
  // System calibration
  Serial.println("Calibrating sensors...");
  sensors.performCalibration();
  delay(2000);
  
  Serial.println("=== System Ready ===");
  Serial.println("Monitoring for crashes...\n");
  systemInitialized = true;
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Handle Firebase connection
  firebase.handleConnection();
  
  // Read sensors at specified interval
  if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL) {
    lastSensorRead = currentMillis;
    
    // Read all sensor data
    currentData = sensors.readAllSensors();
    
    // Add to crash detector history
    crashDetector.addToHistory(currentData);
    
    // Perform crash detection
    int detectedSeverity = crashDetector.detectCrash(currentData);
    
    // Handle crash detection state changes
    if (detectedSeverity > NO_CRASH && !crashDetector.isCrashDetected()) {
      Serial.println("\n🚨 CRASH DETECTED! 🚨");
      Serial.print("Severity Level: ");
      Serial.println(detectedSeverity);
      
      // Send immediate emergency alert
      if (firebase.isReady()) {
        firebase.sendEmergencyAlert(currentData, detectedSeverity);
        firebase.updateCrashStatus(detectedSeverity, true);
      }
    }
    
    // Check for auto-reset of minor crashes
    if (crashDetector.shouldAutoReset()) {
      Serial.println("Auto-resetting crash detection for minor incident");
      crashDetector.resetCrashDetection();
      if (firebase.isReady()) {
        firebase.updateCrashStatus(NO_CRASH, false);
      }
    }
    
    currentCrashSeverity = crashDetector.getCrashSeverity();
  }
  
  // Send data to Firebase at specified interval (or immediately for severe crashes)
  bool shouldSendData = (currentMillis - lastFirebaseSend >= FIREBASE_SEND_INTERVAL) ||
                       (currentCrashSeverity >= MODERATE_CRASH);
  
  if (shouldSendData && firebase.isReady()) {
    lastFirebaseSend = currentMillis;
    firebase.sendSensorData(currentData, currentCrashSeverity, crashDetector.isCrashDetected());
  }
  
  // Debug output at specified interval
  if (currentMillis - lastDebugPrint >= DEBUG_PRINT_INTERVAL) {
    lastDebugPrint = currentMillis;
    printDebugInfo();
  }
  
  // Small delay to prevent watchdog issues
  delay(10);
}

void printDebugInfo() {
  Serial.println("\n--- System Status ---");
  
  // Sensor readings
  Serial.println("Sensor Readings:");
  Serial.printf("  Accel: X=%.2f, Y=%.2f, Z=%.2f g\n", 
                currentData.accelX, currentData.accelY, currentData.accelZ);
  Serial.printf("  Gyro: X=%.2f, Y=%.2f, Z=%.2f °/s\n", 
                currentData.gyroX, currentData.gyroY, currentData.gyroZ);
  Serial.printf("  Distance: %.2f cm\n", currentData.distance);
  Serial.printf("  Vibration: %s\n", currentData.vibration ? "DETECTED" : "NORMAL");
  Serial.printf("  GPS: %.6f, %.6f\n", currentData.latitude, currentData.longitude);
  
  // Crash detection status
  Serial.println("Crash Detection:");
  Serial.printf("  Status: %s\n", crashDetector.isCrashDetected() ? "ACTIVE" : "MONITORING");
  Serial.printf("  Severity: %d\n", currentCrashSeverity);
  
  // System status
  Serial.println("System Status:");
  Serial.printf("  WiFi: %s\n", firebase.isWiFiConnected() ? "Connected" : "Disconnected");
  Serial.printf("  Firebase: %s\n", firebase.isFirebaseConnected() ? "Connected" : "Disconnected");
  Serial.printf("  Uptime: %lu seconds\n", millis() / 1000);
  
  Serial.println("----------------------\n");
}