#include "main.h"

int g_tick = 0;

void setup()
{
	Serial.begin(MONITOR_SPEED);
	Serial.println("reached setup()");
}

TICK g_tickFrame = 0;

void loop()
{
	Clock::Update();

	if (Clock::g_tFrame < 20.0f && int(Clock::g_tFramePrev) != int(Clock::g_tFrame))
	{
		Serial.printf("t: %05.6f (%0.6f)\n", Clock::g_tFrame, Clock::g_dTFrame);
	}
}
