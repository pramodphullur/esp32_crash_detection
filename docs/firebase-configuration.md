# Firebase Configuration Guide

This guide explains how to set up Firebase Realtime Database for the ESP32 Crash Detection System.

## Prerequisites

- Google account
- Basic understanding of Firebase console
- ESP32 crash detection hardware setup completed

## Step 1: Create Firebase Project

### 1.1 Access Firebase Console
1. Go to [Firebase Console](https://console.firebase.google.com/)
2. Sign in with your Google account
3. Click "Create a project" or "Add project"

### 1.2 Project Setup
1. **Project Name**: Enter a name (e.g., "ESP32-Crash-Detection")
2. **Google Analytics**: Choose whether to enable (optional for this project)
3. **Location**: Select your preferred region
4. Click "Create project"

## Step 2: Setup Realtime Database

### 2.1 Create Database
1. In Firebase console, select your project
2. Navigate to "Realtime Database" in the left sidebar
3. Click "Create Database"
4. Choose database location (select closest to your region)
5. Select "Start in test mode" for initial setup

### 2.2 Configure Security Rules
```json
{
  "rules": {
    "Servo1": {
      ".read": true,
      ".write": true,
      "sensors": {
        ".read": true,
        ".write": true
      },
      "emergency": {
        ".read": true,
        ".write": true
      }
    }
  }
}
```

**Security Warning**: These rules allow public read/write access. For production, implement proper authentication and restrictive rules.

### 2.3 Production Security Rules (Recommended)
```json
{
  "rules": {
    "Servo1": {
      ".read": "auth != null",
      ".write": "auth != null",
      "sensors": {
        ".validate": "newData.hasChildren(['timestamp', 'accelX', 'accelY', 'accelZ'])"
      },
      "emergency": {
        ".validate": "newData.hasChildren(['timestamp', 'severity', 'latitude', 'longitude'])"
      }
    }
  }
}
```

## Step 3: Get Firebase Configuration

### 3.1 Get API Key and Database URL
1. In Firebase console, go to Project Settings (gear icon)
2. Navigate to "General" tab
3. Scroll down to "Your apps" section
4. Click "Add app" and select Web app (</>) icon
5. Register your app with a nickname
6. Copy the configuration values:

```javascript
const firebaseConfig = {
  apiKey: "your-api-key-here",
  authDomain: "your-project.firebaseapp.com",
  databaseURL: "https://your-project-default-rtdb.firebaseio.com/",
  projectId: "your-project-id",
  storageBucket: "your-project.appspot.com",
  messagingSenderId: "123456789",
  appId: "your-app-id"
};
```

### 3.2 Update ESP32 Configuration
Edit `include/config.h`:
```cpp
// Firebase credentials - UPDATE THESE
#define API_KEY "your-api-key-here"
#define DATABASE_URL "https://your-project-default-rtdb.firebaseio.com/"
```

## Step 4: Database Structure

### 4.1 Expected Data Structure
```
your-project-default-rtdb/
└── Servo1/
    ├── sensors/
    │   ├── accelX: float
    │   ├── accelY: float
    │   ├── accelZ: float
    │   ├── gyroX: float
    │   ├── gyroY: float
    │   ├── gyroZ: float
    │   ├── distance: float
    │   ├── vibration: int
    │   ├── latitude: float
    │   ├── longitude: float
    │   ├── crashSeverity: int
    │   ├── crashDetected: boolean
    │   └── timestamp: int
    ├── emergency/
    │   └── [timestamp]/
    │       ├── timestamp: int
    │       ├── severity: int
    │       ├── latitude: float
    │       ├── longitude: float
    │       ├── accelMagnitude: float
    │       ├── gyroMagnitude: float
    │       ├── distance: float
    │       └── vibration: int
    ├── crashStatus: int
    └── emergencyActive: boolean
```

### 4.2 Initialize Database (Optional)
You can manually add initial values:
1. Go to Realtime Database in Firebase console
2. Click on the "+" icon next to your database URL
3. Add the initial structure:

```json
{
  "Servo1": {
    "sensors": {
      "accelX": 0,
      "accelY": 0,
      "accelZ": 0,
      "gyroX": 0,
      "gyroY": 0,
      "gyroZ": 0,
      "distance": 0,
      "vibration": 0,
      "latitude": 0,
      "longitude": 0,
      "crashSeverity": 0,
      "crashDetected": false,
      "timestamp": 0
    },
    "crashStatus": 0,
    "emergencyActive": false
  }
}
```

## Step 5: Authentication Setup (Optional)

### 5.1 Anonymous Authentication
For basic security without user management:

1. Go to "Authentication" in Firebase console
2. Click "Get started"
3. Navigate to "Sign-in method" tab
4. Enable "Anonymous" provider
5. Save changes

### 5.2 Update ESP32 Code for Authentication
The current code uses anonymous authentication by default:
```cpp
if (Firebase.signUp(&config, &auth, "", "")) {
  Serial.println("Firebase SignUp OK");
  signupOK = true;
}
```

### 5.3 Custom Token Authentication (Advanced)
For production systems, consider implementing custom tokens:

```cpp
// In firebase_manager.cpp
String customToken = "your-custom-token";
if (Firebase.signUp(&config, &auth, customToken)) {
  // Handle authentication
}
```

## Step 6: Testing Firebase Connection

### 6.1 Upload Test Code
1. Update `include/config.h` with your Firebase credentials
2. Upload the code to ESP32
3. Open Serial Monitor at 115200 baud
4. Look for Firebase connection messages

### 6.2 Verify Data Flow
1. Watch Serial Monitor for "Firebase connected successfully"
2. Check Firebase console for incoming sensor data
3. Verify data updates every 5 seconds

### 6.3 Test Emergency Alerts
1. Manually trigger crash detection (shake the device)
2. Check for emergency data in Firebase console
3. Verify `crashStatus` and `emergencyActive` flags update

## Step 7: Monitoring and Analytics

### 7.1 Firebase Console Monitoring
- **Realtime Database**: Monitor live data updates
- **Usage Tab**: Track read/write operations
- **Rules Tab**: Monitor security rule hits

### 7.2 Set Up Alerts (Optional)
1. Go to "Alerts" in Firebase console
2. Create alerts for:
   - High database usage
   - Security rule violations
   - Failed authentication attempts

### 7.3 Export Data
For data analysis:
1. Go to Realtime Database
2. Click "..." menu next to data node
3. Select "Export JSON"
4. Save for offline analysis

## Troubleshooting

### Common Issues

#### "Permission Denied" Errors
- Check database security rules
- Verify authentication is working
- Ensure correct database URL

#### Connection Timeouts
- Check WiFi connectivity
- Verify API key is correct
- Test with simple Firebase example

#### Data Not Updating
- Check ESP32 serial output for errors
- Verify database rules allow writes
- Check Firebase quota limits

#### Authentication Failures
- Verify anonymous auth is enabled
- Check API key validity
- Review authentication code

## Security Best Practices

### Production Security Checklist

1. **Restrict Database Rules**: Don't use test mode in production
2. **Enable Authentication**: Use proper auth methods
3. **Monitor Access**: Set up alerts for unusual activity
4. **Regular Backups**: Export data regularly
5. **API Key Security**: Don't expose keys in public repositories
6. **Network Security**: Use HTTPS only
7. **Input Validation**: Validate all incoming data

### Sample Production Rules
```json
{
  "rules": {
    "Servo1": {
      ".read": "auth != null && auth.uid == 'allowed-device-id'",
      ".write": "auth != null && auth.uid == 'allowed-device-id'",
      "sensors": {
        ".validate": "newData.hasChildren(['timestamp']) && newData.child('timestamp').isNumber()"
      },
      "emergency": {
        ".validate": "newData.hasChildren(['timestamp', 'severity']) && newData.child('severity').isNumber() && newData.child('severity').val() >= 1 && newData.child('severity').val() <= 3"
      }
    }
  }
}
```

## Cost Considerations

### Firebase Pricing
- **Spark Plan (Free)**: 1GB stored, 10GB/month transfer
- **Blaze Plan (Pay-as-you-go)**: $5/GB stored, $1/GB transfer

### Optimization Tips
1. **Batch Writes**: Group multiple sensor updates
2. **Data Retention**: Implement automatic cleanup of old data
3. **Selective Updates**: Only send changed values
4. **Compression**: Use efficient data formats

## Next Steps

After successful Firebase setup:
1. Test all sensor data flows
2. Implement proper error handling
3. Set up monitoring dashboard
4. Plan for data retention and cleanup
5. Implement proper authentication for production use
