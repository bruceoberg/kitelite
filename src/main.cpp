#include "main.h"

constexpr float s_dTFrameRateMax = 1.0f / 120.0f;
constexpr USEC s_dUsecFrameRateMax = DUsecFromDT(s_dTFrameRateMax);

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

	// enforce max frame rate. without this, FastLED controlled lights will be
	// chaotic when we run too fast (e.g. when bluetooth is disabled).

	USEC dUsecFrameRemaining = UsecNow() - Clock::g_usecFrame;
	U32 dMsecFrameRemaining = U32(dUsecFrameRemaining / 1000);

	delay(dMsecFrameRemaining);

	TRACE(
		s_fTrace && fActive && fSecondRollover,
		"t: %05.6f (%0.6f) delay: %dmsec\n",
		Clock::g_tFrame,
		Clock::g_dTFrame,
		dMsecFrameRemaining);
}
