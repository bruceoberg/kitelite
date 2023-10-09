#include "onboard.h"

#include "input.h"

#include <Adafruit_NeoPixel.h>

namespace OnBoard
{
	// Digital IO pin connected to the button. This will be driven with a
	// pull-up resistor so the switch pulls the pin to ground momentarily.
	// On a high -> low transition the button press logic will execute.

	namespace Button
	{
		constexpr int s_pin = BUTTON;
	}

	// Single neopixel

	namespace NeoPixel
	{
		constexpr int s_c = 1;
		constexpr int s_pin = PIN_NEOPIXEL;
		constexpr int s_npt = NEO_GRB + NEO_KHZ800;

		Adafruit_NeoPixel g_strip(s_c, s_pin, s_npt);

		using HUE = U16;	// 0 to 65535. 0: red .. green .. blue .. red
		using SAT = U8;		// 0 to 255. pure grayscale .. pure hue
		using VAL = U8;		// 0 to 255. black .. full brightness

		constexpr HUE HUE_First = 0;
		constexpr HUE HUE_Last = 65535;

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
	pinMode(Button::s_pin, INPUT);

	NeoPixel::g_strip.begin(); // Initialize NeoPixel strip object (REQUIRED)
	NeoPixel::g_strip.show();  // Initialize all pixels to 'off'
}

void OnBoard::Update()
{
	Input::SetKeyDown(Input::KEY_OnBoard, (digitalRead(Button::s_pin) == LOW));

	bool fIsDown = Input::FIsKeyDown(Input::KEY_OnBoard);

	NeoPixel::HUE hue = fIsDown ? NeoPixel::HUE_Blue : NeoPixel::HUE_Green;
	float dTCycle = 0.5f;
	float rT = PI / dTCycle;
	float uVal = 0.5f + 0.5f * sin(rT * TNow());	// pi seconds per cycle
	NeoPixel::VAL valMin = NeoPixel::VAL_Full / 6;
	NeoPixel::VAL valMax = NeoPixel::VAL_Full / 5;
	NeoPixel::VAL val = valMin + NeoPixel::VAL(uVal * (valMax - valMin));
	U32 rgbRaw = Adafruit_NeoPixel::ColorHSV(hue, NeoPixel::SAT_Full, val);
	U32 rgbGamma = Adafruit_NeoPixel::gamma32(rgbRaw);

	NeoPixel::g_strip.setPixelColor(0, rgbGamma);
	NeoPixel::g_strip.show();
}