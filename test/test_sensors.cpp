#include <unity.h>
#include "sensor_manager.h"
#include "config.h"

SensorManager sensors;

void setUp(void) {
    // Initialize sensors before each test
    if (!sensors.begin()) {
        TEST_FAIL_MESSAGE("Failed to initialize sensors");
    }
}

void tearDown(void) {
    // Cleanup after each test
}

void test_mpu6050_initialization(void) {
    TEST_ASSERT_TRUE(sensors.isMPUReady());
}

void test_mpu6050_reading(void) {
    float accelX, accelY, accelZ;
    float gyroX, gyroY, gyroZ;
    
    bool result = sensors.readMPU6050(accelX, accelY, accelZ, gyroX, gyroY, gyroZ);
    
    TEST_ASSERT_TRUE(result);
    
    // Check if readings are within reasonable ranges
    TEST_ASSERT_FLOAT_WITHIN(50.0, 0.0, accelX);  // ±50g max
    TEST_ASSERT_FLOAT_WITHIN(50.0, 0.0, accelY);
    TEST_ASSERT_FLOAT_WITHIN(50.0, 1.0, accelZ);  // Should be ~1g due to gravity
    
    TEST_ASSERT_FLOAT_WITHIN(2000.0, 0.0, gyroX); // ±2000°/s max
    TEST_ASSERT_FLOAT_WITHIN(2000.0, 0.0, gyroY);
    TEST_ASSERT_FLOAT_WITHIN(2000.0, 0.0, gyroZ);
}

void test_ultrasonic_sensor(void) {
    float distance = sensors.readUltrasonic();
    
    // Distance should be positive or -1 (timeout)
    TEST_ASSERT_TRUE(distance > 0 || distance == -1);
    
    if (distance > 0) {
        // Should be within sensor range (2-400cm for HC-SR04)
        TEST_ASSERT_FLOAT_WITHIN(400.0, 200.0, distance);
    }
}

void test_vibration_sensor(void) {
    int vibration = sensors.readVibrationSensor();
    
    // Should be either HIGH (1) or LOW (0)
    TEST_ASSERT_TRUE(vibration == HIGH || vibration == LOW);
}

void test_gps_reading(void) {
    float latitude, longitude;
    
    // Note: GPS might not have fix indoors, so we just test the function doesn't crash
    bool result = sensors.readGPS(latitude, longitude);
    
    if (result) {
        // If GPS has fix, coordinates should be reasonable
        TEST_ASSERT_FLOAT_WITHIN(180.0, 0.0, latitude);   // ±90° latitude
        TEST_ASSERT_FLOAT_WITHIN(360.0, 0.0, longitude);  // ±180° longitude
    }
    
    // Test should pass regardless of GPS fix status
    TEST_ASSERT_TRUE(true);
}

void test_all_sensors_reading(void) {
    SensorData data = sensors.readAllSensors();
    
    // Check timestamp is recent
    unsigned long currentTime = millis();
    TEST_ASSERT_UINT32_WITHIN(1000, currentTime, data.timestamp);
    
    // Check accelerometer readings are reasonable
    TEST_ASSERT_FLOAT_WITHIN(50.0, 0.0, data.accelX);
    TEST_ASSERT_FLOAT_WITHIN(50.0, 0.0, data.accelY);
    TEST_ASSERT_FLOAT_WITHIN(50.0, 1.0, data.accelZ);
    
    // Check gyroscope readings are reasonable
    TEST_ASSERT_FLOAT_WITHIN(2000.0, 0.0, data.gyroX);
    TEST_ASSERT_FLOAT_WITHIN(2000.0, 0.0, data.gyroY);
    TEST_ASSERT_FLOAT_WITHIN(2000.0, 0.0, data.gyroZ);
    
    // Vibration should be 0 or 1
    TEST_ASSERT_TRUE(data.vibration == 0 || data.vibration == 1);
}

