#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "config.h"
#include <Arduino.h>
#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

class SensorManager {
private:
  MPU6050 mpu;
  TinyGPSPlus gps;
  SoftwareSerial* gpsSerial;
  
  bool mpuInitialized;
  bool gpsInitialized;
  unsigned long lastSensorRead;
  
  // Calibration values
  float accelOffsetX, accelOffsetY, accelOffsetZ;
  float gyroOffsetX, gyroOffsetY, gyroOffsetZ;
  
  // Helper functions
  float readUltrasonicDistance();
  void calibrateMPU6050();

public:
  SensorManager();
  ~SensorManager();
  
  // Initialize all sensors
  bool begin();
  
  // Read all sensors and return data
  SensorData readAllSensors();
  
  // Individual sensor reading functions
  bool readMPU6050(float& accelX, float& accelY, float& accelZ, 
                   float& gyroX, float& gyroY, float& gyroZ);
  float readUltrasonic();
  int readVibrationSensor();
  bool readGPS(float& latitude, float& longitude);
  
  // Sensor status functions
  bool isMPUReady() const;
  bool isGPSReady() const;
  
  // Calibration functions
  void performCalibration();
  void setCalibrationOffsets(float axOff, float ayOff, float azOff,
                           float gxOff, float gyOff, float gzOff);
  
  // Test functions
  bool testMPU6050();
  bool testUltrasonic();
  bool testVibrationSensor();
  bool testGPS();
  
  // Get sensor information
  void printSensorInfo();
  String getSensorStatus();
};

#endif // SENSOR_MANAGER_H