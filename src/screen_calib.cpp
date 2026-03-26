#include "screen_calib.h"

#if ENABLE_DISPLAY

#include "display.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// CScreenCalib

static CScreenCalib g_screenCalib;

void CScreenCalib::OnPush()
{
	m_fDirty = true;
}

void CScreenCalib::OnUncover()
{
	m_fDirty = true;
}

void CScreenCalib::OnInput(Input::SEvent event)
{
	if (event.m_key == Input::KEY_OnBoard0 && event.m_eventk == Input::EVENTK_KeyPressed)
	{
		Screen::Pop();
	}
}

void CScreenCalib::Update()
{
	if (!m_fDirty)
		return;

	m_fDirty = false;

	// BB(bruce): drive CCoordinator steps here once libcalib is integrated

	GFXcanvas16 & canvas = Display::Canvas();
	canvas.fillScreen(ST77XX_BLACK);
	canvas.setTextColor(ST77XX_WHITE);

	S16 xBound, yBound;
	U16 dXBound, dYBound;

	const char * pChz = "Calibration TBD";
	canvas.getTextBounds(pChz, 0, 0, &xBound, &yBound, &dXBound, &dYBound);

	S16 xCursor = ((canvas.width() - S16(dXBound)) / 2) - xBound;
	S16 yCursor = ((canvas.height() - S16(dYBound)) / 2) - yBound;

	canvas.setCursor(xCursor, yCursor);
	canvas.print(pChz);
}

// Screen::Calib accessor

IScreen * Screen::Calib()
{
	return &g_screenCalib;
}

#endif // ENABLE_DISPLAY
