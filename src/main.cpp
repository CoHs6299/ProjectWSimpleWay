#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <Hash.h>
const char* ssid = "Tommy's iPhone";
const char* password = "18615292015";
char host[] = "sleepy-beyond-26290.herokuapp.com";
int port = 80;
int pingCount = 0;
String currentState;
char path[] = "/ws";
ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
DynamicJsonBuffer jsonBuffer;

void processWebScoketRequest(String data)
{
  JsonObject& root = jsonBuffer.parseObject(data);
  String device = (const char*)root["device"];
  String location = (const char*)root["location"];
  String state = (const char*)root["state"];
  String query = (const char*)root["query"];
  String message = "";
  Serial.println(data);
  Serial.println(state);
  if (query=="cmd")
  {
    Serial.println("Recived command!");
    if (state="on")
    {
      digitalWrite(2, HIGH);
      delay(500);
      digitalWrite(2,LOW);
      currentState = "ON";
    }
    else
    {
      digitalWrite(2, LOW);
      delay(1000);
      digitalWrite(2,HIGH);
      message = "{\"state\":\"OFF\"}";
      currentState = "OFF";
    }
  }
  else if(query=="?")
  {
    Serial.println("Recived query!");
    int state = digitalRead(2);
    if (currentState=="ON")
    {
      message = "{\"state\":\"ON\"}";
    }
    else
    {
      message = "{\"state\":\"OFF\"}";
    }
  }
  else
  {
    Serial.println("Command is not recognized");
  }
  Serial.println("Sending response back");
  Serial.println(message);
  webSocket.sendTXT(message);
}
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length)
{
  switch (type)
  {
    case WStype_DISCONNECTED:
      Serial.println("Disconnected! ");
      Serial.println("Connecting...");
      webSocket.begin(host, port, path);
      webSocket.onEvent(webSocketEvent);
      break;
    case WStype_CONNECTED:
      Serial.println("Connected to heroku service! ");
      webSocket.sendTXT("connected");
      break;
    case WStype_TEXT:
      Serial.println("Got data");
      processWebScoketRequest((char*)payload);
      break;
  }
}
void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  WiFiMulti.addAP(ssid, password);
  while (WiFiMulti.run()!=WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println("Connected to WiFi");
  webSocket.begin(host, port, path);
  webSocket.onEvent(webSocketEvent);
}

void loop()
{
  webSocket.loop();
  delay(2000);
  if (pingCount > 20)
  {
    pingCount = 0;
    webSocket.sendTXT("\"heartbeat\":\"keepalive\"");
  }
  else
  {
    pingCount += 1;
  }
}