void test_sensor_calibration(void) {
    // Perform calibration
    sensors.performCalibration();
    
    // After calibration, readings should be closer to zero when stationary
    delay(1000); // Wait for stabilization
    
    float accelX, accelY, accelZ;
    float gyroX, gyroY, gyroZ;
    
    sensors.readMPU6050(accelX, accelY, accelZ, gyroX, gyroY, gyroZ);
    
    // Accelerometer X and Y should be close to 0, Z should be close to 1g
    TEST_ASSERT_FLOAT_WITHIN(0.2, 0.0, accelX);
    TEST_ASSERT_FLOAT_WITHIN(0.2, 0.0, accelY);
    TEST_ASSERT_FLOAT_WITHIN(0.2, 1.0, accelZ);
    
    // Gyroscope should be close to 0 when stationary
    TEST_ASSERT_FLOAT_WITHIN(10.0, 0.0, gyroX);
    TEST_ASSERT_FLOAT_WITHIN(10.0, 0.0, gyroY);
    TEST_ASSERT_FLOAT_WITHIN(10.0, 0.0, gyroZ);
}

void test_sensor_consistency(void) {
    const int numReadings = 10;
    SensorData readings[numReadings];
    
    // Take multiple readings
    for (int i = 0; i < numReadings; i++) {
        readings[i] = sensors.readAllSensors();
        delay(100);
    }
    
    // Calculate standard deviation for accelerometer Z-axis (should be ~1g)
    float sum = 0, sumSquares = 0;
    for (int i = 0; i < numReadings; i++) {
        sum += readings[i].accelZ;
        sumSquares += readings[i].accelZ * readings[i].accelZ;
    }
    
    float mean = sum / numReadings;
    float variance = (sumSquares / numReadings) - (mean * mean);
    float stdDev = sqrt(variance);
    
    // Standard deviation should be small for stationary sensor
    TEST_ASSERT_FLOAT_WITHIN(0.1, 1.0, mean);     // Mean should be ~1g
    TEST_ASSERT_LESS_THAN(0.2, stdDev);           // Low variation
}

void test_sensor_performance(void) {
    unsigned long startTime = millis();
    const int numReadings = 100;
    
    // Measure time for multiple sensor readings
    for (int i = 0; i < numReadings; i++) {
        sensors.readAllSensors();
    }
    
    unsigned long endTime = millis();
    unsigned long totalTime = endTime - startTime;
    float avgTime = totalTime / (float)numReadings;
    
    // Should be able to read sensors in less than 50ms each
    TEST_ASSERT_LESS_THAN(50.0, avgTime);
    
    Serial.print("Average sensor read time: ");
    Serial.print(avgTime);
    Serial.println(" ms");
}

void test_individual_sensor_tests(void) {
    // Test each sensor individually
    TEST_ASSERT_TRUE(sensors.testMPU6050());
    TEST_ASSERT_TRUE(sensors.testUltrasonic());
    TEST_ASSERT_TRUE(sensors.testVibrationSensor());
    
    // GPS test might fail indoors - that's ok
    sensors.testGPS(); // Just run it, don't assert result
}

void setup() {
    delay(2000); // Wait for board to stabilize
    
    Serial.begin(115200);
    Serial.println("Starting sensor tests...");
    
    UNITY_BEGIN();
    
    RUN_TEST(test_mpu6050_initialization);
    RUN_TEST(test_mpu6050_reading);
    RUN_TEST(test_ultrasonic_sensor);
    RUN_TEST(test_vibration_sensor);
    RUN_TEST(test_gps_reading);
    RUN_TEST(test_all_sensors_reading);
    RUN_TEST(test_sensor_calibration);
    RUN_TEST(test_sensor_consistency);
    RUN_TEST(test_sensor_performance);
    RUN_TEST(test_individual_sensor_tests);
    
    UNITY_END();
}

void loop() {
    // Empty - tests run once in setup
}