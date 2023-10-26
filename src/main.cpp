#include "main.h"

void setup()
{
	Trace::Startup();
	OnBoard::Startup();
	Input::Startup();
	BlueTooth::Startup();
	Lights::Startup();
}

void loop()
{
	Clock::Update();
	OnBoard::Update();
	Input::Update();
	BlueTooth::Update();
	Lights::Update();

	static const bool s_fTrace = true;
	static const float s_tActive = 20.0f;
	bool fActive = (Clock::g_tFrame < s_tActive);
	bool fSecondRollover = Clock::FHeartBeat(1.0f);

	TRACE(
		s_fTrace && fActive && fSecondRollover,
		"t: %05.6f (%0.6f)\n",
		Clock::g_tFrame,
		Clock::g_dTFrame);
}
