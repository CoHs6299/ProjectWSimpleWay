#include <ESP8266WiFi.h>
#include "AdafruitIO_WiFi.h"
#include "WiFiUdp.h"

#define IO_USERNAME    "tommy_w2"
#define IO_KEY         "7b0e850df29440a08cbe2f9434a29ba0"
#define WIFI_SSID       "Xperia Z3"
#define WIFI_PASS       "18615292015"

#define myChannelNumber 449055
#define myWriteAPIKey "GATS3K9WRX380X84"
#define commandChannelNumber 456947
#define commandWriteAPIKey "45ZP5FSW9J9JUR4Z"
#define alarmChannelNumber 489741
#define alarmChannleKey "YLMVNO879T9TZ6GE"


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
   Serial.print(". ");
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
