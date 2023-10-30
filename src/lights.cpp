#include "lights.h"

#include "clock.h"
#include "trace.h"

#include "FastLED.h"

namespace Lights
{
	constexpr bool s_fTrace = true;

	CRGB g_aRgb[300];
	constexpr int s_pin = 32;

	// NOTE bruceo: it's tempting to use FastLED.setMaxPowerInMilliWatts() to prevent
	//	LED power (over) consumption from killing the processor. but the milliwatt limiter
	//	is super dynamic, dialing the whole string up or down based on the current colors.
	// i prefer a fixed maximum so that patterns can set their own dynamic range.
	// to get the max brightness where all white won't kill the system, follow these steps:
	//	- set ENABLE_LIGHTS_WATTAGE_TEST below to 1.
	//	- set ENABLE_ONBOARD_NEOPIXEL in onboard.cpp to 0, to take the onboard LED out of the equation.
	//	- set ENABLE_FASTLED_SHOW in main.cpp to 0, to prevent spamming of TTY with power info.
	//	- set POWER_DEBUG_PRINT in FastLED's power_mgt.cpp to 1.
	// then run, the app and you should see a trace of FastLED's power calculation, including
	//	a "recommended brightness" between 1 and 255. use that as s_brightMost below.
	// you may want to tweak s_ampMax below until you get a good "white" from your strip. i found that
	//	some WS2813 strips will "draw" 2amps but the colors will be weird at the end of the strip.
	// (be sure to undo the changes above) 

#define ENABLE_LIGHTS_WATTAGE_TEST 0

#if ENABLE_LIGHTS_WATTAGE_TEST
	constexpr float s_volts = 3.7f;
	constexpr float s_ampMax = 0.75f;
	constexpr U32 s_mwattMax = U32(s_volts * s_ampMax * 1e3f);
#else // !ENABLE_LIGHTS_WATTAGE_TEST
	constexpr U8 s_brightMost = 11;	// 0.75amps on a superlightingled.com thin WS2813-5V-5050RGBX60
#endif // !ENABLE_LIGHTS_WATTAGE_TEST

	CLEDController & g_ledc = FastLED.addLeds<WS2813, s_pin, GRB>(g_aRgb, DIM(g_aRgb));
}

using namespace Lights;

void Lights::Startup()
{
#if ENABLE_LIGHTS_WATTAGE_TEST
	FastLED.setMaxPowerInMilliWatts(s_mwattMax);

	for (int i = 0; i < DIM(g_aRgb); ++i)
	{
		g_aRgb[i] = CRGB(255,255,255);
	}

	FastLED.show();
#else // !ENABLE_LIGHTS_WATTAGE_TEST
	FastLED.setBrightness(s_brightMost);
#endif // !ENABLE_LIGHTS_WATTAGE_TEST
}

void Lights::Update()
{
#if ENABLE_LIGHTS_WATTAGE_TEST
	return;
#endif // ENABLE_LIGHTS_WATTAGE_TEST

	constexpr float s_dTMarch = 0.01f;

	if (!Clock::FHeartBeat(s_dTMarch))
		return;

	constexpr U32 s_cRgbChunk = DIM(g_aRgb) + 1;
	U32 cHeartBeat = Clock::CHeartBeat(s_dTMarch);
	U32 cChunk = cHeartBeat / s_cRgbChunk;
	U32 cRgb = cHeartBeat % s_cRgbChunk;

	if (cRgb == 0)
	{
		ClearArray(g_aRgb);
		return;
	}

	static const U8 s_aHuePattern[] =
	{
		HUE_RED,
		HUE_GREEN,
		HUE_BLUE,
		HUE_RED,	// sat below will ignore this hue
	};
	static const U8 s_aSatPattern[] =
	{
		255,
		255,
		255,
		0,
	};
	static_assert(DIM(s_aHuePattern) == DIM(s_aSatPattern));

	int i = cChunk % DIM(s_aHuePattern);
	U8 hue = s_aHuePattern[i];
	U8 sat = s_aSatPattern[i];
	static const U8 s_val = 255;

	int cRgbTodo = min(DIM(g_aRgb), cRgb);

	for (int i = 0; i < cRgbTodo; ++i)
	{
		g_aRgb[i].setHSV(hue, sat, s_val);
	}

	// NOTE bruceo: FastLED.show() done in loop() so multiple modules all get shown/synced at once.
}