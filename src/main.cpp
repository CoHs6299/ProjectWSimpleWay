#include "ThingSpeak.h"
#include "config.h"
#include <time.h>

int recData[9] = {1,2,3,4,5,6,7,8,9};
int counter = 0;
int sensorData[6] = {26,105,20,32,28,32};
int alarmData[6] = {1,2,3,4,5,6};

WiFiClient client;
bool toggle = false;
bool sign = false;
bool alarm = false;
int command_current = 0;
int command_last = 0;

int timezone = 8 * 3600;
int dst = 0;
void timer0_ISR()
{
	toggle = true;
	timer0_write(ESP.getCycleCount() + 80000000L * 15);
}

void setup()
{
	Serial.begin(115200);
	setupWiFi();
	setupAdafruitIO();
	ThingSpeak.begin(client);
	configTime(timezone, dst, "ntp1.aliyun.com","time.nist.gov");
	while(!time(nullptr))
	{
	 Serial.print("*");
	 delay(100);
	}
	Serial.println("\nTime response....OK");

	noInterrupts();
	timer0_isr_init();
	timer0_attachInterrupt(timer0_ISR);
	timer0_write(ESP.getCycleCount() + 80000000L * 15);
	interrupts();
}
void loop()
{
	if (toggle)
	{
		toggle = false;
		for (size_t i = 1; i < 7; i++)
		{
			ThingSpeak.setField(i, sensorData[(i-1)]);
		}
		//Serial.println("Ready to send data to ThingSpeak");
		ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
	}

	while (Serial.available())
	{
		recData[counter] = (int)Serial.read();
		if(counter == 0 && recData[0] != 253)
		return;
		counter++;
		if(counter == 9)
		{
			counter = 0;
			sign = true;
		}
		if(sign)
		{
			sign = false;
			if(recData[0] == 253 && recData[8] == 254)
			{
				switch (recData[1])
				{
					case 1:
					{
						for (size_t i = 0; i < 6; i++)
						{
							sensorData[i] = recData[i+2];
							//Serial.print("Sensor data is ");
							//Serial.println(sensorData[i]);
						}
						break;
					}
					case 2:
					{
						for (size_t i = 0; i < 3; i++)
						{
							time_t now = time(nullptr);
							struct tm* p_tm = localtime(&now);
							Serial.write(253);
							Serial.write(1);
							Serial.write(p_tm->tm_mday);
							Serial.write(p_tm->tm_mon + 1);
							Serial.write(p_tm->tm_hour);
							Serial.write(p_tm->tm_min);
							Serial.write(254);
							delay(5);
						}
						break;
					}
					case 3:
					{
						//Serial.println("Status message received");
						for (size_t i = 0; i < 6; i++)
						{
							alarmData[i] = recData[i+2];
							//Serial.println(alarmData[i]);
							if (alarmData[i]==1)
							{
								alarm = true;
							}
							ThingSpeak.setField(i+1, alarmData[i]);
						}
						ThingSpeak.writeFields(alarmChannelNumber, alarmChannleKey);
						break;
					}
					default:
						break;
				}
			}
		}
	}

	command_current = ThingSpeak.readIntField(commandChannelNumber, 2);
	if(command_current != command_last)
	{
		if(command_current == 1)
		{
			Serial.println("Command is open");
			for (size_t i = 0; i < 2; i++)
			{
				Serial.write(253);
				Serial.write(2);
				for (size_t i = 0; i < 4; i++)
				{
					Serial.write(1);
				}
				Serial.write(254);
			}
		}
		else if(command_current == 0)
		{
			Serial.println("Command is close");
			for (size_t i = 0; i < 2; i++)
			{
				Serial.write(253);
				Serial.write(2);
				for (size_t i = 0; i < 4; i++)
				{
					Serial.write(0);
				}
				Serial.write(254);
			}
		}
		command_last = command_current;
	}

	io.run();
	if (alarm == true)
	{
		Serial.println("Sending alarm message...");
		sendMessageToAIO(1);
		alarm = false;
		sendMessageToAIO(0);
	}
}
