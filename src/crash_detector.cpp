#include "crash_detector.h"
#include <math.h>

CrashDetector::CrashDetector() {
  sensorHistory = nullptr;
  historySize = SENSOR_HISTORY_SIZE;
  currentIndex = 0;
  crashDetected = false;
  crashDetectionTime = 0;
  currentSeverity = NO_CRASH;
}

CrashDetector::~CrashDetector() {
  if (sensorHistory) {
    delete[] sensorHistory;
  }
}

void CrashDetector::begin(const CrashDetectionConfig& detectorConfig) {
  config = detectorConfig;
  
  // Allocate memory for sensor history
  if (sensorHistory) {
    delete[] sensorHistory;
  }
  sensorHistory = new SensorData[historySize];
  
  // Initialize history with zero values
  for (int i = 0; i < historySize; i++) {
    memset(&sensorHistory[i], 0, sizeof(SensorData));
  }
  
  currentIndex = 0;
  crashDetected = false;
  crashDetectionTime = 0;
  currentSeverity = NO_CRASH;
  
  Serial.println("CrashDetector: Initialized with configuration:");
  Serial.printf("  Accel Threshold: %.2f g\n", config.accelThreshold);
  Serial.printf("  Gyro Threshold: %.2f °/s\n", config.gyroThreshold);
  Serial.printf("  Jerk Threshold: %.2f\n", config.jerkThreshold);
  Serial.printf("  Recovery Time: %.0f ms\n", config.recoveryTime);
}

float CrashDetector::calculateMagnitude(float x, float y, float z) {
  return sqrt(x*x + y*y + z*z);
}

float CrashDetector::calculateJerk(const SensorData& current, const SensorData& previous) {
  float deltaAccelX = current.accelX - previous.accelX;
  float deltaAccelY = current.accelY - previous.accelY;
  float deltaAccelZ = current.accelZ - previous.accelZ;
  float deltaTime = (current.timestamp - previous.timestamp) / 1000.0; // Convert to seconds
  
  if (deltaTime <= 0) return 0;
  
  return calculateMagnitude(deltaAccelX, deltaAccelY, deltaAccelZ) / deltaTime;
}

int CrashDetector::calculateConsecutiveHighReadings() {
  int consecutiveCount = 0;
  int maxConsecutive = min(config.consecutiveReadings, historySize);
  
  for (int i = 0; i < maxConsecutive; i++) {
    int idx = (currentIndex - i - 1 + historySize) % historySize;
    if (idx < 0) break;
    
    float magnitude = calculateMagnitude(sensorHistory[idx].accelX, 
                                       sensorHistory[idx].accelY, 
                                       sensorHistory[idx].accelZ);
    
    if (magnitude > config.accelThreshold * 0.7) {
      consecutiveCount++;
    } else {
      break; // Non-consecutive reading
    }
  }
  
  return consecutiveCount;
}

int CrashDetector::calculateCrashScore(const SensorData& currentReading) {
  int crashScore = 0;
  
  // Factor 1: High acceleration (impact detection)
  float accelMagnitude = calculateMagnitude(currentReading.accelX, 
                                          currentReading.accelY, 
                                          currentReading.accelZ);
  if (accelMagnitude > config.accelThreshold) {
    crashScore += (accelMagnitude > config.severeAccelThreshold) ? 3 : 2;
  }
  
  // Factor 2: High rotation (vehicle spinning/rolling)
  float gyroMagnitude = calculateMagnitude(currentReading.gyroX, 
                                         currentReading.gyroY, 
                                         currentReading.gyroZ);
  if (gyroMagnitude > config.gyroThreshold) {
    crashScore += (gyroMagnitude > config.severeGyroThreshold) ? 3 : 2;
  }
  
  // Factor 3: High jerk (sudden change in acceleration)
  if (currentIndex > 0) {
    int prevIndex = (currentIndex - 1 + historySize) % historySize;
    float jerk = calculateJerk(currentReading, sensorHistory[prevIndex]);
    
    if (jerk > config.jerkThreshold) {
      crashScore += (jerk > config.severeJerkThreshold) ? 3 : 2;
    }
  }
  
  // Factor 4: Vibration sensor triggered
  if (currentReading.vibration == HIGH) {
    crashScore += 2;
  }
  
  // Factor 5: Proximity sensor (obstacle detection)
  if (currentReading.distance < config.proximityThreshold && currentReading.distance > 0) {
    crashScore += 1;
  }
  
  // Factor 6: Check for consecutive high readings
  int consecutiveHighReadings = calculateConsecutiveHighReadings();
  if (consecutiveHighReadings >= config.consecutiveReadings) {
    crashScore += 2;
  }
  
  return crashScore;
}

int CrashDetector::detectCrash(const SensorData& currentReading) {
  int crashScore = calculateCrashScore(currentReading);
  int detectedSeverity = NO_CRASH;
  
  // Determine crash severity based on score
  if (crashScore >= 8) {
    detectedSeverity = SEVERE_CRASH;
  } else if (crashScore >= 5) {
    detectedSeverity = MODERATE_CRASH;
  } else if (crashScore >= 3) {
    detectedSeverity = MINOR_CRASH;
  }
  
  // Update crash detection state
  if (detectedSeverity > NO_CRASH && !crashDetected) {
    crashDetected = true;
    crashDetectionTime = millis();
    currentSeverity = detectedSeverity;
    
    Serial.printf("CrashDetector: Crash detected with score %d, severity %d\n", 
                  crashScore, detectedSeverity);
  }
  
  return detectedSeverity;
}

void CrashDetector::addToHistory(const SensorData& data) {
  sensorHistory[currentIndex] = data;
  currentIndex = (currentIndex + 1) % historySize;
}

bool CrashDetector::isCrashDetected() const {
  return crashDetected;
}

int CrashDetector::getCrashSeverity() const {
  return currentSeverity;
}

void CrashDetector::resetCrashDetection() {
  crashDetected = false;
  crashDetectionTime = 0;
  currentSeverity = NO_CRASH;
  
  Serial.println("CrashDetector: Detection state reset");
}

bool CrashDetector::shouldAutoReset() {
  if (!crashDetected) return false;
  
  unsigned long timeSinceCrash = millis() - crashDetectionTime;
  
  // Auto-reset only for minor crashes after recovery time
  return (currentSeverity == MINOR_CRASH && timeSinceCrash > config.recoveryTime);
}

void CrashDetector::updateConfig(const CrashDetectionConfig& newConfig) {
  config = newConfig;
  Serial.println("CrashDetector: Configuration updated");
}

CrashDetectionConfig CrashDetector::getConfig() const {
  return config;
}

SensorData* CrashDetector::getHistory() const {
  return sensorHistory;
}

int CrashDetector::getHistoryIndex() const {
  return currentIndex;
}