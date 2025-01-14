#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Replace these with the credentials you want to use
const char *ssid = "Your_SSID";         // Enter your WiFi SSID here
const char *password = "Your_Password"; // Enter your WiFi password here

#define fire_threshold 1000 // Set the fire detection threshold here
#define gas_threshold 1300  // Set the gas detection threshold here

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create Event Source on /events
AsyncEventSource events("/events");

// Sensor pins
const int gasSensor = 34;
const int fireSensor = 35;

// ESP32 client IP address
const char *clientIP = "192.168.1.124"; // Enter the IP address of the ESP32 client

// Task handles
TaskHandle_t FireTaskHandle = NULL;
TaskHandle_t GasTaskHandle = NULL;

// Global sensor status
volatile bool fireDetected = false;
volatile bool gasDetected = false;

// WiFi initialization
void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println('.');
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP32 Sensor Notifications</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p { font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #50B8B4; color: white; font-size: 1rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); max-width: 600px; margin: 0 auto; }
    .status { font-size: 1.4rem; }
    .status.red { color: red; }
  </style>
</head>
<body>
  <div class="topnav">
    <h1>ESP32 Sensor Notifications</h1>
  </div>
  <div class="content">
    <div class="card">
      <p class="status" id="fire-status">Fire Status: Loading...</p>
      <p class="status" id="gas-status">Gas Status: Loading...</p>
    </div>
  </div>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');

 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);

 source.addEventListener('fire-status', function(e) {
  console.log("fire-status", e.data);
  var fireStatus = document.getElementById("fire-status");
  fireStatus.innerHTML = "Fire Status: " + e.data;
  if (e.data == "Fire detected") {
    fireStatus.classList.add("red");
  } else {
    fireStatus.classList.remove("red");
  }
 }, false);

 source.addEventListener('gas-status', function(e) {
  console.log("gas-status", e.data);
  var gasStatus = document.getElementById("gas-status");
  gasStatus.innerHTML = "Gas Status: " + e.data;
  if (e.data == "Gas detected") {
    gasStatus.classList.add("red");
  } else {
    gasStatus.classList.remove("red");
  }
 }, false);
}
</script>
</body>
</html>)rawliteral";

// Function to control the alarm
void controlAlarm()
{
  WiFiClient client;
  if (fireDetected || gasDetected)
  {
    if (client.connect(clientIP, 80))
    {
      client.print(String("GET ") + "/alarm/on" + " HTTP/1.1\r\n" +
                   "Host: " + clientIP + "\r\n" +
                   "Connection: close\r\n\r\n");
    }
  }
  else
  {
    if (client.connect(clientIP, 80))
    {
      client.print(String("GET ") + "/alarm/off" + " HTTP/1.1\r\n" +
                   "Host: " + clientIP + "\r\n" +
                   "Connection: close\r\n\r\n");
    }
  }
}

// Task for fire sensor
void FireTask(void *pvParameters)
{
  (void)pvParameters;
  int fireSensorStatus;
  String fireStatus;
  String fireValue;

  for (;;)
  {
    fireSensorStatus = analogRead(fireSensor);
    fireValue = String(fireSensorStatus);

    // Processing fire sensor status
    if (fireSensorStatus > fire_threshold)
    {
      fireStatus = "No fire";
      fireDetected = false;
      Serial.print("No fire | Value: ");
      Serial.println(fireSensorStatus);
    }
    else
    {
      fireStatus = "Fire detected";
      fireDetected = true;
      Serial.print("Fire detected | Value: ");
      Serial.println(fireSensorStatus);
    }

    // Send event to web client with sensor status and value
    events.send(fireStatus.c_str(), "fire-status", millis());
    events.send(fireValue.c_str(), "fire-value", millis());

    // Control alarm based on sensor status
    controlAlarm();

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// Task for gas sensor
void GasTask(void *pvParameters)
{
  (void)pvParameters;
  int gasSensorStatus;
  String gasStatus;
  String gasValue;

  for (;;)
  {
    gasSensorStatus = analogRead(gasSensor);
    gasValue = String(gasSensorStatus);

    // Processing gas sensor status
    if (gasSensorStatus > gas_threshold)
    {
      gasStatus = "Gas detected";
      gasDetected = true;
      Serial.print("Gas detected | Value: ");
      Serial.println(gasSensorStatus);
    }
    else
    {
      gasStatus = "No gas";
      gasDetected = false;
      Serial.print("No gas | Value: ");
      Serial.println(gasSensorStatus);
    }

    // Send event to web client with sensor status and value
    events.send(gasStatus.c_str(), "gas-status", millis());
    events.send(gasValue.c_str(), "gas-value", millis());

    // Control alarm based on sensor status
    controlAlarm();

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup()
{
  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  pinMode(fireSensor, INPUT);
  pinMode(gasSensor, INPUT);

  initWiFi();

  // Handle web server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html); });

  server.addHandler(&events);
  server.begin();

  // Create task for fire sensor
  xTaskCreatePinnedToCore(
      FireTask,        // Function to be executed as task
      "FireTask",      // Name for this task
      8192,            // Stack size for task (adjustable)
      NULL,            // Parameters passed to the task
      1,               // Task priority (higher value = higher priority)
      &FireTaskHandle, // Handle for this task
      0                // Core to run the task on (0 or 1)
  );

  // Create task for gas sensor
  xTaskCreatePinnedToCore(
      GasTask,        // Function to be executed as task
      "GasTask",      // Name for this task
      8192,           // Stack size for task (adjustable)
      NULL,           // Parameters passed to the task
      1,              // Task priority (higher value = higher priority)
      &GasTaskHandle, // Handle for this task
      0               // Core to run the task on (0 or 1)
  );
}

void loop()
{
  // Main loop is empty because all logic is handled in tasks
}
