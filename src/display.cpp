#include "display.h"

#if ENABLE_DISPLAY

#include "onboard.h"
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

	g_display.init(s_dY, s_dX);	// NOTE bruceo: backwards because we're in landscape?
	g_display.setRotation(s_rotation);
	g_canvas.setFont(&FreeSans12pt7b);
	g_canvas.setTextColor(ST77XX_WHITE); 
    pinMode(TFT_BACKLITE, OUTPUT);
}



void Display::Update()
{
	uint16_t colText =
				uint16_t((OnBoard::ColorCycle::r >> 3) << 11) |
				uint16_t((OnBoard::ColorCycle::g >> 2) << 5) |
				uint16_t((OnBoard::ColorCycle::b >> 3) << 0);
    g_canvas.setTextColor(colText);

    g_canvas.fillScreen(ST77XX_BLACK);

	S16 x;
	S16 y;
	U16 dX;
	U16 dY;

	g_canvas.getTextBounds(PROJECT_NAME, 0, 0, &x, &y, &dX, &dY);
	// center text on canvas
	S16 xCursor = ((s_dX - dX) / 2) + x;
	S16 yCursor = ((s_dY - dY) / 2) - y;

	static bool s_fTraceMetrics = false; // s_fTrace;

	TRACE(
		s_fTraceMetrics,
		"[DISPLAY] x:%d, y:%d, dX:%d, dY:%d, xCursor:%d yCursor:%d\n",
		x, y, dX, dY, xCursor, yCursor);

	s_fTraceMetrics = false;

    g_canvas.setCursor(xCursor, yCursor);
    g_canvas.println(PROJECT_NAME);
    g_display.drawRGBBitmap(0, 0, g_canvas.getBuffer(), s_dX, s_dY);
    digitalWrite(TFT_BACKLITE, HIGH);
}

#endif // ENABLE_DISPLAY