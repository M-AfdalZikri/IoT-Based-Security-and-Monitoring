# IoT-Based-Security-and-Monitoring

## Overview
This project involves an IoT system consisting of two interconnected devices designed to enhance safety and monitor activities across different rooms via an internet connection (e.g., WiFi).

### Device 1: Hazard Detection (Gas and Fire Sensors)
- **Location**: Meeting room or other high-risk areas.
- **Components**: 
  - **Gas Sensor**: Detects gas leaks.
  - **Fire Sensor**: Detects the presence of fire or smoke.
- **Function**: 
  - Detects potential hazards such as gas leaks and fires.
  - Sends a warning signal to the connected device (Device 2) in case of a detected threat.

### Device 2: Security Monitoring and Alarm System
- **Location**: Security room.
- **Components**:
  - **ESP32-CAM**: 
    - Captures images at specified intervals.
    - Sends images to the supervisor's Telegram for monitoring security staff activities.
  - **PIR Sensor**: 
    - Detects motion in the security room.
    - Triggers the camera to capture and send images of security personnel in action.
  - **Alarm**: 
    - Receives hazard signals from Device 1.
    - Activates to alert security personnel to respond to the hazardous area.

## Features
- **Real-Time Hazard Detection**: Ensures a rapid response to gas leaks and fires.
- **Security Monitoring**: Increases accountability of security personnel.
- **Automation**: Uses IoT connectivity for seamless communication between devices.

## Applications
- Ideal for office buildings, factories, or facilities requiring security and safety systems.

## Technologies Used
- **ESP32-CAM** for image capturing.
- **PIR Sensor** for motion detection.
- **Gas Sensor** and **Fire Sensor** for hazard detection.
- **WiFi** for IoT communication.

## Installation and Usage
1. Set up both devices in their designated locations.
2. Configure WiFi, ESP32CAM, and Telegram integration.
3. Test sensors and alarms to ensure system readiness.
4. Monitor the system through alerts and images received.

## Future Enhancements
- Add smoke detection for better hazard identification.
- Integrate with cloud storage for extended image logging.
- Implement mobile app notifications for better accessibility.

---
Feel free to contribute or suggest improvements by opening an issue or submitting a pull request!
