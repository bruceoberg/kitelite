#include "onboard.h"

#include "input.h"

#include "FastLED.h"

#define ENABLE_ONBOARD_NEOPIXEL 1

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
#endif // ENABLE_ONBOARD_NEOPIXEL
}

void OnBoard::Update()
{
	for (const auto & buttonp : Button::s_aButtonp)
	{
		Input::SetKeyDown(buttonp.m_key, (digitalRead(buttonp.m_pin) == buttonp.m_down));
	}

#if ENABLE_ONBOARD_NEOPIXEL
	bool fIsDown0 = Input::FIsKeyDown(Input::KEY_OnBoard0);
	bool fIsDown1 = Input::FIsKeyDown(Input::KEY_OnBoard1);
	bool fIsDown2 = Input::FIsKeyDown(Input::KEY_OnBoard2);
	NeoPixel::HUE hue = fIsDown0
							? NeoPixel::HUE_Blue
							: (fIsDown1
								? NeoPixel::HUE_Red
								: (fIsDown2
									? NeoPixel::HUE_Yellow
									: NeoPixel::HUE_Green));
	float dTCycle = 0.5f;
	float rT = PI / dTCycle;
	float uVal = 0.5f + 0.5f * sin(rT * TNow());	// pi seconds per cycle
	NeoPixel::VAL valMin = NeoPixel::VAL_Full / 8;
	NeoPixel::VAL valMax = NeoPixel::VAL_Full / 2;
	NeoPixel::VAL val = valMin + NeoPixel::VAL(uVal * (valMax - valMin));

	NeoPixel::g_rgb.setHSV(hue, NeoPixel::SAT_Full, val);

	// NOTE bruceo: FastLED.show() done in loop() so multiple modules all get shown/synced at once.
#endif // ENABLE_ONBOARD_NEOPIXEL
}