#include "sensor_manager.h"

SensorManager::SensorManager() {
  gpsSerial = nullptr;
  mpuInitialized = false;
  gpsInitialized = false;
  lastSensorRead = 0;
  
  // Initialize calibration offsets to zero
  accelOffsetX = accelOffsetY = accelOffsetZ = 0.0;
  gyroOffsetX = gyroOffsetY = gyroOffsetZ = 0.0;
}

SensorManager::~SensorManager() {
  if (gpsSerial) {
    delete gpsSerial;
  }
}

bool SensorManager::begin() {
  Serial.println("SensorManager: Initializing sensors...");
  
  // Initialize I2C for MPU6050
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Initialize MPU6050
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("SensorManager: MPU6050 connection failed");
    mpuInitialized = false;
  } else {
    // Configure MPU6050
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_RANGE);
    mpu.setFullScaleGyroRange(MPU6050_GYRO_RANGE);
    mpu.setDLPFMode(MPU6050_DLPF_MODE);
    mpuInitialized = true;
    Serial.println("SensorManager: MPU6050 initialized successfully");
  }
  
  // Initialize pin modes
  pinMode(VIBRATION_SENSOR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Initialize GPS
  gpsSerial = new SoftwareSerial(GPS_RX_PIN, GPS_TX_PIN);
  gpsSerial->begin(GPS_BAUD_RATE);
  gpsInitialized = true;
  Serial.println("SensorManager: GPS initialized");
  
  // Test ultrasonic sensor
  float testDistance = readUltrasonicDistance();
  if (testDistance > 0) {
    Serial.println("SensorManager: Ultrasonic sensor working");
  } else {
    Serial.println("SensorManager: Warning - Ultrasonic sensor may not be working");
  }
  
  // Test vibration sensor
  int vibTest = digitalRead(VIBRATION_SENSOR_PIN);
  Serial.printf("SensorManager: Vibration sensor initialized (current: %s)\n", 
                vibTest ? "HIGH" : "LOW");
  
  return mpuInitialized; // Return true if at least MPU6050 is working
}

SensorData SensorManager::readAllSensors() {
  SensorData data;
  memset(&data, 0, sizeof(SensorData));
  
  // Read MPU6050
  if (mpuInitialized) {
    readMPU6050(data.accelX, data.accelY, data.accelZ, 
                data.gyroX, data.gyroY, data.gyroZ);
  }
  
  // Read ultrasonic sensor
  data.distance = readUltrasonic();
  
  // Read vibration sensor
  data.vibration = readVibrationSensor();
  
  // Read GPS
  readGPS(data.latitude, data.longitude);
  
  // Add timestamp
  data.timestamp = millis();
  
  lastSensorRead = millis();
  return data;
}

bool SensorManager::readMPU6050(float& accelX, float& accelY, float& accelZ,
                                float& gyroX, float& gyroY, float& gyroZ) {
  if (!mpuInitialized) {
    accelX = accelY = accelZ = 0.0;
    gyroX = gyroY = gyroZ = 0.0;
    return false;
  }
  
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  
  // Convert to real units and apply calibration
  accelX = (ax / 16384.0) - accelOffsetX; // Convert to g
  accelY = (ay / 16384.0) - accelOffsetY;
  accelZ = (az / 16384.0) - accelOffsetZ;
  gyroX = (gx / 131.0) - gyroOffsetX;     // Convert to degrees/second
  gyroY = (gy / 131.0) - gyroOffsetY;
  gyroZ = (gz / 131.0) - gyroOffsetZ;
  
  return true;
}

float SensorManager::readUltrasonic() {
  return readUltrasonicDistance();
}

float SensorManager::readUltrasonicDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  float duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
  if (duration == 0) return -1; // No echo received
  
  return (duration * 0.034) / 2;
}

int SensorManager::readVibrationSensor() {
  return digitalRead(VIBRATION_SENSOR_PIN);
}

bool SensorManager::readGPS(float& latitude, float& longitude) {
  latitude = longitude = 0.0;
  
  if (!gpsInitialized || !gpsSerial) return false;
  
  bool dataUpdated = false;
  unsigned long startTime = millis();
  
  // Read GPS data for up to 100ms
  while (gpsSerial->available() > 0 && (millis() - startTime) < 100) {
    if (gps.encode(gpsSerial->read())) {
      if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        dataUpdated = true;
      }
    }
  }
  
  return dataUpdated;
}

bool SensorManager::isMPUReady() const {
  return mpuInitialized;
}

