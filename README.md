# ESP32 Crash Detection System

A comprehensive vehicle crash detection system using ESP32 with multiple sensors and Firebase integration.

## Project Structure

```
esp32-crash-detection/
├── .gitignore
├── README.md
├── LICENSE
├── platformio.ini
├── docs/
│   ├── hardware-setup.md
│   ├── firebase-configuration.md
│   ├── crash-detection-algorithm.md
│   └── images/
├── include/
│   ├── config.h
│   ├── crash_detector.h
│   ├── sensor_manager.h
│   └── firebase_manager.h
├── src/
│   ├── main.cpp
│   ├── crash_detector.cpp
│   ├── sensor_manager.cpp
│   └── firebase_manager.cpp
├── lib/
│   └── README
├── test/
│   ├── test_crash_detection.cpp
│   └── test_sensors.cpp
├── data/
│   ├── config.json
│   └── certificates/
└── examples/
    ├── basic_sensor_test.ino
    └── firebase_test.ino
```

## Features

- **Multi-sensor crash detection**: Accelerometer, Gyroscope, Ultrasonic, Vibration, GPS
- **Advanced crash detection algorithm** with severity classification
- **Real-time Firebase integration** for emergency alerts
- **Configurable thresholds** and parameters
- **Comprehensive logging** and debugging capabilities

## Hardware Requirements

- ESP32 Development Board
- MPU6050 (Accelerometer/Gyroscope)
- HC-SR04 Ultrasonic Sensor
- Vibration Sensor
- GPS Module (NEO-6M or similar)
- Jumper wires and breadboard

## Software Dependencies

- PlatformIO
- Firebase ESP Client
- MPU6050 Library
- TinyGPS++
- ArduinoJson
- NTPClient

## Quick Start

1. Clone this repository
2. Configure your WiFi and Firebase credentials in `include/config.h`
3. Upload the code to your ESP32
4. Monitor serial output for sensor readings and crash detection

## Configuration

Edit `include/config.h` to customize:
- WiFi credentials
- Firebase API keys
- Sensor pin assignments
- Crash detection thresholds

## License

This project is licensed under the MIT License - see the LICENSE file for details.
