#include <WiFi.h>
#include "esp_log.h"
#include <MQTT.h>
#include "wifi_creds.h"
#include <Wire.h>

const char *ssid = SSID;          // Change this to your WiFi SSID
const char *password = PASSWORD;  // Change this to your WiFi password
const int SHT20_ADDR = 0x40;


WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;


void connect() {
  ESP_LOGI("APP","checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    ESP_LOGI("APP",".");
    delay(1000);
  }

  ESP_LOGI("APP","\nconnecting...");
  while (!client.connect("arduino", "public", "public")) {
    ESP_LOGI("APP",".");
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
  client.begin("192.168.50.150", net);
  client.onMessage(messageReceived);

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
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    connect();
  }

  // publish a message roughly every second.
  if (millis() - lastMillis > 10000) {
    lastMillis = millis();

    float temp = measureTemp();
    client.publish("/home/esp32_0/temperature", String(temp).c_str());
    float humidity = measureHumidity();
    client.publish("/home/esp32_0/humidity", String(humidity).c_str());
  }
}