bool SensorManager::isGPSReady() const {
  return gpsInitialized && gps.location.isValid();
}

void SensorManager::performCalibration() {
  if (!mpuInitialized) return;
  
  Serial.println("SensorManager: Starting calibration...");
  Serial.println("Keep the device stationary for 5 seconds");
  
  float accelSumX = 0, accelSumY = 0, accelSumZ = 0;
  float gyroSumX = 0, gyroSumY = 0, gyroSumZ = 0;
  int samples = 100;
  
  for (int i = 0; i < samples; i++) {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    
    accelSumX += ax / 16384.0;
    accelSumY += ay / 16384.0;
    accelSumZ += az / 16384.0;
    gyroSumX += gx / 131.0;
    gyroSumY += gy / 131.0;
    gyroSumZ += gz / 131.0;
    
    delay(50);
  }
  
  // Calculate averages
  accelOffsetX = accelSumX / samples;
  accelOffsetY = accelSumY / samples;
  accelOffsetZ = (accelSumZ / samples) - 1.0; // Subtract 1g for Z-axis
  gyroOffsetX = gyroSumX / samples;
  gyroOffsetY = gyroSumY / samples;
  gyroOffsetZ = gyroSumZ / samples;
  
  Serial.println("SensorManager: Calibration complete");
  Serial.printf("Accel offsets: X=%.3f, Y=%.3f, Z=%.3f\n", 
                accelOffsetX, accelOffsetY, accelOffsetZ);
  Serial.printf("Gyro offsets: X=%.3f, Y=%.3f, Z=%.3f\n", 
                gyroOffsetX, gyroOffsetY, gyroOffsetZ);
}

void SensorManager::setCalibrationOffsets(float axOff, float ayOff, float azOff,
                                         float gxOff, float gyOff, float gzOff) {
  accelOffsetX = axOff;
  accelOffsetY = ayOff;
  accelOffsetZ = azOff;
  gyroOffsetX = gxOff;
  gyroOffsetY = gyOff;
  gyroOffsetZ = gzOff;
  
  Serial.println("SensorManager: Calibration offsets updated");
}

bool SensorManager::testMPU6050() {
  if (!mpuInitialized) return false;
  
  float ax, ay, az, gx, gy, gz;
  bool result = readMPU6050(ax, ay, az, gx, gy, gz);
  
  if (result) {
    Serial.printf("MPU6050 Test - Accel: %.2f,%.2f,%.2f | Gyro: %.2f,%.2f,%.2f\n",
                  ax, ay, az, gx, gy, gz);
  }
  
  return result;
}

bool SensorManager::testUltrasonic() {
  float distance = readUltrasonicDistance();
  
  if (distance > 0) {
    Serial.printf("Ultrasonic Test - Distance: %.2f cm\n", distance);
    return true;
  } else {
    Serial.println("Ultrasonic Test - No echo received");
    return false;
  }
}

bool SensorManager::testVibrationSensor() {
  int vibration = readVibrationSensor();
  Serial.printf("Vibration Test - State: %s\n", vibration ? "HIGH" : "LOW");
  return true; // Always returns true as it's a simple digital read
}

bool SensorManager::testGPS() {
  float lat, lon;
  bool result = readGPS(lat, lon);
  
  if (result) {
    Serial.printf("GPS Test - Location: %.6f, %.6f\n", lat, lon);
    Serial.printf("GPS Test - Satellites: %d\n", gps.satellites.value());
  } else {
    Serial.println("GPS Test - No valid location data");
  }
  
  return result;
}

void SensorManager::printSensorInfo() {
  Serial.println("\n=== Sensor Information ===");
  Serial.printf("MPU6050: %s\n", mpuInitialized ? "Connected" : "Disconnected");
  Serial.printf("GPS: %s\n", gpsInitialized ? "Initialized" : "Not initialized");
  Serial.printf("GPS Location Valid: %s\n", gps.location.isValid() ? "Yes" : "No");
  Serial.printf("GPS Satellites: %d\n", gps.satellites.value());
  Serial.printf("Last sensor read: %lu ms ago\n", millis() - lastSensorRead);
  Serial.println("========================\n");
}

String SensorManager::getSensorStatus() {
  String status = "MPU6050:";
  status += mpuInitialized ? "OK" : "FAIL";
  status += ",GPS:";
  status += gpsInitialized ? "OK" : "FAIL";
  status += ",GPS_VALID:";
  status += gps.location.isValid() ? "YES" : "NO";
  return status;
}