/*
 * Basic Sensor Test Example
 * 
 * This example demonstrates basic sensor reading functionality
 * without the full crash detection system.
 */

#include "sensor_manager.h"

SensorManager sensors;

void setup() {
  Serial.begin(115200);
  Serial.println("Basic Sensor Test Starting...");
  
  if (!sensors.begin()) {
    Serial.println("ERROR: Failed to initialize sensors!");
    while (1) {
      delay(1000);
    }
  }
  
  Serial.println("Sensors initialized successfully!");
  Serial.println("Performing calibration...");
  sensors.performCalibration();
  Serial.println("Calibration complete!");
  Serial.println();
}

void loop() {
  // Read all sensors
  SensorData data = sensors.readAllSensors();
  
  // Print sensor data
  Serial.println("=== Sensor Readings ===");
  Serial.printf("Timestamp: %lu ms\n", data.timestamp);
  Serial.printf("Accelerometer: X=%.3f, Y=%.3f, Z=%.3f g\n", 
                data.accelX, data.accelY, data.accelZ);
  Serial.printf("Gyroscope: X=%.2f, Y=%.2f, Z=%.2f °/s\n", 
                data.gyroX, data.gyroY, data.gyroZ);
  Serial.printf("Distance: %.2f cm\n", data.distance);
  Serial.printf("Vibration: %s\n", data.vibration ? "DETECTED" : "NONE");
  Serial.printf("GPS: %.6f, %.6f\n", data.latitude, data.longitude);
  
  // Calculate magnitudes
  float accelMag = sqrt(data.accelX*data.accelX + 
                       data.accelY*data.accelY + 
                       data.accelZ*data.accelZ);
  float gyroMag = sqrt(data.gyroX*data.gyroX + 
                      data.gyroY*data.gyroY + 
                      data.gyroZ*data.gyroZ);
  
  Serial.printf("Acceleration Magnitude: %.3f g\n", accelMag);
  Serial.printf("Gyroscope Magnitude: %.2f °/s\n", gyroMag);
  Serial.println("========================\n");
  
  delay(1000);
}