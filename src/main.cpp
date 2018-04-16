#include "ThingSpeak.h"
#include "config.h"

int current = 0;
int last = 0;

char recData[9];
int counter = 0;
int sensorData[6];

WiFiClient client;
bool toggle = false;
bool sign = false;
bool timeRequest = false;
bool alarm = false;

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

void setup()
{
	pinMode(2, OUTPUT);
	pinMode(BUTTON_PIN, INPUT_PULLUP);
	pinMode(LED_PIN, OUTPUT);
	Serial.begin(115200);
	Serial.setDebugOutput(true);
	setupWiFi();
	setupAdafruitIO();

	noInterrupts();
	timer0_isr_init();
	timer0_attachInterrupt(timer0_ISR);
	timer0_write(ESP.getCycleCount() + 80000000 * 15);
	interrupts();

	ThingSpeak.begin(client);
}
void loop()
{
	Serial.println(ThingSpeak.readIntField(commandChannelNumber, 2));
	if (toggle)
	{
		for (size_t i = 1; i < 7; i++)
		{
			ThingSpeak.setField(i, sensorData[(i-1)]);
		}
		ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
	}
	io.run();
	if (alarm == true)
	{
		Serial.println("Sending alarm message...");
		sendMessageToAIO(1);
	}
}

void serialEvent()
{
	while (Serial.available())
	{
		recData[counter] = (char)Serial.read();
		if(counter == 0 && recData[0] != 0x55)
		return;
		counter++;
		if(counter == 8)
		{
			counter = 0;
			sign = true;
		}
		if(sign)
		{
			sign = 0;
			if(recData[0] == 0x55 && recData[8] == 0x58)
			{
				switch (recData[1])
				{
					case 0x01:
						for (size_t i = 0; i < 6; i++)
						{
							sensorData[i] = recData[i+2];
						}
						break;
					case 0x02:
						timeRequest = true;
						break;
					case 0x03:
						alarm = true;
						break;
					default:
						break;
				}
			}
		}
	}
}
