#pragma once

#include "display.h"
#include "screen.h"
#include "menu.h"
#include "clock.h"

#if ENABLE_DISPLAY

class CScreenMenu : public IScreen // tag: screenmenu
{
public:
				CScreenMenu();

	void		Update() override;
	void		OnInput(Input::SEvent event) override;
	void		OnPush() override;
	void		OnUncover() override;

private:
	CMenuNav	m_menunav;
	bool		m_fDirty;
	USEC		m_usecUncovered;	// timestamp of last OnPush/OnUncover, for ignoring stale input
};

namespace Screen
{
	IScreen * Menu();
}

#endif // ENABLE_DISPLAY
