#ifndef CRASH_DETECTOR_H
#define CRASH_DETECTOR_H

#include "config.h"
#include <Arduino.h>

class CrashDetector {
private:
  CrashDetectionConfig config;
  SensorData* sensorHistory;
  int historySize;
  int currentIndex;
  bool crashDetected;
  unsigned long crashDetectionTime;
  int currentSeverity;

  // Helper functions
  float calculateMagnitude(float x, float y, float z);
  float calculateJerk(const SensorData& current, const SensorData& previous);
  int calculateConsecutiveHighReadings();
  int calculateCrashScore(const SensorData& currentReading);

public:
  CrashDetector();
  ~CrashDetector();
  
  // Initialize the crash detector
  void begin(const CrashDetectionConfig& detectorConfig);
  
  // Main crash detection function
  int detectCrash(const SensorData& currentReading);
  
  // Add sensor reading to history
  void addToHistory(const SensorData& data);
  
  // Check if crash is currently detected
  bool isCrashDetected() const;
  
  // Get current crash severity
  int getCrashSeverity() const;
  
  // Reset crash detection state
  void resetCrashDetection();
  
  // Check if crash detection should auto-reset
  bool shouldAutoReset();
  
  // Update configuration
  void updateConfig(const CrashDetectionConfig& newConfig);
  
  // Get current configuration
  CrashDetectionConfig getConfig() const;
  
  // Get sensor history for debugging
  SensorData* getHistory() const;
  int getHistoryIndex() const;
};

#endif // CRASH_DETECTOR_H