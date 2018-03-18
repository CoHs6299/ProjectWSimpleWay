#include <ESP8266WiFi.h>
#include "AdafruitIO_WiFi.h"

#define IO_USERNAME    "tommy_w2"
#define IO_KEY         "7b0e850df29440a08cbe2f9434a29ba0"
#define WIFI_SSID       "Xperia Z3"
#define WIFI_PASS       "18615292015"
#define BUTTON_PIN 4
#define LED_PIN 2

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
AdafruitIO_Feed *command = io.feed("command");

void setupWiFi()
{
  Serial.println("Connecting WiFi......");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
   delay(500);
   Serial.println(". ");
  }
  Serial.println("Connected to wi-fi");
}
void setupAdafruitIO()
{
  Serial.println("Connecting to Adafruit IO");
  io.connect();
  while(io.status() < AIO_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println(io.statusText());
}
void sendMessageToAIO(int current)
{
  command->save(current);
}
