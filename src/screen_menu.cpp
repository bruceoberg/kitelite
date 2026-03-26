#include "screen_menu.h"

#if ENABLE_DISPLAY

#include "display.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// button icons — 7x7 bitmaps drawn along left edge

// back/cancel — red X
static const U8 s_abIconBack[] =
{
	0b10000010,
	0b01000100,
	0b00101000,
	0b00010000,
	0b00101000,
	0b01000100,
	0b10000010,
};

// go/enter — green circle
static const U8 s_abIconGo[] =
{
	0b00111000,
	0b01000100,
	0b10000010,
	0b10000010,
	0b10000010,
	0b01000100,
	0b00111000,
};

// scroll — yellow down arrow with wide tail
static const U8 s_abIconScroll[] =
{
	0b00111000,
	0b00111000,
	0b00111000,
	0b11111110,
	0b01111100,
	0b00111000,
	0b00010000,
};

static const int s_dXIcon = 7;
static const int s_dYIcon = 7;
static const int s_nIconScale = 4;

static void DrawBitmapScaled(GFXcanvas16 & canvas, S16 x, S16 y, const U8 * pBitmap, int dX, int dY, int nScale, uint16_t col)
{
	for (int iY = 0; iY < dY; ++iY)
	{
		U8 bRow = pBitmap[iY];
		for (int iX = 0; iX < dX; ++iX)
		{
			if (bRow & (0x80 >> iX))
				canvas.fillRect(x + iX * nScale, y + iY * nScale, nScale, nScale, col);
		}
	}
}

// stub menu tree — placeholder items for testing navigation

static void StubAction() { ; }

static const SMenuItem s_aMenuiRoot[] =
{
	{ "Lights",		nullptr,	nullptr,		nullptr },
	{ "Calibrate",	nullptr,	StubAction,		nullptr },
	{ "About",		nullptr,	StubAction,		nullptr },
};

static const SMenu s_menuRoot =
{
	DIM(s_aMenuiRoot),
	s_aMenuiRoot,
};

// CScreenMenu

static CScreenMenu g_screenMenu;

CScreenMenu::CScreenMenu()
: m_fDirty(true),
  m_usecHomeDown(USEC_Nil)
{
}

void CScreenMenu::OnPush()
{
	m_menunav.Push(&s_menuRoot);
	m_fDirty = true;
	m_usecHomeDown = USEC_Nil;
}

void CScreenMenu::OnUncover()
{
	m_fDirty = true;
}

void CScreenMenu::OnInput(Input::SEvent event)
{
	static const USEC s_dUsecLongPress = DUsecFromDT(0.5f);	// 500ms threshold

	switch (event.m_key)
	{
		case Input::KEY_OnBoard0:	// Home
		{
			if (event.m_eventk == Input::EVENTK_KeyDown)
			{
				m_usecHomeDown = event.m_usec;
			}
			else if (event.m_eventk == Input::EVENTK_KeyUp)
			{
				bool fLongPress = (m_usecHomeDown != USEC_Nil) &&
									(event.m_usec - m_usecHomeDown >= s_dUsecLongPress);
				m_usecHomeDown = USEC_Nil;

				if (fLongPress)
				{
					// long press — pop all the way back to the bottom screen

					Screen::Pop();
				}
				else if (m_menunav.PMenuCur() != &s_menuRoot)
				{
					// short press in a submenu — pop one menu level

					m_menunav.Pop();
					m_fDirty = true;
				}
				else
				{
					// short press at root — pop this screen

					Screen::Pop();
				}
			}
			break;
		}

		case Input::KEY_OnBoard1:	// Go
		{
			if (event.m_eventk == Input::EVENTK_KeyPressed)
			{
				m_menunav.Activate();
				m_fDirty = true;
			}
			break;
		}

		case Input::KEY_OnBoard2:	// Scroll
		{
			if (event.m_eventk == Input::EVENTK_KeyPressed)
			{
				m_menunav.ScrollNext();
				m_fDirty = true;
			}
			break;
		}

		default:
			break;
	}
}

void CScreenMenu::Update()
{
	if (!m_fDirty)
		return;

	m_fDirty = false;

	GFXcanvas16 & canvas = Display::Canvas();
	canvas.fillScreen(ST77XX_BLACK);

	const SMenu * pMenu = m_menunav.PMenuCur();
	if (!pMenu)
		return;

	U8 iItemCur = m_menunav.IItemCur();

	// draw button icons along left edge, evenly spaced vertically

	S16 dYCanvas = canvas.height();
	S16 dYSpacing = dYCanvas / 3;
	S16 dYIconScaled = s_dYIcon * s_nIconScale;
	S16 dXIconScaled = s_dXIcon * s_nIconScale;

	DrawBitmapScaled(canvas, 2, (dYSpacing * 0) + (dYSpacing - dYIconScaled) / 2, s_abIconBack,   s_dXIcon, s_dYIcon, s_nIconScale, ST77XX_RED);
	DrawBitmapScaled(canvas, 2, (dYSpacing * 1) + (dYSpacing - dYIconScaled) / 2, s_abIconGo,     s_dXIcon, s_dYIcon, s_nIconScale, ST77XX_GREEN);
	DrawBitmapScaled(canvas, 2, (dYSpacing * 2) + (dYSpacing - dYIconScaled) / 2, s_abIconScroll,  s_dXIcon, s_dYIcon, s_nIconScale, ST77XX_YELLOW);

	// measure item height — assume uniform text size

	S16 xBound, yBound;
	U16 dXBound, dYBound;
	canvas.getTextBounds("X", 0, 0, &xBound, &yBound, &dXBound, &dYBound);
	S16 dYLine = S16(dYBound) + 4;		// line height with padding

	// center the item list vertically in the area right of the icon gutter

	S16 xGutter = dXIconScaled + 6;	// left margin past icons
	S16 dXContent = canvas.width() - xGutter;
	S16 dYList = dYLine * pMenu->m_cMenui;
	S16 yTop = (dYCanvas - dYList) / 2;
	if (yTop < 0)
		yTop = 0;

	// draw items

	for (U8 iItem = 0; iItem < pMenu->m_cMenui; ++iItem)
	{
		const SMenuItem & menui = pMenu->m_aMenui[iItem];

		const char * pChz = menui.m_pChzLabel;
		if (menui.m_pfnLabel)
			pChz = menui.m_pfnLabel();
		if (!pChz)
			pChz = "???";

		S16 yItem = yTop + (dYLine * iItem) - yBound;

		// center horizontally within content area

		canvas.getTextBounds(pChz, 0, 0, &xBound, &yBound, &dXBound, &dYBound);
		S16 xItem = xGutter + (dXContent - S16(dXBound)) / 2;

		if (iItem == iItemCur)
		{
			// highlight: inverted colors

			S16 yFill = yTop + (dYLine * iItem);
			canvas.fillRect(xGutter, yFill, dXContent, dYLine, ST77XX_WHITE);
			canvas.setTextColor(ST77XX_BLACK);
		}
		else
		{
			canvas.setTextColor(ST77XX_WHITE);
		}

		canvas.setCursor(xItem, yItem);
		canvas.print(pChz);
	}
}

// Screen::Menu accessor

IScreen * Screen::Menu()
{
	return &g_screenMenu;
}

#endif // ENABLE_DISPLAY
