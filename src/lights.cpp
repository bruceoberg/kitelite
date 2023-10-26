#include "lights.h"

#include "clock.h"

#include "FastLED.h"

namespace Lights
{
	CRGB g_aRgb[300];
	constexpr int s_pin = 32;
}

using namespace Lights;

void Lights::Startup()
{
	FastLED.addLeds<NEOPIXEL, Lights::s_pin>(Lights::g_aRgb, DIM(Lights::g_aRgb));
}

void Lights::Update()
{
	constexpr float s_dTMarch = 0.1f;

	if (!Clock::FHeartBeat(s_dTMarch))
		return;

	constexpr U32 s_cRgbChunk = 11; // 0..10 leds at once
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
}