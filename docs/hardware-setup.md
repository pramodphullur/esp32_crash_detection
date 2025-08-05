# Hardware Setup Guide

This document provides detailed instructions for setting up the hardware components of the ESP32 Crash Detection System.

## Required Components

### Main Components
- **ESP32 Development Board** (ESP32-WROOM-32 or similar)
- **MPU6050** - 6-axis accelerometer and gyroscope module
- **HC-SR04** - Ultrasonic distance sensor
- **Vibration Sensor Module** (SW-420 or similar)
- **GPS Module** (NEO-6M or NEO-8M)

### Additional Components
- Breadboard or PCB
- Jumper wires (male-to-male, male-to-female)
- Power supply (5V/3.3V)
- Resistors (if needed for pull-up/pull-down)
- Capacitors for power filtering (optional)

## Pin Connections

### ESP32 Pin Assignment

| Component | ESP32 Pin | Function | Notes |
|-----------|-----------|----------|-------|
| MPU6050 | GPIO 21 | SDA | I2C Data |
| MPU6050 | GPIO 22 | SCL | I2C Clock |
| MPU6050 | 3.3V | VCC | Power |
| MPU6050 | GND | GND | Ground |
| HC-SR04 | GPIO 5 | TRIG | Trigger |
| HC-SR04 | GPIO 18 | ECHO | Echo |
| HC-SR04 | 5V | VCC | Power |
| HC-SR04 | GND | GND | Ground |
| Vibration | GPIO 34 | OUT | Digital Output |
| Vibration | 3.3V | VCC | Power |
| Vibration | GND | GND | Ground |
| GPS | GPIO 16 | RX | GPS TX to ESP32 RX |
| GPS | GPIO 17 | TX | GPS RX to ESP32 TX |
| GPS | 3.3V | VCC | Power |
| GPS | GND | GND | Ground |

### Detailed Connections

#### MPU6050 (Accelerometer/Gyroscope)
```
MPU6050    ESP32
VCC   -->  3.3V
GND   -->  GND
SDA   -->  GPIO 21
SCL   -->  GPIO 22
```

#### HC-SR04 (Ultrasonic Sensor)
```
HC-SR04    ESP32
VCC   -->  5V (or 3.3V)
GND   -->  GND
TRIG  -->  GPIO 5
ECHO  -->  GPIO 18
```

#### Vibration Sensor (SW-420)
```
Vibration  ESP32
VCC   -->  3.3V
GND   -->  GND
OUT   -->  GPIO 34 (ADC1_CH6)
```

#### GPS Module (NEO-6M/8M)
```
GPS        ESP32
VCC   -->  3.3V
GND   -->  GND
TX    -->  GPIO 16 (RX2)
RX    -->  GPIO 17 (TX2)
```

## Wiring Diagram

```
                    ESP32
                 ┌─────────┐
                 │         │
                 │   3.3V  ├──┬── MPU6050 VCC
                 │         │  ├── Vibration VCC
                 │         │  └── GPS VCC
                 │         │
                 │    5V   ├───── HC-SR04 VCC
                 │         │
                 │   GND   ├──┬── All GND connections
                 │         │  │
                 │ GPIO 21 ├──── MPU6050 SDA
                 │ GPIO 22 ├──── MPU6050 SCL
                 │ GPIO 5  ├──── HC-SR04 TRIG
                 │ GPIO 18 ├──── HC-SR04 ECHO
                 │ GPIO 34 ├──── Vibration OUT
                 │ GPIO 16 ├──── GPS TX
                 │ GPIO 17 ├──── GPS RX
                 │         │
                 └─────────┘
```

## Assembly Instructions

### Step 1: Prepare the Breadboard
1. Place the ESP32 development board on the breadboard
2. Connect power rails (3.3V and GND) using jumper wires

### Step 2: Mount MPU6050
1. Place MPU6050 on breadboard
2. Connect VCC to 3.3V rail
3. Connect GND to ground rail
4. Connect SDA to GPIO 21
5. Connect SCL to GPIO 22

