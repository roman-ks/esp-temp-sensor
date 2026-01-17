/*
    Go to thingspeak.com and create an account if you don't have one already.
    After logging in, click on the "New Channel" button to create a new channel for your data. This is where your data will be stored and displayed.
    Fill in the Name, Description, and other fields for your channel as desired, then click the "Save Channel" button.
    Take note of the "Write API Key" located in the "API keys" tab, this is the key you will use to send data to your channel.
    Replace the channelID from tab "Channel Settings" and privateKey with "Read API Keys" from "API Keys" tab.
    Replace the host variable with the thingspeak server hostname "api.thingspeak.com"
    Upload the sketch to your ESP32 board and make sure that the board is connected to the internet. The ESP32 should now send data to your Thingspeak channel at the intervals specified by the loop function.
    Go to the channel view page on thingspeak and check the "Field1" for the new incoming data.
    You can use the data visualization and analysis tools provided by Thingspeak to display and process your data in various ways.
    Please note, that Thingspeak accepts only integer values.

    You can later check the values at https://thingspeak.com/channels/2005329
    Please note that this public channel can be accessed by anyone and it is possible that more people will write their values.
 */

#include <WiFi.h>
#include "esp_log.h"
#include "wifi_creds.h"

const char *ssid = SSID;          // Change this to your WiFi SSID
const char *password = PASSWORD;  // Change this to your WiFi password


void setup() {
  Serial.begin(115200);

  // We start by connecting to a WiFi network

  Serial.println();
  ESP_LOGI("APP","******************************************************");
  ESP_LOGI("APP","Connecting to ");
  ESP_LOGI("APP", "SSID: %s", ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    ESP_LOGI("APP",".");
  }

  ESP_LOGI("APP","");
  ESP_LOGI("APP","WiFi connected");
  ESP_LOGI("APP","IP address: %s", WiFi.localIP().toString().c_str());
}

void readResponse(NetworkClient *client) {
  unsigned long timeout = millis();
  while (client->available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client->stop();
      return;
    }
  }
}

void loop() {
}
