#include "main.h"

void setup()
{
	Trace::Startup();
	OnBoard::Startup();
	Input::Startup();
	BlueTooth::Startup();
}

void loop()
{
	Clock::Update();
	OnBoard::Update();
	Input::Update();
	BlueTooth::Update();

	static const bool s_fTrace = false;

	TRACE(
		s_fTrace && Clock::g_tFrame < 20.0f && int(Clock::g_tFramePrev) != int(Clock::g_tFrame),
		"t: %05.6f (%0.6f)\n",
		Clock::g_tFrame,
		Clock::g_dTFrame);
}