### Step 3: Mount HC-SR04
1. Place HC-SR04 on breadboard
2. Connect VCC to 5V (or 3.3V if 5V not available)
3. Connect GND to ground rail
4. Connect TRIG to GPIO 5
5. Connect ECHO to GPIO 18

### Step 4: Mount Vibration Sensor
1. Place vibration sensor on breadboard
2. Connect VCC to 3.3V rail
3. Connect GND to ground rail
4. Connect OUT to GPIO 34

### Step 5: Mount GPS Module
1. Place GPS module on breadboard or connect via wires
2. Connect VCC to 3.3V rail
3. Connect GND to ground rail
4. Connect GPS TX to ESP32 GPIO 16 (RX)
5. Connect GPS RX to ESP32 GPIO 17 (TX)

## Power Considerations

### Power Supply Requirements
- **ESP32**: 3.3V, ~240mA (during WiFi transmission)
- **MPU6050**: 3.3V, ~3.9mA
- **HC-SR04**: 5V preferred, ~15mA
- **Vibration Sensor**: 3.3V, ~5mA
- **GPS Module**: 3.3V, ~45mA

### Total Power Consumption
- **Typical**: ~300mA at 3.3V
- **Peak**: ~400mA at 3.3V (during WiFi transmission)

### Power Supply Options
1. **USB Power**: Use USB cable for development/testing
2. **Battery Pack**: 4xAA batteries with voltage regulator
3. **Li-Po Battery**: 3.7V with appropriate charging circuit
4. **Vehicle Power**: 12V to 3.3V converter for automotive use

## Testing the Hardware

### Pre-Flight Checks
1. **Power Check**: Verify all components receive correct voltage
2. **Connection Check**: Ensure all connections are secure
3. **I2C Check**: Test MPU6050 communication
4. **GPIO Check**: Test all digital pins
5. **Serial Check**: Verify GPS communication

### Initial Testing
1. Upload basic sensor test sketch
2. Monitor serial output for sensor readings
3. Verify each sensor responds correctly
4. Check for any error messages

## Troubleshooting

### Common Issues

#### MPU6050 Not Detected
- Check I2C connections (SDA/SCL)
- Verify power supply (3.3V)
- Try different I2C address (0x68 or 0x69)
- Check for loose connections

#### HC-SR04 No Readings
- Verify TRIG/ECHO pin connections
- Check power supply (5V preferred)
- Ensure clear path for ultrasonic waves
- Check for timing issues

#### GPS No Fix
- Ensure antenna has clear sky view
- Wait sufficient time for initial fix (cold start can take 10+ minutes)
- Check baud rate (9600 default)
- Verify TX/RX connections

### Debugging Tools
- Serial monitor for debug output
- Logic analyzer for I2C/UART signals
- Oscilloscope for analog signals
- Multimeter for power and continuity

## Enclosure Considerations

### For Vehicle Installation
- **Weatherproof Enclosure**: IP65 or higher rating
- **Vibration Resistance**: Secure mounting to prevent false triggers
- **Heat Dissipation**: Adequate ventilation for electronics
- **Antenna Access**: GPS antenna must have sky view

### Mounting Location
- **Central Location**: For accurate crash detection
- **Protected Area**: Away from direct impact zones
- **Accessible**: For maintenance and debugging
- **Power Access**: Near vehicle power source

## Safety Warnings

**Important Safety Notes**

1. **Double-check all connections** before applying power
2. **Use appropriate voltage levels** - never exceed component ratings
3. **Avoid short circuits** - they can damage components permanently
4. **Handle components carefully** - they are sensitive to static electricity
5. **Test thoroughly** before final installation in vehicle

## Component Sources

### Recommended Suppliers
- **ESP32**: Espressif, Adafruit, SparkFun
- **MPU6050**: InvenSense, Various breakout boards
- **HC-SR04**: Generic suppliers, Arduino compatible
- **GPS Modules**: u-blox, various NEO-6M/8M boards
- **Vibration Sensors**: SW-420 or similar digital modules

### Quality Considerations
- Use ESP32-wroom32 (devkit-v1) board for reliability
- Choose MPU6050 boards with proper voltage regulation
- Ensure GPS modules have quality antennas
- Test all components before final assembly
