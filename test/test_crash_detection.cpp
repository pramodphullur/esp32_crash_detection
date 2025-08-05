#include <unity.h>
#include "crash_detector.h"
#include "config.h"

CrashDetector detector;
CrashDetectionConfig testConfig;

void setUp(void) {
    // Set up test configuration
    testConfig.accelThreshold = 3.0;
    testConfig.gyroThreshold = 250.0;
    testConfig.jerkThreshold = 10.0;
    testConfig.proximityThreshold = 30.0;
    testConfig.consecutiveReadings = 3;
    testConfig.recoveryTime = 5000;
    
    detector.begin(testConfig);
}

void tearDown(void) {
    detector.resetCrashDetection();
}

void test_no_crash_normal_conditions(void) {
    SensorData normalData;
    normalData.accelX = 0.1;  // Normal gravity variations
    normalData.accelY = 0.1;
    normalData.accelZ = 1.0;  // 1g due to gravity
    normalData.gyroX = 5.0;   // Small rotations
    normalData.gyroY = 3.0;
    normalData.gyroZ = 2.0;
    normalData.distance = 100.0; // No obstacles
    normalData.vibration = LOW;
    normalData.timestamp = millis();
    
    int severity = detector.detectCrash(normalData);
    
    TEST_ASSERT_EQUAL(NO_CRASH, severity);
    TEST_ASSERT_FALSE(detector.isCrashDetected());
}

void test_minor_crash_detection(void) {
    SensorData crashData;
    crashData.accelX = 3.5;   // Above threshold
    crashData.accelY = 1.0;
    crashData.accelZ = 2.0;
    crashData.gyroX = 100.0;  // Moderate rotation
    crashData.gyroY = 50.0;
    crashData.gyroZ = 30.0;
    crashData.distance = 25.0; // Close obstacle
    crashData.vibration = HIGH; // Vibration detected
    crashData.timestamp = millis();
    
    int severity = detector.detectCrash(crashData);
    
    TEST_ASSERT_EQUAL(MINOR_CRASH, severity);
    TEST_ASSERT_TRUE(detector.isCrashDetected());
}

void test_severe_crash_detection(void) {
    SensorData severeData;
    severeData.accelX = 8.0;   // Very high acceleration
    severeData.accelY = 6.0;
    severeData.accelZ = 4.0;
    severeData.gyroX = 500.0;  // High rotation
    severeData.gyroY = 400.0;
    severeData.gyroZ = 300.0;
    severeData.distance = 15.0; // Very close obstacle
    severeData.vibration = HIGH;
    severeData.timestamp = millis();
    
    int severity = detector.detectCrash(severeData);
    
    TEST_ASSERT_EQUAL(SEVERE_CRASH, severity);
    TEST_ASSERT_TRUE(detector.isCrashDetected());
}

void test_jerk_calculation(void) {
    // First reading
    SensorData reading1;
    reading1.accelX = 1.0;
    reading1.accelY = 0.0;
    reading1.accelZ = 1.0;
    reading1.timestamp = 1000;
    detector.addToHistory(reading1);
    
    // Second reading with sudden change
    SensorData reading2;
    reading2.accelX = 5.0;  // Sudden increase
    reading2.accelY = 3.0;
    reading2.accelZ = 2.0;
    reading2.timestamp = 1100; // 100ms later
    
    int severity = detector.detectCrash(reading2);
    
    // Should detect crash due to high jerk
    TEST_ASSERT_GREATER_THAN(NO_CRASH, severity);
}

void test_consecutive_readings(void) {
    SensorData highData;
    highData.accelX = 3.2;  // Slightly above threshold
    highData.accelY = 1.0;
    highData.accelZ = 1.5;
    highData.gyroX = 50.0;
    highData.gyroY = 30.0;
    highData.gyroZ = 20.0;
    highData.distance = 50.0;
    highData.vibration = LOW;
    
    // Add multiple consecutive high readings
    for (int i = 0; i < 5; i++) {
        highData.timestamp = millis() + i * 100;
        detector.addToHistory(highData);
        int severity = detector.detectCrash(highData);
        
        if (i >= testConfig.consecutiveReadings - 1) {
            TEST_ASSERT_GREATER_THAN(NO_CRASH, severity);
        }
    }
}

void test_auto_reset_minor_crash(void) {
    // Simulate minor crash
    SensorData minorCrash;
    minorCrash.accelX = 3.5;
    minorCrash.accelY = 1.0;
    minorCrash.accelZ = 1.5;
    minorCrash.gyroX = 100.0;
    minorCrash.gyroY = 50.0;
    minorCrash.gyroZ = 30.0;
    minorCrash.distance = 25.0;
    minorCrash.vibration = HIGH;
    minorCrash.timestamp = millis();
    
    int severity = detector.detectCrash(minorCrash);
    TEST_ASSERT_EQUAL(MINOR_CRASH, severity);
    TEST_ASSERT_TRUE(detector.isCrashDetected());
    
    // Wait for auto-reset time
    delay(testConfig.recoveryTime + 100);
    
    // Should auto-reset for minor crashes
    TEST_ASSERT_TRUE(detector.shouldAutoReset());
}

void test_no_auto_reset_severe_crash(void) {
    // Simulate severe crash
    SensorData severeCrash;
    severeCrash.accelX = 8.0;
    severeCrash.accelY = 6.0;
    severeCrash.accelZ = 4.0;
    severeCrash.gyroX = 500.0;
    severeCrash.gyroY = 400.0;
    severeCrash.gyroZ = 300.0;
    severeCrash.distance = 15.0;
    severeCrash.vibration = HIGH;
    severeCrash.timestamp = millis();
    
    int severity = detector.detectCrash(severeCrash);
    TEST_ASSERT_EQUAL(SEVERE_CRASH, severity);
    
    // Wait past auto-reset time
    delay(testConfig.recoveryTime + 100);
    
    // Should NOT auto-reset for severe crashes
    TEST_ASSERT_FALSE(detector.shouldAutoReset());
}

void test_config_update(void) {
    CrashDetectionConfig newConfig = testConfig;
    newConfig.accelThreshold = 5.0; // Higher threshold
    
    detector.updateConfig(newConfig);
    
    CrashDetectionConfig retrievedConfig = detector.getConfig();
    TEST_ASSERT_EQUAL_FLOAT(5.0, retrievedConfig.accelThreshold);
}

void setup() {
    delay(2000); // Wait for board to stabilize
    
    UNITY_BEGIN();
    
    RUN_TEST(test_no_crash_normal_conditions);
    RUN_TEST(test_minor_crash_detection);
    RUN_TEST(test_severe_crash_detection);
    RUN_TEST(test_jerk_calculation);
    RUN_TEST(test_consecutive_readings);
    RUN_TEST(test_auto_reset_minor_crash);
    RUN_TEST(test_no_auto_reset_severe_crash);
    RUN_TEST(test_config_update);
    
    UNITY_END();
}

void loop() {
    // Empty - tests run once in setup
}