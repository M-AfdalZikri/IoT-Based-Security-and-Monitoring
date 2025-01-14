# IoT-Based-Security-and-Monitoring

## Description
This project is an IoT system consisting of two interconnected devices designed to enhance safety and monitor activities across different rooms via an internet connection (e.g., WiFi). The system also provides the capability to monitor the status of gas and fire sensors through a website using the device's IP.

### Device 1: Hazard Detection (Gas and Fire Sensors)
- **Location**: Meeting room or other high-risk areas.
- **Components**: 
  - **Gas Sensor**: Detects the presence of gas leaks.
  - **Fire Sensor**: Detects the presence of fire or combustion.
- **Function**: 
  - Detects potential hazards such as gas leaks and fire.
  - Sends a warning signal to the second device (Device 2) if a threat is detected.
  - **Sensor Monitoring**: The status of the gas and fire sensors can be monitored through a website using the device's IP. Users can check for potential hazards in real-time.

### Device 2: Security Monitoring and Alarm System
- **Location**: Security room.
- **Components**:
  - **ESP32-CAM**: 
    - Captures images at specified intervals.
    - Sends images to the supervisor's Telegram to monitor security staff activities.
  - **PIR Sensor**: 
    - Detects motion in the security room.
    - Triggers the camera to capture and send images to the supervisor as proof of activity.
  - **Alarm**: 
    - Receives hazard signals from Device 1.
    - Activates to alert security personnel to respond to the threat location.

## Features
- **Real-Time Hazard Detection**: Ensures a quick response to gas leaks and fire.
- **Security Monitoring**: Enhances accountability of security personnel.
- **Automation**: Uses IoT connectivity for seamless communication between devices.
- **Website Monitoring**: Enables monitoring of gas and fire sensor status through a website using the device's IP.

## Applications
- Ideal for office buildings, factories, or facilities that require security and safety systems.

## Technologies Used
- **ESP32-CAM** for image capturing.
- **PIR Sensor** for motion detection.
- **Gas Sensor** and **Fire Sensor** for hazard detection.
- **WiFi** for IoT communication.
- **Website** for monitoring gas and fire sensor status.

## Installation and Usage
1. Install both devices in their designated locations.
2. Configure WiFi, ESP32CAM, and Telegram integration.
3. Set up the website to monitor the gas and fire sensor status using the device's IP.
4. Test the sensors and alarms to ensure the system is ready.
5. Monitor the system through alerts, images received, and sensor status on the website.

## Future Enhancements
- Add smoke detection for better hazard identification.
- Integration with cloud storage for extended image logging.
- Implement mobile app notifications for better accessibility.

---
Feel free to contribute or suggest improvements by opening an issue or submitting a pull request!
