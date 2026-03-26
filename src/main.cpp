#include "main.h"
#include "screen_home.h"

#include "FastLED.h"

#define ENABLE_FASTLED_SHOW 1

void setup()
{
	Trace::Startup();
	OnBoard::Startup();
	Input::Startup();
	BlueTooth::Startup();
	Lights::Startup();
	Motion::Startup();
	Display::Startup();
	Screen::Startup();

#if ENABLE_DISPLAY
	static CScreenHome s_screenhome;
	Screen::Push(&s_screenhome);
#endif // ENABLE_DISPLAY
}

void loop()
{
	Clock::Update();
	OnBoard::Update();
	Input::Update();
	BlueTooth::Update();
	Lights::Update();
	Motion::Update();
	Display::Update();

#if ENABLE_FASTLED_SHOW
	// showing LEDs after all updates so multiple modules (e.g. onboard and lights) can cooperate.

	FastLED.show();
#endif // ENABLE_FASTLED_SHOW

	static const bool s_fTrace = !PLAT_FEATHER_S3_REVTFT;	// screen serves as heartbeat
	static const float s_tHeartBeat = 20.0f; // TTY heardbeat until this time
	bool fActive = (Clock::g_tFrame < s_tHeartBeat);
	bool fSecondRollover = Clock::FHeartBeat(1.0f);

	TRACE(
		s_fTrace && fActive && fSecondRollover && !ENABLE_DISPLAY, // with a display, don't worry about heartbeat TTY
		"t: %05.6f (%0.6f)\n",
		Clock::g_tFrame,
		Clock::g_dTFrame);
}
