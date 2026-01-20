#include <WiFi.h>
#include "esp_log.h"
#include <MQTT.h>
#include "wifi_creds.h"
#include <Wire.h>
#include "esp_sleep.h"

#define uS_TO_S_FACTOR  1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP_S 5*60         // Time ESP32 will go to sleep (in seconds)

const char *ssid = SSID;          // Change this to your WiFi SSID
const char *password = PASSWORD;  // Change this to your WiFi password
const int SHT20_ADDR = 0x40;


WiFiClient net;
MQTTClient client;

float measureTemp();
float measureHumidity();

void deepSleep() {
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_S * uS_TO_S_FACTOR);
  ESP_LOGI("APP","Going to sleep now");
  delay(1000);
  client.disconnect();
  WiFi.disconnect();
  Wire.end();
  Serial.flush(); 
  delay(100);
  esp_deep_sleep_start();
}

void connect() {
  ESP_LOGI("APP","checking wifi...");
  unsigned long start = millis();
  while ((WiFi.status() != WL_CONNECTED)) {
    if((millis() - start > 10000)){
      ESP_LOGI("APP","Failed to connect to WiFi");
      deepSleep();
    }
    ESP_LOGI("APP","Connecting to wifi...");
    delay(1000);
  }


  ESP_LOGI("APP","connecting...");
  start = millis();
  while (!client.connect("arduino", "public", "public")) {
    if((millis() - start > 10000)){
      ESP_LOGI("APP","Failed to connect to MQTT broker");
      deepSleep();
    }
    ESP_LOGI("APP","Connecting to mqtt broker...");
    delay(1000);
  }

  ESP_LOGI("APP","\nconnected!");
}

void setup() {
  Serial.begin(115200);

  // We start by connecting to a WiFi network

  ESP_LOGI("APP","******************************************************");
  ESP_LOGI("APP","Connecting to ");
  ESP_LOGI("APP", "SSID: %s", ssid);

  WiFi.begin(ssid, password);
  client.begin(MQTT_SERVER, net);

  ESP_LOGI("APP","");
  ESP_LOGI("APP","WiFi connected");
  ESP_LOGI("APP","IP address: %s", WiFi.localIP().toString().c_str());

  connect();

  Wire.begin(0, 1);
    // 1. Soft reset
  Wire.beginTransmission(SHT20_ADDR);
  Wire.write(0xFE);  // Soft reset command
  Wire.endTransmission();
  delay(20);  // Small delay after reset

  client.loop();

  float temp = measureTemp();
  client.publish("/home/esp32_0/temperature", String(temp).c_str());
  float humidity = measureHumidity();
  client.publish("/home/esp32_0/humidity", String(humidity).c_str());

  deepSleep();
}

const int reqBytes = 3;

uint32_t measureRaw(uint32_t command){
  Wire.beginTransmission(SHT20_ADDR);
  Wire.write(command);
  Wire.endTransmission();

  delay(100);
  Wire.requestFrom(SHT20_ADDR, (uint8_t)reqBytes);
  if (Wire.available() == reqBytes) {
    uint8_t data[reqBytes];
    for (int i = 0; i < reqBytes; i++) {
      data[i] = Wire.read();
    }

    uint32_t raw = ((uint32_t)data[0] << 8) | ((uint32_t)data[1]) & 0xFFFC;

    return raw;
  } else {
    ESP_LOGI("APP","Read error");
    return 0;
  }
}

float measureTemp(){
  uint32_t raw = measureRaw(0xF3);
  if(raw == 0) 
    return NAN;
  return -46.85 + 175.72 * raw / 65536.0;
}

float measureHumidity(){
  uint32_t raw = measureRaw(0xF5);
  if(raw == 0) 
    return NAN;
  return -6.0 + 125.0 * raw / 65536.0;
}

void loop() {
}
