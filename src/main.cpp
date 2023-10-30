#include "main.h"

#include "FastLED.h"

#define ENABLE_FASTLED_SHOW 1

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

#if ENABLE_FASTLED_SHOW
	// showing LEDs after all updates so multiple modules (e.g. onboard and lights) can cooperate.

	FastLED.show();
#endif // ENABLE_FASTLED_SHOW

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
