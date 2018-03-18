#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include "ThingSpeak.h"
#include "config.h"
unsigned long myChannelNumber = 449055;
const char * myWriteAPIKey = "GATS3K9WRX380X84";
int current = 0;
int last = 0;
float voltage = 10.2;
WiFiClient client;
char host[] = "test-project-w.herokuapp.com";
int pingCount = 0;
int port = 80;
char path[] = "/ws";
WebSocketsClient webSocket;
DynamicJsonBuffer jsonBuffer;
bool toggle = false;
void timer0_ISR()
{
	if (toggle)
	{
		digitalWrite(2, HIGH);
		toggle = false;
	}
	else
	{
		digitalWrite(2, LOW);
		toggle = true;
	}
	timer0_write(ESP.getCycleCount() + 80000000L * 15);
}
void processWebScoketRequest(String data)
{
	String jsonResponse = "{\"version\": \"1.0\",\"sessionAttributes\": {},\"response\": {\"outputSpeech\": {\"type\": \"PlainText\",\"text\": \"<text>\"},\"shouldEndSession\": true}}";
	JsonObject& req = jsonBuffer.parseObject(data);
	String type = req["type"];
	Serial.println(type);
	Serial.println("Data-->" + data);
	if (type == "LaunchRequest")
	{
		Serial.println("Recieved LaunchRequest!");
		jsonResponse.replace("<text>", "Welcome to the solar manager skill. You can monitor your solar station by saying, report the station's status");
		jsonResponse.replace("true", "false");
		Serial.print("Sending response back");
		Serial.println(jsonResponse);
		webSocket.sendTXT(jsonResponse);
	}
	else if (type == "IntentRequest")
	{
		String query = req["query"];
		if (query == "status")
		{
			Serial.println("Recieved query!");
			jsonResponse.replace("<text>", "All looks good");
			Serial.print("Sending response back");
			Serial.println(jsonResponse);
			webSocket.sendTXT(jsonResponse);
		}
		else if (query == "temperature")
		{
			Serial.println("Recieved command!");
			jsonResponse.replace("<text>", "Don't worry, solar panel's temperature is 26 degrees celsius");
			Serial.print("Sending response back");
			Serial.println(jsonResponse);
			webSocket.sendTXT(jsonResponse);
		}
		else
		{
			Serial.println("Command is not recognized!");
			jsonResponse.replace("<text>", "Command is not recognized by garage door Alexa skill");
			Serial.print("Sending response back");
			Serial.println(jsonResponse);
			webSocket.sendTXT(jsonResponse);
		}
	}
	else
	{
		Serial.println("Command is not recognized!");
		jsonResponse.replace("<text>", "Command is not recognized by garage door Alexa skill");
		Serial.print("Sending response back");
		Serial.println(jsonResponse);
		webSocket.sendTXT(jsonResponse);
	}
	webSocket.sendTXT(jsonResponse);
}
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length)
{
	switch (type) {
	case WStype_DISCONNECTED:
		Serial.println("Disconnected! ");
		break;
	case WStype_CONNECTED:
	{
		Serial.println("Connected! ");
		webSocket.sendTXT("Connected");
	}
	break;
	case WStype_TEXT:
	{
		Serial.println("Got data");
		processWebScoketRequest((char*)payload);
	}
	break;
	case WStype_BIN:
		hexdump(payload, length);
		Serial.print("Got bin");
		break;
	}
}
void setup()
{
	pinMode(2, OUTPUT);
	pinMode(BUTTON_PIN, INPUT_PULLUP);
	pinMode(LED_PIN, OUTPUT);
	Serial.begin(115200);
	Serial.setDebugOutput(true);
	setupWiFi();
	setupAdafruitIO();
	webSocket.begin(host, port, path);
	webSocket.onEvent(webSocketEvent);
	noInterrupts();
	timer0_isr_init();
	timer0_attachInterrupt(timer0_ISR);
	timer0_write(ESP.getCycleCount() + 80000000 * 15);
	interrupts();
	ThingSpeak.begin(client);
}
void loop()
{
	webSocket.loop();
	if (toggle)
	{
		voltage += 12.36;
		Serial.println(voltage);
		ThingSpeak.writeField(myChannelNumber, 2, voltage, myWriteAPIKey);
	}
	io.run();
	if (digitalRead(BUTTON_PIN) == LOW)
		current = 1;
	else
		current = 0;
	if (current == last)
		return;
	Serial.print("sending button -> ");
	Serial.println(current);
  sendMessageToAIO(current);
	last = current;
}
