#include "main.h"

int g_tick = 0;

void setup()
{
	Serial.begin(115200);
	Serial.println("reached setup()");
}

TICK g_tickFrame = 0;

void loop()
{
	Clock::Update();
	
	if (int(Clock::g_tFramePrev) != int(Clock::g_tFrame))
	{
		Serial.printf("t: %05.6f (%0.6f)\n", Clock::g_tFrame, Clock::g_dTFrame);
	}
}
