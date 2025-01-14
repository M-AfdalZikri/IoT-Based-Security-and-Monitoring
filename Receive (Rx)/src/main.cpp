#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <ESPAsyncWebServer.h>
#include <UniversalTelegramBot.h>
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include "soc/soc.h"
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// Change these to your network credentials
const char *ssid = "Your_SSID";
const char *password = "Your_Password";

// Change these to your Telegram Bot details
String BOTtoken = "Your_BOT_Token"; // Initialize your Telegram Bot token
String CHAT_ID = "Your_Chat_ID";    // Initialize your Telegram chat ID

AsyncWebServer server(80);

WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

#define ALARM_PIN 14 // Pin for alarm

// Set up bot request delay time and photo interval
unsigned long botRequestDelay = 1000; // Check for Telegram messages every 1 second
unsigned long lastTimeBotRan;         // Store the last time a message was checked
unsigned long PhotoInterval = 60000;  // Default photo interval (1 minute)
unsigned long lastPhotoTime;          // Store the last time a photo was taken

bool alarmActive = false; // Track whether the alarm is active or not
bool sendPhoto = false;   // Track whether sending a photo is active or not

void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("\nIP Address: ");
  Serial.print(WiFi.localIP());

  // Send the IP address to the user via Telegram
  String ipAddress = WiFi.localIP().toString();
  String message = "Camera is ready. \nType '/start' to begin.\nSensor IP Address: http://" + ipAddress;
  bot.sendMessage(CHAT_ID, message, "");
}

void configInitCamera()
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  // config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // Camera initialization
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void handleNewMessages(int numNewMessages)
{
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID)
    {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    // Messages sent on Telegram
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;
    if (text == "/start")
    {
      String welcome = "Welcome, " + from_name + "\n";
      welcome += "Please specify the time interval for photo capture on the ESP32-CAM\n";
      welcome += "/60000 : Capture photo every 1 minute (default)\n";
      welcome += "/120000 : Capture photo every 2 minutes\n";
      welcome += "/180000 : Capture photo every 3 minutes\n";
      welcome += "/240000 : Capture photo every 4 minutes\n";
      welcome += "/300000 : Capture photo every 5 minutes\n";
      bot.sendMessage(CHAT_ID, welcome, "");
    }
    if (text == "/60000")
    {
      String message;
      PhotoInterval = 60000;
      Serial.println("Capture photo every 1 minute");
      message += "Capture photo every 1 minute";
      bot.sendMessage(CHAT_ID, message);
    }
    if (text == "/120000")
    {
      String message;
      PhotoInterval = 120000;
      Serial.println("Capture photo every 2 minutes");
      message += "Capture photo every 2 minutes";
      bot.sendMessage(CHAT_ID, message);
    }
    if (text == "/180000")
    {
      String message;
      PhotoInterval = 180000;
      Serial.println("Capture photo every 3 minutes");
      message += "Capture photo every 3 minutes";
      bot.sendMessage(CHAT_ID, message);
    }
    if (text == "/240000")
    {
      String message;
      PhotoInterval = 240000;
      Serial.println("Capture photo every 4 minutes");
      message += "Capture photo every 4 minutes";
      bot.sendMessage(CHAT_ID, message);
    }
    if (text == "/300000")
    {
      String message;
      PhotoInterval = 300000;
      Serial.println("Capture photo every 5 minutes");
      message += "Capture photo every 5 minutes";
      bot.sendMessage(CHAT_ID, message);
    }
  }
}

String sendPhotoTelegram()
{
  const char *myDomain = "api.telegram.org";
  String getAll = "";
  String getBody = "";

  // Dispose first picture because of bad quality
  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get();
  esp_camera_fb_return(fb); // dispose the buffered image

  // Take a new photo
  fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb)
  {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }

  Serial.println("Connect to " + String(myDomain));

  if (clientTCP.connect(myDomain, 443))
  {
    Serial.println("Connection successful");

    String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + CHAT_ID + "\r\n--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--RandomNerdTutorials--\r\n";

    size_t imageLen = fb->len;
    size_t extraLen = head.length() + tail.length();
    size_t totalLen = imageLen + extraLen;

    clientTCP.println("POST /bot" + BOTtoken + "/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=RandomNerdTutorials");
    clientTCP.println();
    clientTCP.print(head);

    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n = 0; n < fbLen; n = n + 1024)
    {
      if (n + 1024 < fbLen)
      {
        clientTCP.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen % 1024 > 0)
      {
        size_t remainder = fbLen % 1024;
        clientTCP.write(fbBuf, remainder);
      }
    }

    clientTCP.print(tail);

    esp_camera_fb_return(fb);

    int waitTime = 10000; // timeout 10 seconds
    long startTimer = millis();
    boolean state = false;

    while ((startTimer + waitTime) > millis())
    {
      Serial.print("...");
      delay(100);
      while (clientTCP.available())
      {
        char c = clientTCP.read();
        if (state == true)
          getBody += String(c);
        if (c == '\n')
        {
          if (getAll.length() == 0)
            state = true;
          getAll = "";
        }
        else if (c != '\r')
          getAll += String(c);
        startTimer = millis();
      }
      if (getBody.length() > 0)
        break;
    }
    clientTCP.stop();
    Serial.println(getBody);
  }
  else
  {
    getBody = "Connected to api.telegram.org failed.";
    Serial.println("Connected to api.telegram.org failed.");
  }
  return getBody;
}

void task()
{
  // Prevent multiple photo sends and check for new Telegram messages
  if (sendPhoto)
  {
    Serial.println("Preparing photo");
    sendPhotoTelegram();
    sendPhoto = false;
  }
  if (millis() > lastTimeBotRan + botRequestDelay)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  // Logic for taking photos based on the time interval
  if (millis() - lastPhotoTime >= PhotoInterval)
  {
    sendPhoto = true;
    lastPhotoTime = millis();
  }
}

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Disable brownout detector

  Serial.begin(115200);

  // Setup PIN
  pinMode(ALARM_PIN, OUTPUT);
  digitalWrite(ALARM_PIN, LOW);

  initWiFi();
  configInitCamera();

  // Handle Web Server
  server.on("/alarm/on", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    digitalWrite(ALARM_PIN, HIGH); // Turn on alarm
    alarmActive = true;
    request->send(200, "text/plain", "Alarm ON"); 
    Serial.println("Alarm ON"); });

  server.on("/alarm/off", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    digitalWrite(ALARM_PIN, LOW); // Turn off alarm
    alarmActive = false;
    request->send(200, "text/plain", "Alarm OFF"); 
    Serial.println("Alarm OFF"); });

  server.begin();
}

void loop()
{
  task();
}
