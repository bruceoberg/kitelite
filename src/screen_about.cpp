#include "screen_about.h"

#if ENABLE_DISPLAY

#include "display.h"
#include "motion.h"
#include "onboard.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

static CScreenAbout g_screenAbout;

void CScreenAbout::Update()
{
	GFXcanvas16 & canvas = Display::Canvas();

	uint16_t colText =
				uint16_t((OnBoard::ColorCycle::r >> 3) << 11) |
				uint16_t((OnBoard::ColorCycle::g >> 2) << 5) |
				uint16_t((OnBoard::ColorCycle::b >> 3) << 0);
	canvas.setTextColor(colText);

	canvas.fillScreen(ST77XX_BLACK);

	S16 x;
	S16 y;
	U16 dX;
	U16 dY;

	const char * pChz = (Motion::FIsSpecial()) ? PROJECT_NAME "!" : PROJECT_NAME;

	canvas.getTextBounds(pChz, 0, 0, &x, &y, &dX, &dY);

	// center text on canvas

	S16 xCursor = ((canvas.width() - dX) / 2) + x;
	S16 yCursor = ((canvas.height() - dY) / 2) - y;

	canvas.setCursor(xCursor, yCursor);
	canvas.println(pChz);
}

void CScreenAbout::OnInput(Input::SEvent event)
{
	if (event.m_key == Input::KEY_OnBoard0 && event.m_eventk == Input::EVENTK_KeyPressed)
	{
		Screen::Pop();
	}
}

// Screen::About accessor

IScreen * Screen::About()
{
	return &g_screenAbout;
}

#endif // ENABLE_DISPLAY
