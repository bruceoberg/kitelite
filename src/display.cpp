#include "display.h"

#if ENABLE_DISPLAY

#include "trace.h"

#include <Adafruit_ST7789.h> 
#include <Fonts/FreeSans12pt7b.h>



namespace Display
{
	constexpr bool s_fTrace = true;

	// ST7789 240x135

	constexpr uint16_t s_dX = 240;
	constexpr uint16_t s_dY = 135;
	GFXcanvas16 g_canvas(s_dX, s_dY);

	constexpr uint8_t s_rotation = 3; // NOTE bruceo: not defined anywhere. 3 means landscape with 3 buttons to the left.
	Adafruit_ST7789 g_display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
}

using namespace Display;

void Display::Startup()
{
	TRACE(s_fTrace, "[DISPLAY] reached '%s'\n", __PRETTY_FUNCTION__);

	g_display.init(s_dY, s_dX);	// NOTE bruceo: yes these are in backwards order on purpose.
	g_display.setRotation(s_rotation);
	g_canvas.setFont(&FreeSans12pt7b);
	g_canvas.setTextColor(ST77XX_WHITE); 
}



void Display::Update()
{
	// CRGB rgb;
	// rgb.setHue(g_hueCur);
	// uint16_t colText = uint16_t((rgb.r >> 3) << 11) | uint16_t((rgb.g >> 2) << 5) | uint16_t(rgb.b >> 3);
    // g_canvas.setTextColor(colText);

    g_canvas.fillScreen(ST77XX_BLACK);
    g_canvas.setCursor(0, 34);
    g_canvas.println("Bruce's Feather");
    g_display.drawRGBBitmap(0, 0, g_canvas.getBuffer(), s_dX, s_dY);
    pinMode(TFT_BACKLITE, OUTPUT);	// BB bruceo: do this once in setup? or write it low then high?
    digitalWrite(TFT_BACKLITE, HIGH);
}

#endif // ENABLE_DISPLAY