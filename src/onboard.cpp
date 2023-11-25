#include "onboard.h"

#include "input.h"

#include "FastLED.h"

// NOTE bruceo: don't use neopixel if we can avoid it. doing so puts FastLED into "disable interrupts" mode
// which can mess with bluetooth.

#define ENABLE_ONBOARD_NEOPIXEL 0 // (!PLAT_FEATHER_S3_REVTFT)

namespace OnBoard
{
	// Digital IO pin connected to the button. This will be driven with a
	// pull-up resistor so the switch pulls the pin to ground momentarily.
	// On a high -> low transition the button press logic will execute.

	namespace Button
	{
		struct SButtonParams // tag: buttonp
		{
			U8			m_pin;	// pin number
			U8			m_mode;	// mode for pinMode() during Startup()
			U8			m_down;	// pin state meaning "down"
			Input::KEY	m_key;	// input key enum
		};

		static const SButtonParams s_aButtonp[] =
		{
#if PLAT_FEATHER_V2
			{ BUTTON, INPUT, LOW, Input::KEY_OnBoard0 },
#elif PLAT_FEATHER_S3
			{ 0, INPUT_PULLUP, LOW, Input::KEY_OnBoard0 },
#elif PLAT_FEATHER_S3_REVTFT
			{ 0, INPUT_PULLUP, LOW, Input::KEY_OnBoard0 },
			{ 1, INPUT_PULLDOWN, HIGH, Input::KEY_OnBoard1 },
			{ 2, INPUT_PULLDOWN, HIGH, Input::KEY_OnBoard2 },
#else
#error Unknown platform
#endif
		};
	}

	// Single neopixel

	namespace NeoPixel
	{
		constexpr int s_pin = PIN_NEOPIXEL;
		constexpr int s_cRgb = 1;
		CRGB g_rgb;

		using HUE = U8;	// 0 to 255. 0: red .. green .. blue .. red
		using SAT = U8;	// 0 to 255. pure grayscale .. pure hue
		using VAL = U8;	// 0 to 255. black .. full brightness

		constexpr HUE HUE_First = 0;
		constexpr HUE HUE_Last = 255;

		constexpr HUE DHUE_OneThird = HUE_Last / 3;
		constexpr HUE DHUE_OneSixth = HUE_Last / 6;

		constexpr HUE HUE_Red = HUE_First;
		constexpr HUE HUE_Green = HUE_Red + DHUE_OneThird;
		constexpr HUE HUE_Blue = HUE_Green + DHUE_OneThird;

		constexpr HUE HUE_Yellow = HUE_Red + DHUE_OneSixth;
		constexpr HUE HUE_Cyan = HUE_Green + DHUE_OneSixth;
		constexpr HUE HUE_Magenta = HUE_Blue + DHUE_OneSixth;

		constexpr SAT SAT_None = 0;
		constexpr SAT SAT_Full = 255;

		constexpr VAL VAL_None = 0;
		constexpr VAL VAL_Full = 255;
	}

	namespace ColorCycle
	{
		U8 r = 255;
		U8 g = 255;
		U8 b = 255;
	}
}

using namespace OnBoard;

void OnBoard::Startup()
{
	for (const auto & buttonp : Button::s_aButtonp)
	{
		pinMode(buttonp.m_pin, buttonp.m_mode);
	}

#if ENABLE_ONBOARD_NEOPIXEL
	FastLED.addLeds<NEOPIXEL, NeoPixel::s_pin>(&NeoPixel::g_rgb, NeoPixel::s_cRgb);
	FastLED.clear(true);
	FastLED.show();
#elif defined(NEOPIXEL_POWER)
	pinMode(NEOPIXEL_POWER, OUTPUT);
	digitalWrite(NEOPIXEL_POWER, LOW);
#endif // !ENABLE_ONBOARD_NEOPIXEL && defined(NEOPIXEL_POWER)
}

void OnBoard::Update()
{
	for (const auto & buttonp : Button::s_aButtonp)
	{
		Input::SetKeyDown(buttonp.m_key, (digitalRead(buttonp.m_pin) == buttonp.m_down));
	}

	bool fIsDown0 = Input::FIsKeyDown(Input::KEY_OnBoard0);
	bool fIsDown1 = Input::FIsKeyDown(Input::KEY_OnBoard1);
	bool fIsDown2 = Input::FIsKeyDown(Input::KEY_OnBoard2);
	NeoPixel::SAT sat = NeoPixel::SAT_None;
	NeoPixel::HUE hue = NeoPixel::HUE_Red; // ignored because of SAT_None;

	if (fIsDown0)
	{
		sat = NeoPixel::SAT_Full;
		hue = NeoPixel::HUE_Red;
	}
	else if (fIsDown1)
	{
		sat = NeoPixel::SAT_Full;
		hue = NeoPixel::HUE_Green;
	}
	else if (fIsDown2)
	{
		sat = NeoPixel::SAT_Full;
		hue = NeoPixel::HUE_Blue;
	}
	
	float dTCycle = 0.5f;
	float rT = PI / dTCycle;
	float uVal = 0.5f + 0.5f * sin(rT * TNow());	// pi seconds per cycle
	NeoPixel::VAL valMin = NeoPixel::VAL_Full / 8;
	NeoPixel::VAL valMax = NeoPixel::VAL_Full / 2;
	NeoPixel::VAL val = valMin + NeoPixel::VAL(uVal * (valMax - valMin));

	NeoPixel::g_rgb.setHSV(hue, sat, val);

	CRGB rgbCycle;
	NeoPixel::VAL valBrightMin = NeoPixel::VAL_Full / 2;
	NeoPixel::VAL valBrightMax = NeoPixel::VAL_Full;
	NeoPixel::VAL valBright = valBrightMin + NeoPixel::VAL(uVal * (valBrightMax - valBrightMin));
	rgbCycle.setHSV(hue, sat, valBright);
	ColorCycle::r = rgbCycle.r;
	ColorCycle::g = rgbCycle.g;
	ColorCycle::b = rgbCycle.b;

	// NOTE bruceo: FastLED.show() done in loop() so multiple modules all get shown/synced at once.
}