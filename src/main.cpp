#include "main.h"

int g_tick = 0;

void setup()
{
	Trace::Startup();
	TRACE("reached setup()\n");
}

TICK g_tickFrame = 0;

void loop()
{
	Clock::Update();

	TRACE(
		Clock::g_tFrame < 20.0f && int(Clock::g_tFramePrev) != int(Clock::g_tFrame),
		"t: %05.6f (%0.6f)\n",
		Clock::g_tFrame,
		Clock::g_dTFrame);
}
