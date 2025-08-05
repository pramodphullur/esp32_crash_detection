#ifndef CONFIG_H
#define CONFIG_H

// Firebase credentials - CHANGE THESE TO YOUR VALUES
#define API_KEY "<Replace_with_the_API_Key>"
#define DATABASE_URL "<Replace_with_the_URL>"

// Wi-Fi credentials - CHANGE THESE TO YOUR VALUES
#define WIFI_SSID "<Replace_with_the_SSID>"
#define WIFI_PASSWORD "<Replace_with_the_WiFi_Password"

// Pin definitions
#define VIBRATION_SENSOR_PIN 34
#define TRIG_PIN 5
#define ECHO_PIN 18
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17

// I2C pins (default for ESP32)
#define SDA_PIN 21
#define SCL_PIN 22

// Crash detection configuration
struct CrashDetectionConfig {
  float accelThreshold = 3.0;      // g-force threshold for crash detection
  float gyroThreshold = 250.0;     // degrees/second threshold
  float impactDuration = 500;      // milliseconds
  int consecutiveReadings = 3;     // number of consecutive high readings
  float recoveryTime = 5000;       // milliseconds before reset
  float proximityThreshold = 30.0; // cm for obstacle detection
  float jerkThreshold = 10.0;      // threshold for jerk detection
  float severeJerkThreshold = 20.0; // threshold for severe jerk
  float severeAccelThreshold = 5.0; // threshold for severe acceleration
  float severeGyroThreshold = 400.0; // threshold for severe rotation
};

// Timing configuration
#define SENSOR_READ_INTERVAL 100    // milliseconds
#define FIREBASE_SEND_INTERVAL 5000 // milliseconds
#define DEBUG_PRINT_INTERVAL 2000   // milliseconds
#define GPS_BAUD_RATE 9600
#define SERIAL_BAUD_RATE 115200

// NTP configuration
#define NTP_SERVER "pool.ntp.org"
#define TIME_OFFSET 19800  // GMT+5:30 for India (in seconds)

// Firebase paths
#define FB_SENSORS_PATH "Servo1/sensors/"
#define FB_EMERGENCY_PATH "Servo1/emergency/"
#define FB_CRASH_STATUS_PATH "Servo1/crashStatus"
#define FB_EMERGENCY_ACTIVE_PATH "Servo1/emergencyActive"

// MPU6050 configuration
#define MPU6050_ACCEL_RANGE MPU6050_ACCEL_FS_8  // ±8g
#define MPU6050_GYRO_RANGE MPU6050_GYRO_FS_500  // ±500°/s
#define MPU6050_DLPF_MODE MPU6050_DLPF_BW_42    // 42Hz filter

// Sensor history size
#define SENSOR_HISTORY_SIZE 10

// Crash severity levels
enum CrashSeverity {
  NO_CRASH = 0,
  MINOR_CRASH = 1,
  MODERATE_CRASH = 2,
  SEVERE_CRASH = 3
};

// Sensor data structure
struct SensorData {
  float accelX, accelY, accelZ;
  float gyroX, gyroY, gyroZ;
  float distance;
  int vibration;
  float latitude, longitude;
  unsigned long timestamp;
};

#endif // CONFIG_H